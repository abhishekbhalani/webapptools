#pragma once
#include <v8/v8.h>
#include <string>
#include <weHtmlEntity.h>

typedef std::map<std::string, v8::Persistent<v8::Value> > jsPropertyMap;

std::string value_to_string(const v8::Local<v8::Value>& val);
v8::Handle<v8::Value> print(const v8::Arguments& args);
v8::Handle<v8::Value> alert(const v8::Arguments& args);
v8::Handle<v8::Value> version(const v8::Arguments& args);
v8::Handle<v8::Value> dump_object(const v8::Arguments& args);

void append_object(v8::Handle<v8::Object> data);
void append_results(const std::string& data);

template<class T> v8::Handle<v8::Object> wrap_object(void *objToWrap)
{
    //enter a handle scope
    v8::HandleScope scope;

    //create a new point instance loc_proto = locationTemplate->InstanceTemplate();
    v8::Local<v8::FunctionTemplate> _template = v8::Local<v8::FunctionTemplate>::New(T::object_template);
    v8::Local<v8::Object> _instance = _template->GetFunction()->NewInstance(); //

    //set that internal field
    _instance->SetInternalField(0, v8::External::New(objToWrap));

    //to prevent the point_instance from being destroyed when its
    //scope handle_scope is, use the Close() function
    return scope.Close(_instance);
}

v8::Handle<v8::Object> wrap_entity(webEngine::html_entity_ptr objToWrap);