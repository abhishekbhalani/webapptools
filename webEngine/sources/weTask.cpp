/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew Abramov aabramov@ptsecurity.ru

    This file is part of webEngine

    webEngine is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
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
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/functional/hash.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/foreach.hpp>

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
    LOG4CXX_TRACE(iLogger::GetLogger(), "WeTaskProcessor started for task " << tsk);
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
                        scan_data_ptr scd = tsk->get_scan_data((*rIt)->BaseUrl().tostring());
                        scd->response = *rIt;
                        if (! (scd->parsed_data)) {
                            scd->parsed_data = parsed_doc;
                        }
                        // 3. Push to processing
                        tsk->set_scan_data((*rIt)->BaseUrl().tostring(), scd);
                    }
                    //(*rIt).reset();
                    rIt = tsk->taskQueue.erase(rIt);
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
    LOG4CXX_TRACE(iLogger::GetLogger(), "WeTaskProcessor finished for task " << tsk);
}

void task_data_process(task* tsk)
{
    scan_data_ptr sc_data;

    LOG4CXX_TRACE(iLogger::GetLogger(), "WeTaskProcessData started for task " << ((void*)&tsk));
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
            /// @todo !!! REMOVE THIS!!! It's only debug! need to implement clever clean-up scheme
            tsk->free_scan_data(sc_data);
        } // if sc_data valid
    } // while sc_process.size > 0
    tsk->ping_tm = btm::second_clock::local_time();
    tsk->remove_thread();
    tsk->dataThread = false;
    LOG4CXX_TRACE(iLogger::GetLogger(), "WeTaskProcessData finished for task " << ((void*)&tsk));
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
    total_reqs = 0;
    total_done = 0;
    profile_id = "0";
    tsk_status = WI_TSK_IDLE;
    tsk_completion = 0;

    parsers.clear();
    transports.clear();
    inventories.clear();
    auditors.clear();
    vulners.clear();

    LOG4CXX_TRACE(iLogger::GetLogger(), "task created");
}

task::task( task& cpy )
{
    kernel = cpy.kernel;
    total_reqs = cpy.total_reqs;
    total_done = cpy.total_done;
    profile_id = cpy.profile_id;
    tsk_status = cpy.tsk_status;
    tsk_completion = cpy.tsk_completion;

    parsers.assign(cpy.parsers.begin(), cpy.parsers.end());
    transports.assign(cpy.transports.begin(), cpy.transports.end());
    inventories.assign(cpy.inventories.begin(), cpy.inventories.end());
    auditors.assign(cpy.auditors.begin(), cpy.auditors.end());
    vulners.assign(cpy.vulners.begin(), cpy.vulners.end());

    LOG4CXX_TRACE(iLogger::GetLogger(), "task assigned");
}

task::~task()
{
    if (IsReady()) {
        Stop();
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
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::get_request_async");
    if (!processThread) {
        processThread = true;
        LOG4CXX_DEBUG(iLogger::GetLogger(), "task::get_request_async: create WeTaskProcessor");
        boost::thread process(task_processor, this);
        tsk_status = WI_TSK_RUN;
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
    bool result;
    int active_threads = LockedGetValue(&thread_count);

    result = !(taskList.size() == 0 && taskQueue.size() == 0 && active_threads == 0);
    result = result || (tsk_status != WI_TSK_IDLE);
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::IsReady - " << result);
    return result;
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
    if (kernel != NULL && kernel->storage() != NULL) {
        scan_id = kernel->storage()->generate_id(weObjTypeScanData);
    }
    else {
        scan_id = "";
    }
    tsk_status = WI_TSK_PAUSED;
    start_tm = btm::second_clock::local_time();
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
    for (size_t i = 0; i < transports.size(); i++)
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "task::Run: initialize " << transports[i]->get_description());
        transports[i]->load_settings(this);
    }
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
    tsk_status = WI_TSK_RUN;
    save_to_db();
    boost::unique_lock<boost::mutex> lock(tsk_mutex);
    tsk_event.notify_all();
}

