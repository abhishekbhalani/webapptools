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
#include "weDispatch.h"
#include "weHTTP.h"
#include "weTask.h"
#include "weHelper.h"

using namespace std;

int main(int argc, char* argv[])
{
    webEngine::LibInit();

    webEngine::engine_dispatcher kernel;
    boost::shared_ptr<webEngine::html_document> doc(new webEngine::html_document());
    boost::shared_ptr< webEngine::i_response > resp;
    boost::shared_ptr<webEngine::HttpResponse> htresp;
    webEngine::HttpRequest  *req = NULL;
    webEngine::task tsk;
    webEngine::i_plugin* plg;
    string url = "http://www.ru";

    kernel.refresh_plugin_list();
    plg = kernel.load_plugin("http_transport");
    if (plg != NULL) {
        tsk.add_plg_transport((webEngine::i_transport*)plg);
        if (argc > 1) {
            url = argv[1];
        }
        req = new webEngine::HttpRequest(url);
        url = req->RequestUrl().tostring();
        printf("Get data from: %s\n", url.c_str());
        resp = tsk.get_request(webEngine::i_request_ptr(req));
        htresp = boost::shared_dynamic_cast<webEngine::HttpResponse>(resp);
        if (htresp != NULL) {
            printf("Response: %d\n", htresp->HttpCode());
            printf("Given URL: %s\n", htresp->BaseUrl().tostring().c_str());
            printf("Relocations: %d\n", htresp->RelocCount());
            printf("Resulting URL: %s\n", htresp->RealUrl().tostring().c_str());
            printf("Data size: %d\n", htresp->Data().size());
            if (htresp->Data().size() > 0) {
                bool sc = doc->ParseData(resp);
                printf("Document parsing: %s\n", (sc?"success":"failed"));
                printf("%s\n", &(doc->Data()[0]));
            }
        } else {
            printf("task::get_request failed!\n");
        }
        // finish receiver thread
        // must be done automatically, by task destructor
        // but for example we call this function manually
        tsk.Stop();
    } else {
        printf("Can't find transport!\n");
    }

    webEngine::LibClose();

    return 0;
}

