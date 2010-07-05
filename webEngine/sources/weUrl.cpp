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
#include <webEngine.h>

#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include "weUrl.h"

using namespace boost::algorithm;

namespace webEngine {

#ifndef __DOXYGEN__
static string& fix_double_slash(string& req)
{
    string path, vals;
    size_t pos;

    pos = req.find('?');
    path = req.substr(0, pos);
    if (pos != string::npos) {
        vals = req.substr(pos);
    }
    else {
        vals = "";
    }
    replace_all(path, "/./", "/");
    while (path.find("//") != string::npos) {
        replace_all(path, "//", "/");
    }
    req = path + vals;
    return req;
}
#endif //__DOXYGEN__

transport_url::transport_url()
{
    port = -1;
    host = "";
    request = "";
    valid = false;
}

transport_url::transport_url( const string url )
{
    port = -1;
    host = "";
    request = "";
    valid = false;
    assign(url);
}

transport_url::transport_url( const transport_url& url )
{
    protocol = url.protocol;
    host = url.host;
    port = url.port;
    request = url.request;
    params = url.params;
    username = url.username;
    password = url.password;
    valid = url.valid;
}

transport_url::~transport_url()
{
    // nothing special
}

transport_url& transport_url::operator=( const transport_url& url )
{
    protocol = url.protocol;
    host = url.host;
    port = url.port;
    request = url.request;
    params = url.params;
    username = url.username;
    password = url.password;
    valid = url.valid;
    return *this;
}

void transport_url::assign(const string& url )
{
    string temp = url;
    size_t pos;

    // Find end
    pos = temp.find(':');
    if (pos == string::npos) {
        valid = false;
        return;
    }

    protocol = temp.substr(0, pos);
    to_lower(protocol);
    temp = temp.substr(pos+1);

    // fix for mailto: proto
    if (protocol == "mailto") {
        host = temp;
        to_lower(host);
        request = "";
        params = "";
        port = 25;
        password = "";
        username = ""; // todo: extract username
        valid = true;
        return;
    }

    if ((temp[0] != '/') || (temp[1] != '/')) {
        valid = false;
        return;
    }

    temp = temp.substr(2);      // We use parsed out the ("http://"), leaving: "user:pass@host:port/request/dir/file.ext"
    pos = temp.find_first_of("/?");
    if (pos == string::npos) {
        request = "/";             // Default root path is "/"
        temp = temp.substr(0, pos);            // leaving: "user:pass@host:port"
    }
    else {
        request = temp.substr(pos);        // parsed out: "/request/dir/file.ext"
        temp = temp.substr(0, pos);            // leaving: "user:pass@host:port"
    }
    if (request[0] != '/') {
        request.insert(0, "/");
    }

    if (pos == 0) {
        valid = false;
        return;
    }

    // Fixed logic here on 02/05/2005
    pos = temp.find('@');
    if(pos != string::npos)  {
        string szAuth = temp.substr(0,pos);  // parsing out: "user:pass"
        temp = temp.substr(pos+1);         // leaving: "host:port"

        pos = szAuth.find(':');
        if( pos != string::npos ) {
            password = szAuth.substr(pos+1);
            username = szAuth.substr(0,pos);
        }
        else
            username = szAuth;
    }

    // left: "host:port"
    pos = temp.find(':');
    if( pos != string::npos ) {
        port = atoi(temp.substr(pos+1).c_str());
        host = temp.substr(0,pos);
        to_lower(host);
    }
    else {
        host = temp;
        to_lower(host);
        if( protocol == "http" )
            port = 80;
        else if( protocol == "https" )
            port = 443;
        else
            port = 80;
    }

    while (starts_with(request, "./"))
    {
        request = request.substr(2);
    }
    fix_double_slash(request);
    pos = request.find('?');
    if (pos != string::npos) {
        params = request.substr(pos + 1);
        request = request.substr(0, pos);
    }
    else {
        params.clear();
    }

    // extract hashlink
    // added 14.04.2010
    pos = request.find('#');
    if (pos != string::npos) {
        hashlink = request.substr(pos + 1);
        request = request.substr(0, pos);
    }
    else {
        hashlink.clear();
    }
    pos = params.find('#');
    if (pos != string::npos) {
        hashlink = params.substr(pos + 1);
        params = params.substr(0, pos);
    }
    else {
        hashlink.clear();
    }
    valid = true;
}

void transport_url::assign_with_referer( const string& url_, transport_url* base /*= NULL*/ )
{
    string  temp;
    size_t  pos;
    string  url = url_;

    if (base == NULL) {
        base = this;
    }

    // fix for mailto: proto
    if (istarts_with(url_, "mailto:")) {
        protocol = "mailto";
        host = url_.substr(7);
        to_lower(host);
        request = "";
        params = "";
        port = 25;
        password = "";
        username = ""; // todo: extract username
        valid = true;
        return;
    }

    // Find protocol end
    pos = url.find(":");
    if (pos == string::npos) {
        // reduced transport_url - only local path
        protocol = base->protocol;
        to_lower(protocol);
        username = base->username;
        password = base->password;
        host = base->host;
        port = base->port;
        if (starts_with(url, "/")) {
            request = url;
            fix_double_slash(request);
            pos = request.find('?');
            if (pos != string::npos) {
                params = request.substr(pos + 1);
                request = request.substr(0, pos);
            }
            else {
                params.clear();
            }
        }
        else {
            pos = base->request.find('?');
            temp = base->request.substr(0, pos);
            pos = temp.rfind('/');
            temp = temp.substr(0, pos);
            // process './'
            while (starts_with(url, "./"))
            {
                url = url.substr(2);
            }
            // process '../'
            while (starts_with(url, "../"))
            {
                pos = temp.rfind('/');
                if (pos == string::npos) {
                    break;
                }
                temp = temp.substr(0, pos);
                url = url.substr(3);
                if (starts_with(url, "./"))
                {
                    url = url.substr(2);
                }
            }
            temp += '/';
            temp += url;
            request = temp;
            fix_double_slash(request);
            pos = request.find('?');
            if (pos != string::npos) {
                params = request.substr(pos + 1);
                request = request.substr(0, pos);
            }
            else {
                params.clear();
            }
        }

        // extract hashlink
        // added 14.04.2010
        pos = request.find('#');
        if (pos != string::npos) {
            hashlink = request.substr(pos + 1);
            request = request.substr(0, pos);
        }
        else {
            hashlink.clear();
        }
        pos = params.find('#');
        if (pos != string::npos) {
            hashlink = params.substr(pos + 1);
            params = params.substr(0, pos);
        }
        else {
            hashlink.clear();
        }
        valid = true;
    }
    else {
        assign(url);
    }
}

bool transport_url::is_valid( void )
{
    bool retval = (valid && !host.empty() && !request.empty() && port > 0 && port < 65535);
    valid = retval;
    return retval;
}

const bool transport_url::is_equal( const string& url )
{
    transport_url tmp;
    tmp.assign(url);
    return is_equal(tmp);
}

const bool transport_url::is_equal( const transport_url& url )
{
    bool retval = true;

    if (protocol != url.protocol)
    {
        retval = false;
    }
    if (host != url.host)
    {
        retval = false;
    }
    if (port != url.port)
    {
        retval = false;
    }
    if (request != url.request)
    {
        retval = false;
    }
    if (params != url.params)
    {
        retval = false;
    }
    /** @todo Is it need to compare credentials?
    if (username != url.username)
    {
        retval = false;
    }
    if (password != url.password)
    {
        retval = false;
    }*/
    return retval;
}

const bool transport_url::is_host_equal( const string& url )
{
    transport_url tmp;
    tmp.assign(url);
    return is_host_equal(tmp);
}

const bool transport_url::is_host_equal( const transport_url& url )
{
    bool retval = true;

    if (protocol != url.protocol)
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "transport_url::is_host_equal - protocols doesn't match");
        retval = false;
    }
    if (host != url.host)
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "transport_url::is_host_equal - hostnames doesn't match: " << url.host << " != " << host);
        if (host != "www." + url.host )
        {
            LOG4CXX_TRACE(iLogger::GetLogger(), "transport_url::is_host_equal - hostnames doesn't match: www." << url.host << " != " << host);
            retval = false;
        }
    }
    return retval;
}

