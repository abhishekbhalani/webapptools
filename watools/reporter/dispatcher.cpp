#include <signal.h>
#include <boost/algorithm/string.hpp>
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
#include <weiPlugin.h>
#include <weUrl.h>
#include <weTask.h>

#include "../common/sysinfo.h"
#include "version.h"

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace std;

namespace po = boost::program_options;
namespace bfs = boost::filesystem;
namespace btm = boost::posix_time;

#define MODULE_CLASS_REPORTER    1

// global variables
bool inLoop = true;
int max_tasks_count = 10;  // why? no comments...
int running_tasks_count = 0;
webEngine::engine_dispatcher* we_dispatcer;

enum _keep_alive_fileds {
    ip_addr = 0,
    //instance,
    module_name,
    keepalive_timeout,
    running_task,
    status,
    module_version,

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

extern LoggerPtr module_logger;
extern string module_uuid;
extern string reporter_version;
extern int reporter_instance;

void signal_halt(int sig)
{
    inLoop = false;
    //! todo: pause all tasks
    if (sig == 0) {
        LOG4CXX_INFO(module_logger, "Received software request to exit");
    } else {
        LOG4CXX_INFO(module_logger, "Signal received - exit reporter");
    }
}

void send_keepalive(webEngine::i_storage* store, int timeout)
{
    LOG4CXX_TRACE(module_logger, "Send keep-alive signal");
    keep_alive_packet[running_task] = boost::lexical_cast<string>(running_tasks_count);

    webEngine::db_condition c_instance, c_class, c_id;
    c_instance.field() = "modules.instance";
    c_instance.operation() = webEngine::db_condition::equal;
    c_instance.value() = reporter_instance;
    c_class.field() = "modules.class";
    c_class.operation() = webEngine::db_condition::equal;
    c_class.value() = MODULE_CLASS_REPORTER;
    c_id.field() = "modules.id";
    c_id.operation() = webEngine::db_condition::equal;
    c_id.value() = module_uuid;

    webEngine::db_cursor rec = store->set(c_instance && c_class && c_id, "modules");
    rec["modules.id"] = module_uuid;
    rec["modules.instance"] = reporter_instance;
    rec["modules.class"] = MODULE_CLASS_REPORTER;
    rec["modules.version"] = keep_alive_packet[module_version];
    rec["modules.ipaddr"] = keep_alive_packet[ip_addr];
    rec["modules.name"] = keep_alive_packet[module_name];
    rec["modules.stamp"] = (int)time(NULL);
    rec["modules.timeout"] = timeout;
    rec["modules.onrun"] = running_tasks_count;
    rec["modules.status"] = keep_alive_packet[status];
    rec.close();
}

void send_sysinfo(webEngine::i_storage* store, int timeout)
{
    LOG4CXX_TRACE(module_logger, "Send system information");
    // not divide to instances - whole system information

    // save to DB1 information about plugins.
    webEngine::db_condition c_instance;
    c_instance.field() = "modules_info.module_id";
    c_instance.operation() = webEngine::db_condition::equal;
    c_instance.value() = module_uuid;
    webEngine::db_cursor rec = store->set(c_instance, "modules_info");

    rec["modules_info.module_id"] = module_uuid;
    rec["modules_info.osname"] = sys_info_packet[os_name];
    LOG4CXX_TRACE(module_logger, "Get memory information");
    sys_info_packet[memory_size] = sys_meminfo();
    rec["modules_info.mem_size"] = sys_info_packet[memory_size];
    LOG4CXX_TRACE(module_logger, "Get CPU information");
    sys_info_packet[cpu_usage] = sys_cpu();
    rec["modules_info.cpu_usage"] = sys_info_packet[cpu_usage];
    LOG4CXX_TRACE(module_logger, "Get disk information");
    sys_info_packet[disk_size] = sys_disk();
    rec["modules_info.disk_size"] = sys_info_packet[disk_size];
    sys_info_packet[max_tasks] = boost::lexical_cast<string>(max_tasks_count);
    rec["modules_info.max_tasks"] = max_tasks_count;
    rec["modules_info.stamp"] = (int)time(NULL);
    rec["modules_info.timeout"] = timeout;

    rec.close();
    // save info

    //send_plugins_list(timeout);
}

string get_command(webEngine::i_storage* store)
{
    string retval = "";

    webEngine::db_condition c_instance, c_stamp;

    c_instance.field() = "module_cmds.module_id";
    c_instance.operation() = webEngine::db_condition::equal;
    c_instance.value() = module_uuid + ":" +  boost::lexical_cast<string>(reporter_instance);
    webEngine::db_cursor rec = store->get(c_instance, "module_cmds");
    if (rec.is_not_end()) {
        retval = rec["module_cmds.cmd"].get<string>();

        c_stamp.field() = "module_cmds.timestamp";
        c_stamp.operation() = webEngine::db_condition::equal;
        c_stamp.value() = rec["module_cmds.timestamp"];
        store->del(c_instance && c_stamp);
    }

    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn int dispatcher_routine(po::variables_map& vm)
///
/// @brief  Dispatcher routine.
///
/// @author A. Abramov
/// @date   20.07.2010
///
/// @param [in,out] vm  The configuration variables map.
///
/// @return code - 0 - graceful shutdown; 1 - need restart.
////////////////////////////////////////////////////////////////////////////////////////////////////

int dispatcher_routine(po::variables_map& vm)
{
    btm::ptime  keep_alive;
    btm::ptime  sys_info;
    btm::ptime  curr_time;
    int         keep_alive_timeout;
    int         sys_info_timeout;
    char        ac[80] = {0};
    string      queue_key;
    int         retcode = 0;

    // set signal processor
    signal(SIGINT, signal_halt);

    we_dispatcer = new webEngine::engine_dispatcher;
    if (we_dispatcer == NULL) {
        LOG4CXX_FATAL(module_logger, "Can't create webEngine dispatcher - exit!");
        return retcode;
    }
    // refresh plugins information
    bfs::path plg_path = vm["plugin_dir"].as<string>();
    we_dispatcer->refresh_plugin_list(plg_path);
    webEngine::i_plugin* plg = we_dispatcer->load_plugin(vm["db_interface"].as<string>());
    if (plg == NULL) {
        LOG4CXX_FATAL(module_logger, "Can't load plug-in for Storage DB connection: " << vm["db_interface"].as<string>());
        return retcode;
    }
    LOG4CXX_INFO(module_logger, "Storage plugin loaded successfully: " << vm["db_interface"].as<string>());
    LOG4CXX_DEBUG(module_logger, "Plugin ID=" << plg->get_id() << "; Description: " << plg->get_description());
    webEngine::i_storage* storage = (webEngine::i_storage*)plg->get_interface("i_storage");
    if (plg == NULL) {
        LOG4CXX_FATAL(module_logger, "No i_storage interface in the plugin " << plg->get_id() << "(ID="  << plg->get_id() );
        LOG4CXX_FATAL(module_logger, "Can't load plug-in for Storage DB connection: " << vm["db_interface"].as<string>());
        return retcode;
    }
    string params = "";
    if (vm.count("db_parameters")) {
        params = vm["db_parameters"].as<string>();
    }
    storage->init_storage(params);
    we_dispatcer->storage(storage);
    LOG4CXX_INFO(module_logger, "Storage initialised");

    // clear outdated records
    // verify instance id
    webEngine::db_condition c_instance, c_class;
    c_instance.field() = "modules.instance";
    c_instance.operation() = webEngine::db_condition::equal;
    c_instance.value() = module_uuid;
    c_class.field() = "modules.class";
    c_class.operation() = webEngine::db_condition::equal;
    c_class.value() = MODULE_CLASS_REPORTER;
    webEngine::db_cursor cur = storage->get(c_instance && c_class, "modules");
    for (cur; cur.is_not_end(); ++cur) {
        if (cur["modules.id"].get<int>() > reporter_instance) {
            reporter_instance = cur["modules.id"].get<int>();
        }
    }
    LOG4CXX_TRACE(module_logger, "Found " << reporter_instance << " instances of " << module_uuid);
    reporter_instance++;
    int max_inst = vm["instances"].as<int>();
    if (reporter_instance > max_inst) {
        LOG4CXX_FATAL(module_logger, "Can't run instance " << reporter_instance << " 'cause the limit is " << max_inst);
        return retcode;
    }

    // init values
    sys_cpu();
    keep_alive_timeout = vm["keepalive_timeout"].as<int>();
    sys_info_timeout = vm["sysinfo_timeout"].as<int>();

    keep_alive_packet[ip_addr] = "";
    // find all ip's of machine
    if (gethostname(ac, sizeof(ac)) != SOCKET_ERROR) {
        struct hostent *phe = gethostbyname(ac);
        if (phe != 0) {
            for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
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
                    LOG4CXX_TRACE(module_logger, "Found ip addr: " << saddr);
                    keep_alive_packet[ip_addr] += saddr;
                    break;
                }
            }
        } else {
            LOG4CXX_ERROR(module_logger, "gethostbyname failed");
            keep_alive_packet[ip_addr] = "<unknown>";
        }
    } else {
        LOG4CXX_ERROR(module_logger, "gethostname failed");
        keep_alive_packet[ip_addr] = "<unknown>";
    }

