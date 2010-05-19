#pragma once
#include <v8/v8.h>
#include <weUrl.h>

class js_location
{
public:
    js_location(void);
    ~js_location(void);

    static bool is_init;
    static v8::Persistent<v8::FunctionTemplate> object_template;
};

v8::Handle<v8::Value> Location(const v8::Arguments& args);
