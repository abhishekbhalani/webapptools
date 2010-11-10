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
#ifndef __SOCISTORAGE_H__
#define __SOCISTORAGE_H__

#pragma once
#include <weiStorage.h>
#include <boost/thread.hpp>

using namespace webEngine;

namespace soci {
class session;
class statement;
}

namespace soci_db {
namespace details {
class db_cursor_base;
class db_cursor_get;
class db_cursor_set;
class db_cursor_ins;
}

class soci_storage :
    public i_storage {
public:
    soci_storage(engine_dispatcher* krnl, void* handle = NULL);
    virtual ~soci_storage(void);

    // iwePlugin functions
    virtual i_plugin* get_interface(const string& ifName);

    // i_storage functions
    virtual bool init_storage(const string& params);
    virtual void flush(const string& params = "") {};
    virtual string generate_id(const string& objType = "");

protected:
    virtual db_cursor get(const string &query, const std::vector<std::string> &fields, bool need_blob = false);
    virtual db_cursor set(const string &query_update, const string &query_insert, const std::vector<std::string> &fields, bool need_blob = false);
    virtual db_cursor ins(const string &query_insert, const std::vector<std::string> &fields, bool need_blob = false);
    virtual int del(const string &query);
    virtual int count(const string &query);

private:
    void get_next(details::db_cursor_get &cursor);
    void set_next(details::db_cursor_set &cursor);
    void ins_next(details::db_cursor_ins &cursor);
    void free_statement(boost::shared_ptr<soci::statement> st);

protected:
    boost::mutex data_access;
    std::set<boost::shared_ptr<soci::statement> > m_statements;
    boost::scoped_ptr<soci::session> m_sql;

    friend class details::db_cursor_base;
    friend class details::db_cursor_get;
    friend class details::db_cursor_set;
    friend class details::db_cursor_ins;
};
}
#endif //__SOCISTORAGE_H__
