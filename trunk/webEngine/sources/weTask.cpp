/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew Abramov aabramov@ptsecurity.ru

    This file is part of webEngine

    webEngine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <weHelper.h>
#include <weTask.h>
#include <weiTransport.h>
#include <weiInventory.h>
#include <weiAudit.h>
#include <weiVulner.h>
#include <weScan.h>
#include <weiStorage.h>
#include <weDispatch.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/thread.hpp>
#include <boost/functional/hash.hpp>

using namespace boost;

namespace webEngine {

#ifndef __DOXYGEN__
static const wOption empty_option("_empty_");
#endif //__DOXYGEN__

void TaskProcessor(task* tsk)
{
    wOption opt;
    size_t i;
    int iData;
    response_list::iterator rIt;
    i_response* resp;
    i_request* curr_url;

    tsk->isRunning = true;
    LOG4CXX_TRACE(iLogger::GetLogger(), "WeTaskProcessor started for task " << ((void*)&tsk));
    while (tsk->IsReady())
    {
        tsk->WaitForData();
        opt = tsk->Option(weoParallelReq);
        SAFE_GET_OPTION_VAL(opt, iData, 1);
        tsk->taskQueueSize = iData;
        LOG4CXX_TRACE(iLogger::GetLogger(), "WeTaskProcessor max requests: " << tsk->taskQueueSize <<
            " in queue: " << tsk->taskQueue.size() << " waiting: " << tsk->taskList.size());

        // send request if slots available
        while (tsk->taskQueueSize > tsk->taskQueue.size()) {
            if (!tsk->taskList.empty())
            {
                curr_url = tsk->taskList[0];
                // search for transport or recreate request to all transports
                if (curr_url->RequestUrl().is_valid()) {
                    // search for transport
                    for(i = 0; i < tsk->transports.size(); i++)
                    {
                        if (tsk->transports[i]->is_own_protocol(curr_url->RequestUrl().protocol)) {
                            resp = tsk->transports[i]->request(curr_url);
                            resp->ID(curr_url->ID());
                            resp->processor = curr_url->processor;
                            resp->context = curr_url->context;
                            tsk->taskQueue.push_back(resp);
                            LOG4CXX_DEBUG(iLogger::GetLogger(), "WeTaskProcessor: send request to " << curr_url->RequestUrl().tostring());
                            break;
                        }
                    }
                }
                else {
                    // try to send request though appropriate transports
                    LOG4CXX_DEBUG(iLogger::GetLogger(), "WeTaskProcessor: send request to " << curr_url->RequestUrl().tostring());
                    for(i = 0; i < tsk->transports.size(); i++)
                    {
                        resp = tsk->transports[i]->request(curr_url);
                        resp->ID(curr_url->ID());
                        resp->processor = curr_url->processor;
                        resp->context = curr_url->context;
                        tsk->taskQueue.push_back(resp);
                    }
                }

                //string u_req = curr_url->RequestUrl().tostring();
                tsk->taskList.erase(tsk->taskList.begin());
                curr_url->release();
            }
            else {
                break; // no URLs in the waiting list
            }
        }

        // if any requests pending process transport operations
        if (tsk->taskQueue.size() > 0) {
            LOG4CXX_TRACE(iLogger::GetLogger(), "WeTaskProcessor: transport->process_requests()");
            for(i = 0; i < tsk->transports.size(); i++)
            {
                tsk->transports[i]->process_requests();
            }
            // just switch context to wake up other tasks
            boost::this_thread::sleep(boost::posix_time::millisec(100));
            for(rIt = tsk->taskQueue.begin(); rIt != tsk->taskQueue.end();) {
                if ((*rIt)->Processed()) {
                    if ((*rIt)->processor) {
                        // send to owner
                        LOG4CXX_DEBUG(iLogger::GetLogger(), "WeTaskProcessor: send response to owner");
                        (*rIt)->processor((*rIt), (*rIt)->context);
                    }
                    else {
                        // send to all inventories
                        for (size_t i = 0; i < tsk->inventories.size(); i++)
                        {
                            LOG4CXX_DEBUG(iLogger::GetLogger(), "WeTaskProcessor: send response to " << tsk->inventories[i]->get_description());
                            tsk->inventories[i]->process_response(*rIt);
                        }
                    }
                    i_response* resp = *rIt;
                    resp->release();
                    tsk->taskQueue.erase(rIt);
                    rIt = tsk->taskQueue.begin();
                }
                else {
                    rIt++;
                }
            }
        }
        tsk->CalcStatus();
    };
    tsk->isRunning = false;
    LOG4CXX_TRACE(iLogger::GetLogger(), "WeTaskProcessor finished for task " << ((void*)&tsk));
}

task::task(engine_dispatcher *krnl /*= NULL*/)
{
    // set the default options
    kernel = krnl;
    processThread = false;
    isRunning = false;
    scandata_mutex = (void*)(new boost::mutex);
    mutex_ptr = (void*)(new boost::mutex);
    event_ptr = (void*)(new boost::condition_variable);
    taskList.clear();
    taskQueue.clear();
    taskListSize = 0;
    thread_count = 0;

    transports.clear();
    inventories.clear();
    auditors.clear();
    vulners.clear();

    scanInfo = new ScanInfo;

    LOG4CXX_TRACE(iLogger::GetLogger(), "task created");
}

task::task( task& cpy )
{
    kernel = cpy.kernel;
    LOG4CXX_TRACE(iLogger::GetLogger(), "task assigned");
}

task::~task()
{
    /// @todo Cleanup
    Stop();
    if (scanInfo != NULL) {
        delete scanInfo;
        scanInfo = NULL;
    }
    if (event_ptr != NULL) {
        boost::condition_variable *cond = (boost::condition_variable*)event_ptr;
        delete cond;
        event_ptr = NULL;
    }
    if (mutex_ptr != NULL) {
        boost::mutex *mt = (boost::mutex*)mutex_ptr;
        delete mt;
        mutex_ptr = NULL;
    }
    if (scandata_mutex != NULL) {
        boost::mutex *mt = (boost::mutex*)scandata_mutex;
        delete mt;
        scandata_mutex = NULL;
    }
    size_t i;
    for (i = 0; i < transports.size(); i++) {
        transports[i]->release();
    }
    for (i = 0; i < inventories.size(); i++) {
        inventories[i]->release();
    }
    for (i = 0; i < auditors.size(); i++) {
        auditors[i]->release();
    }
    for (i = 0; i < vulners.size(); i++) {
        vulners[i]->release();
    }
}

i_response* task::GetRequest( i_request* req )
{
    /// @todo Implement this!
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::GetRequest (WeURL)");
    LOG4CXX_ERROR(iLogger::GetLogger(), " *** Not implemented yet! ***");
    return NULL;
}

void task::GetRequestAsync( i_request* req )
{
    /// @todo Implement this!
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::GetRequestAsync");
    processThread = true;
    if (!isRunning) {
        LOG4CXX_DEBUG(iLogger::GetLogger(), "task::GetRequestAsync: create WeTaskProcessor");
        boost::thread process(TaskProcessor, this);
    }
    // wake-up task_processor
    if (event_ptr != NULL && mutex_ptr != NULL) {
        boost::mutex *mt = (boost::mutex*)mutex_ptr;
        boost::condition_variable *cond = (boost::condition_variable*)event_ptr;
        boost::unique_lock<boost::mutex> lock(*mt);
        taskList.push_back(req);
        taskListSize++;
        LOG4CXX_TRACE(iLogger::GetLogger(), "task::GetRequestAsync: new task size=" << taskList.size());
        cond->notify_all();
    }
    return;
}

bool task::IsReady()
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::IsReady - " << processThread);
    return (processThread);
}

