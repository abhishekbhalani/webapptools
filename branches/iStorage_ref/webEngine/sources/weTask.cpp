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
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/thread.hpp>

namespace webEngine {

void TaskProcessor(Task* tsk)
{
    wOption opt;
    size_t i;
    int iData;
    ResponseList::iterator rIt;
    iResponse* resp;
    iRequest* curr_url;

    LOG4CXX_TRACE(iLogger::GetLogger(), "WeTaskProcessor started for task " << ((void*)&tsk));
    tsk->isRunning = true;
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
                if (curr_url->RequestUrl().IsValid()) {
                    // search for transport
                    for(i = 0; i < tsk->transports.size(); i++)
                    {
                        if (tsk->transports[i]->IsOwnProtocol(curr_url->RequestUrl().protocol)) {
                            resp = tsk->transports[i]->Request(curr_url);
                            resp->ID(curr_url->ID());
                            resp->processor = curr_url->processor;
                            resp->context = curr_url->context;
                            tsk->taskQueue.push_back(resp);
                            LOG4CXX_DEBUG(iLogger::GetLogger(), "WeTaskProcessor: send request to " << curr_url->RequestUrl().ToString());
                            break;
                        }
                    }
                }
                else {
                    // try to send request though appropriate transports
                    LOG4CXX_DEBUG(iLogger::GetLogger(), "WeTaskProcessor: send request to " << curr_url->RequestUrl().ToString());
                    for(i = 0; i < tsk->transports.size(); i++)
                    {
                        resp = tsk->transports[i]->Request(curr_url);
                        resp->ID(curr_url->ID());
                        resp->processor = curr_url->processor;
                        resp->context = curr_url->context;
                        tsk->taskQueue.push_back(resp);
                    }
                }

                string u_req = curr_url->RequestUrl().ToString();
                tsk->taskList.erase(tsk->taskList.begin());
                delete curr_url;
            }
            else {
                break; // no URLs in the waiting list
            }
        }

        // if any requests pending process transport operations
        if (tsk->taskQueue.size() > 0) {
            LOG4CXX_TRACE(iLogger::GetLogger(), "WeTaskProcessor: transport->ProcessRequests()");
            for(i = 0; i < tsk->transports.size(); i++)
            {
                tsk->transports[i]->ProcessRequests();
            }
            boost::this_thread::sleep(boost::posix_time::millisec(500));
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
                            LOG4CXX_DEBUG(iLogger::GetLogger(), "WeTaskProcessor: send response to " << tsk->inventories[i]->GetDesc());
                            tsk->inventories[i]->ProcessResponse(*rIt);
                        }
                    }
                    tsk->taskQueue.erase(rIt);
                    rIt = tsk->taskQueue.begin();
                }
                else {
                    rIt++;
                }
            }
        }
    };
    tsk->isRunning = false;
    LOG4CXX_TRACE(iLogger::GetLogger(), "WeTaskProcessor finished for task " << ((void*)&tsk));
}

Task::Task()
{
    // set the default options
    processThread = false;
    isRunning = false;
    mutex_ptr = (void*)(new boost::mutex);
    event_ptr = (void*)(new boost::condition_variable);
    taskList.clear();
    taskQueue.clear();
    taskListSize = 0;

    transports.clear();
    inventories.clear();
    auditors.clear();
    vulners.clear();

    scanInfo = new ScanInfo;

    LOG4CXX_TRACE(iLogger::GetLogger(), "Task created");
}

Task::Task( Task& cpy )
{
    options = cpy.options;
    LOG4CXX_TRACE(iLogger::GetLogger(), "Task assigned");
}

Task::~Task()
{
    /// @todo Cleanup
}

iResponse* Task::GetRequest( iRequest* req )
{
    /// @todo Implement this!
    LOG4CXX_TRACE(iLogger::GetLogger(), "Task::GetRequest (WeURL)");
    LOG4CXX_ERROR(iLogger::GetLogger(), " *** Not implemented yet! ***");
    return NULL;
}

void Task::GetRequestAsync( iRequest* req )
{
    /// @todo Implement this!
    LOG4CXX_TRACE(iLogger::GetLogger(), "Task::GetRequestAsync");
    processThread = true;
    if (!isRunning) {
        LOG4CXX_DEBUG(iLogger::GetLogger(), "Task::GetRequestAsync: create WeTaskProcessor");
        boost::thread process(TaskProcessor, this);
    }
    // wake-up task_processor
    if (event_ptr != NULL && mutex_ptr != NULL) {
        boost::mutex *mt = (boost::mutex*)mutex_ptr;
        boost::condition_variable *cond = (boost::condition_variable*)event_ptr;
        boost::unique_lock<boost::mutex> lock(*mt);
        taskList.push_back(req);
        taskListSize++;
        LOG4CXX_TRACE(iLogger::GetLogger(), "Task::GetRequestAsync: new task size=" << taskList.size());
        cond->notify_all();
    }
    return;
}

