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

#include <weiPlugin.h>

namespace webEngine {

// forward declarations
class Task;
class ScanData;
class iResponse;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @interface  iAudit
///
/// @brief  Interface for the audit plugins. 
///
/// @author A. Abramov
/// @date   19.08.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class iAudit :
    public i_plugin
{
public:
    iAudit(engine_dispatcher* krnl, void* handle = NULL);
    virtual ~iAudit(void);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn void Start(Task* tsk)
    ///
    /// @brief  Starts the audit process for given ScanData object. 
    ///
    /// @param  tsk	 - If non-null, the pointer to task what handles the process. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void Start(Task* tsk, ScanData* scData) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn void ProcessResponse(iResponse *resp)
    ///
    /// @brief  Process the transport response described by resp.
    /// 		
    /// @param  resp - If non-null, the resp. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void ProcessResponse(iResponse *resp) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn static void ResponseDispatcher(iResponse *resp, void* context)
    ///
    /// @brief  Response dispatcher. Sends the response to process into the appropriate object pointed
    ///         by the context
    ///
    /// @param  resp	 - If non-null, the resp. 
    /// @param  context	 - If non-null, the context. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    static void ResponseDispatcher(iResponse *resp, void* context);

protected:
    Task* task;
};

} // namespace webEngine
#endif //__WEIAUDIT_H__