void task::AddPlgTransport( i_transport* plugin )
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::AddPlgTransport");
    for (size_t i = 0; i < transports.size(); i++)
    {
        if (transports[i]->get_id() == plugin->get_id())
        {
            // transport already in list
            LOG4CXX_DEBUG(iLogger::GetLogger(), "task::AddPlgTransport - transport already in list");
            return;
        }
    }
    transports.push_back(plugin);
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::AddPlgTransport: added " << plugin->get_description());
}

void task::AddPlgInventory( i_inventory* plugin )
{
    int plgPrio = plugin->get_priority();
    int inPlace = -1;

    LOG4CXX_TRACE(iLogger::GetLogger(), "task::AddPlgInventory");
    for (size_t i = 0; i < inventories.size(); i++)
    {
        if (inventories[i]->get_id() == plugin->get_id())
        {
            // plugin already in list
            LOG4CXX_DEBUG(iLogger::GetLogger(), "task::AddPlgInventory - inventory already in list");
            return;
        }
        if (inventories[i]->get_priority() > plgPrio) {
            inPlace = i;
        }
    }
    if (inPlace > -1) {
        inventories.insert(inventories.begin() + inPlace, plugin);
    }
    else {
        inventories.push_back(plugin);
    }
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::AddPlgInventory: added " << plugin->get_description());
}

