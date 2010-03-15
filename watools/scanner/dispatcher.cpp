#include <signal.h>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#ifdef WIN32
#include <errno.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif //WIN32

#include <log4cxx/logger.h>

#include "../common/redisclient.h"

#include "version.h"

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace std;

namespace po = boost::program_options;
namespace bfs = boost::filesystem;
namespace btm = boost::posix_time;

// global variables
bool inLoop = true;
int max_tasks_count = 10;  // why? no comments...
int running_tasks_count = 0;

enum _keep_alive_fileds {
    ip_addr = 0,
    instance,
    scanner_name,
    keepalive_timeout,
	running_task,
    status,

    last_keep_alive_field
} keep_alive_fileds;
string keep_alive_packet[last_keep_alive_field];

enum _sys_info_fields {
	os_name,
	memory_size,
	disk_size,
	cpu_usage,
	max_tasks,
	sysinfo_timeout,

	last_sys_info_field
} sys_info_fields;
string sys_info_packet[last_sys_info_field];

extern LoggerPtr scan_logger;
extern redis::client* db1_client;
extern boost::mutex db1_lock;
extern string scaner_uuid;
extern string db1_instance_name;

#ifdef WIN32
#define BUFSIZE 256

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

string uname()
{
   OSVERSIONINFOEX osvi;
   SYSTEM_INFO si;
   PGNSI pGNSI;
   PGPI pGPI;
   BOOL bOsVersionInfoEx;
   DWORD dwType;
   string result;

   ZeroMemory(&si, sizeof(SYSTEM_INFO));
   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

   if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
      return "Unknown Microsoft Windows";

   // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.

   pGNSI = (PGNSI) GetProcAddress(
      GetModuleHandle(TEXT("kernel32.dll")), 
      "GetNativeSystemInfo");
   if(NULL != pGNSI)
      pGNSI(&si);
   else GetSystemInfo(&si);

   if ( VER_PLATFORM_WIN32_NT==osvi.dwPlatformId && 
        osvi.dwMajorVersion > 4 )
   {
      result = "Microsoft ";

      // Test for the specific product.

      if ( osvi.dwMajorVersion == 6 )
      {
         if( osvi.dwMinorVersion == 0 )
         {
            if( osvi.wProductType == VER_NT_WORKSTATION )
                result += "Windows Vista ";
            else result += "Windows Server 2008 ";
         }

         if ( osvi.dwMinorVersion == 1 )
         {
            if( osvi.wProductType == VER_NT_WORKSTATION )
                result += "Windows 7 ";
            else result += "Windows Server 2008 R2 ";
         }
         
         pGPI = (PGPI) GetProcAddress(
            GetModuleHandle(TEXT("kernel32.dll")), 
            "GetProductInfo");

         pGPI( osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);

         switch( dwType )
         {
            case PRODUCT_ULTIMATE:
               result += "Ultimate Edition";
               break;
/*            case PRODUCT_PROFESSIONAL:
               result += "Professional";
               break;*/
            case PRODUCT_HOME_PREMIUM:
               result += "Home Premium Edition";
               break;
            case PRODUCT_HOME_BASIC:
               result += "Home Basic Edition";
               break;
            case PRODUCT_ENTERPRISE:
               result += "Enterprise Edition";
               break;
            case PRODUCT_BUSINESS:
               result += "Business Edition";
               break;
            case PRODUCT_STARTER:
               result += "Starter Edition";
               break;
            case PRODUCT_CLUSTER_SERVER:
               result += "Cluster Server Edition";
               break;
            case PRODUCT_DATACENTER_SERVER:
               result += "Datacenter Edition";
               break;
            case PRODUCT_DATACENTER_SERVER_CORE:
               result += "Datacenter Edition (core installation)";
               break;
            case PRODUCT_ENTERPRISE_SERVER:
               result += "Enterprise Edition";
               break;
            case PRODUCT_ENTERPRISE_SERVER_CORE:
               result += "Enterprise Edition (core installation)";
               break;
            case PRODUCT_ENTERPRISE_SERVER_IA64:
               result += "Enterprise Edition for Itanium-based Systems";
               break;
            case PRODUCT_SMALLBUSINESS_SERVER:
               result += "Small Business Server";
               break;
            case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
               result += "Small Business Server Premium Edition";
               break;
            case PRODUCT_STANDARD_SERVER:
               result += "Standard Edition";
               break;
            case PRODUCT_STANDARD_SERVER_CORE:
               result += "Standard Edition (core installation)";
               break;
            case PRODUCT_WEB_SERVER:
               result += "Web Server Edition";
               break;
         }
      }

      if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
      {
         if( GetSystemMetrics(SM_SERVERR2) )
            result += "Windows Server 2003 R2, ";
         else if ( osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER )
            result += "Windows Storage Server 2003";
         else if ( osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER ) /*VER_SUITE_WH_SERVER*/
            result += "Windows Home Server";
         else if( osvi.wProductType == VER_NT_WORKSTATION &&
                  si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
         {
            result += "Windows XP Professional x64 Edition";
         }
         else result += "Windows Server 2003, ";

         // Test for the server type.
         if ( osvi.wProductType != VER_NT_WORKSTATION )
         {
            if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_IA64 )
            {
                if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                   result += "Datacenter Edition for Itanium-based Systems";
                else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                   result += "Enterprise Edition for Itanium-based Systems";
            }

            else if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
            {
                if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                   result += "Datacenter x64 Edition";
                else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                   result += "Enterprise x64 Edition";
                else result += "Standard x64 Edition";
            }

            else
            {
                if ( osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER )
                   result += "Compute Cluster Edition";
                else if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                   result += "Datacenter Edition";
                else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                   result += "Enterprise Edition";
                else if ( osvi.wSuiteMask & VER_SUITE_BLADE )
                   result += "Web Edition";
                else result += "Standard Edition";
            }
         }
      }

      if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
      {
         result += "Windows XP ";
         if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
            result += "Home Edition";
         else result += "Professional";
      }

      if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
      {
         result += "Windows 2000 ";

         if ( osvi.wProductType == VER_NT_WORKSTATION )
         {
            result += "Professional";
         }
         else 
         {
            if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
               result += "Datacenter Server";
            else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
               result += "Advanced Server";
            else result += "Server";
         }
      }

       // Include service pack (if any) and build number.

      if( strlen(osvi.szCSDVersion) > 0 )
      {
          result += " ";
          result += osvi.szCSDVersion;
      }

      TCHAR buf[80];

	  result += " (build " + boost::lexical_cast<string>((int)osvi.dwBuildNumber) +")";

      if ( osvi.dwMajorVersion >= 6 )
      {
         if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
            result += ", 64-bit";
         else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL )
            result += ", 32-bit";
      }
   }

   else
   {  
	   result = "Unknown version of Windows: ";
   }
   return result;
}
#endif

