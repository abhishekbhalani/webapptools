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
#include "wot_check.h"
#include <weHelper.h>
#include <weDispatch.h>
#include <weTask.h>
#include <weTagScanner.h>

wot_check::wot_check( webEngine::engine_dispatcher* krnl, void* handle /*= NULL*/ ):
    i_vulner(krnl, handle)
{
    pluginInfo.interface_name = "wot_check";
    pluginInfo.interface_list.push_back("wot_check");
    pluginInfo.plugin_desc = "Site information from WebOfTrust database";
    pluginInfo.plugin_id = "75A0ABE8C7A8"; //{E9C666D6-0B89-4c86-8169-75A0ABE8C7A8}
    // internal initialization
    is_on_run = false;
}

wot_check::~wot_check( void )
{
}

i_plugin* wot_check::get_interface( const string& ifName )
{
    LOG4CXX_TRACE(logger, "wot_check::get_interface " << ifName);
    if (iequals(ifName, "wot_check"))
    {
        LOG4CXX_DEBUG(logger, "wot_check::get_interface found!");
        usageCount++;
        return (this);
    }
    return i_vulner::get_interface(ifName);
}

void wot_check::init( webEngine::task* tsk )
{
    parent_task = tsk;
    is_on_run = true;
    req_list.clear();
}

void wot_check::process( webEngine::task* tsk, webEngine::scan_data_ptr scData )
{
    site_data::iterator sit;
    transport_url dom_url;
    string domain;
    string url;
    i_request_ptr result;

    parent_task = tsk;
    dom_url.assign(scData->object_url);
    domain = dom_url.host;
    sit = req_list.find(domain);
    if (sit == req_list.end() || sit->second == false) {
        req_list[domain] = false;
        LOG4CXX_TRACE(logger, "wot_check::process - search data for " << domain);
        url = "http://api.mywot.com/0.4/public_query2?target=" + domain;
        result = i_request_ptr(new webEngine::HttpRequest(url));
        result->depth(0);
        result->ID(pluginInfo.plugin_id);
        result->context = this;
        result->processor = i_vulner::response_dispatcher;
        parent_task->get_request_async(result);
        req_list[domain] = true; // prevent sending requests
    }
}

void wot_check::stop( webEngine::task* tsk )
{
    is_on_run = false;
}

void wot_check::pause( task* tsk, bool paused /*= true*/ )
{
    if (paused) {
        is_on_run = false;
    }
    else {
        is_on_run = true;
    }
}

void wot_check::process_response( webEngine::i_response_ptr resp )
{
    string sc;
    string domain;
    string rurl = resp->RealUrl().tostring();
    HttpResponse *ht_resp = dynamic_cast<HttpResponse*>(resp.get());
    i_request_ptr result;

    LOG4CXX_DEBUG(logger, "wot_check::process_response: " << rurl);

    domain = resp->RealUrl().host;
    if (ht_resp->HttpCode() > 399) {
        LOG4CXX_WARN(logger, "wot_check::process_response - bad respose: " << ht_resp->HttpCode() << " " << rurl);
        req_list[domain] = false;
    } // if error
    else if (ht_resp->HttpCode() > 299) {
        LOG4CXX_WARN(logger, "wot_check::process_response - want redirect");
        req_list[domain] = true; // prevent sending requests
        string url = ht_resp->Headers().find_first("Location");
        if (!url.empty()) {
            LOG4CXX_DEBUG(iLogger::GetLogger(), "wot_check::process_response redirected to " << url);
            bool to_process = true;
            transport_url baseUrl = ht_resp->BaseUrl();
            baseUrl.assign_with_referer(url);
            LOG4CXX_DEBUG(logger, "wot_check::process_response: reconstructed url is " << baseUrl.tostring());
            result = i_request_ptr(new webEngine::HttpRequest(url));
            result->depth(0);
            result->ID(pluginInfo.plugin_id);
            result->context = this;
            result->processor = i_vulner::response_dispatcher;
            parent_task->get_request_async(result);
        }
    } // if redirect
    else {
        // normal response
        req_list[domain] = true;
        string wparam(resp->Data().begin(), resp->Data().end());
        string v_id = pluginInfo.plugin_id + "/0";
        parent_task->add_vulner(v_id, wparam, "0", 0);
    } // if normal response
}
