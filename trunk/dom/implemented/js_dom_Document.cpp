
/*
  $Id$
*/

#include "precomp.h"

using namespace v8;

js_dom_Document::js_dom_Document() {}

js_dom_Document::~js_dom_Document() {}

v8::Handle<v8::Value> js_dom_Document::createElement(std::string val_tagName)
{
    webEngine::html_entity_ptr ent(new webEngine::html_entity());
    ent->Name(val_tagName);
    v8_wrapper::tree_node_ptr node = v8_wrapper::wrap_entity(ent);
    js_html2_HTMLDocument *doc_ = dynamic_cast<js_html2_HTMLDocument*>(this);
    if(doc_) {
        v8_wrapper::dom_data<js_html2_HTMLDocument> &data = doc_->v8_wrapper::Registrator<js_html2_HTMLDocument>::m_data;
        data.m_execution_point->m_child_list.push_back(node);
    }
    return node->m_this;
}
v8::Handle<v8::Value> js_dom_Document::createDocumentFragment()
{
    return dom::Document::createDocumentFragment();
}
v8::Handle<v8::Value> js_dom_Document::createTextNode(std::string val_data)
{
    return dom::Document::createTextNode(val_data);
}
v8::Handle<v8::Value> js_dom_Document::createComment(std::string val_data)
{
    return dom::Document::createComment(val_data);
}
v8::Handle<v8::Value> js_dom_Document::createCDATASection(std::string val_data)
{
    return dom::Document::createCDATASection(val_data);
}
v8::Handle<v8::Value> js_dom_Document::createProcessingInstruction(std::string val_target, std::string val_data)
{
    return dom::Document::createProcessingInstruction(val_target, val_data);
}
v8::Handle<v8::Value> js_dom_Document::createAttribute(std::string val_name)
{
    return dom::Document::createAttribute(val_name);
}
v8::Handle<v8::Value> js_dom_Document::createEntityReference(std::string val_name)
{
    return dom::Document::createEntityReference(val_name);
}

v8::Handle<v8::Value> js_dom_Document::getElementsByTagName(std::string val_tagname)
{
    return v8_wrapper::getElementsByTagName<js_dom_NodeList>(this, val_tagname);
}

v8::Handle<v8::Value> js_dom_Document::importNode(v8::Handle<v8::Value> val_importedNode, bool val_deep)
{
    return dom::Document::importNode(val_importedNode, val_deep);
}
v8::Handle<v8::Value> js_dom_Document::createElementNS(std::string val_namespaceURI, std::string val_qualifiedName)
{
    return dom::Document::createElementNS(val_namespaceURI, val_qualifiedName);
}
v8::Handle<v8::Value> js_dom_Document::createAttributeNS(std::string val_namespaceURI, std::string val_qualifiedName)
{
    return dom::Document::createAttributeNS(val_namespaceURI, val_qualifiedName);
}
v8::Handle<v8::Value> js_dom_Document::getElementsByTagNameNS(std::string val_namespaceURI, std::string val_localName)
{
    return dom::Document::getElementsByTagNameNS(val_namespaceURI, val_localName);
}
v8::Handle<v8::Value> js_dom_Document::getElementById(std::string val_elementId)
{
    for(v8_wrapper::iterator_dfs it = this->begin_dfs(); it != this->end_dfs(); ++it) {
        boost::shared_ptr<js_html2_HTMLElement> el = boost::shared_dynamic_cast<js_html2_HTMLElement>(*it);
        if(el && el->id == val_elementId)
            return el->m_this;
    }
    return v8::Handle<v8::Value>();
}
