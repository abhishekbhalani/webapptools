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

WeHtmlFactory  weHtmlFactory;
WeTransportFactory weTransportFactory;
static bool isLibInited = false;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void WeLibInit(void)
///
/// @brief  WebEngine library initialize.
/// @throw  WeError if cURL library doesn't initialized
////////////////////////////////////////////////////////////////////////////////////////////////////
void WeLibInit(void)
{
    CURLcode    cCode;
    FUNCTION;
    try {
        path p("trace.config");
        if (exists(p))
        {
#ifdef WIN32
            PropertyConfigurator::configure(log4cxx::File("trace.config"), LOG4CXX_STR("webEngine"));
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
    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeLibInit status = " << isLibInited);
    if (!isLibInited)
    {
        weHtmlFactory.Init();
        weTransportFactory.Init();
        cCode = curl_global_init(CURL_GLOBAL_ALL);
        if (cCode != 0) {
            throw WeError(string("cURL initialization failed : ") + boost::lexical_cast<std::string>(cCode));
        }
    }
    isLibInited = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void WeLibClose(void)
///
/// @brief  WebEngine library finalization.
////////////////////////////////////////////////////////////////////////////////////////////////////
void WeLibClose(void)
{
    FUNCTION;
    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeLibClose status = " << isLibInited);
    if (!isLibInited) {
        return;
    }
    curl_global_cleanup();

    isLibInited = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn string WeScreenXML(const string& xml)
///
/// @brief  Screens the XML entities in the input string. 
///
/// @param  xml - The input. 
///
/// @retval	string - screened data. 
////////////////////////////////////////////////////////////////////////////////////////////////////
string WeScreenXML( const string& xml )
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
/// @fn string WeUnscreenXML(const string& xml)
///
/// @brief  Unscreens the XML entities in the input string.  
///
/// @param  xml - The input. 
///
/// @retval	string - unscreened data. 
////////////////////////////////////////////////////////////////////////////////////////////////////
string WeUnscreenXML(const string& xml)
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
static WeHtmlEntity* weCreateRefObj(iweEntity* prnt)
{
    FUNCTION;
    LOG4CXX_DEBUG(WeLogger::GetLogger(), "WeHtmlFactory: create WeRefrenceObject");
    return new WeRefrenceObject(prnt);
}

static WeHtmlEntity* weCreateText(iweEntity* prnt)
{
    FUNCTION;
    LOG4CXX_DEBUG(WeLogger::GetLogger(), "WeHtmlFactory: create WeInnerText");
    return new WeInnerText(prnt);
}

static WeHtmlEntity* weCreateComment(iweEntity* prnt)
{
    FUNCTION;
    LOG4CXX_DEBUG(WeLogger::GetLogger(), "WeHtmlFactory: create WeHtmlComment");
    return new WeHtmlComment(prnt);
}

static WeHtmlEntity* weCreateCData(iweEntity* prnt)
{
    FUNCTION;
    LOG4CXX_DEBUG(WeLogger::GetLogger(), "WeHtmlFactory: create WeCData");
    return new WeCData(prnt);
}

static WeHtmlEntity* weCreatePhpInc(iweEntity* prnt)
{
    FUNCTION;
    LOG4CXX_DEBUG(WeLogger::GetLogger(), "WeHtmlFactory: create WePhpInclude");
    return new WePhpInclude(prnt);
}

static WeHtmlEntity* weCreateDocument(iweEntity* prnt)
{
    FUNCTION;
    LOG4CXX_DEBUG(WeLogger::GetLogger(), "WeHtmlFactory: create WeHtmlDocument");
    return NULL; //new WeDocument(prnt);
}

static WeHtmlEntity* weCreateScript(iweEntity* prnt)
{
    FUNCTION;
    LOG4CXX_DEBUG(WeLogger::GetLogger(), "WeHtmlFactory: create WeScript");
    return new WeScript(prnt);
}

//////////////////////////////////////////////////////////////////////////
// Transport creation functions
//////////////////////////////////////////////////////////////////////////
static iweTransport* weCreateFile(WeDispatch* krnl)
{
    FUNCTION;
    LOG4CXX_DEBUG(WeLogger::GetLogger(), "WeTransportFactory: create WeFile");
    return NULL; //new WeFile();
}
#endif //__DOXYGEN__


WeHtmlFactory::WeHtmlFactory() :
    WeLinkedList<string, EntityFactory>()
{
    data = new WeLinkedListElem<string, EntityFactory>;
    data->Key("");
    data->Value(NULL);
    data->Link(NULL);
}

void WeHtmlFactory::Add( string name, EntityFactory func )
{
    WeLinkedListElem<string, EntityFactory>* obj;

    LOG4CXX_TRACE(WeLogger::GetLogger(), "new EntityFactory added for " << name);
    obj = new WeLinkedListElem<string, EntityFactory>();
    obj->Key(name);
    obj->Value(func);
    data->Add(obj);
}

void WeHtmlFactory::Init()
{
    Clear();
    data = new WeLinkedListElem<string, EntityFactory>;
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

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	iweEntity* WeHtmlFactory::CreateEntity(string tagName, WeHtmlEntity* prnt)
///
/// @brief  Create WeHtmlEntity by the given TAG name.
/// @param  tagName - Name of the html tag.
/// @param  prnt    - Parent of the newly created object
/// @retval	null if it fails, pointer to WeHtmlEntity ot it's successor else.
////////////////////////////////////////////////////////////////////////////////////////////////////
WeHtmlEntity* WeHtmlFactory::CreateEntity( string tagName, WeHtmlEntity* prnt )
{
    EntityFactory func;

    FUNCTION;
    LOG4CXX_DEBUG(WeLogger::GetLogger(), "WeHtmlFactory::CreateEntity => " << tagName);
    func = FindFirst(tagName);
    if (func == NULL) {
        return new WeHtmlEntity(prnt);
    }
    return func(prnt);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
WeTransportFactory::WeTransportFactory() :
    WeLinkedList<string, TransportFactory>()
{
    data = new WeLinkedListElem<string, TransportFactory>;
    data->Key("");
    data->Value(NULL);
    data->Link(NULL);
}

void WeTransportFactory::Add( string name, TransportFactory func )
{
    WeLinkedListElem<string, TransportFactory>* obj;

    LOG4CXX_TRACE(WeLogger::GetLogger(), "new TransportFactory added for " << name);
    obj = new WeLinkedListElem<string, TransportFactory>();
    obj->Key(name);
    obj->Value(func);
    curr = data;
    while (curr != NULL) {
        if (curr->Key() == name)
        {
            break;
        }
        curr = curr->Next();
    }
    if (curr != NULL)
    {
        curr->Value(func);
    }
    else {
        data->Add(obj);
    }
}

void WeTransportFactory::Init()
{
    Clear();
    data = new WeLinkedListElem<string, TransportFactory>;
    data->Key("");
    data->Value(NULL);
    data->Link(NULL);
    Add("file",     weCreateFile);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	iweTransport* WeTransportFactory::CreateTransport(string tagName)
///
/// @brief  Create iweTrasport by the given name.
/// @param  tagName - Name of the transport.
/// @retval	null if it fails, pointer to iweTransport ot it's successor else.
////////////////////////////////////////////////////////////////////////////////////////////////////
iweTransport* WeTransportFactory::CreateTransport( string tagName, WeDispatch* krnl)
{
    TransportFactory func;

    FUNCTION;
    LOG4CXX_DEBUG(WeLogger::GetLogger(), "WeTransportFactory::CreateTransport => " << tagName);
    func = FindFirst(tagName);
    if (func == NULL) {
        return NULL;
    }
    return func(krnl);
}
