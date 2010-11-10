/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew Abramov aabramov@ptsecurity.ru

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
*/
#pragma once
#ifndef __WEWOT_H__
#define __WEWOT_H__
#include <weiVulner.h>
#include <weScan.h>
#include <boost/thread.hpp>

using namespace webEngine;

class wot_check :
    public i_vulner {
public:
    wot_check(webEngine::engine_dispatcher* krnl, void* handle = NULL);
    ~wot_check(void);

    virtual i_plugin* get_interface(const string& ifName);
    virtual const string get_setup_ui( void ) {
        return "";
    };
    virtual void init(task* tsk);
    virtual void process(task* tsk, scan_data_ptr scData);
    virtual void stop(task* tsk);
    virtual void pause(task* tsk, bool paused = true);
    virtual void process_response(i_response_ptr resp);

protected:
    typedef std::map<std::string, bool> site_data;

    bool is_on_run;
    site_data req_list;
};

#endif //__WEWOT_H__
