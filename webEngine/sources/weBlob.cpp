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
#include <iostream>
#include "weBlob.h"

using namespace webEngine;

// Read BLOB data from a stream.
bool blob::read(istream& file)
{
    if (file.bad())
    {
        return false;
    }

    size_t cb = 0;
    file.read((char*)&cb, streamsize(4));
    if (file.eof() || file.bad() || cb == 0) {
        return false;
    }
    //if (cb>65536) return false;
    resize(cb);
    file.read((char*)&(*this)[0], cb);
    return (!file.bad());
}

// Write BLOB data to the stream.
bool blob::write(ostream& file)
{
    size_t cb = size();
    file.write((const char*)&cb, 4);
    if (file.bad())
    {
        return false;
    }
    file.write((const char*)&(*this)[0], cb);
    return (!file.bad());
}

template<class Archive>
void blob::serialize(Archive &ar, const unsigned int version)
{
//    ar & (std::vector*)this;
}

boost::shared_ptr<tag_stream> blob::stream()
{
    return boost::shared_ptr<tag_stream>(new blob_stream(*this));
}
