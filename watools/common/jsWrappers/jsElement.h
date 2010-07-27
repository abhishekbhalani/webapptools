#pragma once
#include <v8/v8.h>
#include <string>
#include <vector>
#include <map>
#include <weHtmlEntity.h>
#include "jsGlobal.h"

namespace webEngine {
    class jsElement
    {
    public:
        jsElement(html_document_ptr document);
        jsElement(html_entity_ptr ent);
        ~jsElement(void);

        html_entity_ptr entity;

        v8::Handle<v8::Value> GetProperty(v8::Local<v8::String> name, const v8::AccessorInfo &info);
        v8::Handle<v8::Value> SetProperty(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& info);

        static bool is_init;
        static v8::Persistent<v8::FunctionTemplate> object_template;

        static void init();
        static v8::Handle<v8::Value> PropertyGet(v8::Local<v8::String> name, const v8::AccessorInfo &info);
        static v8::Handle<v8::Value> PropertySet(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& info);
        static v8::Handle<v8::Array> PropertyEnum(const v8::AccessorInfo &info);
        static v8::Handle<v8::Value> ToString(const v8::Arguments& args);
        static v8::Handle<v8::Value> AppendChild(const v8::Arguments& args);
    protected:
        static std::vector<std::string> rw_props;
        static std::vector<std::string> ro_props;
        static std::vector<std::string> funcs;
    };
} //namespace webEngine

v8::Handle<v8::Value> Image(const v8::Arguments& args);
v8::Handle<v8::Value> Element(const v8::Arguments& args);
