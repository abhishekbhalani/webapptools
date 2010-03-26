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
    public i_storage
{
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
    virtual void* get_interface(const string& ifName);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual const string get_setup_ui( void )
    ///
    /// @brief  Gets the user interface for the setup dialog.
    ///
    /// @retval The user interface in the XRC format (wxWidgets) or empty string if no setup dialog.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual const string get_setup_ui( void );

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual void apply_settings( const string& xmlData )
    ///
    /// @brief  Applies the settings described by xmlData.
    ///
    /// @param  xmlData	 - Plugin settings described by the XML.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void apply_settings( const string& xmlData );

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
    /// @fn int get( Record& filters, Record& respFilter, RecordSet& results)
    ///
    /// @brief  Gets the RecordSet from given namespace (objType). The response filtered to
    ///         equality of the selected field to the given values. The response will contains only
    ///         the fields included into the given @b respFilter structure.
    ///
    /// @param  filter          - the Record to filter the request 
    /// @param  respFilter      - set of field to be placed into result. If empty - all data will be retrieved
    /// @param  [out] results   - the RecordSet to fill it with results 
    ///
    /// @retval number of records in the response. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual int get(db_record& filter, db_record& respFilter, db_recordset& results);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn int set(Record& filters, Record& data)
    ///
    /// @brief	Stores (updates) the data. @b data may contain subset of fields
    ///         (not the full description of the object), and non-empty @b filters may be used to
    ///         update selected object(s).
    ///
    /// @param  filter  - the Record to select object(s) for update 
    /// @param  data    - the Record to be stored 
    ///
    /// @retval	Number of affected records. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual int set(db_record& filter, db_record& data);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn int set(RecordSet& data)
    ///
    /// @brief	Stores (updates) the data. @b data may contain subset of fields
    ///         (not the full description of the object).
    ///
    /// @param  data - the RecordSet to be stored 
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
    virtual int del(db_record& filter);

protected:
    string_list* search_db(db_record& filter, bool all = false);
    string_list* get_struct(const string& nspace);
    void fix_struct(db_record& strt);
    string_list* get_namespace_idxs(const string& objType);

    string plugin_version;
    string db_host;
    int db_port;
    string db_auth;
    int db_index;
    redis::client* db_cli;
    boost::mutex db_lock;
};
