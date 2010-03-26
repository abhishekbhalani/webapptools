#include <signal.h>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#ifdef WIN32
#include <errno.h>
#include <winsock2.h>
#include <process.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif //WIN32

#include <log4cxx/logger.h>
#include <weDispatch.h>

#include "../common/redisclient.h"
#include "../common/sysinfo.h"
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
webEngine::engine_dispatcher* we_dispatcer;

enum _keep_alive_fileds {
    ip_addr = 0,
    //instance,
    scanner_name,
    keepalive_timeout,
	running_task,
    status,
    scan_version,

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
extern string scaner_version;
extern int scaner_instance;
extern string db1_instance_name;

void signal_halt(int sig)
{
    inLoop = false;
    //! todo: pause all tasks
    if (sig == 0) {
        LOG4CXX_INFO(scan_logger, "Received software request to exit");
    }
    else {
        LOG4CXX_INFO(scan_logger, "Signal received - exit scanner");
    }
}

void save_plugin_ui(string args)
{
    LOG4CXX_TRACE(scan_logger, "Save UI for plugin " << args);
    int pos;
    string db_key;
    string db_data;
    webEngine::string_list str_data;

    pos = args.find(' ');
    if (pos == string::npos) {
        pos = args.find('\t');
    }
    if (pos != string::npos) {
        args = args.substr(0, pos);
    }
    webEngine::plugin_list plgs = we_dispatcer->get_plugin_list();
    try {
        for (int i = 0; i < plgs.size(); i++)
        {
            if (boost::iequals(plgs[i].plugin_id, args) == 0) {
                // save plugin icon
                db_key = "Plugin:Scanner:" + plgs[i].plugin_id + ":Icon";
                db_data = "";
                for (int j = 0; j < plgs[i].plugin_icon.size(); j++)
                {
                    db_data += plgs[i].plugin_icon[j];
                    db_data += "\n";
                }
                db1_client->set(db_key, db_data);
                webEngine::i_plugin* plg = we_dispatcer->load_plugin(args);
                if (plg != NULL) {
                    db_key = "Plugin:Scanner:" + plgs[i].plugin_id + ":UI";
                    db_data = plg->get_setup_ui();
                    db1_client->set(db_key, db_data);
                    plg->release();
                }
                else {
                    LOG4CXX_ERROR(scan_logger, "Can't load plugin " << plgs[i].plugin_id << "; " << plgs[i].plugin_desc);
                }
            }// if (plugin_id == args)
        }
    } catch(redis::redis_error& re) {
        LOG4CXX_FATAL(scan_logger, "Can't save plugin UI. Redis error: " << (string)re);
    }
}

void send_keepalive(int timeout) {
    LOG4CXX_TRACE(scan_logger, "Send keep-alive signal");
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

void send_plugins_list(int timeout) {
    LOG4CXX_TRACE(scan_logger, "Save plugins list");
    string db_key = "ScanModule:PlugIns:" + scaner_uuid + ":" + boost::lexical_cast<string>(scaner_instance);
    try {
        webEngine::plugin_list plgs = we_dispatcer->get_plugin_list();
        if (db1_client->exists(db_key)) {
            db1_client->del(db_key);
        }
        for (int i = 0; i < plgs.size(); i++)
        {
            string info = plgs[i].interface_name + "|";
            info += plgs[i].interface_list[1] + "|";
            info += plgs[i].plugin_id + "|";
            info += plgs[i].plugin_desc;
            LOG4CXX_TRACE(scan_logger, "Save info [" << i << "]: " << info);
            db1_client->rpush(db_key, info);
        }
    } catch(redis::redis_error& re) {
        LOG4CXX_ERROR(scan_logger, "Can't save plugins list. Redis error: " << (string)re);
    }

}

void send_sysinfo(int timeout) {
    LOG4CXX_TRACE(scan_logger, "Send system information");
	// not divide to instances - whole system information
	string db_key =  "ScanModule:SysInfo:" + scaner_uuid;
    try {
        LOG4CXX_TRACE(scan_logger, "Get memory information");
        sys_info_packet[memory_size] = sys_meminfo();
        LOG4CXX_TRACE(scan_logger, "Get CPU information");
        sys_info_packet[cpu_usage] = sys_cpu();
        LOG4CXX_TRACE(scan_logger, "Get disk information");
        sys_info_packet[disk_size] = sys_disk();
        sys_info_packet[max_tasks] = boost::lexical_cast<string>(max_tasks_count);
        if (db1_client->exists(db_key)) {
            db1_client->del(db_key);
        }
        for (int i = 0; i < last_sys_info_field; i++) {
            db1_client->rpush(db_key, sys_info_packet[i]);
        }
        db1_client->expire(db_key, timeout);

        // save to DB1 information about plugins.
        send_plugins_list(timeout);
    } catch(redis::redis_error& re) {
        LOG4CXX_ERROR(scan_logger, "Can't save system information. Redis error: " << (string)re);
    }
}

void dispatcher_routine(po::variables_map& vm)
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

    we_dispatcer = new webEngine::engine_dispatcher;
    if (we_dispatcer == NULL) {
        LOG4CXX_FATAL(scan_logger, "Can't create webEngine dispatcher - exit!");
        return;
    }
    // refresh plugins information
    bfs::path plg_path = vm["plugin_dir"].as<string>();
    we_dispatcer->refresh_plugin_list(plg_path);
    webEngine::i_plugin* plg = we_dispatcer->load_plugin(vm["db2_interface"].as<string>());
    if (plg == NULL) {
        LOG4CXX_FATAL(scan_logger, "Can't load plug-in for Storage DB connection: " << vm["db2_interface"].as<string>());
        return;
    }
    LOG4CXX_INFO(scan_logger, "Storage plugin loaded successfully: " << vm["db2_interface"].as<string>());
    LOG4CXX_DEBUG(scan_logger, "Plugin ID=" << plg->get_id() << "; Description: " << plg->get_description());
    webEngine::i_storage* storage = (webEngine::i_storage*)plg->get_interface("i_storage");
    if (plg == NULL) {
        LOG4CXX_FATAL(scan_logger, "No iStorage interface in the plugin " << plg->get_id() << "(ID="  << plg->get_id() );
        LOG4CXX_FATAL(scan_logger, "Can't load plug-in for Storage DB connection: " << vm["db2_interface"].as<string>());
        return;
    }
    string params = "";
    if (vm.count("db2_parameters")) {
        params = vm["db2_parameters"].as<string>();
    }
    storage->init_storage(params);
    we_dispatcer->storage(storage);
    LOG4CXX_INFO(scan_logger, "Storage initialised");

    // init values 
    sys_cpu();
    queue_key = "ScanModule:Queue:" + scaner_uuid + ":" + boost::lexical_cast<string>(scaner_instance);
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

    //keep_alive_packet[instance] =  boost::lexical_cast<string>(inst);
    keep_alive_packet[scanner_name] = vm["scanner_name"].as<string>();
    keep_alive_packet[keepalive_timeout] = boost::lexical_cast<string>(keep_alive_timeout);
    keep_alive_packet[status] = "READY";
    keep_alive_packet[scan_version] = scaner_version;
    send_keepalive(keep_alive_timeout);

	sys_info_packet[os_name] = sys_uname();
    sys_info_packet[sysinfo_timeout] = boost::lexical_cast<string>(sys_info_timeout);
    send_sysinfo(sys_info_timeout);

    send_plugins_list(sys_info_timeout);

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
            string args;
            int pos;
            cmd = db1_client->lpop(queue_key);
            while (cmd != redis::client::missing_value) {
                LOG4CXX_DEBUG(scan_logger, "Process command: " << cmd);
                // separate command from argumenets
                pos = cmd.find(' ');
                if (pos == string::npos) {
                    pos = cmd.find('\t');
                }
                if (pos != string::npos) {
                    args = cmd.substr(pos + 1);
                    cmd = cmd.substr(0, pos);
                }
                else {
                    args = "";
                }
                boost::to_upper(cmd);

                // process commands
                if (cmd == "EXIT") {
                    signal_halt(0);
                    // need to exit and save other commands in the queue
                    break;
                }
                else if (cmd == "RESTART") {
                    // need to exit this copy and run new instance
                    signal_halt(0);
                    LOG4CXX_INFO(scan_logger, "Restart request recieved");
                    break;
                }
                else if (cmd == "PLUGINS") {
                    // refresh plugins list
                    send_plugins_list(sys_info_timeout);
                }
                else if (cmd == "SAVE_PLUGIN_UI") {
                    // refresh plugins list
                    save_plugin_ui(args);
                }
                else if (cmd == "") {
                    // other cmd's
                }
                cmd = db1_client->lpop(queue_key);
            }
        } catch(redis::redis_error& re) {
            LOG4CXX_FATAL(scan_logger, "Can't get commands queue " << (string)re);
            signal_halt(0); 
        }
    }
    // finalize...
    delete we_dispatcer;

}
