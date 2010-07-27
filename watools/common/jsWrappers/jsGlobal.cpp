#include <weLogger.h>
#include "jsExecutor.h"
#include "jsGlobal.h"

using namespace v8;
using namespace std;

// Functions
// Extracts a C string from a V8 Utf8Value.
string value_to_string(const Local<Value>& val) {

    if (*val) {
        String::Utf8Value utf8(val);
        return string(*utf8);
    }
    return string("<string conversion failed>");
}

Handle<Value> alert(const Arguments& args) {
    bool first = true;
    string res = "";
    for (int i = 0; i < args.Length(); i++) {
        HandleScope handle_scope;
        if (first) {
            first = false;
        } else {
            res += " ";
        }
        res += value_to_string(args[i]);
    }
    LOG4CXX_INFO(webEngine::iLogger::GetLogger(), "js::alert: " << res);
    return Undefined();
}

Handle<Value> print(const Arguments& args) {
    bool first = true;
    string res = "";
    for (int i = 0; i < args.Length(); i++) {
        HandleScope handle_scope;
        if (first) {
            first = false;
        } else {
            res += " ";
        }
        res += value_to_string(args[i]);
    }
    append_results(res);
    LOG4CXX_INFO(webEngine::iLogger::GetLogger(), res);
    return Undefined();
}

Handle<Value> version(const Arguments& args) {
    return String::New(V8::GetVersion());
}

void append_results(const std::string& data)
{
    {
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::append_results: data("<< data <<")");
    }
    HandleScope scope;

    // extract executor
    Local<Context> ctx = v8::Context::GetCurrent();
    Local<Value> exec = ctx->Global()->Get(String::New("v8_context"));
//     LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::append_results: gets v8_context");
    if (exec->IsObject())
    {
        Local<Object> eObj = Local<Object>::Cast(exec);
//         LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::append_results: gets v8_context object");
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(eObj->GetInternalField(0));
//         LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::append_results: gets v8_context wrapper");
        webEngine::jsExecutor* jsExec = static_cast<webEngine::jsExecutor*>(wrap->Value());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::append_results: gets jsExecutor");
        jsExec->append_results(data);
    }
}

void append_object(v8::Handle<v8::Object> data)
{
    {
        String::AsciiValue obj(data->ToString());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::append_object: data("<< *obj <<")");
    }
    HandleScope scope;

    // extract executor
    Local<Context> ctx = v8::Context::GetCurrent();
    Local<Value> exec = ctx->Global()->Get(String::New("v8_context"));
//     LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::append_object: gets v8_context");
    if (exec->IsObject())
    {
        Local<Object> eObj = Local<Object>::Cast(exec);
//         LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::append_object: gets v8_context object");
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(eObj->GetInternalField(0));
//         LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::append_object: gets v8_context wrapper");
        webEngine::jsExecutor* jsExec = static_cast<webEngine::jsExecutor*>(wrap->Value());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::append_object: gets jsExecutor");
        jsExec->append_object(data);
    }
}

Handle<Value> dump_object(const Arguments& args)
{
    Handle<Value> res = Undefined();
    string  nm;
    string  id;
    int     dp;

    Local<Context> ctx = v8::Context::GetCurrent();
    Local<Value> exec = ctx->Global()->Get(String::New("v8_context"));
//     LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::DumpObject: gets v8_context");
    if (exec->IsObject())
    {
        Local<Object> eObj = Local<Object>::Cast(exec);
//         LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::DumpObject: gets v8_context object");
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(eObj->GetInternalField(0));
//         LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::DumpObject: gets v8_context wrapper");
        webEngine::jsExecutor* jsExec = static_cast<webEngine::jsExecutor*>(wrap->Value());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::DumpObject: gets jsExecutor");
        if (args.Length() > 1)
        {
            nm = value_to_string(*args[1]);
        }
        else {
            nm = "";
        }
        if (args.Length() > 2)
        {
            id = value_to_string(*args[2]);
        }
        else {
            id = "";
        }
        if (args.Length() > 3)
        {
            dp = args[3]->Int32Value();
        }
        else {
            dp = 0;
        }
        string dat = jsExec->obj_dump(args[0], nm, id, dp, ctx);
        res = String::New(dat.c_str());
    }

    return res;
}
