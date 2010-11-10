#pragma once
#include <v8/v8.h>
#include <weUrl.h>
#include <v8_wrapper.h>

namespace webEngine {
class jsWindow;

class jsLocation: public v8_wrapper::Registrator<jsLocation> {
public:
    jsLocation(jsWindow* parent);
    ~jsLocation(void);
    transport_url url;
    jsWindow* win;

    static bool is_init;
    static v8::Persistent<v8::FunctionTemplate> object_template;
};
} //namespace webEngine

v8::Handle<v8::Value> Location(const v8::Arguments& args);