bool Task::IsReady()
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "Task::IsReady - " << processThread);
    return (processThread);
}

void Task::AddTransport( iTransport* plugin )
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "Task::AddTransport");
    for (size_t i = 0; i < transports.size(); i++)
    {
        if (transports[i]->GetID() == plugin->GetID())
        {
            // transport already in list
            LOG4CXX_DEBUG(iLogger::GetLogger(), "Task::AddTransport - transport already in list");
            return;
        }
    }
    transports.push_back(plugin);
    LOG4CXX_TRACE(iLogger::GetLogger(), "Task::AddTransport: added " << plugin->GetDesc());
}

void Task::AddInventory( iInventory* plugin )
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "Task::AddInventory");
    for (size_t i = 0; i < inventories.size(); i++)
    {
        if (inventories[i]->GetID() == plugin->GetID())
        {
            // transport already in list
            LOG4CXX_DEBUG(iLogger::GetLogger(), "Task::AddInventory - inventory already in list");
            return;
        }
    }
    inventories.push_back(plugin);
    LOG4CXX_TRACE(iLogger::GetLogger(), "Task::AddInventory: added " << plugin->GetDesc());
}

void Task::AddAuditor( iAudit* plugin )
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "Task::AddAuditor");
    for (size_t i = 0; i < auditors.size(); i++)
    {
        if (auditors[i]->GetID() == plugin->GetID())
        {
            // transport already in list
            LOG4CXX_DEBUG(iLogger::GetLogger(), "Task::AddAuditor - auditor already in list");
            return;
        }
    }
    auditors.push_back(plugin);
    LOG4CXX_TRACE(iLogger::GetLogger(), "Task::AddAuditor: added " << plugin->GetDesc());
}

void Task::AddVulner( iVulner* plugin )
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "Task::AddVulner");
    for (size_t i = 0; i < vulners.size(); i++)
    {
        if (vulners[i]->GetID() == plugin->GetID())
        {
            // transport already in list
            LOG4CXX_DEBUG(iLogger::GetLogger(), "Task::AddVulner - vulner already in list");
            return;
        }
    }
    vulners.push_back(plugin);
    LOG4CXX_TRACE(iLogger::GetLogger(), "Task::AddVulner: added " << plugin->GetDesc());
}

void Task::StorePlugins(vector<iPlugin*>& plugins)
{
    StringList::iterator trsp;
    StringList ifaces;

    for (size_t i = 0; i < plugins.size(); i++)
    {
        ifaces = plugins[i]->InterfaceList();
        LOG4CXX_TRACE(iLogger::GetLogger(), "Task::StorePlugins - plugin: " << plugins[i]->GetDesc() << " ifaces: " << ifaces.size());

        trsp = find(ifaces.begin(), ifaces.end(), "iTransport");
        if (trsp != ifaces.end())
        {
            LOG4CXX_TRACE(iLogger::GetLogger(), "Task::StorePlugins - found transport: " << plugins[i]->GetDesc());
            AddTransport((iTransport*)plugins[i]);
        }

        trsp = find(ifaces.begin(), ifaces.end(), "iInventory");
        if (trsp != ifaces.end())
        {
            LOG4CXX_TRACE(iLogger::GetLogger(), "Task::StorePlugins - found inventory: " << plugins[i]->GetDesc());
            AddInventory((iInventory*)plugins[i]);
        }

        trsp = find(ifaces.begin(), ifaces.end(), "iAudit");
        if (trsp != ifaces.end())
        {
            LOG4CXX_TRACE(iLogger::GetLogger(), "Task::StorePlugins - found auditor: " << plugins[i]->GetDesc());
            AddAuditor((iAudit*)plugins[i]);
        }

        trsp = find(ifaces.begin(), ifaces.end(), "iVulner");
        if (trsp != ifaces.end())
        {
            LOG4CXX_TRACE(iLogger::GetLogger(), "Task::StorePlugins - found vulner: " << plugins[i]->GetDesc());
            AddVulner((iVulner*)plugins[i]);
        }
    }

}

void Task::Run(void)
{
    processThread = true;
    LOG4CXX_DEBUG(iLogger::GetLogger(), "Task::Run: create WeTaskProcessor");
    boost::thread process(TaskProcessor, this);

    for (size_t i = 0; i < inventories.size(); i++)
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "Task::Run: " << inventories[i]->GetDesc());
        inventories[i]->Start(this);
    }
}

void Task::Pause(const bool& state /*= true*/)
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
        LOG4CXX_TRACE(iLogger::GetLogger(), "Task::Pause notify all about TaskStatus change");
        boost::mutex *mt = (boost::mutex*)mutex_ptr;
        boost::condition_variable *cond = (boost::condition_variable*)event_ptr;
        cond->notify_all();
    }
}

