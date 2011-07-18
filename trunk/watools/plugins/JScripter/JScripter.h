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

namespace jscripter
{
namespace detail
{
class jscripter_audit;
class jscripter_inventory;
class jscripter_vulner;
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
private:
    boost::scoped_ptr<detail::jscripter_audit> m_audit;
    boost::scoped_ptr<detail::jscripter_inventory> m_inventory;
    boost::scoped_ptr<detail::jscripter_vulner> m_vulner;
};

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
};
}
}

#endif //__JSCRIPTERPLUGIN__H__
