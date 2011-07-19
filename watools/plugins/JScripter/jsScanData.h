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
    Version:        $Revision$
*/
#pragma once

#ifndef __JSCRIPTERSCANDATA__H__
#define __JSCRIPTERSCANDATA__H__

#include <v8.h>
#include <weUrl.h>
#include <v8_wrapper.h>
#include "weScan.h"

namespace jscripter
{
    class jsScanData: public v8_wrapper::Registrator<jsScanData> {
    public:
        jsScanData(webEngine::scan_data_ptr scData);
        ~jsScanData(void);
        static bool is_init;
        static v8::Persistent<v8::FunctionTemplate> object_template;
    };
}

#endif //__JSCRIPTERSCANDATA__H__
