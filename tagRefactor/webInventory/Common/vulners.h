/*
    scanServer is the HTML processing library
    Copyright (C) 2009 Andrew Abramov stinger911@gmail.com

    This file is part of scanServer

    scanServeris free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    scanServeris distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with scanServer.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __VULNERS_H__
#define __VULNERS_H__
#include <weVulners.h>

string get_vdesc_list(const string& filter);
string set_vdesc(const string& id, const VulnerDesc& desc);

#endif //__VULNERS_H__
