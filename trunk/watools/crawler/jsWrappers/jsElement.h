#pragma once
#include <v8/v8.h>
#include <string>
#include <vector>
#include <map>

class jsDomElement {
public:
    jsDomElement* parent;
    std::vector<v8::Persistent<v8::Value>> elements;

    jsDomElement();
};

class jsElement
{
public:
    jsElement(void);
    ~jsElement(void);

    static bool is_init;
    static v8::Persistent<v8::FunctionTemplate> object_template;
};

v8::Handle<v8::Value> js_el_append_child(const v8::Arguments& args);
v8::Handle<v8::Value> Image(const v8::Arguments& args);
v8::Handle<v8::Value> Element(const v8::Arguments& args);
