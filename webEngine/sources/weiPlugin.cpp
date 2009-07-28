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
#include "weiPlugin.h"
#include "externals/shared_object.hpp"

iwePlugin::iwePlugin( WeDispatch* krnl, void* handle /*= NULL*/ )
{
    kernel = krnl;
    usageCount = 0;
    libHandle = handle;
    pluginInfo.IfaceName = "iwePlugin";
    pluginInfo.IfaceList.clear();
    pluginInfo.IfaceList.push_back("iwePlugin");
    pluginInfo.PluginDesc = "Base plugin interface";
    pluginInfo.PluginId = "C665E995E5B4";
    pluginInfo.PluginIcon.clear();
    pluginInfo.PluginPath = "";
}

const string iwePlugin::InterfaceName()
{
    return pluginInfo.IfaceName;
}

WeStringList iwePlugin::InterfaceList()
{
    return pluginInfo.IfaceList;
}

void* iwePlugin::GetInterface( const string& ifName )
{
    if (ifName == "iwePlugin") {
        usageCount++;
        return this;
    }
    return NULL;
}

const string iwePlugin::GetDesc()
{
    return pluginInfo.PluginDesc;
}

const string iwePlugin::GetID()
{
    return pluginInfo.PluginId;
}

WeStringList iwePlugin::GetIcon()
{
    return pluginInfo.PluginIcon;
}

void iwePlugin::Release()
{
    if (usageCount == 0)
    {
        if (libHandle != NULL)
        {
            LOG4CXX_TRACE(WeLogger::GetLogger(), "iwePlugin::Release: (" << pluginInfo.PluginId << ") free the shared library");
            delete ((dyn::shared_object*)libHandle);
        }
        delete this;
    }
    else {
        usageCount--;
    }
}

WeStringList WeXpmToStringList( char** xpm, int lines )
{
    WeStringList retval;
    retval.clear();

    for (int i = 0; i < lines; i++)
    {
        retval.push_back(xpm[i]);
    }
    return retval;
}