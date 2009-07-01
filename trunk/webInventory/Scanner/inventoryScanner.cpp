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
#include <iostream>
#include <string>
#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

int main(int argc, char* argv[])
{
    try
    {
        WeLibInit(); // for initialize logging
        LOG4CXX_INFO(WeLogger::GetLogger(), "Application started");
        // Declare the supported options.
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
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

        if (vm.count("config")) {
            cout << "Config file is " 
                << vm["config"].as<string>() << ".\n";
        } else {
            cout << "Config file is default.\n";
        }
        boost::asio::io_service io_service;

        using namespace std; // For atoi.
        server s(io_service, 8080);

        io_service.run();
    }
    catch (std::exception& e)
    {
        cerr << "Exception: " << e.what() << "\n";
    }
//     try
//     {
//         if (argc != 2)
//         {
//             std::cerr << "Usage: server [<config >]\n";
//             return 1;
//         }
// 
//         boost::asio::io_service io_service;
// 
//         using namespace std; // For atoi.
//         server s(io_service, atoi(argv[1]));
// 
//         io_service.run();
//     }
//     catch (std::exception& e)
//     {
//         std::cerr << "Exception: " << e.what() << "\n";
//     }

    LOG4CXX_INFO(WeLogger::GetLogger(), "Application finished");
    WeLibClose();
    return 0;
}

