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
#ifndef __WEIAUDIT_H__
#define __WEIAUDIT_H__

#include <weiPlugin.h>

namespace webEngine {

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @interface  iAudit
///
/// @brief  Interface for the audit plugins. 
///
/// @author A. Abramov
/// @date   19.08.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class iAudit :
    public iPlugin
{
public:
    iAudit(Dispatch* krnl, void* handle = NULL);
    virtual ~iAudit(void);

    // iPlugin functions

};

} // namespace webEngine
#endif //__WEIAUDIT_H__
