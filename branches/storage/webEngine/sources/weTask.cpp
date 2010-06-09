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
#include <webEngine.h>

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
#include <boost/functional/hash.hpp>
#include <boost/scoped_ptr.hpp>

using namespace boost;

namespace webEngine {

#ifndef __DOXYGEN__
static const we_option empty_option("_empty_");
#endif //__DOXYGEN__

typedef struct _sync_req_data {
    boost::shared_ptr<boost::mutex> req_mutex;
    boost::shared_ptr<boost::condition_variable> req_event;
    boost::shared_ptr<i_response> response;
} sync_req_data;

void task_sync_request(boost::shared_ptr<i_response> resp, void* context)
{
    LOG4CXX_DEBUG(iLogger::GetLogger(), "task_sync_request - process response");
    sync_req_data *req_data = (sync_req_data*)context;

    boost::lock_guard<boost::mutex> lock(*(req_data->req_mutex.get()));
    req_data->response = resp;
    req_data->req_event->notify_all();
}

void task_processor(task* tsk)
{
    we_option opt;
    size_t i;
    int iData;
    response_list::iterator rIt;
    i_response_ptr resp;
    i_request_ptr curr_url;

    tsk->processThread = true;
    LOG4CXX_INFO(iLogger::GetLogger(), "TRACE: WeTaskProcessor started for task " << ((void*)&tsk));
    while (tsk->IsReady())
    {
        tsk->WaitForData();
        opt = tsk->Option(weoParallelReq);
        SAFE_GET_OPTION_VAL(opt, iData, 1);
        tsk->taskQueueSize = iData;
        LOG4CXX_TRACE(iLogger::GetLogger(), "WeTaskProcessor max requests: " << tsk->taskQueueSize <<
            " in queue: " << tsk->taskQueue.size() << " waiting: " << tsk->taskList.size() << " done: " << tsk->total_done);

        // send request if slots available
        {
            boost::unique_lock<boost::mutex> lock(tsk->tsk_mutex);
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
                                resp = tsk->transports[i]->request(curr_url.get());
                                resp->ID(curr_url->ID());
                                resp->processor = curr_url->processor;
                                resp->context = curr_url->context;
                                tsk->taskQueue.push_back(resp);
                                LOG4CXX_DEBUG(iLogger::GetLogger(), "WeTaskProcessor: send request to " << curr_url->RequestUrl().tostring());
                                resp.reset();
                                break;
                            }
                        }
                        if ( i == tsk->transports.size()) {
                            LOG4CXX_WARN(iLogger::GetLogger(), "WeTaskProcessor: can't send request to " << curr_url->RequestUrl().tostring() <<
                                " no transports accept this request!");
                            // need to send the "abort request" message to caller
                            resp = curr_url->abort_request();
                            resp->ID(curr_url->ID());
                            resp->processor = curr_url->processor;
                            resp->context = curr_url->context;
                            tsk->taskQueue.push_back(resp);
                        }
                    }
                    else {
                        // try to send request though appropriate transports
                        LOG4CXX_DEBUG(iLogger::GetLogger(), "WeTaskProcessor: send request to " << curr_url->RequestUrl().tostring());
                        for(i = 0; i < tsk->transports.size(); i++)
                        {
                            resp = tsk->transports[i]->request(curr_url.get());
                            resp->ID(curr_url->ID());
                            resp->processor = curr_url->processor;
                            resp->context = curr_url->context;
                            tsk->taskQueue.push_back(resp);
                        }
                    }

