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
#ifndef __IWESTORAGE_H__
#define __IWESTORAGE_H__

#pragma once
#include "weiPlugin.h"
#include "weOptions.h"
#include <boost/utility.hpp>

namespace webEngine {

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  db_record
///
/// @brief  set of fields as zero-based array of values.
///
/// @author A. Abramov
/// @date	09.06.2010
////////////////////////////////////////////////////////////////////////////////////////////////////
class db_record : public vector< we_variant >
{
public:
    db_record() : vector< we_variant>() {}
    db_record(size_t sz) : vector< we_variant>() { reserve(sz); }
};

/*///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  db_fw_cursor
///
/// @brief  Provide forward-only, read-only access to records and fields in the recordset.
///
/// @author A. Abramov
/// @date	09.06.2010
////////////////////////////////////////////////////////////////////////////////////////////////////
class db_fw_cursor
{
public:
    db_fw_cursor();
    ~db_fw_cursor();

    // access to current record fields 
    const we_variant& operator[](string name);
    const we_variant& operator[](int index);

    // move cursor thought recordset
    db_fw_cursor& operator++();
    db_fw_cursor& operator++(int);
    db_fw_cursor& operator+=(size_t offs);

    // test cursor
    operator bool() const;
    bool operator==(const db_fw_cursor& other) const;
    bool operator!=(const db_fw_cursor& other) const;
    bool operator>=(const db_fw_cursor& other) const;
    bool operator<=(const db_fw_cursor& other) const;
    bool operator>(const db_fw_cursor& other) const;
    bool operator<(const db_fw_cursor& other) const;

protected:
};

class db_cursor
{
public:
    db_cursor();
    db_cursor(const db_cursor& c);
    ~db_cursor();

    // move cursor thought recordset
    db_cursor& operator++();
    db_cursor& operator++(int);
    db_cursor& operator+=(size_t offs);
    db_cursor& operator--();
    db_cursor& operator--(int);
    db_cursor& operator-=(size_t offs);

    // test cursor
    operator bool() const;
    bool operator==(const db_cursor& other) const;
    bool operator!=(const db_cursor& other) const;
    bool operator>=(const db_cursor& other) const;
    bool operator<=(const db_cursor& other) const;
    bool operator>(const db_cursor& other) const;
    bool operator<(const db_cursor& other) const;

protected:
};*/
class db_cursor : public vector<db_record>::iterator {
public:
    db_cursor() : vector<db_record>::iterator(), parent(NULL) {}
    db_cursor(const db_cursor& cp);
    explicit db_cursor(db_recordset* rs, vector<db_record>::iterator it);

    db_cursor& operator=(const db_cursor& cp);
    // access to current record fields 
    we_variant& operator[](string name);
    we_variant& operator[](int index);
protected:
    db_recordset* parent;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  db_recordset
///
/// @brief  array of db_records. Implement iterations, structure control and others.
///
/// @author A. Abramov
/// @date	09.06.2010
////////////////////////////////////////////////////////////////////////////////////////////////////
class db_recordset : public boost::noncopyable
{
public:

public:
    db_recordset();
    explicit db_recordset(vector<string> fld_names);
    explicit db_recordset(db_record& rec);
    ~db_recordset();

    void erase(db_cursor& at);
    void push_back(db_record& rec);
    void insert(db_cursor& before, db_record& rec);

    const size_t size() { return records.size(); }
    db_cursor begin();
    db_cursor end();

protected:
    friend class db_cursor;

    vector<string> field_names;
    vector<db_record> records;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  db_condition
///
/// @brief  atomic logical operation for DB queries.
///
/// @author A. Abramov
/// @date	09.06.2010
////////////////////////////////////////////////////////////////////////////////////////////////////
class db_condition
{
public:
    db_condition() {}
    db_condition(string s) {}
    db_condition(const db_condition& c) {}
    ~db_condition() {}

    string& field() { return field_name; }
    string& operation() { return op_code; }
    we_variant& value() { return test_value; }

protected:
    string field_name;
    string op_code;
    we_variant test_value;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  db_filter
///
/// @brief  logical set of conditions and filters
///
/// @author A. Abramov
/// @date	09.06.2010
////////////////////////////////////////////////////////////////////////////////////////////////////
class db_filter
{
public:
    db_filter() {}
    db_filter(const db_filter& c) {}
    explicit db_filter(const db_condition& cond);

    db_filter& set(db_condition& cond);
    db_filter& set(db_filter& cond);
    db_filter& or(db_condition& cond);
    db_filter& or(db_filter& cond);
    db_filter& and(db_condition& cond);
    db_filter& and(db_filter& cond);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  db_query
///
/// @brief  set of data for execute DB query
///
/// @author A. Abramov
/// @date	09.06.2010
////////////////////////////////////////////////////////////////////////////////////////////////////
class db_query
{
public:
    db_query();

    db_filter& where() { return filter; }
    vector<string>& what() { return output; }
    db_recordset& values() { return data_set; }

protected:
    db_filter filter;
    vector<string> output;
    db_recordset data_set;
};

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
    /// @fn int get( db_record& filter, db_record& respFilter, db_recordset& results)
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
    /// @fn int set(db_record& filter, db_record& data)
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
    /// @fn int del(db_record& filter)
    ///
    /// @brief	Deletes the filtered object(s). 
    ///
    /// @param  filter - the db_query to select object(s) for deletion 
    ///
    /// @retval	Number of deleted records. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual int del(db_query& filter) = 0;

protected:
    static int last_id;
};

} // namespace webEngine

#define weObjTypeTask       "task"
#define weObjTypeSysOption  "sysopt"
#define weObjTypeDictionary "dict"
#define weObjTypeAuthInfo   "auth"
#define weObjTypeScan       "scan"
#define weObjTypeScanData   "scandata"
#define weObjTypeObject     "object"
#define weObjTypeProfile    "profile"
#define weObjTypeVulner     "vulners"
#define weObjTypeVulnerDesc "vilndesc"

#endif //__IWESTORAGE_H__
