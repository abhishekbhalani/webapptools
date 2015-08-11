# Abstract #

**iwePlugin** is the basic interface for the plugins.

Plugin is the interface to the external dynamic library. The library provides method to create the contained plugin as the basic **iwePlugin** interface. Other interfaces may be requested from the plugin by calling the **GetInterface** function. Plugin object can't be copied, use the **GetInterface** function instead to obtain pointer to the plugin or copy the existing pointer if you are sure to control the object's lifetime. Program must avoid the explicit destruction of the plugin, because it may destroy the object, that can be used somewhere else. Use the **Release** function to free the pointer to the interface when it is no longer needed.

**Important modifications:**
| **Date** | **Author** | **Description** |
|:---------|:-----------|:----------------|
| 19.06.2009  | A. Abramov | Creation        |

# Structure and relations #

Followed scheme describes the structure of the plugins classes inheritance:

![http://webapptools.googlecode.com/svn/wiki/images/iweplugin_graph.png](http://webapptools.googlecode.com/svn/wiki/images/iweplugin_graph.png) [[legend](GraphLegend.md)]


# Interface description #

[Doxygen documentation](http://webapptools.googlecode.com/svn/wiki/doxygen/classweb_engine_1_1i_plugin.html)