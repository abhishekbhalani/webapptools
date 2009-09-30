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
#include <weiInventory.h>

static bool task_control(const string& taskID)
{
    if (!globalData.check_state()) {
        return false;
    }
    { // save the task execution status
        boost::lock_guard<boost::mutex> lock(globalData.locker);
        globalData.task_info->CalcStatus();
        // remove weoTaskSignal option to avoid race condition
        globalData.task_info->Erase(weoTaskSignal);
        globalData.save_task();
    }
    return globalData.execution;
}

void task_executor(const string& taskID)
{
    string sdata;
    wOption opt;
//    WeHttpMap  task_list;
//    WeHttpRequest*  curr_url;
    vector<iTransport*>   transports;

    bool in_process = false;
    LOG4CXX_DEBUG(iLogger::GetLogger(), "Start task execution");
    {
        boost::lock_guard<boost::mutex> lock(globalData.locker);
        LOG4CXX_TRACE(iLogger::GetLogger(), "task_executor start: reload task");
        globalData.task_info->Option(weoTaskStatus, WI_TSK_RUN);
        int pid;
#ifdef WIN32
        pid = (int)GetCurrentProcessId();
#else
        pid_t p = getpid();
        pid = (int)p;
#endif //WIN32

        LOG4CXX_INFO(iLogger::GetLogger(), "task_executor: Process id=" << pid);
        globalData.task_info->Option("ProcessId", pid);
        globalData.save_task();

        // get transport from task options
        globalData.task_info->StorePlugins(globalData.plugins);
        LOG4CXX_TRACE(iLogger::GetLogger(), "task_executor plugins loaded");

    }

    LOG4CXX_TRACE(iLogger::GetLogger(), "task_executor: go to main loop");

    // stage 0: prepare to start
    LOG4CXX_TRACE(iLogger::GetLogger(), "task_executor: stage 0 - start inventories");
    globalData.task_info->Run();

    do
    {
        // stage 1: inventory
        LOG4CXX_TRACE(iLogger::GetLogger(), "task_executor: stage 1 - check inventory");
        if (!task_control(taskID)) {
            break;
        }

        // stage 2: audit
        LOG4CXX_TRACE(iLogger::GetLogger(), "task_executor: stage 2 - check audit");
        boost::this_thread::sleep(boost::posix_time::seconds(1));

        if (!task_control(taskID)) {
            break;
        }

        // stage 3: vulnerabilities
        LOG4CXX_TRACE(iLogger::GetLogger(), "task_executor: stage 3 - check vulnerabilities");
        boost::this_thread::sleep(boost::posix_time::seconds(1));

        if (!task_control(taskID)) {
            break;
        }
    }
    while(true);
    {
        boost::lock_guard<boost::mutex> lock(globalData.locker);
        LOG4CXX_DEBUG(iLogger::GetLogger(), "task_executor: task completed");
        globalData.execution = false;
        globalData.scan_info->finishTime = posix_time::second_clock::local_time();
        if (globalData.scan_info->status == ScanInfo::weScanRunning || globalData.scan_info->status == ScanInfo::weScanIdle)
        {   // set "finished" status only if scan have non-forced finish
            globalData.scan_info->status = ScanInfo::weScanFinished;
        }
        globalData.task_info->Option(weoTaskStatus, WI_TSK_IDLE);
        globalData.task_info->Option(weoTaskCompletion, 0);
        in_process = false;
    }
}
