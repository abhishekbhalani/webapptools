
/*
  $Id: js_html2_HTMLCollection.cpp 35984 2010-11-09 14:35:13Z santonov $
*/

#include <html_js.h>
using namespace v8;
v8::Handle<v8::Value> js_html2_HTMLCollection::item(long unsigned int val_index)
{
    if(val_index < length)
        return m_child_list[val_index]->m_this;
    return v8::Handle<v8::Value>();
}

v8::Handle<v8::Value> js_html2_HTMLCollection::namedItem(html2::DOMString val_name)
{
    v8::Handle<v8::Value> result;
    for(std::vector< boost::shared_ptr<v8_wrapper::TreeNode> >::iterator it = m_child_list.begin();
            (it != m_child_list.end()) && result.IsEmpty(); ++it) {
        result = v8_wrapper::check_object_name( (*it).get(), val_name );
    }
    return result;
}
