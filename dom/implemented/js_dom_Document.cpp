
/*
  $Id: js_dom_Document.cpp 35984 2010-11-09 14:35:13Z santonov $
*/

#include <html_js.h>
using namespace v8;
v8::Handle<v8::Value> js_dom_Document::createElement(std::string val_tagName)
{
    return dom::Document::createElement(val_tagName);
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
