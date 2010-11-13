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
#ifndef WEBENGINE_JSEXECUTOR
#define WEBENGINE_JSEXECUTOR
#include <v8.h>
#include <string>
#include <vector>
#include <weBlob.h>
#include <weiTransport.h>
#include <v8_wrapper.h>

using namespace std;

namespace webEngine {

class task;
//////////////////////////////////////////////////////////////////////////
/// @class jsExecutor
///
/// @brief General-purpose JavaScript executor based on Google-V8 engine
///
//////////////////////////////////////////////////////////////////////////
class jsExecutor : public v8_wrapper::Registrator<jsExecutor> {
public:
    jsExecutor(void);
    ~jsExecutor(void);

    bool execute_string(const string& src,
                        const string& nm,
                        bool print_result,
                        bool report_exceptions,
                        v8::Handle<v8::Context> ctx = v8::Handle<v8::Context>((v8::Context*)NULL));
    void reset_results();
    void set_max_depth(int depth) {
        maxDepth = depth;
    };

    v8::Persistent<v8::Context> get_child_context();
    void close_child_context(v8::Persistent<v8::Context> ctx);

    static v8::Persistent<v8::FunctionTemplate> object_template;
    static void init_globals();
    bool allow_network(task* net_acc) {
        net_access = net_acc;
        return (net_access != NULL);
    }
    i_response_ptr http_request(i_request_ptr req);
    bool http_request_async(i_request_ptr req);

    v8::Local<v8::Value> v8_get(v8::Local<v8::Value> val) {
        return context->Global()->Get(val);
    }

protected:
    task* net_access;
    static bool is_init;
    static v8::Persistent<v8::ObjectTemplate> global;

    v8::Persistent<v8::Context> context;
    int maxDepth;

    void report_exception(v8::TryCatch* handler);
};
}

#endif // WEBENGINE_JSEXECUTOR