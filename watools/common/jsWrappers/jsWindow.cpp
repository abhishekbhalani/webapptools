#include <string>
#include <vector>
#include <map>
#include <weLogger.h>
#include "jsGlobal.h"
#include "jsBrowser.h"
#include "jsWindow.h"
// from common/
#include <boost/lexical_cast.hpp>
#include "boost/uuid.hpp"
#include "boost/uuid_generators.hpp"
#include "boost/uuid_io.hpp"

using namespace v8;
using namespace webEngine;
using namespace boost::uuids;

static char* read_only_props[] = {
    "closed",
    "document",
    "frames",
    "history",
    "length",
    "location",
    "navigator",
    "opener",
    "pageXOffset",
    "pageYOffset",
    "parent",
    "screen",
    "screenLeft",
    "screenTop",
    "screenX",
    "screenY",
    "self",
    "top"
};
static char* func_list[] = {
    "toString",
    "alert",
    "blur",
    "clearInterval",
    "clearTimeout",
    "close",
    "confirm",
    "createPopup",
    "focus",
    "moveBy",
    "moveTo",
    "open",
    "print",
    "prompt",
    "resizeBy",
    "resizeTo",
    "scroll",
    "scrollBy",
    "scrollTo",
    "setInterval",
    "setTimeout"
};

Persistent<FunctionTemplate> jsHistory::object_template;
bool jsHistory::is_init = false;

Persistent<FunctionTemplate> jsWindow::object_template;
bool jsWindow::is_init = false;
std::vector<std::string> jsWindow::ro_props(read_only_props, read_only_props + sizeof read_only_props / sizeof read_only_props[ 0 ]);
std::vector<std::string> jsWindow::funcs(func_list, func_list + sizeof func_list / sizeof func_list[ 0 ]);

jsHistory::jsHistory(jsWindow *win)
{
    if (!is_init) {
        is_init = true;
        Handle<FunctionTemplate> _object = FunctionTemplate::New();
        //get the location's instance template
        Handle<ObjectTemplate> _proto = _object->InstanceTemplate();
        //set its internal field count to one (we'll put references to the C++ point here later)
        _proto->SetInternalFieldCount(1);

        // Add accessors.
        Handle<Value> self = External::New(this);
        _proto->SetAccessor(String::New("length"), jsHistory::GetLength);
        _proto->Set(String::New("forward"), FunctionTemplate::New(jsHistory::Forward));
        _proto->Set(String::New("back"), FunctionTemplate::New(jsHistory::Back));
        _proto->Set(String::New("go"), FunctionTemplate::New(jsHistory::Go));
        object_template = Persistent<FunctionTemplate>::New(_object);
    }
    window = win;
    index = -1;
}

void jsHistory::push_back(const std::string& url)
{
    history.push_back(url);
    index = (int)history.size() - 1;
}

std::string jsHistory::forward()
{
    string retval = "";

    if (index < ((int)history.size() - 1)) {
        index++;
        retval = history[index];
    }
    return retval;
}

std::string jsHistory::back()
{
    string retval = "";

    if (index > 0) {
        index--;
        retval = history[index];
    }
    return retval;
}

std::string jsHistory::go(v8::Handle<v8::Value> pos)
{
    string retval = "";

    if (pos->IsInt32()) {
        // process offset
        int offs = pos->Int32Value();
        offs += index;
        if (offs > 0 && offs < (int)history.size()) {
            index = offs;
            retval = history[index];
        }
    }
    else if (pos->IsString()) {
        // search for substring
        string srch = value_to_string(Local<Value>::New(pos));
        for( size_t i = 0 ; i < history.size(); ++i) {
            if (history[i].find(srch) != string::npos) {
                index = i;
                retval = history[index];
                break;
            }
        }
    }

    return retval;
}

Handle<Value> jsHistory::GetLength( Local<String> name, const AccessorInfo &info )
{
    HandleScope scope;
    Handle<Value> retval;

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsHistory* el = static_cast<jsHistory*>(ptr);
    retval = Int32::New(el->history.size());

    return scope.Close(retval);
}

Handle<Value> jsHistory::Forward(const Arguments& args)
{
    HandleScope scope;
    Handle<Value> retval;

    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsHistory* el = static_cast<jsHistory*>(ptr);

    std::string ret = el->forward();
    retval = String::New(ret.c_str());

    return scope.Close(retval);
}

Handle<Value> jsHistory::Back(const Arguments& args)
{
    HandleScope scope;
    Handle<Value> retval;

    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsHistory* el = static_cast<jsHistory*>(ptr);

    std::string ret = el->back();
    retval = String::New(ret.c_str());

    return scope.Close(retval);
}

