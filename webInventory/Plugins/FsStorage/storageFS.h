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

class FsStorage :
    public iweStorage
{
public:
    FsStorage(WeDispatch* krnl, void* handle = NULL);
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
    /// @fn virtual int Query( const string& objType,
    ///             const string& objId, Operation op, const string& xmlData )
    ///
    /// @brief  Perform query (save, update, delete) to the given type of the storage.
    ///
    /// @param  objType  - Object type.
    /// @param  objId    - Object identifier.
    /// @param  op       - Query operation. Can be 'autoop'.
    /// @param  xmlData	 - Query details described by the XML.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual int Query(const string& objType, const string& objId, Operation op, const string& xmlData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual int Report( const string& objType,
    ///             const string& objId, const string& xmlData, string& result )
    ///
    /// @brief  Builds report for the given type of the storage.
    ///
    /// @param  repType  - Object type.
    /// @param  objId    - Object identifier.
    /// @param  xmlData	 - Plugin settings describing the XML.
    /// @param  result   - Plugin settings describing the XML.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual int Report(const string& repType, const string& objId, const string& xmlData, string& result);
    
protected:
    string FileRead(const string& fname);
    int    FileRemove(const fs::path& fspath, const string& fname);
    int    FileSave(const fs::path& fspath, const string& fname, const string& content);

    string db_dir;
    log4cxx::LoggerPtr logger;
};
