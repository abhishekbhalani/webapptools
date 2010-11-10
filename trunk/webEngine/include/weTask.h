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
#include <boost/preprocessor.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <weiBase.h>
#include <weOptions.h>
#include <weiTransport.h>

using namespace std;

namespace webEngine {

// forward declarations
class i_transport;
class i_inventory;
class i_audit;
class i_vulner;
class i_request;
class i_parser;
class ScanData;
class ScanInfo;
class engine_dispatcher;
class db_cursor;

#define WE_TASK_SIGNAL_NO       0
#define WE_TASK_SIGNAL_RUN      1
#define WE_TASK_SIGNAL_PAUSE    2
#define WE_TASK_SIGNAL_STOP     3
#define WE_TASK_SIGNAL_SUSSPEND 4

//////////////////////////////////////////////////////////////////////////
// Task statuses
//////////////////////////////////////////////////////////////////////////

#define SEQ_TASK_STATUSES (WI_TSK_INIT)(WI_TSK_RUNNING)(WI_TSK_PAUSING)(WI_TSK_PAUSED)(WI_TSK_SUSPENDING)(WI_TSK_FINISHED)(WI_TSK_STOPPED)(WI_TSK_SUSPENDED)
enum TaskStatus { BOOST_PP_SEQ_ENUM(SEQ_TASK_STATUSES) };

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  task
///
/// @brief  Entry point to execute any actions with webEngine
///
/// @author A. Abramov
/// @date   09.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class task : public i_options_provider {
public:
    task(engine_dispatcher *krnl = NULL);
    task(task& cpy);
    ~task();

#ifdef _DEBUG
    inline size_t get_taskQueue_size() const {
        return taskQueue.size();
    }
    inline size_t get_taskList_size() const {
        return taskList.size();
    }
    inline bool get_processThread() const {
        return processThread;
    }
    const char * get_status_name() const;
#endif

    // i_options_provider functions
    virtual we_option Option(const string& name);
    virtual void Option(const string& name, we_variant val);
    virtual bool IsSet(const string& name);
    virtual void Erase(const string& name);
    virtual void Clear();
    // void CopyOptions(i_options_provider* cpy);
    string_list OptionsList();
    size_t OptionSize();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn void add_plg_parser(i_parser* plugin)
    ///
    /// @brief  Adds a parser plugin to the task object.
    /// @param  plugin   - Plugin with i_parser interface.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void add_plg_parser(i_parser* plugin);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn void add_plg_transport(i_transport* plugin)
    ///
    /// @brief  Adds a transport plugin to the task object.
    /// @param  plugin   - Plugin with i_transport interface.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void add_plg_transport(i_transport* plugin);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn void add_plg_inventory(i_inventory* plugin)
    ///
    /// @brief  Adds a inventory plugin to the task object.
    /// @param  plugin   - Plugin with i_inventory interface.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void add_plg_inventory(i_inventory* plugin);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn void add_plg_auditor(i_audit* plugin)
    ///
    /// @brief  Adds a auditor plugin to the task object.
    /// @param  plugin   - Plugin with i_audit interface.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void add_plg_auditor(i_audit* plugin);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn void add_plg_vulner(i_vulner* plugin)
    ///
    /// @brief  Adds a vulnerability search plugin to the task object.
    /// @param  plugin   - Plugin with i_vulner interface.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void add_plg_vulner(i_vulner* plugin);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn	void store_plugins(vector<i_plugin*>& plugins)
    ///
    /// @brief	Adds plugins to the task object. Plugins will be separated to categories and sorted by
    ///         priority.
    ///
    /// @param [in,out]	plugins	 - if non-null, the plugins.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void store_plugins(vector<i_plugin*>& plugins);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn	i_plugin* get_active_plugin(const string& iface_name, bool autoload = false)
    ///
    /// @brief	Returns pointer to the existing plugin instance. If no plugin with such interface name
    ///         loaded, may be attempt to load this plugin on depends of @b autoload parameter.
    ///         Function returns NULL if automatic loading isn't allowed or no such plugin.
    ///
    /// @param [in]	iface_name	 - requested interface name.
    /// @param [in]	autoload	 - perform automatic loading or not.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    i_plugin* get_active_plugin(const string& iface_name, bool autoload = false);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn	void Run(const string &task_id = string())
    ///
    /// @brief	Runs task. If task_id given, trying to load the task saved by Suspend()
    ///
    /// @param [in]	task_id	 - id of saved task to load
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void Run(const string &task_id = string());

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn	TaskStatus Pause(bool wait = true)
    ///
    /// @brief	Pausing task. It sets 'Pausing' status and waits while task status changed. After task is
    ///         'Paused' you can Resume, Suspend, Stop task, or just delete this. Pause() do not saving
    ///         current task data, if you want to save it you can use Suspend() function.
    ///
    /// @param [in]	wait    - if it false Pause returns in 'Pausing' state, and task continue 'pausing'
    /// @param [out] TaskStatus - returns current task state
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TaskStatus Pause(bool wait = true);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn	void Resume()
    ///
    /// @brief	Resuming paused or suspended task.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void Resume();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn	bool Suspend()
    ///
    /// @brief	Pausing and save current task data. You can resume task from this state.
    ///
    /// @param [out] bool - returns true if successfull
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Suspend();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn bool Stop()
    ///
    /// @brief	Pausing and stop task. You can NOT resume task from this state.
    ///
    /// @param [out] bool - returns true if successfull
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Stop();

