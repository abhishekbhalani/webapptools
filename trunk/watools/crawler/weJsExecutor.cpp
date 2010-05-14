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
#include "weJsExecutor.h"
#include "jsWrappers/jsGlobal.h"

// include DOM objects
#include "jsWrappers/jsLocation.h"
#include "jsWrappers/jsBrowser.h"
#include "jsWrappers/jsElement.h"
#include "jsWrappers/jsDocument.h"

#include <boost/thread.hpp>

using namespace v8;

namespace webEngine {

static boost::mutex locker;

bool jsExecutor::is_init = false;
int  jsExecutor::num_objects = 0;
Persistent<FunctionTemplate> jsExecutor::object_template;
Persistent<ObjectTemplate> jsExecutor::global;

jsExecutor::jsExecutor(void)
{
    HandleScope handle_scope;
    Local<ObjectTemplate> _global;
    bool notFirst = is_init;

    maxDepth = 10; //default value
    objects.clear();

    if (!is_init) {
        is_init = true;
        // Create a template for the global object.
        _global = ObjectTemplate::New();
        // Bind the global 'alert' function to the C++ print callback.
        _global->Set(String::New("alert"), FunctionTemplate::New(alert));
        // Bind the global 'print' function to the C++ print callback.
        _global->Set(String::New("print"), FunctionTemplate::New(print));
        // Bind the 'version' function
        _global->Set(String::New("version"), FunctionTemplate::New(version));
        // Bind the 'dumpObj' function
        _global->Set(String::New("dumpObj"), FunctionTemplate::New(dump_object));
        
        // Bind DOM classes
        // Bind the 'Location' function
        _global->Set(v8::String::New("Location"), FunctionTemplate::New(Location));
        js_location location;
        LOG4CXX_TRACE(iLogger::GetLogger(), "jsExecutor: Location initialized");
        // Bind the 'Navigator' function
        _global->Set(v8::String::New("Navigator"), FunctionTemplate::New(Browser));
        jsBrowser navigator;
        LOG4CXX_TRACE(iLogger::GetLogger(), "jsExecutor: Navigator initialized");
        // Bind the 'Element' function
        jsElement element;
        _global->Set(v8::String::New("Element"), FunctionTemplate::New(Element));
        LOG4CXX_TRACE(iLogger::GetLogger(), "jsExecutor: Element initialized");
        _global->Set(v8::String::New("Image"), FunctionTemplate::New(Image));
        LOG4CXX_TRACE(iLogger::GetLogger(), "jsExecutor: Image initialized");
        // Bind the 'Document' function
        jsDocument document;
        _global->Set(v8::String::New("Document"), FunctionTemplate::New(Document));
        LOG4CXX_TRACE(iLogger::GetLogger(), "jsExecutor: Document initialized");

        global = Persistent<ObjectTemplate>::New(_global);

        // Initialize Object template to wrap object into JavaScript
        Handle<FunctionTemplate> _object = FunctionTemplate::New();
        //get the location's instance template
        Handle<ObjectTemplate> _proto = _object->InstanceTemplate();
        //set its internal field count to one (we'll put references to the C++ point here later)
        _proto->SetInternalFieldCount(1);
        object_template = Persistent<FunctionTemplate>::New(_object);
        LOG4CXX_TRACE(iLogger::GetLogger(), "jsExecutor: objectTemplate initialized");
    }
    else {
        _global = Local<ObjectTemplate>::New(global);
    }
    // init global objects
    _global->SetAccessor(v8::String::NewSymbol("v8_objects"), result_object);
    _global->SetAccessor(v8::String::NewSymbol("v8_results"), get_result_string, set_result_string);

    num_objects++;
    context = Context::New(NULL, _global);

    {
        // make executor accessible in the JS
        LOG4CXX_TRACE(iLogger::GetLogger(), "jsExecutor: try to wrap object");

        Context::Scope context_scope(context);
//        LOG4CXX_TRACE(iLogger::GetLogger(), "jsExecutor: enter context");
        Handle<Object> _instance = wrap_object<jsExecutor>(this);
//        LOG4CXX_TRACE(iLogger::GetLogger(), "jsExecutor: wrapping successful");
        context->Global()->Set(String::New("v8_context"), _instance);

        execute_string("var location = new Location()", "", true, true);
        execute_string("var navigator = new Navigator()", "", true, true);
        execute_string("var document = new Document()", "", true, true);
        execute_string("document.createElement('head')", "", true, true);
        // !!!DEBUG!!!
        /*execute_string("document.forms.push({text:{value: '',focus: function(){}}})", "", true, true);
        execute_string("function Window () {this.frames = Array(); this.parent = this;this.top=this;this.navigator=navigator;this.location=location;}", "", true, true);
        execute_string("var window = new Window();", "", true, true);
        execute_string("var self = window;", "", true, true);
        execute_string("var top = window;", "", true, true);
        execute_string("function setTimeout(arg, t) {var X=arg(); v8_results += dumpObj(X, \"setTimeout function\");};", "", true, true);*/

    }
}

jsExecutor::~jsExecutor(void)
{
    context.Dispose();
    num_objects--;
//     if(num_objects <= 0) {
//         is_init = false;
//         global.Dispose();
//         V8::Dispose();
//     }
}

// Executes a string within the current v8 context.
bool jsExecutor::execute_string(const string& src, const string& nm, bool print_result, bool report_exceptions)
{
    boost::lock_guard<boost::mutex> lock(locker);

    HandleScope handle_scope;
    Context::Scope context_scope(context);

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

void jsExecutor::append_results(const string& data)
{
    exec_result += data;
    exec_result += "\n";
}

void jsExecutor::append_object(v8::Handle<v8::Object> data)
{
    objects.push_back(v8::Persistent<v8::Object>::New(data));
}

void jsExecutor::reset_results()
{
    boost::lock_guard<boost::mutex> lock(locker);

    exec_result = "";
    //objects.clear();
}

string jsExecutor::dump(const string& name, const string& indent/* = ""*/, int depth/* = 0*/)
{
    boost::lock_guard<boost::mutex> lock(locker);

    HandleScope scope;

    if (depth > maxDepth) {
        return indent + name + ": <Maximum Depth Reached>\n";
    }
    string res = "";
    for (size_t i = 0; i < objects.size(); i++) {
        Local<Value> val = Local<Value>::New(objects[i]);
        res += obj_dump(val, name, indent, depth);
    }
    return res;
}

string jsExecutor::obj_dump(Local<Value> val, const string& name, const string& indent, int depth)
{
    HandleScope scope;
    Context::Scope context_scope(context);

    string ind = indent;
    if (depth > maxDepth) {
        return ind + name + ": <Maximum Depth Reached>\n";
    }
    string res = "";
    if (val->IsObject()) {
        res = ind + name + "\n";
        Local<Object> vobj(Object::Cast(*val));
        Local<Array> nms = vobj->GetPropertyNames();
        ind += "\t";
        for(int i = 0; i < nms->Length(); i++) {
            Local<Value> pN = nms->Get(Int32::New(i));
            Local<Value> pV = vobj->Get(pN);
            if (pV->IsObject())
            {
                res += obj_dump(pV, value_to_string(pN), ind, depth+1);
            }
            else {
                res += ind + value_to_string(pN) + ": " + value_to_string(pV) + "\n";
            }
        }
    }
    else {
        res = value_to_string(val) + "\n";
    }
    return res;
}

// Functions
static Handle<Value> result_object(Local<String> name, const AccessorInfo &info)
{
    //this only shows information on what object is being used... just for fun
    {
        v8::String::AsciiValue prop(name);
        v8::String::AsciiValue self(info.This()->ToString());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::ResultObjects: self("<< *self <<"), property("<< *prop<<")");
    }
    HandleScope scope;

    Local<Array> res = Array::New();

    // get 'global' object
    Local<Object> self = info.This();
    // extract executor
    Local<Value> exec = self->Get(String::New("v8_context"));
    LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::ResultObjects: gets v8_context");
    if (exec->IsObject())
    {
        Local<Object> eObj = Local<Object>::Cast(exec);
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::ResultObjects: gets v8_context object");
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(eObj->GetInternalField(0));
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::ResultObjects: gets v8_context wrapper");
        jsExecutor* jsExec = static_cast<jsExecutor*>(wrap->Value());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::ResultObjects: gets jsExecutor");
        for(size_t i = 0; i < jsExec->objects.size(); i++) {
            Local<Value> val = Local<Value>::New(jsExec->objects[i]);
            res->Set(Int32::New(i), val);
        }
    }

    return scope.Close(res);
}

static Handle<Value> get_result_string(Local<String> name, const AccessorInfo &info)
{
    //this only shows information on what object is being used... just for fun
    {
        v8::String::AsciiValue prop(name);
        v8::String::AsciiValue self(info.This()->ToString());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::GetResultString: self("<< *self <<"), property("<< *prop<<")");
    }
    HandleScope scope;

    Handle<Value> res = Undefined();

    // get 'global' object
    Local<Object> self = info.This();
    // extract executor
    Local<Value> exec = self->Get(String::New("v8_context"));
    LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::ResultObjects: gets v8_context");
    if (exec->IsObject())
    {
        Local<Object> eObj = Local<Object>::Cast(exec);
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::ResultObjects: gets v8_context object");
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(eObj->GetInternalField(0));
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::ResultObjects: gets v8_context wrapper");
        jsExecutor* jsExec = static_cast<jsExecutor*>(wrap->Value());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::ResultObjects: gets jsExecutor");
        res = String::New(jsExec->exec_result.c_str());
    }

    return scope.Close(res);
}

void set_result_string(Local<String> name, Local<Value> val, const AccessorInfo& info)
{
    {
        v8::String::AsciiValue prop(name);
        v8::String::AsciiValue self(info.This()->ToString());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::GetResultString: self("<< *self <<"), property("<< *prop<<")");
    }
    HandleScope scope;

    // get 'global' object
    Local<Object> self = info.This();
    // extract executor
    Local<Value> exec = self->Get(String::New("v8_context"));
    LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::ResultObjects: gets v8_context");
    if (exec->IsObject())
    {
        Local<Object> eObj = Local<Object>::Cast(exec);
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::ResultObjects: gets v8_context object");
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(eObj->GetInternalField(0));
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::ResultObjects: gets v8_context wrapper");
        jsExecutor* jsExec = static_cast<jsExecutor*>(wrap->Value());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::ResultObjects: gets jsExecutor");
        jsExec->exec_result += value_to_string(val);
        jsExec->exec_result += "\n";
    }

}

} //namespace webEngine
