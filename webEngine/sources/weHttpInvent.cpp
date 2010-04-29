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
#include <weHelper.h>
#include <weDispatch.h>
#include <weTask.h>
#include <weHttpInvent.h>
#include <weScan.h>

static string xrc = "<plugin><category label='Basic settings' name='generic'>\
 <option name='httpInventory/BaseURL' label='start transport_url' type='8' control='string' mode='window'>/</option>\
 <category label='Crawler' name='crawler'>\
  <option name='' label='Crawling mode'>&lt;composed&gt;\
   <option name='httpInventory/StayInDir' label='Stay in directory' type='6' control='bool' chkbox='1'>1</option>\
   <option name='httpInventory/StayInHost' label='Stay in host' type='6' control='bool' chkbox='1'>0</option>\
   <option name='httpInventory/StayInIP' label='Stay in domain and IP' type='6' control='bool' chkbox='1'>0</option>\
   <option name='httpInventory/StayInDomain' label='Stay in domain' type='6' control='bool' chkbox='1'>0</option>\
  </option>\
  <option name='httpInventory/ScanDepth' label='Scan depth' type='2' control='int'>-1</option>\
  <option name='httpInventory/noParamUrl' label='Ignore transport_url parameters' type='6' control='bool' chkbox='1'>0</option>\
 </category>\
 <option name='httpInventory/ParallelReq' label='Number of downloaders' type='2' control='int' min='1' max='100'>10</option>\
 <option name='httpInventory/AllowedCType' label='Allowed content-type' type='2' control='enum'>Any;empty and \"text/*\";only \"text/*\";empty and \"text/html\";only \"text/html\"</option>\
 <option name='httpInventory/Subdomains' label='Allowed subdomains list' type='8' control='string' mode='window'></option>\
</category></plugin>";

