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
#ifndef __PROGRAMCFG_H__
#define __PROGRAMCFG_H__
#include <string>
#include <map>

using namespace std;

class ProgramConfig
{
public:
    short   port;
    string  dbDir;
    string  storageIface;
    string  plugin_options(const string& plugin_id);
    bool    save_to_file(const string& filename);
    bool    load_from_file(const string& filename);

    ProgramConfig();
    ~ProgramConfig();

protected:
    static const string cfg_version;
    map<string, string> options;
};

#endif //__PROGRAMCFG_H__
