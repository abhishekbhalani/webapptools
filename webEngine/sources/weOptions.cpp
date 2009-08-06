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
#include "weOptions.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>

#ifndef __DOXYGEN__
static const WeOption empty_option("_empty_");
#endif //__DOXYGEN__


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
std::string iweOptionsProvider::ToXml( void )
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

    optCount = 0;
    optList = "";
    for (it = options.begin(); it != options.end(); it++) {
        strData = it->first;
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
        optList += "  <option name='" + it->first + "' type='" + boost::lexical_cast<string>(optType) + "'>" + strData + "</option>\n";
    }
    if (optCount > 0)
    {
        retval += "<options count='" + boost::lexical_cast<string>(optCount) + "'>\n";
        retval += optList;
        retval += "</options>\n";
    }
    return retval;
}

void iweOptionsProvider::FromXml( string input )
{
    WeStrStream st(input.c_str());
    WeTagScanner sc(st);

    LOG4CXX_TRACE(WeLogger::GetLogger(), "iweOptionsProvider::FromXml - string");
    FromXml(sc);
}

void iweOptionsProvider::FromXml( WeTagScanner& sc, int token /*= -1*/ )
{
    bool inParsing = true;
    int  parseLevel = 0;
    string name, val, dat;
    string optName;
    int optType;
    string strData;
    int    intData;
    unsigned int uintData;
    char   chData;
    unsigned char uchData;
    long    longData;
    unsigned long ulongData;
    bool    boolData;
    double  doubleData;

    LOG4CXX_TRACE(WeLogger::GetLogger(), "iweOptionsProvider::FromXml - WeTagScanner");
    while (inParsing)
    {
        if (token == -1)
        {
            token = sc.GetToken();
        }
        switch(token)
        {
        case wstError:
            LOG4CXX_WARN(WeLogger::GetLogger(), "iweOptionsProvider::FromXml parsing error");
            inParsing = false;
            break;
        case wstEof:
            LOG4CXX_TRACE(WeLogger::GetLogger(), "iweOptionsProvider::FromXml - EOF");
            inParsing = false;
            break;
        case wstTagStart:
            name = sc.GetTagName();
//             if (name[0] == '?') {
//                 // just skip this tag
//                 parseLevel *= -1;
//                 break;
//             }
            if (parseLevel == 0)
            {
                if (iequals(name, "options"))
                {
                    parseLevel = 1;
                    dat = "";
                }
                else {
                    LOG4CXX_WARN(WeLogger::GetLogger(), "iweOptionsProvider::FromXml unexpected tagStart: " << name);
                    inParsing = false;
                }
            }
            else
            {
                if (iequals(name, "option"))
                {
                    parseLevel = 2;
                    dat = "";
                    optName = "";
                    optType = -1;
                }
                else {
                    LOG4CXX_WARN(WeLogger::GetLogger(), "iweOptionsProvider::FromXml unexpected tagStart: " << name);
                    inParsing = false;
                }
            }
            break;
        case wstTagEnd:
            name = sc.GetTagName();
//             if (name[0] == '?') {
//                 // just skip this tag
//                 if (parseLevel < 0) {
//                     parseLevel *= -1;
//                 }
//                 break;
//             }
            if (parseLevel == 1)
            {
                if (iequals(name, "options"))
                {
                    parseLevel = 0;
                    // we are parse only one set of options,
                    // so - stop parsing now
                    inParsing = false;
                }
                else {
                    LOG4CXX_WARN(WeLogger::GetLogger(), "iweOptionsProvider::FromXml unexpected tagEnd: " << name);
                    inParsing = false;
                }
            }
            else
            {
                if (iequals(name, "option"))
                {
                    // save option
                    dat = WeUnscreenXML(dat);
                    LOG4CXX_TRACE(WeLogger::GetLogger(), "iweOptionsProvider::FromXml save option "
                                << optName << "(" << optType << ") = " << dat);
                    switch(optType)
                    {
                    case 0:
                        chData = boost::lexical_cast<char>(dat);
                        Option(optName, chData);
                    	break;
                    case 1:
                        uchData = boost::lexical_cast<unsigned char>(dat);
                        Option(optName, uchData);
                    	break;
                    case 2:
                        intData = boost::lexical_cast<int>(dat);
                        Option(optName, intData);
                        break;
                    case 3:
                        uintData = boost::lexical_cast<unsigned int>(dat);
                        Option(optName, uintData);
                        break;
                    case 4:
                        longData = boost::lexical_cast<long>(dat);
                        Option(optName, longData);
                    	break;
                    case 5:
                        ulongData = boost::lexical_cast<unsigned long>(dat);
                        Option(optName, ulongData);
                    	break;
                    case 6:
                        boolData = boost::lexical_cast<bool>(dat);
                        Option(optName, boolData);
                        break;
                    case 7:
                        doubleData = boost::lexical_cast<double>(dat);
                        Option(optName, doubleData);
                        break;
                    case 8:
                        Option(optName, dat);
                        break;
                    default:
                        break;
                    }
                    parseLevel = 1;
                }
                else {
                    LOG4CXX_WARN(WeLogger::GetLogger(), "iweOptionsProvider::FromXml unexpected tagEnd: " << name);
                    inParsing = false;
                }
            }
            break;
        case wstAttr:
            name = sc.GetAttrName();
            val = sc.GetValue();
            val = WeUnscreenXML(val);
            if (parseLevel == 2)
            {
                if (iequals(name, "name"))
                {
                    optName = val;
                }
                if (iequals(name, "type"))
                {
                    optType = boost::lexical_cast<int>(val);
                }
            }
            break;
        case wstWord: 
        case wstSpace:
            dat += sc.GetValue();
            break;
        }
        token = -1;
    }
}

WeOption& iweOptionsProvider::Option( const string& name )
{
    WeOptions::iterator it;

    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask::Option(" << name << ")");
    it = options.find(name);
    if (it != options.end())
    {
        return *(it->second);
    }
    return *((WeOption*)&empty_option);
}

void iweOptionsProvider::Option( const string& name, WeOptionVal val )
{
    WeOptions::iterator it;
    WeOption* opt;

    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask::Option(" << name << ") set value=" << val);
    it = options.find(name);
    if (it != options.end())
    {
        opt = it->second;
        opt->SetValue(val);
    }
    else {
        opt = new WeOption();
        opt->Name(name);
        opt->SetValue(val);
        options[name] = opt;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	bool WeTask::IsSet(const string& name )
///
/// @brief	Query if bool options 'name' is set to true.
///
/// @param	name - The option name.
///
/// @retval	true if set, false if not.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool iweOptionsProvider::IsSet( const string& name )
{
    WeOptions::iterator it;
    WeOption* opt;
    bool retval = false;

    FUNCTION;
    it = options.find(name);
    if (it != options.end())
    {
        opt = it->second;
        try
        {
            opt->GetValue(retval);
        }
        catch (...)
        {
            retval = false;
        }
    }
    return retval;
}