/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew Abramov aabramov@ptsecurity.ru

    This file is part of webEngine

    webEngineis free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngineis distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.

    @file   difflib.cpp
    @brief  Example for weDiffLib usage.
*/
#include <iostream>
#include "weDiffLib.h"

using namespace std;
using namespace webEngine;

int main(int argc, char* argv[])
{
    CmpResults* res;

    string t1 = "This   file is part of webEngine";
    string t2 = "this file was part Of dtl";

    if (argc > 1) {
        t1 = argv[1];
    }
    if (argc > 2) {
        t2 = argv[2];
    }

    cout << "Compare '" << t1 << "' and '" << t2 << "'" << endl;
    cout << "Case sensitive comparison:" << endl;
    res = TextDiff(t1, t2, weCmpNothing);
    for (unsigned i = 0; i < res->size(); i++) {
        cout << "Block #" << i << endl;
        switch (res->at(i)->state) {
        case weCmpEqual:
            cout << "weCmpEqual: ";
            break;
        case weCmpDeleted:
            cout << "weCmpDeleted: ";
            break;
        case weCmpInserted:
            cout << "weCmpInserted: ";
            break;
        default:
            cout << "warning! unknown state (" << int(res->at(i)->state) << ")";
            break;
        };
        string data =  boost::get<string>(res->at(i)->entity);
        cout << "'" << data << "'" <<endl;
    }
    cout << endl << "Case insensitive comparison:" << endl;
    res = TextDiff(t1, t2, weCmpCaseInsens);
    for (unsigned i = 0; i < res->size(); i++) {
        cout << "Block #" << i << endl;
        switch (res->at(i)->state) {
        case weCmpEqual:
            cout << "weCmpEqual: ";
            break;
        case weCmpDeleted:
            cout << "weCmpDeleted: ";
            break;
        case weCmpInserted:
            cout << "weCmpInserted: ";
            break;
        default:
            cout << "warning! unknown state (" << int(res->at(i)->state) << ")";
            break;
        };
        string data =  boost::get<string>(res->at(i)->entity);
        cout << "'" << data << "'" <<endl;
    }
    cout << endl << "Case insensitive, collapsed spaces comparison:" << endl;
    res = TextDiff(t1, t2, (weCmpMode)(weCmpCaseInsens | weCmpCollapseSpace));
    for (unsigned i = 0; i < res->size(); i++) {
        cout << "Block #" << i << endl;
        switch (res->at(i)->state) {
        case weCmpEqual:
            cout << "weCmpEqual: ";
            break;
        case weCmpDeleted:
            cout << "weCmpDeleted: ";
            break;
        case weCmpInserted:
            cout << "weCmpInserted: ";
            break;
        default:
            cout << "warning! unknown state (" << int(res->at(i)->state) << ")";
            break;
        };
        string data =  boost::get<string>(res->at(i)->entity);
        cout << "'" << data << "'" <<endl;
    }
    return 0;
}