                    //string u_req = curr_url->RequestUrl().tostring();
                    tsk->taskList.erase(tsk->taskList.begin());
                    //curr_url.reset(); - automatically desctroy on list.erase
                }
                else {
                    break; // no URLs in the waiting list
                }
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
                    LOG4CXX_DEBUG(iLogger::GetLogger(), "WeTaskProcessor: received! Queue: " << tsk->taskQueue.size() << " done: " << tsk->total_done);
                    tsk->total_done++;
                    if ((*rIt)->processor) {
                        // send to owner
                        LOG4CXX_DEBUG(iLogger::GetLogger(), "WeTaskProcessor: send response to owner");
                        (*rIt)->processor((*rIt), (*rIt)->context);
                    }
                    else {
                        // process response by task queue
                        // 1. Parse data
                        i_document_ptr parsed_doc;

                        for (size_t i = 0; i < tsk->parsers.size(); i++)
                        {
                            LOG4CXX_DEBUG(iLogger::GetLogger(), "WeTaskProcessor: parse response by " << tsk->parsers[i]->get_description());
                            parsed_doc = tsk->parsers[i]->parse(*rIt);
                            if (parsed_doc) {
                                // parsed!
                                LOG4CXX_TRACE(iLogger::GetLogger(), "WeTaskProcessor: parsed!");
                                break;
                            }
                        }
                        // 2. Create ScanData
                        scan_data_ptr scd = tsk->GetScanData((*rIt)->BaseUrl().tostring());
                        scd->response = *rIt;
                        if (! (scd->parsed_data)) {
                            scd->parsed_data = parsed_doc;
                        }
                        // 3. Push to processing
                        tsk->SetScanData((*rIt)->BaseUrl().tostring(), scd);
                    }
                    //(*rIt).reset();
                    tsk->taskQueue.erase(rIt);
                    rIt = tsk->taskQueue.begin();
                }
                else {
                    rIt++;
                }
            }
            LOG4CXX_DEBUG(iLogger::GetLogger(), "WeTaskProcessor: queue: " << tsk->taskQueue.size() << " done: " << tsk->total_done);
        }
        tsk->calc_status();
    };
    tsk->processThread = false;
    LOG4CXX_INFO(iLogger::GetLogger(), "TRACE: WeTaskProcessor finished for task " << ((void*)&tsk));
}

void task_data_process(task* tsk)
{
    scan_data_ptr sc_data;

    LOG4CXX_INFO(iLogger::GetLogger(), "TRACE: WeTaskProcessData started for task " << ((void*)&tsk));
    tsk->add_thread();
    while (tsk->sc_process.size() > 0) {
        {
            boost::unique_lock<boost::mutex> lock(tsk->scandata_mutex);
            sc_data = tsk->sc_process[0];
            tsk->sc_process.erase(tsk->sc_process.begin());
        }
        if (sc_data) {
            LOG4CXX_TRACE(iLogger::GetLogger(), "task_data_process: response for " << sc_data->object_url << " id = " << sc_data->data_id );
            // send to all inventories
            for (size_t i = 0; i < tsk->inventories.size(); i++)
            {
                LOG4CXX_DEBUG(iLogger::GetLogger(), "task_data_process: send response to " << tsk->inventories[i]->get_description());
                tsk->inventories[i]->process(tsk, sc_data);
            }
            // send to all auditors
            for (size_t i = 0; i < tsk->auditors.size(); i++)
            {
                LOG4CXX_DEBUG(iLogger::GetLogger(), "task_data_process: send response to " << tsk->auditors[i]->get_description());
                tsk->auditors[i]->process(tsk, sc_data);
            }
            // send to all vulner detectors
            for (size_t i = 0; i < tsk->vulners.size(); i++)
            {
                LOG4CXX_DEBUG(iLogger::GetLogger(), "task_data_process: send response to " << tsk->vulners[i]->get_description());
                tsk->vulners[i]->process(tsk, sc_data);
            }
            /// @todo: !!! REMOVE THIS!!! It's only debug!
            /// need to implement clever clean-up scheme
            tsk->FreeScanData(sc_data);
        } // if sc_data valid
    } // while sc_process.size > 0
    tsk->remove_thread();
    tsk->dataThread = false;
    LOG4CXX_INFO(iLogger::GetLogger(), "TRACE: WeTaskProcessData finished for task " << ((void*)&tsk));
}

task::task(engine_dispatcher *krnl /*= NULL*/)
{
    // set the default options
    kernel = krnl;
    processThread = false;
    dataThread = false;

    taskList.clear();
    taskQueue.clear();
    thread_count = 0;

    parsers.clear();
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
    size_t i;
    for (i = 0; i < parsers.size(); i++) {
        parsers[i]->release();
    }
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

i_response_ptr task::get_request( i_request_ptr req )
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::get_request (WeURL)");
    i_response_ptr retval;
    boost::scoped_ptr<sync_req_data> rdata(new sync_req_data);
    rdata->req_mutex.reset(new boost::mutex);
    rdata->req_event.reset(new boost::condition_variable);
    rdata->response.reset();

    req->processor = task_sync_request;
    req->context   = (void*)(rdata.get());
    get_request_async(req);
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::get_request - waiting for response");
    boost::unique_lock<boost::mutex> lock(*rdata->req_mutex);
    while (!rdata->response) {
        rdata->req_event->wait(lock);
    }
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::get_request - data received");
    retval = rdata->response;
    return retval;
}

