#include <string>
#include <vector>
#include <map>
#include <weLogger.h>
#include "jsGlobal.h"
#include "jsDocument.h"
#include "jsBrowser.h"
#include "jsWindow.h"

using namespace v8;
using namespace webEngine;

Persistent<FunctionTemplate> jsDocument::object_template;
bool jsDocument::isInit = false;

static Handle<Value> DocumentGet(Local<String> name, const AccessorInfo &info)
{
    //this only shows information on what object is being used... just for fun
    {
        String::AsciiValue prop(name);
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "jsDocument::Get: property(" << *prop << ")");
    }

    HandleScope scope;

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsDocument* el = static_cast<jsDocument*>(ptr);
    // Convert the JavaScript string to a std::string.
    std::string key = value_to_string(name);

    // Look up the value if it exists using the standard STL idiom.
    std::map<std::string, Persistent<Value>>::iterator iter = el->namedprops.find(key);

    // If the key is not present return an empty handle as signal.
    if (iter == el->namedprops.end()) return Handle<Value>();

    // Otherwise fetch the value and wrap it in a JavaScript string.
    Local<Value> val = Local<Value>::New(iter->second);
    return scope.Close(val);
}

void DocumentSet(Local<String> name, Local<Value> value, const AccessorInfo& info)
{
    //this only shows information on what object is being used... just for fun
    {
        String::AsciiValue prop(name);
        String::AsciiValue vals(value);
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "jsDocument::Set: property(" << *prop << "), value(" << *vals<<")");
    }
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsDocument* el = static_cast<jsDocument*>(ptr);
    // Convert the JavaScript string to a std::string.
    std::string key = value_to_string(name);

    el->namedprops[key] = Persistent<Value>::New(value); //String::New(dat.c_str(), dat.length());
}



static Handle<Value> getElementsByTagName(const Arguments& args)
{
    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsDocument* dc = static_cast<jsDocument*>(ptr);

    //enter a handle scope
    HandleScope scope;

    Local<Array> res = Array::New();

    if (args.Length() > 0) {
        std::string tagName = value_to_string(args[0]);
        int idx = 0;
/*        for(size_t i = 0; i < dc->elements.size(); i++) {
            Local<Value> aref = Local<Value>::New(dc->elements[i]);
            if (aref->IsObject())
            {
                Local<Object> oref = aref->ToObject();
                Local<Value> tname = oref->Get(String::New("tag"));
                std::string tagCheck = value_to_string(tname);
                if (tagCheck == tagName)
                {
                    res->Set(Int32::New(idx++), aref);
                }
            }
        }*/
    }
    return scope.Close(res);
}

static Handle<Value> createElement(const Arguments& args)
{
    //enter a handle scope
    HandleScope scope;

    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsDocument* dc = static_cast<jsDocument*>(ptr);

    Handle<Value> res(Undefined());

    if (args.Length() > 0) {
        res = Element(args);
        if (args.Length() > 1 && args[1]->IsObject()) {
            Local<Object> tmpl(Object::Cast(*args[1]));
            Local<Object> vals(Object::Cast(*res));
            Local<Array> tprops = tmpl->GetPropertyNames();
            for (int i = 0; i < tprops->Length(); i++)
            {
                Handle<Value> pN = tprops->Get(Int32::New(i));
                String::AsciiValue s1(pN);
                Handle<Value> pV = tmpl->Get(pN);
                String::AsciiValue s2(pV);
                LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "jsDocument::createElement: " << *s1 << ": " << *s2);
                vals->Set(pN, pV);
            }
        }
        //dc->elements.push_back(Persistent<Value>::New(res));
    }
    return scope.Close(res);
}

static Handle<Value> documentWrite(const Arguments& args)
{
    //enter a handle scope
    HandleScope scope;
    bool first = true;
    std::string res = "";

    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsDocument* dc = static_cast<jsDocument*>(ptr);

    for (int i = 0; i < args.Length(); i++) {
        if (first) {
            first = false;
        } else {
            res += " ";
        }
        res += value_to_string(args[i]);
    }
    LOG4CXX_DEBUG(webEngine::iLogger::GetLogger(), "jsDocument::write: " << res);
    append_results(res);

    return Undefined();
}

Handle<Value> Document(const Arguments& args)
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
        Local<External> wrap = Local<External>::Cast(eObj->GetInternalField(0));
        jsBrowser* jsExec = static_cast<jsBrowser*>(wrap->Value());
        jsWindow* win = jsExec->window;
        if (args.Length() > 0 && args[0]->IsObject())
        {
            Local<Object> winObj = Local<Object>::Cast(args[0]);
            Local<External> winPtr = Local<External>::Cast(winObj->GetInternalField(0));
            win = static_cast<jsWindow*>(winPtr->Value());
        }
        jsDocument *p = new jsDocument(win);
        retval = wrap_object<jsDocument>(p);
    }

    return scope.Close(retval);

    String::Utf8Value str(args[0]);
    return scope.Close(retval);
}

jsDocument::jsDocument(jsWindow* holder_)
{
    if (!isInit) {
        isInit = true;
        Handle<FunctionTemplate> _object = FunctionTemplate::New();
        if (!jsElement::is_init)
        {
            jsElement *elem = new jsElement;
        }
        Local<FunctionTemplate> _template = Local<FunctionTemplate>::New(jsElement::object_template);
        _object->Inherit(_template);
        //get the location's instance template
        Handle<ObjectTemplate> _proto = _object->InstanceTemplate();
        //set its internal field count to one (we'll put references to the C++ point here later)
        _proto->SetInternalFieldCount(1);

        // Add accessors for each of the fields of the Location.
        _proto->SetAccessor(String::NewSymbol("images"), DocumentGet, DocumentSet);
        _proto->SetAccessor(String::NewSymbol("forms"), DocumentGet, DocumentSet);
        _proto->Set(String::New("appendChild"), FunctionTemplate::New(js_el_append_child));
        _proto->Set(String::New("getElementsByTagName"), FunctionTemplate::New(getElementsByTagName));
        _proto->Set(String::New("createElement"), FunctionTemplate::New(createElement));
        _proto->Set(String::New("write"), FunctionTemplate::New(documentWrite));

        object_template = Persistent<FunctionTemplate>::New(_object);
    }
    holder = holder_;
}

jsDocument::~jsDocument(void)
{
}
