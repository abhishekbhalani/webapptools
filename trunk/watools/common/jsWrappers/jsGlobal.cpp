#include <weLogger.h>
#include "jsGlobal.h"
#include "jsExecutor.h"
#include "jsBrowser.h"
#include <html_js.h>

#include <vector>
#include <utility>

#include <html_tags_wrapper.h>

using namespace v8;
using namespace std;
using namespace webEngine;

// Functions
// Extracts a C string from a V8 Utf8Value.
string value_to_string(const Local<Value>& val)
{

    if (*val) {
        String::Utf8Value utf8(val);
        return string(*utf8);
    }
    return string("<string conversion failed>");
}

Handle<Value> alert(const Arguments& args)
{
    bool first = true;
    string res = "";
    for (int i = 0; i < args.Length(); i++) {
        HandleScope handle_scope;
        if (first) {
            first = false;
        } else {
            res += " ";
        }
        res += value_to_string(args[i]);
    }
    LOG4CXX_INFO(webEngine::iLogger::GetLogger(), "js::alert: " << res);
    return Undefined();
}

Handle<Value> print(const Arguments& args)
{
    bool first = true;
    string res = "";
    for (int i = 0; i < args.Length(); i++) {
        HandleScope handle_scope;
        if (first) {
            first = false;
        } else {
            res += " ";
        }
        res += value_to_string(args[i]);
    }
    append_results(res);
    LOG4CXX_INFO(webEngine::iLogger::GetLogger(), res);
    return Undefined();
}

Handle<Value> version(const Arguments& args)
{
    return String::New(V8::GetVersion());
}

void append_results(const std::string& data)
{
    {
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::append_results: data("<< data <<")");
    }
    HandleScope scope;

    // extract executor
    Local<Context> ctx = v8::Context::GetCurrent();
    Local<Value> exec = ctx->Global()->Get(String::New("v8_context"));
//     LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::append_results: gets v8_context");
    if (exec->IsObject()) {
        Local<Object> eObj = Local<Object>::Cast(exec);
//         LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::append_results: gets v8_context object");
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(eObj->GetInternalField(0));
//         LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::append_results: gets v8_context wrapper");
        webEngine::jsExecutor* jsExec = static_cast<webEngine::jsExecutor*>(wrap->Value());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::append_results: gets jsExecutor");
        jsExec->append_results(data);
    }
}

void append_object(v8::Handle<v8::Object> data)
{
    {
        String::AsciiValue obj(data->ToString());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::append_object: data("<< *obj <<")");
    }
    HandleScope scope;

    // extract executor
    Local<Context> ctx = v8::Context::GetCurrent();
    Local<Value> exec = ctx->Global()->Get(String::New("v8_context"));
//     LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::append_object: gets v8_context");
    if (exec->IsObject()) {
        Local<Object> eObj = Local<Object>::Cast(exec);
//         LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::append_object: gets v8_context object");
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(eObj->GetInternalField(0));
//         LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::append_object: gets v8_context wrapper");
        webEngine::jsExecutor* jsExec = static_cast<webEngine::jsExecutor*>(wrap->Value());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::append_object: gets jsExecutor");
        jsExec->append_object(data);
    }
}

Handle<Value> dump_object(const Arguments& args)
{
    Handle<Value> res = Undefined();
    string  nm;
    string  id;
    int     dp;

    Local<Context> ctx = v8::Context::GetCurrent();
    Local<Value> exec = ctx->Global()->Get(String::New("v8_context"));
//     LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::DumpObject: gets v8_context");
    if (exec->IsObject()) {
        Local<Object> eObj = Local<Object>::Cast(exec);
//         LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::DumpObject: gets v8_context object");
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(eObj->GetInternalField(0));
//         LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::DumpObject: gets v8_context wrapper");
        webEngine::jsExecutor* jsExec = static_cast<webEngine::jsExecutor*>(wrap->Value());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::DumpObject: gets jsExecutor");
        if (args.Length() > 1) {
            nm = value_to_string(*args[1]);
        } else {
            nm = "";
        }
        if (args.Length() > 2) {
            id = value_to_string(*args[2]);
        } else {
            id = "";
        }
        if (args.Length() > 3) {
            dp = args[3]->Int32Value();
        } else {
            dp = 0;
        }
        string dat = jsExec->obj_dump(args[0], nm, id, dp, ctx);
        res = String::New(dat.c_str());
    }

    return res;
}

