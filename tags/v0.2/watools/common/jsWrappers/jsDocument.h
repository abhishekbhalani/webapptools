#pragma once
#include <v8/v8.h>
#include "jsElement.h"

namespace webEngine {
    class jsWindow;

    class jsDocument : public jsElement
    {
    public:
        jsDocument(jsWindow* holder_);
        ~jsDocument(void);

        virtual html_entity_ptr entity() { return doc; }
        html_document_ptr doc;

        static bool isInit;
        static v8::Persistent<v8::FunctionTemplate> object_template;
        static v8::Handle<v8::Value> PlaceHolder(const v8::Arguments& args);
        static v8::Handle<v8::Value> AppendChild(const v8::Arguments& args);

        jsPropertyMap  namedprops;
    protected:
        jsWindow* holder;

        static std::vector<std::string> rw_props;
        static std::vector<std::string> ro_props;
        static std::vector<std::string> funcs;
    };
} //namespace webEngine

v8::Handle<v8::Value> Document(const v8::Arguments& args);
