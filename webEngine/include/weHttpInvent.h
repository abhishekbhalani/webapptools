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

        // i_plugin functions
        virtual i_plugin* get_interface(const string& ifName);
        virtual const string get_setup_ui( void );
        virtual void init(task* tsk);
        virtual void pause(task* tsk, bool paused = true) {}
        virtual void stop(task* tsk) {}

        // i_inventory functions
        void process(task* tsk, scan_data_ptr scData);
    protected:

        void add_url(transport_url link, HttpResponse *htResp, boost::shared_ptr<ScanData> scData);
        string_list ext_deny;
        string_list domain_allow;

        // processing options
        bool opt_in_host;
        bool opt_in_domain;
        int  opt_ignore_param;
        int  opt_max_depth;
        int  opt_ctype_method;

    };
} // namespace webEngine

/// allowed all hosts on same IP address
#define weoStayInIP          "httpInventory/stay_in_ip"
/// allowed all domains in the list
#define weoStayInDomainList  "httpInventory/stay_in_domain_list"
/// do not leave domain of the request (second-level or higher) (bool)
#define weoStayInDomain      "httpInventory/stay_in_domain"
/// includes weoStayInDomain (bool)
#define weoStayInHost        "httpInventory/stay_in_host"
/// includes woeStayInHost & weoStayInDomain (bool)
#define weoStayInDir         "httpInventory/stay_in_dir"
/// links following depth (integer)
#define weoScanDepth         "httpInventory/scan_depth"
/// semicolon separated list of the denied file types (by extensions)
#define weoDeniedFileTypes   "httpInventory/denied_file_types"
/// semicolon separated list of the allowed sub-domains
#define weoDomainsAllow      "httpInventory/domains_allow"
/// URL parameters processing mode
#define weoIgnoreUrlParam    "httpInventory/url_param"
/// allowed content-type
#define weoAllowedCTypes     "httpInventory/AllowedCType"
/// list of urls to ignore
#define weoIgnoreUrlList     "httpInventory/ignore_urls"

#endif //__WEHTTPINVENTORY_H__