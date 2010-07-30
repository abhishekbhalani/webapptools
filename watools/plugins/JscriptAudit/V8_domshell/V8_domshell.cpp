// V8_domshell.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#define V8_DOMSHELL 1
#include <v8/v8.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <weHelper.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <weDispatch.h>
#include <weTask.h>

using namespace webEngine;
namespace bfs = boost::filesystem;

// from common/
#include "jsWrappers/JsBrowser.h"

class shellExecutor : public jsBrowser
{
public:
    static v8::Persistent<v8::ObjectTemplate> global_object() { return global; }
};

void RunShell(v8::Handle<v8::Context> context);
bool ExecuteString(v8::Handle<v8::String> source,
                   v8::Handle<v8::Value> name,
                   bool print_result,
                   bool report_exceptions);
v8::Handle<v8::Value> Print(const v8::Arguments& args);
v8::Handle<v8::Value> Read(const v8::Arguments& args);
v8::Handle<v8::Value> Load(const v8::Arguments& args);
v8::Handle<v8::Value> Quit(const v8::Arguments& args);
v8::Handle<v8::Value> Version(const v8::Arguments& args);

v8::Handle<v8::Value> Location(const v8::Arguments& args);
void                  SetLocationHref(v8::Local<v8::String> name, v8::Local<v8::Value> val, const v8::AccessorInfo& info);
v8::Handle<v8::Value> GetLocationHref(v8::Local<v8::String> name, const v8::AccessorInfo& info);

v8::Handle<v8::String> ReadFile(const char* name);
void ReportException(v8::TryCatch* handler);

std::vector<v8::Persistent<v8::Value>>  objects;
v8::Handle<v8::Value> GetAnyObject(v8::Local<v8::String> name, const v8::AccessorInfo &info)
{
    //this only shows information on what object is being used... just for fun
    //     {
    //         v8::String::AsciiValue prop(name);
    //         v8::String::AsciiValue self(info.This()->ToString());
    //         LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::BrowserGet: self("<< *self <<"), property("<< *prop<<")");
    //     }
    v8::HandleScope scope;

    v8::Local<v8::Array> res = v8::Array::New();
    for(size_t i = 0; i < objects.size(); i++) {
        v8::Local<v8::Value> val = v8::Local<v8::Value>::New(objects[i]);
        res->Set(v8::Int32::New(i), val);
    }
    return scope.Close(res);
}

// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
    return *value ? *value : "<string conversion failed>";
}

