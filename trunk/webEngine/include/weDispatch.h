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
/// @class  null_storage
///
/// @brief  null storage for the i_storage placeholder.
///
/// @author A. Abramov
/// @date   23.07.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class null_storage :
    public i_storage
{
public:
    null_storage(engine_dispatcher* krnl, void* handle = NULL);
    ~null_storage(void);

    // i_plugin functions
    virtual void* get_interface(const string& ifName);

    // i_storage functions
    virtual bool init_storage(const string& params) { return true; };
    virtual void flush(const string& params = "") { return; };
    virtual int get(db_record& filter, db_record& respFilter, db_recordset& results);
    virtual int set(db_record& filter, db_record& data);
    virtual int set(db_recordset& data);
    virtual int del(db_record& filter);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  plugin_factory
///
/// @brief  The i_plugin creator for "in-memory" plugins. 
///
/// @author A. Abramov
/// @date   10.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class plugin_factory : public linked_list<string, fnWePluginFactory>
{
public:
    plugin_factory();
    void add_plugin_class(string name, fnWePluginFactory func);
    void* create_plugin(string pluginID, engine_dispatcher* krnl);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  engine_dispatcher
///
/// @brief  Dispatcher for tasks processing and system options storage.
///
/// @author A. Abramov
/// @date   16.07.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class engine_dispatcher :
    public iOptionsProvider
{
public:
    engine_dispatcher(void);
    virtual ~engine_dispatcher(void);

    // engine_dispatcher
    // Access the storage
    i_storage* storage(void) const  { return(plg_storage);  };
    void storage(const i_storage* store);

    void flush();

    // Access the plugin_list
    const plugin_list &get_plugin_list(void) const  { return(plg_list); };
    void refresh_plugin_list(boost::filesystem::path& baseDir);

    // Provide Logger to plugins
    log4cxx::LoggerPtr get_logger() { return iLogger::GetLogger(); };

    i_plugin* load_plugin(string id);

#ifndef __DOXYGEN__
protected:
    plugin_factory plg_factory;
    plugin_list plg_list;
    i_storage* plg_storage;

private:
    engine_dispatcher(engine_dispatcher&) {};               ///< Avoid object copying
    engine_dispatcher& operator=(engine_dispatcher&) { return *this; };    ///< Avoid object copying
#endif // __DOXYGEN__
};

}; // namespace webEngine

#endif //__WEDISPATCH_H__
