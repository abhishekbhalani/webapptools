#pragma once
#include <v8.h>
#include <string>
#include <weHtmlEntity.h>
#include <html_tags.h>
#include <map>

typedef std::map<std::string, v8::Persistent<v8::Value> > jsPropertyMap;

std::string value_to_string(const v8::Handle<v8::Value>& val);
v8::Handle<v8::Value> print(const v8::Arguments& args);
v8::Handle<v8::Value> alert(const v8::Arguments& args);
v8::Handle<v8::Value> version(const v8::Arguments& args);
v8::Handle<v8::Value> dump_object(const v8::Arguments& args);

void append_object(v8::Handle<v8::Object> data);
void append_results(const std::string& data);
