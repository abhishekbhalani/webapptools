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
#include <weHelper.h>
#include <weOptions.h>
#include <weiStorage.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/functional/hash.hpp>

namespace webEngine {

#ifndef __DOXYGEN__
static const wOption empty_option("_empty_");
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
RecordSet* iOptionsProvider::ToRS( const string& parentID/* = ""*/ )
{
    RecordSet* res = new RecordSet;
    Record* rec;
    wOptions::iterator it;
    wOptionVal optVal;
    string strData;

    for (it = options.begin(); it != options.end(); it++) {
        strData = it->first;
        optVal = it->second->Value();
        rec = new Record;
        rec->objectID = weObjTypeSysOption;
        rec->Option(weoName, strData);
        rec->Option(weoParentID, parentID);
        rec->Option(weoTypeID, it->second->Which());
        rec->Option(weoValue, optVal);
        strData += parentID;
        strData += boost::lexical_cast<string>(it->second->Which());
        boost::hash<string> strHash;
        size_t hs = strHash(strData);
        strData = boost::lexical_cast<string>(hs);
        rec->Option(weoID, strData);
        res->push_back(*rec);
    }

    return res;
}

void iOptionsProvider::FromRS( RecordSet *rs )
{
    Record rec;
    size_t r;
    int tp;
    wOptionVal optVal;
    wOption opt;
    string sName, strData;
    char c;
    unsigned char uc;
    int i;
    unsigned int ui;
    long l;
    unsigned long ul;
    bool b;
    double d;

    for (r = 0; r < rs->size(); r++)
    {
        rec = (*rs)[r];
        if (rec.objectID == weObjTypeSysOption) {
            opt = rec.Option(weoName);
            SAFE_GET_OPTION_VAL(opt, sName, "");
            // type doesn't matter here
            opt = rec.Option(weoTypeID);
            SAFE_GET_OPTION_VAL(opt, strData, "8");
            tp = boost::lexical_cast<int>(strData);
            opt = rec.Option(weoValue);
            SAFE_GET_OPTION_VAL(opt, strData, "");
            switch(tp)
            {
            case 0:
                c = boost::lexical_cast<char>(strData);
                Option(sName, c);
            	break;
            case 1:
                uc = boost::lexical_cast<unsigned char>(strData);
                Option(sName, uc);
            	break;
            case 2:
                i = boost::lexical_cast<int>(strData);
                Option(sName, i);
                break;
            case 3:
                ui = boost::lexical_cast<unsigned int>(strData);
                Option(sName, ui);
                break;
            case 4:
                l = boost::lexical_cast<long>(strData);
                Option(sName, l);
            	break;
            case 5:
                ul = boost::lexical_cast<unsigned long>(strData);
                Option(sName, ul);
            	break;
            case 6:
                b = boost::lexical_cast<bool>(strData);
                Option(sName, b);
                break;
            case 7:
                d = boost::lexical_cast<double>(strData);
                Option(sName, d);
                break;
            case 8:
            default:
                Option(sName, strData);
                break;
            }
        }
    }
}

wOption& iOptionsProvider::Option( const string& name )
{
    wOptions::iterator it;

    LOG4CXX_TRACE(iLogger::GetLogger(), "iOptionsProvider::Option(" << name << ")");
    it = options.find(name);
    if (it != options.end())
    {
        return *(it->second);
    }
    return *((wOption*)&empty_option);
}

void iOptionsProvider::Option( const string& name, wOptionVal val )
{
    wOptions::iterator it;
    wOption* opt;

    LOG4CXX_TRACE(iLogger::GetLogger(), "iOptionsProvider::Option(" << name << ") set value=" << val);
    it = options.find(name);
    if (it != options.end())
    {
        opt = it->second;
        opt->SetValue(val);
    }
    else {
        opt = new wOption();
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
bool iOptionsProvider::IsSet( const string& name )
{
    wOptions::iterator it;
    wOption* opt;
    bool retval = false;

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
    LOG4CXX_TRACE(iLogger::GetLogger(), "iOptionsProvider::IsSet(" << name << ") value=" << retval);
    return retval;
}

void iOptionsProvider::CopyOptions( iOptionsProvider* cpy )
{
    wOptions::iterator it;

    for (it = cpy->options.begin(); it != cpy->options.end(); it++)
    {
        options[it->first] = it->second;
    }
}

StringList iOptionsProvider::OptionsList()
{
    StringList retval;

    retval.clear();
    wOptions::iterator it;

    for (it = options.begin(); it != options.end(); it++)
    {
        retval.push_back(it->first);
    }

    return retval;
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
std::string iOptionsProvider::ToXml( void )
{
    string retval;
    string optList;
    int optCount;
    int optType;
    wOption optVal;
    string strData;
    int    intData;
    unsigned int uintData;
    char   chData;
    unsigned char uchData;
    long    longData;
    unsigned long ulongData;
    bool    boolData;
    double  doubleData;
    wOptions::iterator it;

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
        strData = ScreenXML(strData);
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

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn void iOptionsProvider::FromXml( string input )
///
/// @brief  Initializes this object from the given from
/// 		XML. It reconstruct object from compact and
///         simplified XML representation, generated by
///         ToXml function
///
/// @param  input - The input. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void iOptionsProvider::FromXml( string input )
{
    StrStream st(input.c_str());
    TagScanner sc(st);

    LOG4CXX_TRACE(iLogger::GetLogger(), "iOptionsProvider::FromXml - string");
    FromXml(sc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn void iOptionsProvider::FromXml( TagScanner& sc,
/// 	int token )
///
/// @brief  Initializes this object from the given from
/// 		XML. It reconstruct object from compact and
///         simplified XML representation, generated by
///         ToXml function
///
/// @param  sc      - the sc. 
/// @param  token   - The token. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void iOptionsProvider::FromXml( TagScanner& sc, int token /*= -1*/ )
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

    LOG4CXX_TRACE(iLogger::GetLogger(), "iOptionsProvider::FromXml - TagScanner");
    while (inParsing)
    {
        if (token == -1)
        {
            token = sc.GetToken();
        }
        switch(token)
        {
        case wstError:
            LOG4CXX_WARN(iLogger::GetLogger(), "iOptionsProvider::FromXml parsing error");
            inParsing = false;
            break;
        case wstEof:
            LOG4CXX_TRACE(iLogger::GetLogger(), "iOptionsProvider::FromXml - EOF");
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
                    LOG4CXX_WARN(iLogger::GetLogger(), "iOptionsProvider::FromXml unexpected tagStart: " << name);
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
                    LOG4CXX_WARN(iLogger::GetLogger(), "iOptionsProvider::FromXml unexpected tagStart: " << name);
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
                    LOG4CXX_WARN(iLogger::GetLogger(), "iOptionsProvider::FromXml unexpected tagEnd: " << name);
                    inParsing = false;
                }
            }
            else
            {
                if (iequals(name, "option"))
                {
                    // save option
                    dat = UnscreenXML(dat);
                    LOG4CXX_TRACE(iLogger::GetLogger(), "iOptionsProvider::FromXml save option "
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
                    LOG4CXX_WARN(iLogger::GetLogger(), "iOptionsProvider::FromXml unexpected tagEnd: " << name);
                    inParsing = false;
                }
            }
            break;
        case wstAttr:
            name = sc.GetAttrName();
            val = sc.GetValue();
            val = UnscreenXML(val);
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

} // namespace webEngine
