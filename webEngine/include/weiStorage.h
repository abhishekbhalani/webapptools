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
    public i_plugin {
public:
    i_storage(engine_dispatcher* krnl, void* handle = NULL);
    virtual ~i_storage(void);

    /** @{ @brief inherited from i_plugin */
    virtual i_plugin* get_interface(const string& ifName);
    virtual void init(task*) {}
    virtual void pause(task*, bool = true) {}
    virtual void stop(task*) {}
    /** @} */

    /** @brief initialize storage
     *
     * In order to i_plugin initialization it can open database file, create connection, allocate memory, or something else.
     * @param const string & params Initialization string for storage. For example in soci_storage it can be "sqlite3://database.db"
     * @return bool - true if successful
     */
    virtual bool init_storage(const string&) {
        return false;
    };

    /** @brief Flushing cached data. */
    virtual void flush(const string& = "") {};
    /** @brief Generate unique id for some reason*/
    virtual string generate_id(const string& = "");

    /**
    * @{
    * @param const db_filter & query  - filter for data in storage, in SQL terms its WHERE statement
    * @param const std::vector<std::string> & fields - list of fields in db_record
    * @param const std::string & ns - namespace to get list of fields
    */


    /**
    * @{
    * @param bool need_blob - set this parameter to blob if you using blob in record set.
    * Restrictions: In Soci_storage you can use only one blob field per
    * record and it must be first in record.
    */

    /**
     * @{
     * @brief Creates iterator cursor for browsing a dataset given by a filter.
     */
    virtual db_cursor get(const db_filter &query, const std::string& ns, bool need_blob = false)	{
        return get(query, get_namespace_struct(ns), need_blob);
    }
    virtual db_cursor get(const db_filter &query, const std::vector<std::string> &fields, bool need_blob = false) {
        return get(sql_constructor::get_sql_select(query, fields), fields, need_blob);
    }
    /** @} */

    /**
    * @{
    * @brief Creates iterator cursor for set (update or insert) data in storage.
     */
    virtual db_cursor set(const db_filter &query, const std::string& ns, bool need_blob = false)	{
        return set(query, get_namespace_struct(ns), need_blob);
    }
    virtual db_cursor set(const db_filter &query, const std::vector<std::string> &fields, bool need_blob = false) {
        return set(sql_constructor::get_sql_update(query, fields), sql_constructor::get_sql_insert(fields), fields, need_blob);
    }
    /** @} */

    /**
    * @{
    * @brief Creates iterator cursor for insert (update or insert) data in storage.
     */
    virtual db_cursor ins(const std::string& ns, bool need_blob = false)	{
        return ins(get_namespace_struct(ns), need_blob);
    }
    virtual db_cursor ins(const std::vector<std::string> &fields, bool need_blob = false) {
        return ins(sql_constructor::get_sql_insert(fields), fields, need_blob);
    }
    /** @} */
    /** @} */


    /**
     * @brief Delete data from storage.
     * @return int  -  affected rows
     */
    virtual int del(const db_filter &query)	{
        return del(sql_constructor::get_sql_delete(query));
    }

    /**
    * @{
     * @brief Count filtered data in storage.
     */
    virtual int count(const db_filter &query, const std::string& ns)	{
        return count(query, get_namespace_struct(ns));
    }
    virtual int count(const db_filter &query, const std::vector<std::string> &fields) {
        return count(sql_constructor::get_sql_count(query, fields));
    }
    /** @} */
    /** @} */

protected:
    static std::vector<std::string> get_namespace_struct(const std::string& ns);

    virtual db_cursor get(const string &query, const std::vector<std::string> &fields, bool need_blob = false) = 0;
    virtual db_cursor set(const string &query_update, const string &query_insert, const std::vector<std::string> &fields, bool need_blob = false) = 0;
    virtual db_cursor ins(const string &query_insert, const std::vector<std::string> &fields, bool need_blob = false) = 0;
    virtual int count(const string &query) = 0;
    virtual int del(const string &query) = 0;

    int last_id;
};

extern const char* idb_struct[];
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
#define weObjTypeVulnerDesc "vulner_desc"

#endif //__IWESTORAGE_H__
