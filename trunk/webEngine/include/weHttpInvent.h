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
#pragma once
#ifndef __WEHTTPINVENTORY_H__
#define __WEHTTPINVENTORY_H__

#include "weiInventory.h"
#include "weScan.h"
#include "weHTTP.h"
#include "weConstants.h"

namespace webEngine {

class http_inventory :
    public i_inventory {
public:
    http_inventory(engine_dispatcher* krnl, void* handle = NULL);
    virtual ~http_inventory(void);

    // i_plugin functions
    virtual i_plugin* get_interface(const string& ifName);
    virtual const string get_setup_ui( void );
    virtual void init(task* tsk);
    virtual void pause(task* tsk, bool paused = true) {}
    virtual void stop(task* tsk) {}

    // i_inventory functions
    void process(task* tsk, scan_data_ptr scData);
    void add_url(transport_url link, HttpResponse *htResp, boost::shared_ptr<ScanData> scData);

protected:
    string_list ext_deny;
    string_list domain_allow;

    // processing options
    bool opt_in_dlist;
    bool opt_in_dir;
    bool opt_in_ip;
    bool opt_in_host;
    bool opt_in_domain;
    int opt_auth_methods;
    string opt_auth_username;
    string opt_auth_password;
    string opt_auth_domain;
    vector<string> opt_auth_form_params;
    int  opt_ignore_param;
    int  opt_max_depth;
    int  opt_ctype_method;

};
} // namespace webEngine

#endif //__WEHTTPINVENTORY_H__