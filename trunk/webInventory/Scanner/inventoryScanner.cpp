/*
    inventoryScanner is the web-application audit program
    Copyright (C) 2009 Andrew "Stinger" Abramov stinger911@gmail.com

    This file is part of inventoryScanner

    inventoryScanner is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    inventoryScanneris distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with inventoryScanner.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "server.h"
#include <weHelper.h>
#include <weDispatch.h>
#include <iostream>
#include <fstream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include "version.h"

#include "taskOperations.h"

using namespace std;
namespace po = boost::program_options;

WeDispatch* globalDispatcher = NULL;

class ProgramConfig
{
public:
    short   port;
    string  dbDir;
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(port);
        ar & BOOST_SERIALIZATION_NVP(dbDir);
    };
};

int main(int argc, char* argv[])
{
    ProgramConfig configuration;

    try
    {
        // initialization
        configuration.port = 8080;
        configuration.dbDir = ".";

        // Declare the supported options.
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("version", "prints version and exit")
            ("generate", "generates 'sample.config' file")
            ("config", po::value<string>(), "configuration file")
            ;
        po::positional_options_description p;
        p.add("config", -1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
            cout << desc << "\n";
            return 1;
        }
        if (vm.count("version")) {
            cout << "webInventory scanner " << AutoVersion::FULLVERSION_STRING
                << " " << AutoVersion::STATUS
#ifdef _WIN32_WINNT
                << " (Windows)";
#else
                << " (Linux)";
#endif
#ifdef _DEBUG
            cout << " SVN build " << AutoVersion::BUILDS_COUNT;
#endif
            cout << endl;
            return 1;
        }
        if (vm.count("generate")) {

            // save data to archive
            try
            {
                std::ofstream ofs("sample.config");
                boost::archive::xml_oarchive oa(ofs);
                // write class instance to archive
                oa << BOOST_SERIALIZATION_NVP(configuration);
            }
            catch (std::exception& e) {
                cerr << "Error writing 'sample.config': " << e.what() << endl;
            }

            return 1;
        }

        WeLibInit(); // for initialize logging
        LOG4CXX_INFO(WeLogger::GetLogger(), "Application started");
        string vers = AutoVersion::FULLVERSION_STRING;
#ifdef _WIN32_WINNT
        vers += " (Windows)";
#else
        vers += " (Linux)";
#endif
#ifdef _DEBUG
        vers += " SVN build " + boost::lexical_cast<std::string>(AutoVersion::BUILDS_COUNT);
#endif

        LOG4CXX_INFO(WeLogger::GetLogger(), "Version is " << vers);
        if (vm.count("config")) {
            string cfgName = vm["config"].as<string>();
            LOG4CXX_INFO(WeLogger::GetLogger(), "Config file is "
                << cfgName);
            try
            {
                std::ifstream itfs(cfgName.c_str());
                {
                    boost::archive::xml_iarchive ia(itfs);
                    ia >> BOOST_SERIALIZATION_NVP(configuration);
                }
            }
            catch (...)
            {
                LOG4CXX_WARN(WeLogger::GetLogger(), "Can't read config from " << cfgName);
            }
        } else {
            LOG4CXX_INFO(WeLogger::GetLogger(), "Config is default, no files readed");
        }
        taskDbDir = configuration.dbDir;
        LOG4CXX_INFO(WeLogger::GetLogger(), "DB directory is " << configuration.dbDir);
        LOG4CXX_INFO(WeLogger::GetLogger(), "Listener port is " << configuration.port);
        boost::asio::io_service io_service;

        globalDispatcher = new WeDispatch;
        if (globalDispatcher != NULL)
        {
            LOG4CXX_INFO(WeLogger::GetLogger(), "Dispatcher created successfully");

            server s(io_service, configuration.port);
            io_service.run();
        }
    }
    catch (std::exception& e)
    {
        LOG4CXX_INFO(WeLogger::GetLogger(), "Exception: " << e.what());
    }

    LOG4CXX_INFO(WeLogger::GetLogger(), "Application finished");
    WeLibClose();
    return 0;
}

