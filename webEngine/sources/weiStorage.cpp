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
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "weiBase.h"
#include "weTagScanner.h"
#include "weiStorage.h"
#include "iweStorage.xpm"

int iweStorage::lastId = 0;

iweStorage::iweStorage(WeDispatch* krnl, void* handle /*= NULL*/) :
    iwePlugin(krnl, handle)
{
    pluginInfo.IfaceName = "iweStorage";
    pluginInfo.IfaceList.push_back("iweStorage");
    pluginInfo.PluginDesc = "Base plugin interface";
    pluginInfo.PluginId = "C7F595160595";
    pluginInfo.PluginIcon = WeXpmToStringList(iweStorage_xpm, sizeof(iweStorage_xpm) / sizeof(char*) );
    lastId = rand();
}

iweStorage::~iweStorage(void)
{
}

void* iweStorage::GetInterface( const string& ifName )
{
    if (iequals(ifName, "iweStorage"))
    {
        usageCount++;
        return (void*)((iweStorage*)this);
    }
    return iwePlugin::GetInterface(ifName);
}

std::string iweStorage::GenerateID( string objType /*= ""*/ )
{
    return lexical_cast<string>(++lastId);
}

int iweStorage::Query( const string& objType, const string& objId, Operation op, const string& xmlData )
{
    /// @todo Implement this!
    throw runtime_error("Not implemented");
    return -1;
}

int iweStorage::Report( const string& repType, const string& objId, const string& xmlData, string& result )
{
    /// @todo Implement this!
    throw runtime_error("Not implemented");
    return -1;
}

int iweStorage::Delete( const string& objType, const string& xmlData )
{
    int retval = 0;
    bool inTask = false;
    WeStrStream st(xmlData.c_str());
    WeTagScanner sc(st);
    int pos;
    int tagStart;
    string id;
    string tag;

    while(true)
    {
        pos = st.GetPos();
        int t = sc.GetToken();
        switch(t)
        {
        case wstError:
            LOG4CXX_WARN(WeLogger::GetLogger(), "iweStorage::Delete - parsing error");
            goto FINISH;
            break;
        case wstEof:
            LOG4CXX_TRACE(WeLogger::GetLogger(), "iweStorage::Delete - parsing EOF");
            goto FINISH;
            break;
        case wstTagStart:
            tag = sc.GetTagName();
            if (iequals(tag, "delete"))
            {
                inTask = true;
                tagStart = pos;
            }
            else {
                LOG4CXX_TRACE(WeLogger::GetLogger(), "iweStorage::Delete - unexpected tag: " << tag);
                goto FINISH;
            }
            break;
        case wstTagEnd:
            tag = sc.GetTagName();
            if (iequals(tag, "delete"))
            {
                inTask = false;
                pos += 2; // at least strlen("/>") for simple cases. Not well-formed XML for complex queries, but it will be parsed
                if (!id.empty()) {
                    retval += Query(objType, id, iweStorage::remove, xmlData.substr(tagStart, pos - tagStart));
                }
            }
            break;
        case wstAttr:
            if (inTask) {
                if (id.empty()) {
                    tag = sc.GetAttrName();
                    if (iequals(tag, objType)) {
                        id = sc.GetValue();
                    }
                }
            }
            break;
        default:
            //LOG4CXX_WARN("iweStorage::TaskSave - unexpected token: " << t);
            break;
        };
    };
FINISH:
    return retval;
}

