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
#ifdef WIN32
#   include <process.h>
#else
#   include <sys/types.h>
#   include <unistd.h>
#endif //WIN32
#include <weiBase.h>
#include <weLogger.h>
#include <weDispatch.h>
#include <weTask.h>
#include <weObject.h>
#include <weProfile.h>
#include <weTagScanner.h>
#ifdef WIN32
#define NOGDI
#include <windows.h>
#endif

namespace fs = boost::filesystem;

extern Dispatch* globalDispatcher;
extern string cfgFile;
extern string workDir;

string taskDbDir = ".";

char* trace_modes[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
int trace_mode_last = 5;

int exec_child(char* cmd[])
{
#ifdef WIN32
    return _spawnv(_P_NOWAIT, "tskScanner", cmd);
#else // UNIX (POSIX) implementation
    char wd[2048];
    getcwd(wd, 2000);
    strcat(wd, "/tskScanner");
    pid_t pid = vfork();
    if (pid < 0)
    {   // error
        return pid;
    }
    if (pid > 0)
    {   // parent process
        return 0;
    }
    // child process
    char* envs[1];

    LOG4CXX_DEBUG(iLogger::GetLogger(), "exec_child: " << wd);
    for (int i =0; i < 10; i++) {
        LOG4CXX_DEBUG(iLogger::GetLogger(), "exec_child: arg" << i << " = " << cmd[i]);
    }
    envs[0] = NULL;
    cmd[0] = "./tskScanner";
    return execve("./tskScanner", cmd, envs);
#endif //WIN32
}

void load_task_list( vector<Task*>& task_list )
{
    Record filter;
    Record reps;
    RecordSet report;
    RecordSet topts;
    int tasks, i, opts;
    wOption opt;
    string sdata;

    LOG4CXX_TRACE(iLogger::GetLogger(), "load_task_list");
    filter.objectID = weObjTypeTask;
    filter.Clear();
    reps.objectID = weObjTypeTask;
    reps.Clear();
    tasks = globalDispatcher->Storage()->Get(filter, reps, report);
    for (i = 0; i < tasks; i++)
    {
        RecordSet local;
        Task* tsk;

        tsk = new Task();
        local.push_back(report[i]);
        tsk->FromRS(&local);

        opt = tsk->Option(weoID);
        SAFE_GET_OPTION_VAL(opt, sdata, "");

        filter.objectID = weObjTypeSysOption;
        filter.Clear();
        filter.Option(weoParentID, sdata);
        reps.objectID = weObjTypeSysOption;
        reps.Clear();
        opts = globalDispatcher->Storage()->Get(filter, reps, topts);
        if (opts > 0)
        {
            tsk->FromRS(&topts);
        }
        task_list.push_back(tsk);
    }
}

void save_task( Task* tsk )
{
    RecordSet* taskRep;

    LOG4CXX_TRACE(iLogger::GetLogger(), "save_task");
    taskRep = tsk->ToRS();
    if (taskRep != NULL)
    {
        globalDispatcher->Storage()->Set(*taskRep);
        globalDispatcher->Storage()->Flush();
    }
}

Task* load_task( const string& id )
{
    Record filter;
    Record reps;
    RecordSet report;
    RecordSet topts;
    int tasks, opts;
    Task* tsk = NULL;

    LOG4CXX_TRACE(iLogger::GetLogger(), "load_task");
    filter.objectID = weObjTypeTask;
    filter.Clear();
    filter.Option(weoID, id);
    reps.objectID = weObjTypeTask;
    reps.Clear();
    tasks = globalDispatcher->Storage()->Get(filter, reps, report);
    if (tasks > 0)
    {
        RecordSet local;

        tsk = new Task();
        local.push_back(report[0]);
        tsk->FromRS(&local);

        filter.objectID = weObjTypeSysOption;
        filter.Clear();
        filter.Option(weoParentID, id);
        reps.objectID = weObjTypeSysOption;
        reps.Clear();
        opts = globalDispatcher->Storage()->Get(filter, reps, topts);
        if (opts > 0)
        {
            tsk->FromRS(&topts);
        }
    }
    return tsk;
}

TaskList* get_task_list(const string& criteria/* = ""*/)
{
    vector<Task*>  task_list;
    TaskList    *lst = NULL;
    TaskRecord  *tsk = NULL;
    wOption    opt;
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
    Task *tsk = NULL;

    tsk = new Task;
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
    Record filter;

    retval = false;

    filter.objectID = weObjTypeTask;
    filter.Clear();
    filter.Option(weoID, id);

    if (globalDispatcher->Storage()->Delete(filter) != 0) {
        retval = true;
    }
    return retval;
}

bool run_task(const string& id)
{
    bool retval = false;
    wOption opt;
    string sdata;
    int idata;
    Task* tsk;
    char* cmdline[11];

    memset(cmdline, 0, sizeof(cmdline));
    tsk = load_task(id);
    if (tsk != NULL)
    {
        opt = tsk->Option(weoTaskStatus);
        SAFE_GET_OPTION_VAL(opt, idata, 0);
        if (idata == WI_TSK_IDLE)
        {
            // prepare task to run
            // tsk->Option(weoTaskStatus, WI_TSK_RUN);
            // save_task(tsk);
            // make base filename
            opt = tsk->Option(weoParentID);
            SAFE_GET_OPTION_VAL(opt, sdata, "");
            string fname = sdata + "_" + posix_time::to_iso_string(posix_time::second_clock::local_time());

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
                tr << "log4j.appender.R.File=" << fname << ".log" << endl;
                tr << "log4j.appender.R.MaxFileSize=10MB" << endl;
                tr << "log4j.appender.R.MaxBackupIndex=100" << endl;
                tr << "log4j.appender.R.immediateflush=true" << endl;
                tr << "log4j.appender.R.layout=org.apache.log4j.PatternLayout" << endl;
                tr << "log4j.appender.R.layout.ConversionPattern=[%d][%5p][%t] - %m%n" << endl;
                tr << "log4j.logger.webEngine=" << trace_modes[idata] << ", R" << endl;
            }
            // compose command line arguments
            sdata = "tskScanner";
            cmdline[0] = strdup(sdata.c_str());
            //cmdline[1] = "placeholder ";
            cmdline[1] = "--tid";
            cmdline[2] = strdup(id.c_str());
            cmdline[3] = "--trace";
            sdata = fname + ".trace";
            cmdline[4] = strdup(sdata.c_str());
            cmdline[5] = "--storage";
            cmdline[6] = strdup(globalDispatcher->Storage()->GetID().c_str());
            if (cfgFile != "") {
                cmdline[7] = "--config";
                cmdline[8] = strdup(cfgFile.c_str());
            }
            cmdline[10] = NULL;
            LOG4CXX_DEBUG(iLogger::GetLogger(), "run_task: " << id << " trace: " << workDir << fname << ".trace");
            int err = exec_child(cmdline);
            if (err != -1) {
                retval = true;
            }
            else {
                LOG4CXX_ERROR(iLogger::GetLogger(), "run_task failed: (" << err << ") ERRNO=" << errno);
                fs::remove(fname + ".trace");
            }
        }
        else if (idata == WI_TSK_PAUSED)
        {
            tsk->Option(weoTaskSignal, WE_TASK_SIGNAL_RUN);
            retval = true;
            save_task(tsk);
        }
    }
    else {
        LOG4CXX_WARN(iLogger::GetLogger(), "run_task - task not found: " << id);
    }

    return retval;
}