void task::Pause(const bool& state /*= true*/)
{
//    we_option opt;
//    opt = Option(weoTaskStatus);
//    SAFE_GET_OPTION_VAL(opt, idata, WI_TSK_RUN);
    if (state) {
        tsk_status = WI_TSK_PAUSED;
    }
    else {
        tsk_status = WI_TSK_RUN;
    }

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
    //processThread = false;
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "task::Stop notify all about TaskStatus change");
        boost::unique_lock<boost::mutex> lock(tsk_mutex);
        tsk_event.notify_all();
    }
    // switch context to finalize TaskProcessor
    boost::this_thread::sleep(boost::posix_time::millisec(10));
    taskList.clear();
    taskQueue.clear();
    if (tsk_status == WI_TSK_RUN) {
        calc_status();
    }
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
    tsk_completion = idata;
    // set task status
    if (taskList.size() == 0 && taskQueue.size() == 0 && active_threads == 0) {
        LOG4CXX_DEBUG(iLogger::GetLogger(), "task::calc_status: finish!");
        //processThread = false;
        finish_tm = btm::second_clock::local_time();
        tsk_status = WI_TSK_IDLE;
        save_to_db();
        boost::unique_lock<boost::mutex> lock(tsk_mutex);
        tsk_event.notify_all();
    }
}

std::auto_ptr<db_recordset> task::get_scan()
{
    vector<string> fnames = i_storage::get_namespace_struct(weObjTypeScanData);
    db_recordset* retval = new db_recordset(fnames);

    if (kernel != NULL && kernel->storage() != NULL)
    {
        db_query flt;
        db_condition icond;

        flt.what = fnames;
        icond.field() = weObjTypeScanData "." "task_id";
        icond.operation() = db_condition::equal;
        icond.value() = scan_id;
        flt.where.set(icond);
        kernel->storage()->get(flt, *retval);
    }

    return auto_ptr<db_recordset>(retval);
}

shared_ptr<ScanData> task::get_scan_data( const string& baseUrl )
{
    boost::unique_lock<boost::mutex> lock(scandata_mutex);
    shared_ptr<ScanData> retval;
    ScanData* data;

    data = NULL;
    if (kernel != NULL && kernel->storage() != NULL) {
        db_query flt;
        db_condition ucond;
        db_condition scond;
        db_recordset rs;

        flt.what = i_storage::get_namespace_struct(weObjTypeScanData);
        db_recordset dbres(flt.what);

        ucond.field() = weObjTypeScanData "." "object_url";
        ucond.operation() = db_condition::equal;
        ucond.value() = baseUrl;
        scond.field() = weObjTypeScanData "." "task_id";
        scond.operation() = db_condition::equal;
        scond.value() = scan_id;
        flt.where.set(scond).and(ucond);
        if (kernel->storage()->get(flt, rs) > 0) {
            db_cursor r = rs.begin();
            data = new ScanData;
            if (! data->from_dataset(r) ){
                delete data;
                data = NULL;
            }
        }// if data found
    } // if storage present
    if (data == NULL) {
        // no data found
        data = new ScanData;
        data->object_url = baseUrl;
    } // if no data found
    if (data->scan_id == "")
    {
        data->scan_id = scan_id;
    }
    retval = shared_ptr<ScanData>(data);
    return retval;
}

void task::set_scan_data( const string& baseUrl, boost::shared_ptr<ScanData> scData )
{
    {
        boost::unique_lock<boost::mutex> lock(scandata_mutex);
        if (kernel != NULL && kernel->storage() != NULL) {
            db_query flt;
            db_condition ucond;
            db_condition scond;

            flt.what = i_storage::get_namespace_struct(weObjTypeScanData);
            db_recordset dbres(flt.what);

            ucond.field() = weObjTypeScanData "." "object_url";
            ucond.operation() = db_condition::equal;
            ucond.value() = baseUrl;
            scond.field() = weObjTypeScanData "." "task_id";
            scond.operation() = db_condition::equal;
            scond.value() = scan_id;
            flt.where.set(scond).and(ucond);

            db_cursor r = dbres.push_back();
            if ( scData->to_dataset(r) ) {
                kernel->storage()->set(flt, dbres);
            }

        } // if storage present
        sc_process.push_back(scData);
    }
    if (!dataThread) {
        dataThread = true;
        boost::thread(task_data_process, this);
    }
}

