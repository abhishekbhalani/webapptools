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

#ifndef __JSCRIPTERPLUGIN__H__
#define __JSCRIPTERPLUGIN__H__

#include <weiInventory.h>
#include <weiAudit.h>
#include <weiVulner.h>
#include "jsWrappers/jsBrowser.h"

namespace jscripter
{
namespace detail
{
class jscripter_audit;
class jscripter_inventory;
class jscripter_vulner;
class jscripter_block;
typedef std::vector<jscripter_block> jscripter_queue;
enum script_group {unknown, inventory, audit, vulner};
}

class jscripter
    : public webEngine::i_plugin
    , private boost::noncopyable
{
public:
    explicit jscripter(webEngine::engine_dispatcher* krnl, void* handle = NULL);
    virtual ~jscripter(void);
    virtual webEngine::i_plugin* get_interface(const std::string& ifName);
    virtual void init(webEngine::task* tsk);
    virtual void pause(webEngine::task* tsk, bool paused = true);
    virtual void stop(webEngine::task* tsk);
    virtual const std::string get_setup_ui(void);

    // "Internal" functions to provide functionality to subsystems
    bool preload (webEngine::task* tsk, detail::script_group grp, detail::jscripter_queue& queue);
    bool execute (webEngine::task* tsk, detail::jscripter_block& code, webEngine::scan_data_ptr scData);
private:
    bool load_queue_list(webEngine::task* tsk, detail::script_group grp, detail::jscripter_queue& queue);
    bool load_code(webEngine::task* tsk, detail::jscripter_block& code);

    boost::scoped_ptr<webEngine::jsBrowser> js_exec;
    boost::scoped_ptr<detail::jscripter_audit> m_audit;
    boost::scoped_ptr<detail::jscripter_inventory> m_inventory;
    boost::scoped_ptr<detail::jscripter_vulner> m_vulner;
};

#define weoJsInventory  "jscripter/ExecuteInventory"
#define weoJsAudit      "jscripter/ExecuteAudit"
#define weoJsVulners    "jscripter/ExecuteVulners"
#define weoJsUseFiles   "jscripter/UseFiles"
#define weoJsBasePath   "jscripter/FilesBasePath"

namespace detail
{
class jscripter_inventory : public webEngine::i_inventory
{
public:
    jscripter_inventory(jscripter& js, webEngine::engine_dispatcher* krnl, void* handle = NULL);
    virtual ~jscripter_inventory(void);

    virtual void process(webEngine::task* tsk, webEngine::scan_data_ptr scData);
    virtual void init(webEngine::task* tsk);
    virtual void pause(webEngine::task* tsk, bool paused = true);
    virtual void stop(webEngine::task* tsk);
private:
    jscripter& m_scripter;
    jscripter_queue m_queue;
};

class jscripter_audit : public webEngine::i_audit
{
public:
    jscripter_audit(jscripter& js, webEngine::engine_dispatcher* krnl, void* handle = NULL);
    virtual ~jscripter_audit(void);
    virtual void process(webEngine::task* tsk, webEngine::scan_data_ptr scData);
    virtual void process_response(webEngine::i_response_ptr resp);
    virtual void init(webEngine::task* tsk);
    virtual void pause(webEngine::task* tsk, bool paused = true);
    virtual void stop(webEngine::task* tsk);
private:
    jscripter& m_scripter;
    jscripter_queue m_queue;
};

class jscripter_vulner : public webEngine::i_vulner
{
public:
    jscripter_vulner(jscripter& js, webEngine::engine_dispatcher* krnl, void* handle = NULL);
    virtual ~jscripter_vulner(void);
    virtual void process(webEngine::task* tsk, webEngine::scan_data_ptr scData);
    virtual void process_response(webEngine::i_response_ptr resp);
    virtual void init(webEngine::task* tsk);
    virtual void pause(webEngine::task* tsk, bool paused = true);
    virtual void stop(webEngine::task* tsk);
private:
    jscripter& m_scripter;
    jscripter_queue m_queue;
};

class jscripter_block
{
public:
    int         priority;
    int         dbid;
    std::string name;
    std::string callable;
    std::string code;

    struct order {
        bool operator()(jscripter_block & a, jscripter_block & b) const {
            if (a.priority < b.priority) {
                return true;
            }
            return false;
        }
    };

    struct remover {
        bool operator()(jscripter_block & a) {
            return a.code.empty();
        }
    };

};

}
}

#endif //__JSCRIPTERPLUGIN__H__
