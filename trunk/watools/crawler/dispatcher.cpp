#include "audit_comment.h"

#include <signal.h>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/foreach.hpp>
#ifdef WIN32
#include <errno.h>
#include <winsock2.h>
#include <process.h>
#include <conio.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif //WIN32

#include <log4cxx/logger.h>
#include <weDispatch.h>
#include <weUrl.h>
#include <weTask.h>
#include <weScan.h>
// for options:
#include <weHttpInvent.h>
// redefine audit_jscript options
#define weoAuditJSenable    "audit_jscript/enable_jscript"
#define weoAuditJSpreloads  "audit_jscript/preload"

#include "sysinfo.h"
#include "version.h"

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace std;

namespace fs = boost::filesystem;
namespace po = boost::program_options;
namespace bfs = boost::filesystem;
namespace btm = boost::posix_time;

class ScanRes : public webEngine::ScanInfo {
public:
    typedef scan_map::iterator iterator;

    int GetScanSize() {
        return scan_data.size();
    }
    iterator begin() {
        return scan_data.begin();
    }
    iterator end() {
        return scan_data.end();
    }

    boost::shared_ptr<webEngine::ScanData> GetData(int i) {
        iterator mit = scan_data.begin();
        boost::shared_ptr<webEngine::ScanData> result;

        if (i > scan_data.size()) {
            i = scan_data.size();
        }
        for (int j = 0; j < i; j++) {
            mit++;
        }
        if (mit != scan_data.end()) {
            result = mit->second;
        }
        return result;
    };
};

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
extern string db1_instance_name;

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

bool sort_results(const webEngine::db_record& left, const webEngine::db_record& right)
{
    return (left[weObjTypeScanData "." "object_url"] < right[weObjTypeScanData "." "object_url"]);
}

void save_results(int format, string fname, webEngine::task* tsk)
{
    if (tsk != NULL) {
        ofstream out;
        bool file = false;
        std::streambuf *old_buffer;
        if (fname != "") {
            try {
                out.open(fname.c_str(), ios_base::out | ios_base::trunc);
                old_buffer = std::cout.rdbuf(out.rdbuf());
                file = true;
            } catch (std::exception &e) {
				LOG4CXX_ERROR(scan_logger, "Can't redirect output to file " << fname << ": " << std::string(e.what()));
                file = false;
            }
        }
        cout << endl;
        cout << "Scan start:    " << tsk->start_time() << endl;
        cout << "Scan finish:   " << tsk->finish_time() << endl;
        btm::time_period scan_elapsed(tsk->start_time(), tsk->finish_time());
        cout << "Scan duration: " << btm::to_simple_string(scan_elapsed.length()) << endl;
        cout << "Scan size:     " << tsk->get_scan_size() << endl;
        if (tsk->total_requests() > 0) {
            double rps = (double)tsk->total_requests()  / scan_elapsed.length().total_seconds();
            cout.setf(ios::fixed,ios::floatfield);
            cout.precision(3);
            cout << "Scan speed:    " << tsk->total_requests()  << " requests; " << rps << " requests per second" << endl;
            cout.precision(3);
        }
        cout << endl;

        if (format == 2) {
            cout << "URL\tHTTP code\tDownload time\tSize" << endl;
        }

        for( webEngine::db_cursor i = tsk->get_scan(); i.is_not_end(); ++i ) {
            try {
                if (format == 0) {
                    cout << "Requested:     " << i[weObjTypeScanData "." "object_url"] << endl;
                    cout << "Response:      " << i[weObjTypeScanData "." "resp_code"] << endl;
                    cout << "Data size:     " << i[weObjTypeScanData "." "data_size"] << endl;
                    cout << "Download time: " << i[weObjTypeScanData "." "dnld_time"] << endl;
                    cout << "Tree level:    " << i[weObjTypeScanData "." "scan_depth"] << endl;
                    cout << endl;
                } else if (format == 1) {
                    cout << i[weObjTypeScanData "." "object_url"] << endl;
                } else if (format == 2) {
                    cout << i[weObjTypeScanData "." "object_url"] << "\t" << i[weObjTypeScanData "." "resp_code"] << "\t" <<
                         i[weObjTypeScanData "." "dnld_time"] << "\t" << i[weObjTypeScanData "." "data_size"] << endl;
                }
            } catch (out_of_range) {}
        }
    }
}

