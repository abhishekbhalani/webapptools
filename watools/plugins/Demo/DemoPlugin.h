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
#include <weiPlugin.h>

using namespace webEngine;

class DemoPlugin :
    public i_plugin {
public:
    DemoPlugin(engine_dispatcher* krnl, void* handle = NULL);
    virtual ~DemoPlugin(void);

    // iPlugin functions
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual void* GetInterface(const string& ifName)
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
    virtual void* GetInterface(const string& ifName);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual const string GetSetupUI( void )
    ///
    /// @brief  Gets the user interface for the setup dialog.
    ///
    /// @retval The user interface in the XRC format (wxWidgets) or empty string if no setup dialog.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual const string GetSetupUI( void );

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual void ApplySettings( const string& xmlData )
    ///
    /// @brief  Applies the settings described by xmlData.
    ///
    /// @param  xmlData	 - Plugin settings describing the XML.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void ApplySettings( const string& xmlData );

    virtual void init(task* tsk) {};
    virtual void pause(task* tsk, bool paused = true) {};
    virtual void stop(task* tsk) {};
};
