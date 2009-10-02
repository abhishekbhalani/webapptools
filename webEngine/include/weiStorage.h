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

namespace webEngine {

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  Record
///
/// @brief  Set of fields (individual data) as name-value pairs.
///
/// @author A. Abramov
/// @date	03.09.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class Record : public iOptionsProvider
{
public:
    /// namespace of the object
    string objectID;
    // nothing special
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  RecordSet
///
/// @brief  Set of records of class Record.
///
/// @author A. Abramov
/// @date	03.09.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class RecordSet : public vector<Record>
{
    // nothing special
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @interface  iStorage
///
/// @brief  Interface for storage subsystem.
///
/// @author A. Abramov, A. Yudin
/// @date	14.07.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class iStorage :
    public iPlugin
{
public:
    iStorage(Dispatch* krnl, void* handle = NULL);
    virtual ~iStorage(void);

    // iPlugin functions
    virtual void* GetInterface(const string& ifName);

    // iStorage functions
    typedef enum { mask = 0xff} Operation;
    static const Operation insert = (Operation)0x01;
    static const Operation update = (Operation)0x02;
    static const Operation remove = (Operation)0x03;
    static const Operation autoop = (Operation)0xff;

    virtual bool InitStorage(const string& params) {return false;};
    virtual void Flush(const string& params = "") {};
    virtual string GenerateID(const string& objType = "");

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn int Get( Record& filters, Record& respFilter, RecordSet& results)
    ///
    /// @brief  Gets the RecordSet from given namespace (objType). The response filtered to
    ///         equality of the selected field to the given values. The response will contains only
    ///         the fields included into the given @b respFilter structure.
    ///
    /// @param  filter          - the Record to filter the request 
    /// @param  respFilter      - Set of field to be placed into result. If empty - all data will be retrieved
    /// @param  [out] results   - the RecordSet to fill it with results 
    ///
    /// @retval number of records in the response. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual int Get(Record& filter, Record& respFilter, RecordSet& results) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn int Set(Record& filters, Record& data)
    ///
    /// @brief	Stores (updates) the data. @b data may contain subset of fields
    ///         (not the full description of the object), and non-empty @b filters may be used to
    ///         update selected object(s).
    ///
    /// @param  filter  - the Record to select object(s) for update 
    /// @param  data    - the Record to be stored 
    ///
    /// @retval	Number of affected records. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual int Set(Record& filter, Record& data) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn int Set(RecordSet& data)
    ///
    /// @brief	Stores (updates) the data. @b data may contain subset of fields
    ///         (not the full description of the object).
    ///
    /// @param  data - the RecordSet to be stored 
    ///
    /// @retval	Number of affected records. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual int Set(RecordSet& data) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn int Delete(Record& filters)
    ///
    /// @brief	Deletes the filtered object(s). 
    ///
    /// @param  filter - the Record to select object(s) for deletion 
    ///
    /// @retval	Number of deleted records. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual int Delete(Record& filter) = 0;

protected:
    static int lastId;
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
