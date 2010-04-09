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
#ifndef __TAGSCANNER_H__
#define __TAGSCANNER_H__

/**
@file	include\weTagScanner.h

@brief	Declares the weTagScanner class.

This file is based on the xh_scanner.h (published under the BSD license) and
implements code of the @e xh_scanner as the class. Original author of the
xh_scanner is Andrew Fedoniouk @ terrainformatica.com. And that code was
designed as part of HTMLayout SDK (http://terrainformatica.com/htmlayout).
The copy of the code was obtained from CodeProject.com
(http://www.codeproject.com/KB/recipes/HTML_XML_Scanner.aspx)

@author A. Abramov
@date 26.05.2009.
@example tagscanner.cpp
*/

#include <string>
#include <vector>
#include <stdlib.h> // wchar_t
#include <string.h> // strcmp, etc.

using namespace std;

namespace webEngine {

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class	InStream
    ///
    /// @brief  TagScanner's input stream.
    ///
    /// This is the base class for input data into the TagScanner. This class is abstract and can't
    /// be used directly. Inherit this class and override the GetChar function to implement your own
    /// input stream.
    ///
    /// @author	A. Abramov
    /// @date	26.05.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class InStream {
    public:
        virtual char GetChar() = 0;
        virtual void PushBack(char c) = 0;
        virtual size_t GetPos() = 0;
        virtual char* GetFrom(int form) = 0;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class	StrStream
    ///
    /// @brief	TagScanner's input string.
    ///
    /// This class implements input stream based on string.
    ///
    /// @author	A. Abramov
    /// @date	26.05.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class StrStream : public InStream
    {
        char* p;
        const char* start;
        const char* end;
    public:
        StrStream(const char* src): p((char*)src), start(src), end(src + strlen(src)) {}
        virtual char GetChar() { return p < end? *p++: 0; }
        virtual void PushBack(char c) {if(p > start) {p--;}}
        virtual size_t GetPos() { return p - start; };
        virtual char* GetFrom(int form) { return (char*)(start + form); };
    };

    enum ScannerToken {
        wstError = -1,
        wstEof = 0,
        wstTagStart,
        wstTagEnd,
        wstAttr,
        wstWord,
        wstSpace,
        wstData,
        wstCommentStart, wstCommentEnd, // after "<!--" and "-->"
        wstCDataStart, wstCDataEnd,     // after "<![CDATA[" and "]]>"
        wstPiStart, wstPiEnd,           // after "<?" and "?>"
        wstEntityStart, wstEntityEnd    // after "<!ENTITY" and ">"
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class	TagScanner
    ///
    /// @brief	TAG scanner for parsing markup languages (XML or HTML).
    ///
    /// This class implements operations to parsing the input stream for tag-based markup (i.e. HTML).
    /// @author	Aabramov
    /// @date	26.05.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class TagScanner {
    public:

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @fn	TagScanner(InStream& is)
        /// @brief  Constructor.
        /// @param  is - the input stream to parse.
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        TagScanner(InStream& is):
          input(is),
              input_char(0),
              //            value_length(0),
              //            tag_name_length(0),
              //            attr_name_length(0),
              got_tail(false) { c_scan = &TagScanner::ScanBody; }
          virtual ~TagScanner(){};

          ////////////////////////////////////////////////////////////////////////////////////////////////////
          /// @fn	ScannerToken GetToken()
          ///
          /// @brief	Gets the next token from stream.
          /// @return	The token.
          ////////////////////////////////////////////////////////////////////////////////////////////////////
          ScannerToken    GetToken() { return (this->*c_scan)(); }
          const char*       GetValue();
          const char*       GetAttrName();
          const char*       GetTagName();

          ////////////////////////////////////////////////////////////////////////////////////////////////////
          /// @fn	virtual wchar_t ResolveEntity(const char* buf, int buf_size)
          ///
          /// @brief    Resolve entity.
          /// @param    buf       - If non-null, the buffer.
          /// @param    buf_size  - Size of the buffer.
          ///
          /// @return   resulting char.
          ////////////////////////////////////////////////////////////////////////////////////////////////////
          virtual char   ResolveEntity(const char* buf, int buf_size) { return 0; }
          virtual size_t GetPos() { return input.GetPos(); };
          virtual void   PushBack(char c);
          virtual char*  GetFrom(int from) {return input.GetFrom(from);};
          virtual void   ResetToBody() {c_scan = &TagScanner::ScanBody;};

    private:
        /* types */
        static const int MAX_TOKEN_SIZE = 1024;
        static const int MAX_NAME_SIZE = 128;

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @typedef ScannerToken (TagScanner::*fnScan)()
        ///
        ///
        /// @brief	WebEngine scanner token parser function.
        ///
        /// This type defines pointer to the scanner function. Used to switch scanner function to parse
        /// different regions of the source file.
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        typedef ScannerToken (TagScanner::*fnScan)();
        fnScan        c_scan; // current 'reader'

        /* methods */
        // content 'readers'
        ScannerToken  ScanBody();
        ScannerToken  ScanHead();
        ScannerToken  ScanComment();
        ScannerToken  ScanCdata();
        ScannerToken  ScanPi();
        ScannerToken  ScanTag();
        ScannerToken  ScanEntityDecl();

        char            SkipWhitespace();
        char            GetChar();
        char            ScanEntity();

        bool            IsWhitespace(char c);

        void            AppendValue(char c);
        void            AppendAttrName(char c);
        void            AppendTagName(char c);

        /* data */
#ifndef __DOXYGEN__
        ScannerToken  token;

        vector<char>    value;
        //int             value_length;

        vector<char>    tag_name;
        //int             tag_name_length;

        vector<char>    attr_name;
        //int             attr_name_length;

        InStream&     input;              ///< input stream for scanning
        char            input_char;         ///< current input char

        bool            got_tail;           ///< aux flag used in scan_comment, etc.
#endif //__DOXYGEN__
    };

} // namespace webEngine

#endif //__TAGSCANNER_H__
