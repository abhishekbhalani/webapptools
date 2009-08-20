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
#ifndef __WEDISPATCH_H__
#define __WEDISPATCH_H__
#pragma once
#include <boost/filesystem.hpp>
#include "weOptions.h"
#include "weiPlugin.h"
#include "weMemStorage.h"


namespace webEngine {

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  NullStorage
///
/// @brief  null storage for the iStorage placeholder.
///
/// @author A. Abramov
/// @date   23.07.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class NullStorage :
    public iStorage
{
public:
    NullStorage(Dispatch* krnl, void* handle = NULL);
    ~NullStorage(void);

    // iPlugin functions
    virtual void* GetInterface(const string& ifName);

    // iStorage functions
    virtual bool InitStorage(const string& params) { return true; };
    virtual void Flush(const string& params = "") { return; };
    virtual int Query(const string& objType, const string& objId, Operation op, const string& xmlData) { return 0; };
    virtual int Report(const string& repType, const string& objId, const string& xmlData, string& result) { return 0; };
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  Dispatch
///
/// @brief  Dispatcher for tasks processing and system options storage.
///
/// @author A. Abramov
/// @date   16.07.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class Dispatch :
    public iOptionsProvider
{
public:
    Dispatch(void);
    virtual ~Dispatch(void);

    // Dispatch
    // Access the Storage
    iStorage* Storage(void) const  { return(storage);  };
    void Storage(const iStorage* store);

    void Flush();

    // Access the PluginList
    const WePluginList &PluginList(void) const  { return(pluginList); };
    void RefreshPluginList(boost::filesystem::path& baseDir);

    // Provide Logger to plugins
    log4cxx::LoggerPtr GetLogger() { return iLogger::GetLogger(); };

    iPlugin* LoadPlugin(string id);

#ifndef __DOXYGEN__
protected:
    WePluginList pluginList;
    iStorage* storage;

private:
    Dispatch(Dispatch&) {};               ///< Avoid object copying
    Dispatch& operator=(Dispatch&) { return *this; };    ///< Avoid object copying
#endif // __DOXYGEN__
};

}; // namespace webEngine

#endif //__WEDISPATCH_H__
