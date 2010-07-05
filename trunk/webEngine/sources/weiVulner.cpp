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

#include <weScan.h>
#include <weiVulner.h>

namespace webEngine {

i_vulner::i_vulner(engine_dispatcher* krnl, void* handle /*= NULL*/) :
    i_plugin(krnl, handle)
{
    pluginInfo.interface_name = "i_vulner";
    pluginInfo.interface_list.push_back("i_vulner");
    pluginInfo.plugin_desc = "Abstract vulnerability detector interface";
    pluginInfo.plugin_id = "4890FF417662"; //{AE859EC2-1A94-4787-BF3F-4890FF417662}
}

i_vulner::~i_vulner(void)
{
}

void i_vulner::response_dispatcher( i_response_ptr resp, void* context )
{
    i_vulner* object = (i_vulner*)context;
    LOG4CXX_TRACE(object->logger, "i_audit::response_dispatcher");
    object->process_response(resp);
}

} // namespace webEngine