void task::AddPlgAuditor( i_audit* plugin )
{
    int plgPrio = plugin->get_priority();
    int inPlace = -1;

    LOG4CXX_TRACE(iLogger::GetLogger(), "task::AddPlgAuditor");
    for (size_t i = 0; i < auditors.size(); i++)
    {
        if (auditors[i]->get_id() == plugin->get_id())
        {
            // plugin already in list
            LOG4CXX_DEBUG(iLogger::GetLogger(), "task::AddPlgAuditor - auditor already in list");
            return;
        }
        if (auditors[i]->get_priority() > plgPrio) {
            inPlace = i;
        }
    }
    if (inPlace > -1) {
        auditors.insert(auditors.begin() + inPlace, plugin);
    }
    else {
        auditors.push_back(plugin);
    }
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::AddPlgAuditor: added " << plugin->get_description());
}

void task::AddPlgVulner( iVulner* plugin )
{
    int plgPrio = plugin->get_priority();
    int inPlace = -1;

    LOG4CXX_TRACE(iLogger::GetLogger(), "task::AddPlgVulner");
    for (size_t i = 0; i < vulners.size(); i++)
    {
        if (vulners[i]->get_id() == plugin->get_id())
        {
            // plugin already in list
            LOG4CXX_DEBUG(iLogger::GetLogger(), "task::AddPlgVulner - vulner already in list");
            return;
        }
        if (vulners[i]->get_priority() > plgPrio) {
            inPlace = i;
        }
    }
    if (inPlace > -1) {
        vulners.insert(vulners.begin() + inPlace, plugin);
    }
    else {
        vulners.push_back(plugin);
    }
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::AddPlgVulner: added " << plugin->get_description());
}

void task::StorePlugins(vector<i_plugin*>& plugins)
{
    string_list::iterator trsp;
    string_list ifaces;

    for (size_t i = 0; i < plugins.size(); i++)
    {
        ifaces = plugins[i]->interface_list();
        LOG4CXX_TRACE(iLogger::GetLogger(), "task::StorePlugins - plugin: " << plugins[i]->get_description() << " ifaces: " << ifaces.size());

        trsp = find(ifaces.begin(), ifaces.end(), "i_transport");
        if (trsp != ifaces.end())
        {
            LOG4CXX_TRACE(iLogger::GetLogger(), "task::StorePlugins - found transport: " << plugins[i]->get_description());
            AddPlgTransport((i_transport*)plugins[i]);
        }

        trsp = find(ifaces.begin(), ifaces.end(), "i_inventory");
        if (trsp != ifaces.end())
        {
            LOG4CXX_TRACE(iLogger::GetLogger(), "task::StorePlugins - found inventory: " << plugins[i]->get_description());
            AddPlgInventory((i_inventory*)plugins[i]);
        }

        trsp = find(ifaces.begin(), ifaces.end(), "i_audit");
        if (trsp != ifaces.end())
        {
            LOG4CXX_TRACE(iLogger::GetLogger(), "task::StorePlugins - found auditor: " << plugins[i]->get_description());
            AddPlgAuditor((i_audit*)plugins[i]);
        }

        trsp = find(ifaces.begin(), ifaces.end(), "iVulner");
        if (trsp != ifaces.end())
        {
            LOG4CXX_TRACE(iLogger::GetLogger(), "task::StorePlugins - found vulner: " << plugins[i]->get_description());
            AddPlgVulner((iVulner*)plugins[i]);
        }
    }

}

void task::Run(void)
{
    LOG4CXX_DEBUG(iLogger::GetLogger(), "task::Run: create WeTaskProcessor");
    Option(weoTaskStatus, WI_TSK_PAUSED);
    boost::thread process(TaskProcessor, this);
    // switch context to initialize TaskProcessor
    boost::this_thread::sleep(boost::posix_time::millisec(10));

    processThread = true;
    for (size_t i = 0; i < auditors.size(); i++)
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "task::Run: initialize " << auditors[i]->get_description());
        auditors[i]->init(this);
    }
    for (size_t i = 0; i < inventories.size(); i++)
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "task::Run: start " << inventories[i]->get_description());
        inventories[i]->start(this);
    }
    Option(weoTaskStatus, WI_TSK_RUN);
    if (event_ptr != NULL && mutex_ptr != NULL) {
        LOG4CXX_TRACE(iLogger::GetLogger(), "task::Pause notify all about TaskStatus change");
        boost::mutex *mt = (boost::mutex*)mutex_ptr;
        boost::condition_variable *cond = (boost::condition_variable*)event_ptr;
        cond->notify_all();
    }
}

