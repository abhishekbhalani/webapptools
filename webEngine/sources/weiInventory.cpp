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
#include <weHelper.h>
#include <weDispatch.h>
#include <weiInventory.h>
#include "iweInventory.xpm"

iweInventory::iweInventory(WeDispatch* krnl, void* handle /*= NULL*/) :
    iwePlugin(krnl, handle)
{
    pluginInfo.IfaceName = "iweInventory";
    pluginInfo.IfaceList.push_back("iweInventory");
    pluginInfo.PluginDesc = "Abstract inventory interface";
    pluginInfo.PluginId = "5C20963DFD87"; //{51038E5D-5948-47ab-9A2A-5C20963DFD87}
    pluginInfo.PluginIcon = WeXpmToStringList(iweInventory_xpm, sizeof(iweInventory_xpm) / sizeof(char*) );
    if (krnl != NULL) {
        logger = krnl->GetLogger();
    }
    else {
        logger = WeLogger::GetLogger();
    }
}

iweInventory::~iweInventory(void)
{
}

void* iweInventory::GetInterface( const string& ifName )
{
    LOG4CXX_TRACE(logger, "iweInventory::GetInterface " << ifName);
    if (iequals(ifName, "iweInventory"))
    {
        LOG4CXX_DEBUG(logger, "iweInventory::GetInterface found!");
        usageCount++;
        return (void*)(this);
    }
    return iwePlugin::GetInterface(ifName);
}