#include <string>
#include <vector>
#include <map>
#include <weLogger.h>
#include "jsGlobal.h"
#include "jsBrowser.h"
#include "jsWindow.h"

using namespace v8;
using namespace webEngine;

Persistent<FunctionTemplate> jsWindow::object_template;
bool jsWindow::isInit = false;

Handle<Value> Window(const v8::Arguments& args)
{
    // throw if called without `new'
    if (!args.IsConstructCall()) 
        return ThrowException(String::New("Cannot call constructor as function"));

    HandleScope scope;

    Handle<Value> retval;
    Local<Context> ctx = v8::Context::GetCurrent();
    Local<Value> exec = ctx->Global()->Get(String::New("v8_context"));
    LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::Window: gets v8_context");
    if (exec->IsObject())
    {
        Local<Object> eObj = Local<Object>::Cast(exec);
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(eObj->GetInternalField(0));
        jsBrowser* jsExec = static_cast<jsBrowser*>(wrap->Value());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::Window: gets jsBrowser");
        jsWindow *p = new jsWindow(jsExec, jsExec->window, jsExec->window);
        retval = wrap_object<jsWindow>(p);
    }

    return scope.Close(retval);
}

jsWindow::jsWindow( jsBrowser* holder_, jsWindow* parent_, jsWindow* creator_ )
{
    if (!isInit) {
        isInit = true;
        Handle<FunctionTemplate> _object = FunctionTemplate::New();
        //get the location's instance template
        Handle<ObjectTemplate> _proto = _object->InstanceTemplate();
        //set its internal field count to one (we'll put references to the C++ point here later)
        _proto->SetInternalFieldCount(1);

        // Add accessors.
        Handle<Value> self = External::New(this);
        _proto->SetAccessor(String::New("document"), GetDocument);
        _proto->SetAccessor(String::New("navigator"), GetNavigator);
        _proto->SetAccessor(String::New("location"), GetLocation);
        _proto->SetAccessor(String::New("parent"), GetParent);
        _proto->SetAccessor(String::New("top"), GetTop);
        _proto->SetAccessor(String::New("self"), GetSelf);

        object_template = Persistent<FunctionTemplate>::New(_object);
    }
    browser = holder_;
    parent = parent_;
    creator = creator_;
    location.url.protocol = "about";
    location.url.host = "blank";
    document = new jsDocument(this);
}

jsWindow::~jsWindow( void )
{
    delete document;
}

Handle<Value> jsWindow::GetDocument(Local<String> name, const AccessorInfo &info)
{
    HandleScope scope;

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsWindow* el = static_cast<jsWindow*>(ptr);

    Handle<Object> _obj = wrap_object<jsDocument>(el->document);

    return scope.Close(_obj);
}


Handle<Value> jsWindow::GetNavigator( Local<String> name, const AccessorInfo &info )
{
    HandleScope scope;

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsWindow* el = static_cast<jsWindow*>(ptr);

    Handle<Object> _obj = wrap_object<jsNavigator>(&el->browser->navigator);

    return scope.Close(_obj);
}

Handle<Value> jsWindow::GetLocation( Local<String> name, const AccessorInfo &info )
{
    HandleScope scope;

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsWindow* el = static_cast<jsWindow*>(ptr);

    Handle<Object> _obj;
    if (el->parent == NULL) {
        _obj = wrap_object<jsLocation>(&el->browser->location);
    }
    else {
        _obj = wrap_object<jsLocation>(&el->location);
    }

    return scope.Close(_obj);
}

Handle<Value> jsWindow::GetParent( Local<String> name, const AccessorInfo &info )
{
    HandleScope scope;

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsWindow* el = static_cast<jsWindow*>(ptr);

    Handle<Object> _obj;
    if (el->parent == NULL) {
        _obj = wrap_object<jsWindow>(el);
    }
    else {
        _obj = wrap_object<jsWindow>(el->parent);
    }

    return scope.Close(_obj);
}

Handle<Value> jsWindow::GetTop( Local<String> name, const AccessorInfo &info )
{
    HandleScope scope;

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsWindow* el = static_cast<jsWindow*>(ptr);

    Handle<Object> _obj;
    _obj = wrap_object<jsWindow>(el->browser->window);

    return scope.Close(_obj);
}

Handle<Value> jsWindow::GetSelf( Local<String> name, const AccessorInfo &info )
{
    HandleScope scope;

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsWindow* el = static_cast<jsWindow*>(ptr);

    Handle<Object> _obj;
    _obj = wrap_object<jsWindow>(el);

    return scope.Close(_obj);
}