static void* create_audit_comments(void* krnl, void* handle = NULL)
{
    return (void*) (new audit_comment((webEngine::engine_dispatcher*)krnl, handle));
}

void save_plugin_ui(webEngine::i_storage* store)
{
    LOG4CXX_TRACE(scan_logger, "Save UI for plugins");
    int pos;
    string db_key;
    string db_data;
    string db_icon;
    webEngine::string_list str_data;
    webEngine::db_filter c_query;
    webEngine::db_condition c_plugin, c_locale;

    c_locale.field() = "profile_ui.locale";
    c_locale.operation() = webEngine::db_condition::equal;
    c_locale.value() = string("en");

    c_plugin.field() = "profile_ui.plugin_id";
    c_plugin.operation() = webEngine::db_condition::equal;

    webEngine::plugin_list plgs = we_dispatcer->get_plugin_list();
    for (unsigned int i = 0; i < plgs.size(); i++) {
        webEngine::string_list::iterator it;
        it = find(plgs[i].interface_list.begin(), plgs[i].interface_list.end(), "i_storage");
        if (it == plgs[i].interface_list.end()) {
            // not a storage plugin
            // get plugin icon
            db_icon = "";
            for (unsigned int j = 0; j < plgs[i].plugin_icon.size(); j++) {
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
                c_query.set(c_plugin)._and_(c_locale);
                webEngine::db_cursor rec = store->set(c_query, "profile_ui");
                rec["profile_ui.plugin_id"] = plgs[i].interface_name;
                rec["profile_ui.plugin_name"] = plgs[i].plugin_desc;
                rec["profile_ui.ui_settings"] = db_data;
                rec["profile_ui.ui_icon"] = db_icon;
                rec["profile_ui.locale"] = string("en");
                rec.close();
            } else {
                LOG4CXX_ERROR(scan_logger, "Can't load plugin " << plgs[i].plugin_id << "; " << plgs[i].plugin_desc);
            }
        } // if not a storage plugin
    } // foreach plugins
}

