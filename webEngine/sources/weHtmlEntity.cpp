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
//WeCompareMode HtmlEntity::compareMode = WeCompatreStrict;
#endif //__DOXYGEN__

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
/// @fn	HtmlEntity::HtmlEntity(iEntity* prnt)
///
/// @brief	Default constructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
HtmlEntity::HtmlEntity(iEntity* prnt /*= NULL*/) // :
//    chldList(), attributes()
{
    chldList.resize(0);
    attributes.clear();
    parent = prnt;
    entityName = "";
    GenerateId();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	HtmlEntity::HtmlEntity(HtmlEntity &entity)
///
/// @brief  Copy constructor.
///
/// @param  entity - the entity to make the copy of.
////////////////////////////////////////////////////////////////////////////////////////////////////
HtmlEntity::HtmlEntity(HtmlEntity &entity)
{
    chldList = entity.chldList;
    attributes = entity.attributes;
    parent = entity.parent;
    entityName = entity.entityName;
    GenerateId();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	HtmlEntity::~HtmlEntity(void)
///
/// @brief  Destructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
HtmlEntity::~HtmlEntity(void)
{
    ClearAttr();
    ClearChildren();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	string& HtmlEntity::InnerText(void)
///
/// @brief	Inner text.
/// Just a plain text inside this entity.
///
/// @retval	null if something wrong, inner text string otherwise.
////////////////////////////////////////////////////////////////////////////////////////////////////
const string& HtmlEntity::InnerText(void)
{
    string *retval = new string;
    EntityList::iterator  chld;

    *retval = "";
    for (chld = chldList.begin(); chld != chldList.end(); chld++) {
        if (iequals((*chld)->Name(), string("#text"))) {
            *retval += (*chld)->Attr("");
        }
    }

	return(*retval);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	string& HtmlEntity::OuterText(void)
///
/// @brief  Outer text.
///
/// The text with outer HTML tags for this entity and all children HTML representations.
/// @retval null if something wrong,
///         string that represents the HTML-encoded entity, includes own tags and all child
///         entities otherwise.
////////////////////////////////////////////////////////////////////////////////////////////////////
const string& HtmlEntity::OuterText(void)
{
    string *retval = new string;
    EntityList::iterator chld;
    AttrMap::iterator attr;
    char quote = '"';

    *retval = "<";
    *retval += entityName;
    for (attr = attributes.begin(); attr != attributes.end(); attr++) {
        *retval += " ";
        *retval += attributes.key(attr);
        *retval += "=";
        if (attributes.val(attr) != "") {
            if (attributes.val(attr).find("\\\"") != string::npos) {
                // quoted " found - use the " mark
                quote = '"';
            }
            else {
                quote = '\'';
            }
            *retval += quote;
            *retval += attributes.val(attr);
            *retval += quote;
        }
    }
    *retval += ">";
    if (chldList.size() > 0 || !WeParseNeedToBreakTag(entityName, "")){
        for (chld = chldList.begin(); chld != chldList.end(); chld++) {
            *retval += (*chld)->OuterText();
        }
        *retval += "</";
        *retval += entityName;
        *retval += ">";
    }

	return(*retval);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	weCmpState HtmlEntity::Compare(iEntity& entity,
///     weCmpMode mode)
///
/// @brief  Query if 'entity' is equal.
///
/// @param  entity  - the entity for comparison.
/// @param  mode    - the mode comparison mode.
///
/// @retval true if equal, false if not.
////////////////////////////////////////////////////////////////////////////////////////////////////
weCmpState HtmlEntity::Compare(iEntity& entity, weCmpMode mode)
{
    if (mode == weCmpDefault) {
        mode = compareMode;
    }
    /// @todo Implement this!
    throw runtime_error("Not implemented");
	return(weCmpLess);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	ScannerToken HtmlEntity::Parse( string tagName,
/// 	TagScanner& scanner, iTransport* processor )
///
/// @brief  Parses the given stream.
///
/// This function goes through the input stream by the TagScanner and creates HTML entity.
/// All child entities are created recursively.
///
/// @param  tagName - name of the tag.
/// @param  scanner - the scanner.
/// @param  processor - HttpTransport object with processing options and network connection
/// @retval	ScannerToken that represents current scanner's state.
////////////////////////////////////////////////////////////////////////////////////////////////////
ScannerToken HtmlEntity::Parse( string tagName, TagScanner& scanner, iTransport* processor /*= NULL*/ )
{
    ScannerToken  state;
    HtmlEntity    *chld;
    WeInnerText     *txt;
    string           txtAttr;
    string        lString;

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
    startPos = (int)(scanner.GetPos() - tagName.length());
    if (tagName.length() > 0) {
        startPos--; // if tagName present - skip the "<" symbol
    }
    attributes.clear();
    ClearChildren();
    txt = NULL;
    chld = NULL;
    txtAttr = "";

    while (true)
    {
        state = scanner.GetToken();
parseRestart:
        if (state == wstEof || state == wstError) {
            if (txt != NULL) {
                txt->Attr("", txtAttr);
                chldList.push_back(txt);
//printf("\tTEXT: {%s}\n", txtAttr.c_str());
                txtAttr.clear();
                txt = NULL;
            }
            break;
        }
        switch(state)
        {
        case wstTagEnd:
            lString = scanner.GetTagName();
            to_lower(lString);
            if (txt != NULL) {
                txt->Attr("", txtAttr);
                chldList.push_back(txt);
//-DBG: printf("\tTEXT: {%s}\n", txtAttr.c_str());
                txtAttr.clear();
                txt = NULL;
            }
            if (lString == "form") {
                /// @todo Add forms processing
                // right now all form's attributes will be placed to parent entity
//-DBG: printf("FORM CLOSE\n");
                break;
            }
            if (entityName != lString) {
                // incomplete structure - close this tag
                // and restart parsing on previous level
//-DBG: printf("TAG END: %s (force)\n", entityName.c_str());
                if (!IsParentTag(lString)) {
                    // if tag is not in paren tree - just skip it
                    break;
                }
            }
            else {
//-DBG: printf("TAG END: %s\n", scanner.GetTagName());
            }
            endPos = (int)scanner.GetPos();
            return state;
        case wstTagStart:
            lString = scanner.GetTagName();
            to_lower(lString);
            if (txt != NULL) {
                txt->Attr("", txtAttr);
                chldList.push_back(txt);
//-DBG: printf("\tTEXT: {%s}\n", txtAttr.c_str());
                txtAttr.clear();
                txt = NULL;
            }
            if (lString == "form") {
                /// @todo Add forms processing
//-DBG: printf("NEW FORM BEGIN\n");
            }
            if (WeParseNeedToBreakTag(entityName, lString)) {
                // incomplete structure - close this tag
                // and restart parsing on previous level
//-DBG: printf("TAG END: %s (force)\n", entityName.c_str());
                return state;
            }
            if (lString == "form") {
                /// @todo Add forms processing
                // right now all form's attributes will be placed to parent entity
                break;
            }
//-DBG: printf("TAG START: %s\n", scanner.GetTagName());
            chld = weHtmlFactory.CreateEntity(lString, this);
            if (chld != NULL) {
                ScannerToken chldState;
                chldList.push_back(chld);
                chldState = chld->Parse(lString, scanner, processor);
                if (iequals(string("form"), lString)) {
                    /// @todo Add forms processing
                    // form finished?
                    // right now all form's attributes will be placed to parent entity
                    break;
                }
                if (chld->Name() != lString) {
                    // previous tag was closed incorrectly
                    // restart parsing
                    state = chldState;
                    goto parseRestart;
                }
                chld = NULL;
            }
        	break;
        case wstAttr:
            if (txt != NULL) {
                txt->Attr("", txtAttr);
                chldList.push_back(txt);
//-DBG: printf("\tTEXT: {%s}\n", txtAttr.c_str());
                txtAttr.clear();
                txt = NULL;
            }
//-DBG: printf("\tATTR: %s=%s\n", scanner.GetAttrName(), scanner.GetValue());
            lString = scanner.GetAttrName();
            to_lower(lString);
            attributes[lString] = scanner.GetValue();
        	break;
        case wstWord:
        case wstSpace:
            if (txt == NULL) {
                txt = new WeInnerText(this);
                txtAttr = "";
            }
            if (state == wstSpace && processor != NULL && processor->IsSet(weoCollapseSpaces)) { //HttpTransport::weoCollapseSpaces)) {
                txtAttr += " ";
            }
            else {
                txtAttr += scanner.GetValue();
            }
            break;
        case wstCommentStart:
            if (txt != NULL) {
                txt->Attr("", txtAttr);
                chldList.push_back(txt);
//-DBG: printf("\tTEXT: {%s}\n", txtAttr.c_str());
                txtAttr.clear();
                txt = NULL;
            }
            chld = new WeHtmlComment(this);
            break;
        case wstCDataStart:
            if (txt != NULL) {
                txt->Attr("", txtAttr);
                chldList.push_back(txt);
//-DBG: printf("\tTEXT: {%s}\n", txtAttr.c_str());
                txtAttr.clear();
                txt = NULL;
            }
            chld = new WeCData(this);
            break;
        case wstPiStart:
            if (txt != NULL) {
                txt->Attr("", txtAttr);
                chldList.push_back(txt);
//-DBG: printf("\tTEXT: {%s}\n", txtAttr.c_str());
                txtAttr.clear();
                txt = NULL;
            }
            chld = new WePhpInclude(this);
            break;
        case wstData:
            // force points the '#text' property to avoid other children types break
//-DBG: printf("%s - DATA: %s\n", chld->entityName.c_str(), scanner.GetValue());
            chld->Attr("#text", scanner.GetValue());
            break;
        case wstCommentEnd:
        case wstCDataEnd:
        case wstPiEnd:
            if (chld != NULL) {
                chldList.push_back(chld);
                chld = NULL;
            }
            break;
        default:
            break;
        }
    }

    endPos = (int)scanner.GetPos();
    return state;
}

CmpResults* HtmlEntity::Diff( iEntity& cmp, weCmpMode mode )
{
    /// @todo Implement this
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	WeInnerText::WeInnerText(iEntity* prnt)
///
/// @brief  Default constructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
WeInnerText::WeInnerText(iEntity* prnt /*= NULL*/) :
    HtmlEntity(prnt)
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
    HtmlEntity()
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
/// @fn	string WeInnerText::Attr(string )
///
/// @brief  Gets the attribute value.
///
/// @param  name - The attribute name (skipped).
///
/// @retval attribute value if the attribute exist,
///         empty string otherwise.
////////////////////////////////////////////////////////////////////////////////////////////////////
const string WeInnerText::Attr(string name)
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
/// @fn	void WeInnerText::Attr(string , string )
///
/// @brief  Sets the attribute.
///
/// @param  name - The attribute name (skipped).
/// @param  value - The attribute value.
////////////////////////////////////////////////////////////////////////////////////////////////////
void WeInnerText::Attr(string name, string value)
{
	attributes["#text"] = value;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	string& WeInnerText::InnerText(void)
///
/// @brief	Inner text.
/// @retval	null if something wrong, inner text string otherwise.
////////////////////////////////////////////////////////////////////////////////////////////////////
const string& WeInnerText::InnerText(void)
{
    AttrMap::iterator it;

    it = attributes.find(string("#text"));
    if (it != attributes.end())
    {
        return attributes.val(it);
    }
	return(*(new string("")));
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
const string& WeInnerText::OuterText(void)
{
    AttrMap::iterator it;

    it = attributes.find(string("#text"));
    if (it != attributes.end())
    {
        return attributes.val(it);
    }
    return(*(new string("")));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn CmpResults* WeInnerText::Diff(iEntity& cmp,
/// 	weCmpMode mode)
///
/// @brief  Builds difference between two texts. 
///
/// @param  cmp  - iEntity to be compared. 
/// @param  mode - The mode. 
///
/// @retval	null if it fails, the list of the compares else. 
////////////////////////////////////////////////////////////////////////////////////////////////////
CmpResults* WeInnerText::Diff(iEntity& cmp, weCmpMode mode)
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
/// @fn weCmpState WeInnerText::Compare(iEntity& cmp,
///     weCmpMode mode)
///
/// @brief  Compares two iEntity& objects to determine
///         their relative ordering. 
///
/// @param  cmp  - iEntity to be compared. 
/// @param  mode - comparison mode. 
///
/// @retval Negative if 'cmp' is less than this object, 0 if they are equal, or positive if it
///         is greater. See the weCmpState enum for symbolic constants.
////////////////////////////////////////////////////////////////////////////////////////////////////
weCmpState WeInnerText::Compare(iEntity& cmp, weCmpMode mode)
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
/// @fn	HtmlDocument::HtmlDocument(iEntity* prnt)
///
/// @brief	Default constructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
HtmlDocument::HtmlDocument(iEntity* prnt /*= NULL*/) //:
//    HtmlEntity(prnt), HttpResponse()
{
    entityName = "#document";
//     baseUrl = "";
//     realUrl = "";
//     relocCount = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	HtmlDocument::HtmlDocument(HtmlDocument& entity)
///
/// @brief  Copy constructor.
///
/// @param  entity  - the document to make copy of.
////////////////////////////////////////////////////////////////////////////////////////////////////
HtmlDocument::HtmlDocument(HtmlDocument& entity) //:
//    HtmlEntity(), HttpResponse()
{
    entityName = "#document";
    response = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	HtmlDocument::~HtmlDocument(void)
///
/// @brief  Destructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
HtmlDocument::~HtmlDocument(void)
{
    // nothing special for this class, just base classes
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	string& HtmlDocument::InnerText(void)
///
/// @brief	Inner text.
/// @retval	null if something wrong, inner text string otherwise.
////////////////////////////////////////////////////////////////////////////////////////////////////
const string& HtmlDocument::InnerText(void)
{
    return OuterText();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	string& HtmlDocument::OuterText(void)
///
/// @brief  Outer text.
///
/// @retval null if something wrong,
///         string that represents the HTML-encoded entity, includes own tags and all child
///         entities otherwise.
////////////////////////////////////////////////////////////////////////////////////////////////////
const string& HtmlDocument::OuterText(void)
{
    string *retval = new string;
    EntityList::iterator  chld;

    *retval = "";
    for (chld = chldList.begin(); chld != chldList.end(); chld++) {
        *retval += (*chld)->OuterText();
    }

    return(*retval);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn bool HtmlDocument::ParseData(iResponse* resp, iTransport* processor = NULL )
///
/// @brief  Parse data stored inside WeHttpResponce.
///
/// @param  processor - iTransport object with processing options and network connection
/// @param  resp - iResponse object with received data to parse
/// @retval	true if it succeeds, false if it fails.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool HtmlDocument::ParseData(iResponse* resp, iTransport* processor /*= NULL*/)
{
    bool retval = false;

    try
    {
        response = dynamic_cast<HttpResponse*>(resp);
        InStream* stream = response->Data().stream();
        if (stream) {
            TagScanner scanner(*stream);
            retval = (Parse("", scanner, processor) != wstError);
        }
        return retval;

    }
    catch (...) { }; // just skip
    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	Blob & HtmlDocument::Data( void )
///
/// @brief	Grants access to the linked response data. 
/// @throw  WeError if no iResponse assigned
////////////////////////////////////////////////////////////////////////////////////////////////////
Blob& HtmlDocument::Data( void )
{
    if (response != NULL) {
        return response->Data();
    }
    throw WeError("WeRefrenceObject::Data - no data linked!");
}

CmpResults* HtmlDocument::Diff( iEntity& cmp, weCmpMode mode )
{
    /// @todo Implement this!
    return NULL;
}

weCmpState HtmlDocument::Compare( iEntity& cmp, weCmpMode mode )
{
    /// @todo Implement this!
    return weCmpNonComparable;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	WeRefrenceObject::WeRefrenceObject(iEntity* prnt)
///
/// @brief  Default constructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
WeRefrenceObject::WeRefrenceObject(iEntity* prnt /*= NULL*/) :
    HtmlDocument(prnt)
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

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	WeScript::WeScript( iEntity* prnt )
///
/// @brief  Default constructor.
///
/// @param  prnt - If non-null, the prnt.
////////////////////////////////////////////////////////////////////////////////////////////////////
WeScript::WeScript( iEntity* prnt /*= NULL*/ ) :
    WeRefrenceObject(prnt)
{
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
/// @fn	ScannerToken WeScript::Parse( string tagName,
/// 	TagScanner& scanner, HttpTransport* processor )
///
/// @brief  Parses the given stream.
///
/// This is the specialisation for the parsing function to parse scripting objects.
///
/// @param  tagName - name of the tag.
/// @param  scanner - the scanner.
/// @param  processor - HttpTransport object with processing options and network connection
/// @retval	ScannerToken that represents current scanner's state.
////////////////////////////////////////////////////////////////////////////////////////////////////
ScannerToken WeScript::Parse( string tagName, TagScanner& scanner, HttpTransport* processor /*= NULL*/ )
{
    ScannerToken  state;
    string          txtAttr;
    bool            inOurTag;
    bool            inOtherTag;
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
        enPos = (int)scanner.GetPos();
        state = scanner.GetToken();
        // skip errors
        if (state == wstEof ) { //|| state == wstError
            //LOG4CXX_TRACE(iLogger::GetLogger(), "WeScript::Parse: wstEof " << scanner.GetTagName() << "; val " << scanner.GetValue());
            inProcess = false;
            break;
        }
        switch(state)
        {
        case wstTagEnd:
            //LOG4CXX_TRACE(iLogger::GetLogger(), "WeScript::Parse: tagend " << scanner.GetTagName());
            inOurTag = false;
            /// @todo process unpaired tag \</script\> inside the data
            // if it our tag - finish parsing
            if (iequals(scanner.GetTagName(), "script")) {
                // It's the finish
                inProcess = false;
            }
            break;
        case wstTagStart:
            //LOG4CXX_TRACE(iLogger::GetLogger(), "WeScript::Parse: tagstart " << scanner.GetTagName());
            // all TAGs inside script are ignored - it's just a text
            inOurTag = false;
            inOtherTag = true;
            break;
        case wstAttr:
            // attributes processed only for our tag, else it's just a text
            if (inOurTag) {
                attributes[scanner.GetAttrName()] = scanner.GetValue();
                //LOG4CXX_TRACE(iLogger::GetLogger(), "WeScript::Parse: " << scanner.GetAttrName() << " = " << scanner.GetValue());
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
            //LOG4CXX_TRACE(iLogger::GetLogger(), "WeScript::Parse: content " << scanner.GetValue());
            if (stPos == -1) {
                stPos = enPos + 1;
            }
            inOurTag = false;
            break;
            }
        }
    //LOG4CXX_TRACE(iLogger::GetLogger(), "WeScript::Parse: get content from " << stPos << " to " << enPos);
    if (stPos > -1 && enPos > stPos) {
        //txtAttr = scanner.GetFrom(stPos);
        enPos -= stPos;
        txtAttr = txtAttr.substr(0, enPos);
        Attr("#code", txtAttr);
    }
    LOG4CXX_TRACE(iLogger::GetLogger(), "WeScript::Parse: exit for: " << scanner.GetTagName());

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
