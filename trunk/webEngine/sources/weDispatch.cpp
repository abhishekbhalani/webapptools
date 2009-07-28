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
#include "weiBase.h"
#include "weHTTP.h"
#include "weDispatch.h"
#include "externals/shared_object.hpp"

using namespace boost::filesystem;

#ifndef __DOXYGEN__
static const WeOption empty_option("_empty_");
#endif //__DOXYGEN_ _

static WeStringList FindFiles ( path baseDir)
{
    WeStringList files;
    WeStringList subs;

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

WeNullStorage::WeNullStorage( WeDispatch* krnl, void* handle /*= NULL*/ ) :
    iweStorage(krnl, handle)
{
    pluginInfo.IfaceName = "WeNullStorage";
    pluginInfo.IfaceList.push_back("WeNullStorage");
    pluginInfo.PluginDesc = "Placeholder storage";
    pluginInfo.PluginId = "7CB7A5F18348";
}

WeNullStorage::~WeNullStorage()
{
}

void* WeNullStorage::GetInterface( const string& ifName )
{
    if (iequals(ifName, "iweStorage"))
    {
        usageCount++;
        return (void*)((iweStorage*)this);
    }
    if (iequals(ifName, "WeNullStorage"))
    {
        usageCount++;
        return (void*)((WeNullStorage*)this);
    }
    return iweStorage::GetInterface(ifName);
}

//////////////////////////////////////////////////////////////////////////
// WeDispatch class
//////////////////////////////////////////////////////////////////////////
WeDispatch::WeDispatch(void)
{
    storage = new WeNullStorage(this);
    pluginList.clear();
}

WeDispatch::~WeDispatch(void)
{
    if (storage != NULL)
    {
        Flush();
        delete storage;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	WeOption& WeDispatch::Option(const string& name )
///
/// @brief	Returns the Option with given name or empty option.
///
/// @param  name - The option name. 
///
/// @retval WeOption. 
////////////////////////////////////////////////////////////////////////////////////////////////////
WeOption& WeDispatch::Option(const  string& name )
{
    WeOptions::iterator it;

    FUNCTION;
    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeDispatch::Option(" << name << ")");
    it = options.find(name);
    if (it != options.end())
    {
        return *(it->second);
    }
    return *((WeOption*)&empty_option);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn void WeDispatch::Option(const string& name,
/// 	WeOptionVal val)
///
/// @brief  Sets the value of the options with given name.
///
/// @param  name - The option name. 
/// @param  val	 - The option value. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void WeDispatch::Option(const string& name, WeOptionVal val)
{
    WeOptions::iterator it;
    WeOption* opt;

    FUNCTION;
    it = options.find(name);
    if (it != options.end())
    {
        opt = it->second;
        opt->SetValue(val);
    }
    else {
        opt = new WeOption();
        opt->Name(name);
        opt->SetValue(val);
        options[name] = opt;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	bool WeDispatch::IsSet(const string& name )
///
/// @brief	Query if bool options 'name' is set to true.
///
/// @param	name - The option name.
///
/// @retval	true if set, false if not.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool WeDispatch::IsSet(const string& name )
{
    WeOptions::iterator it;
    WeOption* opt;
    bool retval = false;

    FUNCTION;
    it = options.find(name);
    if (it != options.end())
    {
        opt = it->second;
        try
        {
            opt->GetValue(retval);
        }
        catch (...)
        {
            retval = false;
        }
    }
    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn void WeDispatch::RefreshPluginList( void )
///
/// @brief  Refresh plugin list. Rebuilds list of the available plugins even embedded or external
////////////////////////////////////////////////////////////////////////////////////////////////////
void WeDispatch::RefreshPluginList( boost::filesystem::path& baseDir )
{
    WePluginInfo *info;
    iwePlugin *plg;

    pluginList.clear();

    WeMemStorage memStore(this);
    pluginList.push_back(*(WePluginInfo*)memStore.Info());
    WeHTTP httpTrans(this);
    pluginList.push_back(*(WePluginInfo*)httpTrans.Info());

    // search for dynamic libraries with plugin interface
    WeStringList files = FindFiles(baseDir.string());
    WeStringList::iterator fl;
    for (fl = files.begin(); fl != files.end(); fl++)
    {
        // load library, get info, free library
        try
        {
            dyn::shared_object so((*fl).c_str());
            fnWePluginFactory ptr = NULL;
            so.get_symbol("WePluginFactory", ptr);
            plg = (iwePlugin*)ptr(this, NULL); // not need to store SO link
            if (plg != NULL)
            {
                info = (WePluginInfo*)plg->Info();
                info->PluginPath = *fl;
                pluginList.push_back(*info);
                LOG4CXX_INFO(WeLogger::GetLogger(), "WeDispatch::RefreshPluginList: loaded plugin " << *fl);
                LOG4CXX_TRACE(WeLogger::GetLogger(), ">>>> ID=" << info->PluginId << "; Desc=" << info->PluginDesc);
                plg->Release();
                plg = NULL;
            }
            else {
                LOG4CXX_WARN(WeLogger::GetLogger(), "WeDispatch::RefreshPluginList: can't get information from plugin " <<*fl);
            }
        }
        catch (std::exception& e)
        {
            LOG4CXX_ERROR(WeLogger::GetLogger(), "WeDispatch::RefreshPluginList: can't load plugin " << *fl << ". Error: " << e.what());
        }
    }
    
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	iwePlugin* WeDispatch::LoadPlugin( string id )
///
/// @brief	Loads a plugin form shared library or internal implementation. 
///
/// @param	id	 - The identifier. 
///
/// @retval	null if it fails, else the plugin interface. 
////////////////////////////////////////////////////////////////////////////////////////////////////
iwePlugin* WeDispatch::LoadPlugin( string id )
{
    iwePlugin* retval = NULL;

    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeDispatch::LoadPlugin " << id);
    if (id == "D82B31419339") {
        // WeMemStorage interface
        LOG4CXX_TRACE(WeLogger::GetLogger(), "WeDispatch::LoadPlugin - embedded plugin: WeMemStorage");
        retval = new WeMemStorage(this);
    }
    if (id == "A44A9A1E7C25") {
        // WeHTTP interface
        LOG4CXX_TRACE(WeLogger::GetLogger(), "WeDispatch::LoadPlugin - embedded plugin: WeHTTP");
        retval = new WeHTTP(this);
    }

    if (retval == NULL) {
        // external plugins
        // go through loop and search plugin
        // if found - loads shared library and create instance
        for (int i = 0; i < pluginList.size(); i++)
        {
            if (pluginList[i].PluginId == id) {
                LOG4CXX_TRACE(WeLogger::GetLogger(), "WeDispatch::LoadPlugin - found plugin: " << pluginList[i].PluginDesc);
                try
                {
                    dyn::shared_object* so = new dyn::shared_object(pluginList[i].PluginPath.c_str());
                    fnWePluginFactory ptr = NULL;
                    so->get_symbol("WePluginFactory", ptr);
                    retval = (iwePlugin*)ptr(this, so);
                }
                catch (std::exception& e)
                {
                	LOG4CXX_ERROR(WeLogger::GetLogger(), "WeDispatch::LoadPlugin: can't load plugin " << pluginList[i].PluginPath << ". Error: " << e.what());
                }
                break;
            }
        }
    }
    if (retval == NULL)
    {
        LOG4CXX_WARN(WeLogger::GetLogger(), "WeDispatch::LoadPlugin: '" << id << "' not found!");
    }
    return retval;
}

void WeDispatch::Storage( const iweStorage* store )
{
    if (storage != NULL)
    {
        storage->Release();
    }

    storage = (iweStorage*)store;
    if (storage == NULL)
    {
        storage = new WeNullStorage(this);
    }
    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeDispatch::SetStorage - plugin: " << storage->GetDesc());
    string xml;
    storage->SystemOptionsReport("", xml);
    iweOptionsProvider::FromXml(xml);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn void WeDispatch::Flush()
///
/// @brief  Stores the options into the storage. 
///
/// @author A. Abramov
/// @date   23.07.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
void WeDispatch::Flush()
{
    if (storage != NULL)
    {
        LOG4CXX_TRACE(WeLogger::GetLogger(), "WeDispatch::Flush ");
        string xml = iweOptionsProvider::ToXml();
        storage->SystemOptionsSave(xml);
    }
}