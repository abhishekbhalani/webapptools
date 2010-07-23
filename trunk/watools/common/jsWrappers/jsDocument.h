#pragma once
#include <v8/v8.h>
#include "jsElement.h"

namespace webEngine {
    class jsWindow;

    class jsDocument
    {
    public:
        jsDocument(jsWindow* holder_);
        ~jsDocument(void);

        static bool isInit;
        static v8::Persistent<v8::FunctionTemplate> object_template;

        std::map<std::string, v8::Persistent<v8::Value>>  namedprops;
    protected:
        jsWindow* holder;
    };
} //namespace webEngine

v8::Handle<v8::Value> Document(const v8::Arguments& args);
