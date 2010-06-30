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
#ifndef __WEMEMSTORAGE_H__
#define __WEMEMSTORAGE_H__

#pragma once
#include <weiStorage.h>
#include <boost/thread.hpp>

namespace webEngine {

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  mem_storage :
///
/// @brief  Memory storage_db class. Provides i_storage interface with keeping data in memory
///         and abilities to save/load state into the file(s)
///
/// @author A. Abramov
/// @date   14.07.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class mem_storage :
    public i_storage
{
public:
    mem_storage(engine_dispatcher* krnl, void* handle = NULL);
    ~mem_storage(void);

    // iwePlugin functions
    virtual i_plugin* get_interface(const string& ifName);

    // i_storage functions
    virtual bool init_storage(const string& params);
    virtual void flush(const string& params = "");
    virtual int get(db_query& query, db_recordset& results);
    virtual int set(db_query& query, db_recordset& data);
    virtual int set(db_recordset& data);
    virtual int del(db_filter& filter);

    // mem_storage functions
    void save_db(const string& fname);
    void load_db(const string& fname);

protected:
    typedef map<string, db_recordset*> mem_tables;

    boost::mutex data_access;
    string file_name;
    mem_tables name_spaces;
};

} // namespace webEngine

#endif //__WEMEMSTORAGE_H__
