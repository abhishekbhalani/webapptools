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
#include "externals/shared_object.hpp"
#include "weiBase.h"
#include "weHTTP.h"
#include "weHttpInvent.h"
#include "weDispatch.h"

using namespace boost::filesystem;
using namespace webEngine;

#ifndef __DOXYGEN__
static const wOption empty_option("_empty_");
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

int null_storage::get(db_record& filter, db_record& respFilter, db_recordset& results)
{
    return 0;
}

int null_storage::set(db_record& filter, db_record& data)
{
    return 0;
}

int null_storage::set(db_recordset& data)
{
    return 0;
}

int null_storage::del(db_record& filter)
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
/// @fn void engine_dispatcher::refresh_plugin_list( void )
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
    HttpTransport httpTrans(this);
    plg_list.push_back(*(plugin_info*)httpTrans.info());
    HttpInventory httpInvent(this);
    plg_list.push_back(*(plugin_info*)httpInvent.info());

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
    db_recordset res;
    db_record filter;
    filter.Clear();
    filter.objectID = weObjTypeSysOption;
    filter.Option(weoParentID, string("0"));
    plg_storage->get(filter, filter, res);
    iOptionsProvider::FromRS(&res);
}


void webEngine::engine_dispatcher::add_plugin_class( string name, fnWePluginFactory func )
{
    int i;
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
/// @date   28.04.2010
////////////////////////////////////////////////////////////////////////////////////////////////////
i_plugin* webEngine::engine_dispatcher::get_interface( string iface )
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
        db_recordset* rs = iOptionsProvider::ToRS();
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
    return (void*) (new HttpTransport((engine_dispatcher*)krnl, handle));
}

static void* create_http_inventory(void* krnl, void* handle = NULL)
{
    return (void*) (new HttpInventory((engine_dispatcher*)krnl, handle));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn plugin_factory::plugin_factory()
///
/// @brief  Default constructor.
///
/// @author A. Abramov
/// @date   24.08.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
plugin_factory::plugin_factory() :
    linked_list<string, fnWePluginFactory>()
{
    data = new linked_list_elem<string, fnWePluginFactory>;
    data->key("");
    data->value(NULL);
    data->link(NULL);
    // add "default" plugins
    add_plugin_class("7CB7A5F18348", create_null_storage);
    add_plugin_class("null_storage", create_null_storage);      // copy for interface name
    add_plugin_class("D82B31419339", create_mem_storage);
    add_plugin_class("mem_storage", create_mem_storage);        // copy for interface name
    add_plugin_class("A44A9A1E7C25", create_http_transport);
    add_plugin_class("http_transport", create_http_transport);  // copy for interface name
    add_plugin_class("AB7ED6E5A7B3", create_http_inventory);
    add_plugin_class("http_inventory", create_http_inventory);  // copy for interface name
}

plugin_factory::~plugin_factory()
{
    // nothing special - just clear the list;
}

void plugin_factory::add_plugin_class( string name, fnWePluginFactory func )
{
    linked_list_elem<string, fnWePluginFactory>* obj;

    LOG4CXX_TRACE(iLogger::GetLogger(), "new plugin_factory added for " << name);
    obj = new linked_list_elem<string, fnWePluginFactory>();
    obj->key(name);
    obj->value(func);
    curr = data;
    while (curr != NULL) {
        if (curr->key() == name)
        {
            break;
        }
        curr = curr->next();
    }
    if (curr != NULL)
    {
        curr->value(func);
    }
    else {
        data->add(obj);
    }
}

void* plugin_factory::create_plugin( string pluginID, engine_dispatcher* krnl )
{
    fnWePluginFactory func;

    func = find_first(pluginID);
    if (func == NULL) {
        LOG4CXX_TRACE(iLogger::GetLogger(), "plugin_factory::create_plugin plugin doesn't register in memory");
        return NULL;
    }
    LOG4CXX_DEBUG(iLogger::GetLogger(), "plugin_factory::create_plugin " << pluginID);
    return func(krnl, NULL);
}
