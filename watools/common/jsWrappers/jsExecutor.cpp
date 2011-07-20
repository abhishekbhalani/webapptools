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

namespace v8_wrapper {
    
template<>
v8::Persistent<v8::FunctionTemplate> Registrator<webEngine::jsExecutor>::GetTemplate()
{
    return webEngine::jsExecutor::object_template;
}
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
                LOG4CXX_TRACE(iLogger::GetLogger(), _T("JS result: ") << cstr);
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
        LOG4CXX_ERROR(iLogger::GetLogger(), _T("JS exception: ") << exception_string);
    } else {
        // print (filename):(line number): (message).
        string filename_string = value_to_string(Local<Value>::New(message->GetScriptResourceName()));
        int linenum = message->GetLineNumber();
        LOG4CXX_ERROR(iLogger::GetLogger(), _T("JS exception: ") << filename_string << _T(":") <<
                      linenum << L": " << exception_string);
        // print line of source code.
        string sourceline_string = value_to_string(Local<Value>::New(message->GetSourceLine()));
        LOG4CXX_ERROR(iLogger::GetLogger(), _T("JS >>>\t") << sourceline_string);
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
    //exec_result += obj_dump(Local<Object>::New(data), "appended object", "", 9, context);
    //execute_string("function showAttr(o) { echo(o); if(o.attributes) { a = o.attributes; for(var i in a) {echo('\t'+a[i].name + ': ' + a[i].value)}}}", "", true, true);
    /*    Local<Object> obj = Local<Object>::New(data);
        exec_result += value_to_string(obj);
        exec_result += "\n";
        Local<Array> attrs = obj->Get(String::New("attributes")).As<Array>();
        if (attrs->IsArray()) {
            //Local<Array> names = attrs->GetPropertyNames();
            for (int i = 0; i < attrs->Length(); i++) {
                exec_result += "\t";
                exec_result += value_to_string((*attrs)[i].Get(String::New("name")));
                exec_result += ": ";
                exec_result += value_to_string((*attrs)[i].Get(String::New("value")));
                exec_result += "\n";
            }
        }*/
}

void jsExecutor::reset_results()
{
    boost::lock_guard<boost::mutex> lock(locker);

    exec_result = "";
    objects.clear();
}

string jsExecutor::dump(const string& name, const string& indent/* = ""*/, int depth/* = 0*/, v8::Handle<v8::Context> ctx/* = NULL*/)
{
    boost::lock_guard<boost::mutex> lock(locker);

    HandleScope scope;

    if (depth > maxDepth) {
        return indent + name + ": <Maximum Depth Reached>\n";
    }
    string res = "";
    for (size_t i = 0; i < objects.size(); i++) {
        Local<Value> val = Local<Value>::New(objects[i]);
        res += obj_dump(val, name, indent, depth, ctx);
    }
    return res;
}

string jsExecutor::obj_dump(Local<Value> val, const string& name, const string& indent, int depth, v8::Handle<v8::Context> ctx)
{
    HandleScope scope;
    if (ctx.IsEmpty()) {
        ctx = context;
    }
    Context::Scope context_scope(ctx);

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
            if (pV->IsObject()) {
                res += obj_dump(pV, value_to_string(pN), ind, depth+1, ctx);
            } else {
                res += ind + value_to_string(pN) + ": " + value_to_string(pV) + "\n";
            }
        }
    } else {
        res = value_to_string(val) + "\n";
    }
    return res;
}

v8::Persistent<v8::Context> jsExecutor::get_child_context()
{
    HandleScope handle_scope;

    /*Local<Object> glob = context->Global();

    v8::Persistent<v8::Context> ctx = v8::Context::New(NULL, global, glob);
    /*{
        // make executor accessible in the JS
        LOG4CXX_TRACE(iLogger::GetLogger(), "jsExecutor: try to wrap object");

        Context::Scope context_scope(ctx);
        Handle<Object> _instance = wrap_object<jsExecutor>(this);
        ctx->Global()->Set(String::New("v8_context"), _instance);
    }*/
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

    LOG4CXX_DEBUG(iLogger::GetLogger(), _T("jsExecutor::http_request URL=") << ptr->RequestUrl().tostring() << _T(" METHOD=") << (int)ptr->Method());
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
        string log = "jsExecutor::http_request\n\thref: ";
        log += req->RequestUrl().tostring();
        append_results(log);
    }
    return retval;
}

