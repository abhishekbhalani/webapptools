#include <weLogger.h>
#include "jsGlobal.h"
#include "jsElement.h"
#include "jsBrowser.h"

using namespace v8;
using namespace webEngine;

/*extern bool ExecuteString(Handle<String> source,
                          Handle<Value> name,
                          bool print_result,
                          bool report_exceptions);*/

static char* func_list[] = {
    "toString",
    "appendChild"
};
static char* rw_list[] = {
    "accessKey",//
    "className",//
    "dir",
    "disabled",
    "height",//
    "id",//
    "innerHTML",
    "lang",
    "style",
    "tabIndex",
    "title",//
    "width"//
};
static char* ro_list[] = {
    "attributes",
    "childNodes",

    "clientHeight",
    "clientWidth",
    "firstChild",
    "lastChild",
    "length",
    "nextSibling",
    "nodeName",
    "nodeType",
    "nodeValue",
    "offsetHeight",
    "offsetLeft",
    "offsetParent",
    "offsetTop",
    "offsetWidth",
    "ownerDocument",
    "parentNode",
    "previousSibling",
    "scrollHeight",
    "scrollLeft",
    "scrollTop",
    "tagName"
};

std::vector<std::string> jsElement::funcs(func_list, func_list + sizeof func_list / sizeof func_list[ 0 ]);
std::vector<std::string> jsElement::rw_props(rw_list, rw_list + sizeof rw_list / sizeof rw_list[ 0 ]);
std::vector<std::string> jsElement::ro_props(ro_list, ro_list + sizeof ro_list / sizeof ro_list[ 0 ]);

Persistent<FunctionTemplate> jsElement::object_template;
bool jsElement::is_init = false;


/*static Handle<Value> GetElementChild(Local<String> name, const AccessorInfo& info)
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
}*/

/*Handle<Value> js_el_append_child(const Arguments& args)
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
}*/

Handle<Value> Image(const Arguments& args)
{
    HandleScope scope;
    Handle<Object> res;
    Local<Context> ctx = v8::Context::GetCurrent();
    Local<Value> exec = ctx->Global()->Get(String::New("v8_context"));
    LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::Window: gets v8_context");
    if (exec->IsObject())
    {
        Local<Object> eObj = Local<Object>::Cast(exec);
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(eObj->GetInternalField(0));
        jsBrowser* jsExec = static_cast<jsBrowser*>(wrap->Value());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::Window: gets jsBrowser");
        jsElement *p = new jsElement(jsExec->window->document->doc);
        p->entity->Name("img");
        res = wrap_object<jsElement>(p);
        append_object(res);
    }
    return scope.Close(res);
}

Handle<Value> Element(const Arguments& args)
{
    HandleScope scope;
    Handle<Object> res;
    Local<Context> ctx = v8::Context::GetCurrent();
    Local<Value> exec = ctx->Global()->Get(String::New("v8_context"));
    LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::Window: gets v8_context");
    if (exec->IsObject())
    {
        Local<Object> eObj = Local<Object>::Cast(exec);
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(eObj->GetInternalField(0));
        jsBrowser* jsExec = static_cast<jsBrowser*>(wrap->Value());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::Window: gets jsBrowser");
        if (args.Length() > 0) {
            jsElement *p = new jsElement(jsExec->window->document->doc);
            string tag = value_to_string(args[0]->ToString());
            p->entity->Name(tag);
            res = wrap_object<jsElement>(p);
            append_object(res);
        }
    }
    return scope.Close(res);
}

jsElement::jsElement(html_document_ptr document)
{
    if (!is_init) {
        init();
    }
    // todo - link to the document
    entity.reset(new html_entity());
}

jsElement::jsElement(html_entity_ptr ent)
{
    if (!is_init) {
        init();
    }
    entity = ent;
}

jsElement::~jsElement(void)
{
}

void jsElement::init()
{
    is_init = true;
    Handle<FunctionTemplate> _object = FunctionTemplate::New();
    //get the location's instance template
    Handle<ObjectTemplate> _proto = _object->InstanceTemplate();
    //set its internal field count to one (we'll put references to the C++ point here later)
    _proto->SetInternalFieldCount(1);

    // Add accessors for each of the fields of the Location.
    _proto->Set(String::New("toString"), FunctionTemplate::New(jsElement::ToString));
    _proto->Set(String::New("appendChild"), FunctionTemplate::New(jsElement::AppendChild));
    _proto->SetNamedPropertyHandler(jsElement::PropertyGet, jsElement::PropertySet, NULL, NULL, jsElement::PropertyEnum);

    object_template = Persistent<FunctionTemplate>::New(_object);
}

