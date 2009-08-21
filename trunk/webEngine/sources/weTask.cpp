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
/// @fn std::string Task::ToXml( void )
///
/// @brief  Converts this object to an XML.
///
/// This function realizes alternate serialization mechanism. It generates more compact and
/// simplified XML representation. This representation is used for internal data exchange
/// (for example to store data through iweStorage interface).
///
/// @retval This object as a std::string.
////////////////////////////////////////////////////////////////////////////////////////////////////
std::string Task::ToXml( void )
{
    string retval;
    string optList;
    int optCount;
    int optType;
    wOption optVal;
    string strData;
    int    intData;
    unsigned int uintData;
    char   chData;
    unsigned char uchData;
    long    longData;
    unsigned long ulongData;
    bool    boolData;
    double  doubleData;
    wOptions::iterator it;

    retval = "";

    LOG4CXX_TRACE(iLogger::GetLogger(), "Task::ToXml");
    optVal = Option(weoID);
    SAFE_GET_OPTION_VAL(optVal, strData, "");
    retval += "<task id='" + ScreenXML(strData) + "'>\n";

    optVal = Option(weoName);
    SAFE_GET_OPTION_VAL(optVal, strData, "");
    retval += "  <name>" + ScreenXML(strData) + "</name>\n";

    optVal = Option(weoTaskStatus);
    SAFE_GET_OPTION_VAL(optVal, intData, 0);
    retval += "  <status>" + boost::lexical_cast<string>(intData) + "</status>\n";

    optVal = Option(weoTaskCompletion);
    SAFE_GET_OPTION_VAL(optVal, intData, 0);
    retval += "  <completion>" + boost::lexical_cast<string>(intData) + "</completion>\n";

    optCount = 0;
    optList = "";
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
        optType = it->second->Which();
        try
        {
            switch(optType)
            {
            case 0:
                it->second->GetValue(chData);
                strData = boost::lexical_cast<string>(chData);
                break;
            case 1:
                it->second->GetValue(uchData);
                strData = boost::lexical_cast<string>(uchData);
                break;
            case 2:
                it->second->GetValue(intData);
                strData = boost::lexical_cast<string>(intData);
                break;
            case 3:
                it->second->GetValue(uintData);
                strData = boost::lexical_cast<string>(uintData);
                break;
            case 4:
                it->second->GetValue(longData);
                strData = boost::lexical_cast<string>(longData);
                break;
            case 5:
                it->second->GetValue(ulongData);
                strData = boost::lexical_cast<string>(ulongData);
                break;
            case 6:
                it->second->GetValue(boolData);
                strData = boost::lexical_cast<string>(boolData);
                break;
            case 7:
                it->second->GetValue(doubleData);
                strData = boost::lexical_cast<string>(doubleData);
                break;
            case 8:
                it->second->GetValue(strData);
                break;
            default:
                //optVal = *(it->second);
                strData = "";
            }
        }
        catch (...)
        {
        	strData = "";
        }
        optCount++;
        strData = ScreenXML(strData);
        optList += "    <option name='" + it->first + "' type='" + boost::lexical_cast<string>(optType) + "'>" + strData + "</option>\n";
    }
    if (optCount > 0)
    {
        retval += "  <options count='" + boost::lexical_cast<string>(optCount) + "'>\n";
        retval += optList;
        retval += "  </options>\n";
    }

    retval += "</task>\n";
    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn void Task::FromXml( string input )
///
/// @brief  Initializes this object from the given from XML.
///
/// This function reconstructs object back from the XML generated by the @b ToXml function
///
/// @param  input - The input XML.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Task::FromXml( string input )
{
    StrStream st(input.c_str());
    TagScanner sc(st);

    LOG4CXX_TRACE(iLogger::GetLogger(), "Task::FromXml - string");
    FromXml(sc);
}

void Task::FromXml( TagScanner& sc, int token /*= -1*/ )
{
    int pos;
    int parseLevel = 0;
    int intData;
    bool inParsing = true;
    string name, val, dat;

    LOG4CXX_TRACE(iLogger::GetLogger(), "Task::FromXml - TagScanner");
    while (inParsing)
    {
        pos = sc.GetPos();
        if (token == -1) {
            token = sc.GetToken();
        }
        switch(token)
        {
        case wstError:
            LOG4CXX_WARN(iLogger::GetLogger(), "Task::FromXml parsing error");
            inParsing = false;
            break;
        case wstEof:
            LOG4CXX_TRACE(iLogger::GetLogger(), "Task::FromXml - EOF");
            inParsing = false;
            break;
        case wstTagStart:
            name = sc.GetTagName();
            if (parseLevel == 0)
            {
                if (iequals(name, "task"))
                {
                    parseLevel = 1;
                    dat = "";
                }
                else {
                    LOG4CXX_WARN(iLogger::GetLogger(), "Task::FromXml unexpected tagStart: " << name);
                    inParsing = false;
                }
                break;
            }
            if (parseLevel == 1)
            {
                parseLevel = 2;
                dat = "";
                if (iequals(name, "options"))
                {
                    iOptionsProvider::FromXml(sc, token);
                    parseLevel = 1;
                }
                break;
            }
            LOG4CXX_WARN(iLogger::GetLogger(), "Task::FromXml unexpected tagStart: " << name);
            inParsing = false;
        	break;
        case wstTagEnd:
            name = sc.GetTagName();
            if (parseLevel == 1)
            {
                if (iequals(name, "task"))
                {
                    parseLevel = 0;
                    dat = "";
                    inParsing = false;
                }
                else {
                    LOG4CXX_WARN(iLogger::GetLogger(), "Task::FromXml unexpected wstTagEnd: " << name);
                    inParsing = false;
                }
            }
            if (parseLevel == 2)
            {
                dat = UnscreenXML(dat);
                if (iequals(name, "name"))
                {
                    Option(weoName, dat);
                }
                if (iequals(name, "status"))
                {
                    intData = boost::lexical_cast<int>(dat);
                    Option(weoTaskStatus, intData);
                }
                if (iequals(name, "completion"))
                {
                    intData = boost::lexical_cast<int>(dat);
                    Option(weoTaskCompletion, intData);
                }
                parseLevel = 1;
            }
            break;
        case wstAttr:
            name = sc.GetAttrName();
            val = sc.GetValue();
            val = UnscreenXML(val);
            if (parseLevel == 1)
            {
                if (iequals(name, "id"))
                {
                    Option(weoID, val);
                }
            }
            break;
        case wstWord:
        case wstSpace:
            dat += sc.GetValue();
            break;
        default:
            break;
        }
        token = -1;
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
