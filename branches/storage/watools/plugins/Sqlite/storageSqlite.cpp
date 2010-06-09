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
    LOG4CXX_TRACE(logger, "SqliteStorage plugin created");
}

SqliteStorage::~SqliteStorage(void)
{
    LOG4CXX_TRACE(logger, "SqliteStorage plugin destroyed");
}

void* SqliteStorage::GetInterface( const string& ifName )
{
    LOG4CXX_TRACE(logger, "SqliteStorage::GetInterface " << ifName);
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

int SqliteStorage::Get(Record& filter, Record& respFilter, RecordSet& results)
{
    LOG4CXX_TRACE(logger, "SqliteStorage::Get objType=" << filter.objectID);
    /// @todo Implement this
    LOG4CXX_FATAL(iLogger::GetLogger(), "SqliteStorage::Get not implemented!");
    return 0;
}

int SqliteStorage::Set(Record& filter, Record& data)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "SqliteStorage::Set(Record, Record); objType=" << filter.objectID);
    /// @todo Implement this
    LOG4CXX_FATAL(iLogger::GetLogger(), "SqliteStorage::Set not implemented!");
    return 0;
}

int SqliteStorage::Set(RecordSet& data)
{
    int retval;
    size_t i;
    Record filt;

    LOG4CXX_TRACE(iLogger::GetLogger(), "SqliteStorage::Set(RecordSet)");
    filt.Clear();
    retval = 0;
    for (i = 0; i < data.size(); i++)
    {
        filt.objectID = data[i].objectID;
        retval += Set(filt, data[i]);
    }

    return retval;
}

int SqliteStorage::Delete(Record& filter)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "SqliteStorage::Delete; objType=" << filter.objectID);
    /// @todo Implement this
    LOG4CXX_FATAL(iLogger::GetLogger(), "SqliteStorage::Delete not implemented!");
    return 0;
}
