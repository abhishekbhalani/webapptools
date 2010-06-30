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
#ifndef __IWESTORAGE_H__
#define __IWESTORAGE_H__

#pragma once
#include "weiPlugin.h"
#include "weOptions.h"
#include "weDbstruct.h"

namespace webEngine {

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @interface  i_storage
///
/// @brief  Interface for storage subsystem.
///
/// @author A. Abramov
/// @date	14.07.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class i_storage :
    public i_plugin
{
public:
    i_storage(engine_dispatcher* krnl, void* handle = NULL);
    virtual ~i_storage(void);

    // i_plugin functions
    virtual i_plugin* get_interface(const string& ifName);
    virtual void init(task* tsk) {}
    virtual void pause(task* tsk, bool paused = true) {}
    virtual void stop(task* tsk) {}

    virtual bool init_storage(const string& params) {return false;};
    virtual void flush(const string& params = "") {};
    virtual string generate_id(const string& objType = "");

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn int get( db_query& query, db_recordset& results)
    ///
    /// @brief  Gets the db_recordset from given namespace (objType). The response filtered to
    ///         equality of the selected field to the given values. The response will contains only
    ///         the fields included into the given @b respFilter structure.
    ///
    /// @param  query           - the db_query to perform request 
    /// @param  [out] results   - the db_recordset to fill it with results 
    ///
    /// @retval number of records in the response. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual int get(db_query& query, db_recordset& results) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn int set(db_query& query, db_recordset& data)
    ///
    /// @brief	Stores (updates) the data. @b data may contain subset of fields
    ///         (not the full description of the object), and non-empty @b filters may be used to
    ///         update selected object(s).
    ///
    /// @param  query   - the db_query to select object(s) for update 
    /// @param  data    - the db_recordset to be stored 
    ///
    /// @retval	Number of affected records. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual int set(db_query& query, db_recordset& data) = 0;

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
    virtual int set(db_recordset& data) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn int del(db_filter& filter)
    ///
    /// @brief	Deletes the filtered object(s). 
    ///
    /// @param  filter - the db_filter to select object(s) for deletion 
    ///
    /// @retval	Number of deleted records. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual int del(db_filter& filter) = 0;

    static std::vector<std::string> get_namespace_struct(const std::string& ns);

protected:
    int last_id;
};

extern char* idb_struct[];
} // namespace webEngine

#define weObjTypeTask       "task"
#define weObjTypeSysOption  "sysopt"
#define weObjTypeDictionary "dict"
#define weObjTypeAuthInfo   "auth"
#define weObjTypeScan       "scan"
#define weObjTypeScanData   "scan_data"
#define weObjTypeObject     "object"
#define weObjTypeProfile    "profile"
#define weObjTypeVulner     "vulners"
#define weObjTypeVulnerDesc "vilndesc"

#endif //__IWESTORAGE_H__
