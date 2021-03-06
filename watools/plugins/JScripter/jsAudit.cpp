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
    Version:        $Revision$
*/
#include "JScripter.h"
#include "jscripter.xpm"
#include "weiPlugin.h"
#include "weTask.h"
#include <boost/foreach.hpp>

#define VERSION_PRODUCTSTR "$Revision$"

namespace jscripter
{
namespace detail
{
jscripter_audit::jscripter_audit(jscripter& js, webEngine::engine_dispatcher* krnl, void* handle)
    : webEngine::i_audit(krnl, handle)
    , m_scripter(js)
{
    pluginInfo.interface_name = "jscripter_audit";
    pluginInfo.interface_list.push_back("i_audit");
	pluginInfo.plugin_desc = "JScripter plugin (" + pluginInfo.interface_name + ") " + VERSION_PRODUCTSTR;
    pluginInfo.plugin_id = "0FB00AD22106";
    pluginInfo.plugin_icon = webEngine::WeXpmToStringList(jscripter_xpm, sizeof(jscripter_xpm) / sizeof(char*) );
}

jscripter_audit::~jscripter_audit(void)
{

}

void jscripter_audit::process(webEngine::task* tsk, webEngine::scan_data_ptr scData)
{
    LOG4CXX_INFO(logger, _T("### ") << std::string(__FUNCTION__) << _T(" of ") << pluginInfo.interface_name);
    // execute all scripts in queue for this stage
    if (tsk->IsSet(std::string(weoJsAudit))) {
        BOOST_FOREACH(jscripter_block &code, m_queue)
        {
            m_scripter.execute(tsk, code, scData);
        }
    }
}

void jscripter_audit::process_response(webEngine::i_response_ptr resp)
{
    LOG4CXX_INFO(logger, _T("### ") << std::string(__FUNCTION__) << _T(" of ") << pluginInfo.interface_name);
}

void jscripter_audit::init(webEngine::task* tsk)
{
    LOG4CXX_INFO(logger, _T("### ") << std::string(__FUNCTION__) << _T(" of ") << pluginInfo.interface_name);
    // preload scripts for this stage
    if (tsk->IsSet(std::string(weoJsAudit))) {
        m_scripter.preload(tsk, audit, m_queue);
    }
}

void jscripter_audit::pause(webEngine::task* tsk, bool paused)
{
    LOG4CXX_INFO(logger, _T("### ") << std::string(__FUNCTION__) << _T(" of ") << pluginInfo.interface_name);
}

void jscripter_audit::stop(webEngine::task* tsk)
{
    LOG4CXX_INFO(logger, _T("### ") << std::string(__FUNCTION__) << _T(" of ") << pluginInfo.interface_name);
}

}
}
