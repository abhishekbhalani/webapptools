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

static StringList FindFiles ( path baseDir)
{
    StringList files;
    StringList subs;

    files.clear();
    if ( exists( baseDir ) ) {
        directory_iterator end_itr; // default construction yields past-the-end
        for ( directory_iterator itr( baseDir );
            itr != end_itr;
            ++itr )
        {
            if ( is_directory(itr->status()) )
            {
                subs = FindFiles( itr->path() );
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

NullStorage::NullStorage( Dispatch* krnl, void* handle /*= NULL*/ ) :
    iStorage(krnl, handle)
{
    pluginInfo.IfaceName = "NullStorage";
    pluginInfo.IfaceList.push_back("NullStorage");
    pluginInfo.PluginDesc = "Placeholder storage";
    pluginInfo.PluginId = "7CB7A5F18348";
}

NullStorage::~NullStorage()
{
}

void* NullStorage::GetInterface( const string& ifName )
{
    if (iequals(ifName, "iStorage"))
    {
        usageCount++;
        return (void*)((iStorage*)this);
    }
    if (iequals(ifName, "NullStorage"))
    {
        usageCount++;
        return (void*)((NullStorage*)this);
    }
    return iStorage::GetInterface(ifName);
}

int NullStorage::Get(const string& objType, Record& filters, Record& respFilter, RecordSet& results)
{
    return 0;
}

int NullStorage::Set(const string& objType, Record& filters, Record& data)
{
    return 0;
}

int NullStorage::Set(const string& objType, RecordSet& data)
{
    return 0;
}

int NullStorage::Delete(const string& objType, Record& filters)
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////
// Dispatch class
//////////////////////////////////////////////////////////////////////////
Dispatch::Dispatch(void)
{
    storage = new NullStorage(this);
    pluginList.clear();
}

Dispatch::~Dispatch(void)
{
    if (storage != NULL)
    {
        Flush();
        delete storage;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn void Dispatch::RefreshPluginList( void )
///
/// @brief  Refresh plugin list. Rebuilds list of the available plugins even embedded or external
////////////////////////////////////////////////////////////////////////////////////////////////////
void Dispatch::RefreshPluginList( boost::filesystem::path& baseDir )
{
    PluginInfo *info;
    iPlugin *plg;

    pluginList.clear();

    MemStorage memStore(this);
    pluginList.push_back(*(PluginInfo*)memStore.Info());
    HttpTransport httpTrans(this);
    pluginList.push_back(*(PluginInfo*)httpTrans.Info());
    HttpInventory httpInvent(this);
    pluginList.push_back(*(PluginInfo*)httpInvent.Info());

    // search for dynamic libraries with plugin interface
    StringList files = FindFiles(baseDir.string());
    StringList::iterator fl;
    for (fl = files.begin(); fl != files.end(); fl++)
    {
        // load library, get info, free library
        try
        {
            dyn::shared_object so((*fl).c_str());
            fnWePluginFactory ptr = NULL;
            so.get_symbol("WePluginFactory", ptr);
            plg = (iPlugin*)ptr(this, NULL); // not need to store SO link
            if (plg != NULL)
            {
                info = (PluginInfo*)plg->Info();
                info->PluginPath = *fl;
                pluginList.push_back(*info);
                LOG4CXX_INFO(iLogger::GetLogger(), "Dispatch::RefreshPluginList: loaded plugin " << *fl);
                LOG4CXX_TRACE(iLogger::GetLogger(), ">>>> ID=" << info->PluginId << "; Desc=" << info->PluginDesc);
                plg->Release();
                plg = NULL;
            }
            else {
                LOG4CXX_WARN(iLogger::GetLogger(), "Dispatch::RefreshPluginList: can't get information from plugin " <<*fl);
            }
        }
        catch (std::exception& e)
        {
            LOG4CXX_ERROR(iLogger::GetLogger(), "Dispatch::RefreshPluginList: can't load plugin " << *fl << ". Error: " << e.what());
        }
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	iPlugin* Dispatch::LoadPlugin( string id )
///
/// @brief	Loads a plugin form shared library or internal implementation.
///
/// @param	id	 - The identifier.
///
/// @retval	null if it fails, else the plugin interface.
////////////////////////////////////////////////////////////////////////////////////////////////////
iPlugin* Dispatch::LoadPlugin( string id )
{
    iPlugin* retval = NULL;

    LOG4CXX_TRACE(iLogger::GetLogger(), "Dispatch::LoadPlugin " << id);
    // first step - in-memory plugins
    retval = (iPlugin*)pluginFactory.CreatePlugin(id, this);

    if (retval == NULL) {
        // external plugins
        // go through loop and search plugin
        // if found - loads shared library and create instance
        for (size_t i = 0; i < pluginList.size(); i++)
        {
            if (pluginList[i].PluginId == id) {
                LOG4CXX_TRACE(iLogger::GetLogger(), "Dispatch::LoadPlugin - found plugin: " << pluginList[i].PluginDesc);
                try
                {
                    dyn::shared_object* so = new dyn::shared_object(pluginList[i].PluginPath.c_str());
                    fnWePluginFactory ptr = NULL;
                    so->get_symbol("WePluginFactory", ptr);
                    retval = (iPlugin*)ptr(this, so);
                }
                catch (std::exception& e)
                {
                	LOG4CXX_ERROR(iLogger::GetLogger(), "Dispatch::LoadPlugin: can't load plugin " << pluginList[i].PluginPath << ". Error: " << e.what());
                }
                break;
            }
        }
    }
    if (retval == NULL)
    {
        LOG4CXX_WARN(iLogger::GetLogger(), "Dispatch::LoadPlugin: '" << id << "' not found!");
    }
    return retval;
}

void Dispatch::Storage( const iStorage* store )
{
    if (storage != NULL)
    {
        storage->Release();
    }

    storage = (iStorage*)store;
    if (storage == NULL)
    {
        storage = new NullStorage(this);
    }
    LOG4CXX_TRACE(iLogger::GetLogger(), "Dispatch::SetStorage - plugin: " << storage->GetDesc());
    RecordSet res;
    Record filter;
    filter.Clear();
    storage->Get(weObjTypeSysOption, filter, filter, res);
    iOptionsProvider::FromRS(&res);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn void Dispatch::Flush()
///
/// @brief  Stores the options into the storage.
///
/// @author A. Abramov
/// @date   23.07.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
void Dispatch::Flush()
{
    if (storage != NULL)
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "Dispatch::Flush ");
        RecordSet* rs = iOptionsProvider::ToRS();
        storage->Set(weObjTypeSysOption, *rs);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static void* CreateNullStorage(void* krnl, void* handle = NULL)
{
    return (void*) (new NullStorage((Dispatch*)krnl, handle));
}

static void* CreateMemStorage(void* krnl, void* handle = NULL)
{
    return (void*) (new MemStorage((Dispatch*)krnl, handle));
}

static void* CreateHttpTransport(void* krnl, void* handle = NULL)
{
    return (void*) (new HttpTransport((Dispatch*)krnl, handle));
}

static void* CreateHttpInventory(void* krnl, void* handle = NULL)
{
    return (void*) (new HttpInventory((Dispatch*)krnl, handle));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn PluginFactory::PluginFactory()
///
/// @brief  Default constructor. 
///
/// @author A. Abramov
/// @date   24.08.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
PluginFactory::PluginFactory() :
    LinkedList<string, fnWePluginFactory>()
{
    data = new LinkedListElem<string, fnWePluginFactory>;
    data->Key("");
    data->Value(NULL);
    data->Link(NULL);
    // add "default" plugins
    Add("7CB7A5F18348", CreateNullStorage);
    Add("D82B31419339", CreateMemStorage);
    Add("A44A9A1E7C25", CreateHttpTransport);
    Add("AB7ED6E5A7B3", CreateHttpInventory);
}

void PluginFactory::Add( string name, fnWePluginFactory func )
{
    LinkedListElem<string, fnWePluginFactory>* obj;

    LOG4CXX_TRACE(iLogger::GetLogger(), "new PluginFactory added for " << name);
    obj = new LinkedListElem<string, fnWePluginFactory>();
    obj->Key(name);
    obj->Value(func);
    curr = data;
    while (curr != NULL) {
        if (curr->Key() == name)
        {
            break;
        }
        curr = curr->Next();
    }
    if (curr != NULL)
    {
        curr->Value(func);
    }
    else {
        data->Add(obj);
    }
}

void* PluginFactory::CreatePlugin( string pluginID, Dispatch* krnl )
{
    fnWePluginFactory func;

    func = FindFirst(pluginID);
    if (func == NULL) {
        LOG4CXX_TRACE(iLogger::GetLogger(), "PluginFactory::CreatePlugin plugin doesn't register in memory");
        return NULL;
    }
    LOG4CXX_DEBUG(iLogger::GetLogger(), "PluginFactory::CreatePlugin " << pluginID);
    return func(krnl, NULL);
}
