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
//#include <boost/shared_ptr.hpp>
#include "weStrings.h"

//using namespace boost;
using namespace std;

namespace webEngine {

struct plugin_info
{
    string  plugin_id;
    string  plugin_desc;
    string  interface_name;
    string_list interface_list;
    string_list plugin_icon;
    int     plugin_status;
    string  plugin_path;
};

// forward declaration
class engine_dispatcher;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @interface  i_plugin
///
/// @brief  Basic interface for the plugins.
///
/// i_plugin is the basic interface for the plugins.
///
/// Plugin is the interface to the external dynamic library. The library provides method to
/// create the contained plugin as the basic i_plugin interface. Other interfaces may be
/// requested from the plugin by calling the get_interface function. Plugin object can't be copied,
/// use the get_interface function instead to obtain pointer to the plugin or copy the existing
/// pointer if you are sure to control the object's lifetime. Program must avoid the explicit
/// destruction of the plugin, because it may destroy the object, that can be used somewhere
/// else. Use the release function to free the pointer to the interface when it is no
/// longer needed.
///
/// @author A. Abramov
/// @date   19.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class i_plugin
{
public:

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn i_plugin(void* handle = NULL)
    ///
    /// @brief  Constructor.
    ///
    /// @param  handle - The handle to the contained library.
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn i_plugin(engine_dispatcher* krnl,
    /// 	void* handle = NULL)
    ///
    /// @brief  Constructor. 
    ///
    /// @param  krnl   - Back link to the kernel
    /// @param  handle - The handle to the contained library. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    i_plugin(engine_dispatcher* krnl, void* handle = NULL);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual ~i_plugin()
    ///
    /// @brief  Destructor.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~i_plugin();

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
    /// @fn virtual void release()
    ///
    /// @brief  Releases this object.
    ///
    /// Decrease usage counter and destroy object if counter is NULL
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void release();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual const string interface_name()
    ///
    /// @brief  Gets the final Interface name.
    ///
    /// @retval	string. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual const string interface_name();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual string_list interface_list()
    ///
    /// @brief  Gets the list of supported Interfaces. 
    ///
    /// @retval strings list. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual string_list interface_list();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual const string& get_description()
    ///
    /// @brief  Gets the plugin description.
    ///
    /// @retval	The description.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual const string get_description();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn	virtual const string& get_id() = 0
    ///
    /// @brief	Gets the identifier.
    ///
    /// Returns string, what identifies this plugin. It may be GUID, or other unique identifier
    ///
    /// @retval	The identifier.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual const string get_id();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual char* get_icon()
    ///
    /// @brief  Gets the icon.
    ///
    /// Returns pointer to the array of chars, that represents image in the XPM format. This image may
    /// be used to identify the plugin in GUI.
    ///
    /// @retval null if no icon, else the icon.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual string_list get_icon();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn	const plugin_info* info(void)
    ///
    /// @brief	Returns information about this object. 
    ///
    /// @retval	null if it fails, plugin_info else. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    const plugin_info* info(void) { return &pluginInfo; };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual const string get_setup_ui( void )
    ///
    /// @brief  Gets the user interface for the setup dialog. 
    ///
    /// @retval The user interface in the XML-based format or empty string if no setup dialog. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual const string get_setup_ui( void ) { return ""; };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual void apply_settings( const string& xmlData )
    ///
    /// @brief  Applies the settings described by xmlData.
    ///
    /// @param  xmlData	 - Plugin settings describing the XML. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void apply_settings( const string& xmlData ) {};

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual void get_priority()
    ///
    /// @brief  Gets the plugin's priority (0 - 100, default: 50).
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual int get_priority() { return priority; };

#ifndef __DOXYGEN__
protected:
    int usageCount;
    int priority;
    void* libHandle;
    plugin_info pluginInfo;
    engine_dispatcher* kernel;
    log4cxx::LoggerPtr logger;

private:
    i_plugin(i_plugin&) {};               ///< Avoid object copying
    i_plugin& operator=(i_plugin&) { return *this; };    ///< Avoid object copying
#endif // __DOXYGEN__
};

typedef vector<plugin_info> plugin_list;

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
/// @fn string_list WeXpmToStringList(char** xpm,
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
string_list WeXpmToStringList(char** xpm, int lines);

} // namespace webEngine

#endif //__WEIPLUGIN_H__
