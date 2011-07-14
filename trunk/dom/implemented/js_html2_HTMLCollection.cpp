
/*
  $Id: js_html2_HTMLCollection.cpp 36341 2010-11-17 12:32:49Z santonov $
*/

#include "precomp.h"
using namespace v8;

js_html2_HTMLCollection::js_html2_HTMLCollection() {}

js_html2_HTMLCollection::~js_html2_HTMLCollection() {}

v8::Handle<v8::Value> js_html2_HTMLCollection::item(long unsigned int val_index)
{
    if(val_index < length)
        return m_child_list[val_index]->m_this;
    return v8::Handle<v8::Value>();
}

v8::Handle<v8::Value> js_html2_HTMLCollection::namedItem(html2::DOMString val_name)
{
    v8::Handle<v8::Value> result;
    for(v8_wrapper::tree_node_list::iterator it = m_child_list.begin();
            (it != m_child_list.end()) && result.IsEmpty(); ++it) {
        result = v8_wrapper::check_object_name( (*it).get(), val_name );
    }
    return result;
}
