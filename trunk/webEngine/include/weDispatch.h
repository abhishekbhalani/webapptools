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
#ifndef __WEDISPATCH_H__
#define __WEDISPATCH_H__
#pragma once
#include "weoptions.h"
#include "weiPlugin.h"
#include "weMemStorage.h"

class WeDispatch :
    public iweOptionsProvider
{
public:
    WeDispatch(void);
    virtual ~WeDispatch(void);

    // iweOptionsProvider
    virtual WeOption& Option(const string& name);
    virtual bool IsSet(const string& name);
    virtual void Option(const string& name, WeOptionVal val);

    // WeDispatch
    // Access the Storage
    const iweStorage* Storage(void) const  { return(storage);  };
    void Storage(const iweStorage* store)  { storage = (iweStorage*)store;  };

    // Access the PluginList
    const WePluginList &PluginList(void) const  { return(pluginList); };
    void RefreshPluginList(void);

    iwePlugin* LoadPlugin(string id);
    
#ifndef __DOXYGEN__
protected:
    WePluginList pluginList;
    iweStorage* storage;

private:
    WeDispatch(WeDispatch&) {};               ///< Avoid object copying
    WeDispatch& operator=(WeDispatch&) { return *this; };    ///< Avoid object copying
#endif // __DOXYGEN__
};

#endif //__WEDISPATCH_H__
