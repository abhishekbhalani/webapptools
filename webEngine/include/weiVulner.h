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
#pragma once
#ifndef __WEIVULNER_H__
#define __WEIVULNER_H__

#include <weiPlugin.h>

namespace webEngine {

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @interface  i_vulner
///
/// @brief  Interface for the vulnerabilities plugins. 
///
/// @author A. Abramov
/// @date   19.08.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class i_vulner :
    public i_plugin
{
public:
    i_vulner(engine_dispatcher* krnl, void* handle = NULL);
    virtual ~i_vulner(void);
};

} // namespace webEngine

#endif //__WEIVULNER_H__
