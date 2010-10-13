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

    // i_plugin functions
    virtual i_plugin* get_interface(const string& ifName);
    virtual void init(task* tsk) {}
    virtual void pause(task* tsk, bool paused = true) {}
    virtual void stop(task* tsk) {}

    virtual bool init_storage(const string& params) {
        return false;
    };
    virtual void flush(const string& params = "") {};
    virtual string generate_id(const string& objType = "");

    virtual db_cursor get(const db_filter &query, const std::string& ns, bool need_blob = false)	{
        return get(query, get_namespace_struct(ns), need_blob);
    }
    virtual db_cursor get(const db_filter &query, const std::vector<std::string> &fields, bool need_blob = false) {
        return get(sql_constructor::get_sql_select(query, fields), fields, need_blob);
    }

    virtual db_cursor set(const db_filter &query, const std::string& ns, bool need_blob = false)	{
        return set(query, get_namespace_struct(ns), need_blob);
    }
    virtual db_cursor set(const db_filter &query, const std::vector<std::string> &fields, bool need_blob = false) {
        return set(sql_constructor::get_sql_update(query, fields), sql_constructor::get_sql_insert(fields), fields, need_blob);
    }

    virtual db_cursor ins(const std::string& ns, bool need_blob = false)	{
        return ins(get_namespace_struct(ns), need_blob);
    }
    virtual db_cursor ins(const std::vector<std::string> &fields, bool need_blob = false) {
        return ins(sql_constructor::get_sql_insert(fields), fields, need_blob);
    }

    virtual int del(const db_filter &query)	{
        return del(sql_constructor::get_sql_delete(query));
    }

    virtual int count(const db_filter &query, const std::string& ns)	{
        return count(query, get_namespace_struct(ns));
    }
    virtual int count(const db_filter &query, const std::vector<std::string> &fields) {
        return count(sql_constructor::get_sql_count(query, fields));
    }

protected:
    static std::vector<std::string> get_namespace_struct(const std::string& ns);

    virtual db_cursor get(const string &query, const std::vector<std::string> &fields, bool need_blob = false) = 0;
    virtual db_cursor set(const string &query_update, const string &query_insert, const std::vector<std::string> &fields, bool need_blob = false) = 0;
    virtual db_cursor ins(const string &query_insert, const std::vector<std::string> &fields, bool need_blob = false) = 0;
    virtual int count(const string &query) = 0;
    virtual int del(const string &query) = 0;

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
#define weObjTypeVulnerDesc "vulner_desc"

#endif //__IWESTORAGE_H__
