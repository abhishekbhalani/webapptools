#include <string>
#include <boost/lexical_cast.hpp>
#ifdef WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#endif

using namespace std;

#ifdef WIN32
#define BUFSIZE 256

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

string sys_uname()
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
#else 
string sys_uname()
{
    struct utsname sys_info;
    string result = "";
    int i;

    i = uname(&sys_info);
    if (i == 0) {
        result += sys_info.sysname;
        result += " ";
        result += sys_info.nodename;
        result += " ";
        result += sys_info.release;
        result += " ";
        result += sys_info.version;
        result += " ";
        result += sys_info.machine;
    }
    else {
        result = "<Unknown> (uname failed)";
    }

    return result;
}
#endif

#ifdef WIN32
string sys_meminfo()
{
    PERFORMANCE_INFORMATION pfInfo;
    string  result = "";
    char    msg[100];
    double  phAll, phUsed, phPerc;

    pfInfo.cb = sizeof(pfInfo);
    if (GetPerformanceInfo(&pfInfo, sizeof(pfInfo))) {
        phAll = (double)pfInfo.PhysicalTotal * pfInfo.PageSize / 1024 / 1024;
        phUsed = phAll - ((double)pfInfo.PhysicalAvailable * pfInfo.PageSize / 1024 / 1024);
        sprintf(msg, "%.2fMb / %.2fMb (%02d%%)", phUsed, phAll, (int)((phUsed / phAll) * 100));
        result = msg;
    }

    return result;
}
#else
string sys_meminfo()
{
    struct sysinfo info;
    string result = "";
    char    msg[100];
    double totRam, availRam;

    if (sysinfo(&info) == 0) {
        totRam = (double)info.totalram * info.mem_unit / 1024 / 1024;
        availRam = (double)info.freeram * info.mem_unit / 1024 / 1024;
        sprintf(msg, "%.2fMb / %.2fMb (%02d%%)", availRam, totRam, (int)((availRam / totRam) * 100));
        result = msg;
    }
    else {
        result = "<Unknown> (sysinfo failed)";
    }

    return result;
}
#endif

#ifdef WIN32
FILETIME idleTime;
FILETIME kernelTime;
FILETIME userTime;
bool isFirstSysTime = true;

string sys_cpu()
{
    string result = "";
    int cpu;
    FILETIME iTime;
    FILETIME kTime;
    FILETIME uTime;
    __int64  idle, krnl, usr, sys, tmp;

    if (isFirstSysTime)
    {
        GetSystemTimes( &idleTime, &kernelTime, &userTime );
        isFirstSysTime = false;
        return "";
    }
    if (GetSystemTimes( &iTime, &kTime, &uTime ) ) {
        idle = ((__int64)iTime.dwHighDateTime << 32) +
            iTime.dwLowDateTime;
        tmp = ((__int64)idleTime.dwHighDateTime << 32) +
            idleTime.dwLowDateTime;
        idle -= tmp;
        idleTime = iTime;

        krnl = ((__int64)kTime.dwHighDateTime << 32) +
            kTime.dwLowDateTime;
        tmp = ((__int64)kernelTime.dwHighDateTime << 32) +
            kernelTime.dwLowDateTime;
        krnl -= tmp;
        kernelTime = kTime;

        usr = ((__int64)uTime.dwHighDateTime << 32) +
            uTime.dwLowDateTime;
        tmp = ((__int64)userTime.dwHighDateTime << 32) +
            userTime.dwLowDateTime;
        krnl -= tmp;
        userTime = uTime;

        sys = krnl + usr;
        cpu = (int)(sys * 100 / (sys+idle));

        result = boost::lexical_cast<string>(cpu) + "%";
    }

    return result;
}
#else
string sys_cpu()
{
    string result = "<Unknown>";

    return result;
}
#endif

#ifdef WIN32
string sys_disk()
{
    string result = "<Unknown>";

    return result;
}
#else
string sys_disk()
{
    string result = "<Unknown>";

    return result;
}
#endif
