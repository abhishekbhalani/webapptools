/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew Abramov aabramov@ptsecurity.ru

    This file is part of webEngine

    webEngineis free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngineis distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __WEIDIFFLIB_H__
#define __WEIDIFFLIB_H__

#include <string>
#include <vector>
#include <set>
#include <map>
#include <boost/variant.hpp>
#include <boost/algorithm/string/predicate.hpp>

using namespace std;
using namespace boost;

/// @file   weDiffLib.h
/// @brief  Difference building operations and structures
/// @example difflib.cpp

enum weCmpState{
    weCmpNonComparable = -99,  ///< comparison can't be performed, or error occurred
    weCmpDeleted    = -2,
    weCmpLess       = -1,
    weCmpEqual      = 0,
    weCmpGreather   = 1,
    weCmpInserted   = 2
} ;

class iweEntity;  // forward declaration for pointer
typedef variant<string, iweEntity*> WeCmpEntity;

struct WeCmpBlock
{
    weCmpState  state;
    WeCmpEntity entity;
};

typedef vector<WeCmpBlock*> WeCmpResults;

enum weCmpMode {
    CmpMask = 0xFFFFFFFF
};

static const weCmpMode weCmpNothing         = (weCmpMode)0x0000;
static const weCmpMode weCmpInnerText       = (weCmpMode)0x0001;
static const weCmpMode weCmpCaseInsens      = (weCmpMode)0x0002;
static const weCmpMode weCmpCollapseSpace   = (weCmpMode)0x0004;
static const weCmpMode weCmpAttributes      = (weCmpMode)0x0008;
static const weCmpMode weCmpAttrValues      = (weCmpMode)0x0010;
static const weCmpMode weCmpAttrOrder       = (weCmpMode)0x0020;
static const weCmpMode weCmpChildren        = (weCmpMode)0x0040;
static const weCmpMode weCmpChildrenTree    = (weCmpMode)0x0080;

static const weCmpMode weCmpDefault         = (weCmpMode)0xffff;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @struct WeDiffLibWord
///
/// @brief  Text element to compare.
///
/// @author A. Abramov
/// @date   25.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
struct WeDiffLibWord
{
#ifndef __DOXYGEN__
    string word;
    weCmpMode mode;
    bool operator==(WeDiffLibWord& cpy)
    {
        if (mode & weCmpCaseInsens)
        {
            return iequals(word, cpy.word);
        }
        return equals(word, cpy.word);
    };
#endif // __DOXYGEN__
};

typedef vector<WeDiffLibWord>   WeDiffLibWordList;

extern WeDiffLibWordList* WeDiffLibTextToList(string txt, weCmpMode mode);
extern WeCmpResults* WeTextDiff(string txt1, string txt2, weCmpMode mode = weCmpDefault);

#endif //__WEIDIFFLIB_H__
