#include <weLogger.h>
#include "jsGlobal.h"
#include "jsLocation.h"

using namespace v8;

Persistent<FunctionTemplate> js_location::object_template;
bool js_location::is_init = false;

static Handle<Value> GetProto(Local<String> name, const AccessorInfo& info)
{
    //this only shows information on what object is being used... just for fun
    {
        String::AsciiValue prop(name);
        String::AsciiValue self(info.This()->ToString());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js_location::GetProto self("<< *self <<"), property("<< *prop<<")");
    }
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    webEngine::transport_url* url = static_cast<webEngine::transport_url*>(ptr);
    if(url->is_valid()) {
        const string& path = url->protocol + ":";
        return String::New(path.c_str());
    }
    else {
        return String::New("http:");
    }
}

static void SetProto(Local<String> name, Local<Value> val, const AccessorInfo& info)
{
    //this only shows information on what object is being used... just for fun
    //     {
    //         String::AsciiValue prop(name);
    //         String::AsciiValue vvv(val);
    //         String::AsciiValue self(info.This()->ToString());
    // //        String::AsciiValue holder(info.Holder()->ToString());
    //         printf("getter: self(%s), property(%s), value(%s)\n", *self, *prop, *vvv);
    //     }
    Local<Object> self = info.This();
    Local<Value> x = self->GetInternalField(0);
    Local<External> wrap = Local<External>::Cast(x);
    void* ptr = wrap->Value();
    webEngine::transport_url* url = static_cast<webEngine::transport_url*>(ptr);
    url->protocol = value_to_string(val);
    if (url->protocol[url->protocol.length()] == ':') {
        url->protocol = url->protocol.substr(0, url->protocol.length()-1);
    }
}



static Handle<Value> GetLocationHash(Local<String> name, const AccessorInfo& info)
{
    //this only shows information on what object is being used... just for fun
    //     {
    //         String::AsciiValue prop(name);
    //         String::AsciiValue self(info.This()->ToString());
    //         String::AsciiValue holder(info.Holder()->ToString());
    //         printf("getter: holder(%s), self(%s), property(%s)\n", *holder, *self, *prop);
    //     }
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    webEngine::transport_url* url = static_cast<webEngine::transport_url*>(ptr);
    const string& path = url->hashlink;
    return String::New(path.c_str());
}

static void SetLocationHash(Local<String> name, Local<Value> val, const AccessorInfo& info)
{
    //this only shows information on what object is being used... just for fun
    //     {
    //         String::AsciiValue prop(name);
    //         String::AsciiValue vvv(val);
    //         String::AsciiValue self(info.This()->ToString());
    // //        String::AsciiValue holder(info.Holder()->ToString());
    //         printf("getter: self(%s), property(%s), value(%s)\n", *self, *prop, *vvv);
    //     }
    Local<Object> self = info.This();
    Local<Value> x = self->GetInternalField(0);
    Local<External> wrap = Local<External>::Cast(x);
    void* ptr = wrap->Value();
    webEngine::transport_url* url = static_cast<webEngine::transport_url*>(ptr);
    url->hashlink = value_to_string(val);
}

static Handle<Value> GetLocationHost(Local<String> name, const AccessorInfo& info)
{
    //this only shows information on what object is being used... just for fun
    //     {
    //         String::AsciiValue prop(name);
    //         String::AsciiValue self(info.This()->ToString());
    //         String::AsciiValue holder(info.Holder()->ToString());
    //         printf("getter: holder(%s), self(%s), property(%s)\n", *holder, *self, *prop);
    //     }
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    webEngine::transport_url* url = static_cast<webEngine::transport_url*>(ptr);
    if(url->is_valid()) {
        const string& path = url->host;
        return String::New(path.c_str());
    }
    else {
        return String::New("", 0);
    }
}

static void SetLocationHost(Local<String> name, Local<Value> val, const AccessorInfo& info)
{
    //this only shows information on what object is being used... just for fun
    //     {
    //         String::AsciiValue prop(name);
    //         String::AsciiValue vvv(val);
    //         String::AsciiValue self(info.This()->ToString());
    // //        String::AsciiValue holder(info.Holder()->ToString());
    //         printf("getter: self(%s), property(%s), value(%s)\n", *self, *prop, *vvv);
    //     }
    Local<Object> self = info.This();
    Local<Value> x = self->GetInternalField(0);
    Local<External> wrap = Local<External>::Cast(x);
    void* ptr = wrap->Value();
    webEngine::transport_url* url = static_cast<webEngine::transport_url*>(ptr);
    url->assign_with_referer(value_to_string(val), url);
}

