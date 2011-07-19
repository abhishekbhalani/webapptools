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
#include "weLogger.h"
#include "jsLogger.h"
#include "weTask.h"
#include "jsTask.h"
#include "jsScanData.h"

#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#define VERSION_PRODUCTSTR "$Revision$"

static std::string xrc = _T("<plugin id='jscripter_plugin'>\
<option name='jscripter/ExecuteInventory' label='Execute scripts on Inventory stage' type='2' control='checkbox'>0</option>\
<option name='jscripter/ExecuteAudit' label='Execute scripts on Audit stage' type='2' control='checkbox'>0</option>\
<option name='jscripter/ExecuteVulners' label='Execute scripts for Vulnerability detection' type='2' control='checkbox'>0</option>\
<option name='jscripter/UseFiles' label='Use files instead DB' type='2' control='checkbox'>0</option>\
<option name='jscripter/FilesBasePath' label='Base path for scripts files' type='4' control='text'>./</option>\
</plugin>");


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
    // create JavaScript executor
    js_exec.reset(new webEngine::jsBrowser());
    {
        //set JS globals
        v8::Locker lock;
        v8::HandleScope hscope;
        v8::Persistent<v8::Context> ctx = js_exec->get_child_context();
        v8::Context::Scope scope(ctx);

        v8::Handle<v8::Object> _instance = v8_wrapper::wrap_object<jsLogger>(new jsLogger());
        ctx->Global()->Set(v8::String::New("log4cxx"), _instance);
    }
}

jscripter::~jscripter(void)
{
    // finalize JavaScript executor
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

const std::string jscripter::get_setup_ui( void )
{
    return xrc;
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

bool jscripter::preload( webEngine::task* tsk, detail::script_group grp, detail::jscripter_queue& queue )
{
    LOG4CXX_INFO(logger, _T("### ") << std::string(__FUNCTION__) << _T(" of ") << pluginInfo.interface_name);
    queue.clear();
    // load all scripts into the queue
    if (load_queue_list(tsk, grp, queue)) {
        LOG4CXX_DEBUG(logger, _T("jscripter::preload for stage ") << grp << _T(". ") << queue.size() << _T(" scripts found."));
        BOOST_FOREACH(detail::jscripter_block &code, queue)
        {
            load_code(tsk, code);
        }
        LOG4CXX_DEBUG(logger, _T("jscripter::preload for stage ") << grp << _T(". All scripts loaded."));
        // clear not loaded code
        std::remove_if(queue.begin(), queue.end(), detail::jscripter_block::remover() );
    }
    // sort queue
    std::sort(queue.begin(), queue.end(), detail::jscripter_block::order());

    // load scripts into the interpreter
    BOOST_FOREACH(detail::jscripter_block &code, queue)
    {
        js_exec->execute_string(code.code, code.name, true, true);
        //m_scripter.execute(tsk, code, scData);
    }
    LOG4CXX_DEBUG(logger, _T("jscripter::preload for stage ") << grp << _T(". ") << queue.size() << _T(" scripts initialized."));

    return true;
}

bool jscripter::execute( webEngine::task* tsk, detail::jscripter_block& code, webEngine::scan_data_ptr scData )
{
    LOG4CXX_INFO(logger, _T("jscripter::execute ") << code.name << _T(" >> ") << code.callable << _T("(ScanData)"));
    v8::Locker lock;
    v8::HandleScope hscope;
    v8::Persistent<v8::Context> ctx = js_exec->get_child_context();
    v8::Context::Scope scope(ctx);

    // set global task
    v8::Handle<v8::Object> _task = v8_wrapper::wrap_object<jsTask>(new jsTask(tsk));
    ctx->Global()->Set(v8::String::New("__watTask"), _task);
    // set global argument
    v8::Handle<v8::Object> _data = v8_wrapper::wrap_object<jsScanData>(new jsScanData(scData));
    ctx->Global()->Set(v8::String::New("__watScanData"), _data);
    // execute script
    std::string src;

    src = code.callable + _T("(__watScanData);");
    return js_exec->execute_string(src, code.name, true, true);
}

bool jscripter::load_queue_list( webEngine::task* tsk, detail::script_group grp, detail::jscripter_queue& queue )
{
    std::string group_name;
    switch(grp) {
    case detail::script_group::inventory:
        group_name = _T("inventory");
        break;
    case detail::script_group::audit:
        group_name = _T("audit");
        break;
    case detail::script_group::vulner:
        group_name = _T("vulner");
        break;
    default:
        LOG4CXX_ERROR(logger, _T("jscripter::load_queue_list - unknown stage name! Exiting"));
        return false;
    }
    if (tsk->IsSet(std::string(weoJsUseFiles))) {
        // load from files
        std::string basepath;
        webEngine::we_option opt = tsk->Option(std::string(weoJsBasePath));
        SAFE_GET_OPTION_VAL(opt, basepath, "");
        boost::filesystem::path dir(basepath);
        dir /= group_name + _T(".txt");
        if (boost::filesystem::exists(dir)) {
            ifstream inp(dir.string());
            if (inp.is_open()) {
                LOG4CXX_DEBUG(logger, _T("jscripter::load_queue_list from ") << dir.string());
                std::string line;
                while (!inp.eof()) {
                    std::getline(inp, line);
                    std::vector<std::string> dump;
                    boost::split(dump, line, boost::is_any_of("\t"));
                    if (dump.size() == 3) {
                        detail::jscripter_block code;
                        code.dbid = 0;
                        try {
                            code.priority = boost::lexical_cast<int>(dump[0]);
                            code.callable = dump[1];
                            code.name = dump[2];
                            code.code = "";
                            queue.push_back(code);
                            LOG4CXX_TRACE(logger, _T("jscripter::load_queue_list: ") << line);
                        }
                        catch (boost::bad_lexical_cast&) {
                            LOG4CXX_WARN(logger, _T("jscripter::load_queue_list - malformed record: ") << line);
                        }
                    }
                    else {
                        LOG4CXX_WARN(logger, _T("jscripter::load_queue_list - malformed record: ") << line);
                    }
                }
            }
        }
        else {
            LOG4CXX_ERROR(logger, _T("jscripter::load_queue_list - can't find stage directory: ") << dir.string());
            return false;
        }
    }
    else {
        // load from database
        LOG4CXX_FATAL(logger, _T("jscripter::load_queue_list - loading from database not implemented yet!"));
        return false;
    }
    return true;
}

bool jscripter::load_code( webEngine::task* tsk, detail::jscripter_block& code )
{
    if (tsk->IsSet(std::string(weoJsUseFiles))) {
        // load from files
        std::string basepath;
        webEngine::we_option opt = tsk->Option(std::string(weoJsBasePath));
        SAFE_GET_OPTION_VAL(opt, basepath, "");
        boost::filesystem::path dir(basepath);
        dir /= code.name;
        ifstream inp(dir.string());
        if (inp.is_open()) {
            LOG4CXX_ERROR(logger, _T("jscripter::load_code from: ") << dir.string());
            code.code = "";
            std::string line;
            while(!inp.eof()) {
                std::getline(inp, line);
                code.code += line;
                code.code += "\n";
            }
        }
        else {
            LOG4CXX_ERROR(logger, _T("jscripter::load_code - can't find: ") << dir.string());
            return false;
        }
    }
    else {
        // load from database
        LOG4CXX_FATAL(logger, _T("jscripter::load_code - loading from database not implemented yet!"));
        return false;
    }
    return true;
}

}
