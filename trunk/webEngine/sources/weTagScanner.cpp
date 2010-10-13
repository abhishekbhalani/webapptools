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
#include <webEngine.h>

#include "weTagScanner.h"

#ifndef __DOXYGEN__
// case sensitive string equality test
// s_lowcase shall be lowercase string
static inline bool equal(const char* s, const char* s1, size_t length)
{
    switch(length) {
    case 8:
        if(s1[7] != s[7]) return false;
    case 7:
        if(s1[6] != s[6]) return false;
    case 6:
        if(s1[5] != s[5]) return false;
    case 5:
        if(s1[4] != s[4]) return false;
    case 4:
        if(s1[3] != s[3]) return false;
    case 3:
        if(s1[2] != s[2]) return false;
    case 2:
        if(s1[1] != s[1]) return false;
    case 1:
        if(s1[0] != s[0]) return false;
    case 0:
        return true;
    default:
        return strncmp(s,s1,length) == 0;
    }
}
#endif //__DOXYGEN__

namespace webEngine {
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	char* tag_scanner::get_value(void)
///
/// @brief  Gets the value.
/// @retval	null if it fails, else the value.
////////////////////////////////////////////////////////////////////////////////////////////////////
const char* tag_scanner::get_value(void)
{
    value.push_back(0); //[value_length] = 0;
    return &value[0];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	char* tag_scanner::get_attr_name(void)
///
/// @brief  Gets the attribute name.
/// @retval	null if it fails, else the attribute name.
////////////////////////////////////////////////////////////////////////////////////////////////////
const char* tag_scanner::get_attr_name(void)
{
    attr_name.push_back(0); //[attr_name_length] = 0;
    return &attr_name[0];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	char* tag_scanner::get_tag_name(void)
///
/// @brief  Gets the tag name.
/// @retval	null if it fails, else the tag name.
////////////////////////////////////////////////////////////////////////////////////////////////////
const char* tag_scanner::get_tag_name(void)
{
    tag_name.push_back(0); //[tag_name_length] = 0;
    return &tag_name[0];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	scanner_token tag_scanner::ScanBody(void)
///
/// @brief  Scans the body.
/// @return	current token.
////////////////////////////////////////////////////////////////////////////////////////////////////
scanner_token tag_scanner::ScanBody(void)
{
    char c = GetChar();

    value.clear();

    bool ws = false;

    if(c == 0) return wstEof;
    else if(c == '<') return ScanTag();
    else if(c == '&')
        c = ScanEntity();
    else
        ws = IsWhitespace(c);

    while(true) {
        AppendValue(c);
        c = input.get_char();
        if(c == 0)  {
            /*push_back(c);*/ break;
        }
        if(c == '<') {
            step_back();
            break;
        }
        if(c == '&') {
            step_back();
            break;
        }

        if(IsWhitespace(c) != ws) {
            step_back();
            break;
        }

    }
    return ws? wstSpace:wstWord;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	scanner_token tag_scanner::ScanHead(void)
///
/// @brief	Scans the head.
/// @return	current token.
////////////////////////////////////////////////////////////////////////////////////////////////////
scanner_token tag_scanner::ScanHead(void)
{
    char c = SkipWhitespace();

    if(c == '>') {
        c_scan = &tag_scanner::ScanBody;
        return ScanBody();
    }
    if(c == '/') {
        char t = GetChar();
        if(t == '>')   {
            c_scan = &tag_scanner::ScanBody;
            return wstTagEnd;
        } else {
            step_back();    // erroneous situation - standalone '/'
            return wstError;
        }
    }

    attr_name.clear();
    value.clear();

    // attribute name...
    while(c != '=') {
        if( c == 0) return wstEof;
        if( c == '>' ) {
            step_back();    // attribute without value (HTML style)
            return wstAttr;
        }
        if( IsWhitespace(c) ) {
            c = SkipWhitespace();
            if(c != '=') {
                step_back();    // attribute without value (HTML style)
                return wstAttr;
            } else break;
        }
        if( c == '<') return wstError;
        AppendAttrName(c);
        c = GetChar();
    }

    c = SkipWhitespace();
    // attribute value...

    if(c == '\"')
        while(c = GetChar()) {
            if(c == '\"') return wstAttr;
            if(c == '&') c = ScanEntity();
            AppendValue(c);
        }
    else if(c == '\'') // allowed in html
        while(c = GetChar()) {
            if(c == '\'') return wstAttr;
            if(c == '&') c = ScanEntity();
            AppendValue(c);
        }
    else  // scan token, allowed in html: e.g. align=center
        do {
            if( IsWhitespace(c) ) return wstAttr;
            /* these two removed in favour of better html support:
            if( c == '/' || c == '>' ) { push_back(c); return wstAttr; }
            if( c == '&' ) c = scan_entity();*/
            if( c == '>' ) {
                step_back();
                return wstAttr;
            }
            AppendValue(c);
        } while(c = GetChar());

    return wstError;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	scanner_token tag_scanner::ScanComment(void)
///
/// @brief  Scans the comment.
/// @return	current token.
////////////////////////////////////////////////////////////////////////////////////////////////////
scanner_token tag_scanner::ScanComment(void)
{
    if(got_tail) {
        c_scan = &tag_scanner::ScanBody;
        got_tail = false;
        return wstCommentEnd;
    }
    value.clear();
    for(int value_length = 0; /*value_length < (MAX_TOKEN_SIZE - 1)*/; value_length++) {
        char c = GetChar();
        if( c == 0) return wstEof;
        value.push_back(c);

        if(value_length >= 2
                && value[value_length] == '>'
                && value[value_length - 1] == '-'
                && value[value_length - 2] == '-') {
            got_tail = true;
            value.pop_back();
            value.pop_back();
            value.pop_back();
            break;
        }
    }
    return wstData;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	scanner_token tag_scanner::ScanCdata()
///
/// @brief  Scans the cdata.
/// @return	current token.
////////////////////////////////////////////////////////////////////////////////////////////////////
scanner_token tag_scanner::ScanCdata()
{
    if(got_tail) {
        c_scan = &tag_scanner::ScanBody;
        got_tail = false;
        return wstCDataEnd;
    }
    value.clear();
    for(int value_length = 0; /*value_length < (MAX_TOKEN_SIZE - 1)*/; value_length++) {
        char c = GetChar();
        if( c == 0) return wstEof;
        value.push_back(c);

        if(value_length >= 2
                && value[value_length] == '>'
                && value[value_length - 1] == ']'
                && value[value_length - 2] == ']') {
            got_tail = true;
            value_length -= 2;
            value.pop_back();
            value.pop_back();
            value.pop_back();
            break;
        }
    }
    return wstData;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	scanner_token tag_scanner::ScanPi()
///
/// @brief  Scans the PHP includes.
/// @return	current token.
////////////////////////////////////////////////////////////////////////////////////////////////////
scanner_token tag_scanner::ScanPi()
{
    if(got_tail) {
        c_scan = &tag_scanner::ScanBody;
        got_tail = false;
        return wstPiEnd;
    }
    value.clear();
    for(int value_length = 0; /*value_length < (MAX_TOKEN_SIZE - 1)*/; value_length++) {
        char c = GetChar();
        if( c == 0) return wstEof;
        value.push_back(c);

        if(value_length >= 1
                && value[value_length] == '>'
                && value[value_length - 1] == '?') {
            got_tail = true;
            value_length -= 1;
            value.pop_back();
            value.pop_back();
            break;
        }
    }
    return wstData;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	scanner_token tag_scanner::ScanTag()
///
/// @brief  Scans the tag.
/// @return	current token.
////////////////////////////////////////////////////////////////////////////////////////////////////
scanner_token tag_scanner::ScanTag()
{
    tag_name.clear();

    char c = GetChar();

    bool is_tail = c == '/';
    if(is_tail) c = GetChar();

    while(c) {
        if(IsWhitespace(c)) {
            c = SkipWhitespace();
            break;
        }
        if(c == '/' || c == '>') break;
        AppendTagName(c);

        switch(tag_name.size()) {
        case 3:
            if(equal(&tag_name[0],"!--",3))  {
                c_scan = &tag_scanner::ScanComment;
                return wstCommentStart;
            }
            break;
        case 8:
            if( equal(&tag_name[0],"![CDATA[",8) ) {
                c_scan = &tag_scanner::ScanCdata;
                return wstCDataStart;
            }
            break;
        case 7:
            if( equal(&tag_name[0],"!ENTITY",8) ) {
                c_scan = &tag_scanner::ScanEntityDecl;
                return wstEntityStart;
            }
            break;
        default:
            if ( tag_name[0] == '?' ) {
                c_scan = &tag_scanner::ScanPi;
                return wstPiStart;
            }
            break;
        }

        c = GetChar();
    }

    if(c == 0) return wstError;

    if(is_tail) {
        if(c == '>') return wstTagEnd;
        return wstError;
    } else
        step_back();

    c_scan = &tag_scanner::ScanHead;
    return wstTagStart;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	scanner_token tag_scanner::ScanEntity()
///
/// @brief  Scans the entity (\&-based encoding).
/// @return	resulting character.
////////////////////////////////////////////////////////////////////////////////////////////////////
char tag_scanner::ScanEntity()
{
    char buf[32];
    int i = 0;
    char t;
    char ret;
    for(; i < 31 ; ++i ) {
        t = GetChar();
        if(t == 0) return wstEof;
        if( !isalnum(t) ) {
            step_back();
            break; // appears a erroneous entity token.
            // but we try to use it.
        }
        buf[i] = char(t);
        if(t == ';')
            break;
    }
    buf[i] = 0;
    ret = 0;
    if(i == 2) {
        if(equal(buf,"gt",2)) ret = '>';
        if(equal(buf,"lt",2)) ret = '<';
        if (ret) {
            GetChar(); // take finalizing ';'
            return ret;
        }
    } else if(i == 3 && equal(buf,"amp",3)) {
        GetChar(); // take finalizing ';'
        return '&';
    } else if(i == 4) {
        if(equal(buf,"apos",4)) ret = '\'';
        if(equal(buf,"quot",4)) ret = '\"';
        if (ret) {
            GetChar(); // take finalizing ';'
            return ret;
        }
    }
    t = resolve_entity(buf,i);
    if(t) {
        GetChar(); // take finalizing ';'
        return t;
    }
    // no luck ...
    AppendValue('&');
    for(int n = 0; n < i; ++n)
        AppendValue(buf[n]);
    return GetChar(); // return erroneous symbol, or finalizing ';'
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	scanner_token tag_scanner::ScanEntityDecl()
///
/// @brief  Scans the cdata.
/// @retval	current token.
////////////////////////////////////////////////////////////////////////////////////////////////////
scanner_token tag_scanner::ScanEntityDecl()
{
    if(got_tail) {
        c_scan = &tag_scanner::ScanBody;
        got_tail = false;
        return wstEntityEnd;
    }
    char t;
    unsigned int tc = 0;
    for(;;) { //value_length = 0; value_length < (MAX_TOKEN_SIZE - 1); ++value_length)
        t = GetChar();
        if( t == 0 ) return wstEof;
        value.push_back(t);
        if(t == '\"') tc++;
        else if( t == '>' && (tc & 1) == 0 ) {
            got_tail = true;
            break;
        }
    }
    return wstData;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	scanner_token tag_scanner::SkipWhitespace()
///
/// @brief  Skip whitespaces.
/// @return first non-whitespace char
////////////////////////////////////////////////////////////////////////////////////////////////////
char tag_scanner::SkipWhitespace()
{
    while(char c = GetChar()) {
        if(!IsWhitespace(c)) return c;
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void tag_scanner::step_back()
///
/// @brief	Moves one character back in the stream.
////////////////////////////////////////////////////////////////////////////////////////////////////
void tag_scanner::step_back()
{
    input.step_back();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	char tag_scanner::GetChar()
///
/// @brief  Gets the char from the input stream.
/// @return The next input char.
////////////////////////////////////////////////////////////////////////////////////////////////////
char tag_scanner::GetChar()
{
    return input.get_char();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	bool tag_scanner::IsWhitespace(char c)
///
/// @brief  Query if 'c' is whitespace.
/// @param	c - The input character.
/// @retval	true if whitespace, @e false if not.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool tag_scanner::IsWhitespace(char c)
{
    return c <= ' '
           && (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f');
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void tag_scanner::AppendValue(char c)
///
/// @brief	Appends a value.
/// @param	c - The char to append to the value.
////////////////////////////////////////////////////////////////////////////////////////////////////
void tag_scanner::AppendValue(char c)
{
    value.push_back(c);
//     if(value_length < (MAX_TOKEN_SIZE - 1))
//         value[value_length++] = c;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void tag_scanner::AppendAttrName(char c)
///
/// @brief  Appends an attribute name.
/// @param	c - The char to append to the attribute name.
////////////////////////////////////////////////////////////////////////////////////////////////////
void tag_scanner::AppendAttrName(char c)
{
    attr_name.push_back((char)c);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void tag_scanner::AppendTagName(char c)
///
/// @brief  Appends a tag name.
/// @param	c - The char to append to the tag name.
////////////////////////////////////////////////////////////////////////////////////////////////////
void tag_scanner::AppendTagName(char c)
{
    tag_name.push_back((char)c);
}

} // namespace webEngine
