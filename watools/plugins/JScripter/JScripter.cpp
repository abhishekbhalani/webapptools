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

#include "weLogger.h"

#define VERSION_PRODUCTSTR "$Revision: 0 $"

namespace jscripter
{
jscripter::jscripter(webEngine::engine_dispatcher* krnl, void* handle)
    : webEngine::i_plugin(krnl, handle)
{
    pluginInfo.interface_name = "jscripter_plugin";
    pluginInfo.interface_list.push_back("jscripter_plugin");
    pluginInfo.interface_list.push_back("i_inventory");
    pluginInfo.interface_list.push_back("i_audit");
    pluginInfo.interface_list.push_back("i_vulner");
    pluginInfo.plugin_desc = "JScripter plugin (" + pluginInfo.interface_name + ") " + VERSION_PRODUCTSTR;
    pluginInfo.plugin_id = "BBD308E5CCB4";
    pluginInfo.plugin_icon = webEngine::WeXpmToStringList(jscripter_xpm, sizeof(jscripter_xpm) / sizeof(char*) );
}

jscripter::~jscripter(void)
{

}

webEngine::i_plugin* jscripter::get_interface( const string& ifName )
{
    LOG4CXX_INFO(logger, _T("### ") << std::string(__FUNCTION__) << _T(" ") << ifName << _T(" requested"));
    if (boost::iequals(ifName, "jscripter") || boost::iequals(ifName, "i_plugin"))
    {
        usageCount++;
        return this;
    }

    if (boost::iequals(ifName, "i_inventory"))
    {
        if (!m_inventory)
            m_inventory.reset(new detail::jscripter_inventory(*this, kernel, libHandle));
        if (m_inventory)
        {
            return m_inventory->get_interface(ifName);
        }
    }

    if (boost::iequals(ifName, "i_audit"))
    {
        if (!m_audit)
            m_audit.reset(new detail::jscripter_audit(*this, kernel, libHandle));
        if (m_audit)
        {
            return m_audit->get_interface(ifName);
        }
    }

    if (boost::iequals(ifName, "i_vulner"))
    {
        if (!m_vulner)
            m_vulner.reset(new detail::jscripter_vulner(*this, kernel, libHandle));
        if (m_vulner)
        {
            return m_vulner->get_interface(ifName);
        }
    }
    return NULL;
}

void jscripter::init(webEngine::task* tsk)
{
    LOG4CXX_INFO(logger, _T("### ") << std::string(__FUNCTION__) << _T(" of ") << pluginInfo.interface_name);
}

void jscripter::pause(webEngine::task* tsk, bool paused)
{
    LOG4CXX_INFO(logger, _T("### ") << std::string(__FUNCTION__) << _T(" of ") << pluginInfo.interface_name);
}

void jscripter::stop(webEngine::task* tsk)
{
    LOG4CXX_INFO(logger, _T("### ") << std::string(__FUNCTION__) << _T(" of ") << pluginInfo.interface_name);
}

}