void task::Pause(const bool& state /*= true*/)
{
    int idata;
//    wOption opt;
//    opt = Option(weoTaskStatus);
//    SAFE_GET_OPTION_VAL(opt, idata, WI_TSK_RUN);
    if (state)
    {
        idata = WI_TSK_PAUSED;
    }
    else
    {
        idata = WI_TSK_RUN;
    }
    Option(weoTaskStatus, idata);
    if (event_ptr != NULL && mutex_ptr != NULL) {
        LOG4CXX_TRACE(iLogger::GetLogger(), "task::Pause notify all about TaskStatus change");
        boost::mutex *mt = (boost::mutex*)mutex_ptr;
        boost::condition_variable *cond = (boost::condition_variable*)event_ptr;
        cond->notify_all();
    }
}

void task::Stop()
{
    int active_threads = LockedGetValue(&thread_count);
    while (active_threads > 0) {
        LOG4CXX_DEBUG(iLogger::GetLogger(), "task::Stop: " << active_threads << " threads still active, waiting...");
        boost::this_thread::sleep(boost::posix_time::millisec(10));
        active_threads = LockedGetValue(&thread_count);
    }
    processThread = false;
    if (event_ptr != NULL && mutex_ptr != NULL) {
        LOG4CXX_TRACE(iLogger::GetLogger(), "task::Stop notify all about TaskStatus change");
        boost::mutex *mt = (boost::mutex*)mutex_ptr;
        boost::condition_variable *cond = (boost::condition_variable*)event_ptr;
        cond->notify_all();
    }
    // switch context to finalize TaskProcessor
    boost::this_thread::sleep(boost::posix_time::millisec(10));
    taskList.clear();
    taskQueue.clear();
    CalcStatus();
}

void task::CalcStatus()
{

    size_t count = taskList.size();
    int idata = (taskListSize - count) * 100 / taskListSize;
    LOG4CXX_DEBUG(iLogger::GetLogger(), "task::CalcStatus: rest " << count << " queries  from " <<
        taskListSize << " (" << (taskListSize - count) << ": " << idata << "%)");
    Option(weoTaskCompletion, idata);
    // set task status
    if (taskList.size() == 0 && taskQueue.size() == 0) {
        LOG4CXX_DEBUG(iLogger::GetLogger(), "task::CalcStatus: finish!");
        processThread = false;
        scanInfo->finishTime = btm::second_clock::local_time();
        Option(weoTaskStatus, WI_TSK_IDLE);
        boost::mutex *mt = (boost::mutex*)mutex_ptr;
        boost::condition_variable *cond = (boost::condition_variable*)event_ptr;
        cond->notify_all();
    }
}

shared_ptr<ScanData> task::GetScanData( const string& baseUrl )
{
    shared_ptr<ScanData> retval = scanInfo->GetScanData( baseUrl );
    if (retval->scan_id == "")
    {
        retval->scan_id = scanInfo->scanID;
    }
    return retval;
}

void task::SetScanData( const string& baseUrl, shared_ptr<ScanData> scData )
{
    scanInfo->SetScanData(baseUrl, scData);
    if (scData->parsed_data) {
        // send to all auditors
        for (size_t i = 0; i < auditors.size(); i++)
        {
            LOG4CXX_DEBUG(iLogger::GetLogger(), "WeTaskProcessor: send response to " << auditors[i]->get_description());
            auditors[i]->start(this, scData);
        }
    }
    /// @todo: !!! REMOVE THIS!!! It's only debug!
    /// need to implement clever clean-up scheme
    FreeScanData(scData);
}

