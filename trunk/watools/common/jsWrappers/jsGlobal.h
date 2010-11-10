#pragma once
#include <v8/v8.h>
#include <string>
#include <weHtmlEntity.h>
#include <html_tags.h>
#include <map>

namespace v8_wrapper {
class TreeNode;
}

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
    v8::HandleScope scope;
    v8::Local<v8::Object> _instance = v8_wrapper::Registrator< T >::GetTemplate()->GetFunction()->NewInstance(); //
    _instance->SetInternalField(0, v8::External::New(objToWrap));
    return scope.Close(_instance);
}

boost::shared_ptr<v8_wrapper::TreeNode> wrap_entity(webEngine::html_entity_ptr objToWrap);
