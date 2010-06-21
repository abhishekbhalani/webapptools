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
#ifndef __WEVARIANT_H__
#define __WEVARIANT_H__

#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>
#include <boost/blank.hpp>
#include <boost/serialization/variant.hpp>

using namespace std;

namespace webEngine {

    typedef boost::variant< char, int, bool, double, string, boost::blank > we_types;
    class we_variant : public we_types
    {
    public:
        // construct/copy/destruct
        we_variant() : we_types(boost::blank()) {}
        we_variant(const we_variant &t) { we_types::operator=(*(static_cast<const we_types*>(&t))); }
        template<typename T> we_variant(const T &t) : we_types(t) {}

        we_variant& operator=(const we_variant &cpy) { we_types::operator=(*(static_cast<const we_types*>(&cpy))); return *this; }
        template<typename T> we_variant& operator=(const T &t) { we_types::operator=(t); return *this; }

        bool operator==(const we_variant &rhl) const {return we_types::operator==(*(static_cast<const we_types*>(&rhl))); }
        template<typename U> void operator==(const U &rhl) const  {}
        bool operator<(const we_variant &rhl) const;
        bool operator>(const we_variant &rhl) const;
        bool operator<=(const we_variant &rhl) const;
        bool operator>=(const we_variant &rhl) const;

        const bool empty() { return type() == typeid(boost::blank); }
        void clear() { we_types::operator=(boost::blank()); }
    protected:
        BOOST_SERIALIZATION_SPLIT_MEMBER()
        friend class ::boost::serialization::access;
        template<class Archive>
        void save(Archive & ar, const unsigned int version) const {
            int vtype = which();
            string value = boost::lexical_cast<string>(*this);
            ar & BOOST_SERIALIZATION_NVP(vtype);
            ar & BOOST_SERIALIZATION_NVP(value);
        }
        template<class Archive>
        void load(Archive & ar, const unsigned int version) {
            int vtype;
            string value;
            ar & BOOST_SERIALIZATION_NVP(vtype);
            ar & BOOST_SERIALIZATION_NVP(value);
            switch (vtype)
            {
            case 0:
                *this = boost::lexical_cast<char>(value);
                break;
            case 1:
                *this = boost::lexical_cast<int>(value);
                break;
            case 2:
                *this = boost::lexical_cast<bool>(value);
                break;
            case 3:
                *this = boost::lexical_cast<double>(value);
                break;
            case 4:
                *this = value;
                break;
            default:
                clear();
            }
        }
    };
} // namespace webEngine
BOOST_CLASS_TRACKING(webEngine::we_variant, boost::serialization::track_never)

#endif // __WEVARIANT_H__
