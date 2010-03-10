#include <string>
#include <iostream>
#include <strstream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
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
wstring default_layout = L"%d{dd MMM yyyy HH:mm:ss,SSS} [%-7p] - %m%n";

// global variables
LoggerPtr scanLogger = Logger::getLogger("watScanner");
string scanerUuid = "";

bool get_bool_option(const string& value)
{
	bool retval = false;
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

void save_config(const string& fname, po::variables_map& vm)
{
    LOG4CXX_INFO(scanLogger, "Save configuration to " << fname);

    try{
        ofstream ofs(fname.c_str());
        for ( po::variables_map::iterator i = vm.begin() ; i != vm.end() ; ++ i )
        {
            try
            {
                boost::any val = (*i).second.value();
                ofs << (*i).first << "=";
                LOG4CXX_DEBUG(scanLogger, "Save variable " << (*i).first);
                if (is_any<string>(val)) {
                    ofs << boost::any_cast<string>(val);
                }
                else if (is_any<const char*>(val)) {
                    ofs << boost::any_cast<const char*>(val);
                }
                else if (is_any<double>(val)) {
                    ofs << boost::any_cast<double>(val);
                }
                else if (is_any<int>(val)) {
                    ofs << boost::any_cast<int>(val);
                }
                ofs << endl;
            }
            catch(const boost::bad_any_cast& ex) {
                LOG4CXX_ERROR(scanLogger, "Can't save " << (*i).first << ": " << ex.what());
            }
        }
    }
    catch(std::exception& e) {
        LOG4CXX_ERROR(scanLogger, "Configuration not saved: " << e.what());
        return;
    }
    LOG4CXX_INFO(scanLogger, "Configuration saved successfully");
}

int main(int argc, char* argv[])
{
    po::options_description cfg_file("Configuration");
    po::options_description cmd_line("Command-line configuration");
    po::variables_map vm;

    cfg_file.add_options()
        ("identity", po::value<string>(), "instalation identificator")
        ("instances",  po::value<int>(), "number of instances")
        ("redis_host",  po::value<string>()->default_value(string("127.0.0.1")), "host of the Redis database")
        ("redis_port",  po::value<int>()->default_value(6379), "port of the Redis database")
        ("log_file",  po::value<string>(), "file to store log information")
        ("log_level",  po::value<int>(), "level of the log information [0=FATAL, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG, 5=TRACE]")
        ("log_layout",  po::value<string>(), "layout of the log messages")
		("daemonize",  po::value<string>(), "run program as daemon")
    ;

    cmd_line.add_options()
        ("help", "this help message")
        ("config",  po::value<string>()->default_value(default_config), "configuration file")
        ("trace",  po::value<string>(), "trace configuration file")
    ;

    store(parse_command_line(argc, argv, cmd_line), vm);
    notify(vm);

    if (vm.count("help")) {
        cout << cmd_line << endl;
        return 0;
    }
    if (vm.count("config")) {
        ifstream ifs;
        ifs.open(vm["config"].as<string>().c_str());
        if (ifs.rdbuf()->is_open() ) {
            //vm["config"].as<string>().c_str()
            //store(parse_config_file( ".\\scanner.conf", cfg_file), vm);
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
    // no trace configuration file - make configuration programmatically
    if (vm.count("log_layout")) {
        default_layout = string_to_string(vm["log_layout"].as<string>());
    }
	LayoutPtr layout(new PatternLayout(default_layout));
	if (!traceFile) {
		// set log file from config file
        if (vm.count("log_file")) {
            FileAppenderPtr appender(new FileAppender(layout, string_to_string(vm["log_file"].as<string>()), true));
            scanLogger->addAppender(appender);
            traceFile = true;
        }
        else {
		    // set trace destination by default

		    if(vm.count("daemonize") && get_bool_option(vm["daemonize"].as<string>())) {
                FileAppenderPtr appender(new FileAppender(layout, default_trace, true));
                scanLogger->addAppender(appender);
                traceFile = true;
		    }
		    else {
			    // set trace to console
                ConsoleAppenderPtr appender(new ConsoleAppender(layout));
                scanLogger->addAppender(appender);
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
                scanLogger->setLevel(Level::getFatal());
                break;
            case 1:
                scanLogger->setLevel(Level::getError());
                break;
            case 2:
                scanLogger->setLevel(Level::getWarn());
                break;
            case 3:
                scanLogger->setLevel(Level::getInfo());
                break;
            case 4:
                scanLogger->setLevel(Level::getDebug());
                break;
            default: // TRACE and bad values
                scanLogger->setLevel(Level::getTrace());
        };
        traceLevel = true;
	}
    LOG4CXX_INFO(scanLogger, "WAT Scanner started");

    // init webEngine library
    if (vm.count("trace")) {
        // init library with the given trace configuration
        webEngine::LibInit(vm["trace"].as<string>());
    }
    else {
        // init library with the existing logger
        AppenderList appList = scanLogger->getAllAppenders();
        webEngine::LibInit(appList[0], scanLogger->getLevel());
    }
    // verify instalation UUID
    if (vm.count("identity")) {
        scanerUuid = vm["identity"].as<string>();
    }
    else {
        LOG4CXX_INFO(scanLogger, "WAT Scanner identificator is undefined, create new");
        basic_random_generator<boost::mt19937> gen;
        uuid tag = gen();
        scanerUuid = boost::lexical_cast<string>(tag);
        // todo: save config
        string value = "identity=" + scanerUuid;
        istrstream ss(value.c_str());
        store(parse_config_file(ss, cfg_file), vm);
        notify(vm);
        LOG4CXX_TRACE(scanLogger, "New config value for identity is " << vm["identity"].as<string>());
        save_config(vm["config"].as<string>(), vm);
    }
    LOG4CXX_INFO(scanLogger, "WAT Scanner identificator is " << scanerUuid);
    // create connection to server

    // verify instance id

    // init plugin factory

    // init dispatcher

    // run dispatcher

    // cleanup
    webEngine::LibClose();

    LOG4CXX_INFO(scanLogger, "WAT Scanner stopped\n\n");
    return 0;
}
