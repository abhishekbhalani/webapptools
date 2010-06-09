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
*/

#include <string>
#include <vector>
#include <cassert>
#include <stdlib.h> // wchar_t
#include <string.h> // strcmp, etc.
#include <boost/numeric/conversion/cast.hpp>

using namespace std;

namespace webEngine {

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class	tag_stream
    ///
    /// @brief  tag_scanner's input stream.
    ///
    /// This is the base class for input data into the tag_scanner. This class is abstract and can't
    /// be used directly. Inherit this class and override the get_char function to implement your own
    /// input stream.
    ///
    /// @author	A. Abramov
    /// @date	26.05.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
	class tag_stream {
	public:
        virtual ~tag_stream() {}
		virtual char get_char() = 0;
        virtual void step_back() = 0;
        virtual size_t get_pos() const = 0;
        virtual const char* get_from(int form) const = 0;
        virtual size_t size() const = 0;
	};

	template <typename iterator>
	class tag_stream_impl : public tag_stream {
    public:
		tag_stream_impl(iterator begin, iterator end) :
			begin_(begin),
			end_(end),
			current_(begin)
		{
		}

        virtual char get_char()
		{
			return current_ == end_ ? 0 : *current_++;
		}

        virtual void step_back()
		{
			assert(current_ != begin_);
			if (current_ != begin_)
				--current_;
		}

        virtual size_t get_pos() const
		{
			return std::distance(begin_, current_);
		}

        virtual const char* get_from(int from) const
		{
			assert(std::distance(begin_, end_) > boost::numeric_cast<typename iterator_traits<iterator>::difference_type>(from));
			iterator it = begin_;
			std::advance(it, from);
			return reinterpret_cast<const char*>(&*it);
		}

        virtual size_t size() const
        {
            return std::distance(begin_, end_);
        }

	private:
		iterator begin_;
		iterator end_;
		iterator current_;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class	str_tag_stream
    ///
    /// @brief	tag_scanner's input string.
    ///
    /// This class implements input stream based on string.
    ///
    /// @author	A. Abramov
    /// @date	26.05.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class str_tag_stream : public tag_stream_impl<const char*>
    {
    public:
        str_tag_stream(const char* src): tag_stream_impl<const char*>(src, src + strlen(src)) {}
    };

    enum scanner_token {
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
    /// @class	tag_scanner
    ///
    /// @brief	TAG scanner for parsing markup languages (XML or HTML).
    ///
    /// This class implements operations to parsing the input stream for tag-based markup (i.e. HTML).
    /// @author	Aabramov
    /// @date	26.05.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class tag_scanner {
    public:

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @fn	tag_scanner(tag_stream& is)
        /// @brief  Constructor.
        /// @param  is - the input stream to parse.
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        tag_scanner(tag_stream& is):
          input(is),
              input_char(0),
              //            value_length(0),
              //            tag_name_length(0),
              //            attr_name_length(0),
              got_tail(false) { c_scan = &tag_scanner::ScanBody; }
          virtual ~tag_scanner(){};

          ////////////////////////////////////////////////////////////////////////////////////////////////////
          /// @fn	scanner_token get_token()
          ///
          /// @brief	Gets the next token from stream.
          /// @return	The token.
          ////////////////////////////////////////////////////////////////////////////////////////////////////
          scanner_token get_token() { return (this->*c_scan)(); }
          const char*   get_value();
          const char*   get_attr_name();
          const char*   get_tag_name();

          size_t size() { return input.size(); }

          ////////////////////////////////////////////////////////////////////////////////////////////////////
          /// @fn	virtual wchar_t resolve_entity(const char* buf, int buf_size)
          ///
          /// @brief    Resolve entity.
          /// @param    buf       - If non-null, the buffer.
          /// @param    buf_size  - Size of the buffer.
          ///
          /// @return   resulting char.
          ////////////////////////////////////////////////////////////////////////////////////////////////////
          virtual char   resolve_entity(const char* buf, int buf_size) { return 0; }
          virtual size_t get_pos() const { return input.get_pos(); };
          virtual void   step_back();
          virtual const char* get_from(int from) const {return input.get_from(from);};
          virtual void   reset_to_body() {c_scan = &tag_scanner::ScanBody;};

    private:
        /* types */
        static const int MAX_TOKEN_SIZE = 1024;
        static const int MAX_NAME_SIZE = 128;

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @typedef scanner_token (tag_scanner::*fnScan)()
        ///
        ///
        /// @brief	WebEngine scanner token parser function.
        ///
        /// This type defines pointer to the scanner function. Used to switch scanner function to parse
        /// different regions of the source file.
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        typedef scanner_token (tag_scanner::*fnScan)();
        fnScan        c_scan; // current 'reader'

        /* methods */
        // content 'readers'
        scanner_token   ScanBody();
        scanner_token   ScanHead();
        scanner_token   ScanComment();
        scanner_token   ScanCdata();
        scanner_token   ScanPi();
        scanner_token   ScanTag();
        scanner_token   ScanEntityDecl();

        char            SkipWhitespace();
        char            GetChar();
        char            ScanEntity();

        bool            IsWhitespace(char c);

        void            AppendValue(char c);
        void            AppendAttrName(char c);
        void            AppendTagName(char c);

        /* data */
#ifndef __DOXYGEN__
        scanner_token  token;

        vector<char>    value;
        //int             value_length;

        vector<char>    tag_name;
        //int             tag_name_length;

        vector<char>    attr_name;
        //int             attr_name_length;

        tag_stream&     input;              ///< input stream for scanning
        char            input_char;         ///< current input char

        bool            got_tail;           ///< aux flag used in scan_comment, etc.
#endif //__DOXYGEN__
    };

} // namespace webEngine

#endif //__TAGSCANNER_H__
