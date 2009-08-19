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
#ifndef __WETASK_H__
#define __WETASK_H__

#include <string>
#include <map>
#include <boost/any.hpp>
#include "weiBase.h"
#include "weOptions.h"

using boost::any_cast;
using namespace boost;
using namespace std;

// forward declarations
class iweTransport;
class iweInventory;
class iweAudit;
class iweVulner;
class iweRequest;
class WeScanData;
class WeScan;

#define WE_VERSION_ID   0

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  WeTask
///
/// @brief  Entry point to execute any actions with webEngine
///
/// @author A. Abramov
/// @date   09.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class WeTask : public iweOptionsProvider
{
public:
    WeTask();
    WeTask(WeTask& cpy);
    ~WeTask();

    void AddTransport(iweTransport* plugin);
    void AddInventory(iweInventory* plugin);
    void AddAuditor(iweAudit* plugin);
    void AddVulner(iweVulner* plugin);
    void StorePlugins(vector<iwePlugin*>& plugins);

    void Run();
    void Pause(const bool& state = true);
    void Stop();

    bool IsReady();
    iweResponse* GetRequest(iweRequest* req);
    void GetRequestAsync(iweRequest* req);

    string ToXml( void );
    void FromXml( string input );
    void FromXml( WeTagScanner& sc, int token = -1 );

    void WaitForData();
    void CalcStatus();

    WeScan* GetScan() { return scanInfo; };
    WeScanData* GetScanData(const string& baseUrl, const string& realUrl);
    void SetScanData(WeScanData* scData);

#ifndef __DOXYGEN__
protected:
    typedef map<string, iweRequest*> WeRequestMap;
    vector<iweTransport*> transports;
    vector<iweInventory*> inventories;
    vector<iweAudit*> auditors;
    vector<iweVulner*> vulners;
    bool processThread;
    bool isRunning;
    void* mutex_ptr;
    void* event_ptr;
    size_t taskQueueSize;
    size_t taskListSize;
    vector<iweRequest*> taskList;
    vector<iweResponse*> taskQueue;
    WeScan* scanInfo;
#endif //__DOXYGEN__

private:
    DECLARE_SERIALIZATOR
    {
        ar & BOOST_SERIALIZATION_NVP(options);
    };
    friend void WeTaskProcessor(WeTask* tsk);
};

BOOST_CLASS_TRACKING(WeTask, boost::serialization::track_never)

#endif //__WETASK_H__
