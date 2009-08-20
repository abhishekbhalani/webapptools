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

locked_data  globalData;

locked_data::locked_data()
{
    dispatcher = NULL;
    task_info = NULL;
    scan_info = NULL;
    execution = true;
    pause = false;
}

locked_data::~locked_data()
{
    // destroy();
}

void locked_data::destroy()
{
    {
        //boost::lock_guard<boost::mutex> lock(locker);
        execution = false;
    }
    //boost::lock_guard<boost::mutex> lock(locker);
    cond.notify_all();
    if (task_info != NULL)
    {
        task_info->Option(weoTaskStatus, WI_TSK_IDLE);
        save_task();
        delete task_info;
        task_info = NULL;
    }
    if (dispatcher != NULL)
    {
        delete dispatcher;
        dispatcher = NULL;
    }
}

Task* locked_data::load_task(const string& id)
{
    string req;
    string report = "";
    int tasks = 0;

    //boost::lock_guard<boost::mutex> lock(locker);

    req = "<report><task value='" + id + "' /></report>";
    tasks = globalData.dispatcher->Storage()->TaskReport(req, report);
    if (globalData.task_info == NULL) {
        globalData.task_info = new Task;
    }
    if (tasks > 0)
    {   // parse the response
        bool in_parsing = true;
        int parsing_level = 0;
        int xml_pos;
        StrStream st(report.c_str());
        TagScanner sc(st);
        string tag;

        while(in_parsing) {
            xml_pos = sc.GetPos();
            int t = sc.GetToken();
            switch(t)
            {
            case wstError:
                LOG4CXX_WARN(iLogger::GetLogger(), "load_task - parsing error");
                in_parsing = false;
                break;
            case wstEof:
                LOG4CXX_TRACE(iLogger::GetLogger(), "load_task - parsing EOF");
                in_parsing = false;
                break;
            case wstTagStart:
                tag = sc.GetTagName();
                if (parsing_level == 0)
                {
                    if (iequals(tag, "report")) {
                        parsing_level++;
                        break;
                    }
                }
                if (parsing_level == 1) {
                    if (iequals(tag, "task")) {
                        // go back to the start of the TAG
                        globalData.task_info->FromXml(sc, t);
                        // stop parsing - only first task need
                        in_parsing = false;
                        break;
                    }
                }
                LOG4CXX_WARN(iLogger::GetLogger(), "load_task - unexpected tag: " << tag);
                in_parsing = false;
                break;
            case wstTagEnd:
                tag = sc.GetTagName();
                if (parsing_level == 0)
                {
                    if (iequals(tag, "report")) {
                        in_parsing = false;
                    }
                }
                break;
            }
        }
    }

    return globalData.task_info;
}

void locked_data::save_task( void )
{
    //boost::lock_guard<boost::mutex> lock(locker);

    if (task_info != NULL)
    {
        string taskRep;

        taskRep = task_info->ToXml();
        dispatcher->Storage()->TaskSave(taskRep);
        dispatcher->Storage()->Flush();
    }
}

bool locked_data::check_state()
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "Watchdog check_state.");
    if (execution)
    {
        boost::unique_lock<boost::mutex> lock(locker);
        while(pause) {
            cond.wait(lock);
        }
    }
    if (task_info)
    {
        return task_info->IsReady();
    }
    return false;
}

void watch_dog_thread(const string& id)
{
    bool in_process = true;

    while(in_process) {
        boost::this_thread::sleep(boost::posix_time::seconds(5));
        LOG4CXX_TRACE(iLogger::GetLogger(), "Watchdog timer. Refresh task state.");
        {
            boost::lock_guard<boost::mutex> lock(globalData.locker);
            globalData.load_task(id);
            if (globalData.task_info != NULL)
            {
                wOption opt = globalData.task_info->Option(weoTaskSignal);
                int idata;
                SAFE_GET_OPTION_VAL(opt, idata, -1);
                if (idata == WE_TASK_SIGNAL_STOP) {
                    LOG4CXX_DEBUG(iLogger::GetLogger(), "Watchdog timer. Stop the task.");
                    globalData.execution = false;
                    globalData.pause = false;
                    in_process = false;
                    globalData.task_info->Option(weoTaskCompletion, 0);
                    globalData.task_info->Stop();
                    if (globalData.scan_info->status != ScanInfo::weScanFinished)
                    {
                        globalData.scan_info->status = ScanInfo::weScanStopped;
                    }
                }
                if (idata == WE_TASK_SIGNAL_PAUSE) {
                    LOG4CXX_DEBUG(iLogger::GetLogger(), "Watchdog timer. Pause the task.");
                    globalData.execution = true;
                    globalData.pause = true;
                    globalData.task_info->Pause();
                    if (globalData.scan_info->status != ScanInfo::weScanFinished && globalData.scan_info->status != ScanInfo::weScanStopped)
                    {
                        globalData.scan_info->status = ScanInfo::weScanPaused;
                    }
                }
                if (idata == WE_TASK_SIGNAL_RUN) {
                    LOG4CXX_DEBUG(iLogger::GetLogger(), "Watchdog timer. Continue task execution.");
                    globalData.execution = true;
                    globalData.pause = false;
                    globalData.task_info->Pause(false);
                    if (globalData.scan_info->status != ScanInfo::weScanFinished && globalData.scan_info->status != ScanInfo::weScanStopped)
                    {
                        globalData.scan_info->status = ScanInfo::weScanRunning;
                    }
                    globalData.cond.notify_all();
                }
            }
            else {
                LOG4CXX_ERROR(iLogger::GetLogger(), "Watchdog timer. Task not loaded!");
                in_process = false;
                globalData.execution = false;
                globalData.pause = false;
                globalData.scan_info->status = ScanInfo::weScanError;
            }
            // Save scan information (watchdog ping)
            LOG4CXX_DEBUG(iLogger::GetLogger(), "Watchdog timer. Update scan information.");
            globalData.scan_info->pingTime = posix_time::second_clock::local_time();
            string report = globalData.scan_info->ToXml();
            globalData.dispatcher->Storage()->ScanSave(report);
        }
    }
}
