#pragma once
#include <v8/v8.h>
#include "jsDocument.h"

namespace webEngine {
    class jsBrowser;
    class jsWindow;

    class jsHistory {
    public:
        int index;
        std::vector<std::string>  history;

        jsHistory(jsWindow *win);

        jsWindow *window;

        void push_back(const std::string& url);
        std::string forward();
        std::string back();
        std::string go(v8::Handle<v8::Value> pos);

        static bool is_init;
        static v8::Persistent<v8::FunctionTemplate> object_template;

        static v8::Handle<v8::Value> GetLength( v8::Local<v8::String> name, const v8::AccessorInfo &info );
        static v8::Handle<v8::Value> Forward(const v8::Arguments& args);
        static v8::Handle<v8::Value> Back(const v8::Arguments& args);
        static v8::Handle<v8::Value> Go(const v8::Arguments& args);
    };


    class jsWindow
    {
    public:
        jsWindow(jsBrowser* holder_, jsWindow* parent_ = NULL, jsWindow* creator_ = NULL);
        ~jsWindow(void);

        const string& get_id() const { return win_uuid; }
        const bool is_closed();
        const bool is_property(string key);

        jsLocation location;
        jsDocument* document;
        jsHistory* history;
		std::map<std::string, v8::Persistent<v8::Value>>  props;

        static bool is_init;
        static v8::Persistent<v8::FunctionTemplate> object_template;

        v8::Handle<v8::Value> GetProperty(v8::Local<v8::String> name, const v8::AccessorInfo &info);
		v8::Handle<v8::Value> SetProperty(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& info);
        /*static v8::Handle<v8::Value> GetDocument(v8::Local<v8::String> name, const v8::AccessorInfo &info);
        static v8::Handle<v8::Value> GetNavigator(v8::Local<v8::String> name, const v8::AccessorInfo &info);
        static v8::Handle<v8::Value> GetLocation(v8::Local<v8::String> name, const v8::AccessorInfo &info);
        static v8::Handle<v8::Value> GetParent(v8::Local<v8::String> name, const v8::AccessorInfo &info);
        static v8::Handle<v8::Value> GetTop(v8::Local<v8::String> name, const v8::AccessorInfo &info);
        static v8::Handle<v8::Value> GetSelf(v8::Local<v8::String> name, const v8::AccessorInfo &info);*/
        static v8::Handle<v8::Value> WindowGet(v8::Local<v8::String> name, const v8::AccessorInfo &info);
        static v8::Handle<v8::Value> WindowSet(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& info);

        static v8::Handle<v8::Value> Close(const v8::Arguments& args);
        static v8::Handle<v8::Value> Focus(const v8::Arguments& args);
        static v8::Handle<v8::Value> Open(const v8::Arguments& args);

        static v8::Handle<v8::Value> toString(const v8::Arguments& args);
        static v8::Handle<v8::Value> PlaceHolder(const v8::Arguments& args);
    protected:
        string win_uuid;
        jsBrowser* browser;
        jsWindow* parent;
        jsWindow* creator;

        static std::vector<std::string> ro_props;
        static std::vector<std::string> funcs;
    };
} //namespace webEngine

v8::Handle<v8::Value> Window(const v8::Arguments& args);
