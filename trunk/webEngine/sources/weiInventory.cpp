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

namespace webEngine {

i_inventory::i_inventory(engine_dispatcher* krnl, void* handle /*= NULL*/) :
    i_plugin(krnl, handle)
{
    pluginInfo.interface_name = "i_inventory";
    pluginInfo.interface_list.push_back("i_inventory");
    pluginInfo.plugin_desc = "Abstract inventory interface";
    pluginInfo.plugin_id = "5C20963DFD87"; //{51038E5D-5948-47ab-9A2A-5C20963DFD87}
    pluginInfo.plugin_icon = WeXpmToStringList(iweInventory_xpm, sizeof(iweInventory_xpm) / sizeof(char*) );
}

i_inventory::~i_inventory(void)
{
}

i_plugin* i_inventory::get_interface( const string& ifName )
{
    LOG4CXX_TRACE(logger, "i_inventory::get_interface " << ifName);
    if (iequals(ifName, "i_inventory"))
    {
        LOG4CXX_DEBUG(logger, "i_inventory::get_interface found!");
        usageCount++;
        return (this);
    }
    return i_plugin::get_interface(ifName);
}

void i_inventory::response_dispatcher( iResponse *resp, void* context )
{
    i_inventory* object = (i_inventory*)context;
//     if (object->process_response) {
        object->process_response(resp);
//     }
//     else
//     {
//         LOG4CXX_ERROR(object->logger, "i_inventory::response_dispatcher: invalid context, or abstract class - process_response == NULL");
//     }
}

} // namespace webEngine