static Handle<Value> GetLocationHostname(Local<String> name, const AccessorInfo& info)
{
    //this only shows information on what object is being used... just for fun
    //     {
    //         String::AsciiValue prop(name);
    //         String::AsciiValue self(info.This()->ToString());
    //         String::AsciiValue holder(info.Holder()->ToString());
    //         printf("getter: holder(%s), self(%s), property(%s)\n", *holder, *self, *prop);
    //     }
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    webEngine::transport_url* url = static_cast<webEngine::transport_url*>(ptr);
    if(url->is_valid()) {
        const string& path = url->host;
        return String::New(path.c_str());
    }
    else {
        return String::New("", 0);
    }
}

static void SetLocationHostname(Local<String> name, Local<Value> val, const AccessorInfo& info)
{
    //this only shows information on what object is being used... just for fun
    //     {
    //         String::AsciiValue prop(name);
    //         String::AsciiValue vvv(val);
    //         String::AsciiValue self(info.This()->ToString());
    // //        String::AsciiValue holder(info.Holder()->ToString());
    //         printf("getter: self(%s), property(%s), value(%s)\n", *self, *prop, *vvv);
    //     }
    Local<Object> self = info.This();
    Local<Value> x = self->GetInternalField(0);
    Local<External> wrap = Local<External>::Cast(x);
    void* ptr = wrap->Value();
    webEngine::transport_url* url = static_cast<webEngine::transport_url*>(ptr);
    url->host = value_to_string(val);
}

static Handle<Value> GetLocationPath(Local<String> name, const AccessorInfo& info)
{
    //this only shows information on what object is being used... just for fun
    //     {
    //         String::AsciiValue prop(name);
    //         String::AsciiValue self(info.This()->ToString());
    //         String::AsciiValue holder(info.Holder()->ToString());
    //         printf("getter: holder(%s), self(%s), property(%s)\n", *holder, *self, *prop);
    //     }
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    webEngine::transport_url* url = static_cast<webEngine::transport_url*>(ptr);
    if(url->is_valid()) {
        const string& path = url->request;
        return String::New(path.c_str());
    }
    else {
        return String::New("", 0);
    }
}

static void SetLocationPath(Local<String> name, Local<Value> val, const AccessorInfo& info)
{
    //this only shows information on what object is being used... just for fun
    //     {
    //         String::AsciiValue prop(name);
    //         String::AsciiValue vvv(val);
    //         String::AsciiValue self(info.This()->ToString());
    // //        String::AsciiValue holder(info.Holder()->ToString());
    //         printf("getter: self(%s), property(%s), value(%s)\n", *self, *prop, *vvv);
    //     }
    Local<Object> self = info.This();
    Local<Value> x = self->GetInternalField(0);
    Local<External> wrap = Local<External>::Cast(x);
    void* ptr = wrap->Value();
    webEngine::transport_url* url = static_cast<webEngine::transport_url*>(ptr);
    url->request = value_to_string(val);
}

static Handle<Value> GetLocationPort(Local<String> name, const AccessorInfo& info)
{
    //this only shows information on what object is being used... just for fun
    //     {
    //         String::AsciiValue prop(name);
    //         String::AsciiValue self(info.This()->ToString());
    //         String::AsciiValue holder(info.Holder()->ToString());
    //         printf("getter: holder(%s), self(%s), property(%s)\n", *holder, *self, *prop);
    //     }
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    webEngine::transport_url* url = static_cast<webEngine::transport_url*>(ptr);
    return Int32::New(url->port);
}

static void SetLocationPort(Local<String> name, Local<Value> val, const AccessorInfo& info)
{
    //this only shows information on what object is being used... just for fun
    //     {
    //         String::AsciiValue prop(name);
    //         String::AsciiValue vvv(val);
    //         String::AsciiValue self(info.This()->ToString());
    // //        String::AsciiValue holder(info.Holder()->ToString());
    //         printf("getter: self(%s), property(%s), value(%s)\n", *self, *prop, *vvv);
    //     }
    Local<Object> self = info.This();
    Local<Value> x = self->GetInternalField(0);
    Local<External> wrap = Local<External>::Cast(x);
    void* ptr = wrap->Value();
    webEngine::transport_url* url = static_cast<webEngine::transport_url*>(ptr);
    url->port = val->Int32Value();
}

static Handle<Value> GetLocationSearch(Local<String> name, const AccessorInfo& info)
{
    //this only shows information on what object is being used... just for fun
    //     {
    //         String::AsciiValue prop(name);
    //         String::AsciiValue self(info.This()->ToString());
    //         String::AsciiValue holder(info.Holder()->ToString());
    //         printf("getter: holder(%s), self(%s), property(%s)\n", *holder, *self, *prop);
    //     }
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    webEngine::transport_url* url = static_cast<webEngine::transport_url*>(ptr);
    if(url->is_valid()) {
        const string& path = url->params;
        return String::New(path.c_str());
    }
    else {
        return String::New("", 0);
    }
}

