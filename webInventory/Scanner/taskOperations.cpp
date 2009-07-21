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
#include <weTagScanner.h>
#ifdef WIN32
#define NOGDI
#include <windows.h>
#endif

extern WeDispatch* globalDispatcher;
string taskDbDir = ".";

void SaveTaskList(TaskList* lst)
{
    string taskRep;
    TaskList::iterator tsk;

    for (tsk = lst->begin(); tsk != lst->end(); tsk++)
    {
        taskRep = "";
        taskRep += "<task id='" + tsk->id + "'>\n";
        taskRep += " <name>" + tsk->name + "</name>\n";
        taskRep += " <status>" + boost::lexical_cast<std::string>(tsk->status) + "</status>\n";
        taskRep += " <completion>" + boost::lexical_cast<std::string>(tsk->completion) + "</completion>\n";
        taskRep += "</task>";
        globalDispatcher->Storage()->TaskSave(taskRep);
    }
}

TaskList* GetTaskList(const string& criteria/* = ""*/)
{
    TaskList    *lst = NULL;
    TaskRecord  *tsk = NULL;
    string report = "";
    int tasks = 0;

    /// @todo process filters
    tasks = globalDispatcher->Storage()->TaskReport("<taskreport><id value='*' /></taskreport>", report);
    if (tasks > 0)
    {   // parse the response
        bool inTask = false;
        bool inReport = false;
        bool inParsing = true;
        bool inName = false;
        bool inStatus = false;
        bool inCompletion = false;
        WeStrStream st(report.c_str());
        WeTagScanner sc(st);
        string id;
        string tag;

        while(inParsing) {
            int t = sc.GetToken();
            switch(t)
            {
            case wstError:
                LOG4CXX_WARN(WeLogger::GetLogger(), "GetTaskList - parsing error");
                inParsing = false;
                break;
            case wstEof:
                LOG4CXX_TRACE(WeLogger::GetLogger(), "GetTaskList - parsing EOF");
                inParsing = false;
                break;
            case wstTagStart:
                tag = sc.GetTagName();
                LOG4CXX_TRACE(WeLogger::GetLogger(), "GetTaskList - tagStart: " << tag);
                if (!inReport)
                {
                    if (iequals(tag, "taskreport")) {
                        inReport = true;
                        lst = new TaskList;
                    }
                    break;
                }
                if (!inTask && inReport) {
                    if (iequals(tag, "task")) {
                        inTask = true;
                        tsk = new TaskRecord();
                    }
                    else {
                        LOG4CXX_WARN(WeLogger::GetLogger(), "GetTaskList - not a task");
                        inParsing = false;
                    }
                    break;
                }
                if (inTask && inReport)
                {
                    if (iequals(tag, "name")) {
                        inName = true;
                        id = "";
                    }
                    if (iequals(tag, "status")) {
                        inStatus = true;
                        id = "";
                    }
                    if (iequals(tag, "completion")) {
                        inCompletion = true;
                        id = "";
                    }
                    break;
                }
                LOG4CXX_WARN(WeLogger::GetLogger(), "GetTaskList - unexpected tag: " << tag);
                inParsing = false;
                break;
            case wstTagEnd:
                tag = sc.GetTagName();
                LOG4CXX_TRACE(WeLogger::GetLogger(), "GetTaskList - tagEnd: " << tag);
                if (inTask) {
                    if (iequals(tag, "task")) {
                        inTask = false;
                        lst->push_back(*tsk);
                    }
                    if (iequals(tag, "name")) {
                        inName = false;
                        tsk->name = id;
                    }
                    if (iequals(tag, "status")) {
                        inStatus = false;
                        tsk->status = boost::lexical_cast<int>(id);
                    }
                    if (iequals(tag, "completion")) {
                        inCompletion = false;
                        tsk->completion = boost::lexical_cast<int>(id);
                    }
                }
                if (inReport) {
                    if (iequals(tag, "taskreport")) {
                        inReport = false;
                        inParsing = false; // not need to parse rest of data
                    }
                }
                break;
            case wstAttr:
                if (inTask) {
                    tag = sc.GetAttrName();
                    if (iequals(tag, "id")) {
                        tsk->id = sc.GetValue();
                    }
                }
                break;
            case wstWord: 
            case wstSpace:
                id += sc.GetValue();
                break;
            default:
                //LOG4CXX_WARN("iweStorage::TaskSave - unexpected token: " << t);
                break;
            };
        };
    }
//     try
//     {
//         string fName = taskDbDir + "\\taskList.config";
//         std::ifstream itfs(fName.c_str());
//         {
//             boost::archive::xml_iarchive ia(itfs);
//             lst = new TaskList;
//             ia >> BOOST_SERIALIZATION_NVP(*lst);
//         }
//     }
//     catch (std::exception& e)
//     {
//         LOG4CXX_ERROR(WeLogger::GetLogger(), "Can't get task list: " << e.what());
//         if (lst != NULL)
//         {
//             delete lst;
//         }
//         lst = NULL;
//     }
    return lst;
}

string AddTask(const string& name)
{
    string retval;
    string fName;
    TaskList    *lst = NULL;
    TaskRecord  *tsk = NULL;

    lst = GetTaskList("");
    if (lst == NULL)
    {
        lst = new TaskList;
    }
    tsk = new TaskRecord;
    tsk->name = name;
    tsk->id = globalDispatcher->Storage()->GenerateID("task");
    tsk->completion = 0;
    tsk->status = 0;
    lst->push_back(*tsk);
    SaveTaskList(lst);
    return tsk->id;
}

bool DelTask(const string& id)
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
