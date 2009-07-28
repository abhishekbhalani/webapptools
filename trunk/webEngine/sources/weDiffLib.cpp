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

#include "weDiffLib.h"
#include <boost/lexical_cast.hpp>
#include "externals/dtl.hpp"

WeDiffLibWordList* WeDiffLibTextToList(string txt, weCmpMode mode)
{
    WeDiffLibWordList*  lst = new WeDiffLibWordList;
    WeDiffLibWord *wObj;
    bool currSpace;
    unsigned idx, last;
    string word;

    last = idx = 0;
    currSpace = std::isspace(txt[idx++], std::locale());
    word = "";
    while(idx < txt.length()) {
        while (currSpace == std::isspace(txt[idx++], std::locale()) && idx < txt.length());
        if (idx == txt.length()) {
            idx++; // fix the last char stripping
        }
        if (currSpace) {
            // process spaces
            if (mode & weCmpCollapseSpace) {
                word += " ";    // add single whitespace
            }
            else {
                word += txt.substr(last, idx - 1 - last);
            }
            wObj = new WeDiffLibWord;
            wObj->mode = mode;
            wObj->word = word;
            lst->push_back(*wObj);
            word = "";
            last = idx - 1;
            currSpace = std::isspace(txt[last], std::locale());
        }
        else {
            // process word
            word += txt.substr(last, idx - 1 - last);
            last = idx - 1;
            currSpace = std::isspace(txt[last], std::locale());
        }
    }
    if (!word.empty()) {
        wObj = new WeDiffLibWord;
        wObj->mode = mode;
        wObj->word = word;
        lst->push_back(*wObj);
    }
    return lst;    
}

WeCmpResults* WeTextDiff(string txt1, string txt2, weCmpMode mode/* = weCmpDefault*/)
{
    WeCmpResults* retval;
    string        txt;
    unsigned      idx;
    WeDiffLibWordList* lst1;
    WeDiffLibWordList* lst2;

    // compose lists from text
    lst1 = WeDiffLibTextToList(txt1, mode);
    lst2 = WeDiffLibTextToList(txt2, mode);

    dtl::Diff<WeDiffLibWord, WeDiffLibWordList> d(*lst1, *lst2);
    d.compose();
    dtl::Ses<WeDiffLibWord> ses = d.getSes();
    vector< pair<WeDiffLibWord, dtl::elemInfo> > ses_v = ses.getSequence();

    // produce combined output
    retval = new WeCmpResults;
    for (idx = 0; idx < ses_v.size(); idx++) {
        if (idx == 0) {
            retval->push_back(new WeCmpBlock());
            retval->back()->state = (weCmpState)ses_v[idx].second.type;
            txt = ses_v[idx].first.word;
        }
        else {
            if(retval->back()->state == ses_v[idx].second.type) {
                txt += ses_v[idx].first.word;
            }
            else {
                retval->back()->entity = txt;
                retval->push_back(new WeCmpBlock());
                retval->back()->state = (weCmpState)ses_v[idx].second.type;
                txt = ses_v[idx].first.word;
            }
        }
    }
    if (!txt.empty()) {
        retval->back()->entity = txt;
    }

    // cleanup
    delete lst1;
    delete lst2;

    return retval;
}
