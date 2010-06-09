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

#include "weStrings.h"
#include "boost/regex.hpp"

using namespace boost;

namespace webEngine {

StringLinks::StringLinks( string sep /*= "="*/, string delim /*= "\n\r"*/ )
{
    separator = sep;
    delimiter = delim;
}

void StringLinks::Parse( string data, string sep /*= ""*/, string delim /*= ""*/ )
{
    string process;
    string head;
    string k, v;
    boost::smatch dlRes, spRes;

    if (delim.empty()) {
        delim = delimiter;
    }
    if (sep.empty()) {
        sep = separator;
    }
    clear();

    string dlRexp = string("(.*?)") + delim + string("(.*)");
    regex dl(dlRexp);
    string spRexp = string("(.*?)") + sep + string("(.*)");
    regex sp(spRexp);

    process = data;
    while(regex_match(process, dlRes, dl)) {
        head = dlRes[1];
        process = dlRes[2];
        if (regex_match(head, spRes, sp)) {
            k = spRes[1];
            v = spRes[2];
        }
        else {
            k = head;
            v = head;
        }
		data_.push_back(std::make_pair(k, v));
    }
    if (!process.empty()) {
        if (regex_match(process, spRes, sp)) {
            k = spRes[1];
            v = spRes[2];
        }
        else {
            k = head;
            v = head;
        }
		data_.push_back(std::make_pair(k, v));
    }
}

string StringLinks::Compose( string sep /*= ""*/, string delim /*= ""*/ )
{
    if (delim.empty()) {
        delim = delimiter;
    }
    if (sep.empty()) {
        sep = separator;
    }

	string retval;
	data_list::const_iterator it = data_.begin();
	if (it != data_.end())
	{
		retval = it->first + sep + it->second;
		++it;

		for (; it != data_.end(); ++it)
			retval += delim + it->first + sep + it->second;
        }
    return retval;
    }

std::string StringLinks::find_first(const string& name)
{
	for (data_list::const_iterator it = data_.begin(); it != data_.end(); ++it)
		if (it->first == name)
			return it->second;

	return std::string();
}

void StringLinks::append( std::string key, std::string val )
{
    for (data_list::iterator it = data_.begin(); it != data_.end(); ++it) {
        if (it->first == key) {
            it->second = val;
            return;
        }
    }
    // if not found
    data_.push_back(std::pair<std::string, std::string>(key, val));

}

string SListToString(string_list& lst)
{
    string retval;

    for (size_t i = 0; i < lst.size(); i++)
    {
        retval += lst[i];
        retval += '\x02';
    }
    // ??? remove tailing \x02
    return retval;
}

string_list* StringToSList(const string& lst)
{
    string_list* retval = new string_list;
    string parse = lst;
    size_t pos;

    pos = parse.find('\x02');
    while(pos != string::npos) {
        string val = parse.substr(0, pos);
        retval->push_back(val);
        if (pos < parse.length())
        {
            parse = parse.substr(pos+1);
        }
        else {
            parse = "";
        }
        pos = parse.find('\x02');
    }
    if (parse != "")
    {
        retval->push_back(parse);
    }
    return retval;
}

} // namespace webEngine
