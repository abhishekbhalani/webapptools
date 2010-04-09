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
//#include <boost/any.hpp>
#include "weiBase.h"
#include "weOptions.h"

//using boost::any_cast;
//using namespace boost;
using namespace std;

namespace webEngine {

    // forward declarations
    class iTransport;
    class i_inventory;
    class i_audit;
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

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @fn void AddPlgTransport(iTransport* plugin)
        ///
        /// @brief  Adds a transport plugin to the Task object.
        /// @param  plugin   - Plugin with iTransport interface. 
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        void AddPlgTransport(iTransport* plugin);

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @fn void AddPlgInventory(i_inventory* plugin)
        ///
        /// @brief  Adds a inventory plugin to the Task object.
        /// @param  plugin   - Plugin with i_inventory interface. 
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        void AddPlgInventory(i_inventory* plugin);

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @fn void AddPlgAuditor(i_inventory* plugin)
        ///
        /// @brief  Adds a auditor plugin to the Task object.
        /// @param  plugin   - Plugin with i_audit interface. 
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        void AddPlgAuditor(i_audit* plugin);

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @fn void AddPlgVulner(i_inventory* plugin)
        ///
        /// @brief  Adds a vulnerability search plugin to the Task object.
        /// @param  plugin   - Plugin with iVulner interface. 
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        void AddPlgVulner(iVulner* plugin);

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @fn	void StorePlugins(vector<i_plugin*>& plugins)
        ///
        /// @brief	Adds plugins to the Task object. Plugins wil br separeted to categories and sorted by
        ///         priority.
        ///
        /// @param [in,out]	plugins	 - if non-null, the plugins. 
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        void StorePlugins(vector<i_plugin*>& plugins);

        void Run();
        void Pause(const bool& state = true);
        void Stop();

        bool IsReady();
        virtual iResponse* GetRequest(iRequest* req);
        virtual void GetRequestAsync(iRequest* req);

        db_recordset* ToRS( const string& parentID = "" );
        void FromRS( db_recordset *rs );

        void WaitForData();
        void CalcStatus();

        virtual ScanInfo* GetScan() { return scanInfo; };
        virtual ScanData* GetScanData(const string& baseUrl, const string& realUrl);
        virtual void SetScanData(ScanData* scData);

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @fn virtual void AddVulner(const string& vId,
        /// 	const string& params,
        /// 	const string& parentId, int vLevel = -1)
        ///
        /// @brief  Adds a vulnerability to the scan results.
        ///
        /// This function may be reimplemented in the descendant class to implement specific features.
        /// First parameter points the vulnerability description in the database to obtain visualization
        /// information and rules. Second parameter describes vulnerability details in XML. This information
        /// may be used to build reports through XSL transformation. parentId points to the ScanData object
        /// containing this vulnerability. And the last parameter allows to control the vulnerability level.
        /// Default value (-1) doesn't affect the severity level which stored in the vulnerability
        /// description. Any other value will overwrite the severity level.
        ///
        /// @param  vId      - vulnerability identifier.
        /// @param  params   - Additional information about vulnerability in XML.
        /// @param  parentId - Identifier of the parent object (ScanData). 
        /// @param  vLevel   - Control the vulnerability level. 
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void AddVulner(const string& vId, const string& params, const string& parentId, int vLevel = -1);

#ifndef __DOXYGEN__
    protected:
        typedef map<string, iRequest*> WeRequestMap;
        vector<iTransport*> transports;
        vector<i_inventory*> inventories;
        vector<i_audit*> auditors;
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
