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
using namespace boost;

int i_storage::last_id = 0;

i_storage::i_storage(engine_dispatcher* krnl, void* handle /*= NULL*/) :
    i_plugin(krnl, handle)
{
    pluginInfo.interface_name = "i_storage";
    pluginInfo.interface_list.push_back("i_storage");
    pluginInfo.plugin_desc = "Base plugin interface";
    pluginInfo.plugin_id = "C7F595160595";
    pluginInfo.plugin_icon = WeXpmToStringList(iweStorage_xpm, sizeof(iweStorage_xpm) / sizeof(char*) );
    last_id = rand();
}

i_storage::~i_storage(void)
{
}

void* i_storage::get_interface( const string& ifName )
{
    if (iequals(ifName, "i_storage"))
    {
        usageCount++;
        return (void*)((i_storage*)this);
    }
    return i_plugin::get_interface(ifName);
}

std::string i_storage::generate_id( const string& objType /*= ""*/ )
{
    return lexical_cast<string>(++last_id);
}
