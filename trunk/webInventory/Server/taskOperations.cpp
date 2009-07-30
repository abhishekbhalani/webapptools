/*
    inventoryScanner is the web-application audit program
    Copyright (C) 2009 Andrew "Stinger" Abramov stinger911@gmail.com

    This file is part of inventoryScanner

    inventoryScanner is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    inventoryScanneris distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with inventoryScanner.  If not, see <http://www.gnu.org/licenses/>.
*//************************************************************
 * @file   taskOperations.cpp
 *
 * @brief  Operations to process task-related queries
 *
 * This is the sample implementation and the subject to change
 * @author	Aabramov
 * @date	03.07.2009
 **************************************************************/

#include "messages.h"
#include <fstream>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/operations.hpp>
#include <process.h>
#include <weiBase.h>
#include <weLogger.h>
#include <weDispatch.h>
#include <weTask.h>
#include <weTagScanner.h>
#ifdef WIN32
#define NOGDI
#include <windows.h>
#endif

namespace fs = boost::filesystem;

extern WeDispatch* globalDispatcher;
extern string cfgFile;

string taskDbDir = ".";

char* trace_modes[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
int trace_mode_last = 5;

void load_task_list( vector<WeTask*>& task_list )
{
    string report = "";
    int tasks = 0;

    tasks = globalDispatcher->Storage()->TaskReport("<report><task value='*' /></report>", report);
    if (tasks > 0)
    {   // parse the response
        bool in_parsing = true;
        int parsing_level = 0;
        int xml_pos;
        WeStrStream st(report.c_str());
        WeTagScanner sc(st);
        WeTask* tsk;
        string tag;

        while(in_parsing) {
            xml_pos = sc.GetPos();
            int t = sc.GetToken();
            switch(t)
            {
            case wstError:
                LOG4CXX_WARN(WeLogger::GetLogger(), "load_task_list - parsing error");
                in_parsing = false;
                break;
            case wstEof:
                LOG4CXX_TRACE(WeLogger::GetLogger(), "load_task_list - parsing EOF");
                in_parsing = false;
                break;
            case wstTagStart:
                tag = sc.GetTagName();
                if (parsing_level == 0)
                {
                    if (iequals(tag, "report")) {
                        task_list.clear();
                        parsing_level++;
                        break;
                    }
                }
                if (parsing_level == 1) {
                    if (iequals(tag, "task")) {
                        tsk = new WeTask();
                        // go back to the start of the TAG
                        tsk->FromXml(sc, t);
                        task_list.push_back(tsk);
                        break;
                    }
                }
                LOG4CXX_WARN(WeLogger::GetLogger(), "load_task_list - unexpected tag: " << tag);
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
}

void save_task( WeTask* tsk )
{
    string taskRep;

    taskRep = tsk->ToXml();
    globalDispatcher->Storage()->TaskSave(taskRep);
    globalDispatcher->Storage()->Flush();
}

WeTask* load_task( const string& id )
{
    string req;
    string report = "";
    int tasks = 0;
    WeTask* tsk;

    req = "<report><task value='" + id + "' /></report>";
    tasks = globalDispatcher->Storage()->TaskReport(req, report);
    tsk = NULL;
    if (tasks > 0)
    {   // parse the response
        bool in_parsing = true;
        int parsing_level = 0;
        int xml_pos;
        WeStrStream st(report.c_str());
        WeTagScanner sc(st);
        string tag;

        while(in_parsing) {
            xml_pos = sc.GetPos();
            int t = sc.GetToken();
            switch(t)
            {
            case wstError:
                LOG4CXX_WARN(WeLogger::GetLogger(), "load_task - parsing error");
                in_parsing = false;
                break;
            case wstEof:
                LOG4CXX_TRACE(WeLogger::GetLogger(), "load_task - parsing EOF");
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
                        tsk = new WeTask();
                        // go back to the start of the TAG
                        tsk->FromXml(sc, t);
                        // stop parsing - only first task need
                        in_parsing = false; 
                        break;
                    }
                }
                LOG4CXX_WARN(WeLogger::GetLogger(), "load_task - unexpected tag: " << tag);
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
    return tsk;
}

TaskList* get_task_list(const string& criteria/* = ""*/)
{
    vector<WeTask*>  task_list;
    TaskList    *lst = NULL;
    TaskRecord  *tsk = NULL;
    WeOption    opt;
    string sdata;
    int idata;
    size_t t;

    load_task_list(task_list);
    lst = new TaskList;
    for(t = 0; t < task_list.size(); t++)
    {
        tsk = new TaskRecord;

        opt = task_list[t]->Option(weoName);
        SAFE_GET_OPTION_VAL(opt, sdata, "");
        tsk->name = sdata;

        opt = task_list[t]->Option(weoID);
        SAFE_GET_OPTION_VAL(opt, sdata, "");
        tsk->id = sdata;

        opt = task_list[t]->Option(weoTaskCompletion);
        SAFE_GET_OPTION_VAL(opt, idata, 0);
        tsk->completion = idata;

        opt = task_list[t]->Option(weoTaskStatus);
        SAFE_GET_OPTION_VAL(opt, idata, 0);
        tsk->status = idata;

        lst->push_back(*tsk);
    }
    task_list.clear();
    return lst;
}

string add_task(const string& name)
{
    string retval;
    string fName;
    TaskList    *lst = NULL;
    WeTask *tsk = NULL;

    tsk = new WeTask;
    retval = globalDispatcher->Storage()->GenerateID("task");
    tsk->Option(weoID, retval);
    tsk->Option(weoName, name);
    tsk->Option(weoTaskCompletion, 0);
    tsk->Option(weoTaskStatus, 0);
    save_task(tsk);
    return retval;
}

bool del_task(const string& id)
{
    bool retval;
    string fName;
    TaskList    *lst = NULL;

    retval = false;
    fName = "<delete task='" + id + "'/>";
    if (globalDispatcher->Storage()->Delete("task", fName) != 0) {
        retval = true;
    }
    return retval;
}

bool run_task(const string& id)
{
    bool retval = false;
    WeOption opt;
    string sdata;
    int idata;
    WeTask* tsk;

    tsk = load_task(id);
    if (tsk != NULL)
    {
        opt = tsk->Option(weoTaskStatus);
        SAFE_GET_OPTION_VAL(opt, idata, 0);
        if (idata == WI_TSK_IDLE || idata == WI_TSK_PAUSED)
        {
            // prepare task to run
            // tsk->Option(weoTaskStatus, WI_TSK_RUN);
            // save_task(tsk);
            // make base filename
            string fname = id + "_" + posix_time::to_iso_string(posix_time::second_clock::local_time());

            opt = tsk->Option(weoLogLevel);
            SAFE_GET_OPTION_VAL(opt, idata, 0);
            if (idata < 0)
            {
                idata = 0;
            }
            if (idata > trace_mode_last)
            {
                idata = trace_mode_last;
            }
            { // create trace.config
                string trace = fname + ".trace";
                ofstream tr(trace.c_str());
                tr << "log4j.appender.R=org.apache.RollingFileAppender" << endl;
                tr << "log4j.appender.R=org.apache.RollingFileAppender" << endl;
                tr << "log4j.appender.R.File=" << fname << ".log" << endl;
                tr << "log4j.appender.R.MaxFileSize=10MB" << endl;
                tr << "log4j.appender.R.MaxBackupIndex=100" << endl;
                tr << "log4j.appender.R.immediateflush=true" << endl;
                tr << "log4j.appender.R.layout=org.apache.log4j.PatternLayout" << endl;
                tr << "log4j.appender.R.layout.ConversionPattern=[%d][%5p][%t] - %m%n" << endl;
                tr << "log4j.logger.webEngine=" << trace_modes[idata] << ", R" << endl;
            }
            // compose command line arguments
            string cmdline = "tskScanner --id " + id;
            cmdline += " --trace " + fname + ".trace";
            if (cfgFile != "") {
                cmdline += " --config " + cfgFile;
            }
            cmdline += " --storage " + globalDispatcher->Storage()->GetID();
            LOG4CXX_DEBUG(WeLogger::GetLogger(), "run_task: " << cmdline);
            int err = spawnl(_P_NOWAIT, "tskScanner", cmdline.c_str(), NULL);
            if (err != -1) {
                retval = true;
            }
            else {
                LOG4CXX_ERROR(WeLogger::GetLogger(), "run_task failed: (" << err << ") ERRNO=" << errno);
                fs::remove(fname + ".trace");
            }
        }
    }
    else {
        LOG4CXX_WARN(WeLogger::GetLogger(), "run_task - task not found: " << id);
    }

    return retval;
}

bool pause_task(const string& id)
{
    bool retval = false;
    WeOption opt;
    string sdata;
    int idata;
    WeTask* tsk;

    tsk = load_task(id);
    if (tsk != NULL)
    {
        opt = tsk->Option(weoTaskStatus);
        SAFE_GET_OPTION_VAL(opt, idata, 0);
        if (idata == WI_TSK_RUN)
        {
            tsk->Option(weoTaskStatus, WI_TSK_PAUSED);
            retval = true;
            // @todo Pause process which executes this task
            save_task(tsk);
        }
    }
    else {
        LOG4CXX_WARN(WeLogger::GetLogger(), "pause_task - task not found: " << id);
    }
    return retval;
}

bool cancel_task(const string& id)
{
    bool retval = false;
    WeOption opt;
    string sdata;
    int idata;
    WeTask* tsk;

    tsk = load_task(id);
    if (tsk != NULL)
    {
        opt = tsk->Option(weoTaskStatus);
        SAFE_GET_OPTION_VAL(opt, idata, 0);
        if (idata == WI_TSK_RUN)
        {
            tsk->Option(weoTaskStatus, WI_TSK_IDLE);
            retval = true;
            // @todo Cancel process which executes this task
            save_task(tsk);
        }
    }
    else {
        LOG4CXX_WARN(WeLogger::GetLogger(), "cancel_task - task not found: " << id);
    }
    return retval;
}

string get_task_opts (const string& id)
{
    string retval = "";
    WeOption opt;
    string sdata;
    WeTask* tsk;

    tsk = load_task(id);
    if (tsk != NULL)
    {
        // need to call exactly iweOptionsProvider::ToXml()
        // 'cause it gets all options in the one pack in difference
        // of structured output of WeTask::ToXml()
        retval = ((iweOptionsProvider*)tsk)->ToXml();
    }
    else {
        LOG4CXX_WARN(WeLogger::GetLogger(), "get_task_opts - task not found: " << id);
    }
    return retval;
}

bool set_task_opts (const string& dat)
{
    bool retval = false;
    string id;
    string xml;
    WeOption opt;
    string sdata;
    size_t t;
    WeTask* tsk;

    t = dat.find(';');
    if (t != string::npos) {
        id = dat.substr(0, t);
        xml = dat.substr(t+1);
        tsk = load_task(id);
        if (tsk != NULL)
        {
            // need to call exactly iweOptionsProvider::FromXml()
            // 'cause it gets all options in the one pack in difference
            // of structured processing of WeTask::FromXml()
            ((iweOptionsProvider*)tsk)->FromXml(xml);
            save_task(tsk);
            retval = true;
        }
        else {
            LOG4CXX_WARN(WeLogger::GetLogger(), "get_task_opts - task not found: " << id);
        }
    }
    return retval;
}