Handle<Value> jsHistory::Go(const Arguments& args)
{
    HandleScope scope;
    Handle<Value> retval;

    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsHistory* el = static_cast<jsHistory*>(ptr);

    std::string ret = el->go(args[0]);
    retval = String::New(ret.c_str());

    return scope.Close(retval);
}

Handle<Value> Window(const v8::Arguments& args)
{
    // throw if called without `new'
    if (!args.IsConstructCall()) 
        return ThrowException(String::New("Cannot call constructor as function"));

    HandleScope scope;

    Handle<Value> retval;
    Local<Context> ctx = v8::Context::GetCurrent();
    Local<Value> exec = ctx->Global()->Get(String::New("v8_context"));
    LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::Window: gets v8_context");
    if (exec->IsObject())
    {
        Local<Object> eObj = Local<Object>::Cast(exec);
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(eObj->GetInternalField(0));
        jsBrowser* jsExec = static_cast<jsBrowser*>(wrap->Value());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::Window: gets jsBrowser");
        jsWindow *p = new jsWindow(jsExec, jsExec->window, jsExec->window);
        jsExec->register_window(p);
        retval = wrap_object<jsWindow>(p);
    }

    return scope.Close(retval);
}

jsWindow::jsWindow( jsBrowser* holder_, jsWindow* parent_, jsWindow* creator_ )
{
    if (!is_init) {
        is_init = true;
        Handle<FunctionTemplate> _object = FunctionTemplate::New();
        //get the location's instance template
        Handle<ObjectTemplate> _proto = _object->InstanceTemplate();
        //set its internal field count to one (we'll put references to the C++ point here later)
        _proto->SetInternalFieldCount(1);

        // Add accessors.
        Handle<Value> self = External::New(this);
        _proto->Set(String::NewSymbol("toString"), FunctionTemplate::New(jsWindow::toString));
        _proto->Set(String::NewSymbol("alert"), FunctionTemplate::New(alert));
        _proto->Set(String::NewSymbol("blur"), FunctionTemplate::New(jsWindow::PlaceHolder));
        _proto->Set(String::NewSymbol("clearInterval"), FunctionTemplate::New(jsWindow::PlaceHolder));
        _proto->Set(String::NewSymbol("clearTimeout"), FunctionTemplate::New(jsWindow::PlaceHolder));
        _proto->Set(String::NewSymbol("close"), FunctionTemplate::New(jsWindow::Close));
        _proto->Set(String::NewSymbol("confirm"), FunctionTemplate::New(jsWindow::PlaceHolder));
        _proto->Set(String::NewSymbol("createPopup"), FunctionTemplate::New(jsWindow::Popup));
        _proto->Set(String::NewSymbol("focus"), FunctionTemplate::New(jsWindow::Focus));
        _proto->Set(String::NewSymbol("moveBy"), FunctionTemplate::New(jsWindow::PlaceHolder));
        _proto->Set(String::NewSymbol("moveTo"), FunctionTemplate::New(jsWindow::PlaceHolder));
        _proto->Set(String::NewSymbol("open"), FunctionTemplate::New(jsWindow::Open));
        _proto->Set(String::NewSymbol("print"), FunctionTemplate::New(jsWindow::PlaceHolder));
        _proto->Set(String::NewSymbol("prompt"), FunctionTemplate::New(jsWindow::PlaceHolder));
        _proto->Set(String::NewSymbol("resizeBy"), FunctionTemplate::New(jsWindow::PlaceHolder));
        _proto->Set(String::NewSymbol("resizeTo"), FunctionTemplate::New(jsWindow::PlaceHolder));
        _proto->Set(String::NewSymbol("scroll"), FunctionTemplate::New(jsWindow::PlaceHolder));
        _proto->Set(String::NewSymbol("scrollBy"), FunctionTemplate::New(jsWindow::PlaceHolder));
        _proto->Set(String::NewSymbol("scrollTo"), FunctionTemplate::New(jsWindow::PlaceHolder));
        _proto->Set(String::NewSymbol("setInterval"), FunctionTemplate::New(jsWindow::PlaceHolder));
        _proto->Set(String::NewSymbol("setTimeout"), FunctionTemplate::New(jsWindow::PlaceHolder));
        _proto->SetNamedPropertyHandler(jsWindow::WindowGet, jsWindow::WindowSet, NULL, NULL, jsWindow::WindowEnum);

        object_template = Persistent<FunctionTemplate>::New(_object);
    }
    basic_random_generator<boost::mt19937> gen;
    uuid tag = gen();
    win_uuid = boost::lexical_cast<string>(tag);

    browser = holder_;
    parent = parent_;
    creator = creator_;
    location.url.protocol = "about";
    location.url.host = "blank";
    document = new jsDocument(this);
    history = new jsHistory(this);

    // properties
    props["closed"] = Persistent<Value>::New(Boolean::New(false));
    props["defaultStatus"] = Persistent<Value>::New(String::New("Done"));
    props["innerHeight"] = Persistent<Value>::New(Int32::New(600));
    props["innerWidth"] = Persistent<Value>::New(Int32::New(800));
    props["name"] = Persistent<Value>::New(String::New(""));
    props["outerHeight"] = Persistent<Value>::New(Int32::New(600));
    props["outerWidth"] = Persistent<Value>::New(Int32::New(800));
    props["pageXOffset"] = Persistent<Value>::New(Int32::New(0));
    props["pageYOffset"] = Persistent<Value>::New(Int32::New(0));
    props["screenLeft"] = Persistent<Value>::New(Int32::New(1));
    props["screenTop"] = Persistent<Value>::New(Int32::New(1));
    props["screenX"] = Persistent<Value>::New(Int32::New(1));
    props["screenY"] = Persistent<Value>::New(Int32::New(1));
    props["status"] = Persistent<Value>::New(String::New(""));
    LOG4CXX_TRACE(iLogger::GetLogger(), "jsWindow new window created - uuid=" << win_uuid);
}

