/*
    webEngine is the HTML processing library
    Copyright (C) 2009-11 Andrew Abramov aabramov@ptsecurity.ru

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

    Author:         Andrew "Stinger" Abramov
    Version:        $Revision$
*/
#include "jsScanData.h"
#include "weLogger.h"

using namespace v8;
using namespace webEngine;

namespace jscripter
{

Persistent<FunctionTemplate> jsScanData::object_template;
bool jsScanData::is_init = false;

}

namespace v8_wrapper {
template<>
v8::Persistent<v8::FunctionTemplate> Registrator<jscripter::jsScanData>::GetTemplate()
{
    return jscripter::jsScanData::object_template;
}
}

namespace jscripter
{

jsScanData::jsScanData(webEngine::scan_data_ptr scData)
{
    if (!is_init) {
        is_init = true;
        Handle<FunctionTemplate> _object = FunctionTemplate::New();
        //get the location's instance template
        Handle<ObjectTemplate> _proto = _object->InstanceTemplate();
        //set its internal field count to one (we'll put references to the C++ point here later)
        _proto->SetInternalFieldCount(1);

        Handle<Value> self = External::New(this);
        // Add accessors for each of the fields of the Location.
        //_proto->Set(String::NewSymbol("toString"), FunctionTemplate::New(LoggerToString));
        //_proto->Set(String::NewSymbol("fatal"), FunctionTemplate::New(LoggerFatal));
        //_proto->Set(String::NewSymbol("error"), FunctionTemplate::New(LoggerError));
        //_proto->Set(String::NewSymbol("warn"), FunctionTemplate::New(LoggerWarn));
        //_proto->Set(String::NewSymbol("info"), FunctionTemplate::New(LoggerInfo));
        //_proto->Set(String::NewSymbol("debug"), FunctionTemplate::New(LoggerDebug));
        //_proto->Set(String::NewSymbol("trace"), FunctionTemplate::New(LoggerTrace));

        object_template = Persistent<FunctionTemplate>::New(_object);
    }
}

jsScanData::~jsScanData( void )
{
}

}
