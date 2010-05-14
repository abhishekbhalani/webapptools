#include <string>
#include <iostream>
#include <strstream>
#include <fstream>
#ifdef WIN32
#include <errno.h>
#include <winsock2.h>
#include <process.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif //WIN32
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
//! todo: upgrade to the boost_1.42 to use natuve version
#include "boost/uuid.hpp"
#include "boost/uuid_generators.hpp"
#include "boost/uuid_io.hpp"
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/appender.h>
// webEngine
#include <weHelper.h>

#include "version.h"

#ifdef NDEBUG
__declspec(dllexport) double _HUGE;
#endif

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace std;
using namespace boost::uuids;

namespace po = boost::program_options;
namespace bfs = boost::filesystem;

#ifdef WIN32
string default_config = ".\\scanner.conf";
wstring default_trace  = L".\\watools.scanner.log";
#else
string default_config = "/etc/watools/scanner.conf";
wstring default_trace  = L"/tmp/watools.scanner.trace";
#endif
int default_trace_level = 3; //INFO
wstring default_layout = L"%d{dd MMM yyyy HH:mm:ss,SSS} [%-5p] - %m%n";

// global variables
LoggerPtr scan_logger = Logger::getLogger("watScanner");
string scaner_uuid = "";
string scaner_version = "";
int scaner_instance = 0;
bool daemonize = false;
string db1_instance_name = "";

// dispatcher prototype
extern void dispatcher_routine(po::variables_map& vm);

bool get_bool_option(const string& value, bool noLogging = false)
{
	bool retval = false;
	string val = value;

	boost::to_lower(val);
	if (val == "yes") {
		retval = true;
	}
    else if (val == "true") {
		retval = true;
	}
    else {
        int i = 0;
        try {
	        i = boost::lexical_cast<int>(val);
        } catch (std::exception &e) {
            if (noLogging) {
                cerr << "Can't parse '" << val << "' as bool: " << e.what() << ". Assume false." << endl;
            }
            else {
                LOG4CXX_WARN(scan_logger, "Can't parse '" << val << "' as bool: " << e.what() << ". Assume false.");
            }
        }
	    if (i != 0) {
		    retval = true;
	    }
    }
    if (!noLogging) {
	    LOG4CXX_DEBUG(scan_logger, "get_bool_option(" << value << ") = " << retval);
    }
	return retval;
}

std::wstring string_to_wstring(const std::string& s)
{
    std::wstring temp(s.length(),L' ');
    std::copy(s.begin(), s.end(), temp.begin());
    return temp;
}

template <typename T>
inline bool is_any(const boost::any& op)
{
  return (op.type() == typeid(T));
}

void save_config(const string& fname, po::variables_map& vm, po::options_description& desc, bool noLogging = false )
{
	if (!noLogging) {
		LOG4CXX_INFO(scan_logger, "Save configuration to " << fname);
	}

    try{
        ofstream ofs(fname.c_str());
		vector< boost::shared_ptr<po::option_description>> opts = desc.options();
		for ( unsigned i = 0; i < opts.size(); i++) {
			string comment = "#";
			string value = "";
			string name = opts[i]->long_name();
			if (!noLogging) {
				LOG4CXX_DEBUG(scan_logger, "Save variable " << name);
			}
			ofs << "#" << opts[i]->description() << endl;
			if (vm.count(name)) {
				comment = "";
				boost::any val = vm[name].value();
                if (is_any<string>(val)) {
                    value = boost::any_cast<string>(val);
                }
                else if (is_any<const char*>(val)) {
                    value = boost::any_cast<const char*>(val);
                }
                else if (is_any<double>(val)) {
					value = boost::lexical_cast<string>(boost::any_cast<double>(val));
                }
                else if (is_any<int>(val)) {
                    value = boost::lexical_cast<string>(boost::any_cast<int>(val));
                }
                else if (is_any<bool>(val)) {
                    value = boost::lexical_cast<string>(boost::any_cast<bool>(val));
                }
				else {
					if (noLogging) {
						cerr << "Unknown variable type " << val.type().name() << endl;
					}
					else {
						LOG4CXX_ERROR(scan_logger, "Unknown variable type " << val.type().name());
					}
				}
			}
			else {
				// get the default value
			}
			ofs << comment << name << "=" << value << endl << endl;
		}
    }
    catch(std::exception& e) {
		if (noLogging) {
			cerr << "Configuration not saved: " << e.what() << endl;
		}
		else {
			LOG4CXX_ERROR(scan_logger, "Configuration not saved: " << e.what());
		}
        return;
    }
	if (!noLogging) {
		LOG4CXX_INFO(scan_logger, "Configuration saved successfully");
	}
}

