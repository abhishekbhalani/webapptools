/*
    tskScanner is the web-application audit program
    Copyright (C) 2009 Andrew "Stinger" Abramov stinger911@gmail.com

    This file is part of tskScanner

    tskScanner is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    tskScanner is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with tskScanner.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "watchdog.h"

typedef map<string, WeHttpRequest*> WeHttpMap;

void process_response(iweResponse *resp, WeHttpMap& tasklist)
{
    WeHttpResponse* htResp;

    try {
        htResp = reinterpret_cast<WeHttpResponse*>(resp);
    }
    catch (...) {
        LOG4CXX_ERROR(WeLogger::GetLogger(), "The response from " << resp->BaseUrl().ToString() << " isn't the HttpResponse!");
        return;
    }
    // process response
    LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: process response with code=" << htResp->HttpCode());
    WeScanData* scdata = new WeScanData;
    scdata->dataID = globalData.dispatcher->Storage()->GenerateID("scan_data");
    scdata->scanID = globalData.scan_info->scanID;
    scdata->parentID = htResp->ID();
    scdata->requestedURL = htResp->BaseUrl().ToString();
    scdata->realURL = htResp->RealUrl().ToString();
    scdata->respCode = htResp->HttpCode();
    scdata->downloadTime = htResp->DownloadTime();
    scdata->dataSize = htResp->Data().size();
    { // interlocked assign
        boost::lock_guard<boost::mutex> lock(globalData.locker);
        LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: add new scan_data to scan information");
        globalData.scan_info->push_back(scdata);
    }

    /// @todo process options
    if (htResp->HttpCode() >= 300 && htResp->HttpCode() < 400) {
        // redirections
        LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: process redirect");
        string url = htResp->Headers().FindFirst("Location");
        if (!url.empty()) {
            LOG4CXX_DEBUG(WeLogger::GetLogger(), "task_executor: redirected to " << url);
            bool to_process = false;
            WeURL baseUrl = htResp->BaseUrl();
            baseUrl.Restore(url);
            // !!! todo LOG4CXX_TRACE
            LOG4CXX_DEBUG(WeLogger::GetLogger(), "task_executor: reconstructed url is " << baseUrl.ToString());
            if (globalData.task_info->IsSet(weoStayInHost))
            {
                if (baseUrl.IsHostEquals(htResp->RealUrl()))
                {
                    to_process = true;
                }
                LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: weoStayInHost check " << to_process);
            }
            if (globalData.task_info->IsSet(weoStayInDomain))
            {
                if (baseUrl.IsDomainEquals(htResp->RealUrl()))
                {
                    to_process = true;
                }
                LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: weoStayInDomain check << " << to_process);
            }

            if (to_process)
            {
                string u_req = baseUrl.ToString();
                if (globalData.task_info->IsSet(weoIgnoreUrlParam)) {
                    u_req = baseUrl.ToStringNoParam();
                }
                if (tasklist.find(u_req) == tasklist.end())
                {
                    WeHttpRequest* new_url = new WeHttpRequest(baseUrl.ToString());
                    new_url->ID(scdata->dataID);
                    tasklist[u_req] = new_url;
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
        LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: parse document");
        WeHtmlDocument parser;
        WeEntityList lst;

        parser.ParseData(resp);
        LOG4CXX_DEBUG(WeLogger::GetLogger(), "task_executor: search for links");
        lst = parser.FindTags("a");
        if (lst.size() == 0) {
            LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: no <a ...> tags");
        }
        else {
            iweEntity* ent = NULL;
            WeEntityList::iterator iEnt;
            string href;
            bool to_process;
            for (iEnt = lst.begin(); iEnt != lst.end(); iEnt++) {
                to_process = false;
                href = (*iEnt)->Attr("href");
                if (href != "") {
                    WeURL link;
                    link.Restore(href, &(htResp->RealUrl()));
                    LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: <a href...> tag. url=" << link.ToString());
                    if (globalData.task_info->IsSet(weoStayInHost))
                    {
                        if (link.IsHostEquals(htResp->RealUrl()))
                        {
                            to_process = true;
                        }
                        LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: weoStayInHost check " << to_process);
                    }
                    if (globalData.task_info->IsSet(weoStayInDomain))
                    {
                        if (link.IsDomainEquals(htResp->RealUrl()))
                        {
                            to_process = true;
                        }
                        LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: weoStayInDomain check << " << to_process);
                    }

                    if (to_process)
                    {
                        string u_req = link.ToString();
                        if (globalData.task_info->IsSet(weoIgnoreUrlParam)) {
                            u_req = link.ToStringNoParam();
                        }
                        if (tasklist.find(u_req) == tasklist.end())
                        {
                            LOG4CXX_DEBUG(WeLogger::GetLogger(), "task_executor: add link to task list. url=" << link.ToString());
                            WeHttpRequest* new_url = new WeHttpRequest(link.ToString());
                            new_url->ID(scdata->dataID);
                            tasklist[u_req] = new_url;
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
}

void task_executor(const string& taskID)
{
    string sdata;
    int idata, count;
    WeOption opt;
    WeHttpMap  task_list;
    WeHttpMap::iterator tsk_it;
    WeHttpMap::iterator tsk_go;
    size_t          task_list_max_size;
    WeHttpRequest*  curr_url;
    int     max_requests;
    WeResponseList resps;
    WeResponseList::iterator rIt;
    vector<iweTransport*>   transports;

    bool in_process = false;
    LOG4CXX_DEBUG(WeLogger::GetLogger(), "Start task execution");
    {
        boost::lock_guard<boost::mutex> lock(globalData.locker);
        LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor start: reload task");
        globalData.load_task(taskID);

        // get transport from task options
        transports.clear();
        for (int i = 0; i < globalData.plugins.size(); i++)
        {
            if (find(globalData.plugins[i].InterfaceList().begin(),
                        globalData.plugins[i].InterfaceList().end(),
                        "iweTransport") != globalData.plugins[i].InterfaceList().end())
            {
                transports.push_back(globalData.plugins[i]);
                LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor found transport: ", globalData.plugins[i].GetDesc());
            }
        }
        LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor load" << transports.size() << " transports");

        opt = globalData.task_info->Option(weoBaseURL);
        SAFE_GET_OPTION_VAL(opt, sdata, "");
        if (sdata == "")
        {
            LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: empty URL - stop the task");
            globalData.scan_info->finishTime = posix_time::second_clock::local_time();
            globalData.scan_info->status = WeScan::weScanFinished;
            globalData.task_info->Option(weoTaskStatus, WI_TSK_IDLE);
            globalData.execution = false;
            return;
        }
        opt = globalData.task_info->Option(weoParallelReq);
        SAFE_GET_OPTION_VAL(opt, max_requests, 1);
        LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: enable parallel requests up to " << max_requests);

        curr_url = new WeHttpRequest(sdata);
        curr_url->ID("0");
        task_list[curr_url->RequestUrl().ToString()] = curr_url;

        LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor start: set task state WI_TSK_RUN");
        globalData.task_info->Option(weoTaskStatus, WI_TSK_RUN);
        globalData.save_task();
    }
    LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: go to main loop");
    task_list_max_size = task_list.size();

    // stage 1: inventory
    for (int i = 0; i < globalData.plugins.size(); i++)
    {
        if (find(globalData.plugins[i].InterfaceList().begin(),
            globalData.plugins[i].InterfaceList().end(),
            "iweInventory") != globalData.plugins[i].InterfaceList().end())
        {
            ((iweInventory*)globalData.plugins[i])->;
            LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor found transport: ", globalData.plugins[i].GetDesc());
        }
    }

    do 
    {
        boost::this_thread::sleep(boost::posix_time::seconds(2));
        if (!globalData.check_state()) {
            in_process = false;
            break;
        }
        tsk_go = task_list.end();
        { // save the task execution status
            boost::lock_guard<boost::mutex> lock(globalData.locker);
            in_process = globalData.execution;
            if (in_process)
            {
                count = 0;
                for (tsk_it = task_list.begin(); tsk_it != task_list.end(); tsk_it++)
                {
                    if (tsk_it->second != NULL)
                    {
                        count++;
                    }
                }

                task_list_max_size = task_list.size();

                idata = (task_list_max_size - count) * 100 / task_list_max_size;
                LOG4CXX_DEBUG(WeLogger::GetLogger(), "task_executor loop: rest " << count << " queries  from " <<
                            task_list_max_size << " (" << idata << "%)");
                LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor loop: reload task");
                globalData.load_task(taskID);
                if (globalData.task_info != NULL)
                {
                    globalData.task_info->Option(weoTaskCompletion, idata);
                    globalData.save_task();
                }
            }
        }
        // perform task operation

        // send request if slots available
        while (resps.size() < max_requests)
        {
            for (tsk_go = task_list.begin(); tsk_go != task_list.end(); tsk_go++)
            {
                if (tsk_go->second != NULL) {
                    break;
                }
            }
            if (tsk_go != task_list.end())
            {
                curr_url = tsk_go->second;
                LOG4CXX_DEBUG(WeLogger::GetLogger(), "task_executor: send request to " << curr_url->RequestUrl().ToString());
                iweResponse* resp = transport->Request(curr_url);
                resp->ID(curr_url->ID());
                resps.push_back(resp);

                string u_req = curr_url->RequestUrl().ToString();
                if (globalData.task_info->IsSet("noParamUrl")) {
                    u_req = curr_url->RequestUrl().ToStringNoParam();
                }
                task_list[u_req] = NULL;
                delete curr_url;
            }
            else {
                break; // no URLs in the waiting list
            }
        }

        // if any requests pending process transport operations
        if (resps.size() > 0) {
            LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor loop: transport->ProcessRequests()");
            transport->ProcessRequests();
            boost::this_thread::sleep(boost::posix_time::millisec(500));
            for(rIt = resps.begin(); rIt != resps.end();) {
                if ((*rIt)->Processed()) {
                    process_response((*rIt), task_list);
                    resps.erase(rIt);
                    rIt = resps.begin();
                }
                else {
                    rIt++;
                }
            }
        }
        
        // check for requests in waiting queue
        count = 0;
        for (tsk_it = task_list.begin(); tsk_it != task_list.end(); tsk_it++)
        {
            if (tsk_it->second != NULL)
            {
                count++;
            }
        }

        // if no requests in waiting and transport queues - exit
        if (count == 0 && resps.size() == 0)
        {
            boost::lock_guard<boost::mutex> lock(globalData.locker);
            LOG4CXX_DEBUG(WeLogger::GetLogger(), "task_executor: no requests in queue");
            globalData.execution = false;
            globalData.scan_info->finishTime = posix_time::second_clock::local_time();
            globalData.scan_info->status = WeScan::weScanFinished;
            globalData.task_info->Option(weoTaskStatus, WI_TSK_IDLE);
            globalData.task_info->Option(weoTaskCompletion, 0);
            in_process = false;
        }
        LOG4CXX_DEBUG(WeLogger::GetLogger(), "task_executor loop: DEBUG --- Next task operation");
    } while(in_process);
}
