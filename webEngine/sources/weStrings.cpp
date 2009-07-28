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

#include "weStrings.h"
#include "boost/regex.hpp"

using namespace boost;

WeStringLinks::WeStringLinks( string sep /*= "="*/, string delim /*= "\n\r"*/ ) :
    WeLinkedList<string, string>()
{
    separator = sep;
    delimiter = delim;
}

WeStringLinks::WeStringLinks( WeStringLinks& lst )
{
    WeLinkedString  *obj;

    data = NULL;
    curr = NULL;
    separator = lst.separator;
    delimiter = lst.delimiter;
    obj = lst.First();
    data = new WeLinkedString(*obj);
    curr = data;
    obj = lst++;
    while (obj != NULL) {
        curr->Add(new WeLinkedString(*obj));
        curr = curr->Next();
        obj = lst++;
    }
}

void WeStringLinks::Parse( string data, string sep /*= ""*/, string delim /*= ""*/ )
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
    Clear();

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
        Append(k, v);
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
        Append(k, v);
    }
}

string& WeStringLinks::Compose( string sep /*= ""*/, string delim /*= ""*/ )
{
    WeLinkedString* elem = data;
    string *retval = new string("");

    if (delim.empty()) {
        delim = delimiter;
    }
    if (sep.empty()) {
        sep = separator;
    }
    while (elem) {
        *retval += elem->Key() + sep + elem->Value();
        if (elem->Next() != NULL) {
            *retval += delim;
        }
        elem = elem->Next();
    }
    return *retval;
}
