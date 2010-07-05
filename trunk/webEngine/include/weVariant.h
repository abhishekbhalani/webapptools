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
#include <boost/operators.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>
#include <boost/blank.hpp>
#include <boost/serialization/variant.hpp>

using namespace std;

namespace webEngine {

    typedef boost::variant< char, int, bool, double, string, boost::blank > we_types;
    class we_variant : public boost::less_than_comparable<we_variant, boost::equivalent< we_variant > >
    {
    public:
        // construct/copy/destruct
        we_variant() : wvalue(boost::blank()) {}
        we_variant(const we_variant &t) { wvalue = t.wvalue; }
        template<typename T> we_variant(const T &t) : wvalue(t) {}

        we_variant& operator=(const we_variant &cpy) { wvalue = cpy.wvalue; return *this; }
        template<typename T> we_variant& operator=(const T &t) { wvalue = t; return *this; }

        template<typename U> void operator==(const U &rhl) const  {}
        friend bool operator<(const we_variant& lhs, const we_variant& rhs);

        template<typename T> T get() const { return boost::get<T>(wvalue); }
        const bool empty() const { return wvalue.type() == typeid(boost::blank); }
        void clear() { wvalue = boost::blank(); }
        int which() const { return wvalue.which(); }
        const std::type_info& type() const { return wvalue.type(); }
        operator const we_types&() const { return (we_types&)wvalue; }
#ifndef __DOXYGEN__
        template < BOOST_TEMPLATED_STREAM_ARGS(E,T) >  friend
        BOOST_TEMPLATED_STREAM(ostream, E,T)& operator<< (BOOST_TEMPLATED_STREAM(ostream, E,T)& out, const we_variant &rhs);
#else
        friend std::ostream& operator<< (std::ostream& out, const we_variant &rhs);
#endif

    protected:
        we_types wvalue;
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & BOOST_SERIALIZATION_NVP(wvalue);
        }
    };

#ifndef __DOXYGEN__
    template < BOOST_TEMPLATED_STREAM_ARGS(E,T) >
    BOOST_TEMPLATED_STREAM(ostream, E,T)& operator<< (BOOST_TEMPLATED_STREAM(ostream, E,T)& out, const we_variant &rhs) {
        out << rhs.wvalue;
        return out;
    }
#endif
} // namespace webEngine

namespace boost {
namespace serialization {
    template<class Archive>

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn inline void serialize( Archive & ar, boost::blank & t, const unsigned int file_version )
    ///
    /// @brief  Implements non-intrusive serialization for boost::blank class.
    ///
    /// @author A. Abramov
    /// @date   22.06.2010
    ///
    /// @param [in,out] ar      The archive. 
    /// @param [in,out] t       The serialized object. 
    /// @param  file_version    The file version. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    inline void serialize( Archive & ar, boost::blank & t, const unsigned int file_version )
    {
        string value = "{blank}";
        ar & BOOST_SERIALIZATION_NVP(value);
    }
}
}

BOOST_CLASS_TRACKING(webEngine::we_variant, boost::serialization::track_never)
BOOST_CLASS_TRACKING(boost::blank, boost::serialization::track_never)

#endif // __WEVARIANT_H__
