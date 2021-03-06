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
#ifndef __WEDISPATCH_H__
#define __WEDISPATCH_H__
#pragma once
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include "weOptions.h"
#include "weiPlugin.h"
#include "weiStorage.h"
#include "weiTransport.h"

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
    public i_storage {
public:
    null_storage(engine_dispatcher* krnl, void* handle = NULL);
    ~null_storage(void);

    // i_plugin functions
    virtual i_plugin* get_interface(const string& ifName);

    // i_storage functions
    virtual bool init_storage(const string& ) {
        return true;
    };
    virtual void flush(const string& = "") {
        return;
    };

    virtual db_cursor get(const string &, const std::vector<std::string> &, bool = false) {
        return db_cursor();
    }
    virtual db_cursor set(const string &, const string &, const std::vector<std::string> &, bool = false) {
        return db_cursor();
    }
    virtual db_cursor ins(const string &, const std::vector<std::string> &, bool = false) {
        return db_cursor();
    }
    virtual int del(const string &) {
        return 0;
    }
    virtual int count(const string &) {
        return 0;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  plugin_factory
///
/// @brief  The i_plugin creator for "in-memory" plugins.
///
/// @author A. Abramov
/// @date   10.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class plugin_factory {
public:
    plugin_factory();
    ~plugin_factory();
    void add_plugin_class(string name, fnWePluginFactory func);
    void* create_plugin(string pluginID, engine_dispatcher* krnl);

private:
    std::map<string, fnWePluginFactory> factories_;
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
    public i_options_provider,
        boost::noncopyable {
public:
    engine_dispatcher(void);
    virtual ~engine_dispatcher(void);

    // engine_dispatcher
    // Access the storage
    i_storage* storage(void) const  {
        return(plg_storage);
    };
    void storage(const i_storage* store);

    void flush();

    // Access the plugin_list
    const plugin_list &get_plugin_list(void) const  {
        return(plg_list);
    };
    void refresh_plugin_list(boost::filesystem::path& baseDir);
    void refresh_plugin_list();

    // Provide Logger to plugins
    log4cxx::LoggerPtr get_logger() {
        return iLogger::GetLogger();
    };

    i_plugin* load_plugin(string id);
    i_plugin* get_interface(string iface);
    void add_plugin_class(string name, fnWePluginFactory func);

    //
    virtual we_option Option(const string& name);
    virtual void Option(const string& name, we_variant val);
    virtual bool IsSet(const string& name);
    virtual void Erase(const string& name);
    virtual void Clear();
    virtual string_list OptionsList();
    virtual size_t OptionSize();

    typedef i_request_ptr request_constructor(we_iarchive&);

    template<class Request>
    void register_request() {
        m_request_map[typeid(Request).name()] = &Request::restore_request;
    }

    i_request_ptr restore_request(const string& request_class, we_iarchive& ar) const;

#ifndef __DOXYGEN__

private:
    std::map<string, request_constructor*> m_request_map;

protected:
    plugin_factory plg_factory;
    plugin_list plg_list;
    i_storage* plg_storage;
#endif // __DOXYGEN__
};

}; // namespace webEngine

#endif //__WEDISPATCH_H__
