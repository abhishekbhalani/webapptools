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
#include <weiBase.h>
#include <weLogger.h>
#include <weDispatch.h>
#include <weTask.h>
#include <weTagScanner.h>
#ifdef WIN32
#define NOGDI
#include <windows.h>
#endif

extern WeDispatch* globalDispatcher;
string taskDbDir = ".";

static vector<WeTask*>  task_list;

void load_task_list( void )
{
    string report = "";
    int tasks = 0;

    tasks = globalDispatcher->Storage()->TaskReport("<taskreport><id value='*' /></taskreport>", report);
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
                    if (iequals(tag, "taskreport")) {
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
                LOG4CXX_WARN(WeLogger::GetLogger(), "get_task_list - unexpected tag: " << tag);
                in_parsing = false;
                break;
            case wstTagEnd:
                tag = sc.GetTagName();
                if (parsing_level == 0)
                {
                    if (iequals(tag, "taskreport")) {
                        in_parsing = false;
                    }
                }
                break;
            }
        }
    }
}

void save_task_list( void )
{
    string taskRep;
    int t;

    for (t = 0; t < task_list.size(); t++)
    {
        taskRep = task_list[t]->ToXml();
        globalDispatcher->Storage()->TaskSave(taskRep);
    }
    globalDispatcher->Storage()->Flush();
}

TaskList* get_task_list(const string& criteria/* = ""*/)
{
    TaskList    *lst = NULL;
    TaskRecord  *tsk = NULL;
    WeOption    opt;
    string sdata;
    int idata;
    int t;

    if (task_list.empty())
    {
        load_task_list();
    }
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
    task_list.push_back(tsk);
    save_task_list();
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
    load_task_list();
    return retval;
}

bool run_task(const string& id)
{
    bool retval = false;
    WeOption opt;
    string sdata;
    int idata;
    int t;

    for (t = 0; t < task_list.size(); t++)
    {
        opt = task_list[t]->Option(weoID);
        SAFE_GET_OPTION_VAL(opt, sdata, "");

        if (sdata == id) {
            opt = task_list[t]->Option(weoTaskStatus);
            SAFE_GET_OPTION_VAL(opt, idata, 0);
            if (idata == WI_TSK_IDLE || idata == WI_TSK_PAUSED)
            {
                task_list[t]->Option(weoTaskStatus, WI_TSK_RUN);
                retval = true;
                // @todo Run process to execute task
                save_task_list();
            }
        }
    }
    return retval;
}

bool pause_task(const string& id)
{
    bool retval = false;
    WeOption opt;
    string sdata;
    int idata;
    int t;

    for (t = 0; t < task_list.size(); t++)
    {
        opt = task_list[t]->Option(weoID);
        SAFE_GET_OPTION_VAL(opt, sdata, "");

        if (sdata == id) {
            opt = task_list[t]->Option(weoTaskStatus);
            SAFE_GET_OPTION_VAL(opt, idata, 0);
            if (idata == WI_TSK_RUN)
            {
                task_list[t]->Option(weoTaskStatus, WI_TSK_PAUSED);
                retval = true;
                // @todo Pause process which executes this task
                save_task_list();
            }
        }
    }
    return retval;
}

bool cancel_task(const string& id)
{
    bool retval = false;
    WeOption opt;
    string sdata;
    int idata;
    int t;

    for (t = 0; t < task_list.size(); t++)
    {
        opt = task_list[t]->Option(weoID);
        SAFE_GET_OPTION_VAL(opt, sdata, "");

        if (sdata == id) {
            opt = task_list[t]->Option(weoTaskStatus);
            SAFE_GET_OPTION_VAL(opt, idata, 0);
            if (idata == WI_TSK_RUN)
            {
                task_list[t]->Option(weoTaskStatus, WI_TSK_IDLE);
                retval = true;
                // @todo Cancel process which executes this task
                save_task_list();
            }
        }
    }
    return retval;
}

string get_task_opts (const string& id)
{
    string retval = "";
    WeOption opt;
    string sdata;
    int t;

    for (t = 0; t < task_list.size(); t++)
    {
        opt = task_list[t]->Option(weoID);
        SAFE_GET_OPTION_VAL(opt, sdata, "");

        if (sdata == id) {
            // need to call exactly iweOptionsProvider::ToXml()
            // 'cause it gets all options in the one pack in difference
            // of structured output of WeTask::ToXml()
            retval = ((iweOptionsProvider*)task_list[t])->ToXml();
            break;
        }
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
    int t;

    t = dat.find(';');
    if (t != string::npos) {
        id = dat.substr(0, t);
        xml = dat.substr(t+1);
        for (t = 0; t < task_list.size(); t++)
        {
            opt = task_list[t]->Option(weoID);
            SAFE_GET_OPTION_VAL(opt, sdata, "");

            if (sdata == id) {
                // need to call exactly iweOptionsProvider::FromXml()
                // 'cause it gets all options in the one pack in difference
                // of structured processing of WeTask::FromXml()
                ((iweOptionsProvider*)task_list[t])->FromXml(xml);
                retval = true;
                break;
            }
        }
    }
    return retval;
}

void fake_task_processing( void )
{
    WeOption opt;
    string sdata;
    int idata;
    int t;

    for (t = 0; t < task_list.size(); t++)
    {
        opt = task_list[t]->Option(weoTaskStatus);
        SAFE_GET_OPTION_VAL(opt, idata, 0);
        if (idata == WI_TSK_RUN)
        {
            opt = task_list[t]->Option(weoTaskCompletion);
            SAFE_GET_OPTION_VAL(opt, idata, 0);
            idata++;
            if (idata == 100)
            {
                task_list[t]->Option(weoTaskStatus, WI_TSK_IDLE);
                idata = 0;
            }
            task_list[t]->Option(weoTaskCompletion, idata);
        }
    }
    save_task_list();
}