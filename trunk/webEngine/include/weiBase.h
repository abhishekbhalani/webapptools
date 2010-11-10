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
#ifndef __WEIBASE_H__
#define __WEIBASE_H__

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/variant.hpp>
#include <boost/serialization/variant.hpp>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <stdexcept>
#include "weLogger.h"

using namespace std;

namespace webEngine {

typedef boost::archive::xml_oarchive we_oarchive;
typedef boost::archive::xml_iarchive we_iarchive;

#define DECLARE_SERIALIZATOR \
    friend class ::boost::serialization::access; \
    template<class Archive> \
    void serialize(Archive & ar, const unsigned int version)
// #ifndef __DOXYGEN__
// #endif
#define DECLARE_SERIAL_SAVE \
    template<class Archive> \
    void save(Archive & ar, const unsigned int version) const

#define DECLARE_SERIAL_LOAD \
    template<class Archive> \
    void load(Archive & ar, const unsigned int version)

#define DECLARE_SERIAL_BASE friend class boost::serialization::access;


template<class _Key, class _Val>
class orderedmap {
public:
    typedef typename std::vector<_Key> keyvector;
    typedef typename std::map<_Key, _Val> valuesmap;
    typedef typename std::pair<_Key, _Val> valuespair;
    typedef typename valuesmap::value_type value_type;
    template <class _Key, class _Val>
    class __iterator {
    public:
        __iterator() {
            parent = NULL;
            pos = -1;
        }
        __iterator(orderedmap<_Key, _Val>* prnt, int pos_) {
            parent = prnt;
            pos = pos_;
        }
        valuespair operator*() {
            return valuespair(parent->keys[pos], parent->values[parent->keys[pos]]);
        }
        valuespair operator->() {
            return valuespair(parent->keys[pos], parent->values[parent->keys[pos]]);
        }
        __iterator<_Key, _Val>& operator++() {
            if(parent != NULL && pos < parent->keys.size()) ++pos;
            return *this;
        }
        const bool operator==(const __iterator<_Key, _Val>& rhs) {
            return (parent == rhs.parent && pos == rhs.pos);
        }
        const bool operator!=(const __iterator<_Key, _Val>& rhs) {
            return !operator==(rhs);
        }
        __iterator<_Key, _Val>& operator=(const __iterator<_Key, _Val>& rhs) {
            parent = rhs.parent;
            pos = rhs.pos;
            return *this;
        }
    protected:
        size_t pos;
        orderedmap<_Key, _Val>* parent;
    };
    typedef typename __iterator<_Key, _Val> iterator;

public:
    orderedmap()
        : values(), keys(0) {
        // construct empty map from defaults
        values.clear();
        keys.clear();
    };

    int erase(const _Key& _Keyval) {
        values.erase(_Keyval);
        keyvector::iterator rm = ::find(keys.begin(), keys.end(), _Keyval);
        keys.erase(rm);
        return 0;
    }

    int erase(const int pos) {
        if (pos >=0 && pos < keys.size()) {
            values.erase(keys[pos]);
            keyvector::iterator rm = keys.begin() + pos;
            keys.erase(rm);
        }
        return 0;
    }

    void clear() {
        values.clear();
        keys.clear();
    };

    size_t size() {
        return keys.size();
    }

    iterator begin() {
        return iterator(this, 0);
    }
    iterator end() {
        return iterator(this, keys.size());
    };

    iterator find(const _Key& _Keyval) {
        // returns the iterator for the given Keyval
        keyvector::iterator it = ::find(keys.begin(), keys.end(), _Keyval);
        if(it != keys.end()) {
            return iterator(this, it - keys.begin());
        } else {
            return iterator(this, keys.size());
        }

    }

    _Val& operator[](const _Key& _Keyval) {
        // find element matching _Keyval or insert with default mapped
        keyvector::iterator _Where = ::find(keys.begin(), keys.end(), _Keyval);
        if (_Where == keys.end()) {
            keys.push_back(_Keyval);
            values[_Keyval] = _Val();
        }
        return  (_Val&)values[_Keyval];
    }

protected:
    valuesmap   values;
    keyvector   keys;
};

} // namespace webEngine

#endif //__WEIBASE_H__
