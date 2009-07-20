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
#ifndef __WEIPLUGIN_H__
#define __WEIPLUGIN_H__
#include "weiBase.h"
#include <string>
#include <boost/shared_ptr.hpp>
#include "weStrings.h"

using namespace boost;
using namespace std;

struct WePluginInfo
{
    string  PluginId;
    string  PluginDesc;
    string  IfaceName;
    WeStringList IfaceList;
    WeStringList PluginIcon;
    int     PluginStatus;
    string  PluginPath;
};

// forward declaration
class WeDispatch;

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

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn iwePlugin(WeDispatch* krnl,
    /// 	void* handle = NULL)
    ///
    /// @brief  Constructor. 
    ///
    /// @param  krnl   - Back link to the kernel
    /// @param  handle - The handle to the contained library. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    iwePlugin(WeDispatch* krnl, void* handle = NULL);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual ~iwePlugin()
    ///
    /// @brief  Destructor.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~iwePlugin() {};

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
    /// @fn virtual void Release()
    ///
    /// @brief  Releases this object.
    ///
    /// Decrease usage counter and destroy object if counter is NULL
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void Release();

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
    virtual const string GetDesc();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn	virtual const string& GetID() = 0
    ///
    /// @brief	Gets the identifier.
    ///
    /// Returns string, what identifies this plugin. It may be GUID, or other unique identifier
    ///
    /// @retval	The identifier.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual const string GetID();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual char* GetIcon()
    ///
    /// @brief  Gets the icon.
    ///
    /// Returns pointer to the array of chars, that represents image in the XPM format. This image may
    /// be used to identify the plugin in GUI.
    ///
    /// @retval null if no icon, else the icon.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual WeStringList GetIcon();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn	const WePluginInfo* Info(void)
    ///
    /// @brief	Returns information about this object. 
    ///
    /// @retval	null if it fails, WePluginInfo else. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    const WePluginInfo* Info(void) { return &pluginInfo; };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual const string GetSetupUI( void )
    ///
    /// @brief  Gets the user interface for the setup dialog. 
    ///
    /// @retval The user interface in the XRC format (wxWidgets) or empty string if no setup dialog. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual const string GetSetupUI( void ) { return ""; };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual void ApplySettings( const string& xmlData )
    ///
    /// @brief  Applies the settings described by xmlData.
    ///
    /// @param  xmlData	 - Plugin settings describing the XML. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void ApplySettings( const string& xmlData ) {};

#ifndef __DOXYGEN__
protected:
    int usageCount;
    void* libHandle;
    WePluginInfo pluginInfo;
    WeDispatch* kernel;

private:
    iwePlugin(iwePlugin&) {};               ///< Avoid object copying
    iwePlugin& operator=(iwePlugin&) { return *this; };    ///< Avoid object copying
#endif // __DOXYGEN__
};

typedef vector<WePluginInfo> WePluginList;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @typedef    void* (*fnWePluginFactory)(void)
///
/// @brief  Declares the pointer to the WePluginFactory function.
///
/// @author A. Abramov
/// @date   19.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
typedef void* (*fnWePluginFactory)(void* kernel, void* handle);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn WeStringList WeXpmToStringList(char** xpm,
/// 	int lines)
///
/// @brief  Convetrs XPM to string list. 
///
/// @author A. Abramov
/// @date   20.07.2009
///
/// @param  xpm   - The XPM image. 
/// @param  lines - The number of lines in the XPM. 
///
/// @retval	. 
////////////////////////////////////////////////////////////////////////////////////////////////////
WeStringList WeXpmToStringList(char** xpm, int lines);

#endif //__WEIPLUGIN_H__
