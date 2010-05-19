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

#include <weiInventory.h>
#include <weScan.h>

namespace webEngine {

    class HttpInventory :
        public i_inventory
    {
    public:
        HttpInventory(engine_dispatcher* krnl, void* handle = NULL);
        virtual ~HttpInventory(void);

        // iwePlugin functions
        virtual i_plugin* get_interface(const string& ifName);
        virtual const string get_setup_ui( void );

        // i_inventory functions
        void start(task* tsk);
        void process_response(boost::shared_ptr<i_response> resp);
    protected:

        void add_url(transport_url link, HttpResponse *htResp, boost::shared_ptr<ScanData> scData);
        string_list ext_deny;
        string_list domain_allow;

        // processing options
        bool opt_in_host;
        bool opt_in_domain;
        bool opt_ignore_param;
        int  opt_max_depth;
        int  opt_ctype_method;

    };
} // namespace webEngine

#endif //__WEHTTPINVENTORY_H__