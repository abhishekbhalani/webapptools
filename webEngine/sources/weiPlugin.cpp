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
#include <weiPlugin.h>
#include <weDispatch.h>
#include "externals/shared_object.hpp"

namespace webEngine {

i_plugin::i_plugin( engine_dispatcher* krnl, void* handle /*= NULL*/ )
{
    kernel = krnl;
    usageCount = 0;
    libHandle = handle;
    pluginInfo.IfaceName = "i_plugin";
    pluginInfo.IfaceList.clear();
    pluginInfo.IfaceList.push_back("i_plugin");
    pluginInfo.PluginDesc = "Base plugin interface";
    pluginInfo.PluginId = "C665E995E5B4";
    pluginInfo.PluginIcon.clear();
    pluginInfo.PluginPath = "";
    priority = 50;
    if (krnl != NULL) {
        logger = krnl->get_logger();
    }
    else {
        logger = iLogger::GetLogger();
    }
}

const string i_plugin::interface_name()
{
    return pluginInfo.IfaceName;
}

string_list i_plugin::interface_list()
{
    return pluginInfo.IfaceList;
}

void* i_plugin::get_interface( const string& ifName )
{
    if (ifName == "i_plugin") {
        usageCount++;
        return this;
    }
    return NULL;
}

const string i_plugin::get_description()
{
    return pluginInfo.PluginDesc;
}

const string i_plugin::get_id()
{
    return pluginInfo.PluginId;
}

string_list i_plugin::get_icon()
{
    return pluginInfo.PluginIcon;
}

void i_plugin::release()
{
    if (usageCount == 0)
    {
        if (libHandle != NULL)
        {
            LOG4CXX_TRACE(iLogger::GetLogger(), "i_plugin::release: (" << pluginInfo.PluginId << ") free the shared library");
            delete ((dyn::shared_object*)libHandle);
        }
        delete this;
    }
    else {
        usageCount--;
    }
}

string_list WeXpmToStringList( char** xpm, int lines )
{
    string_list retval;
    retval.clear();

    for (int i = 0; i < lines; i++)
    {
        retval.push_back(xpm[i]);
    }
    return retval;
}

} // namespace webEngine