bool pause_task(const string& id)
{
    bool retval = false;
    wOption opt;
    string sdata;
    int idata;
    Task* tsk;

    tsk = load_task(id);
    if (tsk != NULL)
    {
        opt = tsk->Option(weoTaskStatus);
        SAFE_GET_OPTION_VAL(opt, idata, 0);
        if (idata == WI_TSK_RUN)
        {
            tsk->Option(weoTaskSignal, WE_TASK_SIGNAL_PAUSE);
            retval = true;
            // @todo Pause process which executes this task
            save_task(tsk);
        }
    }
    else {
        LOG4CXX_WARN(iLogger::GetLogger(), "pause_task - task not found: " << id);
    }
    return retval;
}

bool cancel_task(const string& id)
{
    bool retval = false;
    wOption opt;
    string sdata;
    int idata;
    Task* tsk;

    tsk = load_task(id);
    if (tsk != NULL)
    {
        opt = tsk->Option(weoTaskStatus);
        SAFE_GET_OPTION_VAL(opt, idata, 0);
        if (idata == WI_TSK_RUN)
        {
            tsk->Option(weoTaskSignal, WE_TASK_SIGNAL_STOP);
            retval = true;
            // @todo Cancel process which executes this task
            save_task(tsk);
        }
    }
    else {
        LOG4CXX_WARN(iLogger::GetLogger(), "cancel_task - task not found: " << id);
    }
    return retval;
}

