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
*/
#ifndef __WEBLOB_H__
#define __WEBLOB_H__
#include <vector>
#include "weTagScanner.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  WeBlob
///
/// @brief  BLOB manipulations 
///
/// Specialization of the std::vector to store unstructured data.
/// @author A. Abramov
/// @date   27.05.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class WeBlob : public vector<unsigned char>
{
public:
    WeBlob()                        { clear(); }
    ~WeBlob()                       { clear(); }

    WeBlob(unsigned int cb)         { clear(); resize(cb); zero(); }
    WeBlob(unsigned char* p,
        unsigned int cb)            { clear(); assign(p, cb); }
    WeBlob(const WeBlob& b)         { clear(); assign(b); }
    WeBlob(char* s)                 { clear(); assign(s); }

    void zero()                     { vector<unsigned char>::assign(size(), 0); }
    void assign(unsigned char* p,
        unsigned int cb)            { resize(cb); vector<unsigned char>::assign(p, p+cb); }
    void assign(const WeBlob& b)    { vector<unsigned char>::assign(b.begin(), b.end()); }
    void assign(char* s)            { vector<unsigned char>::assign(s, s + strlen(s)+1); }
    bool read(istream& file);
    bool write(ostream& file);
    WeInStream* stream();
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  WeBlobStream
///
/// @brief  BLOB stream for WeTagScanner.
///
/// This class translates WeBlob to the WeInStream for parsing.
///
/// @author	A. Abramov
/// @date   02.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class WeBlobStream : public WeInStream
{
    const char* p;
    const char* start;
    const char* end; 
public:
    WeBlobStream(const WeBlob& src)
    {
        start = (const char*)&src.front();
        end = (const char*)&src.back();
        p = (char*)start;
    };
    virtual char GetChar() { return p < end? *p++: 0; }
    virtual void PushBack(char c) {if(p > start) {p--;}}
    virtual int GetPos() { return p - start; };
};

#endif //__WEBLOB_H__
