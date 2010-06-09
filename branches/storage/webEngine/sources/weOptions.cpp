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
#include <webEngine.h>

#include <weHelper.h>
#include <weOptions.h>
#include <weiStorage.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/functional/hash.hpp>

namespace webEngine {

#ifndef __DOXYGEN__
wOption i_options_provider::empty_option("_empty_");
#endif //__DOXYGEN__

void i_options_provider::CopyOptions( i_options_provider* cpy )
{
    string_list opt_names;

    opt_names = cpy->OptionsList();

    for (size_t i = 0; i < opt_names.size(); i++) {
        wOption opt = cpy->Option(opt_names[i]);
        Option(opt_names[i], opt.Value());
    }
}

options_provider::~options_provider()
{
    wOptions::iterator optit;

//     for (optit = options.begin(); optit != options.end(); optit++) {
//         delete optit->second;
//         optit->second = NULL;
//     }
    options.clear();
}

db_recordset* options_provider::ToRS( const string& parentID/* = ""*/ )
{
    db_recordset* res = new db_recordset;
    db_record* rec;
    wOptions::iterator it;
    wOptionVal optVal;
    string strData;

    for (it = options.begin(); it != options.end(); it++) {
        strData = it->first;
        optVal = it->second.Value();
        rec = new db_record;
        rec->objectID = weObjTypeSysOption;
        rec->Option(weoName, strData);
        rec->Option(weoParentID, parentID);
        rec->Option(weoTypeID, it->second.Which());
        rec->Option(weoValue, optVal);
        strData += parentID;
        strData += boost::lexical_cast<string>(it->second.Which());
        boost::hash<string> strHash;
        size_t hs = strHash(strData);
        strData = boost::lexical_cast<string>(hs);
        rec->Option(weoID, strData);
        res->push_back(*rec);
    }

    return res;
}

void options_provider::FromRS( db_recordset *rs )
{
    db_record rec;
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
            strData = boost::lexical_cast<string>(opt.Value());
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

wOption options_provider::Option( const string& name )
{
    wOptions::iterator it;

    //LOG4CXX_TRACE(iLogger::GetLogger(), "options_provider::Option(" << name << ")");
    it = options.find(name);
    if (it != options.end())
    {
        return (it->second);
    }
    return empty_option;
}

void options_provider::Option( const string& name, wOptionVal val )
{
    wOptions::iterator it;

    //LOG4CXX_TRACE(iLogger::GetLogger(), "options_provider::Option(" << name << ") set value=" << val);
    it = options.find(name);
    if (it != options.end())
    {
        it->second.SetValue(val);
    }
    else {
        wOption opt;
        opt.name(name);
        opt.SetValue(val);
        options[name] = opt;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	bool options_provider::IsSet(const string& name )
///
/// @brief	Query if bool options 'name' is set to true.
///
/// @param	name - The option name.
///
/// @retval	true if set, false if not.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool options_provider::IsSet( const string& name )
{
    wOptions::iterator it;
    bool retval = false;

    it = options.find(name);
    if (it != options.end())
    {
        try
        {
            it->second.GetValue(retval);
        }
        catch (...)
        {
            retval = false;
        }
    }
    //LOG4CXX_TRACE(iLogger::GetLogger(), "options_provider::IsSet(" << name << ") value=" << retval);
    return retval;
}

string_list options_provider::OptionsList()
{
    string_list retval;

    retval.clear();
    wOptions::iterator it;

    for (it = options.begin(); it != options.end(); it++)
    {
        retval.push_back(it->first);
    }

    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn std::string options_provider::ToXml( void )
///
/// @brief  Converts this object to an XML. 
///
/// This function realizes alternate serialization mechanism. It generates more compact and
/// simplified XML representation. This representation is used for internal data exchange
/// (for example to store data through iweStorage interface).
///
/// @retval This object as a std::string. 
////////////////////////////////////////////////////////////////////////////////////////////////////
std::string options_provider::ToXml( void )
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
        optType = it->second.Which();
        try
        {
            switch(optType)
            {
            case 0:
                it->second.GetValue(chData);
                strData = boost::lexical_cast<string>(chData); 
                break;
            case 1:
                it->second.GetValue(uchData);
                strData = boost::lexical_cast<string>(uchData); 
                break;
            case 2:
                it->second.GetValue(intData);
                strData = boost::lexical_cast<string>(intData); 
                break;
            case 3:
                it->second.GetValue(uintData);
                strData = boost::lexical_cast<string>(uintData); 
                break;
            case 4:
                it->second.GetValue(longData);
                strData = boost::lexical_cast<string>(longData); 
                break;
            case 5:
                it->second.GetValue(ulongData);
                strData = boost::lexical_cast<string>(ulongData); 
                break;
            case 6:
                it->second.GetValue(boolData);
                strData = boost::lexical_cast<string>(boolData); 
                break;
            case 7:
                it->second.GetValue(doubleData);
                strData = boost::lexical_cast<string>(doubleData); 
                break;
            case 8:
                it->second.GetValue(strData);
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
/// @fn void options_provider::FromXml( string input )
///
/// @brief  Initializes this object from the given from
/// 		XML. It reconstruct object from compact and
///         simplified XML representation, generated by
///         ToXml function
///
/// @param  input - The input. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void options_provider::FromXml( string input )
{
    str_tag_stream st(input.c_str());
    tag_scanner sc(st);

    LOG4CXX_TRACE(iLogger::GetLogger(), "options_provider::FromXml - string");
    FromXml(sc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn void options_provider::FromXml( tag_scanner& sc,
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
void options_provider::FromXml( tag_scanner& sc, int token /*= -1*/ )
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

    LOG4CXX_TRACE(iLogger::GetLogger(), "options_provider::FromXml - tag_scanner");
    while (inParsing)
    {
        if (token == -1)
        {
            token = sc.get_token();
        }
        switch(token)
        {
        case wstError:
            LOG4CXX_WARN(iLogger::GetLogger(), "options_provider::FromXml parsing error");
            inParsing = false;
            break;
        case wstEof:
            LOG4CXX_TRACE(iLogger::GetLogger(), "options_provider::FromXml - EOF");
            inParsing = false;
            break;
        case wstTagStart:
            name = sc.get_tag_name();
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
                    LOG4CXX_WARN(iLogger::GetLogger(), "options_provider::FromXml unexpected tagStart: " << name);
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
                    LOG4CXX_WARN(iLogger::GetLogger(), "options_provider::FromXml unexpected tagStart: " << name);
                    inParsing = false;
                }
            }
            break;
        case wstTagEnd:
            name = sc.get_tag_name();
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
                    LOG4CXX_WARN(iLogger::GetLogger(), "options_provider::FromXml unexpected tagEnd: " << name);
                    inParsing = false;
                }
            }
            else
            {
                if (iequals(name, "option"))
                {
                    // save option
                    dat = UnscreenXML(dat);
                    LOG4CXX_TRACE(iLogger::GetLogger(), "options_provider::FromXml save option "
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
                    LOG4CXX_WARN(iLogger::GetLogger(), "options_provider::FromXml unexpected tagEnd: " << name);
                    inParsing = false;
                }
            }
            break;
        case wstAttr:
            name = sc.get_attr_name();
            val = sc.get_value();
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
            dat += sc.get_value();
            break;
        }
        token = -1;
    }
}

} // namespace webEngine
