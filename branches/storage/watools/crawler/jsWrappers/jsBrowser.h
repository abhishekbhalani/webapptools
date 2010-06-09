#pragma once
#include <v8/v8.h>

class jsBrowser
{
public:
    jsBrowser(void);
    ~jsBrowser(void);

    static bool is_init;
    static v8::Persistent<v8::FunctionTemplate> object_template;
};

v8::Handle<v8::Value> Browser(const v8::Arguments& args);