void task::FreeScanData(shared_ptr<ScanData> scData)
{
    if (scandata_mutex != NULL) {
        boost::mutex *mt = (boost::mutex*)scandata_mutex;
        boost::unique_lock<boost::mutex> lock(*mt);
        LOG4CXX_TRACE(iLogger::GetLogger(), "Free memory for parsed data (SC = " << scData.use_count() << "; PD = " << scData->parsed_data << ")");
        scData->parsed_data.reset();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn db_recordset* task::ToRS( const string& prefix = "" )
///
/// @brief	Converts this object to an db_recordset. This function realizes alternate serialization
/// 		mechanism. It generates db_recordset with all necessary data. This
/// 		representation is used for internal data exchange (for example to store data through
/// 		iweStorage interface). 
///
/// @param  prefix  - The naming prefix. 
///
/// @retval	This object as a std::string. 
////////////////////////////////////////////////////////////////////////////////////////////////////
db_recordset* task::ToRS( const string& parentID/* = ""*/ )
{
    db_recordset* res = NULL; /* new db_recordset;
    db_record* rec;
    db_record* trec;
    wOption optVal;
    string strData;
    string tskId;
    int optCount;
    wOptions::iterator it;

    LOG4CXX_TRACE(iLogger::GetLogger(), "task::ToRS");

    trec = new db_record;
    trec->objectID = weObjTypeTask;
    optVal = Option(weoID);
    SAFE_GET_OPTION_VAL(optVal, tskId, "");
    trec->Option(weoID, optVal.Value());

    optVal = Option(weoParentID);
    SAFE_GET_OPTION_VAL(optVal, strData, "");
    trec->Option(weoParentID, strData);

    optVal = Option(weoName);
    trec->Option(weoName, optVal.Value());

    optVal = Option(weoTaskStatus);
    trec->Option(weoTaskStatus, optVal.Value());

    optVal = Option(weoTaskCompletion);
    trec->Option(weoTaskCompletion, optVal.Value());

    res->push_back(*trec);

    optCount = 0;
    for (it = options.begin(); it != options.end(); it++) {
        strData = it->first;
        // skip predefined options
        if (strData == weoID) {
            continue;
        }
        if (strData == weoName) {
            continue;
        }
        if (strData == weoTaskStatus) {
            continue;
        }
        if (strData == weoTaskCompletion) {
            continue;
        }

        strData = it->first;
        optVal = *(it->second);
        rec = new db_record;
        rec->objectID = weObjTypeSysOption;
        rec->Option(weoName, strData);
        rec->Option(weoParentID, tskId);
        rec->Option(weoTypeID, optVal.Which());
        rec->Option(weoValue, optVal.Value());

        strData += parentID;
        strData += boost::lexical_cast<string>(it->second->Which());
        boost::hash<string> strHash;
        size_t hs = strHash(strData);
        strData = boost::lexical_cast<string>(hs);
        rec->Option(weoID, strData);

        res->push_back(*rec);
        optCount++;
    }
    trec->Option("options", optCount);*/

    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn void task::FromRS( db_recordset *rs )
///
/// @brief  Initializes this object from the given from db_recordset.
/// 		
/// @param  rs	 - db_recordset. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void task::FromRS( db_recordset *rs  )
{
/*    db_record rec;
    size_t r;
    wOptionVal optVal;
    wOption opt;
    string strData, sName;
    int tp;
    char c;
    unsigned char uc;
    int i;
    unsigned int ui;
    long l;
    unsigned long ul;
    bool b;
    double d;

    LOG4CXX_TRACE(iLogger::GetLogger(), "task::FromRS");

    for (r = 0; r < rs->size(); r++)
    {
        rec = (*rs)[r];
        if (rec.objectID == weObjTypeTask) {
            opt = rec.Option(weoID);
            Option(weoID, opt.Value());

            opt = rec.Option(weoName);
            Option(weoName, opt.Value());

            opt = rec.Option(weoTaskStatus);
            Option(weoTaskStatus, opt.Value());

            opt = rec.Option(weoTaskCompletion);
            Option(weoTaskCompletion, opt.Value());
        }
        if (rec.objectID == weObjTypeSysOption) {
            opt = rec.Option(weoName);
            SAFE_GET_OPTION_VAL(opt, sName, "");
            // type 
            opt = rec.Option(weoTypeID);
            SAFE_GET_OPTION_VAL(opt, strData, "8");
            tp = boost::lexical_cast<int>(strData);
            // value
            opt = rec.Option(weoValue);
            strData = boost::lexical_cast<string>(opt.Value());
            switch(tp)
            {
            case 0:
                c = boost::lexical_cast<char>(strData);
                Option(sName, c);
                break;
            case 1:
                uc = boost::lexical_cast<unsigned char>(strData);
                Option(sName, uc);
                break;
            case 2:
                i = boost::lexical_cast<int>(strData);
                Option(sName, i);
                break;
            case 3:
                ui = boost::lexical_cast<unsigned int>(strData);
                Option(sName, ui);
                break;
            case 4:
                l = boost::lexical_cast<long>(strData);
                Option(sName, l);
                break;
            case 5:
                ul = boost::lexical_cast<unsigned long>(strData);
                Option(sName, ul);
                break;
            case 6:
                b = boost::lexical_cast<bool>(strData);
                Option(sName, b);
                break;
            case 7:
                d = boost::lexical_cast<double>(strData);
                Option(sName, d);
                break;
            case 8:
            default:
                Option(sName, strData);
                break;
            }
        }
    }*/
}

void task::WaitForData()
{
    int idata;
    wOption opt;
    opt = Option(weoTaskStatus);
    SAFE_GET_OPTION_VAL(opt, idata, WI_TSK_RUN);

    if (event_ptr != NULL && mutex_ptr != NULL) {
        LOG4CXX_TRACE(iLogger::GetLogger(), "task::WaitForData synch object ready");
        boost::mutex *mt = (boost::mutex*)mutex_ptr;
        boost::condition_variable *cond = (boost::condition_variable*)event_ptr;
        boost::unique_lock<boost::mutex> lock(*mt);
        while((taskList.size() == 0 && taskQueue.size() == 0) || idata == WI_TSK_PAUSED) { // (taskList.size() == 0 && taskQueue.size() == 0) ||
            LOG4CXX_DEBUG(iLogger::GetLogger(), "task::WaitForData: go to sleep");
            cond->wait(lock);

            opt = Option(weoTaskStatus);
            SAFE_GET_OPTION_VAL(opt, idata, WI_TSK_RUN);
        }
    }
}

void task::AddVulner( const string& vId, const string& params, const string& parentId, int vLevel/* = -1*/ )
{
    LOG4CXX_INFO(iLogger::GetLogger(), "task::AddVulner add vulner ID=" << vId << "; ParentID=" << parentId);
    LOG4CXX_INFO(iLogger::GetLogger(), "task::AddVulner data: " << params);
}

int task::add_thread()
{
    int nt = LockedIncrement(&thread_count);
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::add_thread " << nt << " threads active");
    return nt; 
}

int task::remove_thread()
{
    int nt = LockedDecrement(&thread_count);
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::remove_thread " << nt << " threads active");
    return nt; 
}

wOption task::Option( const string& name )
{
    wOptions::iterator it;

    LOG4CXX_TRACE(iLogger::GetLogger(), "task::Option(" << name << ")");
    if (kernel != NULL) {
        return kernel->Option(name);
    }

    // worst case... :(
    return i_options_provider::empty_option;

}

void task::Option( const string& name, wOptionVal val )
{
    if (kernel != NULL) {
        kernel->Option(name, val);
    }
}

bool task::IsSet( const string& name )
{
    bool res = false;

    if (kernel != NULL) {
        res = kernel->IsSet(name);
    }
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::IsSet(" << name << ") value=" << res);
    return res;
}

void task::Erase( const string& name )
{
    if (kernel != NULL) {
        kernel->Erase(name);
    }
}

void task::Clear()
{
    if (kernel != NULL) {
        kernel->Clear();
    }
}

void task::CopyOptions( i_options_provider* cpy )
{
    if (kernel != NULL) {
        kernel->CopyOptions(cpy);
    }
}

string_list task::OptionsList()
{
    string_list lst;

    if (kernel != NULL) {
        lst = kernel->OptionsList();
    }

    return lst;
}

size_t task::OptionSize()
{
    size_t res = 0;

    if (kernel != NULL) {
        res = kernel->OptionSize();
    }

    return res;
}

bool task::is_url_processed( string url )
{
    boost::unordered_map<string, int >::iterator mit = processed_urls.find(url);
    return (mit != processed_urls.end());
}

void task::register_url( string url )
{
    boost::unordered_map<string, int >::iterator mit = processed_urls.find(url);

    if (mit == processed_urls.end()) {
        processed_urls[url] = 1;
    }
    else {
        processed_urls[url]++;
    }
}
} // namespace webEngine