void task::get_request_async( i_request_ptr req )
{
    /// @todo Implement this!
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::get_request_async");
    if (!processThread) {
        processThread = true;
        LOG4CXX_DEBUG(iLogger::GetLogger(), "task::get_request_async: create WeTaskProcessor");
        boost::thread process(task_processor, this);
    }
    // wake-up task_processor
    boost::unique_lock<boost::mutex> lock(tsk_mutex);
    if (req->processor != NULL) {
        taskList.insert(taskList.begin(), req);
    }
    else {
        taskList.push_back(req);
    }
    total_reqs++;
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::get_request_async: new task size=" << taskList.size());
    tsk_event.notify_all();
    return;
}

bool task::IsReady()
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::IsReady - " << processThread);
    return (processThread);
}

void task::AddPlgParser(i_parser* plugin)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::AddPlgParser");
    for (size_t i = 0; i < parsers.size(); i++)
    {
        if (parsers[i]->get_id() == plugin->get_id())
        {
            // transport already in list
            LOG4CXX_DEBUG(iLogger::GetLogger(), "task::AddPlgParser - parser already in list");
            return;
        }
    }
    parsers.push_back(plugin);
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::AddPlgParser: added " << plugin->get_description());
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

void task::AddPlgVulner( i_vulner* plugin )
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

        trsp = find(ifaces.begin(), ifaces.end(), "i_parser");
        if (trsp != ifaces.end())
        {
            LOG4CXX_TRACE(iLogger::GetLogger(), "task::StorePlugins - found parser: " << plugins[i]->get_description());
            AddPlgParser((i_parser*)plugins[i]);
        }

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

        trsp = find(ifaces.begin(), ifaces.end(), "i_vulner");
        if (trsp != ifaces.end())
        {
            LOG4CXX_TRACE(iLogger::GetLogger(), "task::StorePlugins - found vulner: " << plugins[i]->get_description());
            AddPlgVulner((i_vulner*)plugins[i]);
        }
    }

}

void task::Run(void)
{
    LOG4CXX_DEBUG(iLogger::GetLogger(), "task::Run: create WeTaskProcessor");
    Option(weoTaskStatus, WI_TSK_PAUSED);
    if (!processThread)
    {
        processThread = true;
        boost::thread process(task_processor, this);
        // switch context to initialize TaskProcessor
        boost::this_thread::sleep(boost::posix_time::millisec(10));
    }

    total_reqs = 0;
    total_done = 0;
    processThread = true;
    // init parsers first
    for (size_t i = 0; i < parsers.size(); i++)
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "task::Run: initialize " << parsers[i]->get_description());
        parsers[i]->init(this);
    }

    // init vulner detectors
    for (size_t i = 0; i < vulners.size(); i++)
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "task::Run: initialize " << vulners[i]->get_description());
        vulners[i]->init(this);
    }

    // init auditors
    for (size_t i = 0; i < auditors.size(); i++)
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "task::Run: initialize " << auditors[i]->get_description());
        auditors[i]->init(this);
    }

    // last step: init inventories and run the process
    for (size_t i = 0; i < inventories.size(); i++)
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "task::Run: start " << inventories[i]->get_description());
        inventories[i]->init(this);
    }
    Option(weoTaskStatus, WI_TSK_RUN);

    boost::unique_lock<boost::mutex> lock(tsk_mutex);
    tsk_event.notify_all();
}

void task::Pause(const bool& state /*= true*/)
{
    int idata;
//    we_option opt;
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

    LOG4CXX_TRACE(iLogger::GetLogger(), "task::Pause notify all about TaskStatus change");
    boost::unique_lock<boost::mutex> lock(tsk_mutex);
    tsk_event.notify_all();
}

