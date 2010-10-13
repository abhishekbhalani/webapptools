#pragma once
#include <v8/v8.h>
#include "jsElement.h"

namespace webEngine {
class jsWindow;

class jsForm : public jsElement {
public:
    jsForm(html_entity_ptr ent);
    ~jsForm(void);

    virtual html_entity_ptr entity() {
        return html_ent;
    }

    virtual v8::Handle<v8::Value> GetProperty(v8::Local<v8::String> name, const v8::AccessorInfo &info);
    virtual v8::Handle<v8::Value> SetProperty(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& info);

    static bool is_init;
    static v8::Persistent<v8::FunctionTemplate> object_template;

    static void init();
    static v8::Handle<v8::Value> PropertyGet(v8::Local<v8::String> name, const v8::AccessorInfo &info);
    static v8::Handle<v8::Value> PropertySet(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& info);
    static v8::Handle<v8::Value> ToString(const v8::Arguments& args);
};
} //namespace webEngine

v8::Handle<v8::Value> Document(const v8::Arguments& args);
