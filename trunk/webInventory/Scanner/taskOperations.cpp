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
#include <weLogger.h>
#ifdef WIN32
#define NOGDI
#include <windows.h>
#endif

string taskDbDir = ".";

TaskList* GetTaskList(const string& criteria/* = ""*/)
{
    TaskList    *lst = NULL;
    TaskRecord  *tsk = NULL;

    try
    {
        string fName = taskDbDir + "\\taskList.config";
        std::ifstream itfs(fName.c_str());
        {
            boost::archive::xml_iarchive ia(itfs);
            lst = new TaskList;
            ia >> BOOST_SERIALIZATION_NVP(*lst);
        }
    }
    catch (std::exception& e)
    {
        LOG4CXX_ERROR(WeLogger::GetLogger(), "Can't get task list: " << e.what());
        if (lst != NULL)
        {
            delete lst;
        }
        lst = NULL;
    }
    return lst;
}

string AddTask(const string& name)
{
    string retval;
    string fName;
    TaskList    *lst = NULL;
    TaskRecord  *tsk = NULL;

    try
    {
#ifdef WIN32
        char tmpName[512];
        GetTempFileNameA(taskDbDir.c_str(), "", 0, tmpName);
        DeleteFileA(tmpName);
        retval = tmpName;
        size_t pos = retval.find_last_of('\\');
        if (pos != string::npos)
        {
            retval = retval.substr(pos + 1);
        }
        pos = retval.find('.');
        if (pos != string::npos)
        {
            retval = retval.substr(0, pos);
        }
#endif
        fName = taskDbDir + "\\taskList.config";
        try
        {
            std::ifstream itfs(fName.c_str());
            boost::archive::xml_iarchive ia(itfs);
            lst = new TaskList;
            ia >> BOOST_SERIALIZATION_NVP(*lst);
        }
        catch (std::exception& e) {
            // hmmm. try to create new task list
            LOG4CXX_ERROR(WeLogger::GetLogger(), "Add task: can't read task list: " << e.what());
        }
        tsk = new TaskRecord;
        tsk->name = name;
        tsk->id = retval;
        tsk->completion = 0;
        tsk->status = 0;
        lst->task.push_back(*tsk);
        {
            std::ofstream otfs(fName.c_str());
            boost::archive::xml_oarchive oa(otfs);
            oa << BOOST_SERIALIZATION_NVP(lst);
        }
        delete lst;
    }
    catch (std::exception& e)
    {
        LOG4CXX_ERROR(WeLogger::GetLogger(), "Can't add task: " << e.what());
        if (lst != NULL)
        {
            delete lst;
        }
        retval = "";
    }
    return retval;
}

bool DelTask(const string& id)
{
    bool retval;
    string fName;
    TaskList    *lst = NULL;

    try
    {
        string fName = taskDbDir + "\\taskList.config";
        std::ifstream itfs(fName.c_str());
        {
            boost::archive::xml_iarchive ia(itfs);
            lst = new TaskList;
            ia >> BOOST_SERIALIZATION_NVP(*lst);
        }
        vector<TaskRecord>::iterator    it;
        for(it = lst->task.begin(); it != lst->task.end(); it++) {
            if ((*it).id == id)
            {
                lst->task.erase(it);
                retval = true;
                break;
            }
        }
        {
            std::ofstream otfs(fName.c_str());
            boost::archive::xml_oarchive oa(otfs);
            oa << BOOST_SERIALIZATION_NVP(lst);
        }
    }
    catch (std::exception& e)
    {
        LOG4CXX_ERROR(WeLogger::GetLogger(), "Can't delete task: " << e.what());
        if (lst != NULL)
        {
            delete lst;
        }
        retval = false;
    }
    return retval;
}
