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
    class orderedmap
    {
    public:
        typedef typename std::vector<_Val> valvector;
        typedef typename std::vector<_Val>::iterator iterator;
        typedef typename std::map<_Key, int> mapkeys;
        typedef typename mapkeys::value_type value_type;

    public:
        orderedmap()
            : values(), keys()
        {	// construct empty map from defaults
        };

        int erase(const _Key& _Keyval)
        {   // erase element at given Keyval
            //         valvector::iterator it = keys[_Keyval];
            //         values.erase(it);
            return (keys.erase(_Keyval));
        }

        void clear() { 
            values.clear();
            keys.clear();
        };

        iterator begin() {return values.begin(); };
        iterator end() {return values.end(); };

        iterator find(const _Key& _Keyval)
        {   // returns the iterator for the given Keyval
            typename mapkeys::iterator it = keys.find(_Keyval);
            if(it != keys.end()) {
                return (values.begin() + it->second);
            }
            else {
                return values.end();
            }

        }

        _Key& key(iterator& _Where)
        {   // gives key for current iterator
            typename mapkeys::iterator it;
            for(it = keys.begin(); it != keys.end(); it++) {
                if ((values.begin() + it->second) == _Where) {
                    return (_Key&)it->first;
                }
            }
            return *(new _Key());
        };

        _Val& val(iterator& _Where)
        {   // gives value for current iterator
            return *_Where;
        };

        _Val& operator[](const _Key& _Keyval)
        {	// find element matching _Keyval or insert with default mapped
            typename mapkeys::iterator _Where = keys.find(_Keyval);
            if (_Where == keys.end()) {
                values.push_back(_Val());
                int ins = values.size() - 1;
                _Where = keys.insert(_Where, value_type(_Keyval, ins));
            }
            return  (_Val&)values[_Where->second];
        }

    protected:
        valvector   values;
        mapkeys     keys;
    };

} // namespace webEngine

#endif //__WEIBASE_H__