namespace webEngine {

HttpInventory::HttpInventory(engine_dispatcher* krnl, void* handle /*= NULL*/) :
    i_inventory(krnl, handle)
{
    pluginInfo.interface_name = "httpInventory";
    pluginInfo.interface_list.push_back("httpInventory");
    pluginInfo.plugin_desc = "Inventory through HTTP protocol";
    pluginInfo.plugin_id = "AB7ED6E5A7B3"; //{7318EAB5-4253-4a31-8284-AB7ED6E5A7B3}
}

HttpInventory::~HttpInventory(void)
{
}

i_plugin* HttpInventory::get_interface( const string& ifName )
{
    LOG4CXX_TRACE(logger, "HttpInventory::get_interface " << ifName);
    if (iequals(ifName, "httpInventory"))
    {
        LOG4CXX_DEBUG(logger, "HttpInventory::get_interface found!");
        usageCount++;
        return (this);
    }
    return i_inventory::get_interface(ifName);
}

const string HttpInventory::get_setup_ui( void )
{
    return xrc;
}

void HttpInventory::start( task* tsk )
{
    transport_url   start_url;

    if (tsk)
    {
        parent_task = tsk;
        if (kernel)
        {
            string host;
            wOption opt = tsk->Option("scan_host");
            SAFE_GET_OPTION_VAL(opt, host, "");
            if (host != "")
            {
                string path;
                int iData;
                opt = tsk->Option("httpInventory/"weoParallelReq);
                SAFE_GET_OPTION_VAL(opt, iData, 1);
                /// @todo Verify existing option and correctly update it
                tsk->Option(weoParallelReq, iData);

                // create list of the blocked extension
                opt = tsk->Option("httpInventory/"weoDeniedFileTypes);
                SAFE_GET_OPTION_VAL(opt, path, 1);
                if (path != "")
                {
                    size_t pos = path.find(';');
                    ext_deny.clear();
                    while(pos != string::npos) {
                        string ext = path.substr(0, pos);
                        ext_deny.push_back(ext);
                        if (pos < path.length())
                        {
                            path = path.substr(pos+1);
                        }
                        else {
                            path = "";
                        }
                        pos = path.find(';');
                    }
                }

                // create list of allowed sub-domains
                opt = tsk->Option("httpInventory/"weoDomainsAllow);
                SAFE_GET_OPTION_VAL(opt, path, 1);
                if (path != "")
                {
                    size_t pos = path.find(';');
                    domain_allow.clear();
                    while(pos != string::npos) {
                        string ext = path.substr(0, pos);
                        domain_allow.push_back(ext);
                        if (pos < path.length())
                        {
                            path = path.substr(pos+1);
                        }
                        else {
                            path = "";
                        }
                        pos = path.find(';');
                    }
                }

                opt = tsk->Option("httpInventory/BaseURL");
                SAFE_GET_OPTION_VAL(opt, path, "");
                start_url.host = host;
                start_url.request = path;
                host += path;
                LOG4CXX_INFO(logger, "HttpInventory::start: start scanning from: " << host << " ==> " << start_url.tostring());
                HttpRequest* req = new HttpRequest;
                req->RequestUrl(start_url);
                LOG4CXX_TRACE(logger, "HttpInventory::start: start request = " << req->RequestUrl().tostring());
                req->processor = HttpInventory::response_dispatcher;
                req->context = (void*)this;
                parent_task->GetRequestAsync(req);
            }
            else {
                LOG4CXX_WARN(logger, "HttpInventory::start: Can't find hostname. Finishing.");
            }
        }
        else {
            LOG4CXX_WARN(logger, "HttpInventory::start: No kernel given - can't read options. Finishing.");
        }
    }
    else {
        LOG4CXX_WARN(logger, "HttpInventory::start: No parent task - can't process requests. Finishing.");
    }
}

void HttpInventory::process_response( i_response *resp )
{
    HttpResponse* htResp;

    try {
        htResp = reinterpret_cast<HttpResponse*>(resp);
    }
    catch (...) {
        LOG4CXX_ERROR(logger, "HttpInventory::process_response: The response from " << resp->BaseUrl().tostring() << " isn't the HttpResponse!");
        return;
    }
    // process response
    LOG4CXX_TRACE(logger, "HttpInventory::process_response: process response with code=" << htResp->HttpCode());
    ScanData* scData = parent_task->GetScanData(htResp->BaseUrl().tostring(), htResp->RealUrl().tostring());
    if (scData->parentID == "")
    {
        scData->parentID = htResp->ID();
    }
    if (scData->dataID == "")
    {
        scData->dataID = kernel->storage()->generate_id(weObjTypeScan);
        scData->respCode = htResp->HttpCode();
        scData->downloadTime = htResp->DownloadTime();
        scData->dataSize = htResp->Data().size();
        scData->scan_depth = htResp->depth();
        scData->content_type = htResp->ContentType();
    }

    /// @todo process options
    if (htResp->HttpCode() >= 300 && htResp->HttpCode() < 400) {
        // redirections
        LOG4CXX_TRACE(logger, "HttpInventory::process_response: process redirect");
        string url = htResp->Headers().find_first("Location");
        if (!url.empty()) {
            LOG4CXX_DEBUG(iLogger::GetLogger(), "task_executor: redirected to " << url);
            bool to_process = true;
            transport_url baseUrl = htResp->BaseUrl();
            baseUrl.assign_with_referer(url);
            LOG4CXX_DEBUG(logger, "HttpInventory::process_response: reconstructed url is " << baseUrl.tostring());
            add_url(baseUrl, htResp, scData);
        }
    }
    /// @todo select appropriate parser
    if ((htResp->HttpCode() > 0 && htResp->HttpCode() < 300) || htResp->Data().size() > 0)
    {
        string cType = htResp->ContentType();
        wOption opt = parent_task->Option("httpInventory/AllowedCType");
        int cTypeMethod;
        SAFE_GET_OPTION_VAL(opt, cTypeMethod, 0); // default - any type
        LOG4CXX_TRACE(logger, "HttpInventory::process_response: content-type analyze method = " << cTypeMethod);
        bool cTypeProcess = false;
        switch(cTypeMethod)
        {
        case 0: // any content-type
            cTypeProcess = true;
            break;
        case 1: // empty and "text/*"
            if (cType == "" || starts_with(cType, "text/"))
            {
                cTypeProcess = true;
            }
        	break;
        case 2: // only "text/*"
            if (starts_with(cType, "text/"))
            {
                cTypeProcess = true;
            }
        	break;
        case 3: // empty and "text/html"
            if (cType == "" || starts_with(cType, "text/html"))
            {
                cTypeProcess = true;
            }
            break;
        case 4: // only "text/*"
            if (starts_with(cType, "text/html"))
            {
                cTypeProcess = true;
            }
            break;
        default:
            cTypeProcess = false;
            LOG4CXX_WARN(logger, "HttpInventory::process_response: unknown content-type analyze method = " << cTypeMethod);
            break;
        }
        if (cTypeProcess)
        {
            LOG4CXX_TRACE(logger, "HttpInventory::process_response: parse document");
            HtmlDocument *parser = new HtmlDocument;
            bool saveParser;
            EntityList lst;

            boost::posix_time::ptime pretm = boost::posix_time::microsec_clock::local_time();
            saveParser = parser->ParseData(resp);
            if (scData->parsedData != NULL) {
                saveParser = false;
            }
            if (saveParser)
            {
                scData->parsedData = parser;
            }
            else {
                // prevent destruction on update
                scData->parsedData->add_ref();
            }
            boost::posix_time::ptime postm = boost::posix_time::microsec_clock::local_time();
            boost::posix_time::time_period duration(pretm, postm);
            LOG4CXX_DEBUG(logger, "HttpInventory::process_response " << htResp->Data().size() << "bytes parsed at " << duration.length().total_milliseconds() << " milliseconds");
            LOG4CXX_DEBUG(logger, "HttpInventory::process_response: search for links");
            lst = parser->FindTags("a");
            if (lst.size() > 0) {
                iEntity* ent = NULL;
                EntityList::iterator iEnt;
                string href;
                for (iEnt = lst.begin(); iEnt != lst.end(); iEnt++) {
                    href = (*iEnt)->Attr("href");
                    if (href != "") {
                        transport_url link;
                        link.assign_with_referer(href, &(htResp->RealUrl()));
                        LOG4CXX_TRACE(logger, "HttpInventory::process_response: <a href...> tag. url=" << link.tostring());
                        add_url(link, htResp, scData);
                    } // end href attribute
                } // end <a ...> loop
                ClearEntityList(lst);
            } // end <a ...> tags processing
            // todo: process other kind of links
            // CSS
            // scripts
            // objects
            // images
            lst = parser->FindTags("img");
            if (lst.size() > 0)
            {
                webEngine::iEntity* ent = NULL;
                webEngine::EntityList::iterator iEnt;
                string href;
                for (iEnt = lst.begin(); iEnt != lst.end(); iEnt++) {
                    href = (*iEnt)->Attr("src");
                    if (href != "") {
                        transport_url link;
                        link.assign_with_referer(href, &(htResp->RealUrl()));
                        LOG4CXX_DEBUG(logger, "HttpInventory::process_response: IMG url = " << link.tostring());
                        add_url(link, htResp, scData);
                    } // end src attribute
                } // end <img ...> loop
                ClearEntityList(lst);
            } // end <img ...> tags processing
            // frames
            lst = parser->FindTags("frame");
            if (lst.size() > 0)
            {
                webEngine::iEntity* ent = NULL;
                webEngine::EntityList::iterator iEnt;
                string href;
                for (iEnt = lst.begin(); iEnt != lst.end(); iEnt++) {
                    href = (*iEnt)->Attr("src");
                    if (href != "") {
                        transport_url link;
                        link.assign_with_referer(href, &(htResp->RealUrl()));
                        LOG4CXX_DEBUG(logger, "HttpInventory::process_response: FRAME url = " << link.tostring());
                        add_url(link, htResp, scData);
                    } // end src attribute
                } // end <iframe ...> loop
                ClearEntityList(lst);
            } // end <iframe ...> tags processing
            // iframes
            lst = parser->FindTags("iframe");
            if (lst.size() > 0)
            {
                webEngine::iEntity* ent = NULL;
                webEngine::EntityList::iterator iEnt;
                string href;
                for (iEnt = lst.begin(); iEnt != lst.end(); iEnt++) {
                    href = (*iEnt)->Attr("src");
                    if (href != "") {
                        transport_url link;
                        link.assign_with_referer(href, &(htResp->RealUrl()));
                        LOG4CXX_DEBUG(logger, "HttpInventory::process_response: IFRAME url = " << link.tostring());
                        add_url(link, htResp, scData);
                    } // end src attribute
                } // end <iframe ...> loop
                ClearEntityList(lst);
            } // end <iframe ...> tags processing
            // forms
            // meta
            lst = parser->FindTags("meta");
            if (lst.size() > 0)
            {
                iEntity* ent = NULL;
                EntityList::iterator iEnt;
                string href;
                for (iEnt = lst.begin(); iEnt != lst.end(); iEnt++) {
                    href = (*iEnt)->Attr("http-equiv");
                    if (boost::iequals(href, "refresh")) {
                        href = (*iEnt)->Attr("content");
                        if (href != "") {
                            transport_url link;
                            string lc = href;
                            to_lower(lc);
                            size_t np = lc.find("url=");
                            if (np != std::string::npos) {
                                href = href.substr(np+4);
                            }
                            link.assign_with_referer(href, &(htResp->RealUrl()));
                            LOG4CXX_DEBUG(logger, "HttpInventory::process_response: META url = " << link.tostring());
                            add_url(link, htResp, scData);
                        }
                    } // end http-equiv attribute
                } // end <meta ...> loop
                ClearEntityList(lst);
            } // end <meta ...> tags processing
            // etc ???

            // cleanup
            if (!saveParser) {
                if (parser != NULL) {
                    parser->release();
                }
                else {
                    htResp->release();
                }
            }
        }
        else {
            LOG4CXX_WARN(logger, "HttpInventory::process_response: inconsistent content-type: " << cType);
        }
    }
    if ((htResp->HttpCode() > 0 && htResp->HttpCode() < 400) || htResp->HttpCode() >= 500)
    {
        parent_task->SetScanData(scData);
    }
}

void HttpInventory::add_url( transport_url link, HttpResponse *htResp, ScanData *scData )
{
    bool allowed = true;
    // verify blocked file types
    string path = link.request;
    int pos = path.find_last_of('.');
    if (pos != string::npos && ext_deny.size() > 0)
    {
        path = path.substr(pos+1);
        LOG4CXX_TRACE(logger, "HttpInventory::add_url: Found extension: " << path << "; Deny list size is " << ext_deny.size());
        for (size_t i = 0; i < ext_deny.size(); i++) {
            if (path == ext_deny[i]) {
                allowed = false;
                break;
            }
        }
    }
    add_http_url(logger, link, htResp->RealUrl(), parent_task, scData, &tasklist, htResp->depth(), (void*)this, HttpInventory::response_dispatcher, allowed);
}

void add_http_url(log4cxx::LoggerPtr logger, transport_url link, transport_url baseUrl, task* task,
                  ScanData *scData, map<string, bool> *tasklist, int scan_depth, void* context, fnProcessResponse* processor,
                  bool download /*= true*/)
{
    bool to_process = true;
    int max_depth = 0;
    wOption opt;

    LOG4CXX_DEBUG(logger, "add_http_url: check for the url=" << link.tostring());
    if (!link.is_host_equal(baseUrl))
    {
        if (task->IsSet("httpInventory/"weoStayInHost))
        {
            to_process = false;
        }
        LOG4CXX_TRACE(logger, "HttpInventory::add_url: weoStayInHost check " << to_process << " (" << link.tostring() << ")");
    }
    if (!link.is_domain_equal(baseUrl))
    {
        if (task->IsSet("httpInventory/"weoStayInDomain))
        {
            to_process = false;
        }
        LOG4CXX_TRACE(logger, "HttpInventory::add_url: weoStayInDomain check << " << to_process << " (" << link.tostring() << ")");
    }

    opt = task->Option("httpInventory/"weoScanDepth);
    SAFE_GET_OPTION_VAL(opt, max_depth, 0);

    if ( max_depth > 0 && scan_depth >= max_depth) {
        LOG4CXX_DEBUG(logger, "HttpInventory::add_url: maximum scanning depth reached! (" << max_depth << ")");
        to_process = false;
    }

    if (to_process)
    {
        string u_req = link.tostring();
        if (task->IsSet("httpInventory/"weoIgnoreUrlParam)) {
            u_req = link.tostring_noparam();
        }
        LOG4CXX_TRACE(logger, "HttpInventory::add_http_url: weoIgnoreUrlParam check << " << u_req);
        if (tasklist->find(u_req) == tasklist->end())
        {
            if (download) {
                HttpRequest* new_url = new HttpRequest(u_req);
                new_url->depth(scan_depth + 1);
                new_url->ID(scData->dataID);
                (*tasklist)[u_req] = true;
                new_url->processor = processor;
                new_url->context = context;
                task->GetRequestAsync(new_url);
            }
            else {
                LOG4CXX_DEBUG(logger, "HttpInventory::add_http_url: not need to download " << u_req);
                // make the pseudo-responce
                ScanData* scData = task->GetScanData(u_req, u_req);
                if (scData->dataID == "")
                {
                    //scData->dataID = ;
                    scData->respCode = 204; // 204 No Content;  The server successfully processed the request, but is not returning any content
                    scData->downloadTime = 0;
                    scData->dataSize = 0;
                    scData->scan_depth = scan_depth + 1;
                    scData->content_type = "application/octet-stream";
                    scData->parsedData = NULL;
                    task->SetScanData(scData);
                }
            }
        }
        else
        {
            // add parent to existing scan data
        }
    }
}
} // namespace webEngine
