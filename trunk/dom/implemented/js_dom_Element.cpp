
/*
  $Id: js_dom_Element.cpp 36276 2010-11-15 14:56:24Z santonov $
*/

#include <html_js.h>
using namespace v8;

std::string js_dom_Element::getAttribute(std::string val_name)
{
    return dom::Element::getAttribute(val_name);
}
void js_dom_Element::setAttribute(std::string val_name, std::string val_value)
{
    dom::Element::setAttribute(val_name, val_value);
}
void js_dom_Element::removeAttribute(std::string val_name)
{
    dom::Element::removeAttribute(val_name);
}
v8::Handle<v8::Value> js_dom_Element::getAttributeNode(std::string val_name)
{
    return dom::Element::getAttributeNode(val_name);
}
v8::Handle<v8::Value> js_dom_Element::setAttributeNode(v8::Handle<v8::Value> val_newAttr)
{
    return dom::Element::setAttributeNode(val_newAttr);
}
v8::Handle<v8::Value> js_dom_Element::removeAttributeNode(v8::Handle<v8::Value> val_oldAttr)
{
    return dom::Element::removeAttributeNode(val_oldAttr);
}
v8::Handle<v8::Value> js_dom_Element::getElementsByTagName(std::string val_name)
{
    return v8_wrapper::getElementsByTagName<js_dom_NodeList>(this, val_name);
}
std::string js_dom_Element::getAttributeNS(std::string val_namespaceURI, std::string val_localName)
{
    return dom::Element::getAttributeNS(val_namespaceURI, val_localName);
}
void js_dom_Element::setAttributeNS(std::string val_namespaceURI, std::string val_qualifiedName, std::string val_value)
{
    dom::Element::setAttributeNS(val_namespaceURI, val_qualifiedName, val_value);
}
void js_dom_Element::removeAttributeNS(std::string val_namespaceURI, std::string val_localName)
{
    dom::Element::removeAttributeNS(val_namespaceURI, val_localName);
}
v8::Handle<v8::Value> js_dom_Element::getAttributeNodeNS(std::string val_namespaceURI, std::string val_localName)
{
    return dom::Element::getAttributeNodeNS(val_namespaceURI, val_localName);
}
v8::Handle<v8::Value> js_dom_Element::setAttributeNodeNS(v8::Handle<v8::Value> val_newAttr)
{
    return dom::Element::setAttributeNodeNS(val_newAttr);
}
v8::Handle<v8::Value> js_dom_Element::getElementsByTagNameNS(std::string val_namespaceURI, std::string val_localName)
{
    return dom::Element::getElementsByTagNameNS(val_namespaceURI, val_localName);
}
bool js_dom_Element::hasAttribute(std::string val_name)
{
    return dom::Element::hasAttribute(val_name);
}
bool js_dom_Element::hasAttributeNS(std::string val_namespaceURI, std::string val_localName)
{
    return dom::Element::hasAttributeNS(val_namespaceURI, val_localName);
}

js_dom_Element::js_dom_Element() {}

js_dom_Element::~js_dom_Element() {}
