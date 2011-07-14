
/*
  DO NOT EDIT!
  This file has been generated by generate_sources.py script.
  $Id: js_ranges_Range.cpp 36341 2010-11-17 12:32:49Z santonov $
*/

#include "precomp.h"
using namespace v8;

void js_ranges_Range::setStart(v8::Handle<v8::Value> val_refNode, long int val_offset)
{
    ranges::Range::setStart(val_refNode, val_offset);
}
void js_ranges_Range::setEnd(v8::Handle<v8::Value> val_refNode, long int val_offset)
{
    ranges::Range::setEnd(val_refNode, val_offset);
}
void js_ranges_Range::setStartBefore(v8::Handle<v8::Value> val_refNode)
{
    ranges::Range::setStartBefore(val_refNode);
}
void js_ranges_Range::setStartAfter(v8::Handle<v8::Value> val_refNode)
{
    ranges::Range::setStartAfter(val_refNode);
}
void js_ranges_Range::setEndBefore(v8::Handle<v8::Value> val_refNode)
{
    ranges::Range::setEndBefore(val_refNode);
}
void js_ranges_Range::setEndAfter(v8::Handle<v8::Value> val_refNode)
{
    ranges::Range::setEndAfter(val_refNode);
}
void js_ranges_Range::collapse(bool val_toStart)
{
    ranges::Range::collapse(val_toStart);
}
void js_ranges_Range::selectNode(v8::Handle<v8::Value> val_refNode)
{
    ranges::Range::selectNode(val_refNode);
}
void js_ranges_Range::selectNodeContents(v8::Handle<v8::Value> val_refNode)
{
    ranges::Range::selectNodeContents(val_refNode);
}
short int js_ranges_Range::compareBoundaryPoints(short unsigned int val_how, v8::Handle<v8::Value> val_sourceRange)
{
    return ranges::Range::compareBoundaryPoints(val_how, val_sourceRange);
}
void js_ranges_Range::deleteContents()
{
    ranges::Range::deleteContents();
}
v8::Handle<v8::Value> js_ranges_Range::extractContents()
{
    return ranges::Range::extractContents();
}
v8::Handle<v8::Value> js_ranges_Range::cloneContents()
{
    return ranges::Range::cloneContents();
}
void js_ranges_Range::insertNode(v8::Handle<v8::Value> val_newNode)
{
    ranges::Range::insertNode(val_newNode);
}
void js_ranges_Range::surroundContents(v8::Handle<v8::Value> val_newParent)
{
    ranges::Range::surroundContents(val_newParent);
}
v8::Handle<v8::Value> js_ranges_Range::cloneRange()
{
    return ranges::Range::cloneRange();
}
ranges::DOMString js_ranges_Range::toString()
{
    return ranges::Range::toString();
}
void js_ranges_Range::detach()
{
    ranges::Range::detach();
}

js_ranges_Range::js_ranges_Range() {}

js_ranges_Range::~js_ranges_Range() {}
