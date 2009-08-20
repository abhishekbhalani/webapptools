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

namespace webEngine {

typedef HtmlEntity* (*fnEntityFactory)(iEntity* prnt);
typedef iTransport* (*fnTransportFactory)(Dispatch* krnl);

// typedef WeLinkedListElem<string, EntityFactory> WeHtmlFuncList;
// typedef WeLinkedListElem<string, TransportFactory> WeTranspFuncList;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  HtmlFactory
///
/// @brief  The HtmlEntity creator. 
///
/// @author A. Abramov
/// @date   29.05.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class HtmlFactory : public LinkedList<string, fnEntityFactory>
{
public:
    HtmlFactory();
    void Init();
    void Add(string name, fnEntityFactory func);
    HtmlEntity* CreateEntity(string tagName, HtmlEntity* prnt);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  TransportFactory
///
/// @brief  The iTransport creator. 
///
/// @author A. Abramov
/// @date   10.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class TransportFactory : public LinkedList<string, fnTransportFactory>
{
public:
    TransportFactory();
    void Init();
    void Add(string name, fnTransportFactory func);
    iTransport* CreateTransport(string tagName, Dispatch* krnl);
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
class WeError : public runtime_error
{
public:

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn	WeError(string const& msg)
    ///
    /// @brief  Creates a WeError object. 
    /// @param 	msg	 - The error message. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    WeError(string const& msg)
        :	runtime_error(string("WebEngine error") + (msg.empty() ? "" : ": ") + msg)
    {
        LOG4CXX_ERROR(iLogger::GetLogger(), "WebEngine error" << (msg.empty() ? "" : ": ") << msg )
    }
};

extern HtmlFactory  weHtmlFactory;
extern TransportFactory weTransportFactory;

void LibInit(const string& config = "");
void LibClose(void);
string ScreenXML(const string& xml);
string UnscreenXML(const string& xml);

} // namespace webEngine

#endif //__WEHELPER_H__