const bool transport_url::is_domain_equal( const string& url )
{
    transport_url tmp;
    tmp.assign(url);
    return is_domain_equal(tmp);
}

const bool transport_url::is_domain_equal( const transport_url& url )
{
    bool retval = true;
    string second_level;
    boost::iterator_range<string::iterator> dot_pos;

    if (protocol != url.protocol)
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "transport_url::is_domain_equal - protocols doesn't match");
        retval = false;
    }
    dot_pos = find_nth(host, ".", -2);
    if (dot_pos.size() != 0) {
        string::iterator pos = dot_pos.begin();
        pos++; // skip the dot
        second_level = "";
        while(pos != host.end())
        {
            second_level += *pos;
            pos++;
        }
    }
    else {
        second_level = host;
    }
    LOG4CXX_TRACE(iLogger::GetLogger(), "transport_url::is_domain_equal - compare " << second_level << " and " << url.host);
    if (!iends_with(url.host, second_level))
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "transport_url::is_domain_equal - iends_with returns fasle");
        retval = false;
    }

    return retval;
}

string transport_url::tostring(bool noDefPort /*= true*/)
{
    string retval; // = new string;

    retval = tostring_nohash(noDefPort);
    if (!hashlink.empty()) {
        retval += "#";
        retval += hashlink;
    }
    return retval;
}

string transport_url::tostring_noparam( bool noDefPort /*= true*/ )
{
    string retval; // = new string;

    retval = protocol + "://";
    if (!username.empty() || !password.empty()) {
        retval += username + ':' + password +'@';
    }
    retval += host;
    if (noDefPort && ((protocol == "http" && port == 80) || (protocol == "https" && port == 443))) {
        // oops, in this case we don't need print anything :)
    }
    else if (port > 0 && port < 65536) {
        retval += ':';
        retval += boost::lexical_cast<std::string>(port);
    }
    retval += request;
    return retval;
}

string transport_url::tostring_nohash( bool noDefPort /*= true*/ )
{
    string retval; // = new string;

    retval = tostring_noparam(noDefPort);
    if (!params.empty()) {
        retval += "?";
        retval += params;
    }
    return retval;
}

} // namespace webEngine
