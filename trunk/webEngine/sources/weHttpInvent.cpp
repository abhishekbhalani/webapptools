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
#include <weHttpInvent.h>
#include "iweInventory.xrc"

WeHttpInvent::WeHttpInvent(WeDispatch* krnl, void* handle /*= NULL*/) :
    iweInventory(krnl, handle)
{
    pluginInfo.IfaceName = "httpInventory";
    pluginInfo.IfaceList.push_back("httpInventory");
    pluginInfo.PluginDesc = "Inventory through HTTP protocol";
    pluginInfo.PluginId = "AB7ED6E5A7B3"; //{7318EAB5-4253-4a31-8284-AB7ED6E5A7B3}
}

WeHttpInvent::~WeHttpInvent(void)
{
}

void* WeHttpInvent::GetInterface( const string& ifName )
{
    LOG4CXX_TRACE(logger, "WeHttpInvent::GetInterface " << ifName);
    if (iequals(ifName, "httpInventory"))
    {
        LOG4CXX_DEBUG(logger, "WeHttpInvent::GetInterface found!");
        usageCount++;
        return (void*)(this);
    }
    return iweInventory::GetInterface(ifName);
}

const string WeHttpInvent::GetSetupUI( void )
{
    return xrc;
}
