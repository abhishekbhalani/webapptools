#include "weJsExecutor.h"
#include "audit_jscript.h"
#include "audit_comment.h"

#include <signal.h>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
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

#include "../common/sysinfo.h"
#include "version.h"

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace std;

namespace fs = boost::filesystem;
namespace po = boost::program_options;
namespace bfs = boost::filesystem;
namespace btm = boost::posix_time;

class ScanRes : public webEngine::ScanInfo
{
public:
    int GetScanSize(){ return scan_data.size(); };
    boost::shared_ptr<webEngine::ScanData> GetData(int i) {
        boost::unordered_map<string, boost::shared_ptr<webEngine::ScanData> >::iterator mit = scan_data.begin();
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
webEngine::jsExecutor* jsExec;

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
    }
    else {
        LOG4CXX_INFO(scan_logger, "Signal received - exit scanner");
    }
}

void save_results(ScanRes *si, int format, string fname)
{
    if (si != NULL)
    {
        ofstream out;
        bool file = false;
        std::streambuf *old_buffer;
        if (fname != "") {
            try{
                out.open(fname.c_str(), ios_base::out | ios_base::trunc);
                old_buffer = std::cout.rdbuf(out.rdbuf());
                file = true;
            }
            catch (std::exception &e) {
                LOG4CXX_ERROR(scan_logger, "Can't redirect output to file " << fname << ": " << e.what());
                file = false;
            }
        }
        cout << endl;
        cout << "Scan start:    " << si->startTime << endl;
        cout << "Scan finish:   " << si->finishTime << endl;
        btm::time_period scan_elapsed(si->startTime, si->finishTime);
        cout << "Scan duration: " << btm::to_simple_string(scan_elapsed.length()) << endl;
        cout << "Scan size:     " << si->GetScanSize() << endl << endl;
        for (int i = 0; i < si->GetScanSize(); i++) {
            boost::shared_ptr<webEngine::ScanData> dt = si->GetData(i);
            if (dt) {
                if (format == 0) {
                    cout << "Requested:     " << dt->object_url << endl;
                    cout << "Response:      " << dt->resp_code << endl;
                    cout << "Data size:     " << dt->data_size << endl;
                    cout << "Download time: " << dt->download_time << endl;
                    cout << "Tree level:    " << dt->scan_depth << endl;
                    cout << endl;
                }
                else if (format == 1) {
                    cout << dt->object_url << endl;
                }
                else if (format == 2) {
                    cout << dt->object_url << " Download state: " << dt->resp_code << " ( " <<
                        dt->download_time << " msec.) / " << dt->data_size << " bytes" << endl;
                }
            } // if result valid
        } // for results
        if(file) {
            std::cout.rdbuf(old_buffer);
        }
    }
}

static void* create_audit_comments(void* krnl, void* handle = NULL)
{
    return (void*) (new audit_comment((webEngine::engine_dispatcher*)krnl, handle));
}

