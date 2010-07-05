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
#pragma once
#ifndef __WEIAUDIT_H__
#define __WEIAUDIT_H__

#include <boost/shared_ptr.hpp>
#include <weiPlugin.h>
#include <weiTransport.h>

//using namespace boost;

namespace webEngine {

// forward declarations
class task;
class ScanData;
class i_response;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @interface  i_audit
///
/// @brief  Interface for the audit plugins. 
///
/// @author A. Abramov
/// @date   19.08.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class i_audit :
    public i_plugin
{
public:
    i_audit(engine_dispatcher* krnl, void* handle = NULL);
    virtual ~i_audit(void);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn void init(task* tsk)
    ///
    /// @brief  Initialize plugin for given task. 
    ///
    /// @param  tsk	   - If non-null, the pointer to task what handles the process. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void init(task* tsk) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn void process(task* tsk, boost::shared_ptr<ScanData> scData)
    ///
    /// @brief  Starts the audit process for given ScanData object. 
    ///
    /// @param  tsk	   - If non-null, the pointer to task what handles the process. 
    /// @param  scData - If non-null, the pointer to scan data what contains values to audit. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void process(task* tsk, boost::shared_ptr<ScanData> scData) = 0;
    virtual void pause(task* tsk, bool paused = true) {}
    virtual void stop(task* tsk) {}

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn void process_response(i_response_ptr resp)
    ///
    /// @brief  Process the transport response described by resp.
    /// 		
    /// @param  resp - If non-null, the resp. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void process_response(i_response_ptr resp) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn static void response_dispatcher(i_response_ptr resp, void* context)
    ///
    /// @brief  Response dispatcher. Sends the response to process into the appropriate object pointed
    ///         by the context
    ///
    /// @param  resp	 - If non-null, the resp. 
    /// @param  context	 - If non-null, the context. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    static void response_dispatcher(i_response_ptr resp, void* context);

protected:
    task* parent_task;
};

} // namespace webEngine
#endif //__WEIAUDIT_H__
