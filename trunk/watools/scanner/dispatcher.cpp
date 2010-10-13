#include <signal.h>
#include <boost/algorithm/string/predicate.hpp>
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

#define MODULE_CLASS_SCANNER    0

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
extern string scaner_uuid;
extern string scaner_version;
extern int scaner_instance;

void signal_halt(int sig)
{
    inLoop = false;
    //! todo: pause all tasks
    if (sig == 0) {
        LOG4CXX_INFO(scan_logger, "Received software request to exit");
    } else {
        LOG4CXX_INFO(scan_logger, "Signal received - exit scanner");
    }
}

void save_plugin_ui(webEngine::i_storage* store)
{
    LOG4CXX_TRACE(scan_logger, "Save UI for plugins");
    int pos;
    string db_key;
    string db_data;
    string db_icon;
    webEngine::string_list str_data;
    webEngine::db_recordset packet(store->get_namespace_struct("profile_ui"));
    webEngine::db_cursor rec = packet.push_back();
    webEngine::db_query c_query;
    webEngine::db_condition c_plugin, c_locale;

    c_locale.field() = "profile_ui.locale";
    c_locale.operation() = webEngine::db_condition::equal;
    c_locale.value() = string("en");

    c_plugin.field() = "profile_ui.plugin_id";
    c_plugin.operation() = webEngine::db_condition::equal;

    rec["profile_ui.locale"] = string("en");

//     pos = args.find(' ');
//     if (pos == string::npos) {
//         pos = args.find('\t');
//     }
//     if (pos != string::npos) {
//         args = args.substr(0, pos);
//     }
    webEngine::plugin_list plgs = we_dispatcer->get_plugin_list();
    for (int i = 0; i < plgs.size(); i++) {
        webEngine::string_list::iterator it;
        it = find(plgs[i].interface_list.begin(), plgs[i].interface_list.end(), "i_storage");
        if (it == plgs[i].interface_list.end()) {
            // not a storage plugin
            // get plugin icon
            db_icon = "";
            for (int j = 0; j < plgs[i].plugin_icon.size(); j++) {
                db_icon += plgs[i].plugin_icon[j];
                db_icon += "\n";
            }
            webEngine::i_plugin* plg = we_dispatcer->load_plugin(plgs[i].plugin_id);
            if (plg != NULL) {
                // get UI
                db_data = plg->get_setup_ui();
                plg->release();
                // save information
                c_plugin.value() = plgs[i].interface_name;
                c_query.where.set(c_plugin).and(c_locale);
                rec["profile_ui.plugin_id"] = plgs[i].interface_name;
                rec["profile_ui.plugin_name"] = plgs[i].plugin_desc;
                rec["profile_ui.ui_settings"] = db_data;
                rec["profile_ui.ui_icon"] = db_icon;
                store->set(c_query, packet);
            } else {
                LOG4CXX_ERROR(scan_logger, "Can't load plugin " << plgs[i].plugin_id << "; " << plgs[i].plugin_desc);
            }
        } // if not a storage plugin
    } // foreach plugins
}

void send_keepalive(webEngine::i_storage* store, int timeout)
{
    LOG4CXX_TRACE(scan_logger, "Send keep-alive signal");
    keep_alive_packet[running_task] = boost::lexical_cast<string>(running_tasks_count);
    webEngine::db_recordset packet(store->get_namespace_struct("modules"));
    webEngine::db_cursor rec = packet.push_back();
    rec["modules.id"] = scaner_uuid;
    rec["modules.instance"] = scaner_instance;
    rec["modules.class"] = MODULE_CLASS_SCANNER;
    rec["modules.version"] = keep_alive_packet[scan_version];
    rec["modules.ipaddr"] = keep_alive_packet[ip_addr];
    rec["modules.name"] = keep_alive_packet[scanner_name];
    rec["modules.stamp"] = (int)time(NULL);
    rec["modules.timeout"] = timeout;
    rec["modules.onrun"] = running_tasks_count;
    rec["modules.status"] = keep_alive_packet[status];

    webEngine::db_query scan_query;
    webEngine::db_condition c_instance, c_class, c_id;
    c_instance.field() = "modules.instance";
    c_instance.operation() = webEngine::db_condition::equal;
    c_instance.value() = scaner_instance;
    c_class.field() = "modules.class";
    c_class.operation() = webEngine::db_condition::equal;
    c_class.value() = MODULE_CLASS_SCANNER;
    c_id.field() = "modules.id";
    c_id.operation() = webEngine::db_condition::equal;
    c_id.value() = scaner_uuid;
    scan_query.where.set(c_instance).and(c_class).and(c_id);
    store->set(scan_query, packet);
}