boost::shared_ptr<v8_wrapper::TreeNode> wrap_entity( html_entity_ptr objToWrap )
{
    boost::shared_ptr<v8_wrapper::TreeNode> node;
    HTML_TAG html_tag = objToWrap->HtmlTag();

    switch(html_tag) {
    case  HTML_TAG_a:
        node = TreeNodeFromEntity<js_html2_HTMLAnchorElement, true>(objToWrap);
        break;
    case  HTML_TAG_abbr :
    case  HTML_TAG_acronym :
    case  HTML_TAG_address :
    case  HTML_TAG_applet :
    case  HTML_TAG_area :
    case  HTML_TAG_b :
    case  HTML_TAG_base :
    case  HTML_TAG_basefont :
    case  HTML_TAG_bdo :
    case  HTML_TAG_big :
    case  HTML_TAG_blockquote :
        break;
    case  HTML_TAG_body :
        node = TreeNodeFromEntity<js_html2_HTMLBodyElement, true>(objToWrap);
        break;
    case  HTML_TAG_br :
        node = TreeNodeFromEntity<js_html2_HTMLBRElement, true>(objToWrap);
        break;
    case  HTML_TAG_button :
        node = TreeNodeFromEntity<js_html2_HTMLButtonElement, true>(objToWrap);
        break;
    case  HTML_TAG_caption :
        node = TreeNodeFromEntity<js_html2_HTMLTableCaptionElement, true>(objToWrap);
        break;
    case  HTML_TAG_center :
    case  HTML_TAG_cite :
    case  HTML_TAG_code :
    case  HTML_TAG_col :
    case  HTML_TAG_colgroup :
    case  HTML_TAG_dd :
    case  HTML_TAG_del :
    case  HTML_TAG_dfn :
    case  HTML_TAG_dir :
        break;
    case  HTML_TAG_div :
        node = TreeNodeFromEntity<js_html2_HTMLDivElement, true>(objToWrap);
        break;
    case  HTML_TAG_dl :
    case  HTML_TAG_dt :
    case  HTML_TAG_em :
    case  HTML_TAG_fieldset :
    case  HTML_TAG_font :
        break;
    case  HTML_TAG_form:
        node = TreeNodeFromEntity<js_html2_HTMLFormElement, true>(objToWrap);
        break;
    case  HTML_TAG_frame :
    case  HTML_TAG_frameset :
        break;
    case  HTML_TAG_h1 :
    case  HTML_TAG_h2 :
    case  HTML_TAG_h3 :
    case  HTML_TAG_h4 :
    case  HTML_TAG_h5 :
    case  HTML_TAG_h6 :
        node = TreeNodeFromEntity<js_html2_HTMLHeadingElement, true>(objToWrap);
        break;
    case  HTML_TAG_head :
        node = TreeNodeFromEntity<js_html2_HTMLHeadElement, true>(objToWrap);
        break;
    case  HTML_TAG_hr :
        node = TreeNodeFromEntity<js_html2_HTMLHRElement, true>(objToWrap);
        break;
    case  HTML_TAG_html :
        node = TreeNodeFromEntity<js_html2_HTMLHtmlElement, true>(objToWrap);
        break;
    case  HTML_TAG_i :
        break;
    case  HTML_TAG_iframe :
        node = TreeNodeFromEntity<js_html2_HTMLIFrameElement, true>(objToWrap);
        break;
    case  HTML_TAG_img :
        node = TreeNodeFromEntity<js_html2_HTMLImageElement, true>(objToWrap);
        break;
    case  HTML_TAG_input :
        node = TreeNodeFromEntity<js_html2_HTMLInputElement, true>(objToWrap);
        break;
    case  HTML_TAG_ins :
    case  HTML_TAG_isindex :
    case  HTML_TAG_kbd :
        break;
    case  HTML_TAG_label :
        node = TreeNodeFromEntity<js_html2_HTMLLabelElement, true>(objToWrap);
        break;
    case  HTML_TAG_legend :
        node = TreeNodeFromEntity<js_html2_HTMLLegendElement, true>(objToWrap);
        break;
    case  HTML_TAG_li :
        node = TreeNodeFromEntity<js_html2_HTMLLIElement, true>(objToWrap);
        break;
    case  HTML_TAG_link :
        node = TreeNodeFromEntity<js_html2_HTMLLinkElement, true>(objToWrap);
        break;
    case  HTML_TAG_map :
        node = TreeNodeFromEntity<js_html2_HTMLMapElement, true>(objToWrap);
        break;
    case  HTML_TAG_menu :
        node = TreeNodeFromEntity<js_html2_HTMLMenuElement, true>(objToWrap);
        break;
    case  HTML_TAG_meta :
        node = TreeNodeFromEntity<js_html2_HTMLMetaElement, true>(objToWrap);
        break;
    case  HTML_TAG_noframes :
    case  HTML_TAG_noscript :
        break;
    case  HTML_TAG_object :
        node = TreeNodeFromEntity<js_html2_HTMLObjectElement, true>(objToWrap);
        break;
    case  HTML_TAG_ol :
        node = TreeNodeFromEntity<js_html2_HTMLOListElement, true>(objToWrap);
        break;
    case  HTML_TAG_optgroup :
    case  HTML_TAG_option :
    case  HTML_TAG_p :
    case  HTML_TAG_param :
    case  HTML_TAG_pre :
    case  HTML_TAG_q :
    case  HTML_TAG_s :
    case  HTML_TAG_samp :
        break;
    case  HTML_TAG_script :
        node = TreeNodeFromEntity<js_html2_HTMLScriptElement, true>(objToWrap);
        break;
    case  HTML_TAG_select :
        node = TreeNodeFromEntity<js_html2_HTMLSelectElement, true>(objToWrap);
        break;
    case  HTML_TAG_small :
    case  HTML_TAG_span :
    case  HTML_TAG_strike :
    case  HTML_TAG_strong :
    case  HTML_TAG_style :
    case  HTML_TAG_sub :
    case  HTML_TAG_sup :
        break;
    case  HTML_TAG_table :
        node = TreeNodeFromEntity<js_html2_HTMLTableElement, true>(objToWrap);
        break;
    case  HTML_TAG_tbody :
    case  HTML_TAG_td :
    case  HTML_TAG_textarea :
    case  HTML_TAG_tfoot :
    case  HTML_TAG_th :
    case  HTML_TAG_thead :
        break;
    case  HTML_TAG_title :
        node = TreeNodeFromEntity<js_html2_HTMLTitleElement, true>(objToWrap);
        break;
    case  HTML_TAG_tr :
    case  HTML_TAG_tt :
    case  HTML_TAG_u :
    case  HTML_TAG_ul :
    case  HTML_TAG_var :
    default:
        node = TreeNodeFromEntity<js_html2_HTMLElement, true>(objToWrap);
        break;
    }

    if(!node)
        node = TreeNodeFromEntity<js_html2_HTMLElement, true>(objToWrap);

    node->m_entity = objToWrap;
    return node;
}

