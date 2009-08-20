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
/// @class  LinkedListElem
///
/// @brief  Element of the linked list of objects.
///
/// @author A. Abramov
/// @date   27.05.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename _Key, typename _Val>
class LinkedListElem
{
public:
    LinkedListElem() :
      first(), second()
      {
          next = NULL;
      };

      LinkedListElem( LinkedListElem<_Key, _Val> &elem ) :
      first(), second()
      {
          first = elem.first;
          second = elem.second;
          next = NULL;
      };

      ~LinkedListElem()
      {
          next = NULL;
      }

      void Add(LinkedListElem<_Key, _Val> *elem)
      {
          LinkedListElem<_Key, _Val> *curr = next;
          next = elem;
          elem->next = curr;
      }


      void Link(LinkedListElem<_Key, _Val> *elem)   { next = elem; };
      LinkedListElem<_Key, _Val>* Next() {return next;};

      // Access the Key
      const _Key &Key(void) const   { return(first);  };
      void Key(const _Key &elem)    { first = elem;   };

      // Access the Value
      const _Val &Value(void) const   { return(second);   };
      void Value(const _Val &elem)    { second = elem;    };

protected:
    _Key    first;
    _Val    second;
    LinkedListElem<_Key, _Val> *next;

private:
    DECLARE_SERIAL_BASE;
    DECLARE_SERIAL_SAVE
    {
        bool last = (next == NULL);
        ar & BOOST_SERIALIZATION_NVP(first);
        ar & BOOST_SERIALIZATION_NVP(second);
        ar & BOOST_SERIALIZATION_NVP(last);
        if (! last) {
            ar & BOOST_SERIALIZATION_NVP(next);
        };
    };
    DECLARE_SERIAL_LOAD
    {
        bool last;
        ar & BOOST_SERIALIZATION_NVP(first);
        ar & BOOST_SERIALIZATION_NVP(second);
        ar & BOOST_SERIALIZATION_NVP(last);
        if (!last) {
            next = new LinkedListElem<_Key, _Val>;
            ar & BOOST_SERIALIZATION_NVP(next);
        }
    };
    BOOST_SERIALIZATION_SPLIT_MEMBER();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  LinkedList
///
/// @brief  List of linked objects.
///
/// @author A. Abramov
/// @date   10.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename _Key, typename _Val>
class LinkedList
{
public:
    LinkedList()
    {
        data = NULL;
        curr = NULL;
    };
    LinkedList(LinkedList<_Key, _Val>& lst)
    {
        LinkedListElem<_Key, _Val> *obj;

        data = NULL;
        curr = NULL;
        obj = lst.First();
        data = new LinkedListElem<_Key, _Val>(*obj);
        curr = data;
        obj = lst++;
        while (obj != NULL) {
            curr->Add(new LinkedListElem<_Key, _Val>(*obj));
            curr = curr->Next();
            obj = lst++;
        }
    };
    virtual ~LinkedList()
    {
        Clear();
        data = NULL;
        curr = NULL;
    };

    virtual _Val& FindFirst(_Key name)
    {
        LinkedListElem<_Key, _Val>* elem = curr;

        curr = data;
        while (curr != NULL) {
            if (curr->Key() == name) {
                break;
            }
            curr = curr->Next();
        }
        if (curr != NULL) {
            return *(new _Val(curr->Value()));
        }
        curr = elem;
        return *(new _Val());
    };

    virtual _Val& FindNext(_Key name)
    {
        LinkedListElem<_Key, _Val>* elem = curr;

        if (curr != NULL) {
            curr = curr->Next();
        }
        while (curr != NULL) {
            if (curr->Key() == name) {
                break;
            }
            curr = curr->Next();
        }
        if (curr != NULL) {
            return *(new _Val(curr->Value()));
        }
        curr = elem;
        return *(new _Val());
    };

    void Append(_Key name, _Val value)
    {
        LinkedListElem<_Key, _Val>* elem;
        LinkedListElem<_Key, _Val>* obj;

        obj = new LinkedListElem<_Key, _Val>();
        obj->Key(name);
        obj->Value(value);
        if (data == NULL) {
            data = obj;
            curr = data;
        }
        else {
            elem = curr;
            Last();
            curr->Add(obj);
            curr = elem;
        }
    };
    void Insert(_Key name, _Val value)
    {
        LinkedListElem<_Key, _Val>* obj;

        obj = new LinkedListElem<_Key, _Val>();
        obj->Key(name);
        obj->Value(value);
        if (data == NULL) {
            data = obj;
            curr = data;
        }
        else {
            curr->Add(obj);
        }
    };
    void Clear()
    {
        curr = data;
        while (curr != NULL) {
            curr = curr->Next();
            delete data;
            data = curr;
        }
    };

    LinkedListElem<_Key, _Val>* First() { curr = data; return (curr); };
    LinkedListElem<_Key, _Val>* Last()
    {
        if (curr == NULL) {
            return curr;
        }
        while (curr->Next() != NULL) {
            curr = curr->Next();
        }
        return curr;
    };
    LinkedListElem<_Key, _Val>* Current() { return curr;  };
    void Erase()
    {
        LinkedListElem<_Key, _Val>* obj;

        if (curr == NULL) {
            return;
        }
        obj = data;
        while (obj != NULL) {
            if (obj->Next() == curr) {
                break;
            }
            obj = obj->Next();
        }
        if (obj == NULL) {
            return;
        }
        obj->Link(curr->Next());

    };
    LinkedListElem<_Key, _Val>* operator++(int) { if(curr != NULL) { curr = curr->Next(); } return curr;    };

protected:
#ifndef __DOXYGEN__
    LinkedListElem<_Key, _Val>* data;
    LinkedListElem<_Key, _Val>* curr;
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
            data = new LinkedListElem<_Key, _Val>;
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

    void clear() { values.clear(); keys.clear(); };

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