void send_plugins_list(int timeout)
{
    LOG4CXX_TRACE(scan_logger, "Save plugins list");
    string db_key = "ScanModule:PlugIns:" + scaner_uuid + ":" + boost::lexical_cast<string>(scaner_instance);
    webEngine::plugin_list plgs = we_dispatcer->get_plugin_list();
    for (int i = 0; i < plgs.size(); i++) {
        string info = plgs[i].interface_name + "|";
        info += plgs[i].interface_list[1] + "|";
        info += plgs[i].plugin_id + "|";
        info += plgs[i].plugin_desc;
        LOG4CXX_TRACE(scan_logger, "Save info [" << i << "]: " << info);
        // save info
    }
}

void send_sysinfo(webEngine::i_storage* store, int timeout)
{
    LOG4CXX_TRACE(scan_logger, "Send system information");
    // not divide to instances - whole system information
    webEngine::db_recordset packet(store->get_namespace_struct("modules_info"));
    webEngine::db_cursor rec = packet.push_back();

    rec["modules_info.module_id"] = scaner_uuid;
    rec["modules_info.osname"] = sys_info_packet[os_name];
    LOG4CXX_TRACE(scan_logger, "Get memory information");
    sys_info_packet[memory_size] = sys_meminfo();
    rec["modules_info.mem_size"] = sys_info_packet[memory_size];
    LOG4CXX_TRACE(scan_logger, "Get CPU information");
    sys_info_packet[cpu_usage] = sys_cpu();
    rec["modules_info.cpu_usage"] = sys_info_packet[cpu_usage];
    LOG4CXX_TRACE(scan_logger, "Get disk information");
    sys_info_packet[disk_size] = sys_disk();
    rec["modules_info.disk_size"] = sys_info_packet[disk_size];
    sys_info_packet[max_tasks] = boost::lexical_cast<string>(max_tasks_count);
    rec["modules_info.max_tasks"] = max_tasks_count;
    rec["modules_info.stamp"] = (int)time(NULL);
    rec["modules_info.timeout"] = timeout;
    // save info

    // save to DB1 information about plugins.
    webEngine::db_query scan_query;
    webEngine::db_condition c_instance;
    c_instance.field() = "modules_info.module_id";
    c_instance.operation() = webEngine::db_condition::equal;
    c_instance.value() = scaner_uuid;
    scan_query.where.set(c_instance);
    store->set(scan_query, packet);

    //send_plugins_list(timeout);
}

