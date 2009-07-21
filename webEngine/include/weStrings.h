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
#ifndef __WESTRINGS_H__
#define __WESTRINGS_H__

#include <string>
#include <map>
#include <list>
#include "weiBase.h"

using namespace std;

typedef map<string, string> WeStringMap;
typedef orderedmap<string, string> WeAttrMap;
typedef vector<string> WeStringList;
typedef WeLinkedListElem<string, string> WeLinkedString;
typedef WeLinkedList<string, string> WeLinkedListStr;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  WeStringLinks
///
/// @brief  Operations with linked list of strings.
///
/// Separator and delimiter are regular expressions to process input string. Delimiter is used to
/// break string onto the parts of key-value, and separator is used to detach key from value.
///
/// @author A. Abramov
/// @date   27.05.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class WeStringLinks : public WeLinkedList<string, string>
{
public:
    WeStringLinks(string sep = "=", string delim = "\n\r");
    WeStringLinks(WeStringLinks& lst);

    void Parse(string data, string sep = "", string delim = "");
    string& Compose(string sep = "", string delim = "");

	// Access the Separator
	const string &Separator(void) const { return(separator);};
	void Separator(const string &sep)   { separator = sep;  };

    // Access the Delimiter
    const string &Delimiter(void) const { return(delimiter);    };
    void Delimiter(const string &delim) { delimiter = delim;    };

protected:
#ifndef __DOXYGEN__
    string separator;
    string delimiter;
    WeLinkedString* data;
    WeLinkedString* curr;
#endif //__DOXYGEN__
private:
    DECLARE_SERIALIZATOR
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(WeLinkedListStr);
        ar & BOOST_SERIALIZATION_NVP(separator);
        ar & BOOST_SERIALIZATION_NVP(delimiter);
    };
};

BOOST_CLASS_TRACKING(WeLinkedString, boost::serialization::track_never)
BOOST_CLASS_TRACKING(WeStringList, boost::serialization::track_never)

#endif //__WESTRINGS_H__
