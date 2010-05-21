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


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  linked_list_elem
    ///
    /// @brief  Element of the linked list of objects.
    ///
    /// @author A. Abramov
    /// @date   27.05.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename _Key, typename _Val>
    class linked_list_elem
    {
    public:
        linked_list_elem() :
          first(), second()
          {
              nxt = NULL;
          };

          linked_list_elem( linked_list_elem<_Key, _Val> &elem ) :
          first(), second()
          {
              first = elem.first;
              second = elem.second;
              nxt = NULL;
          };

          ~linked_list_elem()
          {
              nxt = NULL;
          }

          void add(linked_list_elem<_Key, _Val> *elem)
          {
              linked_list_elem<_Key, _Val> *curr = nxt;
              nxt = elem;
              elem->nxt = curr;
          }


          void link(linked_list_elem<_Key, _Val> *elem)   { nxt = elem; };
          linked_list_elem<_Key, _Val>* next() {return nxt;};

          // Access the key
          const _Key &key(void) const   { return(first);  };
          void key(const _Key &elem)    { first = elem;   };

          // Access the value
          const _Val &value(void) const   { return(second);   };
          void value(const _Val &elem)    { second = elem;    };

    protected:
        _Key    first;
        _Val    second;
        linked_list_elem<_Key, _Val> *nxt;

    private:
        DECLARE_SERIAL_BASE;
        DECLARE_SERIAL_SAVE
        {
            bool last = (nxt == NULL);
            ar & BOOST_SERIALIZATION_NVP(first);
            ar & BOOST_SERIALIZATION_NVP(second);
            ar & BOOST_SERIALIZATION_NVP(last);
            if (! last) {
                ar & BOOST_SERIALIZATION_NVP(nxt);
            };
        };
        DECLARE_SERIAL_LOAD
        {
            bool last;
            ar & BOOST_SERIALIZATION_NVP(first);
            ar & BOOST_SERIALIZATION_NVP(second);
            ar & BOOST_SERIALIZATION_NVP(last);
            if (!last) {
                nxt = new linked_list_elem<_Key, _Val>;
                ar & BOOST_SERIALIZATION_NVP(nxt);
            }
        };
        BOOST_SERIALIZATION_SPLIT_MEMBER();
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  linked_list
    ///
    /// @brief  List of linked objects.
    ///
    /// @author A. Abramov
    /// @date   10.06.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename _Key, typename _Val>
    class linked_list
    {
    public:
        linked_list()
        {
            data = NULL;
            curr = NULL;
        };
        linked_list(linked_list<_Key, _Val>& lst)
        {
            linked_list_elem<_Key, _Val> *obj;

            data = NULL;
            curr = NULL;
            obj = lst.First();
            data = new linked_list_elem<_Key, _Val>(*obj);
            curr = data;
            obj = lst++;
            while (obj != NULL) {
                curr->add(new linked_list_elem<_Key, _Val>(*obj));
                curr = curr->next();
                obj = lst++;
            }
        };
        virtual ~linked_list()
        {
            clear();
            data = NULL;
            curr = NULL;
        };

        virtual _Val& find_first(_Key name)
        {
            linked_list_elem<_Key, _Val>* elem = curr;

            curr = data;
            while (curr != NULL) {
                if (curr->key() == name) {
                    break;
                }
                curr = curr->next();
            }
            if (curr != NULL) {
                return (_Val&)(curr->value());
            }
            curr = elem;
            return empty_val;
        };

        virtual _Val& find_next(_Key name)
        {
            linked_list_elem<_Key, _Val>* elem = curr;

            if (curr != NULL) {
                curr = curr->next();
            }
            while (curr != NULL) {
                if (curr->key() == name) {
                    break;
                }
                curr = curr->next();
            }
            if (curr != NULL) {
                return (_Val&)(curr->value());
            }
            curr = elem;
            return empty_val;
        };

        void append(_Key name, _Val value)
        {
            linked_list_elem<_Key, _Val>* elem;
            linked_list_elem<_Key, _Val>* obj;

            obj = new linked_list_elem<_Key, _Val>();
            obj->key(name);
            obj->value(value);
            if (data == NULL) {
                data = obj;
                curr = data;
            }
            else {
                elem = curr;
                last();
                curr->add(obj);
                curr = elem;
            }
        };
        void insert(_Key name, _Val value)
        {
            linked_list_elem<_Key, _Val>* obj;

            obj = new linked_list_elem<_Key, _Val>();
            obj->key(name);
            obj->value(value);
            if (data == NULL) {
                data = obj;
                curr = data;
            }
            else {
                curr->add(obj);
            }
        };
        void clear()
        {
            curr = data;
            while (curr != NULL) {
                curr = curr->next();
                delete data;
                data = curr;
            }
        };

        linked_list_elem<_Key, _Val>* first() { curr = data; return (curr); };
        linked_list_elem<_Key, _Val>* last()
        {
            if (curr == NULL) {
                return curr;
            }
            while (curr->next() != NULL) {
                curr = curr->next();
            }
            return curr;
        };
        linked_list_elem<_Key, _Val>* current() { return curr;  };
        void erase()
        {
            linked_list_elem<_Key, _Val>* obj;

            if (curr == NULL) {
                return;
            }
            obj = data;
            while (obj != NULL) {
                if (obj->next() == curr) {
                    break;
                }
                obj = obj->next();
            }
            if (obj == NULL) {
                return;
            }
            obj->Link(curr->next());

        };
        linked_list_elem<_Key, _Val>* operator++(int) { if(curr != NULL) { curr = curr->next(); } return curr;    };

    protected:
#ifndef __DOXYGEN__
        linked_list_elem<_Key, _Val>* data;
        linked_list_elem<_Key, _Val>* curr;
        _Val empty_val;
#endif //__DOXYGEN__
    private:
        DECLARE_SERIAL_BASE;
        DECLARE_SERIAL_SAVE
        {
            bool last = (data == NULL);
            ar & BOOST_SERIALIZATION_NVP(last);
            if (!last) {
                ar & BOOST_SERIALIZATION_NVP(data);
            };
        };
        DECLARE_SERIAL_LOAD
        {
            bool last;
            ar & BOOST_SERIALIZATION_NVP(last);
            if (! last) {
                data = new linked_list_elem<_Key, _Val>;
                ar & BOOST_SERIALIZATION_NVP(data);
            }
            else {
                data = NULL;
            }
            curr = data;
        };
        BOOST_SERIALIZATION_SPLIT_MEMBER();
    };

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
