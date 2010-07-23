#pragma once
#include <v8/v8.h>
#include "jsDocument.h"

namespace webEngine {
    class jsBrowser;

    class jsWindow
    {
    public:
        jsWindow(jsBrowser* holder_, jsWindow* parent_ = NULL, jsWindow* creator_ = NULL);
        ~jsWindow(void);

        jsLocation location;
        jsDocument* document;
        static bool isInit;
        static v8::Persistent<v8::FunctionTemplate> object_template;

        static v8::Handle<v8::Value> GetDocument(v8::Local<v8::String> name, const v8::AccessorInfo &info);
        static v8::Handle<v8::Value> GetNavigator(v8::Local<v8::String> name, const v8::AccessorInfo &info);
        static v8::Handle<v8::Value> GetLocation(v8::Local<v8::String> name, const v8::AccessorInfo &info);
        static v8::Handle<v8::Value> GetParent(v8::Local<v8::String> name, const v8::AccessorInfo &info);
        static v8::Handle<v8::Value> GetTop(v8::Local<v8::String> name, const v8::AccessorInfo &info);
        static v8::Handle<v8::Value> GetSelf(v8::Local<v8::String> name, const v8::AccessorInfo &info);
    protected:
        jsBrowser* browser;
        jsWindow* parent;
        jsWindow* creator;
    };
} //namespace webEngine

v8::Handle<v8::Value> Window(const v8::Arguments& args);
