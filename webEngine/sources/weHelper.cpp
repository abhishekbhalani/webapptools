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
#include <string.h>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/operations.hpp>
#include <curl/curl.h>
#include "weHelper.h"
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace boost::algorithm;
using namespace boost::filesystem;

namespace webEngine {

HtmlFactory  weHtmlFactory;
static bool isLibInited = false;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void LibInit(void)
///
/// @brief  WebEngine library initialize.
/// @throw  WeError if cURL library doesn't initialized
////////////////////////////////////////////////////////////////////////////////////////////////////
void LibInit(const string& config /*= ""*/)
{
    CURLcode    cCode;
    string      cfgFile;

    try {
        cfgFile = config;
        if (cfgFile == "") {
            cfgFile = "trace.config";
        }
        path p(cfgFile);
        if (exists(p))
        {
#ifdef WIN32
            PropertyConfigurator::configure(log4cxx::File(cfgFile), LOG4CXX_STR("webEngine"));
#else
            PropertyConfigurator::configure(log4cxx::File("trace.config"));
#endif
        }
        else {
            // file not exist - default init
            BasicConfigurator::configure();
        }
    }
    catch (...) {
        try {
            BasicConfigurator::configure();
        }
        catch (...) {
            throw WeError("Can't initialize logging subsystem!");
        }
    };
    LOG4CXX_TRACE(iLogger::GetLogger(), "LibInit status = " << isLibInited);
    if (!isLibInited)
    {
        weHtmlFactory.Init();
        cCode = curl_global_init(CURL_GLOBAL_ALL);
        if (cCode != 0) {
            throw WeError(string("cURL initialization failed : ") + boost::lexical_cast<std::string>(cCode));
        }
    }
    isLibInited = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void LibClose(void)
///
/// @brief  WebEngine library finalization.
////////////////////////////////////////////////////////////////////////////////////////////////////
void LibClose(void)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "LibClose status = " << isLibInited);
    if (!isLibInited) {
        return;
    }
    curl_global_cleanup();

    isLibInited = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn string ScreenXML(const string& xml)
///
/// @brief  Screens the XML entities in the input string. 
///
/// @param  xml - The input. 
///
/// @retval	string - screened data. 
////////////////////////////////////////////////////////////////////////////////////////////////////
string ScreenXML( const string& xml )
{
    string retval = xml;
    const char* pre_expression = "(<)|(>)|(&)|(')|(\")";
    const char* pre_format = "(?1&lt;)(?2&gt;)(?3&amp;)(?4&#39;)(?5&quot;)";

    boost::regex rx;
    rx.assign(pre_expression);

    retval = boost::regex_replace(retval, rx, pre_format, boost::match_default | boost::format_all);
    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn string UnscreenXML(const string& xml)
///
/// @brief  Unscreens the XML entities in the input string.  
///
/// @param  xml - The input. 
///
/// @retval	string - unscreened data. 
////////////////////////////////////////////////////////////////////////////////////////////////////
string UnscreenXML(const string& xml)
{
    string retval = xml;
    const char* pre_expression = "(&lt;)|(&gt;)|(&amp;)|(&#39;)|(&quot;)";
    const char* pre_format = "(?1<)(?2>)(?3&)(?4')(?5\")";

    boost::regex rx;
    rx.assign(pre_expression);

    retval = boost::regex_replace(retval, rx, pre_format, boost::match_default | boost::format_all);
    return retval;
}

#ifndef __DOXYGEN__
//////////////////////////////////////////////////////////////////////////
// HTML elements creation functions
//////////////////////////////////////////////////////////////////////////
static HtmlEntity* weCreateRefObj(iEntity* prnt)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "HtmlFactory: create WeRefrenceObject");
    return new WeRefrenceObject(prnt);
}

static HtmlEntity* weCreateText(iEntity* prnt)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "HtmlFactory: create WeInnerText");
    return new WeInnerText(prnt);
}

static HtmlEntity* weCreateComment(iEntity* prnt)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "HtmlFactory: create WeHtmlComment");
    return new WeHtmlComment(prnt);
}

static HtmlEntity* weCreateCData(iEntity* prnt)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "HtmlFactory: create WeCData");
    return new WeCData(prnt);
}

static HtmlEntity* weCreatePhpInc(iEntity* prnt)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "HtmlFactory: create WePhpInclude");
    return new WePhpInclude(prnt);
}

static HtmlEntity* weCreateDocument(iEntity* prnt)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "HtmlFactory: create WeHtmlDocument");
    return NULL; //new WeDocument(prnt);
}

static HtmlEntity* weCreateScript(iEntity* prnt)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "HtmlFactory: create WeScript");
    return new WeScript(prnt);
}

HtmlFactory::HtmlFactory() :
    LinkedList<string, fnEntityFactory>()
{
    data = new LinkedListElem<string, fnEntityFactory>;
    data->Key("");
    data->Value(NULL);
    data->Link(NULL);
}

void HtmlFactory::Add( string name, fnEntityFactory func )
{
    LinkedListElem<string, fnEntityFactory>* obj;

    LOG4CXX_TRACE(iLogger::GetLogger(), "new EntityFactory added for " << name);
    obj = new LinkedListElem<string, fnEntityFactory>();
    obj->Key(name);
    obj->Value(func);
    data->Add(obj);
}

void HtmlFactory::Init()
{
    Clear();
    data = new LinkedListElem<string, fnEntityFactory>;
    data->Key("");
    data->Value(NULL);
    data->Link(NULL);
    Add("#document",    weCreateDocument);
    Add("#text",        weCreateText);
    Add("#comment",     weCreateComment);
    Add("#cdata",       weCreateCData);
    Add("#php",         weCreatePhpInc);
//     Add("img",          weCreateRefObj);
//     Add("frame",        weCreateRefObj);
//     Add("iframe",       weCreateRefObj);
//     Add("script",       weCreateScript);
//     Add("a",            weCreateRefObj);
}
#endif //__DOXYGEN__

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	iEntity* HtmlFactory::CreateEntity(string tagName, HtmlEntity* prnt)
///
/// @brief  Create HtmlEntity by the given TAG name.
/// @param  tagName - Name of the html tag.
/// @param  prnt    - Parent of the newly created object
/// @retval	null if it fails, pointer to HtmlEntity ot it's successor else.
////////////////////////////////////////////////////////////////////////////////////////////////////
HtmlEntity* HtmlFactory::CreateEntity( string tagName, HtmlEntity* prnt )
{
    fnEntityFactory func;

    LOG4CXX_TRACE(iLogger::GetLogger(), "HtmlFactory::CreateEntity => " << tagName);
    func = FindFirst(tagName);
    if (func == NULL) {
        return new HtmlEntity(prnt);
    }
    return func(prnt);
}

} // namespace webEngine
