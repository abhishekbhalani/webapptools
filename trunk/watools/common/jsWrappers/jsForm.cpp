#include <string>
#include <vector>
#include <map>
#include <weLogger.h>
#include "jsGlobal.h"
#include "jsForm.h"
#include "jsBrowser.h"

using namespace v8;
using namespace webEngine;

Persistent<FunctionTemplate> jsForm::object_template;
bool jsForm::is_init = false;


jsForm::jsForm( html_entity_ptr ent ) : jsElement(ent)
{
    if (!is_init) {
        init();
    }
}

jsForm::~jsForm( void )
{

}

void jsForm::init()
{
    is_init = true;
    Handle<FunctionTemplate> _object = FunctionTemplate::New();
    _object->Inherit(jsElement::object_template);
    //get the location's instance template
    Handle<ObjectTemplate> _proto = _object->InstanceTemplate();
    //set its internal field count to one (we'll put references to the C++ point here later)
    _proto->SetInternalFieldCount(1);

    // Add accessors for each of the fields.
    _proto->Set(String::New("toString"), FunctionTemplate::New(jsForm::ToString));
    _proto->SetNamedPropertyHandler(jsForm::PropertyGet, jsForm::PropertySet, NULL, NULL, jsElement::PropertyEnum);

    object_template = Persistent<FunctionTemplate>::New(_object);

}

Handle<Value> jsForm::GetProperty( Local<String> name, const AccessorInfo &info )
{
    Handle<Value> val = jsElement::GetProperty(name, info);

    if (val.IsEmpty()) {
        // try to search by names
        string key = value_to_string(name);
        entity_list elems = html_ent->FindTags("*");
        for(size_t i = 0; i < elems.size(); ++i) {
            string nm = elems[i]->attr("name");
            if (nm == key) {
                val = wrap_entity(boost::shared_dynamic_cast<html_entity>(elems[i]));
                break;
            }
        }
        ClearEntityList(elems);
    }

    return val;
}

Handle<Value> jsForm::SetProperty( Local<String> name, Local<Value> value, const AccessorInfo& info )
{
    Handle<Value> val = jsElement::SetProperty(name, value, info);
    return val;
}

Handle<Value> jsForm::PropertyGet( Local<String> name, const AccessorInfo &info )
{
    HandleScope scope;
    string key = value_to_string(name);

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsForm* el = static_cast<jsForm*>(ptr);
    Handle<Value> val = el->GetProperty(name, info);
    return scope.Close(val);
}

Handle<Value> jsForm::PropertySet( Local<String> name, Local<Value> value, const AccessorInfo& info )
{
    string key = value_to_string(name);
    Handle<Value> retval;

    vector<string>::iterator it = find(jsElement::funcs.begin(), jsElement::funcs.end(), key);
    if (it == jsElement::funcs.end()) {
        Local<Object> self = info.This();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        jsForm* el = static_cast<jsForm*>(ptr);
        retval = el->SetProperty(name, value, info);
    }
    return retval;
}

Handle<Value> jsForm::ToString( const Arguments& args )
{
    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsElement* el = static_cast<jsElement*>(ptr);

    string attr;
    string ret = "[Form";

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
