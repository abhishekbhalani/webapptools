#include <string>
#include <map>
#include <weLogger.h>
#include "jsGlobal.h"
#include "jsBrowser.h"

using namespace v8;
using namespace webEngine;

Persistent<FunctionTemplate> jsNavigator::object_template;
bool jsNavigator::is_init = false;

static Handle<Value> BrowserGet(Local<String> name, const AccessorInfo &info)
{
    //this only shows information on what object is being used... just for fun
//     {
//         String::AsciiValue prop(name);
//         String::AsciiValue self(info.This()->ToString());
//         LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::BrowserGet: self("<< *self <<"), property("<< *prop<<")");
//     }
    HandleScope scope;

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsNavigator* el = static_cast<jsNavigator*>(ptr);
    // Convert the JavaScript string to a std::string.
    std::string key = value_to_string(name);

    // Look up the value if it exists using the standard STL idiom.
    std::map<std::string, Persistent<Value>>::iterator iter = el->props.find(key);

    // If the key is not present return an empty handle as signal.
    if (iter == el->props.end()) return Handle<Value>();

    // Otherwise fetch the value and wrap it in a JavaScript string.
    Local<Value> val = Local<Value>::New(iter->second);
    return scope.Close(val);
}

static Handle<Value> BrowserSet(Local<String> name, Local<Value> value, const AccessorInfo& info)
{
    //this only shows information on what object is being used... just for fun
//     {
//         String::AsciiValue prop(name);
//         String::AsciiValue self(info.This()->ToString());
//         LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::BrowserSet: self("<< *self <<"), property("<< *prop<<")");
//     }
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsNavigator* el = static_cast<jsNavigator*>(ptr);
    // Convert the JavaScript string to a std::string.
    std::string key = value_to_string(name);

    el->props[key] = Persistent<Value>::New(value);
    return value;
}

jsNavigator::jsNavigator()
{
    props["appCodeName"] = Persistent<Value>::New(String::New("Mozilla"));
    props["appMinorVersion"] = Persistent<Value>::New(Int32::New(0));
    props["appName"] = Persistent<Value>::New(String::New("Netscape"));
    props["appVersion"] = Persistent<Value>::New(String::New("5.0 (Windows; ru)"));
    props["cookieEnabled"] = Persistent<Value>::New(Boolean::New(true));
    props["javaEnabled"] = Persistent<Value>::New(Boolean::New(false));
    props["cpuClass"] = Persistent<Value>::New(String::New(""));
    props["onLine"] = Persistent<Value>::New(Boolean::New(true));
    props["platform"] = Persistent<Value>::New(String::New("Win32"));
    props["userAgent"] = Persistent<Value>::New(String::New("Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US) PT-MaxPatrol V8/3022"));
    props["browserLanguage"] = Persistent<Value>::New(String::New(""));
    props["systemLanguage"] = Persistent<Value>::New(String::New(""));
    props["userLanguage"] = Persistent<Value>::New(String::New(""));

    if (!is_init) {
        is_init = true;
        Handle<FunctionTemplate> _object = FunctionTemplate::New();
        //get the location's instance template
        Handle<ObjectTemplate> _proto = _object->InstanceTemplate();
        //set its internal field count to one (we'll put references to the C++ point here later)
        _proto->SetInternalFieldCount(1);

        // Add accessors for each of the fields of the Location.
        _proto->SetNamedPropertyHandler(BrowserGet, BrowserSet);

        object_template = Persistent<FunctionTemplate>::New(_object);
    }
}

jsBrowser::jsBrowser(void)
{
    if (!is_init) {
        init_globals();
    }
    // delete default context from jsExecutor
    context.Dispose();
    // init values
    location.url.protocol = "about";
    location.url.host = "blank";
    window = new jsWindow(this, NULL, NULL);
    // assign extensions
    Handle<Value> self = External::New(this);
    instance_global = Persistent<ObjectTemplate>::New(global);
    instance_global->SetAccessor(String::New("navigator"), GetNavigator, NULL, self);
    instance_global->SetAccessor(String::New("window"), GetWindow, NULL, self);
    instance_global->SetAccessor(String::New("location"), GetLocation, NULL, self);
    instance_global->Set(String::New("Location"), FunctionTemplate::New(Location));
    instance_global->Set(String::New("Window"), FunctionTemplate::New(Window));
    // make new context
    context = Context::New(NULL, instance_global);
    {
        // make executor accessible in the JS
        LOG4CXX_TRACE(iLogger::GetLogger(), "jsExecutor: try to wrap object");
        Context::Scope context_scope(context);
        Handle<Object> _instance = wrap_object<jsExecutor>(this);
        context->Global()->Set(String::New("v8_context"), _instance);
    }
}

jsBrowser::~jsBrowser(void)
{
    delete window;
}

Handle<Value> jsBrowser::GetNavigator( Local<String> name, const AccessorInfo &info )
{
    HandleScope scope;

    Local<External> wrap = Local<External>::Cast(info.Data());
    void* ptr = wrap->Value();
    jsBrowser* el = static_cast<jsBrowser*>(ptr);

    Handle<Object> _obj = wrap_object<jsNavigator>(&el->navigator);

    return scope.Close(_obj);
}

Handle<Value> jsBrowser::GetWindow( Local<String> name, const AccessorInfo &info )
{
    HandleScope scope;

    Local<External> wrap = Local<External>::Cast(info.Data());
    void* ptr = wrap->Value();
    jsBrowser* el = static_cast<jsBrowser*>(ptr);

    Handle<Object> _obj = wrap_object<jsWindow>(el->window);

    return scope.Close(_obj);
}

Handle<Value> jsBrowser::GetLocation( Local<String> name, const AccessorInfo &info )
{
    HandleScope scope;

    Local<External> wrap = Local<External>::Cast(info.Data());
    void* ptr = wrap->Value();
    jsBrowser* el = static_cast<jsBrowser*>(ptr);

    Handle<Object> _obj = wrap_object<jsLocation>(&el->location);

    return scope.Close(_obj);
}
