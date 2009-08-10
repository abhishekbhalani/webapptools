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

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @interface  iweStorage
///
/// @brief  Interface for storage subsystem.
///
/// @author A. Abramov, A. Yudin
/// @date	14.07.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class iweStorage :
    public iwePlugin
{
public:
    iweStorage(WeDispatch* krnl, void* handle = NULL);
    virtual ~iweStorage(void);

    // iwePlugin functions
    virtual void* GetInterface(const string& ifName);

    // iweStorage functions
    typedef enum { mask = 0xff} Operation;
    static const Operation insert = (Operation)0x01;
    static const Operation update = (Operation)0x02;
    static const Operation remove = (Operation)0x03;
    static const Operation autoop = (Operation)0xff;

    virtual bool InitStorage(const string& params) {return false;};
    virtual void Flush(const string& params = "") {};
    virtual string GenerateID(const string& objType = "");

    virtual int Query(const string& objType, const string& objId, Operation op, const string& xmlData);
    virtual int Report(const string& repType, const string& objId, const string& xmlData, string& result);

    virtual int Delete(const string& objType, const string& xmlData);

    virtual int TaskSave(const string& xmlData, Operation op = iweStorage::autoop);
    virtual int DictionarySave(const string& xmlData, Operation op = iweStorage::autoop);
    virtual int AuthorizationSave(const string& xmlData, Operation op = iweStorage::autoop);
    virtual int SystemOptionsSave(const string& xmlData, Operation op = iweStorage::autoop);
    virtual int ScanSave(const string& xmlData, Operation op = iweStorage::autoop);

    virtual int TaskReport(const string& xmlData, string& result);
    virtual int DictionaryReport(const string& xmlData, string& result);
    virtual int AuthorizationReport(const string& xmlData, string& result);
    virtual int SystemOptionsReport(const string& xmlData, string& result);
    virtual int ScanReport(const string& xmlData, string& result);

protected:
    virtual int ObjectReport(const string& objType, const string& xmlData, string& result);
    virtual int ObjectQuery(const string& objType, const string& xmlData, Operation op = iweStorage::autoop);
    static int lastId;
};

#define weObjTypeTask       "task"
#define weObjTypeSysOption  "sysopt"
#define weObjTypeDictionary "dict"
#define weObjTypeAuthInfo   "auth"
#define weObjTypeScan       "scan"
#define weObjTypeObject     "object"
#define weObjTypeProfile    "profile"

#endif //__IWESTORAGE_H__
