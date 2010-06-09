#pragma once
#include <v8/v8.h>
#include "jsElement.h"

class jsDomDocument : public jsDomElement
{
public:
    std::map<std::string, v8::Persistent<v8::Value>>  namedprops;

    jsDomDocument();
};

class jsDocument
{
public:
    jsDocument(void);
    ~jsDocument(void);

    static bool isInit;
    static v8::Persistent<v8::FunctionTemplate> object_template;
};

v8::Handle<v8::Value> Document(const v8::Arguments& args);
