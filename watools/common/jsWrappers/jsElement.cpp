#include <boost/algorithm/string.hpp>
#include <weLogger.h>
#include "jsGlobal.h"
#include "jsElement.h"
#include "jsBrowser.h"
#include "jsCssStyle.h"

using namespace v8;
using namespace webEngine;

/*extern bool ExecuteString(Handle<String> source,
                          Handle<Value> name,
                          bool print_result,
                          bool report_exceptions);*/

static char* func_list[] = {
    "appendChild",
    "blur",
    "click",
    "cloneNode",
    "focus",
    "getAttribute",
    "getElementsByTagName",
    "hasChildNodes",
    "insertBefore",
    "item",
    "normalize",
    "removeAttribute",
    "removeChild",
    "replaceChild",
    "setAttribute",
    "toString"
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
        p->entity()->Name("img");
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
            p->entity()->Name(tag);
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
    html_ent.reset(new html_entity());
    css_style = new jsCssStyle(html_ent);
}

jsElement::jsElement(html_entity_ptr ent)
{
    if (!is_init) {
        init();
    }
    html_ent = ent;
    css_style = new jsCssStyle(html_ent);
}

jsElement::~jsElement(void)
{
    delete css_style;
}

void jsElement::init()
{
    is_init = true;
    Handle<FunctionTemplate> _object = FunctionTemplate::New();
    //get the location's instance template
    Handle<ObjectTemplate> _proto = _object->InstanceTemplate();
    //set its internal field count to one (we'll put references to the C++ point here later)
    _proto->SetInternalFieldCount(1);

    // Add accessors for each of the fields.
    _proto->Set(String::New("toString"), FunctionTemplate::New(jsElement::ToString));
    _proto->Set(String::New("appendChild"), FunctionTemplate::New(jsElement::AppendChild));
    _proto->Set(String::New("blur"), FunctionTemplate::New(jsElement::PlaceHolder));
    _proto->Set(String::New("click"), FunctionTemplate::New(jsElement::PlaceHolder));
    _proto->Set(String::New("cloneNode"), FunctionTemplate::New(jsElement::CloneNode));
    _proto->Set(String::New("focus"), FunctionTemplate::New(jsElement::PlaceHolder));
    _proto->Set(String::New("getAttribute"), FunctionTemplate::New(jsElement::GetAttribute));
    _proto->Set(String::New("getElementsByTagName"), FunctionTemplate::New(jsElement::GetElemsByName));
    _proto->Set(String::New("hasChildNodes"), FunctionTemplate::New(jsElement::PlaceHolder));
    _proto->Set(String::New("insertBefore"), FunctionTemplate::New(jsElement::PlaceHolder));
    _proto->Set(String::New("item"), FunctionTemplate::New(jsElement::PlaceHolder));
    _proto->Set(String::New("normalize"), FunctionTemplate::New(jsElement::PlaceHolder));
    _proto->Set(String::New("removeAttribute"), FunctionTemplate::New(jsElement::PlaceHolder));
    _proto->Set(String::New("removeChild"), FunctionTemplate::New(jsElement::PlaceHolder));
    _proto->Set(String::New("replaceChild"), FunctionTemplate::New(jsElement::PlaceHolder));
    _proto->Set(String::New("setAttribute"), FunctionTemplate::New(jsElement::SetAttribute));
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
            string sval = entity()->attr(key);
            val = Local<Value>::New(String::New(sval.c_str()));
        }
        else if (key == "className") {
            // "attributes" property, but fix name
            string sval = entity()->attr("class");
            val = Local<Value>::New(String::New(sval.c_str()));
        }
        else if (key == "style") {
            val = wrap_object<jsCssStyle>(css_style);
        }
        else if (key == "innerHTML") {
            string sval = entity()->InnerHtml();
            val = Local<Value>::New(String::New(sval.c_str()));
        }
    }
    else {
        // Look up the value in the RO propeties list.
        iter = find(ro_props.begin(), ro_props.end(), key);
        if (iter != ro_props.end()) {
            val = Local<Value>::New(Undefined());
            if (key == "childNodes") {
                Handle<Array> elems = Local<Array>::New(Array::New());

                entity_list ptrs = entity()->Children();
                for(size_t i = 0; i < ptrs.size(); ++i) {
                    jsElement* e = new jsElement(boost::shared_dynamic_cast<html_entity>(ptrs[i]));
                    Handle<Object> w = wrap_object<jsElement>(e);
                    elems->Set(Number::New(i), w);
                }
                val = elems;
            }
            else if (key == "firstChild") {
                if (entity()->Children().size() > 0) {
                    jsElement* p = new jsElement(boost::shared_dynamic_cast<html_entity>(entity()->Child(0)));
                    val = wrap_object<jsElement>(p);
                }
            }
            else if (key == "lastChild") {
                if (entity()->Children().size() > 0) {
                    int idx = entity()->Children().size() - 1;
                    jsElement* p = new jsElement(boost::shared_dynamic_cast<html_entity>(entity()->Child(idx)));
                    val = wrap_object<jsElement>(p);
                }
            }
            else if (key == "nodeName") {
                string sval = entity()->Name();
                boost::to_upper(sval);
                val = String::New(sval.c_str());
            }
            else if (key == "nodeType") {
                string sval = entity()->Name();
                int ntp = ELEMENT_NODE;
                if (sval == "#document") {
                    ntp = DOCUMENT_NODE;
                }
                else if (sval == "#text") {
                    ntp = TEXT_NODE;
                }
                else if (sval == "#comment") {
                    ntp = COMMENT_NODE;
                }
                else if (sval == "#cdata") {
                    ntp = CDATA_SECTION_NODE;
                }
                else if (sval == "#fragment") {
                    ntp = DOCUMENT_FRAGMENT_NODE;
                }
                val = Number::New(ntp);
            }
            else if (key == "nodeValue") {
                string sval = entity()->Name();
                if (sval == "#text" || sval == "#cdata") {
                    sval = entity()->attr("");
                }
                else {
                    sval = entity()->attr("value");
                }
                val = String::New(sval.c_str());
            }
            else if (key == "ownerDocument") {
                html_document_ptr d((html_document*)entity()->GetRootDocument());
                jsDocument* jd = new jsDocument(NULL);
                jd->doc = d;
                val = wrap_object<jsDocument>(jd);
            }
            else if (key == "tagName") {
                string sval = entity()->Name();
                val = String::New(sval.c_str());
            }
        }
        else {
            // Look up the value in the attributes list.
            AttrMap::iterator itmp;
            itmp = entity()->attr_list().find(key);
            if (itmp != entity()->attr_list().end())
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
                entity()->attr(key, sval);
            }
            else if (key == "className") {
                // "attributes" property, but fix name
                string sval = value_to_string(value);
                entity()->attr("class", sval);
            }
            else if (key == "style" && value->IsString()) {
                string code = value_to_string(value);
                css_style->computeStyle(code);
            }
            else if (key == "innerHTML" && value->IsString()) {
                // clear previous values
                entity()->ClearChildren();
                string code = value_to_string(value);
                str_tag_stream stream(code.c_str());
                tag_scanner scanner(stream);
                entity()->Parse("", scanner, NULL);
            }
        }
        else {
            // set the attribute
            string sval = value_to_string(value);
            entity()->attr(key, sval);
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
    for (it = el->entity()->attr_list().begin(); it != el->entity()->attr_list().end(); ++it) {
        prop_list[el->entity()->attr_list().key(it)] = 1;
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

Handle<Value> jsElement::PlaceHolder( const Arguments& args )
{
    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsElement* el = static_cast<jsElement*>(ptr); 

    string ret;
    string attr;
    ret = value_to_string(args.Callee()->ToString());
    ret += " [Element; tag=";
    ret += el->entity()->Name();

    attr = el->entity()->attr("id");
    if (attr != "") {
        ret += "; id=";
        ret += attr;
    }
    attr = el->entity()->attr("name");
    if (attr != "") {
        ret += "; name=";
        ret += attr;
    }
    ret += "]";
    LOG4CXX_DEBUG(iLogger::GetLogger(), "jsElement::PlacHolder - " << ret);
    return String::New(ret.c_str());
}

Handle<Value> jsElement::AppendChild( const Arguments& args )
{
    LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "jsElement::AppendChild");
    HandleScope scope;

    Handle<Value> retval;

    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    jsElement* el = static_cast<jsElement*>(ptr);
    if (args.Length() > 0 && args[0]->IsObject()) {
        Local<Object> aref = args[0]->ToObject();
        Local<External> awrap = Local<External>::Cast(aref->GetInternalField(0));
        void* aptr = awrap->Value();
        jsElement* chld = static_cast<jsElement*>(aptr);
        el->entity()->Children().push_back(chld->entity());
        chld->entity()->Parent(el->entity());
        retval = args[0];
    }
    else {
        LOG4CXX_ERROR(webEngine::iLogger::GetLogger(), "jsElement::AppendChild exception: argument must be an object!\n");
    }

    return scope.Close(retval);
}

Handle<Value> jsElement::ToString( const Arguments& args )
{
    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsElement* el = static_cast<jsElement*>(ptr); 

    string attr;
    string ret = "[Element; tag=";
    ret += el->entity()->Name();

    attr = el->entity()->attr("id");
    if (attr != "") {
        ret += "; id=";
        ret += attr;
    }
    attr = el->entity()->attr("name");
    if (attr != "") {
        ret += "; name=";
        ret += attr;
    }
    ret += "]";
    return String::New(ret.c_str());
}

Handle<Value> jsElement::GetAttribute( const Arguments& args )
{
    HandleScope scope;
    Handle<Value> retval;

    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsElement* el = static_cast<jsElement*>(ptr); 

    string attr;
    string key;

    if (args.Length() > 0) {
        key = value_to_string(args[0]);
        attr = el->entity()->attr(key);
        retval = String::New(attr.c_str());
    }

    return scope.Close(retval);
}

Handle<Value> jsElement::SetAttribute( const Arguments& args )
{
    HandleScope scope;
    Handle<Value> retval(Undefined());

    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsElement* el = static_cast<jsElement*>(ptr); 

    string attr;
    string key;

    if (args.Length() > 0) {
        key = value_to_string(args[0]);
        if (args.Length() > 1) {
            attr = value_to_string(args[1]);
            el->entity()->attr(key, attr);
            retval = String::New(attr.c_str());
        }
    }

    return scope.Close(retval);
}

Handle<Value> jsElement::GetElemsByName( const Arguments& args )
{
    HandleScope scope;
    Handle<Value> retval(Undefined());

    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsElement* el = static_cast<jsElement*>(ptr); 

    string key;

    if (args.Length() > 0) {
        Handle<Array> elems = Local<Array>::New(Array::New());

        key = value_to_string(args[0]);
        entity_list ptrs = el->entity()->FindTags(key);
        for(size_t i = 0; i < ptrs.size(); ++i) {
            jsElement* e = new jsElement(boost::shared_dynamic_cast<html_entity>(ptrs[i]));
            Handle<Object> w = wrap_object<jsElement>(e);
            elems->Set(Number::New(i), w);
        }
        retval = elems;
    }

    return scope.Close(retval);
}

Handle<Value> jsElement::CloneNode( const Arguments& args )
{
    HandleScope scope;
    Handle<Value> retval(Undefined());

    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsElement* el = static_cast<jsElement*>(ptr); 

    /// @todo Warning! Fake clone! Implement real clone operation for entity
    html_entity_ptr hte(new html_entity());
    *hte = *(el->entity());
    jsElement* p = new jsElement(hte);
    retval = wrap_object<jsElement>(p);

    return scope.Close(retval);
}