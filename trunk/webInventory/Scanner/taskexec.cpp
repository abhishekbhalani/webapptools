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
    bool in_process = false;
    LOG4CXX_DEBUG(WeLogger::GetLogger(), "Start task execution");
    {
        boost::lock_guard<boost::mutex> lock(globalData.locker);
        LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor start: reload task");
        globalData.load_task(taskID);
        LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor start: set task state WI_TSK_RUN");
        globalData.task_info->Option(weoTaskStatus, WI_TSK_RUN);
        globalData.save_task();
    }
    LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor: go to main loop");
    do 
    {
        boost::this_thread::sleep(boost::posix_time::seconds(2));
        if (!globalData.check_state()) {
            in_process = false;
            break;
        }
        {
            boost::lock_guard<boost::mutex> lock(globalData.locker);
            in_process = globalData.execution;
            if (in_process)
            {
                // !!! debug fake task execution
                LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor loop: reload task");
                globalData.load_task(taskID);
                if (globalData.task_info != NULL)
                {
                    WeOption opt = globalData.task_info->Option(weoTaskCompletion);
                    int idata;
                    SAFE_GET_OPTION_VAL(opt, idata, 0);
                    if (idata < 100)
                    {
                        idata++;
                    }
                    else {
                        LOG4CXX_TRACE(WeLogger::GetLogger(), "task_executor loop: task completed");
                        globalData.task_info->Option(weoTaskStatus, WI_TSK_IDLE);
                        idata = 0;
                    }
                    globalData.task_info->Option(weoTaskCompletion, idata);
                    globalData.save_task();
                }
            }
        }
        LOG4CXX_DEBUG(WeLogger::GetLogger(), "task_executor loop: DEBUG --- Next task operation");
    } while(in_process);
}