string get_command(webEngine::i_storage* store)
{
    string retval = "";
    webEngine::db_recordset packet;

    webEngine::db_query scan_query;
    webEngine::db_condition c_instance, c_stamp;

    c_instance.field() = "module_cmds.module_id";
    c_instance.operation() = webEngine::db_condition::equal;
    c_instance.value() = scaner_uuid + ":" +  boost::lexical_cast<string>(scaner_instance);
    scan_query.what = store->get_namespace_struct("module_cmds");
    scan_query.where.set(c_instance);
    store->get(scan_query, packet);
    if (packet.size() > 0) {
        webEngine::db_cursor rec = packet.begin();
        retval = rec["module_cmds.cmd"].get<string>();

        c_stamp.field() = "module_cmds.timestamp";
        c_stamp.operation() = webEngine::db_condition::equal;
        c_stamp.value() = rec["module_cmds.timestamp"];
        scan_query.where.set(c_instance).and(c_stamp);
        store->del(scan_query.where);
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
        LOG4CXX_FATAL(scan_logger, "Can't create webEngine dispatcher - exit!");
        return retcode;
    }
    // refresh plugins information
    bfs::path plg_path = vm["plugin_dir"].as<string>();
    we_dispatcer->refresh_plugin_list(plg_path);
    webEngine::i_plugin* plg = we_dispatcer->load_plugin(vm["db_interface"].as<string>());
    if (plg == NULL) {
        LOG4CXX_FATAL(scan_logger, "Can't load plug-in for Storage DB connection: " << vm["db_interface"].as<string>());
        return retcode;
    }
    LOG4CXX_INFO(scan_logger, "Storage plugin loaded successfully: " << vm["db_interface"].as<string>());
    LOG4CXX_DEBUG(scan_logger, "Plugin ID=" << plg->get_id() << "; Description: " << plg->get_description());
    webEngine::i_storage* storage = (webEngine::i_storage*)plg->get_interface("i_storage");
    if (plg == NULL) {
        LOG4CXX_FATAL(scan_logger, "No iStorage interface in the plugin " << plg->get_id() << "(ID="  << plg->get_id() );
        LOG4CXX_FATAL(scan_logger, "Can't load plug-in for Storage DB connection: " << vm["db_interface"].as<string>());
        return retcode;
    }
    string params = "";
    if (vm.count("db_parameters")) {
        params = vm["db_parameters"].as<string>();
    }
    storage->init_storage(params);
    we_dispatcer->storage(storage);
    LOG4CXX_INFO(scan_logger, "Storage initialised");

    // clear outdated records
    // verify instance id
    webEngine::db_recordset scanners;
    webEngine::db_query scan_query;
    webEngine::db_condition c_instance, c_class;
    scan_query.what = storage->get_namespace_struct("modules");
    c_instance.field() = "modules.instance";
    c_instance.operation() = webEngine::db_condition::equal;
    c_instance.value() = scaner_uuid;
    c_class.field() = "modules.class";
    c_class.operation() = webEngine::db_condition::equal;
    c_class.value() = MODULE_CLASS_SCANNER;
    scan_query.where.set(c_instance).and(c_class);
    storage->get(scan_query, scanners);
    scaner_instance = scanners.size();
    if (scaner_instance > 0) {
        // fix holes
        webEngine::db_cursor cur;
        for (cur = scanners.begin(); cur != scanners.end(); ++cur) {
            if (cur["modules.id"].get<int>() > scaner_instance) {
                scaner_instance = cur["modules.id"].get<int>();
            }
        }
    }
    LOG4CXX_TRACE(scan_logger, "Found " << scaner_instance << " instances of " << scaner_uuid);
    scaner_instance++;
    int max_inst = vm["instances"].as<int>();
    if (scaner_instance > max_inst) {
        LOG4CXX_FATAL(scan_logger, "Can't run instance " << scaner_instance << " 'cause the limit is " << max_inst);
        return retcode;
    }

    // init values
    sys_cpu();
    queue_key = "ScanModule:Queue:" + scaner_uuid + ":" + boost::lexical_cast<string>(scaner_instance);
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
                    LOG4CXX_TRACE(scan_logger, "Found ip addr: " << saddr);
                    keep_alive_packet[ip_addr] += saddr;
                    break;
                }
            }
        } else {
            LOG4CXX_ERROR(scan_logger, "gethostbyname failed");
            keep_alive_packet[ip_addr] = "<unknown>";
        }
    } else {
        LOG4CXX_ERROR(scan_logger, "gethostname failed");
        keep_alive_packet[ip_addr] = "<unknown>";
    }

    //keep_alive_packet[instance] =  boost::lexical_cast<string>(inst);
    keep_alive_packet[scanner_name] = vm["scanner_name"].as<string>();
    keep_alive_packet[keepalive_timeout] = boost::lexical_cast<string>(keep_alive_timeout);
    keep_alive_packet[status] = "READY";
    keep_alive_packet[scan_version] = scaner_version;
    LOG4CXX_DEBUG(scan_logger, "Register instance #" << scaner_instance);
    send_keepalive(storage, keep_alive_timeout);

    sys_info_packet[os_name] = sys_uname();
    sys_info_packet[sysinfo_timeout] = boost::lexical_cast<string>(sys_info_timeout);
    send_sysinfo(storage, sys_info_timeout);

    save_plugin_ui(storage);
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
            LOG4CXX_INFO(scan_logger, "DEBUG: Process command: " << cmd);
            // separate command from argumenets
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
                LOG4CXX_INFO(scan_logger, "Restart request recieved");
                retcode = 1; // restart need
                break;
            } else if (cmd == "PLUGINS") {
                // refresh plugins list
                send_plugins_list(sys_info_timeout);
            } else if (cmd == "SAVE_PLUGIN_UI") {
                // refresh plugins list
                save_plugin_ui(storage);
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
    c_instance.value() = scaner_instance;
    c_class.field() = "modules.class";
    c_class.operation() = webEngine::db_condition::equal;
    c_class.value() = MODULE_CLASS_SCANNER;
    c_id.field() = "modules.id";
    c_id.operation() = webEngine::db_condition::equal;
    c_id.value() = scaner_uuid;
    scan_query.where.set(c_instance).and(c_class).and(c_id);
    storage->del(scan_query.where);

    delete we_dispatcer;
    return retcode;
}
