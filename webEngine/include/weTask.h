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
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
//#include <boost/any.hpp>
#include "weiBase.h"
#include "weOptions.h"

//using boost::any_cast;
//using namespace boost;
using namespace std;

namespace webEngine {

    // forward declarations
    class i_transport;
    class i_inventory;
    class i_audit;
    class iVulner;
    class i_request;
    class ScanData;
    class ScanInfo;
    class engine_dispatcher;

#define WE_TASK_SIGNAL_NO       0
#define WE_TASK_SIGNAL_RUN      1
#define WE_TASK_SIGNAL_PAUSE    2
#define WE_TASK_SIGNAL_STOP     3
#define WE_TASK_SIGNAL_SUSSPEND 4

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  task
    ///
    /// @brief  Entry point to execute any actions with webEngine
    ///
    /// @author A. Abramov
    /// @date   09.06.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class task : public i_options_provider
    {
    public:
        task(engine_dispatcher *krnl = NULL);
        task(task& cpy);
        ~task();

        // i_options_provider functions
        virtual wOption Option(const string& name);
        virtual void Option(const string& name, wOptionVal val);
        virtual bool IsSet(const string& name);
        virtual void Erase(const string& name);
        virtual void Clear();
        void CopyOptions(i_options_provider* cpy);
        string_list OptionsList();
        size_t OptionSize();

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @fn void AddPlgTransport(i_transport* plugin)
        ///
        /// @brief  Adds a transport plugin to the task object.
        /// @param  plugin   - Plugin with i_transport interface. 
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        void AddPlgTransport(i_transport* plugin);

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @fn void AddPlgInventory(i_inventory* plugin)
        ///
        /// @brief  Adds a inventory plugin to the task object.
        /// @param  plugin   - Plugin with i_inventory interface. 
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        void AddPlgInventory(i_inventory* plugin);

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @fn void AddPlgAuditor(i_audit* plugin)
        ///
        /// @brief  Adds a auditor plugin to the task object.
        /// @param  plugin   - Plugin with i_audit interface. 
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        void AddPlgAuditor(i_audit* plugin);

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @fn void AddPlgVulner(iVulner* plugin)
        ///
        /// @brief  Adds a vulnerability search plugin to the task object.
        /// @param  plugin   - Plugin with iVulner interface. 
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        void AddPlgVulner(iVulner* plugin);

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @fn	void StorePlugins(vector<i_plugin*>& plugins)
        ///
        /// @brief	Adds plugins to the task object. Plugins will be separated to categories and sorted by
        ///         priority.
        ///
        /// @param [in,out]	plugins	 - if non-null, the plugins. 
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        void StorePlugins(vector<i_plugin*>& plugins);

        void Run();
        void Pause(const bool& state = true);
        void Stop();

        bool IsReady();
        virtual i_response* GetRequest(i_request* req);
        virtual void GetRequestAsync(i_request* req);

        db_recordset* ToRS( const string& parentID = "" );
        void FromRS( db_recordset *rs );

        void WaitForData();
        void CalcStatus();

        virtual ScanInfo* GetScan() { return scanInfo; };
        virtual boost::shared_ptr<ScanData> GetScanData(const string& baseUrl);
        virtual void SetScanData(const string& baseUrl, boost::shared_ptr<ScanData> scData);
        virtual void FreeScanData(boost::shared_ptr<ScanData> scData);

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

        virtual int add_thread();
        virtual int remove_thread();

        virtual bool is_url_processed(string url);
        virtual void register_url(string url);

#ifndef __DOXYGEN__
    protected:
        typedef map<string, i_request*> WeRequestMap;
        vector<i_transport*> transports;
        vector<i_inventory*> inventories;
        vector<i_audit*> auditors;
        vector<iVulner*> vulners;
        bool processThread;
        bool isRunning;
        void* mutex_ptr;
        void* event_ptr;
        void* scandata_mutex;
        size_t taskQueueSize;
        size_t taskListSize;
        vector<i_request*> taskList;
        vector<i_response*> taskQueue;
        ScanInfo* scanInfo;
        int thread_count;
        engine_dispatcher *kernel;
        boost::unordered_map<string, int > processed_urls;
#endif //__DOXYGEN__

    private:
        friend void TaskProcessor(task* tsk);
    };

} // namespace webEngine

BOOST_CLASS_TRACKING(webEngine::task, boost::serialization::track_never)

#endif //__WETASK_H__
