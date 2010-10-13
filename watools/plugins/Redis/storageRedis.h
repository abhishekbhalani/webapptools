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
#pragma once
#include <boost/thread.hpp>
#include <weiStorage.h>
#include "redisclient.h"

using namespace webEngine;

class redis_storage :
    public i_storage {
public:
    redis_storage(engine_dispatcher* krnl, void* handle = NULL);
    virtual ~redis_storage(void);

    // iwePlugin functions
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual void* get_interface(const string& ifName)
    ///
    /// @brief  Gets an interface.
    ///
    /// Returns abstracted pointer to the requested interface, or NULL if the requested interface isn't
    /// provided by this object. The interface name depends on the plugin implementation.
    ///
    /// @param  ifName - Name of the interface.
    ///
    /// @retval	null if it fails, else the interface.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual i_plugin* get_interface(const string& ifName);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual bool init_storage( const string& params )
    ///
    /// @brief  Initializes internal storage structures.
    ///
    /// @param  params   - Settings (database directory).
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual bool init_storage(const string& params);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual void flush( const string& params )
    ///
    /// @brief  Flushes unsaved data to disk.
    ///
    /// @param  params   - Settings (database directory).
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void flush(const string& params = "");

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual string generate_id( const string& objType )
    ///
    /// @brief  Generates unique ID for the given type of the object.
    ///
    /// @param  objType  - Object type for ID generation.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual string generate_id(const string& objType = "");

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn int get(db_query& query, db_recordset& results)
    ///
    /// @brief  Gets the RecordSet from given namespace (objType). The response filtered to
    ///         equality of the selected field to the given values. The response will contains only
    ///         the fields included into the given @b respFilter structure.
    ///
    /// @param  query           - the database query
    /// @param  results         - result of the query
    ///
    /// @retval number of records in the response.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual int get(db_query& query, db_recordset& results);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn int set(Record& filters, Record& data)
    ///
    /// @brief	Stores (updates) the data. @b data may contain subset of fields
    ///         (not the full description of the object), and non-empty @b filters may be used to
    ///         update selected object(s).
    ///
    /// @param  query   - the filter for update
    /// @param  data    - the db_recordset to be stored
    ///
    /// @retval	Number of affected records.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual int set(db_query& query, db_recordset& data);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn int set(db_recordset& data)
    ///
    /// @brief	Stores (updates) the data. @b data may contain subset of fields
    ///         (not the full description of the object).
    ///
    /// @param  data - the db_recordset to be stored
    ///
    /// @retval	Number of affected records.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual int set(db_recordset& data);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn int del(Record& filters)
    ///
    /// @brief	Deletes the filtered object(s).
    ///
    /// @param  filter - the Record to select object(s) for deletion
    ///
    /// @retval	Number of deleted records.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual int del(db_filter& filter);

protected:
    int insert(string ns, vector<string> fields, db_record& rec);

    string plugin_version;
    string db_host;
    int db_port;
    string db_auth;
    int db_index;
    redis::client* db_cli;
    boost::mutex db_lock;
};
