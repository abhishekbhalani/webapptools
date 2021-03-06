
/*
  DO NOT EDIT!
  This file has been generated by generate_sources.py script.
  $Id$
*/

#include "precomp.h"
using namespace v8;

v8::Handle<v8::Value> js_dom_NamedNodeMap::getNamedItem(std::string val_name)
{
    return dom::NamedNodeMap::getNamedItem(val_name);
}
v8::Handle<v8::Value> js_dom_NamedNodeMap::setNamedItem(v8::Handle<v8::Value> val_arg)
{
    return dom::NamedNodeMap::setNamedItem(val_arg);
}
v8::Handle<v8::Value> js_dom_NamedNodeMap::removeNamedItem(std::string val_name)
{
    return dom::NamedNodeMap::removeNamedItem(val_name);
}
v8::Handle<v8::Value> js_dom_NamedNodeMap::item(long unsigned int val_index)
{
    return dom::NamedNodeMap::item(val_index);
}
v8::Handle<v8::Value> js_dom_NamedNodeMap::getNamedItemNS(std::string val_namespaceURI, std::string val_localName)
{
    return dom::NamedNodeMap::getNamedItemNS(val_namespaceURI, val_localName);
}
v8::Handle<v8::Value> js_dom_NamedNodeMap::setNamedItemNS(v8::Handle<v8::Value> val_arg)
{
    return dom::NamedNodeMap::setNamedItemNS(val_arg);
}
v8::Handle<v8::Value> js_dom_NamedNodeMap::removeNamedItemNS(std::string val_namespaceURI, std::string val_localName)
{
    return dom::NamedNodeMap::removeNamedItemNS(val_namespaceURI, val_localName);
}

js_dom_NamedNodeMap::js_dom_NamedNodeMap() {}

js_dom_NamedNodeMap::~js_dom_NamedNodeMap() {}
