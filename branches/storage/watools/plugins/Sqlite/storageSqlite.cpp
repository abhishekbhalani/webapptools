/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew Abramov aabramov@ptsecurity.ru

    This file is part of webEngine

    webEngine is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "storageSqlite.h"
#include "version.h"
#include "sqliteStorage.xpm"
#include "sqlite3.h"

struct sqlite_handle
{
};

sqlite_storage::sqlite_storage( engine_dispatcher* krnl, void* handle /*= NULL*/ ) :
    i_storage(krnl, handle)
{
    pluginInfo.interface_name = "sqlite_storage";
    pluginInfo.interface_list.push_back("sqlite_storage");
    pluginInfo.plugin_desc = "Sqlite storage";
    pluginInfo.plugin_id = "9FBCC44C52A1";
    pluginInfo.plugin_icon = WeXpmToStringList(sqliteStorage_xpm, sizeof(sqliteStorage_xpm) / sizeof(char*) );
    LOG4CXX_TRACE(logger, "SqliteStorage plugin created");
}

sqlite_storage::~sqlite_storage(void)
{
    LOG4CXX_TRACE(logger, "SqliteStorage plugin destroyed");
}

i_plugin* sqlite_storage::get_interface( const string& ifName )
{
	if (boost::iequals(ifName, "i_storage"))
    {
        usageCount++;
        return ((i_storage*)this);
    }
    if (boost::iequals(ifName, "sqlite_storage"))
    {
        usageCount++;
        return ((sqlite_storage*)this);
    }
    return i_storage::get_interface(ifName);
}

int sqlite_storage::get(db_query& query, db_recordset& results)
{
    LOG4CXX_TRACE(logger, "SqliteStorage::get");
    /// @todo Implement this
    LOG4CXX_FATAL(iLogger::GetLogger(), "SqliteStorage::Get not implemented!");
    return 0;
}

int sqlite_storage::set(db_query& query, db_recordset& data)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "SqliteStorage::Set(db_query, db_recordset)");
    /// @todo Implement this
    LOG4CXX_FATAL(iLogger::GetLogger(), "SqliteStorage::Set not implemented!");
    return 0;
}

int sqlite_storage::set(db_recordset& data)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "SqliteStorage::Set(db_recordset)");
    /// @todo Implement this
    LOG4CXX_FATAL(iLogger::GetLogger(), "SqliteStorage::Set not implemented!");
    return 0;
}

int sqlite_storage::del(db_filter& filter)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "SqliteStorage::Delete;");
    /// @todo Implement this
    LOG4CXX_FATAL(iLogger::GetLogger(), "SqliteStorage::Delete not implemented!");
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn bool sqlite_storage::init_storage( const string& params )
///
/// @brief  Initializes the storage_db. 
///
/// @param  params - Pathname for the storage_db file.
/// @retval	true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool sqlite_storage::init_storage( const string& params )
{
	bool result = false;
    boost::unique_lock<boost::mutex> locker(data_access);

	return result;
}

//void sqlite_storage::flush( const string& params /*= ""*/)
//{
//}
