#ifndef WE_I_TASK__H__2011_04_06
#define WE_I_TASK__H__2011_04_06
/*******************************************************************************
COPYRIGHT    :   Positive Technologies, 2011

PROGRAMMED BY:   Stanislav Antonov

VERSION INFO :   $HeadURL: svn://10.111.111.10/mp8/WebEngine/trunk/webEngine/include/weiTask.h $
                 $Revision: 43158 $
                 $Date: 2011-04-19 11:48:41 +0400 (Вт, 19 апр 2011) $
                 $Author: santonov $
*******************************************************************************/

#include "weOptions.h"
#include <boost/preprocessor.hpp>

namespace webEngine {
//////////////////////////////////////////////////////////////////////////
// Task statuses
//////////////////////////////////////////////////////////////////////////
#define SEQ_TASK_STATUSES (WI_TSK_INIT)(WI_TSK_RUNNING)(WI_TSK_PAUSING)(WI_TSK_PAUSED)(WI_TSK_SUSPENDING)(WI_TSK_FINISHED)(WI_TSK_STOPPED)(WI_TSK_SUSPENDED)
enum TaskStatus { BOOST_PP_SEQ_ENUM(SEQ_TASK_STATUSES) };

class i_task: public i_options_provider {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn	void Run(const string &task_id = string())
    ///
    /// @brief	Runs task. If task_id given, trying to load the task saved by Suspend()
    ///
    /// @param [in]	task_id	 - id of saved task to load
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void Run(const string &task_id = string()) = 0;

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
    virtual TaskStatus Pause(bool wait = true) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn	void Resume()
    ///
    /// @brief	Resuming paused or suspended task.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void Resume() = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn	bool Suspend()
    ///
    /// @brief	Pausing and save current task data. You can resume task from this state.
    ///
    /// @param [out] bool - returns true if successfull
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual bool Suspend() = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn bool Stop()
    ///
    /// @brief	Pausing and stop task. You can NOT resume task from this state.
    ///
    /// @param [out] bool - returns true if successfull
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual bool Stop() = 0;

    virtual TaskStatus status() const = 0;
    virtual bool IsReady() = 0;
    virtual int completion() const = 0;

    virtual size_t total_requests() const = 0;
    virtual size_t total_processed() const = 0;
    virtual void set_profile_id(const string& id) = 0;
    virtual void store_plugins(const string& plugins) = 0;

};
}










#endif //WE_I_TASK__H__2011_04_06


