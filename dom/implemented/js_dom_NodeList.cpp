
/*
  $Id: js_dom_NodeList.cpp 35984 2010-11-09 14:35:13Z santonov $
*/

#include <html_js.h>
using namespace v8;
v8::Handle<v8::Value> js_dom_NodeList::item(long unsigned int val_index)
{
    if(val_index < length)
        return m_child_list[val_index]->m_this;
    return v8::Handle<v8::Value>();
}
