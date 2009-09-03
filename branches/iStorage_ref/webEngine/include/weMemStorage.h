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
/// @brief  Memory storage class. Provides iStorage interface with keeping data in memory
///         and abilities to save/load state into the file(s)
///
/// @author A. Abramov
/// @date   14.07.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class MemStorage :
    public iStorage
{
public:
    MemStorage(Dispatch* krnl, void* handle = NULL);
    ~MemStorage(void);

    // iwePlugin functions
    virtual void* GetInterface(const string& ifName);

    // iStorage functions
    virtual bool InitStorage(const string& params);
    virtual void Flush(const string& params = "");
    virtual int Query(const string& objType, const string& objId, Operation op, const string& xmlData);
    virtual int Report(const string& repType, const string& objId, const string& xmlData, string& result);

    // MemStorage functions
    void Save(const string& fileName);
    void Load(const string& fileName);

protected:
    string fileName;
    StringMap tasks;
    StringMap dicts;
    StringMap auths;
    StringMap sysopts;
};

} // namespace webEngine

#endif //__WEMEMSTORAGE_H__
