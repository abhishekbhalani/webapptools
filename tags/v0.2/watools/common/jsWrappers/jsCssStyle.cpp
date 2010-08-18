#include <weLogger.h>
#include "jsGlobal.h"
#include "jsElement.h"
#include "jsCssStyle.h"

using namespace v8;
using namespace webEngine;

/*extern bool ExecuteString(Handle<String> source,
                          Handle<Value> name,
                          bool print_result,
                          bool report_exceptions);*/

static char* func_list[] = {
    "toString"
};
static char* rw_list[] = {
    "azimuth"
};
static char* ro_list[] = {
    "cssText"
};

std::vector<std::string> jsCssStyle::funcs(func_list, func_list + sizeof func_list / sizeof func_list[ 0 ]);
std::vector<std::string> jsCssStyle::rw_props(rw_list, rw_list + sizeof rw_list / sizeof rw_list[ 0 ]);
std::vector<std::string> jsCssStyle::ro_props(ro_list, ro_list + sizeof ro_list / sizeof ro_list[ 0 ]);

Persistent<FunctionTemplate> jsCssStyle::object_template;
bool jsCssStyle::is_init = false;

jsCssStyle::jsCssStyle(html_entity_ptr ent)
{
    if (!is_init) {
        init();
    }
    entity = ent;
}

jsCssStyle::~jsCssStyle(void)
{
}

void jsCssStyle::init()
{
    is_init = true;
    Handle<FunctionTemplate> _object = FunctionTemplate::New();
    //get the location's instance template
    Handle<ObjectTemplate> _proto = _object->InstanceTemplate();
    //set its internal field count to one (we'll put references to the C++ point here later)
    _proto->SetInternalFieldCount(1);

    // Add accessors for each of the fields of the Location.
    _proto->Set(String::New("toString"), FunctionTemplate::New(jsCssStyle::ToString));
    _proto->SetNamedPropertyHandler(jsCssStyle::PropertyGet, jsCssStyle::PropertySet, NULL, NULL, jsCssStyle::PropertyEnum);

    object_template = Persistent<FunctionTemplate>::New(_object);
}

Handle<Value> jsCssStyle::GetProperty( Local<String> name, const AccessorInfo &info )
{
    Handle<Value> val;
    std::string key = value_to_string(name);

    vector<string>::iterator iter;
    // Look up the value in the RW propeties list.
    iter = find(rw_props.begin(), rw_props.end(), key);
    if (iter != rw_props.end()) {
        // get the RW property
    }
    else {
        // Look up the value in the RO propeties list.
        iter = find(ro_props.begin(), ro_props.end(), key);
        if (iter != ro_props.end()) {
            // get the RO property
        }
        else {
            // Look up the value in the attributes list.
            val = props[key];
        } // RO property search
    } // RW property search

    return val;
}

Handle<Value> jsCssStyle::SetProperty( Local<String> name, Local<Value> value, const AccessorInfo& info )
{
    Handle<Value> val;
    std::string key = value_to_string(name);

    vector<string>::iterator iter;
    // Look up the value in the RO properties list.
    iter = find(ro_props.begin(), ro_props.end(), key);
    if (iter == ro_props.end()) {
        // Look up the value in the RW properties list.
        iter = find(rw_props.begin(), rw_props.end(), key);
        if (iter != rw_props.end()) {
            // set RW property
        }
        else {
            // set the attribute
            string sval = value_to_string(value);
            props[key] = Persistent<Value>::New(value);
        } // RW property search
    } // RO property search
    val = value;

    return val;
}

v8::Handle<v8::Array> webEngine::jsCssStyle::EnumProps( const v8::AccessorInfo &info )
{
    Handle<Array> retval = Local<Array>::New(Array::New());

    map<string, int> prop_list;
    size_t i;
    for (i = 0; i < ro_props.size(); ++i) {
        prop_list[ro_props[i]] = 1;
    }
    for (i = 0; i < rw_props.size(); ++i) {
        prop_list[rw_props[i]] = 1;
    }
    jsPropertyMap::iterator it;
    for (it = props.begin(); it != props.end(); ++it) {
        prop_list[it->first] = 1;
    }
    map<string, int>::iterator ins;
    i = 0;
    for (ins = prop_list.begin(); ins != prop_list.end(); ++ins) {
        string val = ins->first;
        retval->Set(Number::New(i), String::New(val.c_str()));
        i++;
    }
    return retval;
}

string jsCssStyle::cssString()
{
    return "{}";
}

void jsCssStyle::computeStyle( const string& style /*= ""*/ )
{
    // create new style

    // process argument
    parse(style);
    // fill from entity's 'style' attribute up to the DOM tree
    compute(entity);
}

void jsCssStyle::compute( base_entity_ptr ent )
{
    string code;
    // update style from entity
    code = entity->attr("style");
    parse(code);

    // process parent entity
    if (!ent->Parent().expired()) {
        compute(ent->Parent().lock());
    }
}

void webEngine::jsCssStyle::parse( string code )
{

}

Handle<Value> jsCssStyle::PropertyGet( Local<String> name, const AccessorInfo &info )
{
    HandleScope scope;
    string key = value_to_string(name);

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsCssStyle* el = static_cast<jsCssStyle*>(ptr);
    Handle<Value> val = el->GetProperty(name, info);
    return scope.Close(val);
}

Handle<Value> jsCssStyle::PropertySet( Local<String> name, Local<Value> value, const AccessorInfo& info )
{
    string key = value_to_string(name);
    Handle<Value> retval;

    vector<string>::iterator it = find(jsCssStyle::funcs.begin(), jsCssStyle::funcs.end(), key);
    if (it == jsCssStyle::funcs.end()) {
        Local<Object> self = info.This();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        jsCssStyle* el = static_cast<jsCssStyle*>(ptr); 
        retval = el->SetProperty(name, value, info);
    }
    return retval;
}


Handle<Array> jsCssStyle::PropertyEnum( const AccessorInfo &info )
{
    HandleScope scope;

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsCssStyle* el = static_cast<jsCssStyle*>(ptr); 

    Handle<Array> retval = el->EnumProps(info);
    return scope.Close(retval);
}

Handle<Value> jsCssStyle::ToString( const Arguments& args )
{
    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsCssStyle* el = static_cast<jsCssStyle*>(ptr); 

    string ret = "[CssStyle]";
    return String::New(ret.c_str());
}

/*Handle<Value> jsCssStyle::PlaceHolder( const Arguments& args )
{
    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsElement* el = static_cast<jsElement*>(ptr); 

    string ret;
    string attr;
    ret = value_to_string(args.Callee()->ToString());
    ret += " [Element; tag=";
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
    LOG4CXX_DEBUG(iLogger::GetLogger(), "jsElement::PlacHolder - " << ret);
    return String::New(ret.c_str());
}*/
