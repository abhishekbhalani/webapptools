#include <string>
#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

#ifdef WIN32
string default_config = ".\\scanner.conf";
#else
string default_config = "/etc/watools/scanner.conf";
#endif

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
    if (vm.count("identity")) {
        cout << "Identity: " << vm["identity"].as<string>() << endl;
    }

    return 0;
}