void task::free_scan_data(boost::shared_ptr<ScanData> scData)
{
    boost::unique_lock<boost::mutex> lock(scandata_mutex);
    LOG4CXX_TRACE(iLogger::GetLogger(), "Free memory for parsed data (SC = " << scData.use_count() << "; PD = " << scData->parsed_data << ")");
    scData->response.reset();
    scData->parsed_data.reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn db_recordset* task::save_to_db( )
///
/// @brief	Converts this object to an db_recordset. This function realizes alternate serialization
/// 		mechanism. It generates db_recordset with all necessary data. This
/// 		representation is used for internal data exchange (for example to store data through
/// 		iweStorage interface). 
///
/// @retval	This object as a std::string. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool task::save_to_db( )
{
    bool res = false;

    if (kernel != NULL && kernel->storage() != NULL)
    {
        vector<string> fields = i_storage::get_namespace_struct(weObjTypeTask);
        db_recordset dbres(fields);

        db_cursor rec = dbres.push_back();
        rec[weObjTypeTask "." weoID] = scan_id;
        rec[weObjTypeTask "." weoProfileID] = profile_id;
        rec[weObjTypeTask "." "name"] = task_name;
        rec[weObjTypeTask "." weoTaskCompletion] = tsk_completion;
        rec[weObjTypeTask "." weoTaskStatus] = tsk_status;
        rec[weObjTypeTask "." "start_time"] = boost::lexical_cast<string>(start_tm);
        rec[weObjTypeTask "." "finish_time"] = boost::lexical_cast<string>(finish_tm);
        rec[weObjTypeTask "." "ping_time"] = boost::lexical_cast<string>(ping_tm);
        rec[weObjTypeTask "." "requests"] = (int)total_reqs;
        string urls = "";
        BOOST_FOREACH( we_url_map::value_type i, processed_urls ) {
            urls += i.first;
            urls += "\t";
            urls += boost::lexical_cast<string>(i.second);
            urls += "\x0d";
        }
        rec[weObjTypeTask "." "processed_urls"] = urls;

        // save to storage
        db_query flt;

        db_condition icond;
        icond.field() = weObjTypeTask "." weoID;
        icond.operation() = db_condition::equal;
        icond.value() = scan_id;
        flt.where.set(icond);
        int r = kernel->storage()->set(flt, dbres);
        if (r == 1) {
            // only one record inserted/updated
            res = true;
        }
    }
    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn void task::load_from_db( string& id )
///
/// @brief  Initializes this object from the given from db_recordset.
/// 		
/// @param  id	 - task identifier in database. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool task::load_from_db( string& id  )
{
    bool res = false;

    if (kernel != NULL && kernel->storage() != NULL)
    {
        db_query flt;
        db_condition icond;
        db_recordset dbres;

        flt.what = i_storage::get_namespace_struct(weObjTypeTask);
        icond.field() = weObjTypeTask "." weoID;
        icond.operation() = db_condition::equal;
        icond.value() = scan_id;
        flt.where.set(icond);

        int r = kernel->storage()->set(flt, dbres);
        if (r > 0) {
            res = true;
            db_cursor rec = dbres.begin();
            string data;
            try{
                scan_id = rec[weObjTypeTask "." weoID].get<string>();
                profile_id = rec[weObjTypeTask "." weoProfileID].get<string>();
                task_name = rec[weObjTypeTask "." "name"].get<string>();
                tsk_completion = rec[weObjTypeTask "." weoTaskCompletion].get<int>();
                tsk_status = rec[weObjTypeTask "." weoTaskStatus].get<int>();
                data = rec[weObjTypeTask "." "start_time"].get<string>();
                start_tm = boost::lexical_cast<boost::posix_time::ptime>(data);
                data = rec[weObjTypeTask "." "finish_time"].get<string>();
                finish_tm = boost::lexical_cast<boost::posix_time::ptime>(data);
                data = rec[weObjTypeTask "." "ping_time"].get<string>();
                ping_tm = boost::lexical_cast<boost::posix_time::ptime>(data);
                total_reqs = rec[weObjTypeTask "." "requests"].get<int>();
                data = rec[weObjTypeTask "." "processed_urls"].get<string>();
                vector<string> urls;
                boost::split(urls, data, is_any_of("\x0d"));
                processed_urls.clear();
                for(size_t i = 0; i < urls.size(); i++) {
                    string u = urls[i];
                    string n = "0";
                    size_t pos = u.find('\t');
                    int nl = 0;
                    if(pos != string::npos) {
                        n = u.substr(pos+1);
                        u = u.substr(0, pos);
                        nl = boost::lexical_cast<int>(n);
                    }
                    processed_urls[u] = nl;
                } // foreach url
            }
            catch (std::exception &e) {
                LOG4CXX_ERROR(iLogger::GetLogger(), "task::load_from_db exception: " << e.what());
                // may be out_of_range or bad_cast
                res = false;
            } // try/catch
        } // if data found
    } // if storage exist
    return res;
}

void task::WaitForData()
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::WaitForData synch object ready");
    boost::unique_lock<boost::mutex> lock(tsk_mutex);
    while((taskList.size() == 0 && taskQueue.size() == 0 && tsk_status == WI_TSK_RUN) || tsk_status == WI_TSK_PAUSED) { // (taskList.size() == 0 && taskQueue.size() == 0) ||
        LOG4CXX_DEBUG(iLogger::GetLogger(), "task::WaitForData: go to sleep");
        tsk_event.wait(lock);
    }
}

void task::add_vulner( const string& vId, const string& params, const string& parentId, int vLevel/* = -1*/ )
{
    LOG4CXX_INFO(iLogger::GetLogger(), "task::add_vulner add vulner ID=" << vId << "; ParentID=" << parentId);
    LOG4CXX_DEBUG(iLogger::GetLogger(), "task::add_vulner data: " << params);
    string plg_id, v_id, id;
    size_t pos;

    plg_id = "";
    pos = vId.find('/');
    if (pos != string::npos) {
        plg_id = vId.substr(0, pos);
        v_id = vId.substr(pos + 1);
    }
    else {
        v_id = vId;
    }
    if (kernel != NULL && kernel->storage() != NULL) {
        id = kernel->storage()->generate_id(weObjTypeVulner);

        vector<string> fields = i_storage::get_namespace_struct(weObjTypeVulner);
        db_recordset dbres(fields);

        db_cursor rec = dbres.push_back();
        rec[weObjTypeVulner "." "id"] = id;
        rec[weObjTypeVulner "." "task_id"] = scan_id;
        rec[weObjTypeVulner "." "object_id"] = parentId;
        rec[weObjTypeVulner "." "plugin_id"] = plg_id;
        rec[weObjTypeVulner "." "severity"] = vLevel;
        rec[weObjTypeVulner "." "vulner_id"] = v_id;
        rec[weObjTypeVulner "." "params"] = params;

        db_query flt;
        db_condition icond;
        db_condition tcond;

        icond.field() = weObjTypeVulner "." "id";
        icond.operation() = db_condition::equal;
        icond.value() = id;

        tcond.field() = weObjTypeVulner "." "task_id";
        tcond.operation() = db_condition::equal;
        tcond.value() = scan_id;

        flt.what = fields;
        flt.where.set(icond).and(tcond);

        kernel->storage()->set(flt, dbres);
    }

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

we_option task::Option( const string& name )
{
    db_recordset res;
    db_query flt;
    we_option opt;
    char c;
    int i;
    bool b;
    double d;
    string s;

    LOG4CXX_TRACE(iLogger::GetLogger(), "task::Option(" << name << ")");
    opt = i_options_provider::empty_option;
    if (kernel != NULL && kernel->storage() != NULL) {
        flt.what.clear();
        flt.what.push_back(weObjTypeProfile "." weoTypeID);
        flt.what.push_back(weObjTypeProfile "." weoValue);

        db_condition p_cond;
        db_condition n_cond;

        p_cond.field() = weObjTypeProfile "." weoProfileID;
        p_cond.operation() = db_condition::equal;
        p_cond.value() = profile_id;

        n_cond.field() = weObjTypeProfile "." weoName;
        n_cond.operation() = db_condition::equal;
        n_cond.value() = name;

        flt.where.set(p_cond).and(n_cond);
        kernel->storage()->get(flt, res);

        if (res.size() > 0) {
            db_cursor rec = res.begin();
            try{
                if (!rec[1].empty()) {
                    int tp = boost::lexical_cast<int>(rec[0]);
                    opt.name(name);
                    switch(tp)
                    {
                    case 0: // char
                        c = rec[weObjTypeProfile "." weoValue].get<char>();
                        opt.SetValue(c);
                        break;
                    case 1: // int
                        i = rec[weObjTypeProfile "." weoValue].get<int>();
                        opt.SetValue(i);
                        break;
                    case 2: // bool
                        b = rec[weObjTypeProfile "." weoValue].get<bool>();
                        opt.SetValue(b);
                        break;
                    case 3: // double
                        d = rec[weObjTypeProfile "." weoValue].get<double>();
                        opt.SetValue(d);
                        break;
                    case 4: // string
                        s = rec[weObjTypeProfile "." weoValue].get<string>();
                        opt.SetValue(s);
                        break;
                    default:
                        opt.SetValue(boost::blank());
                    }
                } // if result not <empty>
            }
            catch(std::exception &e) {
                opt = i_options_provider::empty_option;
                LOG4CXX_ERROR(iLogger::GetLogger(), "engine_dispatcher::Option(" << name << ") can't get option value: " << e.what());
            }
        }
    }

    return opt;

}

void task::Option( const string& name, we_variant val )
{
    db_query flt;

    if (kernel != NULL && kernel->storage() != NULL) {
        flt.what.clear();
        flt.what.push_back(weObjTypeProfile "." weoProfileID);
        flt.what.push_back(weObjTypeProfile "." weoName);
        flt.what.push_back(weObjTypeProfile "." weoTypeID);
        flt.what.push_back(weObjTypeProfile "." weoValue);

        db_condition p_cond;
        db_condition n_cond;

        p_cond.field() = weObjTypeProfile "." weoProfileID;
        p_cond.operation() = db_condition::equal;
        p_cond.value() = profile_id;

        n_cond.field() = weObjTypeProfile "." weoName;
        n_cond.operation() = db_condition::equal;
        n_cond.value() = name;

        flt.where.set(p_cond).and(n_cond);

        db_recordset data(flt.what);
        db_cursor rec = data.push_back();

        rec[0] = profile_id;
        rec[1] = name;
        rec[2] = val.which();
        rec[3] = val;

        kernel->storage()->set(flt, data);
    }
}

bool task::IsSet( const string& name )
{
    bool retval = false;
    db_recordset res;
    db_query flt;

    if (kernel != NULL && kernel->storage() != NULL) {
        flt.what.clear();
        flt.what.push_back(weObjTypeProfile "." weoTypeID);
        flt.what.push_back(weObjTypeProfile "." weoValue);

        db_condition p_cond;
        db_condition n_cond;

        p_cond.field() = weObjTypeProfile "." weoProfileID;
        p_cond.operation() = db_condition::equal;
        p_cond.value() = profile_id;

        n_cond.field() = weObjTypeProfile "." weoName;
        n_cond.operation() = db_condition::equal;
        n_cond.value() = name;

        flt.where.set(p_cond).and(n_cond);
        kernel->storage()->get(flt, res);

        if (res.size() > 0) {
            db_cursor rec = res.begin();
            try{
                if (!rec[1].empty()) {
                    int tp = boost::lexical_cast<int>(rec[0]);
                    if ( tp == 2) {
                        // we_variant::which(bool)
                        retval = boost::lexical_cast<bool>(rec[1]);
                    }
                    else {
                        retval = true;
                    }
                } // if result not <empty>
            }
            catch(bad_cast &e) {
                retval = false;
                LOG4CXX_ERROR(iLogger::GetLogger(), "engine_dispatcher::IsSet(" << name << ") can't get option value: " << e.what());
            }
        } // if result size > 0
    }
    LOG4CXX_TRACE(iLogger::GetLogger(), "task::IsSet(" << name << ") value=" << retval);
    return retval;
}

void task::Erase( const string& name )
{
    if (kernel != NULL && kernel->storage() != NULL) {
        db_filter filter;
        db_condition p_cond;
        db_condition n_cond;

        p_cond.field() = weObjTypeProfile "." weoProfileID;
        p_cond.operation() = db_condition::equal;
        p_cond.value() = profile_id;

        n_cond.field() = weObjTypeProfile "." weoName;
        n_cond.operation() = db_condition::equal;
        n_cond.value() = name;

        filter.set(p_cond).and(n_cond);
        LOG4CXX_TRACE(iLogger::GetLogger(), "engine_dispatcher::Erase - " << filter.tostring());
        kernel->storage()->del(filter);
    }
}

void task::Clear()
{
    if (kernel != NULL && kernel->storage() != NULL) {
        string_list opt_names;
        opt_names = OptionsList();
        for (size_t i = 0; i < opt_names.size(); i++) {
            Erase(opt_names[i]);
        }
    }
}

// void task::CopyOptions( i_options_provider* cpy )
// {
//     if (kernel != NULL) {
//         kernel->CopyOptions(cpy);
//     }
// }
// 
string_list task::OptionsList()
{
    string_list retval;
    db_recordset res;
    db_query flt;
    string name;

    if (kernel != NULL && kernel->storage() != NULL) {
        flt.what.clear();
        flt.what.push_back(weObjTypeProfile "." weoName);
        db_condition p_cond;

        p_cond.field() = weObjTypeProfile "." weoProfileID;
        p_cond.operation() = db_condition::equal;
        p_cond.value() = profile_id;

        flt.where.set(p_cond);
        kernel->storage()->get(flt, res);

        db_cursor rec = res.begin();
        while(rec != res.end()) {
            name = rec[0].get<string>();
            if (name != "") {
                retval.push_back(name);
            } // if name present
            ++rec;
        } // foreach record
    }

    return retval;
}

size_t task::OptionSize()
{
    int retval = 0;
    db_recordset res;
    db_query flt;

    if (kernel != NULL && kernel->storage() != NULL) {
        flt.what.clear();
        flt.what.push_back(weObjTypeProfile "." weoName);
        db_condition p_cond;

        p_cond.field() = weObjTypeProfile "." weoProfileID;
        p_cond.operation() = db_condition::equal;
        p_cond.value() = profile_id;

        flt.where.set(p_cond);
        kernel->storage()->get(flt, res);
        retval = res.size();
    }

    return retval;
}

bool task::is_url_processed( string& url )
{
    we_url_map::iterator mit = processed_urls.find(url);
    return (mit != processed_urls.end());
}

void task::register_url( string& url )
{
    we_url_map::iterator mit = processed_urls.find(url);

    if (mit == processed_urls.end()) {
        processed_urls[url] = 1;
    }
    else {
        processed_urls[url]++;
    }
}

} // namespace webEngine