int iweStorage::TaskSave( const string& xmlData, Operation op /*= iweStorage::autoop*/ )
{
    int retval = 0;
    bool inTask = false;
    WeStrStream st(xmlData.c_str());
    WeTagScanner sc(st);
    int pos;
    int taskStart;
    string id;
    string tag;

    while(true)
    {
        pos = st.GetPos();
        int t = sc.GetToken();
        switch(t)
        {
        case wstError:
            LOG4CXX_WARN(WeLogger::GetLogger(), "iweStorage::TaskSave - parsing error");
            goto FINISH;
            break;
        case wstEof:
            LOG4CXX_TRACE(WeLogger::GetLogger(), "iweStorage::TaskSave - parsing EOF");
            goto FINISH;
            break;
        case wstTagStart:
            tag = sc.GetTagName();
            if (!inTask) {
                if (iequals(tag, "task")) {
                    inTask = true;
                    id = "";
                    taskStart = pos;
                    retval++;
                }
                else {
                    LOG4CXX_WARN(WeLogger::GetLogger(), "iweStorage::TaskSave - not a task");
                    goto FINISH;
                }
            }
            break;
        case wstTagEnd:
            tag = sc.GetTagName();
            if (inTask) {
                if (iequals(tag, "task")) {
                    inTask = false;
                    pos += 7; // strlen("</task>") if the XML is wellformed
                    if (!id.empty()) {
                        LOG4CXX_TRACE(WeLogger::GetLogger(), "iweStorage::TaskSave - " << id << " saved(updated)");
                        Query("task", id, op, xmlData.substr(taskStart, pos - taskStart));
                    }
                }
            }
            break;
        case wstAttr:
            if (inTask) {
                if (id.empty()) {
                    tag = sc.GetAttrName();
                    if (iequals(tag, "id")) {
                        id = sc.GetValue();
                    }
                }
            }
            break;
        default:
            //LOG4CXX_WARN("iweStorage::TaskSave - unexpected token: " << t);
            break;
        };
    }
FINISH:
    return retval;
}

int iweStorage::DictionarySave( const string& xmlData, Operation op /*= iweStorage::autoop*/ )
{
    /// @todo Add input data validation
    return Query("dict", "-1", op, xmlData);
}

int iweStorage::AuthorizationSave( const string& xmlData, Operation op /*= iweStorage::autoop*/ )
{
    /// @todo Add input data validation
    return Query("auth", "-1", op, xmlData);
}

int iweStorage::SystemOptionsSave( const string& xmlData, Operation op /*= iweStorage::autoop*/ )
{
    /// @todo Add input data validation
    return Query("sysopt", "-1", op, xmlData);
}

int iweStorage::TaskReport( const string& xmlData, string& result )
{
    int retval = 0;
    WeStrStream st(xmlData.c_str());
    WeTagScanner sc(st);
    bool inTask = false;
    string id;
    string tag;
    WeStringMap::iterator tsk;

    result = "<taskreport>\n";
    while(true)
    {
        int t = sc.GetToken();
        switch(t)
        {
        case wstError:
            LOG4CXX_WARN(WeLogger::GetLogger(), "iweStorage::TaskReport - parsing error");
            goto FINISH;
            break;
        case wstEof:
            LOG4CXX_TRACE(WeLogger::GetLogger(), "iweStorage::TaskReport - parsing EOF");
            goto FINISH;
            break;
        case wstTagStart:
            tag = sc.GetTagName();
            if (inTask)
            {
                if (iequals(tag, "id"))
                {
                    id = "";
                }
                else
                {
                    LOG4CXX_WARN(WeLogger::GetLogger(), "iweStorage::TaskReport - unexpected tag");
                }
            }
            else
            {
                if (iequals(tag, "taskreport"))
                {
                    inTask = true;
                }
                else
                {
                    LOG4CXX_WARN(WeLogger::GetLogger(), "iweStorage::TaskReport - unexpected tag");
                }
            }
            break;
        case wstTagEnd:
            tag = sc.GetTagName();
            break;
        case wstAttr:
            if (inTask) {
                if (id.empty()) {
                    tag = sc.GetAttrName();
                    if (iequals(tag, "value")) {
                        id = sc.GetValue();
                        tag = "";
                        int rep = Report("task", id, "", tag);
                        if (rep > 0) {
                            retval += rep;
                            result += tag;
                        }
                    }
                }
            }
            break;
        default:
            LOG4CXX_WARN(WeLogger::GetLogger(), "iweStorage::TaskReport - unexpected token: " << t);
            break;
        };
    };
FINISH:
    result += "</taskreport>\n";
    return retval;
}

int iweStorage::DictionaryReport( const string& xmlData, string& result )
{
    int retval = Report("dict", "-1", xmlData, result);
    /// @todo postprocess result
    return retval;
}

int iweStorage::AuthorizationReport( const string& xmlData, string& result )
{
    int retval = Report("auth", "-1", xmlData, result);
    /// @todo postprocess result
    return retval;
}

int iweStorage::SystemOptionsReport( const string& xmlData, string& result )
{
    int retval = Report("sysopt", "-1", xmlData, result);
    /// @todo postprocess result
    return retval;
}
