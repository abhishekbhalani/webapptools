#pragma once
#include "jsExecutor.h"
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

    class jsScreen {
    public:
        std::map<std::string, v8::Persistent<v8::Value>>  props;

        jsScreen();

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
        std::map<std::string, v8::Persistent<v8::Value>>  props;

        jsNavigator navigator;
        jsScreen screen;
        jsWindow* window;
        jsWindow* topmost() const { return top_win; }
        jsWindow* window_by_id(std::string id);
        jsWindow* window_by_name(std::string name);
        void register_window(jsWindow* win) { win_list[win->get_id()] = win; }
//        jsDocument document;
        static bool is_init;
        static v8::Handle<v8::Value> GetWindow(v8::Local<v8::String> name, const v8::AccessorInfo &info);
    protected:
        jsWindow* top_win;
        typedef std::map<std::string, jsWindow*> windows_list;
        windows_list win_list;
    };
} //namespace webEngine