void signal_halt(int sig)
{
    inLoop = false;
    //! todo: stop all tasks
    if (sig == 0) {
        LOG4CXX_INFO(scan_logger, "Received software request to exit");
    }
    else {
        LOG4CXX_INFO(scan_logger, "Signal received - exit scanner");
    }
} 

void send_keepalive(int timeout) {
    LOG4CXX_DEBUG(scan_logger, "Send keep-alive signal");
    try {
		keep_alive_packet[running_task] = boost::lexical_cast<string>(running_tasks_count);
        if (db1_client->exists(db1_instance_name)) {
            db1_client->del(db1_instance_name);
        }
        for (int i = 0; i < last_keep_alive_field; i++) {
            db1_client->rpush(db1_instance_name, keep_alive_packet[i]);
        }
        db1_client->expire(db1_instance_name, timeout);
    } catch(redis::redis_error& re) {
        LOG4CXX_FATAL(scan_logger, "Can't save keep-alive information. Redis error: " << (string)re);
    }
}

void send_sysinfo(int timeout) {
    LOG4CXX_DEBUG(scan_logger, "Send system information");
	// not divide to instances - whole system information
	string db_key =  "ScanModule:SysInfo:" + scaner_uuid;
    try {
		sys_info_packet[max_tasks] = boost::lexical_cast<string>(max_tasks_count);
        if (db1_client->exists(db_key)) {
            db1_client->del(db_key);
        }
        for (int i = 0; i < last_sys_info_field; i++) {
            db1_client->rpush(db_key, sys_info_packet[i]);
        }
        db1_client->expire(db_key, timeout);
    } catch(redis::redis_error& re) {
        LOG4CXX_ERROR(scan_logger, "Can't save system information. Redis error: " << (string)re);
    }
}

