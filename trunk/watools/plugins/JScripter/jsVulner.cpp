/*
    webEngine is the HTML processing library
    Copyright (C) 2009-11 Andrew Abramov aabramov@ptsecurity.ru

    This file is part of webEngine

    webEngine is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.

    Author:         Andrew "Stinger" Abramov
    Version:        $Revision: 0 $
*/
#include "JScripter.h"
#include "jscripter.xpm"
#include "weiPlugin.h"

#define VERSION_PRODUCTSTR "$Revision: 0 $"

namespace jscripter
{
namespace detail
{
jscripter_vulner::jscripter_vulner(jscripter& js, webEngine::engine_dispatcher* krnl, void* handle)
    : webEngine::i_vulner(krnl, handle)
    , m_scripter(js)
{
    pluginInfo.interface_name = "jscripter_vulner";
    pluginInfo.interface_list.push_back("i_vulner");
	pluginInfo.plugin_desc = "JScripter plugin (" + pluginInfo.interface_name + ") " + VERSION_PRODUCTSTR;
    pluginInfo.plugin_id = "FE370A5B65A3";
    pluginInfo.plugin_icon = webEngine::WeXpmToStringList(jscripter_xpm, sizeof(jscripter_xpm) / sizeof(char*) );
}

jscripter_vulner::~jscripter_vulner(void)
{

}

void jscripter_vulner::process(webEngine::task* tsk, webEngine::scan_data_ptr scData)
{
    LOG4CXX_INFO(logger, _T("### ") << std::string(__FUNCTION__) << _T(" of ") << pluginInfo.interface_name);
}

void jscripter_vulner::process_response(webEngine::i_response_ptr resp)
{
    LOG4CXX_INFO(logger, _T("### ") << std::string(__FUNCTION__) << _T(" of ") << pluginInfo.interface_name);
}

void jscripter_vulner::init(webEngine::task* tsk)
{
    LOG4CXX_INFO(logger, _T("### ") << std::string(__FUNCTION__) << _T(" of ") << pluginInfo.interface_name);
}

void jscripter_vulner::pause(webEngine::task* tsk, bool paused)
{
    LOG4CXX_INFO(logger, _T("### ") << std::string(__FUNCTION__) << _T(" of ") << pluginInfo.interface_name);
}

void jscripter_vulner::stop(webEngine::task* tsk)
{
    LOG4CXX_INFO(logger, _T("### ") << std::string(__FUNCTION__) << _T(" of ") << pluginInfo.interface_name);
}

}
}
