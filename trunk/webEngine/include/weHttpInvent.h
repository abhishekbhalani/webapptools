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
#ifndef __WEHTTPINVENTORY_H__
#define __WEHTTPINVENTORY_H__

#include <weiInventory.h>

namespace webEngine {

class HttpInventory :
    public iInventory
{
public:
    HttpInventory(Dispatch* krnl, void* handle = NULL);
    virtual ~HttpInventory(void);

    // iwePlugin functions
    virtual void* GetInterface(const string& ifName);
    virtual const string GetSetupUI( void );

    // iInventory functions
    void Start(Task* tsk);
    void ProcessResponse(iResponse *resp);
protected:
    map<string, bool> tasklist;
};

} // namespace webEngine

#endif //__WEHTTPINVENTORY_H__