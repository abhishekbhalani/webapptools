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

iPlugin::iPlugin( Dispatch* krnl, void* handle /*= NULL*/ )
{
    kernel = krnl;
    usageCount = 0;
    libHandle = handle;
    pluginInfo.IfaceName = "iPlugin";
    pluginInfo.IfaceList.clear();
    pluginInfo.IfaceList.push_back("iPlugin");
    pluginInfo.PluginDesc = "Base plugin interface";
    pluginInfo.PluginId = "C665E995E5B4";
    pluginInfo.PluginIcon.clear();
    pluginInfo.PluginPath = "";
    if (krnl != NULL) {
        logger = krnl->GetLogger();
    }
    else {
        logger = iLogger::GetLogger();
    }
}

const string iPlugin::InterfaceName()
{
    return pluginInfo.IfaceName;
}

StringList iPlugin::InterfaceList()
{
    return pluginInfo.IfaceList;
}

void* iPlugin::GetInterface( const string& ifName )
{
    if (ifName == "iPlugin") {
        usageCount++;
        return this;
    }
    return NULL;
}

const string iPlugin::GetDesc()
{
    return pluginInfo.PluginDesc;
}

const string iPlugin::GetID()
{
    return pluginInfo.PluginId;
}

StringList iPlugin::GetIcon()
{
    return pluginInfo.PluginIcon;
}

void iPlugin::Release()
{
    if (usageCount == 0)
    {
        if (libHandle != NULL)
        {
            LOG4CXX_TRACE(iLogger::GetLogger(), "iPlugin::Release: (" << pluginInfo.PluginId << ") free the shared library");
            delete ((dyn::shared_object*)libHandle);
        }
        delete this;
    }
    else {
        usageCount--;
    }
}

StringList WeXpmToStringList( char** xpm, int lines )
{
    StringList retval;
    retval.clear();

    for (int i = 0; i < lines; i++)
    {
        retval.push_back(xpm[i]);
    }
    return retval;
}

} // namespace webEngine