jsWindow::~jsWindow( void )
{
    delete document;
    delete history;
    LOG4CXX_TRACE(iLogger::GetLogger(), "jsWindow window destroyed - uuid=" << win_uuid);
}

const bool jsWindow::is_closed()
{
    bool ret = false;
    if (props["closed"]->BooleanValue()) {
        LOG4CXX_WARN(iLogger::GetLogger(), "jsWindow access to closed window - uuid=" << win_uuid);
        ret = true;
    }
    return ret;
}

const bool jsWindow::is_property(string key)
{
    bool retval;
    std::map<std::string, Persistent<Value>>::iterator iter = props.find(key);
    retval = (iter != props.end()); 
    if (!retval) {
        vector<string>::iterator itro = find(ro_props.begin(), ro_props.end(), key);
        retval = (itro != ro_props.end());
        if (! retval) {
            vector<string>::iterator itfn = find(funcs.begin(), funcs.end(), key);
            retval = (itfn != funcs.end());
        }
    }
    return retval;
}

Handle<Value> jsWindow::GetProperty(Local<String> name, const AccessorInfo &info)
{
	Handle<Value> val;
	std::string key = value_to_string(name);

	if (key == "navigator") {
		val = wrap_object<jsNavigator>(&browser->navigator);
	}
	else if (key == "screen") {
		val = wrap_object<jsScreen>(&browser->screen);
	}
	else if (key == "history") {
		val = wrap_object<jsHistory>(history);
	}
	else if (key == "location") {
		val = wrap_object<jsLocation>(&location);
	}
	else if (key == "document") {
		val = wrap_object<jsDocument>(document);
	}
	else if (key == "opener") {
        if (creator == NULL) {
            val = wrap_object<jsWindow>(this);
        }
        else {
            val = wrap_object<jsWindow>(creator);
        }
	}
	else if (key == "self") {
		val = wrap_object<jsWindow>(this);
	}
	else if (key == "top") {
		val = wrap_object<jsWindow>(browser->window);
	}
	else if (key == "parent") {
        if (parent == NULL) {
            val = wrap_object<jsWindow>(this);
        }
        else {
            val = wrap_object<jsWindow>(parent);
        }
	}
    else {
        // Look up the value if it exists using the standard STL idiom.
        jsPropertyMap::iterator iter = props.find(key);
        // If the key is not present return an empty handle as signal.
        if (iter != props.end()) {
            // Otherwise fetch the value and wrap it in a JavaScript string.
            val = Local<Value>::New(iter->second);
        }
    }
    return val;
}

Handle<Value> jsWindow::SetProperty(Local<String> name, Local<Value> value, const AccessorInfo& info)
{
    Handle<Value> val;
    std::string key = value_to_string(name);
    vector<string>::iterator it = find(ro_props.begin(), ro_props.end(), key);

    if (it == ro_props.end()) {
        props[key] = Persistent<Value>::New(value);
        val = value;
    }
    else if (key == "location" && value->IsString()) {
        std::string uri = value_to_string(value);
        if (location.url.tostring() == uri) {
            // reload
        }
        else if (uri == "about:blank") {
            // assign
            location.url.protocol = "about";
            location.url.host = "blank";
        }
        else {
            location.url.assign_with_referer(uri);
            history->push_back(uri);
            // todo - reload document
        }
        // todo - log the location changes
    }
	return val;
}

Handle<Value> jsWindow::WindowGet(Local<String> name, const AccessorInfo &info)
{
    HandleScope scope;
    string key = value_to_string(name);

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsWindow* el = static_cast<jsWindow*>(ptr);
    Handle<Value> val = el->GetProperty(name, info);
    return scope.Close(val);
}