Handle<Value> jsElement::GetProperty( Local<String> name, const AccessorInfo &info )
{
    Handle<Value> val;
    std::string key = value_to_string(name);

    vector<string>::iterator iter;
    // Look up the value in the RW propeties list.
    iter = find(rw_props.begin(), rw_props.end(), key);
    if (iter != rw_props.end()) {
        if (key == "id" || key == "height" || key == "width" || key == "title" || key == "accessKey") {
            // "attributes" properties
            string sval = entity->attr(key);
            val = Local<Value>::New(String::New(sval.c_str()));
        }
        else if (key == "className") {
            // "attributes" property, but fix name
            string sval = entity->attr("class");
            val = Local<Value>::New(String::New(sval.c_str()));
        }
    }
    else {
        // Look up the value in the RO propeties list.
        iter = find(ro_props.begin(), ro_props.end(), key);
        if (iter != ro_props.end()) {

        }
        else {
            // Look up the value in the attributes list.
            AttrMap::iterator itmp;
            itmp = entity->attr_list().find(key);
            if (itmp != entity->attr_list().end())
            {
                val = Local<Value>::New(String::New(itmp->c_str()));
            } // attribute found
        } // RO property search
    } // RW property search

    return val;
}

Handle<Value> jsElement::SetProperty( Local<String> name, Local<Value> value, const AccessorInfo& info )
{
    Handle<Value> val;
    std::string key = value_to_string(name);

    vector<string>::iterator iter;
    // Look up the value in the RO propeties list.
    iter = find(ro_props.begin(), ro_props.end(), key);
    if (iter == ro_props.end()) {
        // Look up the value in the RW propeties list.
        iter = find(rw_props.begin(), rw_props.end(), key);
        if (iter != rw_props.end()) {
            if (key == "id" || key == "height" || key == "width" || key == "title" || key == "accessKey") {
                // "attributes" properties
                string sval = value_to_string(value);
                entity->attr(key, sval);
            }
            else if (key == "className") {
                // "attributes" property, but fix name
                string sval = value_to_string(value);
                entity->attr("class", sval);
            }
        }
        else {
            // set the attribute
            string sval = value_to_string(value);
            entity->attr(key, sval);
        } // RO property search
    } // RW property search
    val = value;

    return val;
}

Handle<Value> jsElement::PropertyGet( Local<String> name, const AccessorInfo &info )
{
    HandleScope scope;
    string key = value_to_string(name);

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsElement* el = static_cast<jsElement*>(ptr);
    Handle<Value> val = el->GetProperty(name, info);
    return scope.Close(val);
}

Handle<Value> jsElement::PropertySet( Local<String> name, Local<Value> value, const AccessorInfo& info )
{
    string key = value_to_string(name);
    Handle<Value> retval;

    vector<string>::iterator it = find(jsElement::funcs.begin(), jsElement::funcs.end(), key);
    if (it == jsElement::funcs.end()) {
        Local<Object> self = info.This();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        jsElement* el = static_cast<jsElement*>(ptr); 
        retval = el->SetProperty(name, value, info);
    }
    return retval;
}


Handle<Array> jsElement::PropertyEnum( const AccessorInfo &info )
{
    HandleScope scope;

    Handle<Array> retval = Local<Array>::New(Array::New());

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsElement* el = static_cast<jsElement*>(ptr); 

    map<string, int> prop_list;
    size_t i;
    for (i = 0; i < ro_props.size(); ++i) {
        prop_list[ro_props[i]] = 1;
    }
    for (i = 0; i < rw_props.size(); ++i) {
        prop_list[rw_props[i]] = 1;
    }
    AttrMap::iterator it;
    for (it = el->entity->attr_list().begin(); it != el->entity->attr_list().end(); ++it) {
        prop_list[el->entity->attr_list().key(it)] = 1;
    }
    map<string, int>::iterator ins;
    i = 0;
    for (ins = prop_list.begin(); ins != prop_list.end(); ++ins) {
        string val = ins->first;
        retval->Set(Number::New(i), String::New(val.c_str()));
        i++;
    }

    return scope.Close(retval);
}

Handle<Value> jsElement::AppendChild( const Arguments& args )
{
    LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "jsElement::AppendChild");

    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    jsElement* el = static_cast<jsElement*>(ptr);
    if (args.Length() > 0 && args[0]->IsObject()) {
        Local<Object> aref = args[0]->ToObject();
        Local<External> awrap = Local<External>::Cast(aref->GetInternalField(0));
        void* aptr = awrap->Value();
        jsElement* chld = static_cast<jsElement*>(aptr);
        el->entity->Children().push_back(chld->entity);
        chld->entity->Parent(el->entity);
    }
    else {
        LOG4CXX_ERROR(webEngine::iLogger::GetLogger(), "jsElement::AppendChild exception: argument must be an object!\n");
    }

    return Undefined();
}

Handle<Value> jsElement::ToString( const Arguments& args )
{
    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsElement* el = static_cast<jsElement*>(ptr); 

    string attr;
    string ret = "[Element; tag=";
    ret += el->entity->Name();

    attr = el->entity->attr("id");
    if (attr != "") {
        ret += "; id=";
        ret += attr;
    }
    attr = el->entity->attr("name");
    if (attr != "") {
        ret += "; name=";
        ret += attr;
    }
    ret += "]";
    return String::New(ret.c_str());

}