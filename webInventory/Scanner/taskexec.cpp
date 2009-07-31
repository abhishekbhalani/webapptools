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

void task_executor(const string& taskID)
{
    string sdata;
    int idata;
    WeOption opt;
    vector<WeHttpRequest*>  task_list;
    int             task_list_max_size;
    WeHttpRequest*  curr_url;

    bool in_process = false;
    LOG4CXX_DEBUG(WeLogger::GetLogger(), "Start task execution");
    {
        boost::lock_guard<boost::mutex> lock(globalData.locker);
        LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor start: reload task");
        globalData.load_task(taskID);

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
        curr_url = new WeHttpRequest(sdata);
        curr_url->ID("0");
        task_list.push_back(curr_url);

        LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor start: set task state WI_TSK_RUN");
        globalData.task_info->Option(weoTaskStatus, WI_TSK_RUN);
        globalData.save_task();
    }
    LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: go to main loop");
    task_list_max_size = task_list.size();
    do 
    {
        boost::this_thread::sleep(boost::posix_time::seconds(2));
        if (!globalData.check_state()) {
            in_process = false;
            break;
        }
        { // save the task execution status
            boost::lock_guard<boost::mutex> lock(globalData.locker);
            in_process = globalData.execution;
            if (in_process)
            {
                LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor loop: reload task");
                globalData.load_task(taskID);
                if (globalData.task_info != NULL)
                {
                    idata = (task_list_max_size - task_list.size()) * 100 / task_list_max_size;
                    globalData.task_info->Option(weoTaskCompletion, idata);
                    globalData.save_task();
                }
            }
        }
        // perform task operation
        curr_url = task_list[0];
        WeHttpResponse resp;
        // get transport from task options
        WeHTTP* transport = (WeHTTP*)globalData.dispatcher->LoadPlugin("A44A9A1E7C25"); // load HTTP transport
        // set transport options such as authorization etc
        LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: send request to " << curr_url->RequestUrl().ToString());
        transport->Request(curr_url, &resp);
        while (!resp.Processed()) {
            transport->ProcessRequests();
        }
        // process response
        LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: process response with code=" << resp.HttpCode());
        WeScanData* scdata = new WeScanData;
        scdata->dataID = globalData.dispatcher->Storage()->GenerateID("scan_data");
        scdata->scanID = globalData.scan_info->scanID;
        scdata->parentID = curr_url->ID();
        scdata->requestedURL = curr_url->RequestUrl().ToString();
        scdata->realURL = resp.RealUrl().ToString();
        scdata->respCode = resp.HttpCode();
        scdata->downloadTime = resp.DownloadTime();
        scdata->dataSize = resp.Data().size();
        { // interlocked assign
            boost::lock_guard<boost::mutex> lock(globalData.locker);
            LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: add new scan_data to scan information");
            globalData.scan_info->push_back(scdata);
        }

        /// @todo process options
        if (scdata->respCode >= 300 && scdata->respCode < 400) {
            // redirections
            LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: process redirect");
            string url = resp.Headers().FindFirst("Location");
            if (!url.empty()) {
                LOG4CXX_DEBUG(WeLogger::GetLogger(), "task_executor: redirected to " << url);
                WeURL baseUrl = curr_url->RequestUrl();
                baseUrl.Restore(url);
                WeHttpRequest* new_url = new WeHttpRequest(baseUrl.ToString());
                new_url->ID(scdata->dataID);
                task_list.push_back(new_url);
                task_list_max_size++;
            }
        }

        delete curr_url;
        task_list.erase(task_list.begin());
        if (task_list.size() == 0)
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
