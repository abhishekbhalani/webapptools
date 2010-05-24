/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew Abramov aabramov@ptsecurity.ru

    This file is part of webEngine

    webEngine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <time.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/compare.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/regex.hpp>
#include "weHelper.h"
#include "weHtmlEntity.h"

using namespace boost;
using namespace boost::algorithm;

using namespace webEngine;

#ifndef __DOXYGEN__
//WeCompareMode html_entity::compareMode = WeCompatreStrict;
#endif //__DOXYGEN__

static string empty_string = "";

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	static bool WeParseNeedToBreakTag(const string& tagName,
/// 	const string& nextTag)
///
/// @brief  Is the parsing need to break tag.
///
/// @author A. Abramov
/// @date   02.06.2009
///
/// @param  tagName - name of the current tag.
/// @param  nextTag - the next tag, which started before current is closed.
///
/// @retval true if it need to start new tag, false otherwise.
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool WeParseNeedToBreakTag(const string& tagName, const string& nextTag)
{
    bool retval = false;

    /// @remark The list of TAGs and rules is the subject to addition
    if (iequals(tagName, "meta")) {
        return true;
    }
    if (iequals(tagName, "br")) {
        return true;
    }
    if (iequals(tagName, "img")) {
        return true;
    }
    if (iequals(tagName, "base")) {
        return true;
    }
    if (iequals(tagName, "link")) {
        return true;
    }
    if (iequals(tagName, "input")) {
        return true;
    }
    if (iequals(tagName, "li")) {
        if (iequals(nextTag, "li")) {
            return true;
        }
    }
    if (iequals(tagName, "tr")) {
        if (iequals(nextTag, "tr")) {
            return true;
        }
//         if (iequals(nextTag, "table")) {
//             return true;
//         }
    }
    if (iequals(tagName, "td")) {
        if (iequals(nextTag, "td")) {
            return true;
        }
        if (iequals(nextTag, "tr")) {
            return true;
        }
    }

    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	html_entity::html_entity(base_entity_ptr prnt)
///
/// @brief	Default constructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
html_entity::html_entity(base_entity_ptr prnt /*= NULL*/) // :
//    chldList(), attributes()
{
    chldList.resize(0);
    attributes.clear();
    parent = prnt;
    entityName = "";
    GenerateId();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	html_entity::html_entity(html_entity &entity)
///
/// @brief  Copy constructor.
///
/// @param  entity - the entity to make the copy of.
////////////////////////////////////////////////////////////////////////////////////////////////////
html_entity::html_entity(html_entity &entity)
{
    chldList = entity.chldList;
    attributes = entity.attributes;
    parent = entity.parent;
    entityName = entity.entityName;
    GenerateId();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	html_entity::~html_entity(void)
///
/// @brief  Destructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
html_entity::~html_entity(void)
{
    ClearAttr();
    ClearChildren();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	string& html_entity::InnerText(void)
///
/// @brief	Inner text.
/// Just a plain text inside this entity.
///
/// @retval	null if something wrong, inner text string otherwise.
////////////////////////////////////////////////////////////////////////////////////////////////////
const string html_entity::InnerText(void)
{
    string retval; // = new string;
    entity_list::iterator  chld;

    retval = "";
    for (chld = chldList.begin(); chld != chldList.end(); chld++) {
        if (iequals((*chld)->Name(), string("#text"))) {
            retval += (*chld)->attr("");
        }
    }

	return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	string& html_entity::OuterText(void)
///
/// @brief  Outer text.
///
/// The text with outer HTML tags for this entity and all children HTML representations.
/// @retval null if something wrong,
///         string that represents the HTML-encoded entity, includes own tags and all child
///         entities otherwise.
////////////////////////////////////////////////////////////////////////////////////////////////////
const string html_entity::OuterText(void)
{
    string retval; // = new string;
    entity_list::iterator chld;
    AttrMap::iterator attr;
    char quote = '"';

    retval = "<";
    retval += entityName;
    for (attr = attributes.begin(); attr != attributes.end(); attr++) {
        retval += " ";
        retval += attributes.key(attr);
        retval += "=";
        if (attributes.val(attr) != "") {
            if (attributes.val(attr).find("\\\"") != string::npos) {
                // quoted " found - use the " mark
                quote = '"';
            }
            else {
                quote = '\'';
            }
            retval += quote;
            retval += attributes.val(attr);
            retval += quote;
        }
    }
    retval += ">";
    if (chldList.size() > 0 || !WeParseNeedToBreakTag(entityName, "")){
        for (chld = chldList.begin(); chld != chldList.end(); chld++) {
            retval += (*chld)->OuterText();
        }
        retval += "</";
        retval += entityName;
        retval += ">";
    }

	return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	weCmpState html_entity::Compare(base_entity& entity,
///     weCmpMode mode)
///
/// @brief  Query if 'entity' is equal.
///
/// @param  entity  - the entity for comparison.
/// @param  mode    - the mode comparison mode.
///
/// @retval true if equal, false if not.
////////////////////////////////////////////////////////////////////////////////////////////////////
weCmpState html_entity::Compare(base_entity& entity, weCmpMode mode)
{
    if (mode == weCmpDefault) {
        mode = compareMode;
    }
    /// @todo Implement this!
    throw runtime_error("Not implemented");
	return(weCmpLess);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	scanner_token html_entity::Parse( string tagName,
/// 	tag_scanner& scanner, i_transport* processor )
///
/// @brief  Parses the given stream.
///
/// This function goes through the input stream by the tag_scanner and creates HTML entity.
/// All child entities are created recursively.
///
/// @param  tagName - name of the tag.
/// @param  scanner - the scanner.
/// @param  processor - HttpTransport object with processing options and network connection
/// @retval	scanner_token that represents current scanner's state.
////////////////////////////////////////////////////////////////////////////////////////////////////
scanner_token html_entity::Parse( string tagName, tag_scanner& scanner, i_transport* processor /*= NULL*/ )
{
    scanner_token  state;
    base_entity_ptr     chld;
    WeInnerText*   txt;
    string         txtAttr;
    string         lString;

    if (! tagName.empty()) {
        // for WeDocuments this parameter must be empty
        // and not to overwrite the 'name' property
        entityName = tagName;
    }

    if (processor != NULL) {
        // opts = processor->Options();
    }
    else {
        // if no HttpTransport given - use default options? but no follow the links
        //opts = HttpTransport::weoDefault & (~HttpTransport::weoFollowLinks);
    }
    startPos = (int)(scanner.get_pos() - tagName.length());
    if (tagName.length() > 0) {
        startPos--; // if tagName present - skip the "<" symbol
    }
    attributes.clear();
    ClearChildren();
    txt = NULL;
    chld.reset();
    txtAttr = "";

    while (true)
    {
        state = scanner.get_token();
parseRestart:
        if (state == wstEof || state == wstError) {
            if (txt != NULL) {
                txt->attr("", txtAttr);
                chldList.push_back(base_entity_ptr(txt));
//printf("\tTEXT: {%s}\n", txtAttr.c_str());
                txtAttr.clear();
                txt = NULL;
            }
            break;
        }
        switch(state)
        {
        case wstTagEnd:
            lString = scanner.get_tag_name();
            to_lower(lString);
            if (txt != NULL) {
                txt->attr("", txtAttr);
                chldList.push_back(base_entity_ptr(txt));
                txtAttr.clear();
                txt = NULL;
            }
            if (lString == "form") {
                /// @todo Add forms processing
                // right now all form's attributes will be placed to parent entity
                LOG4CXX_TRACE(iLogger::GetLogger(), "html_entity::Parse: FORM CLOSE");
                break;
            }
            if (entityName != lString) {
                // incomplete structure - close this tag
                // and restart parsing on previous level
                LOG4CXX_TRACE(iLogger::GetLogger(), "html_entity::Parse: force TAG END(wstTagEnd): " << entityName);
                if (!IsParentTag(lString)) {
                    // if tag is not in paren tree - just skip it
                    break;
                }
            }
            else {
                LOG4CXX_TRACE(iLogger::GetLogger(), "html_entity::Parse: TAG END: " << scanner.get_tag_name());
            }
            endPos = (int)scanner.get_pos();
            return state;
        case wstTagStart:
            lString = scanner.get_tag_name();
            to_lower(lString);
            LOG4CXX_TRACE(iLogger::GetLogger(), "html_entity::Parse: TAG START: " << lString);
            if (txt != NULL) {
                txt->attr("", txtAttr);
                chldList.push_back(base_entity_ptr(txt));
                //LOG4CXX_TRACE(iLogger::GetLogger(), "html_entity::Parse: save previous TEXT: " << txtAttr);
                txtAttr.clear();
                txt = NULL;
            }
            if (WeParseNeedToBreakTag(entityName, lString)) {
                // incomplete structure - close this tag
                // and restart parsing on previous level
                LOG4CXX_TRACE(iLogger::GetLogger(), "html_entity::Parse: force TAG END(wstTagStart): " << entityName);
                return state;
            }
            if (lString == "form") {
                /// @todo Add forms processing
                // right now all form's attributes will be placed to parent entity
                LOG4CXX_TRACE(iLogger::GetLogger(), "html_entity::Parse: NEW FORM BEGIN");
                break;
            }
            chld = weHtmlFactory.CreateEntity(lString, shared_from_this());
            if (chld) {
                scanner_token chldState;
                chldList.push_back(chld);
                chldState = chld->Parse(lString, scanner, processor);
                LOG4CXX_TRACE(iLogger::GetLogger(), "html_entity::Parse: from child, state=" << chldState <<
                    " current=" << entityName << ", child=" << chld->Name() << ", active=" << lString);
                if (iequals(string("form"), lString)) {
                    /// @todo Add forms processing
                    // form finished?
                    // right now all form's attributes will be placed to parent entity
                    break;
                }
                if (chldState != wstTagEnd && state != wstEof && state != wstError) {
                    // previous tag was closed incorrectly
                    // restart parsing
                    LOG4CXX_TRACE(iLogger::GetLogger(), "html_entity::Parse: restarting from child");
                    state = chldState;
                    goto parseRestart;
                }
                //chld = NULL;
            }
        	break;
        case wstAttr:
            if (txt != NULL) {
                txt->attr("", txtAttr);
                chldList.push_back(base_entity_ptr(txt));
//-DBG: printf("\tTEXT: {%s}\n", txtAttr.c_str());
                txtAttr.clear();
                txt = NULL;
            }
//-DBG: printf("\tATTR: %s=%s\n", scanner.get_attr_name(), scanner.get_value());
            lString = scanner.get_attr_name();
            to_lower(lString);
            attributes[lString] = scanner.get_value();
        	break;
        case wstWord:
        case wstSpace:
            if (txt == NULL) {
                txt = new WeInnerText(shared_from_this());
                txtAttr = "";
            }
            /// @todo: implement option receiver for parser
            if (state == wstSpace && processor != NULL && processor->is_set(weoCollapseSpaces)) { //HttpTransport::weoCollapseSpaces)) {
                txtAttr += " ";
            }
            else {
                txtAttr += scanner.get_value();
            }
            break;
        case wstCommentStart:
            if (txt != NULL) {
                txt->attr("", txtAttr);
                chldList.push_back(base_entity_ptr(txt));
//-DBG: printf("\tTEXT: {%s}\n", txtAttr.c_str());
                txtAttr.clear();
                txt = NULL;
            }
            chld = base_entity_ptr(new WeHtmlComment(shared_from_this()));
            break;
        case wstCDataStart:
            if (txt != NULL) {
                txt->attr("", txtAttr);
                chldList.push_back(base_entity_ptr(txt));
//-DBG: printf("\tTEXT: {%s}\n", txtAttr.c_str());
                txtAttr.clear();
                txt = NULL;
            }
            chld = base_entity_ptr(new WeCData(shared_from_this()));
            break;
        case wstPiStart:
            if (txt != NULL) {
                txt->attr("", txtAttr);
                chldList.push_back(base_entity_ptr(txt));
//-DBG: printf("\tTEXT: {%s}\n", txtAttr.c_str());
                txtAttr.clear();
                txt = NULL;
            }
            chld = base_entity_ptr(new WePhpInclude(shared_from_this()));
            break;
        case wstData:
            // force points the '#text' property to avoid other children types break
//-DBG: printf("%s - DATA: %s\n", chld->entityName.c_str(), scanner.get_value());
            chld->attr("#text", scanner.get_value());
            break;
        case wstCommentEnd:
        case wstCDataEnd:
        case wstPiEnd:
            if (chld != NULL) {
                chldList.push_back(chld);
                //chld = NULL;
            }
            break;
        default:
            break;
        }
    }

    endPos = (int)scanner.get_pos();
    return state;
}

CmpResults* html_entity::Diff( base_entity& cmp, weCmpMode mode )
{
    /// @todo Implement this
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	WeInnerText::WeInnerText(base_entity_ptr prnt)
///
/// @brief  Default constructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
WeInnerText::WeInnerText(base_entity_ptr prnt /*= NULL*/) :
    html_entity(prnt)
{
    entityName = "#text";
    attributes["#text"] = "";
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	WeInnerText::WeInnerText(WeInnerText& entity)
///
/// @brief  Copy constructor.
///
/// @param  entity  - the entity.
////////////////////////////////////////////////////////////////////////////////////////////////////
WeInnerText::WeInnerText(WeInnerText& entity) :
    html_entity()
{
    entityName = "#text";
    attributes["#text"] = entity.attributes["#text"];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	WeInnerText::~WeInnerText()
///
/// @brief  Destructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
WeInnerText::~WeInnerText()
{
    // nothing special
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	string WeInnerText::attr(string )
///
/// @brief  Gets the attribute value.
///
/// @param  name - The attribute name (skipped).
///
/// @retval attribute value if the attribute exist,
///         empty string otherwise.
////////////////////////////////////////////////////////////////////////////////////////////////////
const string WeInnerText::attr(string name)
{
    AttrMap::iterator it;

    it = attributes.find("#text");
    if (it != attributes.end())
    {
        return attributes.val(it);
    }
    return(*(new string("")));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void WeInnerText::attr(string , string )
///
/// @brief  Sets the attribute.
///
/// @param  name - The attribute name (skipped).
/// @param  value - The attribute value.
////////////////////////////////////////////////////////////////////////////////////////////////////
void WeInnerText::attr(string name, string value)
{
	attributes["#text"] = value;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	string& WeInnerText::InnerText(void)
///
/// @brief	Inner text.
/// @retval	null if something wrong, inner text string otherwise.
////////////////////////////////////////////////////////////////////////////////////////////////////
const string WeInnerText::InnerText(void)
{
    AttrMap::iterator it;

    it = attributes.find(string("#text"));
    if (it != attributes.end())
    {
        return attributes.val(it);
    }
	return empty_string;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	string& WeInnerText::OuterText(void)
///
/// @brief  Outer text.
///
/// @retval null if something wrong,
///         string that represents the HTML-encoded entity, includes own tags and all child
///         entities otherwise.
////////////////////////////////////////////////////////////////////////////////////////////////////
const string WeInnerText::OuterText(void)
{
    AttrMap::iterator it;

    it = attributes.find(string("#text"));
    if (it != attributes.end())
    {
        return attributes.val(it);
    }
    return empty_string;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn CmpResults* WeInnerText::Diff(base_entity& cmp,
/// 	weCmpMode mode)
///
/// @brief  Builds difference between two texts. 
///
/// @param  cmp  - base_entity to be compared. 
/// @param  mode - The mode. 
///
/// @retval	null if it fails, the list of the compares else. 
////////////////////////////////////////////////////////////////////////////////////////////////////
CmpResults* WeInnerText::Diff(base_entity& cmp, weCmpMode mode)
{
    WeInnerText* ptr;
    CmpResults* retval = NULL;
    string s1, s2;
    regex repl("\\s+");

    if (mode == weCmpDefault) {
        mode = compareMode;
    }
    try {
        ptr = reinterpret_cast<WeInnerText*>(&cmp);
        s1 = attributes["#text"];
        s2 = ptr->attributes["#text"];
        retval = TextDiff(s1, s2, mode);
    } catch (...) {
        if (retval != NULL) {
            delete retval;
        }
        retval = NULL;
    }

    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn weCmpState WeInnerText::Compare(base_entity& cmp,
///     weCmpMode mode)
///
/// @brief  Compares two base_entity& objects to determine
///         their relative ordering. 
///
/// @param  cmp  - base_entity to be compared. 
/// @param  mode - comparison mode. 
///
/// @retval Negative if 'cmp' is less than this object, 0 if they are equal, or positive if it
///         is greater. See the weCmpState enum for symbolic constants.
////////////////////////////////////////////////////////////////////////////////////////////////////
weCmpState WeInnerText::Compare(base_entity& cmp, weCmpMode mode)
{
    WeInnerText* ptr;
    string s1, s2;
    weCmpState retval = weCmpNonComparable;
    regex repl("\\s+");

    if (mode == weCmpDefault) {
        mode = compareMode;
    }
    try {
        ptr = reinterpret_cast<WeInnerText*>(&cmp);
        s1 = attributes["#text"];
        s2 = ptr->attributes["#text"];
        if (mode & weCmpCollapseSpace) {
            s1 = regex_replace(s1, repl, " ");
            s2 = regex_replace(s2, repl, " ");
        }
        if (mode & weCmpCaseInsens) {
            is_iless cp_less;
            if (iequals(s1, s2)) {
                retval = weCmpEqual;
            }
            else if(cp_less(s1.c_str(), s2.c_str())) {
                retval = weCmpLess;
            } else {
                retval = weCmpGreather;
            }
        }
        else {
            is_less cp_less;
            if (equals(s1, s2)) {
                retval = weCmpEqual;
            }
            else if(cp_less(s1, s2)) {
                retval = weCmpLess;
            } else {
                retval = weCmpGreather;
            }
        }
    } catch (...) {
        retval = weCmpNonComparable;
    }

    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	html_document::html_document(base_entity_ptr prnt)
///
/// @brief	Default constructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
html_document::html_document(base_entity_ptr prnt /*= NULL*/) //:
//    html_entity(prnt), HttpResponse()
{
    response.reset();
    entityName = "#document";
//     baseUrl = "";
//     realUrl = "";
//     relocCount = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	html_document::html_document(html_document& entity)
///
/// @brief  Copy constructor.
///
/// @param  entity  - the document to make copy of.
////////////////////////////////////////////////////////////////////////////////////////////////////
html_document::html_document(html_document& entity) //:
//    html_entity(), HttpResponse()
{
    response.reset();
    entityName = "#document";
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	html_document::~html_document(void)
///
/// @brief  Destructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
html_document::~html_document(void)
{
    if (response) {
        response.reset();
    }
    // nothing special for this class, just base classes
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	string& html_document::InnerText(void)
///
/// @brief	Inner text.
/// @retval	null if something wrong, inner text string otherwise.
////////////////////////////////////////////////////////////////////////////////////////////////////
const string html_document::InnerText(void)
{
    return OuterText();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	string& html_document::OuterText(void)
///
/// @brief  Outer text.
///
/// @retval null if something wrong,
///         string that represents the HTML-encoded entity, includes own tags and all child
///         entities otherwise.
////////////////////////////////////////////////////////////////////////////////////////////////////
const string html_document::OuterText(void)
{
    string retval; // = new string;
    entity_list::iterator  chld;

    retval = "";
    for (chld = chldList.begin(); chld != chldList.end(); chld++) {
        retval += (*chld)->OuterText();
    }

    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn bool html_document::ParseData(i_response* resp, i_transport* processor = NULL )
///
/// @brief  Parse data stored inside WeHttpResponce.
///
/// @param  processor - i_transport object with processing options and network connection
/// @param  resp - i_response object with received data to parse
/// @retval	true if it succeeds, false if it fails.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool html_document::ParseData(boost::shared_ptr<i_response> resp, i_transport* processor /*= NULL*/)
{
    bool retval = false;

    try
    {
        response = boost::shared_dynamic_cast<HttpResponse>(resp);
        boost::shared_ptr<tag_stream> stream = response->Data().stream();
        if (stream.get()) {
            tag_scanner scanner(*stream);
            retval = (Parse("", scanner, processor) != wstError);
        }
        return retval;

    }
    catch (...) { }; // just skip
    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	blob & html_document::Data( void )
///
/// @brief	Grants access to the linked response data. 
/// @throw  WeError if no i_response assigned
////////////////////////////////////////////////////////////////////////////////////////////////////
blob& html_document::Data( void )
{
    if (response != NULL) {
        return response->Data();
    }
    throw WeError("WeRefrenceObject::Data - no data linked!");
}

CmpResults* html_document::Diff( base_entity& cmp, weCmpMode mode )
{
    /// @todo Implement this!
    return NULL;
}

weCmpState html_document::Compare( base_entity& cmp, weCmpMode mode )
{
    /// @todo Implement this!
    return weCmpNonComparable;
}

scanner_token html_document::Parse( string tagName, tag_scanner& scanner, i_transport* processor /*= NULL*/ )
{
    return html_entity::Parse(tagName, scanner, processor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	WeRefrenceObject::WeRefrenceObject(base_entity_ptr prnt)
///
/// @brief  Default constructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
WeRefrenceObject::WeRefrenceObject(base_entity_ptr prnt /*= NULL*/) :
    html_document(prnt)
{
    /// @todo Implement this!
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	WeRefrenceObject::WeRefrenceObject( WeRefrenceObject& entity )
///
/// @brief  Copy constructor.
///
/// @param entity  - the entity to make copy of.
////////////////////////////////////////////////////////////////////////////////////////////////////
WeRefrenceObject::WeRefrenceObject( WeRefrenceObject& entity )
{
    /// @todo Implement this!
    throw runtime_error("Not implemented");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	WeRefrenceObject::~WeRefrenceObject()
///
/// @brief  Destructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
WeRefrenceObject::~WeRefrenceObject()
{
    /// @todo Implement this!
}

scanner_token WeRefrenceObject::Parse( string tagName, tag_scanner& scanner, i_transport* processor /*= NULL*/ )
{
    return html_entity::Parse(tagName, scanner, processor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	WeScript::WeScript( base_entity_ptr prnt )
///
/// @brief  Default constructor.
///
/// @param  prnt - If non-null, the prnt.
////////////////////////////////////////////////////////////////////////////////////////////////////
WeScript::WeScript( base_entity_ptr prnt /*= NULL*/ ) :
    WeRefrenceObject(prnt)
{
    entityName = "script";
    /// @todo Implement this!
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	WeScript::WeScript( WeScript& entity )
///
/// @brief  Copy constructor.
///
/// @param entity - the entity to make copy of.
////////////////////////////////////////////////////////////////////////////////////////////////////
WeScript::WeScript( WeScript& entity )
{
    /// @todo Implement this!
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	WeScript::~WeScript()
///
/// @brief  Destructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
WeScript::~WeScript()
{
    /// @todo Implement this!
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	scanner_token WeScript::Parse( string tagName,
/// 	tag_scanner& scanner, i_transport* processor = NULL )
///
/// @brief  Parses the given stream.
///
/// This is the specialisation for the parsing function to parse scripting objects.
///
/// @param  tagName - name of the tag.
/// @param  scanner - the scanner.
/// @param  processor - HttpTransport object with processing options and network connection
/// @retval	scanner_token that represents current scanner's state.
////////////////////////////////////////////////////////////////////////////////////////////////////
scanner_token WeScript::Parse( string tagName, tag_scanner& scanner, i_transport* processor /*= NULL*/ )
{
    scanner_token  state;
    string          txtAttr;
    bool            inOurTag;
    bool            inProcess;
    int             enPos, stPos;

    LOG4CXX_TRACE(iLogger::GetLogger(), "WeScript::Parse: enter for: " << tagName);
    if (! tagName.empty()) {
        // for WeDocuments this parameter must be empty
        // and not to overwrite the 'name' property
        entityName = tagName;
    }

    if (processor != NULL) {
        //opts = processor->Options();
    }
    else {
        // if no HttpTransport given - use default options? but no follow the links
        // opts = HttpTransport::weoDefault & (~HttpTransport::weoFollowLinks);
    }
    attributes.clear();
    ClearChildren();
    txtAttr = "";
    inOurTag = true;
    inProcess = true;
    stPos = -1;

    while (inProcess)
    {
        enPos = (int)scanner.get_pos();
        state = scanner.get_token();
        // skip errors
        if (state == wstEof ) { //|| state == wstError
            //LOG4CXX_TRACE(iLogger::GetLogger(), "WeScript::Parse: wstEof " << scanner.get_tag_name() << "; val " << scanner.get_value());
            inProcess = false;
            break;
        }
        switch(state)
        {
        case wstTagEnd:
            LOG4CXX_TRACE(iLogger::GetLogger(), "WeScript::Parse: tagend " << scanner.get_tag_name());
            inOurTag = false;
            /// @todo process unpaired tag \</script\> inside the data
            // if it our tag - finish parsing
            if (iequals(scanner.get_tag_name(), "script")) {
                // It's the finish
                inProcess = false;
            }
            break;
        case wstTagStart:
            LOG4CXX_TRACE(iLogger::GetLogger(), "WeScript::Parse: tagstart " << scanner.get_tag_name());
            // all TAGs inside script are ignored - it's just a text
            scanner.reset_to_body();
            break;
        case wstAttr:
            // attributes processed only for our tag, else it's just a text
            if (inOurTag) {
                attributes[scanner.get_attr_name()] = scanner.get_value();
                //LOG4CXX_TRACE(iLogger::GetLogger(), "WeScript::Parse: " << scanner.get_attr_name() << " = " << scanner.get_value());
            }
            // reset state to parse content
            break;

        // all other cases
        case wstWord:
        case wstSpace:
        case wstCommentStart:
        case wstCDataStart:
        case wstPiStart:
        case wstData:
        case wstCommentEnd:
        case wstCDataEnd:
        case wstPiEnd:
        default:
            LOG4CXX_TRACE(iLogger::GetLogger(), "WeScript::Parse: content " << scanner.get_value());
            if (stPos == -1) {
                stPos = enPos + 1;
            }
            inOurTag = false;
            break;
        }
    }
    //LOG4CXX_TRACE(iLogger::GetLogger(), "WeScript::Parse: get content from " << stPos << " to " << enPos);
    if (stPos > -1 && enPos > stPos) {
        txtAttr = scanner.get_from(stPos);
        enPos -= stPos;
        txtAttr = txtAttr.substr(0, enPos);
        attr("#code", txtAttr);
    }
    LOG4CXX_TRACE(iLogger::GetLogger(), "WeScript::Parse: exit for: " << scanner.get_tag_name());

    /// @todo Implement post-process - download and others
    return state;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void WeScript::SetEngine( void *engine )
///
/// @brief  Sets the scripting engine.
///
/// Sets or creates the scripting engine. If given parameter engine is NULL, this function tries
/// to create the scripting engine based on the @e language attribute value. The default engine
/// is the JavaScript.
///
/// @param  engine - The pointer to the engine
/// @retval true if success, false otherwise
////////////////////////////////////////////////////////////////////////////////////////////////////
bool WeScript::SetEngine( void *engine /*= NULL*/ )
{
    /// @todo implement this
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void* WeScript::Execute()
///
/// @brief  Executes the inherited script.
///
/// @retval	null if it fails, cript execution results else.
////////////////////////////////////////////////////////////////////////////////////////////////////
void* WeScript::Execute()
{
    /// @todo implement this
    return NULL;
}