void dispatcher_routine(po::variables_map& vm)
{
    btm::ptime  keep_alive;
    btm::ptime  curr_time;
    int         keep_alive_timeout;
    char        ac[80] = {0};
    string      queue_key;

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
    we_dispatcer->add_plugin_class("D00E80357E27", create_audit_comments);
    we_dispatcer->add_plugin_class("audit_comment", create_audit_comments);  // copy for interface name

    webEngine::i_plugin* plg = we_dispatcer->load_plugin(vm["db_interface"].as<string>());
    if (plg == NULL) {
        LOG4CXX_FATAL(scan_logger, "Can't load plug-in for Storage DB connection: " << vm["db_interface"].as<string>());
        return;
    }
    LOG4CXX_INFO(scan_logger, "Storage plugin loaded successfully: " << vm["db_interface"].as<string>());
    LOG4CXX_DEBUG(scan_logger, "Plugin ID=" << plg->get_id() << "; Description: " << plg->get_description());
    webEngine::i_storage* storage = (webEngine::i_storage*)plg->get_interface("i_storage");
    if (plg == NULL) {
        LOG4CXX_FATAL(scan_logger, "No iStorage interface in the plugin " << plg->get_id() << "(ID="  << plg->get_id() );
        LOG4CXX_FATAL(scan_logger, "Can't load plug-in for Storage DB connection: " << vm["db_interface"].as<string>());
        return;
    }
    // no need to keep plugin - it can be removed by the dispatcher
    storage->release();

    string params = "";
    if (vm.count("db_parameters")) {
        params = vm["db_parameters"].as<string>();
    }
    storage->init_storage(params);
    we_dispatcer->storage(storage);
    LOG4CXX_INFO(scan_logger, "Storage initialised");

    LOG4CXX_INFO(scan_logger, "Operating System: " << sys_uname());
    LOG4CXX_INFO(scan_logger, "Memory Information: " << sys_meminfo());

    // update plugins information
    save_plugin_ui(storage);

    // if the init_db option given - we don't need to do anything
    if (vm.count("init_db")) {
        return;
    }

    webEngine::plugin_list plgs = we_dispatcer->get_plugin_list();
    vector<webEngine::i_plugin*> scan_plugins;
    for (size_t i = 0; i < plgs.size(); i++) {
        string info = plgs[i].interface_name + "|";
        info += plgs[i].interface_list[1] + "|";
        info += plgs[i].plugin_id + "|";
        info += plgs[i].plugin_desc;
        plg = we_dispatcer->load_plugin(plgs[i].plugin_id);
        scan_plugins.push_back(plg->get_interface("i_plugin")); //(webEngine::i_plugin*)
        LOG4CXX_TRACE(scan_logger, "Save info [" << i << "]: " << info);

    }

    // init values
    sys_cpu();
    queue_key = "ScanModule:Queue:" + scaner_uuid + ":" + boost::lexical_cast<string>(scaner_instance);
    keep_alive_timeout = vm["keepalive_timeout"].as<int>();

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

    inLoop = true;
    keep_alive = btm::second_clock::local_time();

    // create task executor
    webEngine::task* tsk = new webEngine::task(we_dispatcer);

    if (vm.count("profile")) {
        int ival = vm["profile"].as<int>();
        string sval = boost::lexical_cast<string>(ival);
        // load profile
        tsk->set_profile_id(sval);
        // set-up plugins usage
        scan_plugins.clear();

        webEngine::we_option opt = tsk->Option("plugin_list");
        SAFE_GET_OPTION_VAL(opt, sval, "");
        if (sval != "") {
            vector<string> plgs;
            boost::split(plgs, sval, boost::is_any_of(";"));
            for (size_t i = 0; i < plgs.size(); ++i) {
                if (plgs[i] != "") {
                    plg = we_dispatcer->load_plugin(plgs[i]);
                    if (plg) {
                        scan_plugins.push_back(plg->get_interface("i_plugin"));
                    } // if plugin loaded
                } // if name given
            } // foreach plugin
        } else {
            LOG4CXX_ERROR(scan_logger, "No plugins attached to profile " << ival << ". Nothing to do, exit.");
            return;
        }
    } else {
        // initialize task from configuration
        tsk->set_profile_id(string("0")); // todo - take it from params
        // set scanning options
        bool is_jscript = true;
        if (vm.count("jscript")) {
            try {
                is_jscript = vm["jscript"].as<bool>();
            } catch (...) { } // just skip
        }
        if (vm.count("no-jscript")) {
            is_jscript = false;
        }
        tsk->Option(weoAuditJSenable, is_jscript);
        LOG4CXX_DEBUG(scan_logger, "Set " << string(weoAuditJSenable) << " to " << is_jscript);

        if (vm.count("depth")) {
            int val = vm["depth"].as<int>();
            tsk->Option(weoScanDepth, val);
            LOG4CXX_DEBUG(scan_logger, "Set " << string(weoScanDepth) << " to " << val);
        }
        if (vm.count("dir")) {
            bool val = true;
            try {
                val = vm["dir"].as<bool>();
            } catch (...) { } // just skip
            tsk->Option(weoStayInDir, val);
            LOG4CXX_DEBUG(scan_logger, "Set " << string(weoStayInDir) << " to " << val);
        }
        if (vm.count("host")) {
            bool val = true;
            try {
                val = vm["host"].as<bool>();
            } catch (...) { } // just skip
            tsk->Option(weoStayInHost, val);
            LOG4CXX_DEBUG(scan_logger, "Set " << string(weoStayInHost) << " to " << val);
        }
        if (vm.count("domain")) {
            bool val = true;
            try {
                val = vm["domain"].as<bool>();
            } catch (...) { } // just skip
            tsk->Option(weoStayInDomain, val);
            LOG4CXX_DEBUG(scan_logger, "Set " << string(weoStayInDomain) << " to " << val);
        }
        if (vm.count("dlist")) {
            bool val = true;
            try {
                val = vm["dlist"].as<bool>();
            } catch (...) { } // just skip
            tsk->Option(weoStayInDomainList, val);
            LOG4CXX_DEBUG(scan_logger, "Set " << string(weoStayInDomainList) << " to " << val);
        }
        if (vm.count("ip")) {
            bool val = true;
            try {
                val = vm["ip"].as<bool>();
            } catch (...) { } // just skip
            tsk->Option(weoStayInIP, val);
            LOG4CXX_DEBUG(scan_logger, "Set " << string(weoStayInIP) << " to " << val);
        }
        if (vm.count("url_param")) {
            bool val = true;
            try {
                val = vm["url_param"].as<bool>();
            } catch (...) { } // just skip
            tsk->Option(weoIgnoreUrlParam, val);
            LOG4CXX_DEBUG(scan_logger, L"Set " << string(weoIgnoreUrlParam) << " to " << val);
        }
        if (vm.count("content")) {
            int val = vm["content"].as<int>();
            tsk->Option(weoAllowedCTypes, val);
            LOG4CXX_DEBUG(scan_logger, "Set " << string(weoAllowedCTypes) << " to " << val);
        }
        if (vm.count("parallel")) {
            int val = vm["parallel"].as<int>();
            tsk->Option(weoParallelReq, val);
            LOG4CXX_DEBUG(scan_logger, "Set " << string(weoParallelReq) << " to " << val);
        }
        if (vm.count("ext_deny")) {
            string val = vm["ext_deny"].as<string>();
            tsk->Option(weoDeniedFileTypes, val);
            LOG4CXX_DEBUG(scan_logger, "Set " << string(weoDeniedFileTypes) << " to " << val);
        }
    }

    tsk->store_plugins(scan_plugins);
    // reconstruct URL to scan
    string url = vm["target"].as<string>();
    if (! boost::algorithm::starts_with(url, "http://")) {
        url = "http://" + url;
    }
    if (vm.count("name")) {
        string val = vm["name"].as<string>();
        tsk->set_name(val);
    } else {
        tsk->set_name("Crawling " + url);
    }
    webEngine::transport_url t_url(url);
    LOG4CXX_INFO(scan_logger, "Start scanning for: " << t_url.tostring());
    tsk->Option(weoScanHost, t_url.host);
    tsk->Option(weoBaseURL,  url);

    // run endless loop for the task
    //tsk->Run("1613");
    tsk->Run();

    cout << "Running task (scan_id: \"" << tsk->get_scan_id() << "\") starts from \"" << t_url.tostring() << "\"" << endl;

    // init scan-time vars
    inLoop = true;
    char ch;
    size_t sz, dn;
    double rps;
    string fname = "";
    if (vm.count("result")) {
        fname = vm["result"].as<string>();
    }
    int format = vm["output"].as<int>();

    btm::ptime scan_start = btm::second_clock::local_time();
    btm::ptime scan_curr;
    int kp_tm = 0;
    cout << "Scanning... 0 objects found... 0% complete\r";
    do {
        if (!tsk->IsReady()) {
            inLoop = false;
        }
        boost::this_thread::sleep(boost::posix_time::seconds(1));
        int compl = tsk->completion();
        scan_curr = btm::second_clock::local_time();
        btm::time_period scan_elapsed(scan_start, scan_curr);
        cout << "Scanning for " << btm::to_simple_string(scan_elapsed.length()) << "... " << tsk->get_scan_size() << " objects found... " << compl << "% complete; "
#ifdef _DEBUG
             << "tL: " << tsk->get_taskList_size() << "; tQ: " << tsk->get_taskQueue_size() << "; st: " << tsk->get_status_name()
             << "; pT: " << (tsk->get_processThread() ? "r" : "s")
#endif
             << "   \n";
        LOG4CXX_TRACE(scan_logger, "Memory information: " << sys_meminfo());
        if (kbhit()) {
            ch = getch();
            ch = toupper(ch);
            switch (ch) {
            case 'S':
                sz = tsk->total_requests();
                dn = tsk->total_processed();
                rps = (double)dn / scan_elapsed.length().total_seconds();
                cout.setf(ios::fixed,ios::floatfield);
                cout.precision(3);
                cout << endl << "Total requests: " << sz << "; processed: " << dn << " (" << rps << " requests per second)" << endl;
                break;
            case 'P':
                cout << endl << "Pausing... ";
                tsk->Pause();
                cout << endl << "Done" << endl;
                break;
            case 'U':
                cout << endl << "Suspending... ";
                tsk->Suspend();
                cout << endl << "Done" << endl;
                break;
            case 'R':
                cout << endl << "Resume..." << endl;
                tsk->Resume();
                break;
            case 'Q':
                cout << endl << "Stopping..." << endl;
                tsk->Stop();
                cout << endl << "Done" << endl;
                inLoop = false;
                break;
            case ' ':
                if (fname == "") {
                    cout << endl << "Can't save intermediate information - no results file given" << endl;
                } else {
                    cout << endl << "Saving intermediate information ...";
                    //save_results(format, fname, tsk);
                    cout << " done!" << endl;
                }
            }
        }
        //tsk->calc_status();
        kp_tm++;
        if (kp_tm > keep_alive_timeout) {
            tsk->save_to_db();
            if (we_dispatcer->storage() != NULL) {
                we_dispatcer->storage()->flush();
            }
        }
    } while(inLoop);

    while (tsk->IsReady()) {
        int compl = tsk->completion();
        scan_curr = btm::second_clock::local_time();
        btm::time_period scan_elapsed(scan_start, scan_curr);
        cout << "Scanning for " << btm::to_simple_string(scan_elapsed.length()) << "... " << tsk->get_scan_size() << " objects found... " << compl << "% complete\r";
        boost::this_thread::sleep(boost::posix_time::seconds(1));
    }
    tsk->save_to_db();

    webEngine::db_filter flt;
    webEngine::db_condition tc;//("scan_data.task_id == id");
    webEngine::db_condition lc;//("scan_data.resp_code >= 400");
    webEngine::db_condition gc;//("scan_data.resp_code < 500");
    cout << "Remove errors from results... ";
    tc.field() = "scan_data.task_id";
    tc.operation() = webEngine::db_condition::equal;
    tc.value() = tsk->get_scan_id();

    lc.field() = "scan_data.resp_code";
    lc.operation() = webEngine::db_condition::great_or_equal;
    lc.value() = 400;

    gc.field() = "scan_data.resp_code";
    gc.operation() = webEngine::db_condition::less;
    gc.value() = 500;

    flt.set(tc)._and_(lc)._and_(gc);
    if (we_dispatcer->storage() != NULL) {
        int r = we_dispatcer->storage()->del(flt);
        cout << r << " records ";
    }
    cout << "done" << endl;

    cout << "Remove redirects from results... ";

    lc.field() = "scan_data.resp_code";
    lc.operation() = webEngine::db_condition::great_or_equal;
    lc.value() = 300;

    gc.field() = "scan_data.resp_code";
    gc.operation() = webEngine::db_condition::less;
    gc.value() = 400;

    flt.set(tc)._and_(lc)._and_(gc);
    if (we_dispatcer->storage() != NULL) {
        int r = we_dispatcer->storage()->del(flt);
        cout << r << " records ";
    }
    cout << "done" << endl;

    if (we_dispatcer->storage() != NULL) {
        vector<string> fupd;
        fupd.push_back("task.processed_urls");
        webEngine::db_filter query;
        tc.field() = "task.id";
        tc.operation() = webEngine::db_condition::equal;
        tc.value() = tsk->get_scan_id();
        query.set(tc);
        webEngine::db_cursor rec = we_dispatcer->storage()->set(query, fupd);
        rec[0] = string("");

        cout << "Remove URL's usage... ";
        rec.close();
        cout << rec.get_affected_rows() << " records ";
    }
    cout << "done" << endl;

    // print result
    if (we_dispatcer->storage() != NULL) {
        we_dispatcer->storage()->flush();
    }
    fname = "";
    if (vm.count("result")) {
        fname = vm["result"].as<string>();
    }
    format = vm["output"].as<int>();
    save_results(format, fname, tsk);

    for (size_t i = 0; i < scan_plugins.size(); i++) {
        scan_plugins[i]->release();
    }
    // finalize...
    delete tsk;

    delete we_dispatcer;

}
