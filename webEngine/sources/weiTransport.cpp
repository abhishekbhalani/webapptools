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

#include "weiTransport.h"
#include "weHelper.h"
#include "iweTransport.xpm"

using namespace webEngine;

static wOption empty_option;

//////////////////////////////////////////////////////////////////////////
//             i w e T r a n s p o r t    c l a s s
//////////////////////////////////////////////////////////////////////////
i_transport::i_transport(engine_dispatcher* krnl, void* handle /*= NULL*/ ) :
    i_plugin(krnl, handle)
{
    parent = NULL;
    relocCount = 0;
    siteDepth = 0;
    responces.clear();
    // i_plugin structure
    pluginInfo.interface_name = "i_transport";
    pluginInfo.interface_list.push_back("i_transport");
    pluginInfo.plugin_desc = "Abstract transport interface";
    pluginInfo.plugin_id = "14826EDC8653";
    pluginInfo.plugin_icon = WeXpmToStringList(iweTransport_xpm, sizeof(iweTransport_xpm) / sizeof(char*) );
}

i_plugin* i_transport::get_interface( const string& ifName )
{
    if (iequals(ifName, "i_transport"))
    {
        usageCount++;
        return this;
    }
    return i_plugin::get_interface(ifName);
}

//////////////////////////////////////////////////////////////////////////
//              i w e O p e r a t i o n    c l a s s
//////////////////////////////////////////////////////////////////////////
i_operation::~i_operation()
{
    children.clear();
}

void i_operation::AddChild( iweOperationPtr chld )
{
    vector<iweOperationPtr>::iterator it;

    it = find(children.begin(), children.end(), chld);
    if (it == children.end()) {
        children.push_back(chld);
    }
    chld->previous.reset(this);
}

void i_operation::RemoveChild( iweOperationPtr chld )
{
    vector<iweOperationPtr>::iterator it;

    it = find(children.begin(), children.end(), chld);
    if (it != children.end()) {
        children.erase(it);
    }
}
