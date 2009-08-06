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
#include "weHelper.h"
#include "weTask.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>

WeTask::WeTask()
{
    FUNCTION;
    transport = NULL;
    // set the default options
    Option(weoName, string(""));
    Option(weoID, string(""));
    Option(weoTaskStatus, 0);
    Option(weoTaskCompletion, 0);
    Option(weoAutoProcess, true);
    Option(weoStayInDir, true);
    Option(weoFollowLinks, true);
    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask created");
}

WeTask::WeTask( WeTask& cpy )
{
    options = cpy.options;
    if (cpy.transport != NULL) {
        SetTransport(cpy.transport->GetName());
    }
    else {
        transport = NULL;
    }
    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask assignment");
}

WeTask::~WeTask()
{
    /// @todo Cleanup
}

iweResponse* WeTask::GetRequest( iweRequest* req )
{
    iweResponse* resp;

    FUNCTION;
    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask::GetRequest");
    resp = GetRequestAsync( req );
    if (resp != NULL) {
        /// @todo: wait for response completion
        LOG4CXX_ERROR(WeLogger::GetLogger(), " *** Not implemented yet! ***");
    }
    return resp;
}

iweResponse* WeTask::GetRequestAsync( iweRequest* req )
{
    /// @todo Implement this!
    FUNCTION;
    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask::GetRequestAsync");
    LOG4CXX_ERROR(WeLogger::GetLogger(), " *** Not implemented yet! ***");
    return NULL;
}

bool WeTask::IsReady()
{
    string name;

    FUNCTION;
    // try to complete initialization
    if (transport == NULL) {
        if (!Option(weoTransport).IsEmpty()) {
            Option(weoTransport).GetValue(name);
            // create named transport
            //transport = WeCreateNamedTransport(name);
        }
    }
    return (transport != NULL);
}

void WeTask::SetTransport(const  string& transp )
{
    FUNCTION;
    /// @todo Cleanup previous transport
    Option(weoTransport, transp);
    //transport = WeCreateNamedTransport(transp);
}