int RunMain(int argc, char* argv[]) {
    v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
    v8::HandleScope handle_scope;
    // init global objects
    engine_dispatcher we_dispatcer;
    bfs::path plg_path = ".";
    we_dispatcer.refresh_plugin_list(plg_path);
    i_plugin* plg = we_dispatcer.load_plugin("sqlite_storage");
    if (plg == NULL) {
        LOG4CXX_FATAL(iLogger::GetLogger(), "Can't load plug-in for Storage DB connection: sqlite_storage");
        return 0;
    }
    i_storage* storage = (i_storage*)plg->get_interface("i_storage");
    if (plg == NULL) {
        LOG4CXX_FATAL(iLogger::GetLogger(), "No iStorage interface in the plugin " << plg->get_id() << "(ID="  << plg->get_id() );
        return 0;
    }
    string params = "v8_domshell.sqlite";
    storage->init_storage(params);
    we_dispatcer.storage(storage);
    plugin_list plgs = we_dispatcer.get_plugin_list();
    vector<i_plugin*> scan_plugins;
    for (size_t i = 0; i < plgs.size(); i++)
    {
        plg = we_dispatcer.load_plugin(plgs[i].plugin_id);
        scan_plugins.push_back(plg->get_interface("i_plugin")); //(webEngine::i_plugin*)
    }
    task* tsk = new webEngine::task(&we_dispatcer);
    tsk->store_plugins(scan_plugins);
    tsk->set_profile_id(string("0")); // todo - take it from params

    shellExecutor::init_globals();
    shellExecutor::global_object()->Set(v8::String::New("read"), v8::FunctionTemplate::New(Read));
    // Bind the global 'load' function to the C++ Load callback.
    shellExecutor::global_object()->Set(v8::String::New("load"), v8::FunctionTemplate::New(Load));
    // Bind the 'quit' function
    shellExecutor::global_object()->Set(v8::String::New("quit"), v8::FunctionTemplate::New(Quit));
    // Bind the 'version' function
    // shellExecutor::global_object()->Set(v8::String::New("version"), v8::FunctionTemplate::New(Version));
    // Bind the 'version' function
    // shellExecutor::global_object()->Set(v8::String::New("echo"), v8::FunctionTemplate::New(Print));
    // init global objects
    shellExecutor::global_object()->SetAccessor(v8::String::NewSymbol("objects"), GetAnyObject);

    shellExecutor executor;
    executor.allow_network(tsk);

    // Bind the global 'read' function to the C++ Read callback.
    v8::Persistent<v8::Context> context = executor.get_child_context();

    // Create a new execution environment containing the built-in
    // functions
    // Enter the newly created execution environment.
    v8::Context::Scope context_scope(context);
    executor.window->history->push_back("http://www.ru");
    executor.window->history->push_back("http://www.ya.ru");
    bool run_shell = (argc == 1);
    for (int i = 1; i < argc; i++) {
        const char* str = argv[i];
        if (strcmp(str, "--shell") == 0) {
            run_shell = true;
        } else if (strcmp(str, "-f") == 0) {
            // Ignore any -f flags for compatibility with the other stand-
            // alone JavaScript engines.
            continue;
        } else if (strncmp(str, "--", 2) == 0) {
            printf("Warning: unknown flag %s.\nTry --help for options\n", str);
        } else if (strcmp(str, "-e") == 0 && i + 1 < argc) {
            // Execute argument given to -e option directly
            v8::HandleScope handle_scope;
            v8::Handle<v8::String> file_name = v8::String::New("unnamed");
            v8::Handle<v8::String> source = v8::String::New(argv[i + 1]);
            if (!ExecuteString(source, file_name, false, true))
                return 1;
            i++;
        } else {
            // Use all other arguments as names of files to load and run.
            v8::HandleScope handle_scope;
            v8::Handle<v8::String> file_name = v8::String::New(str);
            v8::Handle<v8::String> source = ReadFile(str);
            if (source.IsEmpty()) {
                printf("Error reading '%s'\n", str);
                return 1;
            }
            if (!ExecuteString(source, file_name, false, true))
                return 1;
        }
    }
    if (run_shell) RunShell(context);

    executor.allow_network(NULL);
    delete tsk;
    executor.close_child_context(context);
    return 0;
}


int main(int argc, char* argv[]) {
    webEngine::LibInit(".\\trace.config");
    int result = RunMain(argc, argv);
    v8::V8::Dispose();
    webEngine::LibClose();
    return result;
}

// The callback that is invoked by v8 whenever the JavaScript 'print'
// function is called.  Prints its arguments on stdout separated by
// spaces and ending with a newline.
v8::Handle<v8::Value> Print(const v8::Arguments& args) {
    bool first = true;
    for (int i = 0; i < args.Length(); i++) {
        v8::HandleScope handle_scope;
        if (first) {
            first = false;
        } else {
            printf(" ");
        }
        v8::String::Utf8Value str(args[i]);
        const char* cstr = ToCString(str);
        printf("%s", cstr);
    }
    printf("\n");
    fflush(stdout);
    return v8::Undefined();
}


// The callback that is invoked by v8 whenever the JavaScript 'read'
// function is called.  This function loads the content of the file named in
// the argument into a JavaScript string.
v8::Handle<v8::Value> Read(const v8::Arguments& args) {
    if (args.Length() != 1) {
        return v8::ThrowException(v8::String::New("Bad parameters"));
    }
    v8::String::Utf8Value file(args[0]);
    if (*file == NULL) {
        return v8::ThrowException(v8::String::New("Error loading file"));
    }
    v8::Handle<v8::String> source = ReadFile(*file);
    if (source.IsEmpty()) {
        return v8::ThrowException(v8::String::New("Error loading file"));
    }
    return source;
}


