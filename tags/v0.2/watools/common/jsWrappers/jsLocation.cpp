#include <weLogger.h>
#include "jsGlobal.h"
#include "jsLocation.h"
#include "jsWindow.h"

using namespace v8;
using namespace webEngine;

Persistent<FunctionTemplate> jsLocation::object_template;
bool jsLocation::is_init = false;

static Handle<Value> GetProto(Local<String> name, const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    jsLocation* url = static_cast<jsLocation*>(ptr);
    if(url->url.is_valid()) {
        string path = url->url.protocol + ":";
        return String::New(path.c_str());
    }
    else {
        return String::New("http:");
    }
}

static void SetProto(Local<String> name, Local<Value> val, const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<Value> x = self->GetInternalField(0);
    Local<External> wrap = Local<External>::Cast(x);
    void* ptr = wrap->Value();

    jsLocation* url = static_cast<jsLocation*>(ptr);
    url->url.protocol = value_to_string(val);
    if (url->url.protocol[url->url.protocol.length()] == ':') {
        url->url.protocol = url->url.protocol.substr(0, url->url.protocol.length()-1);
    }
}

static Handle<Value> GetLocationHash(Local<String> name, const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    jsLocation* url = static_cast<jsLocation*>(ptr);
    string path = url->url.hashlink;
    return String::New(path.c_str());
}

static void SetLocationHash(Local<String> name, Local<Value> val, const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<Value> x = self->GetInternalField(0);
    Local<External> wrap = Local<External>::Cast(x);
    void* ptr = wrap->Value();

    jsLocation* url = static_cast<jsLocation*>(ptr);
    url->url.hashlink = value_to_string(val);
}

static Handle<Value> GetLocationHost(Local<String> name, const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    jsLocation* url = static_cast<jsLocation*>(ptr);
    if(url->url.is_valid()) {
        string path = url->url.host;
        return String::New(path.c_str());
    }
    else {
        return String::New("", 0);
    }
}

static void SetLocationHost(Local<String> name, Local<Value> val, const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<Value> x = self->GetInternalField(0);
    Local<External> wrap = Local<External>::Cast(x);
    void* ptr = wrap->Value();

    jsLocation* url = static_cast<jsLocation*>(ptr);
    url->url.assign_with_referer(value_to_string(val), &url->url);
}

static Handle<Value> GetLocationHostname(Local<String> name, const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    jsLocation* url = static_cast<jsLocation*>(ptr);
    if(url->url.is_valid()) {
        string path = url->url.host;
        return String::New(path.c_str());
    }
    else {
        return String::New("", 0);
    }
}

static void SetLocationHostname(Local<String> name, Local<Value> val, const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<Value> x = self->GetInternalField(0);
    Local<External> wrap = Local<External>::Cast(x);
    void* ptr = wrap->Value();

    jsLocation* url = static_cast<jsLocation*>(ptr);
    url->url.host = value_to_string(val);
}

static Handle<Value> GetLocationPath(Local<String> name, const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    jsLocation* url = static_cast<jsLocation*>(ptr);
    if(url->url.is_valid()) {
        string path = url->url.request;
        return String::New(path.c_str());
    }
    else {
        return String::New("", 0);
    }
}

static void SetLocationPath(Local<String> name, Local<Value> val, const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<Value> x = self->GetInternalField(0);
    Local<External> wrap = Local<External>::Cast(x);
    void* ptr = wrap->Value();

    jsLocation* url = static_cast<jsLocation*>(ptr);
    url->url.request = value_to_string(val);
}

static Handle<Value> GetLocationPort(Local<String> name, const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    jsLocation* url = static_cast<jsLocation*>(ptr);
    return Int32::New(url->url.port);
}

static void SetLocationPort(Local<String> name, Local<Value> val, const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<Value> x = self->GetInternalField(0);
    Local<External> wrap = Local<External>::Cast(x);
    void* ptr = wrap->Value();

    jsLocation* url = static_cast<jsLocation*>(ptr);
    url->url.port = val->Int32Value();
}

static Handle<Value> GetLocationSearch(Local<String> name, const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    jsLocation* url = static_cast<jsLocation*>(ptr);
    if(url->url.is_valid()) {
        string path = url->url.params;
        return String::New(path.c_str());
    }
    else {
        return String::New("", 0);
    }
}

static void SetLocationSearch(Local<String> name, Local<Value> val, const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<Value> x = self->GetInternalField(0);
    Local<External> wrap = Local<External>::Cast(x);
    void* ptr = wrap->Value();

    jsLocation* url = static_cast<jsLocation*>(ptr);
    url->url.params = value_to_string(val);
}

