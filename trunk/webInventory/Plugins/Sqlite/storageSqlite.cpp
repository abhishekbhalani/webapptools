/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew Abramov aabramov@ptsecurity.ru

    This file is part of webEngine

    webEngine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "storageSqlite.h"
#include "version.h"
#include "sqliteStorage.xpm"
//#include "demoPlugin.xrc"

SqliteStorage::SqliteStorage( Dispatch* krnl, void* handle /*= NULL*/ ) :
    iStorage(krnl, handle)
{
    pluginInfo.IfaceName = "SqliteStorage";
    pluginInfo.IfaceList.push_back("SqliteStorage");
    pluginInfo.PluginDesc = "Sqlite storage";
    pluginInfo.PluginId = "9FBCC44C52A1";
    pluginInfo.PluginIcon = WeXpmToStringList(sqliteStorage_xpm, sizeof(sqliteStorage_xpm) / sizeof(char*) );
}

SqliteStorage::~SqliteStorage(void)
{
}

void* SqliteStorage::GetInterface( const string& ifName )
{
    if (iequals(ifName, "SqliteStorage"))
    {
        usageCount++;
        return (void*)(this);
    }
    return iStorage::GetInterface(ifName);
}

const string SqliteStorage::GetSetupUI( void )
{
    /// @todo: change XRC to set the fields values
    return "";
}

void SqliteStorage::ApplySettings( const string& xmlData )
{

}