static void SetLocationSearch(Local<String> name, Local<Value> val, const AccessorInfo& info)
{
    //this only shows information on what object is being used... just for fun
    //     {
    //         String::AsciiValue prop(name);
    //         String::AsciiValue vvv(val);
    //         String::AsciiValue self(info.This()->ToString());
    // //        String::AsciiValue holder(info.Holder()->ToString());
    //         printf("getter: self(%s), property(%s), value(%s)\n", *self, *prop, *vvv);
    //     }
    Local<Object> self = info.This();
    Local<Value> x = self->GetInternalField(0);
    Local<External> wrap = Local<External>::Cast(x);
    void* ptr = wrap->Value();
    webEngine::transport_url* url = static_cast<webEngine::transport_url*>(ptr);
    url->params = value_to_string(val);
}

static Handle<Value> GetLocationHref(Local<String> name, const AccessorInfo& info)
{
    //this only shows information on what object is being used... just for fun
    //     {
    //         String::AsciiValue prop(name);
    //         String::AsciiValue self(info.This()->ToString());
    //         String::AsciiValue holder(info.Holder()->ToString());
    //         printf("getter: holder(%s), self(%s), property(%s)\n", *holder, *self, *prop);
    //     }
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    webEngine::transport_url* url = static_cast<webEngine::transport_url*>(ptr);
    if(url->is_valid()) {
        const string& path = url->tostring();
        return String::New(path.c_str(), path.length());
    }
    else {
        return String::New("", 0);
    }
}

static void SetLocationHref(Local<String> name, Local<Value> val, const AccessorInfo& info)
{
    //this only shows information on what object is being used... just for fun
    //     {
    //         String::AsciiValue prop(name);
    //         String::AsciiValue vvv(val);
    //         String::AsciiValue self(info.This()->ToString());
    // //        String::AsciiValue holder(info.Holder()->ToString());
    //         printf("getter: self(%s), property(%s), value(%s)\n", *self, *prop, *vvv);
    //     }
    Local<Object> self = info.This();
    Local<Value> x = self->GetInternalField(0);
    Local<External> wrap = Local<External>::Cast(x);
    void* ptr = wrap->Value();
    webEngine::transport_url* url = static_cast<webEngine::transport_url*>(ptr);
    std::string log_info = "Location changed from '";
    if(url->is_valid()) {
        log_info += url->tostring();
    }
    else {
        log_info += "undefined";
    }
    log_info += "' to \n\thref: ";
    url->assign_with_referer(value_to_string(val), url);
    if(url->is_valid()) {
        log_info += url->tostring();
    }
    else {
        log_info += "undefined";
    }
    append_results(log_info);
}

Handle<Value> Location(const Arguments& args)
{
    // throw if called without `new'
    if (!args.IsConstructCall()) 
        return ThrowException(String::New("Cannot call constructor as function"));

    HandleScope scope;

    webEngine::transport_url *p = new webEngine::transport_url();
    p->protocol = "http";
    p->host = "";
    p->port = 80;
    p->request = "";
    p->params = "";
    p->username = "";
    p->password = "";
    if (args.Length() > 0) {
        p->assign_with_referer(value_to_string(args[0]), p);
    }
    return scope.Close(wrap_object<js_location>(p));
}

js_location::js_location(void)
{
    if (!is_init) {
        is_init = true;
        Handle<FunctionTemplate> _object = FunctionTemplate::New();
        //get the location's instance template
        Handle<ObjectTemplate> _proto = _object->InstanceTemplate();
        //set its internal field count to one (we'll put references to the C++ point here later)
        _proto->SetInternalFieldCount(1);

        // Add accessors for each of the fields of the Location.
        _proto->SetAccessor(String::NewSymbol("hash"),      GetLocationHash, SetLocationHash);
        _proto->SetAccessor(String::NewSymbol("host"),      GetLocationHost, SetLocationHost);
        _proto->SetAccessor(String::NewSymbol("hostname"),  GetLocationHostname, SetLocationHostname);
        _proto->SetAccessor(String::NewSymbol("href"),      GetLocationHref, SetLocationHref);
        _proto->SetAccessor(String::NewSymbol("pathname"),  GetLocationPath, SetLocationPath);
        _proto->SetAccessor(String::NewSymbol("port"),      GetLocationPort, SetLocationPort);
        _proto->SetAccessor(String::NewSymbol("protocol"),  GetProto, SetProto);
        _proto->SetAccessor(String::NewSymbol("search"),    GetLocationSearch, SetLocationSearch);

        object_template = Persistent<FunctionTemplate>::New(_object);
    }
}

js_location::~js_location(void)
{
}