Handle<Value> jsWindow::WindowSet(Local<String> name, Local<Value> value, const AccessorInfo& info)
{
    string key = value_to_string(name);
    Handle<Value> retval;

    vector<string>::iterator it = find(jsWindow::funcs.begin(), jsWindow::funcs.end(), key);
    if (it == jsWindow::funcs.end()) {
        Local<Object> self = info.This();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        jsWindow* el = static_cast<jsWindow*>(ptr); 
        retval = el->SetProperty(name, value, info);
    }
    return retval;
}


Handle<Array> jsWindow::WindowEnum( const AccessorInfo &info )
{
    HandleScope scope;

    Handle<Array> retval = Local<Array>::New(Array::New());

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsWindow* el = static_cast<jsWindow*>(ptr); 

    map<string, int> prop_list;
    size_t i;
    for (i = 0; i < ro_props.size(); ++i) {
        prop_list[ro_props[i]] = 1;
    }
    jsPropertyMap::iterator it;
    for (it = el->props.begin(); it != el->props.end(); ++it) {
        prop_list[it->first] = 1;
    }
    map<string, int>::iterator ins;
    i = 0;
    for (ins = prop_list.begin(); ins != prop_list.end(); ++ins) {
        string val = ins->first;
        retval->Set(Number::New(i), String::New(val.c_str()));
        i++;
    }

    return scope.Close(retval);
}

Handle<Value> jsWindow::toString(const Arguments& args)
{
    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsWindow* el = static_cast<jsWindow*>(ptr); 
    
    string ret = "[Window; uuid=";
    ret += el->win_uuid;
    ret += "; name=";
    ret += value_to_string(el->props["name"]->ToString());
    ret += "]";
    return String::New(ret.c_str());
}

Handle<Value> jsWindow::PlaceHolder(const Arguments& args)
{
    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsWindow* el = static_cast<jsWindow*>(ptr); 
    
    string ret;
    ret = value_to_string(args.Callee()->ToString());
    ret += " [Window; uuid=";
    ret += el->win_uuid;
    ret += "; name=";
    ret += value_to_string(el->props["name"]->ToString());
    ret += "]";
    LOG4CXX_DEBUG(iLogger::GetLogger(), "jsWindow::PlacHolder - " << ret);
    return String::New(ret.c_str());
}

Handle<Value> jsWindow::Close(const Arguments& args)
{
    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsWindow* el = static_cast<jsWindow*>(ptr); 
    
    LOG4CXX_TRACE(iLogger::GetLogger(), "jsWindow::Close - uuid=" << el->win_uuid);
    el->props["closed"] = Persistent<Value>::New(Boolean::New(false));
    return Handle<Value>();
}

Handle<Value> jsWindow::Focus(const Arguments& args)
{
    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsWindow* el = static_cast<jsWindow*>(ptr); 
    
    if (!el->is_closed()) {
        el->browser->window = el;
    }
    return Handle<Value>();
}

Handle<Value> jsWindow::Open(const Arguments& args)
{
    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsWindow* el = static_cast<jsWindow*>(ptr); 
    
    Handle<Value> val;
    if (!el->is_closed()) {
        transport_url url;
        string nm = "";
        bool is_replace = false;
        jsWindow* new_win = NULL;
        if (args.Length() > 0) {
            url.assign_with_referer(value_to_string(args[0]), &el->location.url);
        }
        else {
            url.protocol = "about";
            url.host = "blank";
        }
        if (args.Length() > 1) {
            nm = value_to_string(args[1]);
        }
        // features (args[2]) argument not yet implemented
        if (args.Length() > 3) {
            is_replace = args[3]->BooleanValue();
        }
        if (nm != "") {
            new_win = el->browser->window_by_name(nm);
        }
        if (new_win == NULL) {
            new_win = new jsWindow(el->browser, el->browser->window, el);
            new_win->props["name"] = Persistent<Value>::New(String::New(nm.c_str()));
        }
        new_win->location.url = url;
        el->browser->register_window(new_win);
        val = wrap_object<jsWindow>(new_win); 
        LOG4CXX_TRACE(iLogger::GetLogger(), "jsWindow::Open new window - uuid=" << new_win->win_uuid
            << "; url=" << url.tostring());
        // todo load data
        string msg = "Open new Window\r\n\thref: ";
        msg += url.tostring();
        append_results(msg);
    }
    return val;
}

Handle<Value> jsWindow::Popup( const Arguments& args )
{
    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsWindow* el = static_cast<jsWindow*>(ptr); 

    Handle<Value> val;
    if (!el->is_closed()) {
        jsWindow* new_win = new jsWindow(el->browser, el, el);
        el->browser->register_window(new_win);
        val = wrap_object<jsWindow>(new_win); 
        LOG4CXX_TRACE(iLogger::GetLogger(), "jsWindow::Popup new window - uuid=" << new_win->win_uuid);
    }
    return val;
}
