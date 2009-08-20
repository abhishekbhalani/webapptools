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

string taskDbDir = ".";

char* trace_modes[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
int trace_mode_last = 5;

int exec_child(const string cmd)
{
#ifdef WIN32
    return _spawnl(_P_NOWAIT, "tskScanner", cmd.c_str(), NULL);
#else // UNIX (POSIX) implementation
    string line = string("./") + cmd;
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
    char* args[2];
    char* envs[1];

    args[0] = strdup(line.c_str());
    args[1] = NULL;
    envs[0] = NULL;
    execve("./tskScanner", args, envs);
#endif //WIN32
}

void load_task_list( vector<Task*>& task_list )
{
    string report = "";
    int tasks = 0;

    tasks = globalDispatcher->Storage()->TaskReport("<report><task value='*' /></report>", report);
    if (tasks > 0)
    {   // parse the response
        bool in_parsing = true;
        int parsing_level = 0;
        size_t xml_pos;
        StrStream st(report.c_str());
        TagScanner sc(st);
        Task* tsk;
        string tag;

        while(in_parsing) {
            xml_pos = sc.GetPos();
            int t = sc.GetToken();
            switch(t)
            {
            case wstError:
                LOG4CXX_WARN(iLogger::GetLogger(), "load_task_list - parsing error");
                in_parsing = false;
                break;
            case wstEof:
                LOG4CXX_TRACE(iLogger::GetLogger(), "load_task_list - parsing EOF");
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
                        tsk = new Task();
                        // go back to the start of the TAG
                        tsk->FromXml(sc, t);
                        task_list.push_back(tsk);
                        break;
                    }
                }
                LOG4CXX_WARN(iLogger::GetLogger(), "load_task_list - unexpected tag: " << tag);
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

void save_task( Task* tsk )
{
    string taskRep;

    taskRep = tsk->ToXml();
    globalDispatcher->Storage()->TaskSave(taskRep);
    globalDispatcher->Storage()->Flush();
}

Task* load_task( const string& id )
{
    string req;
    string report = "";
    int tasks = 0;
    Task* tsk;

    req = "<report><task value='" + id + "' /></report>";
    tasks = globalDispatcher->Storage()->TaskReport(req, report);
    tsk = NULL;
    if (tasks > 0)
    {   // parse the response
        bool in_parsing = true;
        int parsing_level = 0;
        size_t xml_pos;
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
                        tsk = new Task();
                        // go back to the start of the TAG
                        tsk->FromXml(sc, t);
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
    wOption opt;
    string sdata;
    int idata;
    Task* tsk;

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
            LOG4CXX_DEBUG(iLogger::GetLogger(), "run_task: " << cmdline);
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
        retval = ((iOptionsProvider*)tsk)->ToXml();
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
            ((iOptionsProvider*)tsk)->FromXml(xml);
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
    string report = "";
    string result;
    string crit;
    string query = "";
    int tasks = 0, i;
    ObjectList* lst;
    ObjectInf* obj;

    if (criteria == "") {
        crit = "*";
    }
    else {
        crit = criteria;
    }
    lst = NULL;
    //query = "<report>";
    tasks = 0;
    result = "<report>\n";
    size_t tpos = crit.find(',');
    while (tpos != string::npos) {
        //query += "<object value='" + crit.substr(0, tpos) + "' />";
        i = globalDispatcher->Storage()->Report(weObjTypeObject, crit.substr(0,tpos), "", report);
        if(i > 0) {
            tasks += i;
            result += report;
        }
        crit = crit.substr(tpos+1);
        tpos = crit.find(',');
    }
    //query += "<object value='" + crit + "' />";
    i = globalDispatcher->Storage()->Report(weObjTypeObject, crit, "", report);
    if(i > 0) {
        tasks += i;
        result += report;
    }
    //query += "</report>";
    result += "</report>";
    //tasks = globalDispatcher->Storage()->ObjectReport(weObjTypeObject, query, report);
    if (tasks > 0)
    {   // parse the response
        bool in_parsing = true;
        int parsing_level = 0;
        size_t xml_pos;
        StrStream st(result.c_str());
        TagScanner sc(st);
        ScanObject tsk;
        string tag;

        while(in_parsing) {
            xml_pos = sc.GetPos();
            int t = sc.GetToken();
            switch(t)
            {
            case wstError:
                LOG4CXX_WARN(iLogger::GetLogger(), "get_object_list - parsing error");
                in_parsing = false;
                break;
            case wstEof:
                LOG4CXX_TRACE(iLogger::GetLogger(), "get_object_list - parsing EOF");
                in_parsing = false;
                break;
            case wstTagStart:
                tag = sc.GetTagName();
                if (parsing_level == 0)
                {
                    if (iequals(tag, "report")) {
                        lst = new ObjectList;
                        lst->clear();
                        parsing_level++;
                        break;
                    }
                }
                if (parsing_level == 1) {
                    if (iequals(tag, weObjTypeObject)) {
                        // go back to the start of the TAG
                        tsk.FromXml(sc, t);
                        obj = new ObjectInf;
                        obj->ObjectId = tsk.ObjectId;
                        obj->Name = tsk.ObjName;
                        obj->Address = tsk.Address;
                        lst->push_back(*obj);
                        break;
                    }
                }
                LOG4CXX_WARN(iLogger::GetLogger(), "get_object_list - unexpected tag: " << tag);
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
    return lst;
}

string update_object(ObjectInf& obj)
{
    ScanObject tsk;
    string result;

    if (obj.ObjectId == "0")
    {
        obj.ObjectId = globalDispatcher->Storage()->GenerateID(weObjTypeObject);
    }
    tsk.ObjectId = obj.ObjectId;
    tsk.ObjName = obj.Name;
    tsk.Address = obj.Address;
    result = tsk.ToXml();
    globalDispatcher->Storage()->Query(weObjTypeObject, obj.ObjectId, iStorage::autoop, result);
    return obj.ObjectId;
}

bool del_object(const string& id)
{
    bool retval;
    string fName;
    TaskList    *lst = NULL;

    retval = false;
    fName = "<delete object='" + id + "'/>";
    if (globalDispatcher->Storage()->Delete(weObjTypeObject, fName) != 0) {
        retval = true;
    }
    return retval;
}

WeProfile* get_profile(const string& id)
{
    string report = "";
    string result;
    string crit;
    string query = "";
    int tasks = 0, i;
    WeProfile* obj;

    tasks = 0;
    result = "<report>\n";
    i = globalDispatcher->Storage()->Report(weObjTypeProfile, id, "", report);
    if(i > 0) {
        tasks += i;
        result += report;
    }
    result += "</report>";
    obj = NULL;
    if (tasks > 0)
    {   // parse the response
        bool in_parsing = true;
        int parsing_level = 0;
        size_t xml_pos;
        StrStream st(result.c_str());
        TagScanner sc(st);
        wOption opt;
        string sdata;
        string tag;

        while(in_parsing) {
            xml_pos = sc.GetPos();
            int t = sc.GetToken();
            switch(t)
            {
            case wstError:
                LOG4CXX_WARN(iLogger::GetLogger(), "get_profile - parsing error");
                in_parsing = false;
                break;
            case wstEof:
                LOG4CXX_TRACE(iLogger::GetLogger(), "get_profile - parsing EOF");
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
                    if (iequals(tag, weObjTypeProfile)) {
                        // go back to the start of the TAG
                        obj = new WeProfile;
                        obj->FromXml(sc, t);
                        in_parsing = false; // only first object
                        break;
                    }
                }
                LOG4CXX_WARN(iLogger::GetLogger(), "get_profile - unexpected tag: " << tag);
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
    return obj;
}

string get_profile_opts (const string& id)
{
    string retval = "";
    string result = "";
    string query = "";
    int objs;

    query = "<report><profile value='" + id + "'/></report>";
    retval = "<report>";
    objs = globalDispatcher->Storage()->Report(weObjTypeProfile, id, query, result);
    if (objs > 0)
    {
        retval += result;
    }
    else {
        LOG4CXX_WARN(iLogger::GetLogger(), "get_profile_opts - profile not found: " << id);
    }
    retval += "</report>";
    return retval;
}

bool set_profile_opts (const string& dat)
{
    bool retval = false;
    string id;
    string xml;
    size_t t;
    WeProfile* prof;

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
            xml = prof->ToXml();
            if (globalDispatcher->Storage()->Query(weObjTypeProfile, id, iStorage::autoop, xml) > 0) {
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
    bool retval = false;
    string query = "";

    query = "<delete profile='" + id + "'/>";
    if (globalDispatcher->Storage()->Delete(weObjTypeProfile, query) > 0) {
        retval = true;
    }
    else {
        LOG4CXX_WARN(iLogger::GetLogger(), "del_profile - profile " << id << " not deleted!");
    }
    return retval;
}

ProfileList* get_profile_list(const string& criteria = "")
{
    string report = "";
    string result;
    string crit;
    string query = "";
    int tasks = 0, i;
    ProfileList* lst;
    ProfileInf* obj;

    if (criteria == "") {
        crit = "*";
    }
    else {
        crit = criteria;
    }
    lst = NULL;
    //query = "<report>";
    tasks = 0;
    result = "<report>\n";
    size_t tpos = crit.find(',');
    while (tpos != string::npos) {
        //query += "<profile value='" + crit.substr(0, tpos) + "' />";
        i = globalDispatcher->Storage()->Report(weObjTypeProfile, crit.substr(0,tpos), "", report);
        if(i > 0) {
            tasks += i;
            result += report;
        }
        crit = crit.substr(tpos+1);
        tpos = crit.find(',');
    }
    //query += "<profile value='" + crit + "' />";
    i = globalDispatcher->Storage()->Report(weObjTypeProfile, crit, "", report);
    if(i > 0) {
        tasks += i;
        result += report;
    }
    //query += "</report>";
    result += "</report>";
    //tasks = globalDispatcher->Storage()->ObjectReport(weObjTypeProfile, query, report);
    if (tasks > 0)
    {   // parse the response
        bool in_parsing = true;
        int parsing_level = 0;
        size_t xml_pos;
        StrStream st(result.c_str());
        TagScanner sc(st);
        wOption opt;
        string sdata;
        WeProfile tsk;
        string tag;

        while(in_parsing) {
            xml_pos = sc.GetPos();
            int t = sc.GetToken();
            switch(t)
            {
            case wstError:
                LOG4CXX_WARN(iLogger::GetLogger(), "get_profile_list - parsing error");
                in_parsing = false;
                break;
            case wstEof:
                LOG4CXX_TRACE(iLogger::GetLogger(), "get_profile_list - parsing EOF");
                in_parsing = false;
                break;
            case wstTagStart:
                tag = sc.GetTagName();
                if (parsing_level == 0)
                {
                    if (iequals(tag, "report")) {
                        lst = new ProfileList;
                        lst->clear();
                        parsing_level++;
                        break;
                    }
                }
                if (parsing_level == 1) {
                    if (iequals(tag, weObjTypeProfile)) {
                        // go back to the start of the TAG
                        tsk.FromXml(sc, t);
                        obj = new ProfileInf;
                        opt = tsk.Option(weoID);
                        SAFE_GET_OPTION_VAL(opt, sdata, "0");
                        obj->ObjectId = sdata;
                        opt = tsk.Option(weoName);
                        SAFE_GET_OPTION_VAL(opt, sdata, "0");
                        obj->Name = sdata;
                        lst->push_back(*obj);
                        break;
                    }
                }
                LOG4CXX_WARN(iLogger::GetLogger(), "get_profile_list - unexpected tag: " << tag);
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
    return lst;
}