bool jsExecutor::http_request_async(i_request_ptr req)
{
    bool retval = false;
    HttpRequest* ptr = (HttpRequest*)req.get();

    LOG4CXX_DEBUG(iLogger::GetLogger(), _T("jsExecutor::http_request_async URL=") << ptr->RequestUrl().tostring() << _T(" METHOD=") << (int)ptr->Method());
    if(net_access != NULL) {
        net_access->get_request_async(req);
        retval = false;
    } else {
        string log = "jsExecutor::http_request_async\n\thref: ";
        log += req->RequestUrl().tostring();
        append_results(log);
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
        // Bind the 'dumpObj' function
        global->Set(String::New("dumpObj"), FunctionTemplate::New(dump_object));
        // Bind the 'dumpRsults' function
        global->Set(String::New("dumpResults"), FunctionTemplate::New(result_object_info));

        // Initialize Object template to wrap object into JavaScript
        Handle<FunctionTemplate> _object = FunctionTemplate::New();
        //get the location's instance template
        Handle<ObjectTemplate> _proto = _object->InstanceTemplate();
        //set its internal field count to one (we'll put references to the C++ point here later)
        _proto->SetInternalFieldCount(1);
        object_template = Persistent<FunctionTemplate>::New(_object);
    }
    LOG4CXX_TRACE(iLogger::GetLogger(), _T("jsExecutor: objectTemplate initialized"));
}

string jsExecutor::dump_results()
{
    string result;
    Context::Scope context_scope(context);

    for(size_t i = 0; i < objects.size(); i++) {
        Local<Value> obje = Local<Value>::New(objects[i]);
        result += value_to_string(obje);
        if (obje->IsObject()) {
            Local<Object> obj = Local<Object>::New(obje.As<Object>());
            result += "\n";
            Local<Array> attrs = obj->Get(String::New("attributes")).As<Array>();
            if (attrs->IsArray()) {
                Local<Array> names = attrs->GetPropertyNames();
                for (int i = 0; i < names->Length(); i++) {
                    Local<Value> nm = names->Get(Number::New(i));
                    result += "\t";
                    result += value_to_string(attrs->Get(nm)->ToObject()->Get(String::New("name")));
                    result += ": ";
                    result += value_to_string(attrs->Get(nm)->ToObject()->Get(String::New("value")));
                    result += "\n";
                }
            } // if has Array
        } // if Object
    } // foreach object
    return result;
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
    if (exec->IsObject()) {
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


static Handle<Value> result_object_info( const Arguments& args )
{
    HandleScope scope;
    Handle<Value> res;

    Local<Object> self = args.This();
    Local<Value> exec = self->Get(String::New("v8_context"));
    Local<Object> eObj = Local<Object>::Cast(exec);
    Local<External> wrap = Local<External>::Cast(eObj->GetInternalField(0));
    jsExecutor* jsExec = static_cast<jsExecutor*>(wrap->Value());

    string result = jsExec->dump_results();
    res = String::New(result.c_str());

    return scope.Close(res);
}

static Handle<Value> get_result_string(Local<String> name, const AccessorInfo &info)
{
    HandleScope scope;

    Handle<Value> res = Undefined();

    // get 'global' object
    Local<Object> self = info.This();
    // extract executor
    Local<Value> exec = self->Get(String::New("v8_context"));
    LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::ResultObjects: gets v8_context");
    if (exec->IsObject()) {
        Local<Object> eObj = Local<Object>::Cast(exec);
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::ResultObjects: gets v8_context object");
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(eObj->GetInternalField(0));
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::ResultObjects: gets v8_context wrapper");
        jsExecutor* jsExec = static_cast<jsExecutor*>(wrap->Value());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::ResultObjects: gets jsExecutor");
        res = String::New(jsExec->get_results().c_str());
    }

    return scope.Close(res);
}

void set_result_string(Local<String> name, Local<Value> val, const AccessorInfo& info)
{
    HandleScope scope;

    // get 'global' object
    Local<Object> self = info.This();
    // extract executor
    Local<Value> exec = self->Get(String::New("v8_context"));
    LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::ResultObjects: gets v8_context");
    if (exec->IsObject()) {
        Local<Object> eObj = Local<Object>::Cast(exec);
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::ResultObjects: gets v8_context object");
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(eObj->GetInternalField(0));
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::ResultObjects: gets v8_context wrapper");
        jsExecutor* jsExec = static_cast<jsExecutor*>(wrap->Value());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::ResultObjects: gets jsExecutor");
        string sval = value_to_string(val);
        jsExec->append_results(sval);
    }

}

} //namespace webEngine
