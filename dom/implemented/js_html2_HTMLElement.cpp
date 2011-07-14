
/*
  $Id: js_html2_HTMLElement.cpp 36341 2010-11-17 12:32:49Z santonov $
*/

#include "precomp.h"
using namespace v8;

js_html2_HTMLElement::js_html2_HTMLElement()
{
    boost::shared_ptr< js_css_CSS2Properties > node(new js_css_CSS2Properties());
    node->m_this = v8::Persistent<v8::Object>::New(v8_wrapper::wrap_object< js_css_CSS2Properties >(node.get()));
    //node->m_this.MakeWeak( NULL , v8_wrapper::Registrator< js_css_CSS2Properties >::Destructor);
    style = node->m_this;
    m_fields.push_back(boost::shared_static_cast<v8_wrapper::tree_node>(node));
}

js_html2_HTMLElement::~js_html2_HTMLElement()
{
    m_fields.clear();
    style.Clear();
}
