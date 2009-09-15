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
#ifndef __WEURL_H__
#define __WEURL_H__

#include <string>
#include "weiBase.h"

using namespace std;

namespace webEngine {

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  URL
///
/// @brief  URL storage and operations  
///
/// @author A. Abramov
/// @date   29.05.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class URL
{
public:
    URL();
    URL(const string url);
    URL(const URL& url);
    ~URL();

    URL& operator=(const string& url)  { Assign(url); return *this; };
    URL& operator=(const URL& url);

    string ToString(bool noDefPort = true);
    string ToStringNoParam(bool noDefPort = true);

    void Assign(const string& url);
    void Restore(const string& url, URL* base = NULL);

    const bool Equals(const string& url);
    const bool Equals(const URL& url);

    const bool IsHostEquals(const string& url);
    const bool IsHostEquals(const URL& url);

    const bool IsDomainEquals(const string& url);
    const bool IsDomainEquals(const URL& url);

    bool IsValid(void);

    string  protocol;       ///< transfer protocol
    string  host;           ///< hostname
    unsigned port;          ///< port number
    string  request;        ///< request path
    string  params;         ///< request parameters
    string  username;       ///< username
    string  password;       ///< password

#ifndef __DOXYGEN__
protected:
    bool valid;
#endif //__DOXYGEN__
private:
    DECLARE_SERIALIZATOR
    {
        ar & BOOST_SERIALIZATION_NVP(protocol);
        ar & BOOST_SERIALIZATION_NVP(host);
        ar & BOOST_SERIALIZATION_NVP(port);
        ar & BOOST_SERIALIZATION_NVP(request);
        ar & BOOST_SERIALIZATION_NVP(params);
        ar & BOOST_SERIALIZATION_NVP(username);
        ar & BOOST_SERIALIZATION_NVP(password);
    };
};

} // namespace webEngine

#endif //__WEURL_H__
