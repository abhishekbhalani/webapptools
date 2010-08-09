/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew Abramov aabramov@ptsecurity.ru

    This file is part of webEngine

    webEngine is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <webEngine.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include "weiBase.h"
#include "weiStorage.h"
#include "iweStorage.xpm"

using namespace webEngine;
using namespace boost;

static map< string, vector<string> > namespaces_structs;
static bool ns_init = false;
static void init_namespaces();

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @property   char* webEngine::idb_struct[]
///
/// @brief   Defines database structure (tables) 
///
/// Each value in this array is the definition for one database namespace (table). Last value MUST
/// be NULL to detect the end of definitions. This array must be used to database initialization
/// and consistence checking.
///
/// Record format is:@n
/// @b @e "table_name:column1_name column1_type column1_attributes:..." @n
///
/// @b columnN block may repeats as need to describe table structure. Atributes are divided by space. @n
/// @b columnN_name is the required attribute. @n
/// @b columnN_type is not the required, but should be used. @n
/// @b columnN_attributes is the optional attribute(s) and may repeat as need. @n
////////////////////////////////////////////////////////////////////////////////////////////////////
char* webEngine::idb_struct[] =
{

    "profile:profile_id INT:name TEXT:type INT:value VARIANT",
    "profile_ui:plugin_id TEXT NOT NULL:plugin_name TEXT NOT NULL:locale TEXT NOT NULL:ui_settings BLOB NOT NULL:ui_icon BLOB NOT NULL",
    "scan_data:id INT:task_id INT:parent_id INT:object_url TEXT:scan_depth INT:resp_code INT:data_size INT:dnld_time INT:content_type TEXT",
    "task:id INT:profile_id INT:name TEXT:status INT:completion INT:start_time TEXT:finish_time TEXT:ping_time TEXT:requests INT:processed_urls BLOB",
    "vulners:id INT NOT NULL:task_id INT NOT NULL:object_id INT:plugin_id TEXT:severity INT DEFAULT 0:vulner_id INT:params TEXT",
    "vulner_desc:plugin_id TEXT NOT NULL:id INT NOT NULL:severity INT NOT NULL:title TEXT:short TEXT:desc BLOB:locale TEXT",
    "modules:id TEXT NOT NULL:instance INT NOT NULL:class INT NOT NULL:version TEXT:ipaddr TEXT:name TEXT:stamp INT:timeout INT:onrun INT:status TEXT",
    "modules_info:module_id TEXT NOT NULL  UNIQUE:osname TEXT:mem_size TEXT:cpu_usage TEXT:disk_size TEXT:max_tasks INT:stamp INT NOT NULL DEFAULT 0:timeout INT NOT NULL DEFAULT 0",
    "module_cmds:module_id TEXT NOT NULL:timestamp INT NOT NULL:cmd BLOB",
    "auth_data:task_id INT NOT NULL:data_type INT NOT NULL:name TEXT NOT NULL:value TEXT:timeout INT:path TEXT:domain TEXT",
    NULL
};

i_storage::i_storage(engine_dispatcher* krnl, void* handle /*= NULL*/) :
    i_plugin(krnl, handle)
{
    pluginInfo.interface_name = "i_storage";
    pluginInfo.interface_list.push_back("i_storage");
    pluginInfo.plugin_desc = "Base plugin interface";
    pluginInfo.plugin_id = "C7F595160595";
    pluginInfo.plugin_icon = WeXpmToStringList(iweStorage_xpm, sizeof(iweStorage_xpm) / sizeof(char*) );
    last_id = rand();
    init_namespaces();
}

i_storage::~i_storage(void)
{
}

i_plugin* i_storage::get_interface( const string& ifName )
{
    if (iequals(ifName, "i_storage"))
    {
        usageCount++;
        return ((i_storage*)this);
    }
    return i_plugin::get_interface(ifName);
}

std::string i_storage::generate_id( const string& objType /*= ""*/ )
{
    return lexical_cast<string>(++last_id);
}

std::vector<std::string> i_storage::get_namespace_struct( const std::string& ns )
{
    if (namespaces_structs.find(ns) != namespaces_structs.end()) {
        return namespaces_structs[ns];
    }
    else {
        return namespaces_structs[""];
    }
}

void init_namespaces()
{
    size_t i = 0;
    std::vector<std::string> inits;
    std::vector<std::string> fields;

    if (!ns_init) {
        ns_init = true;
        fields.clear();
        namespaces_structs[""] = fields;

        while (idb_struct[i] != NULL) {
            boost::split(inits, idb_struct[i], boost::is_any_of(":"));
            if (inits.size() > 0) {
                //create namespace
                fields.clear();
                for (size_t j = 1; j < inits.size(); j++) {
                    string fld = inits[j];
                    size_t pos = fld.find(' ');
                    if (pos != string::npos) {
                        fld = fld.substr(0, pos);
                    }
                    if (fld.length() > 0) {
                        fields.push_back(inits[0] + "." + fld);
                    }
                } // for each field
                namespaces_structs[inits[0]] = fields;
            } // if inits parsed
            i++;
        } // while exist initializer
    }
}