void task::Stop()
{
    int active_threads = LockedGetValue(&thread_count);

    // stop vulner detectors
    for (size_t i = 0; i < vulners.size(); i++)
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "task::Run: stopping " << vulners[i]->get_description());
        vulners[i]->stop(this);
    }

    // stop auditors
    for (size_t i = 0; i < auditors.size(); i++)
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "task::Run: stopping " << auditors[i]->get_description());
        auditors[i]->stop(this);
    }

    // stop inventories
    for (size_t i = 0; i < inventories.size(); i++)
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "task::Run: stopping " << inventories[i]->get_description());
        inventories[i]->stop(this);
    }

    // stop parsers
    for (size_t i = 0; i < parsers.size(); i++)
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "task::Run: stopping " << parsers[i]->get_description());
        parsers[i]->stop(this);
    }

    while (active_threads > 0) {
        LOG4CXX_DEBUG(iLogger::GetLogger(), "task::Stop: " << active_threads << " threads still active, waiting...");
        boost::this_thread::sleep(boost::posix_time::millisec(100));
        active_threads = LockedGetValue(&thread_count);
    }
    processThread = false;
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "task::Stop notify all about TaskStatus change");
        boost::unique_lock<boost::mutex> lock(tsk_mutex);
        tsk_event.notify_all();
    }
    // switch context to finalize TaskProcessor
    boost::this_thread::sleep(boost::posix_time::millisec(10));
    taskList.clear();
    taskQueue.clear();
    calc_status();
}

void task::calc_status()
{

    size_t count = taskList.size();
    int idata = 100;
    if (total_reqs != 0) {
        idata = (total_reqs - count) * 100 / total_reqs;
    }
    int active_threads = LockedGetValue(&thread_count);
    LOG4CXX_DEBUG(iLogger::GetLogger(), "task::calc_status: rest " << count << " queries  from " <<
        total_reqs << " (" << (total_reqs - count) << ": " << idata << "%); waiting for: " << taskQueue.size() <<
        " requests, done: " << total_done << "; "<< active_threads << " active threads");
    if ( (count + total_done + taskQueue.size()) < total_reqs) {
        LOG4CXX_WARN(iLogger::GetLogger(), "task::calc_status: counters mismatch! count + total_done + taskQueue.size() < total_reqs!");
    }
    Option(weoTaskCompletion, idata);
    // set task status
    if (taskList.size() == 0 && taskQueue.size() == 0 && active_threads == 0) {
        LOG4CXX_DEBUG(iLogger::GetLogger(), "task::calc_status: finish!");
        processThread = false;
        scanInfo->finishTime = btm::second_clock::local_time();
        Option(weoTaskStatus, WI_TSK_IDLE);
        boost::unique_lock<boost::mutex> lock(tsk_mutex);
        tsk_event.notify_all();
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
    {
        boost::unique_lock<boost::mutex> lock(scandata_mutex);
        scanInfo->SetScanData(baseUrl, scData);
        sc_process.push_back(scData);
    }
    if (!dataThread) {
        dataThread = true;
        boost::thread(task_data_process, this);
    }
}

void task::FreeScanData(shared_ptr<ScanData> scData)
{
    boost::unique_lock<boost::mutex> lock(scandata_mutex);
    LOG4CXX_TRACE(iLogger::GetLogger(), "Free memory for parsed data (SC = " << scData.use_count() << "; PD = " << scData->parsed_data << ")");
    scData->response.reset();
    scData->parsed_data.reset();
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
    we_option optVal;
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
    we_variant optVal;
    we_option opt;
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
    we_option opt;
    opt = Option(weoTaskStatus);
    SAFE_GET_OPTION_VAL(opt, idata, WI_TSK_RUN);

    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "task::WaitForData synch object ready");
        boost::unique_lock<boost::mutex> lock(tsk_mutex);
        while((taskList.size() == 0 && taskQueue.size() == 0) || idata == WI_TSK_PAUSED) { // (taskList.size() == 0 && taskQueue.size() == 0) ||
            LOG4CXX_DEBUG(iLogger::GetLogger(), "task::WaitForData: go to sleep");
            tsk_event.wait(lock);

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

we_option& task::Option( const string& name )
{
    wOptions::iterator it;

    LOG4CXX_TRACE(iLogger::GetLogger(), "task::Option(" << name << ")");
    if (kernel != NULL) {
        return kernel->Option(name);
    }

    // worst case... :(
    return i_options_provider::empty_option;

}

void task::Option( const string& name, we_variant val )
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
