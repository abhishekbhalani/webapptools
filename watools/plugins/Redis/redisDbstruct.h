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
#ifndef __REDISDBSTRUCT_H__
#define __REDISDBSTRUCT_H__

#pragma once
#include <weDbstruct.h>
#include "redisclient.h"

using namespace webEngine;

class redis_recordset : public db_recordset {
public:
    redis_recordset() {
        records.resize(1);
    }
    recordset_& data() {
        return records;
    }
};

class redis_cursor : public db_cursor_base {
public:
    redis_cursor();
    redis_cursor(redis::client* dbcli, string nspace, size_t idx);
    redis_cursor(const redis_cursor& cpy);
    ~redis_cursor();

    redis_cursor& operator=(const redis_cursor& cpy);
    redis_cursor& operator++();

    //@{
    /// @fn we_variant& operator[](string name)
    ///
    /// @brief  Access to named field.
    ///
    /// @return we_variant - value of fields
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    we_variant& operator[](string name) {
        return db_cursor_base::operator[](name);
    }
    const we_variant& operator[](string name) const {
        return db_cursor_base::operator[](name);
    }
    //@}

    //@{
    /// @fn we_variant& operator[](int index)
    ///
    /// @brief  Access to zero-based indexed field.
    ///
    /// @return we_variant - value of fields
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    we_variant& operator[](int index) {
        return db_cursor_base::operator[](index);
    }
    const we_variant& operator[](int index) const {
        return db_cursor_base::operator[](index);
    }
    //@}

    bool commit();
    const int size();
    const size_t record_size() const {
        return rec_sz;
    }
    const string& get_last_error() const {
        return last_err;
    }
    const vector<string>& field_names() const {
        return fnames;
    };
    const size_t row_id() const {
        return rowid;
    }

    const bool operator==(const redis_cursor& rhs) const;
    const bool operator!=(const redis_cursor& rhs) const {
        return !(this->operator==( rhs ));
    }

    static redis_cursor begin(redis::client* dbcli, string nspace);
    static redis_cursor end(redis::client* dbcli, string nspace);

protected:
    const size_t get_record_size();
    const bool is_valid_rowid(size_t id) const;

    redis_recordset* base;
    redis::client* db_cli;
    string name_space;
    string last_err;
    vector<string> fnames;
    int rowid;
    size_t rec_sz;
};

#endif // __REDISDBSTRUCT_H__
