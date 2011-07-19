/*
    webEngine is the HTML processing library
    Copyright (C) 2009-11 Andrew Abramov aabramov@ptsecurity.ru

    This file is part of webEngine

    webEngine is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.

    Author:         Andrew "Stinger" Abramov
    Version:        $Revision: 0 $
*/
#pragma once

#ifndef __JSCRIPTERTASK__H__
#define __JSCRIPTERTASK__H__

#include <v8.h>
#include <weUrl.h>
#include <v8_wrapper.h>
#include "weTask.h"

namespace jscripter
{
    class jsTask: public v8_wrapper::Registrator<jsTask> {
    public:
        jsTask(webEngine::task* tsk);
        ~jsTask(void);
        static bool is_init;
        static v8::Persistent<v8::FunctionTemplate> object_template;
    };
}

#endif //__JSCRIPTERTASK__H__
