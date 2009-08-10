/*
    tskScanner is the web-application audit program
    Copyright (C) 2009 Andrew "Stinger" Abramov stinger911@gmail.com

    This file is part of tskScanner

    tskScanner is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    tskScanner is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with tskScanner.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "watchdog.h"
#include <iostream>
#include <fstream>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <programcfg.h>
#include "version.h"

using namespace std;
namespace fs = boost::filesystem;
namespace po = boost::program_options;

extern void task_executor(const string& taskID);

int main(int argc, char* argv[])
{
    ProgramConfig configuration;
    string traceName;
    string cfgFile;
    string taskID;

    try
    {
        // initialization
        configuration.port = 8080;
        configuration.dbDir = ".";
        configuration.storageIface = "D82B31419339";
        configuration.fileDB = "sample.db";

        // Declare the supported options.
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("version", "prints version and exit")
            ("config", po::value<string>(), "configuration file")
            ("id", po::value<string>(), "identifier of the task to run")
            ("trace", po::value<string>(), "trace configuration file")
            ("storage", po::value<string>(), "storage interface redefinition")
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
            cout << "webInventory task executor " << AutoVersion::FULLVERSION_STRING
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
        if (vm.count("trace")) {
            traceName = vm["trace"].as<string>();
            fs::path tp(traceName);
            if (!exists(tp))
            {
                cerr << "Can't find trace configuration file: " << traceName << endl;
                return 1;
            }
        }
        else {
            cerr << "Trace configuration file must be present!" << endl;
            return 1;
        }
        WeLibInit(traceName); // for initialize logging
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
            cfgFile = vm["config"].as<string>();
            LOG4CXX_INFO(WeLogger::GetLogger(), "Config file is "
                << cfgFile);
            try
            {
                std::ifstream itfs(cfgFile.c_str());
                {
                    boost::archive::xml_iarchive ia(itfs);
                    ia >> BOOST_SERIALIZATION_NVP(configuration);
                }
            }
            catch (...)
            {
                cfgFile = "";
                LOG4CXX_WARN(WeLogger::GetLogger(), "Can't read config from " << cfgFile);
            }
        } else {
            cfgFile = "";
            LOG4CXX_INFO(WeLogger::GetLogger(), "Config is default, no files readed");
        }

        if (vm.count("storage")) {
            LOG4CXX_INFO(WeLogger::GetLogger(), "Override storage plugin from command-line parameter");
            configuration.storageIface = vm["storage"].as<string>();
            LOG4CXX_INFO(WeLogger::GetLogger(), "Storage interface is " << configuration.storageIface);
        }

        if (vm.count("id")) {
            taskID = vm["id"].as<string>();
            LOG4CXX_DEBUG(WeLogger::GetLogger(), "Given task ID is " << taskID);
        }
        else {
            // skip all other code - just finalize execution
<<<<<<< .mine
            throw std::exception();//("No task identifier given - exiting!");
=======
            throw std::runtime_error("No task identifier given - exiting!");
>>>>>>> .r64
        }

        globalData.dispatcher = new WeDispatch;
        if (globalData.dispatcher != NULL)
        {
            LOG4CXX_DEBUG(WeLogger::GetLogger(), "Dispatcher created successfully");
            fs::path cfgPath;
            cfgPath = argv[0];
            cfgPath = cfgPath.remove_filename();
            if (cfgPath.string().empty())
            {
                cfgPath = "./";
            }
            globalData.dispatcher->RefreshPluginList(cfgPath);

            iwePlugin* plugin = globalData.dispatcher->LoadPlugin(configuration.storageIface);

            if (plugin != NULL) {
                LOG4CXX_INFO(WeLogger::GetLogger(), "Storage plugin loaded successfully.");
                LOG4CXX_INFO(WeLogger::GetLogger(), "Plugin ID=" << plugin->GetID() << "; Description: " << plugin->GetDesc());
                iweStorage* storage = (iweStorage*)plugin->GetInterface("iweStorage");

                if (storage != NULL)
                {
                    cfgPath = configuration.dbDir;
                    cfgPath /= configuration.fileDB;
                    storage->InitStorage(cfgPath.string());
                    globalData.dispatcher->Storage(storage);

                    // check task presence
                    globalData.load_task(taskID);
                    if (globalData.task_info == NULL)
                    {
                        string msg = "Task ID=" + taskID + " not found - exiting!";
<<<<<<< .mine
                        throw std::exception();//(msg.c_str());
=======
                        throw std::runtime_error(msg.c_str());
>>>>>>> .r64
                    }
                    globalData.scan_info = new WeScan;
                    if (globalData.scan_info == NULL)
                    {
                        string msg = "Can't create scan information - exiting!";
<<<<<<< .mine
                        throw std::exception();//(msg.c_str());
=======
                        throw std::runtime_error(msg.c_str());
>>>>>>> .r64
                    }
                    WeOption opt = globalData.task_info->Option(weoParentID);
                    string objectID;
                    SAFE_GET_OPTION_VAL(opt, objectID, "0");
                    globalData.scan_info->scanID = globalData.dispatcher->Storage()->GenerateID(weObjTypeScan);
                    globalData.scan_info->objectID = objectID;
                    LOG4CXX_INFO(WeLogger::GetLogger(), "Scan information ID=" << globalData.scan_info->scanID
                        << "; start time: " << posix_time::to_simple_string(globalData.scan_info->startTime));

                    // create watchdog thread
                    boost::thread watch_dog(watch_dog_thread, taskID);

                    // go-go-go! :)
                    task_executor(taskID);

                    // finalize
                    string report = globalData.scan_info->ToXml();
                    globalData.dispatcher->Storage()->ScanSave(report);
                }
                else {
                    LOG4CXX_FATAL(WeLogger::GetLogger(), "No iweStorage interface in the plugin " << plugin->GetID());
                }
            }
            else {
                LOG4CXX_FATAL(WeLogger::GetLogger(), "Can't load the plugin " << configuration.storageIface);
            }
        }
        else {
            LOG4CXX_ERROR(WeLogger::GetLogger(), "Can't create dispatcher");
        }
    }
    catch (std::exception& e)
    {
        LOG4CXX_FATAL(WeLogger::GetLogger(), "Exception: " << e.what());
        // postprocess
        if (globalData.dispatcher != NULL)
        {
            if (globalData.scan_info != NULL)
            {
                LOG4CXX_TRACE(WeLogger::GetLogger(), "Exception handler: save scan information");
                if (globalData.scan_info->status != WeScan::weScanFinished && globalData.scan_info->status != WeScan::weScanStopped)
                {
                    globalData.scan_info->status = WeScan::weScanError;
                }
                if (globalData.scan_info->finishTime == posix_time::not_a_date_time)
                {
                    globalData.scan_info->finishTime = posix_time::second_clock::local_time();
                }
                string report = globalData.scan_info->ToXml();
                globalData.dispatcher->Storage()->ScanSave(report);
            }
            if (globalData.task_info != NULL)
            {
                LOG4CXX_TRACE(WeLogger::GetLogger(), "Exception handler: save task information");
                globalData.task_info->Option(weoTaskStatus, WI_TSK_IDLE);
                globalData.task_info->Option(weoTaskCompletion, 0);
                globalData.save_task();
            }
        }
    }

    LOG4CXX_INFO(WeLogger::GetLogger(), "Application finished");
    WeLibClose();

    // remove trace configuration
    fs::remove(traceName);

    return 0;
}
