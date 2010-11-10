
/*
  $Id: js_dom_Node.cpp 35984 2010-11-09 14:35:13Z santonov $
*/

#include <html_js.h>
using namespace v8;
v8::Handle<v8::Value> js_dom_Node::insertBefore(v8::Handle<v8::Value> val_newChild, v8::Handle<v8::Value> val_refChild)
{
    return dom::Node::insertBefore(val_newChild, val_refChild);
}
v8::Handle<v8::Value> js_dom_Node::replaceChild(v8::Handle<v8::Value> val_newChild, v8::Handle<v8::Value> val_oldChild)
{
    return dom::Node::replaceChild(val_newChild, val_oldChild);
}
v8::Handle<v8::Value> js_dom_Node::removeChild(v8::Handle<v8::Value> val_oldChild)
{
    return dom::Node::removeChild(val_oldChild);
}
v8::Handle<v8::Value> js_dom_Node::appendChild(v8::Handle<v8::Value> val_newChild)
{
    return dom::Node::appendChild(val_newChild);
}
bool js_dom_Node::hasChildNodes()
{
    return !m_child_list.empty();
}
v8::Handle<v8::Value> js_dom_Node::cloneNode(bool val_deep)
{
    return dom::Node::cloneNode(val_deep);
}
void js_dom_Node::normalize()
{
    dom::Node::normalize();
}
bool js_dom_Node::isSupported(std::string val_feature, std::string val_version)
{
    return dom::Node::isSupported(val_feature, val_version);
}
bool js_dom_Node::hasAttributes()
{
    return dom::Node::hasAttributes();
}
