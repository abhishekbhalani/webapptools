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
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "weiBase.h"
#include "weTagScanner.h"
#include "weiStorage.h"
#include "iweStorage.xpm"

using namespace webEngine;

int iStorage::lastId = 0;

iStorage::iStorage(Dispatch* krnl, void* handle /*= NULL*/) :
    iPlugin(krnl, handle)
{
    pluginInfo.IfaceName = "iStorage";
    pluginInfo.IfaceList.push_back("iStorage");
    pluginInfo.PluginDesc = "Base plugin interface";
    pluginInfo.PluginId = "C7F595160595";
    pluginInfo.PluginIcon = WeXpmToStringList(iweStorage_xpm, sizeof(iweStorage_xpm) / sizeof(char*) );
    lastId = rand();
}

iStorage::~iStorage(void)
{
}

void* iStorage::GetInterface( const string& ifName )
{
    if (iequals(ifName, "iStorage"))
    {
        usageCount++;
        return (void*)((iStorage*)this);
    }
    return iPlugin::GetInterface(ifName);
}

std::string iStorage::GenerateID( const string& objType /*= ""*/ )
{
    return lexical_cast<string>(++lastId);
}
