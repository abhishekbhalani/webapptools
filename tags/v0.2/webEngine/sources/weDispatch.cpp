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
#include <webEngine.h>

#include "externals/shared_object.hpp"
#include "weiBase.h"
#include "weHTTP.h"
#include "weHttpInvent.h"
#include "weDispatch.h"
#include <boost/functional/hash.hpp>

using namespace boost::filesystem;
using namespace webEngine;

#ifndef __DOXYGEN__
static const we_option empty_option("_empty_");
#endif //__DOXYGEN_ _

static string_list find_files ( path baseDir)
{
    string_list files;
    string_list subs;

    files.clear();
    if ( exists( baseDir ) ) {
        directory_iterator end_itr; // default construction yields past-the-end
        for ( directory_iterator itr( baseDir );
            itr != end_itr;
            ++itr )
        {
            if ( is_directory(itr->status()) )
            {
                subs = find_files( itr->path() );
                files.insert(files.end(), subs.begin(), subs.end());
            }
            else if ( ends_with(itr->leaf(), ".plg") ) // see below
            {
                files.push_back(itr->path().string());
            }
        }
    }
    return files;
}

null_storage::null_storage( engine_dispatcher* krnl, void* handle /*= NULL*/ ) :
    i_storage(krnl, handle)
{
    pluginInfo.interface_name = "null_storage";
    pluginInfo.interface_list.push_back("null_storage");
    pluginInfo.plugin_desc = "Placeholder storage";
    pluginInfo.plugin_id = "7CB7A5F18348";
}

null_storage::~null_storage()
{
}

i_plugin* null_storage::get_interface( const string& ifName )
{
    if (iequals(ifName, "i_storage"))
    {
        usageCount++;
        return ((i_storage*)this);
    }
    if (iequals(ifName, "null_storage"))
    {
        usageCount++;
        return ((null_storage*)this);
    }
    return i_storage::get_interface(ifName);
}

int null_storage::get(db_query& query, db_recordset& results)
{
    return 0;
}

int null_storage::set(db_query& query, db_recordset& data)
{
    return 0;
}

int null_storage::set(db_recordset& data)
{
    return 0;
}

int null_storage::del(db_filter& filter)
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////
// engine_dispatcher class
//////////////////////////////////////////////////////////////////////////
engine_dispatcher::engine_dispatcher(void)
{
    plg_storage = new null_storage(this);
    plg_list.clear();
}

