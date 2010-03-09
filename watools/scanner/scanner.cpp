#include <string>
#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
//! todo: upgrade to the boost_1.42 to use natuve version
#include "boost/uuid.hpp"
#include "boost/uuid_generators.hpp"

using namespace std;
namespace po = boost::program_options;
namespace bfs = boost::filesystem;

#ifdef WIN32
string default_config = ".\\scanner.conf";
string default_trace  = ".\\watools.scanner.log";
#else
string default_config = "/etc/watools/scanner.conf";
string default_trace  = "/tmp/watools.scanner.trace";
#endif
string default_trace_level = "INFO";

bool get_bool_option(const string& value)
{
	bool retval = false;
	return retval;
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
        ("log_level",  po::value<string>(), "level of the log information [ERROR, WARN, INFO, DEBUG, TRACE]")
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
            store(parse_config_file(ifs, cfg_file), vm);
            notify(vm);
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
    if (vm.count("trace")) {
		bool traceFile = false;
		bool traceLevel = false;

		if ( bfs::exists(vm["trace"].as<string>()) ){
			// try to load given trace configuration
		}
		if (!traceFile) {
			// set trace file from config file
		}
		if (!traceLevel) {
			// set trace level from config file
		}
		if (!traceFile) {
			// set trace destination by default
			if(get_bool_option(vm["daemonize"].as<string>())) {
				// set trace to file: default_trace
			}
			else {
				// set trace to console
			}
		}
		if (!traceLevel) {
			// set trace level by default_trace_level
		}
    }

    return 0;
}
