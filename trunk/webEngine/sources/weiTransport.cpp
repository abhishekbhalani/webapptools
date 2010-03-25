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

#include "weiTransport.h"
#include "weHelper.h"
#include "iweTransport.xpm"

using namespace webEngine;

//////////////////////////////////////////////////////////////////////////
//             i w e T r a n s p o r t    c l a s s
//////////////////////////////////////////////////////////////////////////
iTransport::iTransport(engine_dispatcher* krnl, void* handle /*= NULL*/ ) :
    i_plugin(krnl, handle)
{
    parent = NULL;
    relocCount = 0;
    siteDepth = 0;
    responces.clear();
    // i_plugin structure
    pluginInfo.IfaceName = "iTransport";
    pluginInfo.IfaceList.push_back("iTransport");
    pluginInfo.PluginDesc = "Abstract transport interface";
    pluginInfo.PluginId = "14826EDC8653";
    pluginInfo.PluginIcon = WeXpmToStringList(iweTransport_xpm, sizeof(iweTransport_xpm) / sizeof(char*) );
}

wOption& iTransport::Option(const  string& name )
{
    wOptions::iterator it;

    it = options.find(name);
    if (it != options.end()) {
        return *(it->second);
    }

    if (parent != NULL) {
        return parent->Option(name);
    }

    return *(new wOption);
}

bool iTransport::IsSet(const  string& name )
{
    bool retval = false;
    wOptions::iterator it;
    wOption* opt;

    try
    {
        it = options.find(name);
        if (it != options.end())
        {
            opt = it->second;
            opt->GetValue(retval);
        }
        else if (parent != NULL) {
            retval = parent->IsSet(name);
        }
    }
    catch (...)
    {
        if (parent != NULL) {
            retval = parent->IsSet(name);
        }
    }

    return retval;
}

void iTransport::Option(const  string& name, wOptionVal val )
{
    wOptions::iterator it;
    wOption* opt;

    it = options.find(name);
    if (it != options.end())
    {
        opt = it->second;
        opt->SetValue(val);
    }
    else {
        opt = new wOption();
        opt->Name(name);
        opt->SetValue(val);
        options[name] = opt;
    }
}

void* iTransport::GetInterface( const string& ifName )
{
    if (iequals(ifName, "iTransport"))
    {
        usageCount++;
        return (void*)((iTransport*)this);
    }
    return i_plugin::get_interface(ifName);
}

//////////////////////////////////////////////////////////////////////////
//              i w e O p e r a t i o n    c l a s s
//////////////////////////////////////////////////////////////////////////
iOperation::~iOperation()
{
    vector<iweOperationPtr*>::iterator it;

    for (it = children.begin(); it != children.end(); it++) {
        delete (*it);
    }
    children.clear();
}

void iOperation::AddChild( iweOperationPtr* chld )
{
    vector<iweOperationPtr*>::iterator it;

    it = find(children.begin(), children.end(), chld);
    if (it == children.end()) {
        children.push_back(chld);
    }
    (*chld)->previous = this;
}

void iOperation::RemoveChild( iweOperationPtr* chld )
{
    vector<iweOperationPtr*>::iterator it;

    it = find(children.begin(), children.end(), chld);
    if (it != children.end()) {
        delete (*it);
    }
}
