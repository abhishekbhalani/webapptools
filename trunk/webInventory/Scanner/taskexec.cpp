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
        globalData.load_task(taskID);
        globalData.task_info->CalcStatus();
        globalData.save_task();
    }
    return globalData.execution;
}

void task_executor(const string& taskID)
{
    string sdata;
    WeOption opt;
//    WeHttpMap  task_list;
//    WeHttpRequest*  curr_url;
    vector<iweTransport*>   transports;

    bool in_process = false;
    LOG4CXX_DEBUG(WeLogger::GetLogger(), "Start task execution");
    {
        boost::lock_guard<boost::mutex> lock(globalData.locker);
        LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor start: reload task");
        globalData.task_info->Option(weoTaskStatus, WI_TSK_RUN);
        globalData.save_task();

        // get transport from task options
        for (size_t i = 0; i < globalData.plugins.size(); i++)
        {
            WeStringList ifaces = globalData.plugins[i]->InterfaceList();
            LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor plugin: " << globalData.plugins[i]->GetDesc() << " ifaces: " << ifaces.size());
            WeStringList::iterator trsp = find(ifaces.begin(), ifaces.end(), "iweTransport");
            if (trsp != ifaces.end())
            {
                LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor found transport: " << (*trsp));
                globalData.task_info->AddTransport((iweTransport*)globalData.plugins[i]);
                LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor found transport: " << globalData.plugins[i]->GetDesc());
            }
        }
        LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor transports loaded");

    }
    LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: go to main loop");

    // stage 1: inventory
    LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: stage 1 - inventory");
    for (size_t i = 0; i < globalData.plugins.size(); i++)
    {
        WeStringList ifaces = globalData.plugins[i]->InterfaceList();
        LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor plugin: " << globalData.plugins[i]->GetDesc() << " ifaces: " << ifaces.size());
        WeStringList::iterator trsp = find(ifaces.begin(), ifaces.end(), "iweInventory");
        if (trsp != ifaces.end())
        {
            LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor found inventory: " << globalData.plugins[i]->GetDesc());
            ((iweInventory*)globalData.plugins[i])->Start(globalData.task_info);
        }
    }
    task_control(taskID);

    // stage 2: audit
    LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: stage 2 - audit");
    boost::this_thread::sleep(boost::posix_time::seconds(1));

    task_control(taskID);

    // stage 3: vulnerabilities
    LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: stage 3 - vulnerabilities");
    boost::this_thread::sleep(boost::posix_time::seconds(1));

    task_control(taskID);


    // final stage - wait for data in queues
    do
    {
        boost::this_thread::sleep(boost::posix_time::seconds(1));
        if (!task_control(taskID)) {
            break;
        }
        globalData.task_info->WaitForData();
    }
    while(true);
}
