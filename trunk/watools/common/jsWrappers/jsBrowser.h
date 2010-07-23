#pragma once
#include "weJsExecutor.h"
#include "jsLocation.h"
#include "jsWindow.h"

using namespace std;

namespace webEngine {

    class jsNavigator {
    public:
        std::map<std::string, v8::Persistent<v8::Value>>  props;

        jsNavigator();

        static bool is_init;
        static v8::Persistent<v8::FunctionTemplate> object_template;
    };

    //////////////////////////////////////////////////////////////////////////
    /// @class jsBrowser
    /// 
    /// @brief Implements jsExecutor as headless browser
    ///
    //////////////////////////////////////////////////////////////////////////
    class jsBrowser : public jsExecutor
    {
    public:
        jsBrowser(void);
        ~jsBrowser(void);

        jsWindow* window;
        jsNavigator navigator;
        jsLocation  location;
//        jsDocument document;
        static v8::Handle<v8::Value> GetNavigator(v8::Local<v8::String> name, const v8::AccessorInfo &info);
        static v8::Handle<v8::Value> GetWindow(v8::Local<v8::String> name, const v8::AccessorInfo &info);
        static v8::Handle<v8::Value> GetLocation(v8::Local<v8::String> name, const v8::AccessorInfo &info);
    };
} //namespace webEngine
