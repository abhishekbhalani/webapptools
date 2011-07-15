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
#ifndef __WEHELPER_H__
#define __WEHELPER_H__

/**
@file weHelper.h
@brief WebEngine static objects and routines

This file contains helper object to parse input data and create
weHtmlEntity objects properly. Library initialisation routines
also presented in this module.

@author A. Abramov
@date   28.05.2009
*/

#include "weiBase.h"
#include "weStrings.h"
#include "weiTransport.h"
#include "weHtmlEntity.h"
#pragma warning( disable : 4231)
#include <log4cxx/appender.h>
#pragma warning( default : 4231)

namespace webEngine {

typedef base_entity_ptr (*fnEntityFactory)(base_entity_ptr prnt);

// typedef WeLinkedListElem<string, EntityFactory> WeHtmlFuncList;
// typedef WeLinkedListElem<string, transport_factory> WeTranspFuncList;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  HtmlFactory
///
/// @brief  The HtmlEntity creator.
///
/// @author A. Abramov
/// @date   29.05.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class HtmlFactory {
public:
    HtmlFactory();
    void Init();
    void Add(string name, fnEntityFactory func);
    base_entity_ptr CreateEntity(string tagName, base_entity_ptr prnt);
    void Clean();
private:
    std::map<string, fnEntityFactory> factories_;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  WeError
///
/// @brief  Class for exception processing
///
/// Exception thrown whenever an error occurs in a WebEngine operations
/// @author A. Abramov
/// @date   29.05.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class WeError : public runtime_error {
public:

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn	WeError(string const& msg)
    ///
    /// @brief  Creates a WeError object.
    /// @param 	msg	 - The error message.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    WeError(string const& msg)
        :	runtime_error(string("WebEngine error") + (msg.empty() ? "" : ": ") + msg) {
        LOG4CXX_ERROR(iLogger::GetLogger(), _T("WebEngine error") << (msg.empty() ? _T("") : _T(": ")) << msg )
    }
};

extern HtmlFactory  weHtmlFactory;
extern transport_factory weTransportFactory;

void LibInit(const string& config = "");
void LibInit(log4cxx::AppenderPtr appender, log4cxx::LevelPtr level);
void LibClose(void);
string ScreenXML(const string& xml);
string UnscreenXML(const string& xml);
int LockedIncrement(int *val);
int LockedDecrement(int *val);
int LockedGetValue(int *val);

} // namespace webEngine

#endif //__WEHELPER_H__