static Handle<Value> GetLocationHref(Local<String> name, const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    jsLocation* url = static_cast<jsLocation*>(ptr);
    if(url->url.is_valid()) {
        string path = url->url.tostring();
        return String::New(path.c_str(), path.length());
    }
    else {
        return String::New("", 0);
    }
}

static void SetLocationHref(Local<String> name, Local<Value> val, const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<Value> x = self->GetInternalField(0);
    Local<External> wrap = Local<External>::Cast(x);
    void* ptr = wrap->Value();

    jsLocation* url = static_cast<jsLocation*>(ptr);
    std::string log_info = "Location changed from '";
    if(url->url.is_valid()) {
        log_info += url->url.tostring();
    }
    else {
        log_info += "undefined";
    }
    log_info += "' to \n\thref: ";
    url->url.assign_with_referer(value_to_string(val), &url->url);
    if(url->url.is_valid()) {
        log_info += url->url.tostring();
    }
    else {
        log_info += "undefined";
    }
    append_results(log_info);
}

static Handle<Value> LocationToString(const v8::Arguments& args)
{
    Local<Object> self = args.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    jsLocation* url = static_cast<jsLocation*>(ptr);
    string path = url->url.tostring();
    Handle<Value> retval = String::New(path.c_str(), path.length());
    return retval;
}

static Handle<Value> LocationReplace(const v8::Arguments& args)
{
    Local<Object> self = args.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    jsLocation* url = static_cast<jsLocation*>(ptr);
    if (url->win != NULL && args.Length() > 0) {
        url->url.assign_with_referer(value_to_string(args[0]), &url->url);
        url->win->history->push_back(url->url.tostring());
        url->win->load(url->url.tostring());
    }
    else {
        std::string log_info = "Location changed from '";
        if(url->url.is_valid()) {
            log_info += url->url.tostring();
        }
        else {
            log_info += "undefined";
        }
        log_info += "' to \n\thref: ";
        if (args.Length() > 0) {
            url->url.assign_with_referer(value_to_string(args[0]), &url->url);
            if(url->url.is_valid()) {
                log_info += url->url.tostring();
            }
            else {
                log_info += "undefined";
            }
        }
        else {
            if(url->url.is_valid()) {
                log_info += url->url.tostring();
            }
            else {
                log_info += "undefined";
            }
        }
        append_results(log_info);
    }
    return Handle<Value>();
}

static Handle<Value> LocationReload(const v8::Arguments& args)
{
    Local<Object> self = args.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    jsLocation* url = static_cast<jsLocation*>(ptr);
    if (url->win != NULL) {
        url->win->load(url->url.tostring());
    }
    else {
        std::string log_info = "Location reloaded for ";
        log_info += url->url.tostring();
        append_results(log_info);
    }
    return Handle<Value>();
}

jsLocation::jsLocation(jsWindow* parent)
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
        _proto->SetAccessor(String::NewSymbol("hash"),      GetLocationHash, SetLocationHash);
        _proto->SetAccessor(String::NewSymbol("host"),      GetLocationHost, SetLocationHost);
        _proto->SetAccessor(String::NewSymbol("hostname"),  GetLocationHostname, SetLocationHostname);
        _proto->SetAccessor(String::NewSymbol("href"),      GetLocationHref, SetLocationHref);
        _proto->SetAccessor(String::NewSymbol("pathname"),  GetLocationPath, SetLocationPath);
        _proto->SetAccessor(String::NewSymbol("port"),      GetLocationPort, SetLocationPort);
        _proto->SetAccessor(String::NewSymbol("protocol"),  GetProto, SetProto);
        _proto->SetAccessor(String::NewSymbol("search"),    GetLocationSearch, SetLocationSearch);
        _proto->Set(String::NewSymbol("toString"), FunctionTemplate::New(LocationToString));
        _proto->Set(String::NewSymbol("assign"), FunctionTemplate::New(LocationReplace));
        _proto->Set(String::NewSymbol("reload"), FunctionTemplate::New(LocationReload));
        _proto->Set(String::NewSymbol("replace"), FunctionTemplate::New(LocationReplace));

        object_template = Persistent<FunctionTemplate>::New(_object);
    }
    url.protocol = "http";
    url.host = "";
    url.port = 80;
    url.request = "";
    url.params = "";
    url.username = "";
    url.password = "";
    win = parent;
}

jsLocation::~jsLocation(void)
{
}
