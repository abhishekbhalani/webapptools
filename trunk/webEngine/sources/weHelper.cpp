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
#include <string.h>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/thread.hpp>
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
static boost::mutex ref_count_locker;

static void LibInitCommon()
{
    CURLcode    cCode;

    LOG4CXX_TRACE(iLogger::GetLogger(), "LibInit status = " << isLibInited);
    if (!isLibInited)
    {
        weHtmlFactory.Init();
        cCode = curl_global_init(CURL_GLOBAL_ALL);
        if (cCode != 0) {
            LOG4CXX_FATAL(iLogger::GetLogger(), "cURL initialization failed : " << cCode);
        }
        char* cver = curl_version();
        LOG4CXX_INFO(iLogger::GetLogger(), "cURL library version: " << cver);
    }
    isLibInited = true;
    LOG4CXX_INFO(iLogger::GetLogger(), "WebEngine initialized. Version " WE_VERSION_PRODUCTSTR);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void LibInit(const string& config = "")
///
/// @brief  WebEngine library initialize.
/// @throw  WeError if cURL library doesn't initialized
////////////////////////////////////////////////////////////////////////////////////////////////////
void LibInit(const string& config /*= ""*/)
{
    string      cfgFile;

    try {
        cfgFile = config;
        if (cfgFile != "") { // no configuration needed
            cfgFile = "trace.config";
        }
        path p(cfgFile);
        if (exists(p))
        {
#ifdef WIN32
            PropertyConfigurator::configure(log4cxx::File(cfgFile), LOG4CXX_STR("webEngine"));
#else
            PropertyConfigurator::configure(log4cxx::File(cfgFile));
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
    LibInitCommon();
}
void LibInit(AppenderPtr appender, LevelPtr level )
{
    iLogger::m_logger->removeAllAppenders();
    iLogger::m_logger->addAppender(appender);
    iLogger::m_logger->setLevel(level);
    LibInitCommon();
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
    weHtmlFactory.Clean();

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
static base_entity_ptr weCreateRefObj(base_entity_ptr prnt)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "HtmlFactory: create WeRefrenceObject");
    return base_entity_ptr(new WeRefrenceObject(prnt));
}

static base_entity_ptr weCreateText(base_entity_ptr prnt)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "HtmlFactory: create html_textnode");
    return base_entity_ptr(new html_textnode(prnt));
}

static base_entity_ptr weCreateComment(base_entity_ptr prnt)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "HtmlFactory: create html_comment");
    return base_entity_ptr(new html_comment(prnt));
}

static base_entity_ptr weCreateCData(base_entity_ptr prnt)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "HtmlFactory: create html_cdata");
    return base_entity_ptr(new html_cdata(prnt));
}

static base_entity_ptr weCreatePhpInc(base_entity_ptr prnt)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "HtmlFactory: create WePhpInclude");
    return base_entity_ptr(new WePhpInclude(prnt));
}

static base_entity_ptr weCreateDocument(base_entity_ptr prnt)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "HtmlFactory: create WeHtmlDocument");
    return base_entity_ptr((base_entity*)NULL); //new WeDocument(prnt);
}

static base_entity_ptr weCreateScript(base_entity_ptr prnt)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "HtmlFactory: create WeScript");
    return base_entity_ptr(new WeScript(prnt));
}

HtmlFactory::HtmlFactory()
{
}

void HtmlFactory::Add( string name, fnEntityFactory func )
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "new EntityFactory added for " << name);
	factories_[name] = func;
}

void HtmlFactory::Init()
{
    factories_.clear();
    Add("#document",    weCreateDocument);
    Add("#text",        weCreateText);
    Add("#comment",     weCreateComment);
    Add("#cdata",       weCreateCData);
    Add("#php",         weCreatePhpInc);
//     Add("img",          weCreateRefObj);
//     Add("frame",        weCreateRefObj);
//     Add("iframe",       weCreateRefObj);
    Add("script",       weCreateScript);
//     Add("a",            weCreateRefObj);
}
#endif //__DOXYGEN__

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	base_entity* HtmlFactory::CreateEntity(string tagName, base_entity_ptr prnt)
///
/// @brief  Create html_entity by the given TAG name.
/// @param  tagName - Name of the html tag.
/// @param  prnt    - Parent of the newly created object
/// @retval	null if it fails, pointer to html_entity ot it's successor else.
////////////////////////////////////////////////////////////////////////////////////////////////////
base_entity_ptr HtmlFactory::CreateEntity( string tagName, base_entity_ptr prnt )
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "HtmlFactory::CreateEntity => " << tagName);
	std::map<string, fnEntityFactory>::const_iterator it = factories_.find(tagName);
	if (it == factories_.end())
		return base_entity_ptr(new html_entity(prnt));
	return it->second(prnt);
}

void HtmlFactory::Clean()
{
    factories_.clear();
}

int LockedIncrement( int *val )
{
    boost::unique_lock<boost::mutex> lock(ref_count_locker);
#ifdef _DEBUG
    LOG4CXX_DEBUG(iLogger::GetLogger(), "LockedIncrement for &" << val << " = " <<  *val);
#endif // _DEBUG
    (*val)++;
    return *val;
}

int LockedDecrement( int *val )
{
    boost::unique_lock<boost::mutex> lock(ref_count_locker);
#ifdef _DEBUG
    LOG4CXX_DEBUG(iLogger::GetLogger(), "LockedDecrement for &" << val << " = " << *val);
#endif // _DEBUG
    (*val)--;
    return *val;
}

int LockedGetValue( int *val )
{
    boost::unique_lock<boost::mutex> lock(ref_count_locker);
#ifdef _DEBUG
    LOG4CXX_DEBUG(iLogger::GetLogger(), "LockedGetValue for &" << val << " = " << *val);
#endif // _DEBUG
    int res = (*val);
    return res;

}
} // namespace webEngine
