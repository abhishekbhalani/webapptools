#include <string>
#include <map>
#include <weLogger.h>
#include "jsGlobal.h"
#include "jsBrowser.h"
#include "jsElement.h"

using namespace v8;
using namespace webEngine;

bool jsBrowser::is_init = false;

Persistent<FunctionTemplate> jsNavigator::object_template;
bool jsNavigator::is_init = false;
Persistent<FunctionTemplate> jsScreen::object_template;
bool jsScreen::is_init = false;

static Handle<Value> NavigatorGet(Local<String> name, const AccessorInfo &info)
{
    HandleScope scope;

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsNavigator* el = static_cast<jsNavigator*>(ptr);
    // Convert the JavaScript string to a std::string.
    std::string key = value_to_string(name);

    // Look up the value if it exists using the standard STL idiom.
    std::map<std::string, Persistent<Value>>::iterator iter = el->props.find(key);

    // If the key is not present return an empty handle as signal.
    if (iter == el->props.end()) return Handle<Value>();

    // Otherwise fetch the value and wrap it in a JavaScript string.
    Local<Value> val = Local<Value>::New(iter->second);
    return scope.Close(val);
}

static Handle<Value> NavigatorSet(Local<String> name, Local<Value> value, const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsNavigator* el = static_cast<jsNavigator*>(ptr);
    // Convert the JavaScript string to a std::string.
    std::string key = value_to_string(name);

    el->props[key] = Persistent<Value>::New(value);
    return value;
}

jsNavigator::jsNavigator()
{
    props["appCodeName"] = Persistent<Value>::New(String::New("Mozilla"));
    props["appMinorVersion"] = Persistent<Value>::New(Int32::New(0));
    props["appName"] = Persistent<Value>::New(String::New("Netscape"));
    props["appVersion"] = Persistent<Value>::New(String::New("5.0 (Windows; ru)"));
    props["cookieEnabled"] = Persistent<Value>::New(Boolean::New(true));
    props["javaEnabled"] = Persistent<Value>::New(Boolean::New(false));
    props["cpuClass"] = Persistent<Value>::New(String::New(""));
    props["onLine"] = Persistent<Value>::New(Boolean::New(true));
    props["platform"] = Persistent<Value>::New(String::New("Win32"));
    props["userAgent"] = Persistent<Value>::New(String::New("Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US) PT-MaxPatrol V8/3022"));
    props["browserLanguage"] = Persistent<Value>::New(String::New(""));
    props["systemLanguage"] = Persistent<Value>::New(String::New(""));
    props["userLanguage"] = Persistent<Value>::New(String::New(""));

    if (!is_init) {
        is_init = true;
        Handle<FunctionTemplate> _object = FunctionTemplate::New();
        //get the location's instance template
        Handle<ObjectTemplate> _proto = _object->InstanceTemplate();
        //set its internal field count to one (we'll put references to the C++ point here later)
        _proto->SetInternalFieldCount(1);

        // Add accessors for each of the fields of the Location.
        _proto->SetNamedPropertyHandler(NavigatorGet, NavigatorSet);

        object_template = Persistent<FunctionTemplate>::New(_object);
    }
}

static Handle<Value> ScreenGet(Local<String> name, const AccessorInfo &info)
{
    HandleScope scope;

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsScreen* el = static_cast<jsScreen*>(ptr);
    // Convert the JavaScript string to a std::string.
    std::string key = value_to_string(name);

    // Look up the value if it exists using the standard STL idiom.
    std::map<std::string, Persistent<Value>>::iterator iter = el->props.find(key);

    // If the key is not present return an empty handle as signal.
    if (iter == el->props.end()) return Handle<Value>();

    // Otherwise fetch the value and wrap it in a JavaScript string.
    Local<Value> val = Local<Value>::New(iter->second);
    return scope.Close(val);
}

static Handle<Value> ScreenSet(Local<String> name, Local<Value> value, const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsScreen* el = static_cast<jsScreen*>(ptr);
    // Convert the JavaScript string to a std::string.
    std::string key = value_to_string(name);

    el->props[key] = Persistent<Value>::New(value);
    return value;
}

