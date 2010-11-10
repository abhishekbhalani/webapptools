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

    @file   tagscanner.cpp
    @brief  Example for WeTagScanner class usage.
*/

#include <fstream>
#include "weHtmlEntity.h"
#include "weTagScanner.h"

using namespace std;
using namespace webEngine;

const char* html = "<html><body><p align=right dir='rtl'>Begin &amp;    &copy; back</p>"
                   "<a href=http://terrainformatica.com/index.php?a=1&b=2>link</a><br/><!-- comment -->"
                   "<script language='jscript'>document.write(\"<h1>test</h1><hr size='1'>\");</script>"
                   "</body></html>";

int main(int argc, char* argv[])
{
    blob  file;
    boost::shared_ptr<tag_stream> st;
    boost::shared_ptr<tag_stream> si;

    if (argc > 1) {
        ifstream ifs(argv[1]);
        file.read(ifs);
        st = file.stream();
    } else {
        si = boost::shared_ptr<tag_stream>(new str_tag_stream(html));
        st = si;
    }
    tag_scanner sc(*st);

    while(true) {
        int t = sc.get_token();
        switch(t) {
        case wstError:
            printf("ERROR\n");
            break;
        case wstEof:
            printf("EOF\n");
            goto FINISH;
        case wstTagStart:
            printf("TAG START:%s\n", sc.get_tag_name());
            break;
        case wstTagEnd:
            printf("TAG END:%s\n", sc.get_tag_name());
            break;
        case wstAttr:
            printf("\tATTR:%s=%s\n", sc.get_tag_name(), sc.get_value());
            break;
        case wstCommentStart:
            printf("COMMENT START\n");
            break;
        case wstCommentEnd:
            printf("COMMENT END\n");
            break;
        case wstData:
            printf("\tDATA = %s\n", sc.get_value());
            break;
        case wstWord:
        case wstSpace:
            printf("{%s}\n", sc.get_value());
            break;
        }
    }
FINISH:
    printf("--------------------------\n");
    return 0;
}

