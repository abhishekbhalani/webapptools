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
#include "weiPlugin.h"

namespace webEngine {

class Task;
class iResponse;

class i_inventory :
    public i_plugin
{
public:
    i_inventory(engine_dispatcher* krnl, void* handle = NULL);
    virtual ~i_inventory(void);

    // i_plugin functions
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual void* get_interface(const string& ifName)
    ///
    /// @brief  Gets an interface.
    ///
    /// Returns abstracted pointer to the requested interface, or NULL if the requested interface isn't
    /// provided by this object. The interface name depends on the plugin implementation.
    ///
    /// @param  ifName - Name of the interface.
    ///
    /// @retval	null if it fails, else the interface.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void* get_interface(const string& ifName);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn void start(Task* tsk)
    ///
    /// @brief  Starts the inventory process. 
    ///
    /// @param  tsk	 - If non-null, the pointer to task what handles the process. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void start(Task* tsk) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn void process_response(iResponse *resp)
    ///
    /// @brief  Process the transport response described by resp.
    /// 		
    /// @param  resp - If non-null, the resp. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void process_response(iResponse *resp) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn static void response_dispatcher(iResponse *resp, void* context)
    ///
    /// @brief  Response dispatcher. Sends the response to process into the appropriate object pointed
    ///         by the context
    ///
    /// @param  resp	 - If non-null, the resp. 
    /// @param  context	 - If non-null, the context. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    static void response_dispatcher(iResponse *resp, void* context);

protected:
    Task* task;
};

} // namespace webEngine
