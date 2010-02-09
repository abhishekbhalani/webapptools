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
 <option name='httpInventory/BaseURL' label='Start URL' type='8' control='string' mode='window'>/</option>\
 <category label='Crawler' name='crawler'>\
  <option name='' label='Crawling mode'>&lt;composed&gt;\
   <option name='httpInventory/StayInDir' label='Stay in directory' type='6' control='bool' chkbox='1'>1</option>\
   <option name='httpInventory/StayInHost' label='Stay in host' type='6' control='bool' chkbox='1'>0</option>\
   <option name='httpInventory/StayInIP' label='Stay in domain and IP' type='6' control='bool' chkbox='1'>0</option>\
   <option name='httpInventory/StayInDomain' label='Stay in domain' type='6' control='bool' chkbox='1'>0</option>\
  </option>\
  <option name='httpInventory/ScanDepth' label='Scan depth' type='2' control='int'>-1</option>\
  <option name='httpInventory/noParamUrl' label='Ignore URL parameters' type='6' control='bool' chkbox='1'>0</option>\
 </category>\
 <option name='httpInventory/ParallelReq' label='Number of downloaders' type='2' control='int' min='1' max='100'>10</option>\
 <option name='httpInventory/AllowedCType' label='Allowed content-type' type='2' control='enum'>Any;empty and \"text/*\";only \"text/*\";empty and \"text/html\";only \"text/html\"</option>\
 <option name='httpInventory/Subdomains' label='Allowed subdomains list' type='8' control='string' mode='window'></option>\
</category></plugin>";

