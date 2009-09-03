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

using namespace webEngine;

int iStorage::lastId = 0;

iStorage::iStorage(Dispatch* krnl, void* handle /*= NULL*/) :
    iPlugin(krnl, handle)
{
    pluginInfo.IfaceName = "iStorage";
    pluginInfo.IfaceList.push_back("iStorage");
    pluginInfo.PluginDesc = "Base plugin interface";
    pluginInfo.PluginId = "C7F595160595";
    pluginInfo.PluginIcon = WeXpmToStringList(iweStorage_xpm, sizeof(iweStorage_xpm) / sizeof(char*) );
    lastId = rand();
}

iStorage::~iStorage(void)
{
}

void* iStorage::GetInterface( const string& ifName )
{
    if (iequals(ifName, "iStorage"))
    {
        usageCount++;
        return (void*)((iStorage*)this);
    }
    return iPlugin::GetInterface(ifName);
}

std::string iStorage::GenerateID( const string& objType /*= ""*/ )
{
    return lexical_cast<string>(++lastId);
}

int iStorage::Query( const string& objType, const string& objId, Operation op, const string& xmlData )
{
    /// @todo Implement this!
    LOG4CXX_FATAL(iLogger::GetLogger(), "iStorage::Query - Not implemented");
    return -1;
}

int iStorage::Report( const string& repType, const string& objId, const string& xmlData, string& result )
{
    /// @todo Implement this!
    LOG4CXX_FATAL(iLogger::GetLogger(), "iStorage::Report - Not implemented");
    return -1;
}

int iStorage::Delete( const string& objType, const string& xmlData )
{
    int retval = 0;
    bool inTask = false;
    StrStream st(xmlData.c_str());
    TagScanner sc(st);
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
            LOG4CXX_WARN(iLogger::GetLogger(), "iStorage::Delete - parsing error");
            goto FINISH;
            break;
        case wstEof:
            LOG4CXX_TRACE(iLogger::GetLogger(), "iStorage::Delete - parsing EOF");
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
                LOG4CXX_TRACE(iLogger::GetLogger(), "iStorage::Delete - unexpected tag: " << tag);
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
                    retval += Query(objType, id, iStorage::remove, xmlData.substr(tagStart, pos - tagStart));
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
            //LOG4CXX_WARN("iStorage::TaskSave - unexpected token: " << t);
            break;
        };
    };
FINISH:
    return retval;
}

int iStorage::TaskSave( const string& xmlData, Operation op /*= iStorage::autoop*/ )
{
    return ObjectQuery(weObjTypeTask, xmlData, op);
}

int iStorage::DictionarySave( const string& xmlData, Operation op /*= iStorage::autoop*/ )
{
    return ObjectQuery(weObjTypeDictionary, xmlData, op);
}

int iStorage::AuthorizationSave( const string& xmlData, Operation op /*= iStorage::autoop*/ )
{
    return ObjectQuery(weObjTypeAuthInfo, xmlData, op);
}

int iStorage::SystemOptionsSave( const string& xmlData, Operation op /*= iStorage::autoop*/ )
{
    return Query(weObjTypeSysOption, "0", op, xmlData);
}

int iStorage::ScanSave( const string& xmlData, Operation op /*= iStorage::autoop*/ )
{
    return ObjectQuery(weObjTypeScan, xmlData, op);
}

int iStorage::TaskReport( const string& xmlData, string& result )
{
    return ObjectReport(weObjTypeTask, xmlData, result);
}

int iStorage::DictionaryReport( const string& xmlData, string& result )
{
    return ObjectReport(weObjTypeDictionary, xmlData, result);
}

int iStorage::AuthorizationReport( const string& xmlData, string& result )
{
    return ObjectReport(weObjTypeAuthInfo, xmlData, result);
}

int iStorage::SystemOptionsReport( const string& xmlData, string& result )
{
    return Report(weObjTypeSysOption, "0", xmlData, result);
}

int iStorage::ScanReport( const string& xmlData, string& result )
{
    return ObjectReport(weObjTypeScan, xmlData, result);
}

