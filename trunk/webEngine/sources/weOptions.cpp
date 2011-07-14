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
we_option i_options_provider::empty_option("_empty_");
#endif //__DOXYGEN__

void i_options_provider::CopyOptions( i_options_provider* cpy )
{
    string_list opt_names;

    opt_names = cpy->OptionsList();

    for (size_t i = 0; i < opt_names.size(); i++) {
        we_option opt = cpy->Option(opt_names[i]);
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

we_option options_provider::Option( const string& name )
{
    wOptions::iterator it;

    //LOG4CXX_TRACE(iLogger::GetLogger(), "options_provider::Option(" << name << ")");
    it = options.find(name);
    if (it != options.end()) {
        return (it->second);
    }
    return empty_option;
}

void options_provider::Option( const string& name, we_variant val )
{
    wOptions::iterator it;

    //LOG4CXX_TRACE(iLogger::GetLogger(), "options_provider::Option(" << name << ") set value=" << val);
    it = options.find(name);
    if (it != options.end()) {
        it->second.SetValue(val);
    } else {
        we_option opt;
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
    if (it != options.end()) {
        try {
            it->second.GetValue(retval);
        } catch (...) {
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

    for (it = options.begin(); it != options.end(); it++) {
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
    we_option optVal;
    string strData;
    wOptions::iterator it;

    retval = "";

    optCount = 0;
    optList = "";
    for (it = options.begin(); it != options.end(); it++) {
        strData = it->first;
        optType = it->second.Value().which();
        try {
            strData = boost::lexical_cast<string>(it->second.Value());
        } catch (bad_cast &e) {
			LOG4CXX_ERROR(iLogger::GetLogger(), "options_provider::ToXml exception - " << std::string(e.what()));
            strData = "";
        }
        optCount++;
        strData = ScreenXML(strData);
        optList += "  <option name='" + it->first + "' type='" + boost::lexical_cast<string>(optType) + "'>" + strData + "</option>\n";
    }
    if (optCount > 0) {
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
    char   chData;
    bool   boolData;
    double doubleData;
    boost::blank empt;

    LOG4CXX_TRACE(iLogger::GetLogger(), "options_provider::FromXml - tag_scanner");
    while (inParsing) {
        if (token == -1) {
            token = sc.get_token();
        }
        switch(token) {
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
            if (parseLevel == 0) {
                if (iequals(name, "options")) {
                    parseLevel = 1;
                    dat = "";
                } else {
                    LOG4CXX_WARN(iLogger::GetLogger(), "options_provider::FromXml unexpected tagStart: " << name);
                    inParsing = false;
                }
            } else {
                if (iequals(name, "option")) {
                    parseLevel = 2;
                    dat = "";
                    optName = "";
                    optType = -1;
                } else {
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
            if (parseLevel == 1) {
                if (iequals(name, "options")) {
                    parseLevel = 0;
                    // we are parse only one set of options,
                    // so - stop parsing now
                    inParsing = false;
                } else {
                    LOG4CXX_WARN(iLogger::GetLogger(), "options_provider::FromXml unexpected tagEnd: " << name);
                    inParsing = false;
                }
            } else {
                if (iequals(name, "option")) {
                    // save option
                    dat = UnscreenXML(dat);
                    LOG4CXX_TRACE(iLogger::GetLogger(), "options_provider::FromXml save option "
                                  << optName << "(" << optType << ") = " << dat);
                    switch(optType) {
                    case 0:
                        chData = boost::lexical_cast<char>(dat);
                        Option(optName, chData);
                        break;
                    case 1:
                        intData = boost::lexical_cast<int>(dat);
                        Option(optName, intData);
                        break;
                    case 2:
                        boolData = boost::lexical_cast<bool>(dat);
                        Option(optName, boolData);
                        break;
                    case 3:
                        doubleData = boost::lexical_cast<double>(dat);
                        Option(optName, doubleData);
                        break;
                    case 4:
                        Option(optName, dat);
                        break;
                    default:
                        Option(optName, empt);
                        break;
                    }
                    parseLevel = 1;
                } else {
                    LOG4CXX_WARN(iLogger::GetLogger(), "options_provider::FromXml unexpected tagEnd: " << name);
                    inParsing = false;
                }
            }
            break;
        case wstAttr:
            name = sc.get_attr_name();
            val = sc.get_value();
            val = UnscreenXML(val);
            if (parseLevel == 2) {
                if (iequals(name, "name")) {
                    optName = val;
                }
                if (iequals(name, "type")) {
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
