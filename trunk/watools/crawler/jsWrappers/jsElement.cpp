#include <weLogger.h>
#include "jsGlobal.h"
#include "jsElement.h"

using namespace v8;

extern bool ExecuteString(Handle<String> source,
                          Handle<Value> name,
                          bool print_result,
                          bool report_exceptions);

Persistent<FunctionTemplate> jsElement::object_template;
bool jsElement::is_init = false;

jsDomElement::jsDomElement() {
    parent = NULL;
    elements.clear();
}

static Handle<Value> GetElementChild(Local<String> name, const AccessorInfo& info)
{
    //this only shows information on what object is being used... just for fun
    {
        String::AsciiValue prop(name);
        String::AsciiValue self(info.This()->ToString());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "jsElement::GetElementChild self("<< *self <<"), property("<< *prop<<")");
    }
    Local<Object> self = info.This();
    Handle<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsDomElement* el = static_cast<jsDomElement*>(ptr);

    HandleScope scope;

    Local<Array> res = Array::New();
    for(size_t i = 0; i < el->elements.size(); i++) {
        Local<Value> val = Local<Value>::New(el->elements[i]);
        res->Set(Int32::New(i), val);
    }
    return scope.Close(res);
}

static Handle<Value> GetElementParent(Local<String> name, const AccessorInfo& info)
{
    //this only shows information on what object is being used... just for fun
    {
        String::AsciiValue prop(name);
        String::AsciiValue self(info.This()->ToString());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "jsElement::GetElementParent self("<< *self <<"), property("<< *prop<<")");
    }
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    jsDomElement* el = static_cast<jsDomElement*>(ptr);
    jsDomElement* parent = el->parent;
    if (parent != NULL)
    {
        return wrap_object<jsElement>(parent);
    }
    else {
        return Undefined();
    }
}

static void SetElementParent(Local<String> name, Local<Value> val, const AccessorInfo& info)
{
    //this only shows information on what object is being used... just for fun
    {
        String::AsciiValue prop(name);
        String::AsciiValue vvv(val);
        String::AsciiValue self(info.This()->ToString());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "jsElement::SetElementParent self("<< *self <<
            "), property("<< *prop<<"), value("<< *vvv << ")");
    }
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsDomElement* el = static_cast<jsDomElement*>(ptr);
    if(val->IsObject()) {
        Local<Object> aref = val->ToObject();
        Local<External> awrap = Local<External>::Cast(aref->GetInternalField(0));
        void* aptr = awrap->Value();
        jsDomElement* ch = static_cast<jsDomElement*>(aptr);
        if (ch != el) {
            el->parent = ch;
        }
        else {
            LOG4CXX_ERROR(webEngine::iLogger::GetLogger(), "JsException: can't add object to itself!\n");
        }
    }
    else {
        LOG4CXX_ERROR(webEngine::iLogger::GetLogger(), "JsException: argument must be an object!\n");
    }
}

Handle<Value> js_el_append_child(const Arguments& args)
{
    //this only shows information on what object is being used... just for fun
    LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "jsElement::js_el_append_child");
//     {
//         String::AsciiValue prop(args.This());
//         printf("function: this(%s), args(%d):", *prop, args.Length());
//         for(int i = 0; i < args.Length(); i++) {
//             String::AsciiValue prop(args[i]);
//             printf("(%s); ", *prop);
//         }
//         printf("\n");
//     }
    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    jsDomElement* el = static_cast<jsDomElement*>(ptr);
    if (args.Length() > 0 && args[0]->IsObject()) {
        Local<Object> aref = args[0]->ToObject();
        el->elements.push_back(Persistent<Value>::New(aref));
    }
    else {
        LOG4CXX_ERROR(webEngine::iLogger::GetLogger(), "Exception: argument must be an object!\n");
    }
    return Undefined();
}

Handle<Value> Image(const Arguments& args)
{
    // throw if called without `new'
//     if (!args.IsConstructCall()) 
//         return ThrowException(String::New("js::Image: Cannot call constructor as function"));

    HandleScope scope;
    Handle<External> external;

    jsDomElement *p = new jsDomElement();
    Handle<Object> res = wrap_object<jsElement>(p);
    res->Set(String::New("tag"), String::New("img"));
    append_object(res);
    return scope.Close(res);
}

Handle<Value> Element(const Arguments& args)
{
    // throw if called without `new'
//     if (!args.IsConstructCall()) 
//         return ThrowException(String::New("js::Element: Cannot call constructor as function"));

    HandleScope scope;
    Handle<External> external;

    String::Utf8Value fname(args.Callee()->GetName());

    if (args.Length() > 0) {
        jsDomElement *p = new jsDomElement();
        Handle<Object> res = wrap_object<jsElement>(p);
        res->Set(String::New("tag"), args[0]);
        append_object(res);
        return scope.Close(res);
    }
    else {
        return Undefined();
    }
}

jsElement::jsElement(void)
{
    if (!is_init) {
        is_init = true;
        Handle<FunctionTemplate> _object = FunctionTemplate::New();
        //get the location's instance template
        Handle<ObjectTemplate> _proto = _object->InstanceTemplate();
        //set its internal field count to one (we'll put references to the C++ point here later)
        _proto->SetInternalFieldCount(1);

        // Add accessors for each of the fields of the Location.
        _proto->SetAccessor(String::NewSymbol("parent"),     GetElementParent, SetElementParent,
            Handle<Value>(), DEFAULT,  DontEnum);
        _proto->SetAccessor(String::NewSymbol("elements"),   GetElementChild); //, SetElementChild
        _proto->Set(String::New("appendChild"), FunctionTemplate::New(js_el_append_child));

        object_template = Persistent<FunctionTemplate>::New(_object);
    }
}

jsElement::~jsElement(void)
{
}
