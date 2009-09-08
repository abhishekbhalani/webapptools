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

namespace webEngine {

    // forward declarations
    class iTransport;
    class iInventory;
    class iAudit;
    class iVulner;
    class iRequest;
    class ScanData;
    class ScanInfo;

#define WE_TASK_SIGNAL_NO       0
#define WE_TASK_SIGNAL_RUN      1
#define WE_TASK_SIGNAL_PAUSE    2
#define WE_TASK_SIGNAL_STOP     3
#define WE_TASK_SIGNAL_SUSSPEND 4

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  Task
    ///
    /// @brief  Entry point to execute any actions with webEngine
    ///
    /// @author A. Abramov
    /// @date   09.06.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class Task : public iOptionsProvider
    {
    public:
        Task();
        Task(Task& cpy);
        ~Task();

        void AddTransport(iTransport* plugin);
        void AddInventory(iInventory* plugin);
        void AddAuditor(iAudit* plugin);
        void AddVulner(iVulner* plugin);
        void StorePlugins(vector<iPlugin*>& plugins);

        void Run();
        void Pause(const bool& state = true);
        void Stop();

        bool IsReady();
        iResponse* GetRequest(iRequest* req);
        void GetRequestAsync(iRequest* req);

        RecordSet* ToRS( const string& parentID = "" );
        void FromRS( RecordSet *rs );

        void WaitForData();
        void CalcStatus();

        ScanInfo* GetScan() { return scanInfo; };
        ScanData* GetScanData(const string& baseUrl, const string& realUrl);
        void SetScanData(ScanData* scData);

#ifndef __DOXYGEN__
    protected:
        typedef map<string, iRequest*> WeRequestMap;
        vector<iTransport*> transports;
        vector<iInventory*> inventories;
        vector<iAudit*> auditors;
        vector<iVulner*> vulners;
        bool processThread;
        bool isRunning;
        void* mutex_ptr;
        void* event_ptr;
        size_t taskQueueSize;
        size_t taskListSize;
        vector<iRequest*> taskList;
        vector<iResponse*> taskQueue;
        ScanInfo* scanInfo;
#endif //__DOXYGEN__

    private:
        DECLARE_SERIALIZATOR
        {
            ar & BOOST_SERIALIZATION_NVP(options);
        };
        friend void TaskProcessor(Task* tsk);
    };

} // namespace webEngine

BOOST_CLASS_TRACKING(webEngine::Task, boost::serialization::track_never)

#endif //__WETASK_H__
