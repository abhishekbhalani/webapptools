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

#include <weiAudit.h>

namespace webEngine {

i_audit::i_audit(engine_dispatcher* krnl, void* handle /*= NULL*/) :
    i_plugin(krnl, handle)
{
    pluginInfo.interface_name = "i_audit";
    pluginInfo.interface_list.push_back("i_audit");
    pluginInfo.plugin_desc = "Abstract audit interface";
    pluginInfo.plugin_id = "182FE628B8EA"; //{F9ECB03C-5A5C-4571-B6A0-182FE628B8EA}
    //pluginInfo.plugin_icon = WeXpmToStringList(iweInventory_xpm, sizeof(iweInventory_xpm) / sizeof(char*) );
}

i_audit::~i_audit(void)
{
}

void i_audit::response_dispatcher( iResponse *resp, void* context )
{
    i_audit* object = (i_audit*)context;
    //     if (object->process_response) {
    object->process_response(resp);
    //     }
    //     else
    //     {
    //         LOG4CXX_ERROR(object->logger, "i_audit::response_dispatcher: invalid context, or abstract class - process_response == NULL");
    //     }
}

} // namespace webEngine
