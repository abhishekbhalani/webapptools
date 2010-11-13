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
#include <weLogger.h>
#include <weTask.h>
#include "jsExecutor.h"
#include "jsGlobal.h"

// include DOM objects
#include "jsLocation.h"
#include "jsBrowser.h"

#include <boost/thread.hpp>

using namespace v8;

v8::Persistent<v8::FunctionTemplate> v8_wrapper::Registrator<webEngine::jsExecutor>::GetTemplate()
{
    return webEngine::jsExecutor::object_template;
}

namespace webEngine {

static boost::mutex locker;

bool jsExecutor::is_init = false;
Persistent<FunctionTemplate> jsExecutor::object_template;
Persistent<ObjectTemplate> jsExecutor::global;

jsExecutor::jsExecutor(void):net_access(NULL), maxDepth(10)
{
    HandleScope handle_scope;
    maxDepth = 10; //default value
    init_globals();
    context = Context::New(NULL, global);
}

jsExecutor::~jsExecutor(void)
{
    boost::lock_guard<boost::mutex> lock(locker);
    context.Dispose();
    //global.Dispose();
    //V8::Dispose();
}

// Executes a string within the current v8 context.
bool jsExecutor::execute_string(const string& src, const string& nm, bool print_result, bool report_exceptions, v8::Handle<v8::Context> ctx/* = NULL*/)
{
    boost::lock_guard<boost::mutex> lock(locker);

    HandleScope handle_scope;
    if (ctx.IsEmpty()) {
        ctx = context;
    }
    Context::Scope context_scope(ctx);

    Handle<String> source = String::New(src.c_str());
    Handle<Value> name = String::New(nm.c_str());
    TryCatch try_catch;
    Handle<Script> script = Script::Compile(source, name);
    if (script.IsEmpty()) {
        // print errors that happened during compilation.
        if (report_exceptions)
            report_exception(&try_catch);
        return false;
    } else {
        Handle<Value> result = script->Run();
        if (result.IsEmpty()) {
            // print errors that happened during execution.
            if (report_exceptions)
                report_exception(&try_catch);
            return false;
        } else {
            if (print_result && !result->IsUndefined()) {
                // If all went well and the result wasn't undefined then print
                // the returned value.
                string cstr = value_to_string(Local<Value>::New(result));
                LOG4CXX_TRACE(iLogger::GetLogger(), "JS result: " << cstr);
            }
            return true;
        }
    }
}

void jsExecutor::report_exception(TryCatch* try_catch)
{
    HandleScope handle_scope;

    string exception_string = value_to_string(try_catch->Exception());
    Handle<Message> message = try_catch->Message();
    if (message.IsEmpty()) {
        // V8 didn't provide any extra information about this error; just
        // print the exception.
        LOG4CXX_ERROR(iLogger::GetLogger(), "JS exception: " << exception_string);
    } else {
        // print (filename):(line number): (message).
        string filename_string = value_to_string(Local<Value>::New(message->GetScriptResourceName()));
        int linenum = message->GetLineNumber();
        LOG4CXX_ERROR(iLogger::GetLogger(), "JS exception: " << filename_string << ":" <<
                      linenum << ": " << exception_string);
        // print line of source code.
        string sourceline_string = value_to_string(Local<Value>::New(message->GetSourceLine()));
        LOG4CXX_ERROR(iLogger::GetLogger(), "JS >>>\t" << sourceline_string);
        // print wavy underline (GetUnderline is deprecated).
        //int start = message->GetStartColumn();
        //for (int i = 0; i < start; i++) {
        //    printf(" ");
        //}
        //int end = message->GetEndColumn();
        //for (int i = start; i < end; i++) {
        //    printf("^");
        //}
        //printf("\n");
    }
}

v8::Persistent<v8::Context> jsExecutor::get_child_context()
{
    return context;
}

void jsExecutor::close_child_context( v8::Persistent<v8::Context> ctx )
{
    /*if (!ctx.IsEmpty()) {
        ctx.Dispose();
    }*/
}

i_response_ptr jsExecutor::http_request(i_request_ptr req)
{
    i_response_ptr retval;
    HttpRequest* ptr = (HttpRequest*)req.get();

    LOG4CXX_DEBUG(iLogger::GetLogger(), "jsExecutor::http_request URL=" << ptr->RequestUrl().tostring() << " METHOD=" << ptr->Method());
    if(net_access != NULL) {
        retval = net_access->get_request(req);
    } else {
        HttpResponse* ptr = new HttpResponse();
        ptr->HttpCode(503);
        ptr->BaseUrl(req->RequestUrl());
        ptr->RealUrl(req->RequestUrl());
        ptr->Data().clear();
        ptr->Headers().clear();
        ptr->Cookies().clear();
        retval.reset(ptr);
    }
    return retval;
}

bool jsExecutor::http_request_async(i_request_ptr req)
{
    bool retval = false;
    HttpRequest* ptr = (HttpRequest*)req.get();

    LOG4CXX_DEBUG(iLogger::GetLogger(), "jsExecutor::http_request_async URL=" << ptr->RequestUrl().tostring() << " METHOD=" << ptr->Method());
    if(net_access != NULL) {
        net_access->get_request_async(req);
        retval = false;
    }
    return false;
}

void jsExecutor::init_globals()
{
    if(!is_init) {
        is_init = true;
        // Create a template for the global object.
        global = Persistent<ObjectTemplate>::New(ObjectTemplate::New());
        // Bind the global 'alert' function to the C++ print callback.
        global->Set(String::New("alert"), FunctionTemplate::New(alert));
        // Bind the global 'print' function to the C++ print callback.
        global->Set(String::New("echo"), FunctionTemplate::New(print));
        // Bind the 'version' function
        global->Set(String::New("version"), FunctionTemplate::New(version));

        // Initialize Object template to wrap object into JavaScript
        Handle<FunctionTemplate> _object = FunctionTemplate::New();
        //get the location's instance template
        Handle<ObjectTemplate> _proto = _object->InstanceTemplate();
        //set its internal field count to one (we'll put references to the C++ point here later)
        _proto->SetInternalFieldCount(1);
        object_template = Persistent<FunctionTemplate>::New(_object);
    }
    LOG4CXX_TRACE(iLogger::GetLogger(), "jsExecutor: objectTemplate initialized");
}

}