// The callback that is invoked by v8 whenever the JavaScript 'load'
// function is called.  Loads, compiles and executes its argument
// JavaScript file.
v8::Handle<v8::Value> Load(const v8::Arguments& args) {
    for (int i = 0; i < args.Length(); i++) {
        v8::HandleScope handle_scope;
        v8::String::Utf8Value file(args[i]);
        if (*file == NULL) {
            return v8::ThrowException(v8::String::New("Error loading file"));
        }
        v8::Handle<v8::String> source = ReadFile(*file);
        if (source.IsEmpty()) {
            return v8::ThrowException(v8::String::New("Error loading file"));
        }
        if (!ExecuteString(source, v8::String::New(*file), false, true)) {
            return v8::ThrowException(v8::String::New("Error executing file"));
        }
    }
    return v8::Undefined();
}


// The callback that is invoked by v8 whenever the JavaScript 'quit'
// function is called.  Quits.
v8::Handle<v8::Value> Quit(const v8::Arguments& args) {
    // If not arguments are given args[0] will yield undefined which
    // converts to the integer value 0.
    int exit_code = args[0]->Int32Value();
    exit(exit_code);
    return v8::Undefined();
}


v8::Handle<v8::Value> Version(const v8::Arguments& args) {
    return v8::String::New(v8::V8::GetVersion());
}

// Reads a file into a v8 string.
v8::Handle<v8::String> ReadFile(const char* name) {
    FILE* file = fopen(name, "rb");
    if (file == NULL) return v8::Handle<v8::String>();

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);

    char* chars = new char[size + 1];
    chars[size] = '\0';
    for (int i = 0; i < size;) {
        int read = fread(&chars[i], 1, size - i, file);
        i += read;
    }
    fclose(file);
    v8::Handle<v8::String> result = v8::String::New(chars, size);
    delete[] chars;
    return result;
}


// The read-eval-execute loop of the shell.
void RunShell(v8::Handle<v8::Context> context) {
    printf("V8 version %s\n", v8::V8::GetVersion());
    static const int kBufferSize = 256;
    while (true) {
        char buffer[kBufferSize];
        printf("> ");
        char* str = fgets(buffer, kBufferSize, stdin);
        if (str == NULL) break;
        v8::HandleScope handle_scope;
        ExecuteString(v8::String::New(str),
            v8::String::New("(shell)"),
            true,
            true);
    }
    printf("\n");
}


// Executes a string within the current v8 context.
bool ExecuteString(v8::Handle<v8::String> source,
                   v8::Handle<v8::Value> name,
                   bool print_result,
                   bool report_exceptions) {
                       v8::HandleScope handle_scope;
                       v8::TryCatch try_catch;
                       v8::Handle<v8::Script> script = v8::Script::Compile(source, name);
                       if (script.IsEmpty()) {
                           // Print errors that happened during compilation.
                           if (report_exceptions)
                               ReportException(&try_catch);
                           return false;
                       } else {
                           v8::Handle<v8::Value> result = script->Run();
                           if (result.IsEmpty()) {
                               // Print errors that happened during execution.
                               if (report_exceptions)
                                   ReportException(&try_catch);
                               return false;
                           } else {
                               if (print_result && !result->IsUndefined()) {
                                   // If all went well and the result wasn't undefined then print
                                   // the returned value.
                                   v8::String::Utf8Value str(result);
                                   const char* cstr = ToCString(str);
                                   printf("%s\n", cstr);
                               }
                               return true;
                           }
                       }
}


void ReportException(v8::TryCatch* try_catch) {
    v8::HandleScope handle_scope;
    v8::String::Utf8Value exception(try_catch->Exception());
    const char* exception_string = ToCString(exception);
    v8::Handle<v8::Message> message = try_catch->Message();
    if (message.IsEmpty()) {
        // V8 didn't provide any extra information about this error; just
        // print the exception.
        printf("%s\n", exception_string);
    } else {
        // Print (filename):(line number): (message).
        v8::String::Utf8Value filename(message->GetScriptResourceName());
        const char* filename_string = ToCString(filename);
        int linenum = message->GetLineNumber();
        printf("%s:%i: %s\n", filename_string, linenum, exception_string);
        // Print line of source code.
        v8::String::Utf8Value sourceline(message->GetSourceLine());
        const char* sourceline_string = ToCString(sourceline);
        printf("%s\n", sourceline_string);
        // Print wavy underline (GetUnderline is deprecated).
        int start = message->GetStartColumn();
        for (int i = 0; i < start; i++) {
            printf(" ");
        }
        int end = message->GetEndColumn();
        for (int i = start; i < end; i++) {
            printf("^");
        }
        printf("\n");
    }
}
