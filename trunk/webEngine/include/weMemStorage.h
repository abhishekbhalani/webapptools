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
#ifndef __WEMEMSTORAGE_H__
#define __WEMEMSTORAGE_H__

#pragma once
#include "weiStorage.h"

namespace webEngine {

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  MemStorage :
///
/// @brief  Memory storage class. Provides i_storage interface with keeping data in memory
///         and abilities to save/load state into the file(s)
///
/// @author A. Abramov
/// @date   14.07.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class MemStorage :
    public i_storage
{
public:
    MemStorage(engine_dispatcher* krnl, void* handle = NULL);
    ~MemStorage(void);

    // iwePlugin functions
    virtual void* GetInterface(const string& ifName);

    // i_storage functions
    virtual bool init_storage(const string& params);
    virtual void flush(const string& params = "");
    virtual int get(db_record& filter, db_record& respFilter, db_recordset& results);
    virtual int set(db_record& filter, db_record& data);
    virtual int set(db_recordset& data);
    virtual int del(db_record& filter);

    // MemStorage functions

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn string_list* Search(const string& objType,
    /// 	db_record& filters)
    ///
    /// @brief  Searches database for objects. 
    ///
    /// @param  filters - the db_record to filter the request 
    ///
    /// @retval List of object ID's 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    string_list* Search(db_record& filter);
    string_list* GetNamespaceIdxs(const string& objType);
    void SetNamespaceIdxs(const string& objType, string_list* lst);
    void FixNamespaceStruct(db_record& filter);
    string_list* GetNamespaceStruct(db_record& filter);
    void Save(const string& fileName);
    void Load(const string& fileName);

protected:
    string fileName;
    StringMap storage;
};

} // namespace webEngine

#endif //__WEMEMSTORAGE_H__