string get_task_opts (const string& id)
{
    string retval = "";
    wOption opt;
    string sdata;
    Task* tsk;

    tsk = load_task(id);
    if (tsk != NULL)
    {
        // need to call exactly iOptionsProvider::ToXml()
        // 'cause it gets all options in the one pack in difference
        // of structured output of Task::ToXml()
        retval = tsk->ToXml();
    }
    else {
        LOG4CXX_WARN(iLogger::GetLogger(), "get_task_opts - task not found: " << id);
    }
    return retval;
}

bool set_task_opts (const string& dat)
{
    bool retval = false;
    string id;
    string xml;
    wOption opt;
    string sdata;
    size_t t;
    Task* tsk;

    t = dat.find(';');
    if (t != string::npos) {
        id = dat.substr(0, t);
        xml = dat.substr(t+1);
        tsk = load_task(id);
        if (tsk != NULL)
        {
            // need to call exactly iOptionsProvider::FromXml()
            // 'cause it gets all options in the one pack in difference
            // of structured processing of Task::FromXml()
            tsk->FromXml(xml);
            save_task(tsk);
            retval = true;
        }
        else {
            LOG4CXX_WARN(iLogger::GetLogger(), "get_task_opts - task not found: " << id);
        }
    }
    return retval;
}

ObjectList* get_object_list(const string& criteria /*= ""*/)
{
    Record filter;
    Record reps;
    RecordSet report;
    int objs = 0, i;
    ObjectList* lst;
    ObjectInf* obj;

    LOG4CXX_TRACE(iLogger::GetLogger(), "get_object_list");
    lst = NULL;

    filter.objectID = weObjTypeObject;
    filter.Clear();
    reps.objectID = weObjTypeObject;
    reps.Clear();
    objs = globalDispatcher->Storage()->Get(filter, reps, report);

    lst = new ObjectList;
    lst->clear();
    for(i = 0; i < objs; i++)
    {
        ScanObject tsk;
        RecordSet local;
        local.push_back(report[i]);
        tsk.FromRS(&local);
        obj = new ObjectInf;
        obj->ObjectId = tsk.ObjectId;
        obj->Name = tsk.ObjName;
        obj->Address = tsk.Address;
        lst->push_back(*obj);
    }
    return lst;
}

string update_object(ObjectInf& obj)
{
    ScanObject tsk;
    RecordSet* result;

    if (obj.ObjectId == "0" || obj.ObjectId == "")
    {
        obj.ObjectId = globalDispatcher->Storage()->GenerateID(weObjTypeObject);
    }
    tsk.ObjectId = obj.ObjectId;
    tsk.ObjName = obj.Name;
    tsk.Address = obj.Address;
    result = tsk.ToRS();
    if (result != NULL) {
        globalDispatcher->Storage()->Set(*result);
    }
    
    return obj.ObjectId;
}