void Task::Stop()
{
    processThread = false;
    taskList.clear();
    taskQueue.clear();
    CalcStatus();
}

void Task::CalcStatus()
{

    size_t count = taskList.size();
    int idata = (taskListSize - count) * 100 / taskListSize;
    LOG4CXX_DEBUG(iLogger::GetLogger(), "Task::CalcStatus: rest " << count << " queries  from " <<
        taskListSize << " (" << (taskListSize - count) << ": " << idata << "%)");
    Option(weoTaskCompletion, idata);
    // set task status
    if (taskList.size() == 0 && taskQueue.size() == 0) {
        LOG4CXX_DEBUG(iLogger::GetLogger(), "Task::CalcStatus: finish!");
        processThread = false;
        Option(weoTaskStatus, WI_TSK_IDLE);
        boost::mutex *mt = (boost::mutex*)mutex_ptr;
        boost::condition_variable *cond = (boost::condition_variable*)event_ptr;
        cond->notify_all();
    }
}

ScanData* Task::GetScanData( const string& baseUrl, const string& realUrl )
{
    ScanData* retval = scanInfo->GetScanData( baseUrl, realUrl );
    if (retval->scanID == "")
    {
        retval->scanID = scanInfo->scanID;
    }
    return retval;
}

void Task::SetScanData( ScanData* scData )
{
    scanInfo->SetScanData(scData);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn RecordSet* Task::ToRS( const string& prefix = "" )
///
/// @brief	Converts this object to an RecordSet. This function realizes alternate serialization
/// 		mechanism. It generates RecordSet with all necessary data. This
/// 		representation is used for internal data exchange (for example to store data through
/// 		iweStorage interface). 
///
/// @param  prefix  - The naming prefix. 
///
/// @retval	This object as a std::string. 
////////////////////////////////////////////////////////////////////////////////////////////////////
RecordSet* Task::ToRS( const string& parentID/* = ""*/ )
{
    RecordSet* res = new RecordSet;
    Record* rec;
    Record* trec;
    wOption optVal;
    string strData;
    string tskId;
    int optCount;
    wOptions::iterator it;

    LOG4CXX_TRACE(iLogger::GetLogger(), "Task::ToRS");

    trec = new Record;
    trec->objectID = weObjTypeTask;
    optVal = Option(weoID);
    SAFE_GET_OPTION_VAL(optVal, tskId, "");
    trec->Option(weoID, optVal.Value());

    trec->Option(weoParentID, parentID);

    optVal = Option(weoName);
    trec->Option(weoName, optVal.Value());

    optVal = Option(weoTaskStatus);
    trec->Option(weoTaskStatus, optVal.Value());

    optVal = Option(weoTaskCompletion);
    trec->Option(weoTaskCompletion, optVal.Value());

    res->push_back(*trec);

    tskId += "_";
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
        rec = new Record;
        rec->objectID = weObjTypeSysOption;
        rec->Option(weoName, strData);
        rec->Option(weoParentID, tskId);
        rec->Option(weoTypeID, optVal.Which());
        rec->Option(weoValue, optVal.Value());
        res->push_back(*rec);
        optCount++;
    }
    trec->Option("options", optCount);

    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn void Task::FromRS( RecordSet *rs )
///
/// @brief  Initializes this object from the given from RecordSet.
/// 		
/// @param  rs	 - RecordSet. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void Task::FromRS( RecordSet *rs  )
{
    Record rec;
    size_t r;
    wOptionVal optVal;
    wOption opt;
    string strData;

    LOG4CXX_TRACE(iLogger::GetLogger(), "Task::FromRS");

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
            SAFE_GET_OPTION_VAL(opt, strData, "");
            // type doesn't matter here
            // opt = rec.Option(weoTypeID);
            opt = rec.Option(weoValue);
            optVal = opt.Value();
            Option(strData, optVal);
        }
    }
}

void Task::WaitForData()
{
    int idata;
    wOption opt;
    opt = Option(weoTaskStatus);
    SAFE_GET_OPTION_VAL(opt, idata, WI_TSK_RUN);

    if (event_ptr != NULL && mutex_ptr != NULL) {
        LOG4CXX_TRACE(iLogger::GetLogger(), "Task::WaitForData synch object ready");
        boost::mutex *mt = (boost::mutex*)mutex_ptr;
        boost::condition_variable *cond = (boost::condition_variable*)event_ptr;
        boost::unique_lock<boost::mutex> lock(*mt);
        while((taskList.size() == 0 && taskQueue.size() == 0) || idata == WI_TSK_PAUSED) {
            LOG4CXX_DEBUG(iLogger::GetLogger(), "Task::WaitForData: go to sleep");
            cond->wait(lock);

            opt = Option(weoTaskStatus);
            SAFE_GET_OPTION_VAL(opt, idata, WI_TSK_RUN);
        }
    }
}

} // namespace webEngine
