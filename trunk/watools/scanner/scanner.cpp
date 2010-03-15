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
#include "../common/redisclient.h"
// webEngine
#include <weHelper.h>

#include "version.h"

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
redis::client* db1_client;
boost::mutex db1_lock;
bool daemonize = false;
string db1_instance_name = "";

// dispatcher prototype
extern void dispatcher_routine(int inst, po::variables_map& vm);

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

std::wstring string_to_string(const std::string& s)
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

    vector<string> redis_out;
    int inst_num, max_inst;

    cfg_file.add_options()
        ("identity", po::value<string>(), "instalation identificator")
        ("instances",  po::value<int>()->default_value(int(1)), "number of instances")
        ("redis_host",  po::value<string>()->default_value(string("127.0.0.1")), "host of the Redis database")
        ("redis_port",  po::value<int>()->default_value(int(6379)), "port of the Redis database")
		("redis_auth",  po::value<string>(), "authentication to redis database")
		("redis_db",  po::value<int>()->default_value(int(0)), "redis database index (0-16)")
		("keepalive_timeout", po::value<int>()->default_value(int(5)), "keep-alive timeout in seconds")
		("sysinfo_timeout", po::value<int>()->default_value(int(60)), "system information timeout in seconds")
		("scanner_name", po::value<string>()->default_value(string("default scanner")), "human-readable identificator of the scanner instalation")
        ("log_file",  po::value<string>(), "file to store log information")
        ("log_level",  po::value<int>(), "level of the log information [0=FATAL, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG, 5=TRACE]")
        ("log_layout",  po::value<string>(), "layout of the log messages (see the log4cxx documentation)")
		("daemonize",  po::value<string>(), "run program as daemon (yes|no or true|false)")
    ;

    cmd_line_vis.add_options()
        ("help", "this help message")
        ("config",  po::value<string>()->default_value(default_config), "configuration file")
        ("trace",  po::value<string>(), "trace configuration file")
		("generate", po::value<string>(), "make config file with current values")
    ;
	// hidden options
    cmd_line.add_options()
        ("instance", "used to run instance")
	;

	cmd_line.add(cmd_line_vis);

    store(parse_command_line(argc, argv, cmd_line), vm);
    notify(vm);

	string version;
	{
		stringstream ost;
		ost << VERSION_MAJOR << "." << VERSION_MINOR << "." <<
			VERSION_BUILDNO << "." << VERSION_EXTEND << 
			" build at " << VERSION_DATE <<
#ifdef _DEBUG
			" " << VERSION_TIME << " " << VERSION_SVN <<
#endif
#ifdef WIN32
			" (Windows)" <<
#else
			" (Linux)" <<
#endif
			" ";
		version = ost.str();
	}

    if (vm.count("help")) {
        cout << cmd_line_vis << endl;
        return 0;
    }
	// create config file
	if (vm.count("generate")) {
		cout << "WAT scanner " << version << endl;
		cout << "write config file " << vm["generate"].as<string>() << endl;
        {   
            istrstream ss("");
			store(parse_config_file(ss, cfg_file), vm);
			notify(vm);
        }
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
        default_layout = string_to_string(vm["log_layout"].as<string>());
    }
	LayoutPtr layout(new PatternLayout(default_layout));
	if (!traceFile) {
		// set log file from config file
        if (vm.count("log_file")) {
            FileAppenderPtr appender(new FileAppender(layout, string_to_string(vm["log_file"].as<string>()), true));
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
	LOG4CXX_INFO(scan_logger, "WAT Scanner started. Version: " << version);
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
    // verify installation UUID
    if (vm.count("identity")) {
        scaner_uuid = vm["identity"].as<string>();
    }
    else {
        LOG4CXX_INFO(scan_logger, "WAT Scanner identificator is undefined, create new");
        basic_random_generator<boost::mt19937> gen;
        uuid tag = gen();
        scaner_uuid = boost::lexical_cast<string>(tag);
        // todo: save config
        string value = "identity=" + scaner_uuid;
		{
			istrstream ss(value.c_str());
			store(parse_config_file(ss, cfg_file), vm);
			notify(vm);
		}
        LOG4CXX_TRACE(scan_logger, "New config value for identity is " << vm["identity"].as<string>());
        save_config(vm["config"].as<string>(), vm, cfg_file);
    }
    LOG4CXX_INFO(scan_logger, "WAT Scanner identificator is " << scaner_uuid);

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

	// demonize and run instances
#ifdef WIN32
    // prevent instance to fall into endless "run-the-instances" loop
    // only on Win32, 'cause the *NIX have the fork() syscall, which do all this work
    LOG4CXX_TRACE(scan_logger, "running as " << argv[0]);
    if (boost::iequals(argv[0], "instance") ) {
        daemonize = false;
        LOG4CXX_DEBUG(scan_logger, "Win32 - already daemonized instance");
    }
#endif
	if (daemonize) {
		int instances = vm["instances"].as<int>();
		LOG4CXX_INFO(scan_logger, "daemonize. try to run " << instances << " instances");
		bool master = true;
		for (int i = 0; i < instances; i++) {
			LOG4CXX_TRACE(scan_logger, "Start instance");
#ifdef WIN32
            char** cmd = new char*[argc+1];
            for (int j = 1; j < argc; j++) {
                cmd[j] = strdup(argv[j]);
            }
            cmd[argc] = NULL;
            cmd[0] = strdup("instance");
            int pid = _spawnv(_P_NOWAIT, argv[0], cmd);
            if (pid <= 0) {
                LOG4CXX_ERROR(scan_logger, "_spawnv failed: (" << pid << ") ERRNO=" << errno);
            }
#else
            pid_t pid = vfork();
            if (pid < 0) {
                // error 
                LOG4CXX_ERROR(scan_logger, "vfork returns error: " << pid);
            }
            else if (pid == 0) {
                // child process
                LOG4CXX_INFO(scan_logger, "instance started!");
                master = false;
                break;
            }
#endif
            LOG4CXX_DEBUG(scan_logger, "running instance #" << i+1);
		}
        if (master) {
            LOG4CXX_INFO(scan_logger, "All instances started, exit launcher");
            goto finish;
        }
	}

    // create connection to Redis server
    db1_stage = "connection creation";
    db1_client = NULL;
    try {
        db1_client = new redis::client(vm["redis_host"].as<string>(), vm["redis_port"].as<int>());
        if (vm.count("redis_auth")) {
            string db_auth = vm["redis_auth"].as<string>();
            if (db_auth != "")
            {
                db1_stage = "authentication";
                db1_client->auth(db_auth);
            }
        }
        int db_index = vm["redis_db"].as<int>();
        db1_stage = "select DB";
        db1_client->select(db_index);
    }
    catch (redis::redis_error& re) {
    	LOG4CXX_FATAL(scan_logger, "Redis DB connection error (" << db1_stage << "): " << (string)re);
        if (db1_client != NULL) {
            delete db1_client;
        }
        db1_client = NULL;
    }
    if (db1_client == NULL) {
        LOG4CXX_TRACE(scan_logger, "Redis DB connection error - exit");
        goto finish;
    }
    LOG4CXX_INFO(scan_logger, "Redis connection established");

    // verify instance id
    inst_num = db1_client->keys("ScanModule:" + scaner_uuid + "*", redis_out);
    LOG4CXX_TRACE(scan_logger, "DB1 returns " << inst_num << " as number of instances");
    inst_num++;
    max_inst = vm["instances"].as<int>();
    if (inst_num > max_inst) {
        LOG4CXX_FATAL(scan_logger, "Can't run instance " << inst_num << " 'cause the limit is " << max_inst);
        goto finish;
    }
    LOG4CXX_DEBUG(scan_logger, "Register instance #" << inst_num);
    db1_instance_name = "ScanModule:" + scaner_uuid + ":" + boost::lexical_cast<string>(inst_num);
    db1_client->set(db1_instance_name, "0");
    db1_client->expire(db1_instance_name, 10);

    // init plugin factory

    // init dispatcher

    // run dispatcher
    dispatcher_routine(inst_num, vm);
    

finish:
    // cleanup
    if (db1_client != NULL) {
        delete db1_client;
    }
    webEngine::LibClose();

    LOG4CXX_INFO(scan_logger, "WAT Scanner stopped\n\n");
    return 0;
}