bool del_object(const string& id)
{
    Record filter;
    bool retval = false;

    filter.objectID = weObjTypeObject;
    filter.Clear();
    filter.Option(weoID, id);

    if (globalDispatcher->Storage()->Delete(filter) != 0) {
        retval = true;
    }
    return retval;
}

WeProfile* get_profile(const string& id)
{
    Record filter;
    Record reps;
    RecordSet report;
    int tasks = 0;
    WeProfile* obj;

    LOG4CXX_TRACE(iLogger::GetLogger(), "load_task_list");
    filter.objectID = weObjTypeProfile;
    filter.Clear();
    filter.Option(weoID, id);
    reps.objectID = weObjTypeProfile;
    reps.Clear();
    tasks = globalDispatcher->Storage()->Get(filter, reps, report);

    obj = NULL;
    if (tasks > 0)
    {   // parse the response
        obj = new WeProfile;
        obj->FromRS(&report);

        filter.objectID = weObjTypeSysOption;
        filter.Clear();
        filter.Option(weoParentID, id);
        reps.objectID = weObjTypeSysOption;
        reps.Clear();
        tasks = globalDispatcher->Storage()->Get(filter, reps, report);
        if (tasks > 0)
        {
            obj->FromRS(&report);
        }
    }
    return obj;
}

string get_profile_opts (const string& id)
{
    string retval = "";
    Record filter;
    Record reps;
    RecordSet report;
    WeProfile* opts;

    opts = get_profile(id);
    if (opts != NULL)
    {
        retval = opts->ToXml();
    }
    return retval;
}

bool set_profile_opts (const string& dat)
{
    bool retval = false;
    string id;
    string xml;
    size_t t;
    WeProfile* prof;
    RecordSet* rs;

    t = dat.find(';');
    if (t != string::npos) {
        id = dat.substr(0, t);
        xml = dat.substr(t+1);
        prof = get_profile(id);
        if (prof != NULL)
        {
            // need to call exactly iOptionsProvider::FromXml()
            // 'cause it gets all options in the one pack in difference
            // of structured processing of WeProfile::FromXml()
            ((iOptionsProvider*)prof)->FromXml(xml);
            rs = prof->ToRS();
            if (globalDispatcher->Storage()->Set(*rs) > 0) {
                retval = true;
            }
            else {
                LOG4CXX_WARN(iLogger::GetLogger(), "set_profile_opts - profile " << id << " not saved!");
            }
        }
        else {
            LOG4CXX_WARN(iLogger::GetLogger(), "set_profile_opts - profile not found: " << id);
        }
    }
    return retval;
}

bool del_profile (const string& id)
{
    Record filter;
    bool retval = false;

    filter.objectID = weObjTypeProfile;
    filter.Clear();
    filter.Option(weoID, id);

    if (globalDispatcher->Storage()->Delete(filter) != 0) {
        retval = true;
    }
    return retval;
}

ProfileList* get_profile_list(const string& criteria = "")
{
    Record filter;
    Record reps;
    RecordSet report;
    string id;
    int tasks = 0, i;
    ProfileList* lst;
    ProfileInf* obj;
    wOption opt;
    string strData;

    LOG4CXX_TRACE(iLogger::GetLogger(), "get_profile_list");
    lst = NULL;
    filter.objectID = weObjTypeProfile;
    filter.Clear();
    reps.objectID = weObjTypeProfile;
    reps.Clear();
    tasks = globalDispatcher->Storage()->Get(filter, reps, report);

    lst = new ProfileList;
    for (i = 0; i < tasks; i++)
    {   // parse the response
        RecordSet local;
        WeProfile tsk;
        
        local.push_back(report[i]);
        tsk.FromRS(&local);

        obj = new ProfileInf;
        opt = tsk.Option(weoID);
        SAFE_GET_OPTION_VAL(opt, strData, "0");
        obj->ObjectId = strData;
        opt = tsk.Option(weoName);
        SAFE_GET_OPTION_VAL(opt, strData, "0");
        obj->Name = strData;
        lst->push_back(*obj);

    }
    return lst;
}
