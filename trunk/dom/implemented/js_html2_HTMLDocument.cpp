
/*
  $Id: js_html2_HTMLDocument.cpp 36450 2010-11-19 14:32:26Z santonov $
*/

#include "precomp.h"
using namespace v8;

js_html2_HTMLDocument::js_html2_HTMLDocument() {}

js_html2_HTMLDocument::~js_html2_HTMLDocument() {}

void js_html2_HTMLDocument::open()
{
    v8_wrapper::dom_data<js_html2_HTMLDocument> &data = v8_wrapper::Registrator<js_html2_HTMLDocument>::m_data;
    data.m_strbuf.str("");
    data.m_opened = true;
}

void js_html2_HTMLDocument::close()
{
    v8_wrapper::dom_data<js_html2_HTMLDocument> &data = v8_wrapper::Registrator<js_html2_HTMLDocument>::m_data;
    data.m_opened = false;
    write(data.m_strbuf.str());
    data.m_strbuf.str("");
}

void js_html2_HTMLDocument::write(html2::DOMString val_text)
{
    v8_wrapper::dom_data<js_html2_HTMLDocument> &data = v8_wrapper::Registrator<js_html2_HTMLDocument>::m_data;
    if(data.m_opened)
        data.m_strbuf << val_text;
    else {
        webEngine::html_entity_ptr wrapped(new webEngine::html_entity());
        wrapped->Parse("", webEngine::tag_scanner(webEngine::str_tag_stream(val_text.c_str())));
        wrap_dom( wrapped, data.m_execution_point );
        v8_wrapper::update_document(*this);
    }
}

void js_html2_HTMLDocument::writeln(html2::DOMString val_text)
{
    v8_wrapper::dom_data<js_html2_HTMLDocument> &data = v8_wrapper::Registrator<js_html2_HTMLDocument>::m_data;
    if(data.m_opened)
        data.m_strbuf << val_text << std::endl;
    else {
        val_text.append("\n");
        write(val_text);
    }
}

v8::Handle<v8::Value> js_html2_HTMLDocument::getElementsByName(html2::DOMString val_elementName)
{
    return html2::HTMLDocument::getElementsByName(val_elementName);
}