jsScreen::jsScreen()
{
    props["availHeight"] = Persistent<Value>::New(Int32::New(600));
    props["availWidth"] = Persistent<Value>::New(Int32::New(800));
    props["colorDepth"] = Persistent<Value>::New(Int32::New(32));
    props["height"] = Persistent<Value>::New(Int32::New(600));
    props["pixelDepth"] = Persistent<Value>::New(Int32::New(32));
    props["width"] = Persistent<Value>::New(Int32::New(800));

    if (!is_init) {
        is_init = true;
        Handle<FunctionTemplate> _object = FunctionTemplate::New();
        //get the location's instance template
        Handle<ObjectTemplate> _proto = _object->InstanceTemplate();
        //set its internal field count to one (we'll put references to the C++ point here later)
        _proto->SetInternalFieldCount(1);

        // Add accessors for each of the fields of the Location.
        _proto->SetNamedPropertyHandler(ScreenGet, ScreenSet);

        object_template = Persistent<FunctionTemplate>::New(_object);
    }
}

static Handle<Value> BrowserGet(Local<String> name, const AccessorInfo &info)
{
    HandleScope scope;
    Handle<Value> val;
    std::string key = value_to_string(name);

    LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "jsBrowserGet: property("<< key <<")");

    Local<External> wrap = Local<External>::Cast(info.Data());
    void* ptr = wrap->Value();
    jsBrowser* el = static_cast<jsBrowser*>(ptr);
        if (key == "v8_context") {
        val = wrap_object<jsExecutor>(el);
    }
    else if (key == "window") {
        val = wrap_object<jsWindow>(el->window);
    }
    else if (el->window->is_property(key)) {
        val = el->window->GetProperty(name, info);
    }
    else {
        // Look up the value if it exists using the standard STL idiom.
        jsPropertyMap::iterator iter = el->props.find(key);
        // If the key is not present return an empty handle as signal.
        if (iter != el->props.end()) {
            // Otherwise fetch the value and wrap it in a JavaScript string.
            val = Local<Value>::New(iter->second);
        }
    }
 
    return scope.Close(val);
}

static Handle<Value> BrowserSet(Local<String> name, Local<Value> value, const AccessorInfo& info)
{
    Handle<Value> val;
    std::string key = value_to_string(name);
    
    LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "jsBrowserSet: property("<< key <<")");
    Local<External> wrap = Local<External>::Cast(info.Data());
    void* ptr = wrap->Value();
    if (ptr != NULL) {
        jsBrowser* el = static_cast<jsBrowser*>(ptr);
        if (el->window->is_property(key)) {
            val = el->window->GetProperty(name, info);
        }
        else {
            el->props[key] = Persistent<Value>::New(value);
        }
    }
    return val;
}