    bool IsReady();
    virtual i_response_ptr get_request(i_request_ptr req);
    virtual void get_request_async(i_request_ptr req);

    bool save_to_db( );
    bool load_from_db( string& id );

    void WaitForData();

    virtual webEngine::db_cursor get_scan();
    virtual int get_scan_size();
    virtual boost::shared_ptr<ScanData> get_scan_data(const string& baseUrl);
    virtual void set_scan_data(const string& baseUrl, boost::shared_ptr<ScanData> scData);
    virtual void free_scan_data(boost::shared_ptr<ScanData> scData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual void add_vulner(const string& vId,
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
    virtual void add_vulner(const string& vId, const string& params, const string& parentId, int vLevel = -1);

    virtual int add_thread();
    virtual int remove_thread();

    virtual bool is_url_processed(string& url);
    virtual void register_url(string& url);
    size_t total_requests() const {
        return total_reqs;
    }
    size_t total_processed() const {
        return total_done;
    }

    const string& get_profile_id() const {
        return profile_id;
    }
    const string& get_scan_id() const {
        return scan_id;
    }
    void set_profile_id(const string& id) {
        profile_id = id;
    }
    const string& get_name() const {
        return task_name;
    }
    void set_name(const string& name) {
        task_name = name;
    }
    TaskStatus status() const {
        return tsk_status;
    }
    int completion() const {
        return tsk_completion;
    }
    const boost::posix_time::ptime& start_time() const {
        return start_tm;
    }
    const boost::posix_time::ptime& finish_time() const {
        return finish_tm;
    }
    const boost::posix_time::ptime& ping_time() const {
        return ping_tm;
    }
    boost::posix_time::ptime& ping() {
        return ping_tm = boost::posix_time::second_clock::local_time();
    }

#ifndef __DOXYGEN__
protected:
    typedef map< string, i_request* > WeRequestMap;
    typedef boost::unordered_map< string, int > we_url_map;

    volatile TaskStatus tsk_status;
    int tsk_completion;
    string profile_id;
    string scan_id;
    string task_name;
    boost::posix_time::ptime start_tm;
    boost::posix_time::ptime finish_tm;
    boost::posix_time::ptime ping_tm;
    vector<i_parser*> parsers;
    vector<i_transport*> transports;
    vector<i_inventory*> inventories;
    vector<i_audit*> auditors;
    vector<i_vulner*> vulners;
    vector< boost::shared_ptr<ScanData> > sc_process;
    bool dataThread;
    bool processThread;
    boost::mutex tsk_mutex;
    boost::condition_variable tsk_event;
    boost::mutex scandata_mutex;
    size_t taskQueueSize;
    vector<i_request_ptr> taskList;
    vector< boost::shared_ptr<i_response> > taskQueue;
    int thread_count;
    size_t total_reqs;
    size_t total_done;
    engine_dispatcher *kernel;
    we_url_map processed_urls;
    bool need_lock;
#endif //__DOXYGEN__

private:
    friend void task_processor(task* tsk);
    friend void task_data_process(task* tsk);

    /// wake-up task_processor
    void check_task_processor();

    void save_task_list_and_params();
    void load_task_list();
    void load_task_params();

    void calc_status();
};

} // namespace webEngine

BOOST_CLASS_TRACKING(webEngine::task, boost::serialization::track_never)

/// base URL for processing
#define weoBaseURL           "base_url"
/// number of parallel requests to transport (integer)
#define weoParallelReq       "parallel_req"
/// hostname for scanning
#define weoScanHost          "scan_host"

#endif //__WETASK_H__
