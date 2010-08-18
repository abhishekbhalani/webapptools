/*
    scanServer is the HTML processing library
    Copyright (C) 2009 Andrew Abramov stinger911@gmail.com

    This file is part of scanServer

    scanServeris free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    scanServeris distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with scanServer.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __WEVULNERS_H__
#define __WEVULNERS_H__
#include <string>
#include "weiStorage.h"

using namespace std;

namespace webEngine {

class VulnerDesc {
public:
    string  id;
    string  title;
    string  shortDesc;
    string  longDesc;
    int     severity;

    db_recordset* ToRS( const string& parentID = "" );
    void FromRS( db_recordset *rs );
    string ToXML();
    bool FromXML(const string& xml);
};

}

#endif //__WEVULNERS_H__
