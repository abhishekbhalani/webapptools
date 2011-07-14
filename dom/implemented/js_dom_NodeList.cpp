
/*
  $Id: js_dom_NodeList.cpp 36341 2010-11-17 12:32:49Z santonov $
*/

#include "precomp.h"
using namespace v8;

js_dom_NodeList::js_dom_NodeList() {};

js_dom_NodeList::~js_dom_NodeList() {};

v8::Handle<v8::Value> js_dom_NodeList::item(long unsigned int val_index)
{
    if(val_index < length)
        return m_child_list[val_index]->m_this;
    return v8::Handle<v8::Value>();
}