void WeTask::SetTransport( iweTransport* transp )
{
    FUNCTION;
    transport = transp;
    if (transport != NULL) {
        Option(weoTransport, transport->GetName());
    }
    else {
        Option(weoTransport, string(""));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn std::string WeTask::ToXml( void )
///
/// @brief  Converts this object to an XML. 
///
/// This function realizes alternate serialization mechanism. It generates more compact and
/// simplified XML representation. This representation is used for internal data exchange
/// (for example to store data through iweStorage interface).
///
/// @retval This object as a std::string. 
////////////////////////////////////////////////////////////////////////////////////////////////////
std::string WeTask::ToXml( void )
{
    string retval;
    string optList;
    int optCount;
    int optType;
    WeOption optVal;
    string strData;
    int    intData;
    unsigned int uintData;
    char   chData;
    unsigned char uchData;
    long    longData;
    unsigned long ulongData;
    bool    boolData;
    double  doubleData;
    WeOptions::iterator it;

    retval = "";

    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask::ToXml");
    optVal = Option(weoID);
    SAFE_GET_OPTION_VAL(optVal, strData, "");
    retval += "<task id='" + WeScreenXML(strData) + "'>\n";

    optVal = Option(weoName);
    SAFE_GET_OPTION_VAL(optVal, strData, "");
    retval += "  <name>" + WeScreenXML(strData) + "</name>\n";

    optVal = Option(weoTaskStatus);
    SAFE_GET_OPTION_VAL(optVal, intData, 0);
    retval += "  <status>" + boost::lexical_cast<string>(intData) + "</status>\n";

    optVal = Option(weoTaskCompletion);
    SAFE_GET_OPTION_VAL(optVal, intData, 0);
    retval += "  <completion>" + boost::lexical_cast<string>(intData) + "</completion>\n";

    optCount = 0;
    optList = "";
    for (it = options.begin(); it != options.end(); it++) {
        strData = it->first;
        // skip predefined options
        if (strData == weoID) {
            continue;
        }
        if (strData == weoName) {
            continue;
        }
        if (strData == weoTaskStatus) {
            continue;
        }
        if (strData == weoTaskCompletion) {
            continue;
        }
        optType = it->second->Which();
        try
        {
            switch(optType)
            {
            case 0:
                it->second->GetValue(chData);
                strData = boost::lexical_cast<string>(chData); 
                break;
            case 1:
                it->second->GetValue(uchData);
                strData = boost::lexical_cast<string>(uchData); 
                break;
            case 2:
                it->second->GetValue(intData);
                strData = boost::lexical_cast<string>(intData); 
                break;
            case 3:
                it->second->GetValue(uintData);
                strData = boost::lexical_cast<string>(uintData); 
                break;
            case 4:
                it->second->GetValue(longData);
                strData = boost::lexical_cast<string>(longData); 
                break;
            case 5:
                it->second->GetValue(ulongData);
                strData = boost::lexical_cast<string>(ulongData); 
                break;
            case 6:
                it->second->GetValue(boolData);
                strData = boost::lexical_cast<string>(boolData); 
                break;
            case 7:
                it->second->GetValue(doubleData);
                strData = boost::lexical_cast<string>(doubleData); 
                break;
            case 8:
                it->second->GetValue(strData);
                break;
            default:
                //optVal = *(it->second);
                strData = "";
            }
        }
        catch (...)
        {
        	strData = "";
        }
        optCount++;
        strData = WeScreenXML(strData);
        optList += "    <option name='" + it->first + "' type='" + boost::lexical_cast<string>(optType) + "'>" + strData + "</option>\n";
    }
    if (optCount > 0)
    {
        retval += "  <options count='" + boost::lexical_cast<string>(optCount) + "'>\n";
        retval += optList;
        retval += "  </options>\n";
    }

    retval += "</task>\n";
    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn void WeTask::FromXml( string input )
///
/// @brief  Initializes this object from the given from XML. 
///
/// This function reconstructs object back from the XML generated by the @b ToXml function
///
/// @param  input - The input XML. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void WeTask::FromXml( string input )
{
    WeStrStream st(input.c_str());
    WeTagScanner sc(st);

    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask::FromXml - string");
    FromXml(sc);
}

void WeTask::FromXml( WeTagScanner& sc, int token /*= -1*/ )
{
    int pos;
    int parseLevel = 0;
    int intData;
    bool inParsing = true;
    string name, val, dat;

    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask::FromXml - WeTagScanner");
    while (inParsing)
    {
        pos = sc.GetPos();
        if (token == -1) {
            token = sc.GetToken();
        }
        switch(token)
        {
        case wstError:
            LOG4CXX_WARN(WeLogger::GetLogger(), "WeTask::FromXml parsing error");
            inParsing = false;
            break;
        case wstEof:
            LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask::FromXml - EOF");
            inParsing = false;
            break;
        case wstTagStart:
            name = sc.GetTagName();
            if (parseLevel == 0)
            {
                if (iequals(name, "task"))
                {
                    parseLevel = 1;
                    dat = "";
                }
                else {
                    LOG4CXX_WARN(WeLogger::GetLogger(), "WeTask::FromXml unexpected tagStart: " << name);
                    inParsing = false;
                }
                break;
            }
            if (parseLevel == 1)
            {
                parseLevel = 2;
                dat = "";
                if (iequals(name, "options"))
                {
                    iweOptionsProvider::FromXml(sc, token);
                    parseLevel = 1;
                }
                break;
            }
            LOG4CXX_WARN(WeLogger::GetLogger(), "WeTask::FromXml unexpected tagStart: " << name);
            inParsing = false;
        	break;
        case wstTagEnd:
            name = sc.GetTagName();
            if (parseLevel == 1)
            {
                if (iequals(name, "task"))
                {
                    parseLevel = 0;
                    dat = "";
                    inParsing = false;
                }
                else {
                    LOG4CXX_WARN(WeLogger::GetLogger(), "WeTask::FromXml unexpected wstTagEnd: " << name);
                    inParsing = false;
                }
            }
            if (parseLevel == 2)
            {
                dat = WeUnscreenXML(dat);
                if (iequals(name, "name"))
                {
                    Option(weoName, dat);
                }
                if (iequals(name, "status"))
                {
                    intData = boost::lexical_cast<int>(dat);
                    Option(weoTaskStatus, intData);
                }
                if (iequals(name, "completion"))
                {
                    intData = boost::lexical_cast<int>(dat);
                    Option(weoTaskCompletion, intData);
                }
                parseLevel = 1;
            }
            break;
        case wstAttr:
            name = sc.GetAttrName();
            val = sc.GetValue();
            val = WeUnscreenXML(val);
            if (parseLevel == 1)
            {
                if (iequals(name, "id"))
                {
                    Option(weoID, val);
                }
            }
            break;
        case wstWord: 
        case wstSpace:
            dat += sc.GetValue();
            break;
        default:
            break;
        }
        token = -1;
    }
}