    //keep_alive_packet[instance] =  boost::lexical_cast<string>(inst);
    keep_alive_packet[module_name] = vm["reporter_name"].as<string>();
    keep_alive_packet[keepalive_timeout] = boost::lexical_cast<string>(keep_alive_timeout);
    keep_alive_packet[status] = "READY";
    keep_alive_packet[module_version] = reporter_version;
    LOG4CXX_DEBUG(module_logger, "Register instance #" << reporter_instance);
    send_keepalive(storage, keep_alive_timeout);

    sys_info_packet[os_name] = sys_uname();
    sys_info_packet[sysinfo_timeout] = boost::lexical_cast<string>(sys_info_timeout);
    send_sysinfo(storage, sys_info_timeout);

    inLoop = true;
    keep_alive = btm::second_clock::local_time();
    sys_info = btm::second_clock::local_time();
    // run endless loop
    while(inLoop) {
        vector<string>  out;

        boost::this_thread::sleep(boost::posix_time::milliseconds(500)); // :seconds(1));
        curr_time = btm::second_clock::local_time();
        if ( (keep_alive + btm::seconds(keep_alive_timeout)) <= curr_time) {
            keep_alive = curr_time;
            send_keepalive(storage, keep_alive_timeout);
        }
        if ( (sys_info + btm::seconds(sys_info_timeout)) <= curr_time) {
            sys_info = curr_time;
            send_sysinfo(storage, sys_info_timeout);
        }
        // check commands
        string cmd;
        string args;
        int pos;
        cmd = get_command(storage);
        while (cmd != "") {
            LOG4CXX_INFO(module_logger, "DEBUG: Process command: " << cmd);
            // separate command from arguments
            pos = cmd.find(' ');
            if (pos == string::npos) {
                pos = cmd.find('\t');
            }
            if (pos != string::npos) {
                args = cmd.substr(pos + 1);
                cmd = cmd.substr(0, pos);
            } else {
                args = "";
            }
            boost::to_upper(cmd);

            // process commands
            if (cmd == "EXIT") {
                signal_halt(0);
                // need to exit and save other commands in the queue
                break;
            } else if (cmd == "RESTART") {
                // need to exit this copy and run new instance
                signal_halt(0);
                LOG4CXX_INFO(module_logger, "Restart request recieved");
                retcode = 1; // restart need
                break;
            } else if (cmd == "") {
                // other cmd's
            }
            cmd = get_command(storage);
        }
    }
    // finalize...
    webEngine::db_condition c_id;
    c_instance.field() = "modules.instance";
    c_instance.operation() = webEngine::db_condition::equal;
    c_instance.value() = reporter_instance;
    c_class.field() = "modules.class";
    c_class.operation() = webEngine::db_condition::equal;
    c_class.value() = MODULE_CLASS_REPORTER;
    c_id.field() = "modules.id";
    c_id.operation() = webEngine::db_condition::equal;
    c_id.value() = module_uuid;
    storage->del(c_instance && c_class && c_id);

    delete we_dispatcer;
    return retcode;
}
