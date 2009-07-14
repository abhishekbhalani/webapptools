/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew Abramov aabramov@ptsecurity.ru

    This file is part of webEngine

    webEngineis free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngineis distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __WEIPLUGIN_H__
#define __WEIPLUGIN_H__
#include <string>
#include <boost/shared_ptr.hpp>
#include "weStrings.h"

using namespace boost;
using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @interface  iwePlugin
///
/// @brief  Basic interface for the plugins.
///
/// Plugin is the interface to the external dynamic library. The library provides method to create
/// the contained plugin as the basic iwePlugin interface. Other interfaces may be requested from
/// the plugin by calling the GetInterface function. Plugin object can't be copied, use the GetRef
/// function instead to obtain pointer to the plugin. Program must avoid the explicit destruction
/// of the plugin, because it may destroy the object, that can be used somewhere else. Use the
/// shared_ptr to the plugin instead to keep references .
///
/// @author A. Abramov
/// @date   19.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class iwePlugin
{
public:

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn iwePlugin(void* handle = NULL)
    ///
    /// @brief  Constructor.
    ///
    /// @param  handle - The handle to the contained library.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    iwePlugin(void* handle = NULL) { usageCount = 0; libHandle = handle; };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual ~iwePlugin()
    ///
    /// @brief  Finaliser.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~iwePlugin() {};

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual shared_ptr<iwePlugin> GetRef()
    ///
    /// @brief  Adds  reference
    ///
    /// Must be used instead the copying the object
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual shared_ptr<iwePlugin>& GetRef() { return (* new shared_ptr<iwePlugin>(this)); };

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
    virtual void* GetInterface(const string& ifName) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual const string InterfaceName()
    ///
    /// @brief  Gets the final Interface name.
    ///
    /// @retval	string. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual const string InterfaceName();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual WeStringList InterfaceList()
    ///
    /// @brief  Gets the list of supported Interfaces. 
    ///
    /// @retval strings list. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual WeStringList InterfaceList();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual const string& GetDesc()
    ///
    /// @brief  Gets the plugin description.
    ///
    /// @retval	The description.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual const string GetDesc() = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn	virtual const string& GetID() = 0
    ///
    /// @brief	Gets the identifier.
    ///
    /// Returns string, what identifies this plugin. It may be GUID, or other unique identifier
    ///
    /// @retval	The identifier.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual const string GetID() = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual char* GetIcon() = 0
    ///
    /// @brief  Gets the icon.
    ///
    /// Returns pointer to the array of chars, that represents image in the XPM format. This image may
    /// be used to identify the plugin in GUI.
    ///
    /// @retval null if no icon, else the icon.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual char** GetIcon() = 0;

#ifndef __DOXYGEN__
protected:
    int usageCount;
    void* libHandle;

private:
    iwePlugin(iwePlugin&) {};               ///< Avoid object copying
    iwePlugin& operator=(iwePlugin&) { return *this; };    ///< Avoid object copying
#endif // __DOXYGEN__
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @typedef    void* (*fnWePluginFactory)(void)
///
/// @brief  Declares the pointer to the WePluginFactory function.
///
/// @author A. Abramov
/// @date   19.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
typedef void* (*fnWePluginFactory)(void);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn shared_ptr<iwePlugin> weLoadPlugin(const string& libName)
///
/// @brief  Load plugin library.
///
/// This function performs the platform-specific actions to load dynamic library and obtain the
/// iwePlugin interface.@n
/// The library must provide undecorated function (extern "C" specification) named @b WePluginFactory
/// to create the new instance of the contained plugin. Each library contains only one plugin.@n
///
/// @author A. Abramov
/// @date   19.06.2009
///
/// @param  libName	 - Name of the library.
///
/// @retval null if it fails, the iwePlugin interface else.
////////////////////////////////////////////////////////////////////////////////////////////////////
extern shared_ptr<iwePlugin>& weLoadPlugin(const string& libName);

#endif //__WEIPLUGIN_H__
