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
/// @class  transport_url
///
/// @brief  transport_url storage and operations  
///
/// @author A. Abramov
/// @date   29.05.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class transport_url
{
public:
    transport_url();
    transport_url(const string url);
    transport_url(const transport_url& url);
    ~transport_url();

    transport_url& operator=(const string& url)  { assign(url); return *this; };
    transport_url& operator=(const transport_url& url);

    string tostring(bool noDefPort = true);
    string tostring_noparam(bool noDefPort = true);

    void assign(const string& url);
    void assign_with_referer(const string& url, transport_url* base = NULL);

    const bool is_equal(const string& url);
    const bool is_equal(const transport_url& url);

    const bool is_host_equal(const string& url);
    const bool is_host_equal(const transport_url& url);

    const bool is_domain_equal(const string& url);
    const bool is_domain_equal(const transport_url& url);

    bool is_valid(void);

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