engine_dispatcher::~engine_dispatcher(void)
{
    if (plg_storage != NULL)
    {
        flush();
        plg_storage->release();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn void engine_dispatcher::refresh_plugin_list( boost::filesystem::path& baseDir )
///
/// @brief  Refresh plugin list. Rebuilds list of the available plugins even embedded or external
////////////////////////////////////////////////////////////////////////////////////////////////////
void engine_dispatcher::refresh_plugin_list( boost::filesystem::path& baseDir )
{
    plugin_info *info;
    i_plugin *plg;

    plg_list.clear();

    mem_storage memStore(this);
    plg_list.push_back(*(plugin_info*)memStore.info());
    http_transport httpTrans(this);
    plg_list.push_back(*(plugin_info*)httpTrans.info());
    http_inventory httpInvent(this);
    plg_list.push_back(*(plugin_info*)httpInvent.info());
    html_parser htmlParser(this);
    plg_list.push_back(*(plugin_info*)htmlParser.info());

    // search for dynamic libraries with plugin interface
    string_list files = find_files(baseDir.string());
    string_list::iterator fl;
    for (fl = files.begin(); fl != files.end(); fl++)
    {
        // load library, get info, free library
        try
        {
            dyn::shared_object* so = new dyn::shared_object((*fl).c_str());
            fnWePluginFactory ptr = NULL;
            so->get_symbol("WePluginFactory", ptr);
            plg = (i_plugin*)ptr(this, NULL); // not need to store SO link
            if (plg != NULL)
            {
                info = (plugin_info*)plg->info();
                info->plugin_path = *fl;
                plg_list.push_back(*info);
                LOG4CXX_INFO(iLogger::GetLogger(), "engine_dispatcher::refresh_plugin_list: loaded plugin " << *fl);
                LOG4CXX_TRACE(iLogger::GetLogger(), ">>>> ID=" << info->plugin_id << "; Desc=" << info->plugin_desc);
                plg->release();
                plg = NULL;
                //plg_factory.add_plugin_class(info->plugin_id, ptr);
                //plg_factory.add_plugin_class(info->interface_name, ptr);
            }
            else {
                LOG4CXX_WARN(iLogger::GetLogger(), "engine_dispatcher::refresh_plugin_list: can't get information from plugin " <<*fl);
            }
            delete so;
        }
        catch (std::exception& e)
        {
            LOG4CXX_ERROR(iLogger::GetLogger(), "engine_dispatcher::refresh_plugin_list: can't load plugin " << *fl << ". Error: " << e.what());
        }
    }

}


void engine_dispatcher::refresh_plugin_list()
{
    boost::filesystem::path baseDir(".");
    refresh_plugin_list(baseDir);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn i_plugin* engine_dispatcher::load_plugin( string id )
///
/// @brief  Loads a plugin form shared library or internal implementation.
///
/// @param  id    - The identifier.
///
/// @retval null if it fails, else the plugin interface.
////////////////////////////////////////////////////////////////////////////////////////////////////
i_plugin* engine_dispatcher::load_plugin( string id )
{
    i_plugin* retval = NULL;

    LOG4CXX_TRACE(iLogger::GetLogger(), "engine_dispatcher::load_plugin " << id);
    // first step - in-memory plugins
    retval = (i_plugin*)plg_factory.create_plugin(id, this);

    if (retval == NULL) {
        // external plugins
        // go through loop and search plugin
        // if found - loads shared library and create instance
        for (size_t i = 0; i < plg_list.size(); i++)
        {
            if (plg_list[i].plugin_id == id || plg_list[i].interface_name == id) {
                LOG4CXX_TRACE(iLogger::GetLogger(), "engine_dispatcher::load_plugin - found plugin: " << plg_list[i].plugin_desc << "; " << plg_list[i].plugin_path);
                try
                {
                    //char* pth = strdup(plg_list[i].plugin_path.c_str());
                    dyn::shared_object* so = new dyn::shared_object(plg_list[i].plugin_path.c_str());
                    fnWePluginFactory ptr = NULL;
                    so->get_symbol("WePluginFactory", ptr);
                    retval = (i_plugin*)ptr(this, so);
                    //delete pth;
                }
                catch (std::exception& e)
                {
                    LOG4CXX_ERROR(iLogger::GetLogger(), "engine_dispatcher::load_plugin: can't load plugin " << plg_list[i].plugin_path << ". Error: " << e.what());
                }
                break;
            }
        }
    }
    if (retval == NULL)
    {
        LOG4CXX_WARN(iLogger::GetLogger(), "engine_dispatcher::load_plugin: '" << id << "' not found!");
    }
    return retval;
}

void engine_dispatcher::storage( const i_storage* store )
{
    if (plg_storage != NULL)
    {
        plg_storage->release();
    }

    plg_storage = (i_storage*)store;
    if (plg_storage == NULL)
    {
        plg_storage = new null_storage(this);
    }
    LOG4CXX_TRACE(iLogger::GetLogger(), "engine_dispatcher::SetStorage - plugin: " << plg_storage->get_description());
}


void engine_dispatcher::add_plugin_class( string name, fnWePluginFactory func )
{
    size_t i;
    i_plugin* plg = (i_plugin*)func(this, NULL);
    for (i = 0; i < plg_list.size(); i++) {
        if (plg_list[i].plugin_id == plg->info()->plugin_id)
            break;
    }
    if (i == plg_list.size()) {
        plg_list.push_back(*(plugin_info*)plg->info());
    }
    plg_factory.add_plugin_class(name, func);
    delete plg;
}

we_option engine_dispatcher::Option( const string& name )
{
    db_recordset res;
    db_query flt;
    we_option opt;
    char c;
    int i;
    bool b;
    double d;
    string s;
    string parentID = "0";

    LOG4CXX_TRACE(iLogger::GetLogger(), "engine_dispatcher::Option(" << name << ")");

    opt = i_options_provider::empty_option;
    if (plg_storage != NULL) {
        flt.what.clear();
        flt.what.push_back(weObjTypeProfile "." weoTypeID);
        flt.what.push_back(weObjTypeProfile "." weoValue);

        db_condition p_cond;
        db_condition n_cond;

        p_cond.field() = weObjTypeProfile "." weoProfileID;
        p_cond.operation() = db_condition::equal;
        p_cond.value() = parentID;

        n_cond.field() = weObjTypeProfile "." weoName;
        n_cond.operation() = db_condition::equal;
        n_cond.value() = name;

        flt.where.set(p_cond).and(n_cond);
        plg_storage->get(flt, res);

        if (res.size() > 0) {
            db_cursor rec = res.begin();
            try{
                if (!rec[1].empty()) {
                    int tp = boost::lexical_cast<int>(rec[0]);
                    opt.name(name);
                    switch(tp)
                    {
                    case 0: // char
                        c = rec[weObjTypeProfile "." weoValue].get<char>();
                        opt.SetValue(c);
                        break;
                    case 1: // int
                        i = rec[weObjTypeProfile "." weoValue].get<int>();
                        opt.SetValue(i);
                        break;
                    case 2: // bool
                        b = rec[weObjTypeProfile "." weoValue].get<bool>();
                        opt.SetValue(b);
                        break;
                    case 3: // double
                        d = rec[weObjTypeProfile "." weoValue].get<double>();
                        opt.SetValue(d);
                        break;
                    case 4: // string
                        s = rec[weObjTypeProfile "." weoValue].get<string>();
                        opt.SetValue(s);
                        break;
                    default:
                        opt.SetValue(boost::blank());
                    }
                } // if result not <empty>
            }
            catch(std::exception &e) {
                opt = i_options_provider::empty_option;
                LOG4CXX_ERROR(iLogger::GetLogger(), "engine_dispatcher::Option(" << name << ") can't get option value: " << e.what());
            }
        }
    }

    return opt;
}

void engine_dispatcher::Option( const string& name, we_variant val )
{
    db_query flt;
    string parentID = "0";

    LOG4CXX_TRACE(iLogger::GetLogger(), "engine_dispatcher::Option(" << name << ") set value=" << val);
    if (plg_storage != NULL) {
        flt.what.clear();
        flt.what.push_back(weObjTypeProfile "." weoProfileID);
        flt.what.push_back(weObjTypeProfile "." weoName);
        flt.what.push_back(weObjTypeProfile "." weoTypeID);
        flt.what.push_back(weObjTypeProfile "." weoValue);

        db_condition p_cond;
        db_condition n_cond;

        p_cond.field() = weObjTypeProfile "." weoProfileID;
        p_cond.operation() = db_condition::equal;
        p_cond.value() = parentID;

        n_cond.field() = weObjTypeProfile "." weoName;
        n_cond.operation() = db_condition::equal;
        n_cond.value() = name;

        flt.where.set(p_cond).and(n_cond);

        db_recordset data(flt.what);
        db_cursor rec = data.push_back();

        rec[0] = parentID;
        rec[1] = name;
        rec[2] = val.which();
        rec[3] = val;

        plg_storage->set(flt, data);
    }
}

bool engine_dispatcher::IsSet( const string& name )
{
    bool retval;
    db_recordset res;
    db_query flt;
    string parentID = "0";

    retval = false;
    if (plg_storage != NULL) {
        flt.what.clear();
        flt.what.push_back(weObjTypeProfile "." weoTypeID);
        flt.what.push_back(weObjTypeProfile "." weoValue);

        db_condition p_cond;
        db_condition n_cond;

        p_cond.field() = weObjTypeProfile "." weoProfileID;
        p_cond.operation() = db_condition::equal;
        p_cond.value() = parentID;

        n_cond.field() = weObjTypeProfile "." weoName;
        n_cond.operation() = db_condition::equal;
        n_cond.value() = name;

        flt.where.set(p_cond).and(n_cond);
        plg_storage->get(flt, res);

        if (res.size() > 0) {
            db_cursor rec = res.begin();
            try{
                if (!rec[1].empty()) {
                    int tp = boost::lexical_cast<int>(rec[0]);
                    if ( tp == 2) {
                        // we_variant::which(bool)
                        retval = boost::lexical_cast<bool>(rec[1]);
                    }
                    else {
                        retval = true;
                    }
                } // if result not <empty>
            }
            catch(bad_cast &e) {
                retval = false;
                LOG4CXX_ERROR(iLogger::GetLogger(), "engine_dispatcher::IsSet(" << name << ") can't get option value: " << e.what());
            }
        } // if result size > 0
    } // if plg_storage != NULL
    LOG4CXX_TRACE(iLogger::GetLogger(), "engine_dispatcher::IsSet(" << name << ") value=" << retval);
    return retval;
}

void engine_dispatcher::Erase( const string& name )
{
    db_filter filter;
    string parentID = "0";

    if (plg_storage != NULL) {
        db_condition p_cond;
        db_condition n_cond;

        p_cond.field() = weObjTypeProfile "." weoProfileID;
        p_cond.operation() = db_condition::equal;
        p_cond.value() = parentID;

        n_cond.field() = weObjTypeProfile "." weoName;
        n_cond.operation() = db_condition::equal;
        n_cond.value() = name;

        filter.set(p_cond).and(n_cond);
        LOG4CXX_TRACE(iLogger::GetLogger(), "engine_dispatcher::Erase - " << filter.tostring());
        plg_storage->del(filter);
    }
}

void engine_dispatcher::Clear()
{
    string_list opt_names;

    opt_names = OptionsList();

    for (size_t i = 0; i < opt_names.size(); i++) {
        Erase(opt_names[i]);
    }
}

string_list engine_dispatcher::OptionsList()
{
    string_list retval;
    db_recordset res;
    db_query flt;
    string name;
    string parentID = "0";

    if (plg_storage != NULL) {
        flt.what.clear();
        flt.what.push_back(weObjTypeProfile "." weoName);
        db_condition p_cond;

        p_cond.field() = weObjTypeProfile "." weoProfileID;
        p_cond.operation() = db_condition::equal;
        p_cond.value() = parentID;

        flt.where.set(p_cond);
        plg_storage->get(flt, res);

        db_cursor rec = res.begin();
        while(rec != res.end()) {
            name = rec[0].get<string>();
            if (name != "") {
                retval.push_back(name);
            } // if name present
            ++rec;
        } // foreach record
    } // if plg_storage != NULL

    return retval;
}

size_t engine_dispatcher::OptionSize()
{
    int retval = 0;
    db_recordset res;
    db_query flt;
    string parentID = "0";

    if (plg_storage != NULL) {
        flt.what.clear();
        flt.what.push_back(weObjTypeProfile "." weoName);
        db_condition p_cond;

        p_cond.field() = weObjTypeProfile "." weoProfileID;
        p_cond.operation() = db_condition::equal;
        p_cond.value() = parentID;

        flt.where.set(p_cond);
        plg_storage->get(flt, res);
        retval = res.size();
    }
    return retval;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn void engine_dispatcher::get_interface( string iface )
///
/// @brief  Search in loaded plugins for specified interface.
///
/// @param  iface   - The interface identifier.
///
/// @retval null if no such interface loaded, else the requested interface casted to i_plugin.
///
/// @author A. Abramov
/// @date   19.05.2010
////////////////////////////////////////////////////////////////////////////////////////////////////
i_plugin* engine_dispatcher::get_interface( string iface )
{
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn void engine_dispatcher::flush()
///
/// @brief  Stores the options into the storage.
///
/// @author A. Abramov
/// @date   23.07.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
void engine_dispatcher::flush()
{
    if (plg_storage != NULL)
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "engine_dispatcher::flush ");
        db_recordset* rs = NULL; /*i_options_provider::ToRS();*/
        if (rs != NULL) {
            plg_storage->set(*rs);
            delete rs;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static void* create_null_storage(void* krnl, void* handle = NULL)
{
    return (void*) (new null_storage((engine_dispatcher*)krnl, handle));
}

static void* create_mem_storage(void* krnl, void* handle = NULL)
{
    return (void*) (new mem_storage((engine_dispatcher*)krnl, handle));
}

static void* create_http_transport(void* krnl, void* handle = NULL)
{
    return (void*) (new http_transport((engine_dispatcher*)krnl, handle));
}

static void* create_http_inventory(void* krnl, void* handle = NULL)
{
    return (void*) (new http_inventory((engine_dispatcher*)krnl, handle));
}

static void* create_html_parser(void* krnl, void* handle = NULL)
{
    return (void*) (new html_parser((engine_dispatcher*)krnl, handle));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn plugin_factory::plugin_factory()
///
/// @brief  Default constructor.
///
/// @author A. Abramov
/// @date   24.08.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
plugin_factory::plugin_factory()
{
    // add "default" plugins
    add_plugin_class("7CB7A5F18348", create_null_storage);
    add_plugin_class("null_storage", create_null_storage);      // copy for interface name
    add_plugin_class("D82B31419339", create_mem_storage);
    add_plugin_class("mem_storage", create_mem_storage);        // copy for interface name
    add_plugin_class("A44A9A1E7C25", create_http_transport);
    add_plugin_class("http_transport", create_http_transport);  // copy for interface name
    add_plugin_class("AB7ED6E5A7B3", create_http_inventory);
    add_plugin_class("http_inventory", create_http_inventory);  // copy for interface name
    add_plugin_class("7467A5250777", create_html_parser);
    add_plugin_class("html_parser", create_html_parser);  // copy for interface name
}

plugin_factory::~plugin_factory()
{
}

void plugin_factory::add_plugin_class( string name, fnWePluginFactory func )
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "new plugin_factory added for " << name);
	factories_[name] = func;
}

void* plugin_factory::create_plugin( string pluginID, engine_dispatcher* krnl )
{
	map<string, fnWePluginFactory>::const_iterator it = factories_.find(pluginID);
	if (it == factories_.end())
	{
        LOG4CXX_TRACE(iLogger::GetLogger(), "plugin_factory::create_plugin plugin doesn't register in memory");
        return NULL;
    }

    LOG4CXX_DEBUG(iLogger::GetLogger(), "plugin_factory::create_plugin " << pluginID);
    return it->second(krnl, NULL);
}
