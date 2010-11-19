#include <weLogger.h>
#include "jsGlobal.h"
#include "jsExecutor.h"
#include "jsBrowser.h"
#include <html_js.h>

#include <vector>
#include <utility>

using namespace v8;
using namespace std;
using namespace webEngine;

// Functions
// Extracts a C string from a V8 Utf8Value.
string value_to_string(const Handle<Value>& val)
{
    if (!val.IsEmpty()) {
        String::Utf8Value utf8(val);
        return string(*utf8);
    }
    return string("<string conversion failed>");
}

Handle<Value> alert(const Arguments& args)
{
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

Handle<Value> print(const Arguments& args)
{
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
    LOG4CXX_INFO(webEngine::iLogger::GetLogger(), res);
    return Undefined();
}

Handle<Value> version(const Arguments& args)
{
    return String::New(V8::GetVersion());
}

