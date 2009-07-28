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

iweTransport* WeCreateNamedTransport( string name, WeDispatch* krnl )
{
    return weTransportFactory.CreateTransport(name, krnl);
}

//////////////////////////////////////////////////////////////////////////
//             i w e T r a n s p o r t    c l a s s
//////////////////////////////////////////////////////////////////////////
iweTransport::iweTransport(WeDispatch* krnl, void* handle /*= NULL*/ ) :
    iwePlugin(krnl, handle)
{
    parent = NULL;
    relocCount = 0;
    siteDepth = 0;
    responces.clear();
    // iwePlugin structure
    pluginInfo.IfaceName = "iweTransport";
    pluginInfo.IfaceList.push_back("iweTransport");
    pluginInfo.PluginDesc = "Abstract transport interface";
    pluginInfo.PluginId = "14826EDC8653";
    pluginInfo.PluginIcon = WeXpmToStringList(iweTransport_xpm, sizeof(iweTransport_xpm) / sizeof(char*) );
}

WeOption& iweTransport::Option(const  string& name )
{
    WeOptions::iterator it;

    it = options.find(name);
    if (it != options.end()) {
        return *(it->second);
    }

    if (parent != NULL) {
        return parent->Option(name);
    }

    return *(new WeOption);
}

bool iweTransport::IsSet(const  string& name )
{
    bool retval = false;
    WeOptions::iterator it;
    WeOption* opt;

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

void iweTransport::Option(const  string& name, WeOptionVal val )
{
    WeOptions::iterator it;
    WeOption* opt;

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
}

void* iweTransport::GetInterface( const string& ifName )
{
    if (iequals(ifName, "iweTransport"))
    {
        usageCount++;
        return (void*)((iweTransport*)this);
    }
    return iwePlugin::GetInterface(ifName);
}

//////////////////////////////////////////////////////////////////////////
//              i w e O p e r a t i o n    c l a s s
//////////////////////////////////////////////////////////////////////////
iweOperation::~iweOperation()
{
    vector<iweOperationPtr*>::iterator it;

    for (it = children.begin(); it != children.end(); it++) {
        delete (*it);
    }
    children.clear();
}

void iweOperation::AddChild( iweOperationPtr* chld )
{
    vector<iweOperationPtr*>::iterator it;

    it = find(children.begin(), children.end(), chld);
    if (it == children.end()) {
        children.push_back(chld);
    }
    (*chld)->previous = this;
}

void iweOperation::RemoveChild( iweOperationPtr* chld )
{
    vector<iweOperationPtr*>::iterator it;

    it = find(children.begin(), children.end(), chld);
    if (it != children.end()) {
        delete (*it);
    }
}
