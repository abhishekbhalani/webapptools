
/*
  $Id: js_html2_HTMLInputElement.cpp 36341 2010-11-17 12:32:49Z santonov $
*/

#include "precomp.h"
using namespace v8;

void js_html2_HTMLInputElement::blur()
{
    html2::HTMLInputElement::blur();
}
void js_html2_HTMLInputElement::focus()
{
}
void js_html2_HTMLInputElement::select()
{
    html2::HTMLInputElement::select();
}
void js_html2_HTMLInputElement::click()
{
    html2::HTMLInputElement::click();
}

js_html2_HTMLInputElement::js_html2_HTMLInputElement() {}

js_html2_HTMLInputElement::~js_html2_HTMLInputElement() {}