jsBrowser::jsBrowser(void)
{
    if (!is_init) {
        is_init = true;
        init_globals();
        // assign extensions
        Handle<Value> self = External::New(this);
        global->SetNamedPropertyHandler(BrowserGet, BrowserSet, NULL, NULL, NULL, self);
        //global->SetAccessor(String::New("window"), GetWindow, NULL, self);
        global->Set(String::New("Location"), FunctionTemplate::New(Location));
        global->Set(String::New("Window"), FunctionTemplate::New(Window));
        global->Set(String::New("Element"), FunctionTemplate::New(Element));
        global->Set(String::New("Image"), FunctionTemplate::New(Image));

        // intercept window's functions
        global->Set(String::NewSymbol("blur"), FunctionTemplate::New(jsBrowser::WinInterceptor));
        global->Set(String::NewSymbol("clearInterval"), FunctionTemplate::New(jsBrowser::WinInterceptor));
        global->Set(String::NewSymbol("clearTimeout"), FunctionTemplate::New(jsBrowser::WinInterceptor));
        global->Set(String::NewSymbol("close"), FunctionTemplate::New(jsBrowser::WinInterceptor));
        global->Set(String::NewSymbol("confirm"), FunctionTemplate::New(jsBrowser::WinInterceptor));
        global->Set(String::NewSymbol("createPopup"), FunctionTemplate::New(jsBrowser::WinInterceptor));
        global->Set(String::NewSymbol("focus"), FunctionTemplate::New(jsBrowser::WinInterceptor));
        global->Set(String::NewSymbol("moveBy"), FunctionTemplate::New(jsBrowser::WinInterceptor));
        global->Set(String::NewSymbol("moveTo"), FunctionTemplate::New(jsBrowser::WinInterceptor));
        global->Set(String::NewSymbol("open"), FunctionTemplate::New(jsBrowser::WinInterceptor));
        global->Set(String::NewSymbol("print"), FunctionTemplate::New(jsBrowser::WinInterceptor));
        global->Set(String::NewSymbol("prompt"), FunctionTemplate::New(jsBrowser::WinInterceptor));
        global->Set(String::NewSymbol("resizeBy"), FunctionTemplate::New(jsBrowser::WinInterceptor));
        global->Set(String::NewSymbol("resizeTo"), FunctionTemplate::New(jsBrowser::WinInterceptor));
        global->Set(String::NewSymbol("scroll"), FunctionTemplate::New(jsBrowser::WinInterceptor));
        global->Set(String::NewSymbol("scrollBy"), FunctionTemplate::New(jsBrowser::WinInterceptor));
        global->Set(String::NewSymbol("scrollTo"), FunctionTemplate::New(jsBrowser::WinInterceptor));
        global->Set(String::NewSymbol("setInterval"), FunctionTemplate::New(jsBrowser::WinInterceptor));
        global->Set(String::NewSymbol("setTimeout"), FunctionTemplate::New(jsBrowser::WinInterceptor));

    }
    // delete default context from jsExecutor
    context.Dispose();
    // init values
    window = new jsWindow(this, NULL, NULL);
    top_win = window;
    win_list[window->get_id()] = window;
    // make new context
    context = Context::New(NULL, global);
    {
        // make executor accessible in the JS
        LOG4CXX_TRACE(iLogger::GetLogger(), "jsBrowser: try to wrap object");
        Context::Scope context_scope(context);
        Handle<Object> _instance = wrap_object<jsExecutor>(this);
        context->Global()->Set(String::New("v8_context"), _instance);
    }
}

jsBrowser::~jsBrowser(void)
{
    windows_list::iterator it;

    for (it = win_list.begin(); it != win_list.end(); ++it) {
        delete it->second;
    }
}

jsWindow* jsBrowser::window_by_id(std::string id)
{
    jsWindow* retval = NULL;
    windows_list::iterator it = win_list.find(id);

    if (it != win_list.end()) {
        retval = it->second;
    }

    return retval;
}

jsWindow* jsBrowser::window_by_name(std::string name)
{
    jsWindow* retval = NULL;
    windows_list::iterator it;
    string nm;

    for (it = win_list.begin(); it != win_list.end(); ++it) {
        nm = value_to_string(it->second->props["name"]->ToString());
        if (nm == name) {
            retval = it->second;
            break;
        }
    }
    return retval;
}

Handle<Value> jsBrowser::GetWindow( Local<String> name, const AccessorInfo &info )
{
    HandleScope scope;

    Local<External> wrap = Local<External>::Cast(info.Data());
    void* ptr = wrap->Value();
    jsBrowser* el = static_cast<jsBrowser*>(ptr);

    Handle<Object> _obj = wrap_object<jsWindow>(el->window);

    return scope.Close(_obj);
}

Handle<Value> jsBrowser::WinInterceptor( const Arguments& args )
{
    Handle<Value> retval;
    Local<Context> ctx = v8::Context::GetCurrent();
    Local<Object> exec = Local<Object>::Cast(ctx->Global()->Get(String::New("v8_context")));
    Local<External> wrap = Local<External>::Cast(exec->GetInternalField(0));
    void* ptr = wrap->Value();
    jsBrowser* el = static_cast<jsBrowser*>(ptr); 

    string fname = value_to_string(args.Callee()->GetName()->ToString());

    Local<Object> _obj = Local<Object>::New(wrap_object<jsWindow>(el->window));
    Local<Function> _func = Local<Function>::Cast(_obj->Get(String::New(fname.c_str())));
    Handle<Value> *fargs = new Handle<Value>[args.Length()];
    for (int i = 0; i < args.Length(); ++i) {
        fargs[i] = args[i];
    }
    retval = _func->Call(_obj, args.Length(), fargs );

    return retval;
}