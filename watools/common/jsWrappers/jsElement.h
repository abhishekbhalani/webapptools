#pragma once
#include <v8/v8.h>
#include <string>
#include <vector>
#include <map>
#include <weHtmlEntity.h>
#include "jsGlobal.h"

namespace webEngine {
    class jsCssStyle;

    class jsElement
    {
    public:
        jsElement(html_document_ptr document);
        jsElement(html_entity_ptr ent);
        ~jsElement(void);

        virtual html_entity_ptr entity() { return html_ent; }

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
        static v8::Handle<v8::Value> GetAttribute(const v8::Arguments& args);
        static v8::Handle<v8::Value> SetAttribute(const v8::Arguments& args);
        static v8::Handle<v8::Value> GetElemsByName(const v8::Arguments& args);
        static v8::Handle<v8::Value> CloneNode(const v8::Arguments& args);

        static v8::Handle<v8::Value> PlaceHolder(const v8::Arguments& args);
    protected:
        html_entity_ptr html_ent;
        jsCssStyle* css_style;

        static std::vector<std::string> rw_props;
        static std::vector<std::string> ro_props;
        static std::vector<std::string> funcs;
    };
} //namespace webEngine

v8::Handle<v8::Value> Image(const v8::Arguments& args);
v8::Handle<v8::Value> Element(const v8::Arguments& args);

// nodeType constants
#define ELEMENT_NODE                1
#define ATTRIBUTE_NODE              2
#define TEXT_NODE                   3
#define CDATA_SECTION_NODE          4
#define ENTITY_REFERENCE_NODE       5
#define ENTITY_NODE                 6
#define PROCESSING_INSTRUCTION_NODE 7
#define COMMENT_NODE                8
#define DOCUMENT_NODE               9
#define DOCUMENT_TYPE_NODE          10
#define DOCUMENT_FRAGMENT_NODE      11
#define NOTATION_NODE               12
#define NAMESPACE_NODE              13
