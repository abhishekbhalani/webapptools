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
#include <v8/v8.h>
#include <string>
#include <vector>

using namespace std;

namespace webEngine {
    class jsExecutor
    {
    public:
        jsExecutor(void);
        ~jsExecutor(void);

        bool execute_string(const string& src,
            const string& nm,
            bool print_result,
            bool report_exceptions);
        void reset_results();
        const string get_results() {return exec_result; };
        void append_results(const string& data);
        void append_object(v8::Handle<v8::Object> data);
        void set_max_depth(int depth) {maxDepth = depth; };
        string dump(const string& name, const string& indent = "", int depth = 0);
        string obj_dump(v8::Local<v8::Value> val, const string& name, const string& indent, int depth);

        static v8::Persistent<v8::FunctionTemplate> object_template;

    protected:
        static bool is_init;
        static int  num_objects;
        static v8::Persistent<v8::ObjectTemplate> global;

        v8::Persistent<v8::Context> context;
        vector<v8::Persistent<v8::Value>> objects;
        string exec_result;
        int maxDepth;

        void report_exception(v8::TryCatch* handler);

        friend v8::Handle<v8::Value> result_object(v8::Local<v8::String> name, const v8::AccessorInfo &info);
        friend v8::Handle<v8::Value> get_result_string(v8::Local<v8::String> name, const v8::AccessorInfo &info);
        friend void set_result_string(v8::Local<v8::String> name, v8::Local<v8::Value> val, const v8::AccessorInfo& info);
        //friend v8::Handle<v8::Value> dump(const v8::Arguments& args);
    };
}

#endif // WEBENGINE_JSEXECUTOR