int iStorage::ObjectReport( const string& objType, const string& xmlData, string& result )
{
    int retval = 0;
    StrStream st(xmlData.c_str());
    TagScanner sc(st);
    bool inObject = false;
    bool inReport = false;
    bool inParse = true;
    string id;
    string tag;

    //int retval = Report(weObjTypeScan, "-1", xmlData, result);
    result = "<report>\n";
    while (inParse)
    {
        int t = sc.GetToken();
        switch(t)
        {
        case wstError:
            LOG4CXX_WARN(iLogger::GetLogger(), "iStorage::ObjectReport - parsing error");
            inParse = false;
            break;
        case wstEof:
            LOG4CXX_TRACE(iLogger::GetLogger(), "iStorage::ObjectReport - parsing EOF");
            inParse = false;
            break;
        case wstTagStart:
            tag = sc.GetTagName();
            if (inReport)
            {
                if (iequals(tag, objType))
                {
                    id = "";
                    inObject = true;
                }
                else
                {
                    LOG4CXX_WARN(iLogger::GetLogger(), "iStorage::ObjectReport - unexpected tag " << tag);
                }
            }
            else
            {
                if (iequals(tag, "report"))
                {
                    inReport = true;
                    inObject = false;
                }
                else
                {
                    LOG4CXX_WARN(iLogger::GetLogger(), "iStorage::ObjectReport - unexpected tag " << tag);
                }
            }
            break;
        case wstTagEnd:
            tag = sc.GetTagName();
            if (inReport && !inObject)
            {
                if (iequals(tag, "report"))
                {
                    inReport = false;
                    inObject = false;
                    inParse  = false;
                }
                else
                {
                    LOG4CXX_WARN(iLogger::GetLogger(), "iStorage::ObjectReport - unexpected tag " << tag);
                    inParse = false;
                    break;
                }
            }
            else if (inObject) {
                if (iequals(tag, objType))
                {
                    id = "";
                    inObject = false;
                }
                else
                {
                    LOG4CXX_WARN(iLogger::GetLogger(), "iStorage::ObjectReport - unexpected tag " << tag);
                }
            }
            else {
                LOG4CXX_WARN(iLogger::GetLogger(), "iStorage::ObjectReport - unexpected tag" << tag);
                inParse = false;
                break;
            }
            break;
        case wstAttr:
            if (inObject) {
                if (id.empty()) {
                    tag = sc.GetAttrName();
                    if (iequals(tag, "value")) {
                        id = sc.GetValue();
                        tag = "";
                        int rep = Report(objType, id, "", tag);
                        if (rep > 0) {
                            retval += rep;
                            result += tag;
                        }
                    }
                }
            }
            break;
        default:
            LOG4CXX_WARN(iLogger::GetLogger(), "iStorage::ObjectReport - unexpected token: " << t);
            break;
        };
    }
    result += "</report>\n";
    result += '\0';
    return retval;
}

int iStorage::ObjectQuery( const string& objType, const string& xmlData, Operation op /*= autoop*/ )
{
    int retval = 0;
    bool inObject = false;
    bool inParse = true;
    StrStream st(xmlData.c_str());
    TagScanner sc(st);
    int pos;
    int objStart;
    string id;
    string tag;

    LOG4CXX_DEBUG(iLogger::GetLogger(), "iStorage::ObjectQuery - saving " << objType);
    while(inParse)
    {
        pos = st.GetPos();
        int t = sc.GetToken();
        switch(t)
        {
        case wstError:
            LOG4CXX_WARN(iLogger::GetLogger(), "iStorage::ObjectQuery - parsing error");
            inParse = false;
            break;
        case wstEof:
            LOG4CXX_TRACE(iLogger::GetLogger(), "iStorage::ObjectQuery - parsing EOF");
            inParse = false;
            break;
        case wstTagStart:
            tag = sc.GetTagName();
            if (!inObject) {
                if (iequals(tag, objType)) {
                    inObject = true;
                    id = "";
                    objStart = pos;
                    retval++;
                }
                else {
                    LOG4CXX_WARN(iLogger::GetLogger(), "iStorage::ObjectQuery - not an object of type " << objType);
                    inParse = false;
                }
            }
            break;
        case wstTagEnd:
            tag = sc.GetTagName();
            if (inObject) {
                if (iequals(tag, objType)) {
                    inObject = false;
                    pos += objType.length() + 3; // strlen("</objType>") if the XML is wellformed
                    if (!id.empty()) {
                        Query(objType, id, op, xmlData.substr(objStart, pos - objStart));
                        LOG4CXX_TRACE(iLogger::GetLogger(), "iStorage::ObjectQuery - " << objType << " ID=" << id << " saved(updated)");
                    }
                }
            }
            break;
        case wstAttr:
            if (inObject) {
                if (id.empty()) {
                    tag = sc.GetAttrName();
                    if (iequals(tag, "id")) {
                        id = sc.GetValue();
                    }
                }
            }
            break;
        default:
            //LOG4CXX_WARN("iStorage::TaskSave - unexpected token: " << t);
            break;
        };
    }
    return retval;
}