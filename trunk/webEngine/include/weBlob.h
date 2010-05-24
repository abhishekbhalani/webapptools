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
#ifndef __WEBLOB_H__
#define __WEBLOB_H__
#include <vector>
#include "weTagScanner.h"
#include <boost/shared_ptr.hpp>

using namespace std;

namespace webEngine {

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  blob
///
/// @brief  BLOB manipulations 
///
/// Specialization of the std::vector to store unstructured data.
/// @author A. Abramov
/// @date   27.05.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class blob : public vector<unsigned char>
{
public:
    blob() {}
    ~blob() {}
    explicit blob(unsigned int cb) : vector<unsigned char>(cb, 0) {}    
    explicit blob(char* s) : vector<unsigned char>(s, s + strlen(s) + 1) {}
    blob(unsigned char* p, unsigned int cb) : vector<unsigned char>(p, p + cb) {}

    void zero()                     { vector<unsigned char>::assign(size(), 0); }
    void assign(unsigned char* p,
        unsigned int cb)            { resize(cb); vector<unsigned char>::assign(p, p+cb); }
    void assign(const blob& b)    { vector<unsigned char>::assign(b.begin(), b.end()); }
    void assign(char* s)            { vector<unsigned char>::assign(s, s + strlen(s)+1); }
    bool read(istream& file);
    bool write(ostream& file);
	boost::shared_ptr<tag_stream> stream();
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  blob_stream
///
/// @brief  BLOB stream for WeTagScanner.
///
/// This class translates blob to the tag_stream for parsing.
///
/// @author	A. Abramov
/// @date   02.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class blob_stream : public tag_stream_impl<blob::const_iterator>
{
public:
    blob_stream(const blob& src) : tag_stream_impl<blob::const_iterator>(src.begin(), src.end()) {}
    ~blob_stream() {}
};

} // namespace webEngine

#endif //__WEBLOB_H__