namespace webEngine {

HttpInventory::HttpInventory(Dispatch* krnl, void* handle /*= NULL*/) :
    iInventory(krnl, handle)
{
    pluginInfo.IfaceName = "httpInventory";
    pluginInfo.IfaceList.push_back("httpInventory");
    pluginInfo.PluginDesc = "Inventory through HTTP protocol";
    pluginInfo.PluginId = "AB7ED6E5A7B3"; //{7318EAB5-4253-4a31-8284-AB7ED6E5A7B3}
}

HttpInventory::~HttpInventory(void)
{
}

void* HttpInventory::GetInterface( const string& ifName )
{
    LOG4CXX_TRACE(logger, "HttpInventory::GetInterface " << ifName);
    if (iequals(ifName, "httpInventory"))
    {
        LOG4CXX_DEBUG(logger, "HttpInventory::GetInterface found!");
        usageCount++;
        return (void*)(this);
    }
    return iInventory::GetInterface(ifName);
}

const string HttpInventory::GetSetupUI( void )
{
    return xrc;
}

void HttpInventory::Start( Task* tsk )
{
    URL   start_url;

    if (tsk)
    {
        task = tsk;
        if (kernel)
        {
            string host;
            wOption opt = tsk->Option("scan_host");
            SAFE_GET_OPTION_VAL(opt, host, "");
            if (host != "")
            {
                wOption opt;
                string path;
                int iData;
                opt = tsk->Option("httpInventory/"weoParallelReq);
                SAFE_GET_OPTION_VAL(opt, iData, 1);
                /// @todo Verify existing option and correctly update it
                tsk->Option(weoParallelReq, iData);

                opt = tsk->Option("httpInventory/BaseURL");
                SAFE_GET_OPTION_VAL(opt, path, "");
                start_url.host = host;
                start_url.request = path;
                host += path;
                LOG4CXX_INFO(logger, "HttpInventory::Start: start scanning from: " << host << " ==> " << start_url.ToString());
                HttpRequest* req = new HttpRequest;
                req->RequestUrl(start_url);
                LOG4CXX_TRACE(logger, "HttpInventory::Start: start request = " << req->RequestUrl().ToString());
                req->processor = HttpInventory::ResponseDispatcher;
                req->context = (void*)this;
                task->GetRequestAsync(req);
            }
            else {
                LOG4CXX_WARN(logger, "HttpInventory::Start: Can't find hostname. Finishing.");
            }
        }
        else {
            LOG4CXX_WARN(logger, "HttpInventory::Start: No kernel given - can't read options. Finishing.");
        }
    }
    else {
        LOG4CXX_WARN(logger, "HttpInventory::Start: No parent task - can't process requests. Finishing.");
    }
}

void HttpInventory::ProcessResponse( iResponse *resp )
{
    HttpResponse* htResp;

    try {
        htResp = reinterpret_cast<HttpResponse*>(resp);
    }
    catch (...) {
        LOG4CXX_ERROR(logger, "HttpInventory::ProcessResponse: The response from " << resp->BaseUrl().ToString() << " isn't the HttpResponse!");
        return;
    }
    // process response
    LOG4CXX_TRACE(logger, "HttpInventory::ProcessResponse: process response with code=" << htResp->HttpCode());
    ScanData* scData = task->GetScanData(htResp->BaseUrl().ToString(), htResp->RealUrl().ToString());
    if (scData->parentID == "")
    {
        scData->parentID = htResp->ID();
    }
    if (scData->dataID == "")
    {
        scData->dataID = kernel->Storage()->GenerateID(weObjTypeScan);
        scData->respCode = htResp->HttpCode();
        scData->downloadTime = htResp->DownloadTime();
        scData->dataSize = htResp->Data().size();
    }
    //task->AddScanData();

    /// @todo process options
    if (htResp->HttpCode() >= 300 && htResp->HttpCode() < 400) {
        // redirections
        LOG4CXX_TRACE(logger, "HttpInventory::ProcessResponse: process redirect");
        string url = htResp->Headers().FindFirst("Location");
        if (!url.empty()) {
            LOG4CXX_DEBUG(iLogger::GetLogger(), "task_executor: redirected to " << url);
            bool to_process = true;
            URL baseUrl = htResp->BaseUrl();
            baseUrl.Restore(url);
            // !!! todo LOG4CXX_TRACE
            LOG4CXX_DEBUG(logger, "HttpInventory::ProcessResponse: reconstructed url is " << baseUrl.ToString());
            if (!baseUrl.IsHostEquals(htResp->RealUrl()))
            {
                if (task->IsSet("httpInventory/"weoStayInHost))
                {
                    to_process = false;
                }
                LOG4CXX_TRACE(logger, "HttpInventory::ProcessResponse: weoStayInHost check " << to_process);
            }
            if (!baseUrl.IsDomainEquals(htResp->RealUrl()))
            {
                if (task->IsSet("httpInventory/"weoStayInDomain))
                {
                    to_process = false;
                }
                LOG4CXX_TRACE(logger, "HttpInventory::ProcessResponse: weoStayInDomain check << " << to_process);
            }

            if (to_process)
            {
                string u_req = baseUrl.ToString();
                if (task->IsSet("httpInventory/"weoIgnoreUrlParam)) {
                    u_req = baseUrl.ToStringNoParam();
                }
                LOG4CXX_TRACE(logger, "HttpInventory::ProcessResponse: weoIgnoreUrlParam check << " << u_req);
                if (tasklist.find(u_req) == tasklist.end())
                {
                    HttpRequest* new_url = new HttpRequest(baseUrl.ToString());
                    new_url->ID(scData->dataID);
                    tasklist[u_req] = true;
                    new_url->processor = HttpInventory::ResponseDispatcher;
                    new_url->context = (void*)this;
                    task->GetRequestAsync(new_url);
                }
                else
                {
                    // add parent to existing scan data
                }
            }
        }
    }
    /// @todo select appropriate parser
    if (htResp->HttpCode() >= 200 && htResp->HttpCode() < 300)
    {
        string cType = htResp->ContentType();
        wOption opt = task->Option("httpInventory/AllowedCType");
        int cTypeMethod;
        SAFE_GET_OPTION_VAL(opt, cTypeMethod, 0); // default - any type
        LOG4CXX_TRACE(logger, "HttpInventory::ProcessResponse: content-type analyze method = " << cTypeMethod);
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
            LOG4CXX_WARN(logger, "HttpInventory::ProcessResponse: unknown content-type analyze method = " << cTypeMethod);
            break;
        }
        if (cTypeProcess)
        {
            LOG4CXX_TRACE(logger, "HttpInventory::ProcessResponse: parse document");
            HtmlDocument parser;
            EntityList lst;

            boost::posix_time::ptime pretm = boost::posix_time::microsec_clock::local_time();
            parser.ParseData(resp);
            boost::posix_time::ptime postm = boost::posix_time::microsec_clock::local_time();
            boost::posix_time::time_period duration(pretm, postm);
            LOG4CXX_DEBUG(logger, "HttpInventory::ProcessResponse " << htResp->Data().size() << "bytes parsed at " << duration.length().total_milliseconds() << " milliseconds");
            LOG4CXX_DEBUG(logger, "HttpInventory::ProcessResponse: search for links");
            lst = parser.FindTags("a");
            if (lst.size() == 0) {
                LOG4CXX_TRACE(logger, "HttpInventory::ProcessResponse: no <a ...> tags");
            }
            else {
                iEntity* ent = NULL;
                EntityList::iterator iEnt;
                string href;
                bool to_process;
                for (iEnt = lst.begin(); iEnt != lst.end(); iEnt++) {
                    to_process = true;
                    href = (*iEnt)->Attr("href");
                    if (href != "") {
                        URL link;
                        link.Restore(href, &(htResp->RealUrl()));
                        LOG4CXX_TRACE(logger, "HttpInventory::ProcessResponse: <a href...> tag. url=" << link.ToString());
                        if (!link.IsHostEquals(htResp->RealUrl()))
                        {
                            if (task->IsSet("httpInventory/"weoStayInHost))
                            {
                                to_process = false;
                            }
                            LOG4CXX_TRACE(logger, "HttpInventory::ProcessResponse: weoStayInHost check " << to_process << " (" << link.ToString() << ")");
                        }
                        if (!link.IsDomainEquals(htResp->RealUrl()))
                        {
                            if (task->IsSet("httpInventory/"weoStayInDomain))
                            {
                                to_process = false;
                            }
                            LOG4CXX_TRACE(logger, "HttpInventory::ProcessResponse: weoStayInDomain check << " << to_process << " (" << link.ToString() << ")");
                        }

                        if (to_process)
                        {
                            string u_req = link.ToString();
                            if (task->IsSet("httpInventory/"weoIgnoreUrlParam)) {
                                u_req = link.ToStringNoParam();
                            }
                            LOG4CXX_TRACE(logger, "HttpInventory::ProcessResponse: weoIgnoreUrlParam check << " << u_req);
                            if (tasklist.find(u_req) == tasklist.end())
                            {
                                LOG4CXX_DEBUG(logger, "HttpInventory::ProcessResponse: add link to task list. url=" << link.ToString());
                                HttpRequest* new_url = new HttpRequest(link.ToString());
                                new_url->ID(scData->dataID);
                                tasklist[u_req] = true;
                                new_url->processor = HttpInventory::ResponseDispatcher;
                                new_url->context = (void*)this;
                                task->GetRequestAsync(new_url);
                            }
                            else
                            {
                                // add parent to existing scan data
                            }
                        }
                    } // end href attribute
                } // end <a ...> loop
            } // end <a ...> tags processing
            // todo: process other kind of links
            // CSS
            // scripts
            // objects
            // images
            // frames
            // etc ???
        }
        else {
            LOG4CXX_WARN(logger, "HttpInventory::ProcessResponse: inconsistent content-type: " << cType);
        }
    }
    task->SetScanData(scData);
}

} // namespace webEngine
