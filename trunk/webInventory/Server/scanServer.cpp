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
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include "version.h"
#include <programcfg.h>

#include "taskOperations.h"

using namespace std;
using namespace boost::filesystem;
namespace po = boost::program_options;
using namespace webEngine;

Dispatch* globalDispatcher = NULL;
string cfgFile;
string workDir;

int main(int argc, char* argv[])
{
    ProgramConfig configuration;

    try
    {
        // initialization
        configuration.port = 8080;
        configuration.dbDir = ".";
        configuration.storageIface = "D82B31419339";

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
            if (!configuration.save_to_file("sample.config"))
            {
                cerr << "Error writing 'sample.config'";
            }

            return 1;
        }

        LibInit(); // for initialize logging
        LOG4CXX_INFO(iLogger::GetLogger(), "Application started");
        string vers = AutoVersion::FULLVERSION_STRING;
#ifdef _WIN32_WINNT
        vers += " (Windows)";
#else
        vers += " (Linux)";
#endif
#ifdef _DEBUG
        vers += " SVN build " + boost::lexical_cast<std::string>(AutoVersion::BUILDS_COUNT);
#endif

        LOG4CXX_INFO(iLogger::GetLogger(), "Version is " << vers);
        if (vm.count("config")) {
            cfgFile = vm["config"].as<string>();
            LOG4CXX_INFO(iLogger::GetLogger(), "Config file is "
                << cfgFile);
            if (!configuration.load_from_file(cfgFile))
            {
                LOG4CXX_WARN(iLogger::GetLogger(), "Can't read config from " << cfgFile);
                cfgFile = "";
            }
        } else {
            cfgFile = "";
            LOG4CXX_INFO(iLogger::GetLogger(), "Config is default, no files readed");
        }
        taskDbDir = configuration.dbDir;
        LOG4CXX_INFO(iLogger::GetLogger(), "DB directory is " << configuration.dbDir);
        LOG4CXX_INFO(iLogger::GetLogger(), "Listener port is " << configuration.port);
        boost::asio::io_service io_service;

        globalDispatcher = new Dispatch;
        if (globalDispatcher != NULL)
        {
            LOG4CXX_INFO(iLogger::GetLogger(), "Dispatcher created successfully");
            path cfgPath;

            cfgPath = argv[0];
            cfgPath = cfgPath.remove_filename();
            if (cfgPath.string().empty())
            {
                cfgPath = "./";
            }
            workDir = cfgPath.string();
            globalDispatcher->RefreshPluginList(cfgPath);

            iPlugin* plugin = globalDispatcher->LoadPlugin(configuration.storageIface);

            if (plugin != NULL) {
                LOG4CXX_INFO(iLogger::GetLogger(), "Storage plugin loaded successfully.");
                LOG4CXX_INFO(iLogger::GetLogger(), "Plugin ID=" << plugin->GetID() << "; Description: " << plugin->GetDesc());
                iStorage* storage = (iStorage*)plugin->GetInterface("iStorage");

                if (storage != NULL)
                {
                    string plg_settings = configuration.plugin_options(plugin->GetID());
                    storage->InitStorage(plg_settings);
                    globalDispatcher->Storage(storage);

                    server s(io_service, configuration.port);
                    io_service.run();
                }
                else {
                    LOG4CXX_FATAL(iLogger::GetLogger(), "No iStorage interface in the plugin " << plugin->GetID());
                }
            }
            else {
                LOG4CXX_FATAL(iLogger::GetLogger(), "Can't load the plugin " << configuration.storageIface);
            }
        }
    }
    catch (std::exception& e)
    {
        LOG4CXX_INFO(iLogger::GetLogger(), "Exception: " << e.what());
    }

    LOG4CXX_INFO(iLogger::GetLogger(), "Application finished");
    LibClose();
    return 0;
}

string save_cfg_storage(const string& id)
{
    string retval = "";
    ProgramConfig configuration;

    if (cfgFile != "")
    {
        if (! configuration.load_from_file(cfgFile) )
        {
            LOG4CXX_WARN(iLogger::GetLogger(), "Can't read config from " << cfgFile);
            cfgFile = "";
            return retval;
        }
        configuration.storageIface = id;
        if (! configuration.save_to_file(cfgFile) )
        {
            LOG4CXX_WARN(iLogger::GetLogger(), "Can't save config to " << cfgFile);
            cfgFile = "";
            return retval;
        }
        retval = configuration.plugin_options(id);
    }
    return retval;
}