void dispatcher_routine(int inst, po::variables_map& vm)
{
    btm::ptime  keep_alive;
    btm::ptime  sys_info;
    btm::ptime  curr_time;
    int         keep_alive_timeout;
    int         sys_info_timeout;
    char        ac[80] = {0};
    string      queue_key;

    if (db1_client == NULL) {
        LOG4CXX_FATAL(scan_logger, "Redis client seems to be uninitialized! Exiting...");
        return;
    }
    // set signal processor
    signal(SIGINT, signal_halt);

    // init values 
    queue_key = "ScanModule:Queue:" + scaner_uuid + ":" + boost::lexical_cast<string>(inst);
    keep_alive_timeout = vm["keepalive_timeout"].as<int>();
    sys_info_timeout = vm["sysinfo_timeout"].as<int>();

    keep_alive_packet[ip_addr] = "";
    // find all ip's of machine
    if (gethostname(ac, sizeof(ac)) != SOCKET_ERROR) 
    {
        struct hostent *phe = gethostbyname(ac);
        if (phe != 0) 
        {
            for (int i = 0; phe->h_addr_list[i] != 0; ++i) 
            {
                struct in_addr addr;
                string saddr;
                memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
                if ((addr.s_addr & 0xFF0000) != 0x7F0000) {
                    saddr  = boost::lexical_cast<string>((int)addr.s_net) + ".";
                    saddr += boost::lexical_cast<string>((int)addr.s_host) + ".";
                    saddr += boost::lexical_cast<string>((int)addr.s_lh) + ".";
                    saddr += boost::lexical_cast<string>((int)addr.s_impno);
                    if (keep_alive_packet[ip_addr].length() != 0 ) {
                        keep_alive_packet[ip_addr] += "; ";
                    }
                    LOG4CXX_TRACE(scan_logger, "Found ip addr: " << saddr);
                    keep_alive_packet[ip_addr] += saddr;
                    break;
                }
            }
        }
        else {
            LOG4CXX_ERROR(scan_logger, "gethostbyname failed");
            keep_alive_packet[ip_addr] = "<unknown>";
        }
    }
    else {
        LOG4CXX_ERROR(scan_logger, "gethostname failed");
        keep_alive_packet[ip_addr] = "<unknown>";
    }

    keep_alive_packet[instance] =  boost::lexical_cast<string>(inst);
    keep_alive_packet[scanner_name] = vm["scanner_name"].as<string>();
    keep_alive_packet[keepalive_timeout] = boost::lexical_cast<string>(vm["keepalive_timeout"].as<int>());
    keep_alive_packet[status] = "READY";
    send_keepalive(keep_alive_timeout);

	sys_info_packet[os_name] = uname();
    send_sysinfo(sys_info_timeout);

    inLoop = true;
    keep_alive = btm::second_clock::local_time();
    sys_info = btm::second_clock::local_time();
    // run endless loop
    while(inLoop) {
        vector<string>  out;
        int             reply;

        boost::this_thread::sleep(boost::posix_time::milliseconds(500)); // :seconds(1));
        curr_time = btm::second_clock::local_time();
        if ( (keep_alive + btm::seconds(keep_alive_timeout)) <= curr_time) {
            keep_alive = curr_time;
            send_keepalive(keep_alive_timeout);
        }
        if ( (sys_info + btm::seconds(sys_info_timeout)) <= curr_time) {
            sys_info = curr_time;
            send_sysinfo(sys_info_timeout);
        }
        // check commands
        try {
            string cmd;
            cmd = db1_client->lpop(queue_key);
            while (cmd != redis::client::missing_value) {
                LOG4CXX_DEBUG(scan_logger, "Process command: " << cmd);
                if (cmd == "EXIT") {
                    signal_halt(0);
                    // need to exit and save other commands in the queue
                    break;
                }
                cmd = db1_client->rpop(queue_key);
            }
        } catch(redis::redis_error& re) {
            LOG4CXX_FATAL(scan_logger, "Can't get commands queue " << (string)re);
            signal_halt(0); 
        }
    }

}
