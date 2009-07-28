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

    @file   httpget.cpp
    @brief  Example for WeHTTP and related classes usage. 
*/

#include "weHtmlEntity.h"
#include "weHTTP.h"
#include "weTask.h"
#include "weHelper.h"

using namespace std;

int main(int argc, char* argv[])
{
    WeHTTP proc;
    WeHtmlDocument  doc;
    WeHttpResponse resp;

    WeLibInit();

    proc.Option(weoCollapseSpaces, true);
    proc.Option(weoStayInHost, true);
    proc.Option(weoFollowLinks, true);
    proc.RelocationCount(10);

    proc.Request("http://www.ru", &resp);

    while (!resp.Processed()) {
        proc.ProcessRequests();
    }

    printf("Response: %d\n", resp.HttpCode());
    printf("Given URL: %s\n", resp.BaseUrl().ToString().c_str());
    printf("Relocations: %d\n", resp.RelocCount());
    printf("Resulting URL: %s\n", resp.RealUrl().ToString().c_str());
    printf("Data size: %d\n", resp.Data().size());
    if (resp.Data().size() > 0) {
        bool sc = doc.ParseData(&resp, &proc);
        printf("Document parsing: %s\n", (sc?"success":"fail"));
        printf("%s\n", &(doc.Data()[0]));
    }

    WeLibClose();

    return 0;
}

