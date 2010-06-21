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

#include <weVariant.h>

using namespace webEngine;

bool we_variant::operator<( const we_variant &rhl ) const
{
    bool result = false;
    char c_t, c_a;
    int i_t, i_a;
    bool b_t, b_a;
    double d_t, d_a;
    string s_t, s_a;

    switch (which()) {
    case 0: // char
        c_t = boost::get<char>(*this);
        c_a = boost::get<char>(rhl);
        result = (c_t < c_a);
        break;
    case 1: // int
        i_t = boost::get<int>(*this);
        i_a = boost::get<int>(rhl);
        result = (i_t < i_a);
        break;
    case 2: // bool
        b_t = boost::get<bool>(*this);
        b_a = boost::get<bool>(rhl);
        result = (b_t < b_a);
        break;
    case 3: // double
        d_t = boost::get<double>(*this);
        d_a = boost::get<double>(rhl);
        result = (d_t < d_a);
        break;
    case 4: // string
        s_t = boost::get<string>(*this);
        s_a = boost::get<string>(rhl);
        result = (s_t < s_a);
        break;
    default:
        result = false;
    }
    return result;
}

bool we_variant::operator>( const we_variant &rhl ) const
{
    bool result = false;
    char c_t, c_a;
    int i_t, i_a;
    bool b_t, b_a;
    double d_t, d_a;
    string s_t, s_a;

    switch (which()) {
    case 0: // char
        c_t = boost::get<char>(*this);
        c_a = boost::get<char>(rhl);
        result = (c_t > c_a);
        break;
    case 1: // int
        i_t = boost::get<int>(*this);
        i_a = boost::get<int>(rhl);
        result = (i_t > i_a);
        break;
    case 2: // bool
        b_t = boost::get<bool>(*this);
        b_a = boost::get<bool>(rhl);
        result = (b_t > b_a);
        break;
    case 3: // double
        d_t = boost::get<double>(*this);
        d_a = boost::get<double>(rhl);
        result = (d_t > d_a);
        break;
    case 4: // string
        s_t = boost::get<string>(*this);
        s_a = boost::get<string>(rhl);
        result = (s_t > s_a);
        break;
    default:
        result = false;
    }
    return result;
}

bool we_variant::operator<=( const we_variant &rhl ) const
{
    bool result = false;
    char c_t, c_a;
    int i_t, i_a;
    bool b_t, b_a;
    double d_t, d_a;
    string s_t, s_a;

    switch (which()) {
    case 0: // char
        c_t = boost::get<char>(*this);
        c_a = boost::get<char>(rhl);
        result = (c_t <= c_a);
        break;
    case 1: // int
        i_t = boost::get<int>(*this);
        i_a = boost::get<int>(rhl);
        result = (i_t <= i_a);
        break;
    case 2: // bool
        b_t = boost::get<bool>(*this);
        b_a = boost::get<bool>(rhl);
        result = (b_t <= b_a);
        break;
    case 3: // double
        d_t = boost::get<double>(*this);
        d_a = boost::get<double>(rhl);
        result = (d_t <= d_a);
        break;
    case 4: // string
        s_t = boost::get<string>(*this);
        s_a = boost::get<string>(rhl);
        result = (s_t <= s_a);
        break;
    default:
        result = false;
    }
    return result;
}

bool we_variant::operator>=( const we_variant &rhl ) const
{
    bool result = false;
    char c_t, c_a;
    int i_t, i_a;
    bool b_t, b_a;
    double d_t, d_a;
    string s_t, s_a;
    /*    double uc_t, uc_a;
    double ui_t, ui_a;
    double l_t, l_a;
    double ul_t, ul_a;*/

    switch (which()) {
    case 0: // char
        c_t = boost::get<char>(*this);
        c_a = boost::get<char>(rhl);
        result = (c_t >= c_a);
        break;
    case 1: // int
        i_t = boost::get<int>(*this);
        i_a = boost::get<int>(rhl);
        result = (i_t >= i_a);
        break;
    case 2: // bool
        b_t = boost::get<bool>(*this);
        b_a = boost::get<bool>(rhl);
        result = (b_t >= b_a);
        break;
    case 3: // double
        d_t = boost::get<double>(*this);
        d_a = boost::get<double>(rhl);
        result = (d_t >= d_a);
        break;
    case 4: // string
        s_t = boost::get<string>(*this);
        s_a = boost::get<string>(rhl);
        result = (s_t >= s_a);
        break;
    default:
        result = false;
        /*    case 1: // unsigned char
        uc_t = boost::get<unsigned char>(*this);
        uc_a = boost::get<unsigned char>(rhl);
        result = (uc_t >= uc_a);
        break;
        case 3: // unsigned int
        ui_t = boost::get<unsigned int>(*this);
        ui_a = boost::get<unsigned int>(rhl);
        result = (ui_t >= ui_a);
        break;
        case 4: // long
        l_t = boost::get<long>(*this);
        l_a = boost::get<long>(rhl);
        result = (l_t >= l_a);
        break;
        case 5: // unsigned long
        ul_t = boost::get<unsigned long>(*this);
        ul_a = boost::get<unsigned long>(rhl);
        result = (ul_t >= ul_a);
        break;*/
    }
    return result;
}