static void* create_audit_jscript(void* krnl, void* handle = NULL)
{
    return (void*) (new audit_jscript((webEngine::engine_dispatcher*)krnl, handle));
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
    we_dispatcer->add_plugin_class("9251BAB1B2C8", create_audit_jscript);
    we_dispatcer->add_plugin_class("audit_jscript", create_audit_jscript);  // copy for interface name

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
    // no need to keep plugin - it can be removed by the dispatcher 
    storage->release();

    string params = "";
    if (vm.count("db2_parameters")) {
        params = vm["db2_parameters"].as<string>();
    }
    storage->init_storage(params);
    we_dispatcer->storage(storage);
    LOG4CXX_INFO(scan_logger, "Storage initialised");

    LOG4CXX_INFO(scan_logger, "Operating System: " << sys_uname());
    LOG4CXX_INFO(scan_logger, "Memory Information: " << sys_meminfo());

    webEngine::plugin_list plgs = we_dispatcer->get_plugin_list();
    vector<webEngine::i_plugin*> scan_plugins;
    for (size_t i = 0; i < plgs.size(); i++)
    {
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

    inLoop = true;
    keep_alive = btm::second_clock::local_time();
    sys_info = btm::second_clock::local_time();

    bool is_jscript = true;
    if (vm.count("jscript"))
    {
        try {
            is_jscript = vm["jscript"].as<bool>();
        } catch (...) { } // just skip
    }
    if (vm.count("no-jscript")) {
        is_jscript = false;
    }
    if (is_jscript) {
        LOG4CXX_DEBUG(scan_logger, "Create JavaScript processor");
        jsExec = new webEngine::jsExecutor;
        jsExec->execute_string("print('V8 Engine version is: ' + version())", "", true, true);
        if (vm.count("js_preload")) {
            string fname = vm["js_preload"].as<string>();
            char* buff = NULL;
            LOG4CXX_DEBUG(scan_logger, "Execute JavaScript preloads from " << fname);
            try{
                size_t fsize = fs::file_size(fs::path(fname));
                ifstream ifs(fname.c_str());
                buff = new char[fsize + 10];
                if(buff != NULL)
                {
                    memset(buff, 0, fsize+10);
                    ifs.read(buff, fsize);
                    jsExec->execute_string(buff, "", true, true);
                    delete buff;
#ifdef _DEBUG
                    LOG4CXX_TRACE(scan_logger, "JavaScript preloads execution result: " << jsExec->get_results());
                    LOG4CXX_TRACE(scan_logger, "JavaScript preloads execution result (dump): " << jsExec->dump("Object"));
#endif // _DEBUG
                }
            }
            catch(std::exception& e)
            {
                LOG4CXX_WARN(scan_logger, "JavaScript preloads execution failed " << e.what());
                if (buff != NULL)
                {
                    delete buff;
                }
            }
        }
        audit_jscript::js_exec = jsExec;
    }
    else {
        jsExec = NULL;
        LOG4CXX_DEBUG(scan_logger, "JavaScript will not used");
    }

    // set scanning options
    if (vm.count("depth")) {
        int val = vm["depth"].as<int>();
        we_dispatcer->Option("httpInventory/"weoScanDepth, val);
        LOG4CXX_DEBUG(scan_logger, "Set httpInventory/"weoScanDepth" to " << val);
    }
    if (vm.count("dir")) {
        bool val = true;
        try {
            val = vm["dir"].as<bool>();
        } catch (...) { } // just skip
        we_dispatcer->Option("httpInventory/"weoStayInDir, val);
        LOG4CXX_DEBUG(scan_logger, "Set httpInventory/"weoStayInDir" to " << val);
    }
    if (vm.count("host")) {
        bool val = true;
        try {
            val = vm["host"].as<bool>();
        } catch (...) { } // just skip
        we_dispatcer->Option("httpInventory/"weoStayInHost, val);
        LOG4CXX_DEBUG(scan_logger, "Set httpInventory/"weoStayInHost" to " << val);
    }
    if (vm.count("domain")) {
        bool val = true;
        try {
            val = vm["domain"].as<bool>();
        } catch (...) { } // just skip
        we_dispatcer->Option("httpInventory/"weoStayInDomain, val);
        LOG4CXX_DEBUG(scan_logger, "Set httpInventory/"weoStayInDomain" to " << val);
    }
    if (vm.count("dlist")) {
        bool val = true;
        try {
            val = vm["dlist"].as<bool>();
        } catch (...) { } // just skip
        we_dispatcer->Option("httpInventory/StayInDomainList", val);
        LOG4CXX_DEBUG(scan_logger, "Set httpInventory/StayInDomainList to " << val);
    }
    if (vm.count("ip")) {
        bool val = true;
        try {
            val = vm["ip"].as<bool>();
        } catch (...) { } // just skip
        we_dispatcer->Option("httpInventory/StayInIP", val);
        LOG4CXX_DEBUG(scan_logger, "Set httpInventory/StayInIP to " << val);
    }
    if (vm.count("url_param")) {
        bool val = true;
        try {
            val = vm["url_param"].as<bool>();
        } catch (...) { } // just skip
        we_dispatcer->Option("httpInventory/"weoIgnoreUrlParam, val);
        LOG4CXX_DEBUG(scan_logger, "Set httpInventory/"weoIgnoreUrlParam" to " << val);
    }
    if (vm.count("content")) {
        int val = vm["content"].as<int>();
        we_dispatcer->Option("httpInventory/AllowedCType", val);
        LOG4CXX_DEBUG(scan_logger, "Set httpInventory/AllowedCType to " << val);
    }
    if (vm.count("parallel")) {
        int val = vm["parallel"].as<int>();
        we_dispatcer->Option("httpInventory/"weoParallelReq, val);
        LOG4CXX_DEBUG(scan_logger, "Set httpInventory/"weoParallelReq" to " << val);
    }
    if (vm.count("ext_deny")) {
        string val = vm["ext_deny"].as<string>();
        we_dispatcer->Option("httpInventory/"weoDeniedFileTypes, val);
        LOG4CXX_DEBUG(scan_logger, "Set httpInventory/"weoDeniedFileTypes" to " << val);
    }
    
    // reconstruct URL to scan
    string url = vm["target"].as<string>();
    if (! boost::algorithm::starts_with(url, "http://")) {
        url = "http://" + url;
    }
    webEngine::transport_url t_url(url);
    LOG4CXX_INFO(scan_logger, "Start scanning for: " << t_url.tostring());
    we_dispatcer->Option("scan_host", t_url.host);
    we_dispatcer->Option("httpInventory/BaseURL",  t_url.request);

    // create task executor
    webEngine::task* tsk = new webEngine::task(we_dispatcer);
    tsk->StorePlugins(scan_plugins);

    // run endless loop for the task
    tsk->Run();

    // init scan-time vars
    inLoop = true;
    char ch;
    ScanRes *si;
    string fname = "";
    if (vm.count("result")) {
        fname = vm["result"].as<string>();
    }
    int format = vm["output"].as<int>();

    btm::ptime scan_start = btm::second_clock::local_time();
    btm::ptime scan_curr;
    cout << "Scanning... 0 objects found... 0% complete\r";
    do
    {
        if (!tsk->IsReady()) {
            inLoop = false;
        }
        boost::this_thread::sleep(boost::posix_time::seconds(1));
        si = (ScanRes*)tsk->GetScan();
        webEngine::wOption opt = tsk->Option("completion");
        int compl;
        SAFE_GET_OPTION_VAL(opt, compl, 0);
        scan_curr = btm::second_clock::local_time();
        btm::time_period scan_elapsed(scan_start, scan_curr);
        cout << "Scanning for " << btm::to_simple_string(scan_elapsed.length()) << "... " << si->GetScanSize() << " objects found... " << compl << "% complete\r";
        LOG4CXX_TRACE(scan_logger, "Memory information: " << sys_meminfo());
        if (kbhit()) {
            ch = getch();
            ch = toupper(ch);
            switch (ch)
            {
            case 'Q':
                inLoop = false;
                break;
            case ' ':
                if (fname == "") {
                    cout << endl << "Can't save intermediate information - no results file given" << endl;
                } 
                else {
                    cout << endl << "Saving intermediate information ...";
                    save_results(si, format, fname);
                    cout << " done!" << endl;
                }
            }
        }
        tsk->CalcStatus();
        //if (time_to_save) {
        //    save_results(si, format, fname);
        //}
    }
    while(inLoop);
    tsk->Stop();

    // print result
    si = (ScanRes*)tsk->GetScan();
    fname = "";
    if (vm.count("result")) {
        fname = vm["result"].as<string>();
    }
    format = vm["output"].as<int>();
    save_results(si, format, fname);

    for (size_t i = 0; i < scan_plugins.size(); i++) {
        scan_plugins[i]->release();
    }
    // finalize...
    delete tsk;

    if (jsExec != NULL) {
        //LOG4CXX_DEBUG(scan_logger, "V8 results: " << jsExec->get_results());
        delete jsExec;
    }
    delete we_dispatcer;

}
