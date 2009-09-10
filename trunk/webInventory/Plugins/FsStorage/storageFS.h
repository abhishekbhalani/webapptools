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
#pragma once
#include <weiStorage.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

namespace fs = boost::filesystem;
using namespace webEngine;

class FsStorage :
    public iStorage
{
public:
    FsStorage(Dispatch* krnl, void* handle = NULL);
    virtual ~FsStorage(void);

    // iwePlugin functions
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual void* GetInterface(const string& ifName)
    ///
    /// @brief  Gets an interface.
    ///
    /// Returns abstracted pointer to the requested interface, or NULL if the requested interface isn't
    /// provided by this object. The interface name depends on the plugin implementation.
    ///
    /// @param  ifName - Name of the interface.
    ///
    /// @retval	null if it fails, else the interface.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void* GetInterface(const string& ifName);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual const string GetSetupUI( void )
    ///
    /// @brief  Gets the user interface for the setup dialog.
    ///
    /// @retval The user interface in the XRC format (wxWidgets) or empty string if no setup dialog.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual const string GetSetupUI( void );

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual void ApplySettings( const string& xmlData )
    ///
    /// @brief  Applies the settings described by xmlData.
    ///
    /// @param  xmlData	 - Plugin settings described by the XML.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void ApplySettings( const string& xmlData );

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual bool InitStorage( const string& params )
    ///
    /// @brief  Initializes internal storage structures.
    ///
    /// @param  params   - Settings (database directory).
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual bool InitStorage(const string& params);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual void Flush( const string& params )
    ///
    /// @brief  Flushes unsaved data to disk.
    ///
    /// @param  params   - Settings (database directory).
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void Flush(const string& params = "");

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual string GenerateID( const string& objType )
    ///
    /// @brief  Generates unique ID for the given type of the object.
    ///
    /// @param  objType  - Object type for ID generation.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
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
    virtual int Get(Record& filter, Record& respFilter, RecordSet& results);

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
    virtual int Set(Record& filter, Record& data);

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
    virtual int Set(RecordSet& data);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn int Delete(Record& filters)
    ///
    /// @brief	Deletes the filtered object(s). 
    ///
    /// @param  filter - the Record to select object(s) for deletion 
    ///
    /// @retval	Number of deleted records. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual int Delete(Record& filter);

protected:
    Record* FileRead(const string& fname);
    int FileSave(const fs::path& fspath, const RecordSet& content);
    StringList* GetStruct(const string& nspace);
    void FixStruct(const string& nspace, Record& strt);
    RecordSet* Search(Record& filter, bool all = false);
    void LockDB();
    void UnlockDB();

    string db_dir;
};
