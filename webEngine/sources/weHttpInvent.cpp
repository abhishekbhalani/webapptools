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
#include "iweInventory.xrc"

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
                        LOG4CXX_TRACE(logger, "HttpInventory::ProcessResponse: weoStayInHost check " << to_process);
                    }
                    if (!link.IsDomainEquals(htResp->RealUrl()))
                    {
                        if (task->IsSet("httpInventory/"weoStayInDomain))
                        {
                            to_process = false;
                        }
                        LOG4CXX_TRACE(logger, "HttpInventory::ProcessResponse: weoStayInDomain check << " << to_process);
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
    task->SetScanData(scData);
}

} // namespace webEngine