int main(int argc, char* argv[])
{
    po::options_description cfg_file("Configuration");
    po::options_description cmd_line("Command-line configuration");
    po::options_description cmd_line_vis("Command-line configuration");
    po::variables_map vm;
    string db1_stage;

    cfg_file.add_options()
		("keepalive_timeout", po::value<int>()->default_value(int(5)), "keep-alive timeout in seconds")
		("sysinfo_timeout", po::value<int>()->default_value(int(60)), "system information timeout in seconds")
		("scanner_name", po::value<string>()->default_value(string("default scanner")), "human-readable identificator of the scanner instalation")
        ("log_file",  po::value<string>(), "file to store log information")
        ("log_level",  po::value<int>(), "level of the log information [0=FATAL, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG, 5=TRACE]")
        ("log_layout",  po::value<string>(), "layout of the log messages (see the log4cxx documentation)")
        ("plugin_dir",  po::value<string>()->default_value(string("./")), "directory, where plug-ins are placed")
        ("db2_interface",  po::value<string>()->default_value(string("mem_storage")), "plug-in identifier to connect to Storage DB")
        ("db2_parameters",  po::value<string>()->default_value(string("database")), "plug-in configuration to connect to Storage DB")
        ("domain_list",  po::value<string>(), "semicolon separated list of the allowed sub-domains")
        ("ext_deny",  po::value<string>(), "semicolon separated list of the denied file types (by extensions)")
        ("depth", po::value<int>(), "scan depth")
        ("dir", po::value<bool>(), "stay in the directory")
        ("host", po::value<bool>(), "stay on the host")
        ("domain", po::value<bool>(), "stay on the domain")
        ("dlist", po::value<bool>(), "stay in the domain's list")
        ("ip", po::value<bool>(), "stay on the IP address")
        ("url_param", po::value<bool>(), "ignore URL parameters")
        ("parallel", po::value<int>(), "number of parallel requests")
        ("jscript", po::value<bool>(), "analyze JavaScript links")
        ("js_preload", po::value<string>(), "file with JavaScripts preloads (debug routines)")
        ("content", po::value<int>(), "conent_type processing mode")
        ("output", po::value<int>(), "output format: 0 - full, 1 - only links")
    ;

    cmd_line_vis.add_options()
        ("help", "this help message")
        ("target", po::value<string>(), "target to scan")
        ("output", po::value<int>()->default_value(int(0)), "output format: 0 - full, 1 - only links")
        ("depth", po::value<int>()->default_value(int(10)), "scan depth")
        ("dir", "stay in the directory")
        ("host", "stay on the host")
        ("domain", "stay on the domain")
        ("dlist", "stay in the domain's list")
        ("ip",  "stay on the IP address")
        ("url_param", "ignore URL parameters")
        ("parallel", po::value<int>()->default_value(int(10)), "number of parallel requests")
        ("no-jscript", "not analyze JavaScript links")
        ("content", po::value<bool>()->default_value(int(0)), "conent_type processing mode")
        ("config",  po::value<string>()->default_value(default_config), "configuration file")
        ("result",  po::value<string>(), "results file (stdout, if not set)")
        ("trace",  po::value<string>(), "trace configuration file")
		("generate", po::value<string>(), "make config file with current values")
    ;
// 	// hidden options
//     cmd_line.add_options()
//         ("instance", "used to run instance")
// 	;

	cmd_line.add(cmd_line_vis);

	{
		stringstream ost;
		ost << VERSION_MAJOR << "." << VERSION_MINOR << "." <<
			VERSION_BUILDNO << "." << VERSION_EXTEND << 
			" build at " << VERSION_DATE <<
#ifdef _DEBUG
			" " << VERSION_TIME <<
#endif
#ifdef WIN32
			" (Windows)" <<
#else
			" (Linux)" <<
#endif
			" ";
		scaner_version = ost.str();
	}
    cout << "HTTP links collector " << scaner_version << endl;

    try {
        store(parse_command_line(argc, argv, cmd_line), vm);
        notify(vm);
    }
    catch(std::exception &e) {
        cerr << "Can't parse command line: " << e.what();
        return -1;
    }


    if (vm.count("help")) {
        cout << cmd_line_vis << endl;
        return 0;
    }
	// create config file
	if (vm.count("generate")) {
		cout << "WAT scanner " << scaner_version << endl;
		cout << "write config file " << vm["generate"].as<string>() << endl;
		save_config(vm["generate"].as<string>(), vm, cfg_file, true);
		return 0;
	}

	// parse configuration
    if (vm.count("config")) {
        ifstream ifs;
        ifs.open(vm["config"].as<string>().c_str());
        if (ifs.rdbuf()->is_open() ) {
            try{
                store(parse_config_file(ifs, cfg_file, true), vm);
                notify(vm);
            }
            catch(std::exception &e) {
                cerr << "Can't load config file: " << vm["config"].as<string>() << "! " << e.what() << endl;
                return 1;
            }
        }
        else {
            cerr << "Can't open config file: " << vm["config"].as<string>() << "!" << endl;
            return 1;
        }
    }
    else {
        cerr << "Config file must be specified!" << endl;
        return 1;
    }

    // init log subsystem
	bool traceFile = false;
	bool traceLevel = false;
    if (vm.count("trace")) {
		if ( bfs::exists(vm["trace"].as<string>()) ){
			// try to load given trace configuration
            try {
#ifdef WIN32
                PropertyConfigurator::configure(log4cxx::File(vm["trace"].as<string>()), LOG4CXX_STR("webEngine"));
#else
                PropertyConfigurator::configure(log4cxx::File(cfgFile));
#endif
                traceFile = true;
		        traceLevel = true;
            }
            catch (...) {
                traceFile = false;
		        traceLevel = false;
            }
		}
    }

	// check daemonize
	if (vm.count("daemonize")) {
		daemonize = get_bool_option(vm["daemonize"].as<string>(), true);
	}

    // no trace configuration file - make configuration programmatically
    if (vm.count("log_layout")) {
        default_layout = string_to_wstring(vm["log_layout"].as<string>());
    }
	LayoutPtr layout(new PatternLayout(default_layout));
	if (!traceFile) {
		// set log file from config file
        if (vm.count("log_file")) {
            FileAppenderPtr appender(new FileAppender(layout, string_to_wstring(vm["log_file"].as<string>()), true));
            scan_logger->addAppender(appender);
            traceFile = true;
        }
        else {
		    // set trace destination by default

		    if(daemonize) {
                FileAppenderPtr appender(new FileAppender(layout, default_trace, true));
                scan_logger->addAppender(appender);
                traceFile = true;
		    }
		    else {
			    // set trace to console
                ConsoleAppenderPtr appender(new ConsoleAppender(layout));
                scan_logger->addAppender(appender);
                traceFile = true;
		    }
        }
	}
	if (!traceLevel) {
		// set trace level from config file
        int lvl = default_trace_level;
        if (vm.count("log_level")) {
            lvl = vm["log_level"].as<int>();
        }
        switch (lvl) {
            case 0:
                scan_logger->setLevel(Level::getFatal());
                break;
            case 1:
                scan_logger->setLevel(Level::getError());
                break;
            case 2:
                scan_logger->setLevel(Level::getWarn());
                break;
            case 3:
                scan_logger->setLevel(Level::getInfo());
                break;
            case 4:
                scan_logger->setLevel(Level::getDebug());
                break;
            default: // TRACE and bad values
                scan_logger->setLevel(Level::getTrace());
        };
        traceLevel = true;
	}
    LOG4CXX_INFO(scan_logger, "\n\n");
	LOG4CXX_INFO(scan_logger, "WAT Scanner started. Version: " << scaner_version);
	LOG4CXX_INFO(scan_logger, "Loaded configuration from " << vm["config"].as<string>());

	// init webEngine library
    if (vm.count("trace")) {
        // init library with the given trace configuration
        webEngine::LibInit(vm["trace"].as<string>());
    }
    else {
        // init library with the existing logger
        AppenderList appList = scan_logger->getAllAppenders();
		webEngine::LibInit(appList[0], scan_logger->getLevel());
    }

#ifdef WIN32
    //----------------------
    // Initialize Winsock
    WSADATA wsaData;
    int iResult;
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != NO_ERROR) {
        LOG4CXX_FATAL(scan_logger, "WSAStartup failed: " << iResult);
		return 1;
    }
#endif

    if (vm.count("target")) {
        // run dispatcher
        dispatcher_routine(vm);
    }
    else {
        LOG4CXX_FATAL(scan_logger, "Target not specified");
        cerr << "FATAL: Target not specified" << endl << endl;
        cout << cmd_line_vis << endl;
    }    

    // cleanup
    webEngine::LibClose();

    LOG4CXX_INFO(scan_logger, "WAT Scanner stopped");
    return 0;
}
