#include <weHelper.h>
#include <weTask.h>
#include <weDispatch.h>
// for add_http_url
#include <weHttpInvent.h>
#include <html_js.h>

#include <fstream>
// from common/
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <boost/regex.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "audit_jscript.h"
#include "jscript.xpm"
#include "version.h"

#ifdef WIN32
double _HUGE;
#endif

using namespace webEngine;
namespace fs = boost::filesystem;

static string xrc = "<plugin id='audit_jscript'>\
<option name='' label='This plugin depends on HTTP Inventory and uses some it&apos;s settings' control='none'></option>\
<option name='audit_jscript/enable_jscript' label='Enable JavaScript processing' type='2' control='checkbox'>0</option>\
<option name='audit_jscript/preload' label='Preload JavaScript code' type='4' control='text'>./dom.js</option>\
<option name='audit_jscript/allow_network' label='Allow newtwork access' type='2' control='checkbox'>0</option>\
<option name='' label='This option allows JavaScript to load data from the server independently from scanning process.' control='none'></option>\
</plugin>";

static log4cxx::LoggerPtr js_logger;

static bool remove_empty(ajs_to_process_ptr obj)
{
    return (obj->pending_requests == 0);
}

audit_jscript::audit_jscript(engine_dispatcher* krnl, void* handle /*= NULL*/) :
    i_audit(krnl, handle)
{
    pluginInfo.interface_name = "audit_jscript";
    pluginInfo.interface_list.push_back("audit_jscript");
    pluginInfo.plugin_desc = "Search JavaScript content for links";
    pluginInfo.plugin_id = "9251BAB1B2C8"; //{70ED0090-75AF-4925-A546-9251BAB1B2C8}
    pluginInfo.plugin_icon = WeXpmToStringList(jscript_xpm, sizeof(jscript_xpm) / sizeof(char*) );
    js_logger = logger;
    thread_running = false;
    preloads_data = NULL;
    LOG4CXX_DEBUG(logger, _T("audit_jscript plugin created; version ") << VERSION_PRODUCTSTR);
}

audit_jscript::~audit_jscript(void)
{
    if (preloads_data != NULL) {
        delete preloads_data;
    }
}

i_plugin* audit_jscript::get_interface( const string& ifName )
{
    LOG4CXX_TRACE(logger, _T("audit_jscript::get_interface ") << ifName);
    if (iequals(ifName, "audit_jscript")) {
        LOG4CXX_DEBUG(logger, _T("audit_jscript::get_interface found!"));
        usageCount++;
        return (this);
    }
    return i_audit::get_interface(ifName);
}

const string audit_jscript::get_setup_ui( void )
{
    return xrc;
}

void audit_jscript::init( task* tsk )
{
    v8::Locker lock;
    we_option opt;
    // create list of the blocked extension
    parent_task = tsk;
    opt_use_js = tsk->IsSet(weoAuditJSenable);
    opt_allow_network = tsk->IsSet(weoAuditJSnetwork);
    opt = tsk->Option(weoAuditJSpreloads);
    SAFE_GET_OPTION_VAL(opt, opt_preloads, "");

    if (opt_use_js) {
        v8::HandleScope handle_scope;
        jsBrowser js_exec;
        js_exec.execute_string("echo('V8 Engine version is: ' + version())", "", true, true);
        if (opt_preloads != "") {
#ifdef WIN32
            boost::replace_all(opt_preloads, "/", "\\");
#else
            boost::replace_all(opt_preloads, "\\", "/");
#endif
            LOG4CXX_DEBUG(logger, "Execute JavaScript preloads from " << opt_preloads);
            try {
                size_t fsize = fs::file_size(fs::path(opt_preloads));
                ifstream ifs(opt_preloads.c_str());
                preloads_data = new char[fsize + 10];
                if(preloads_data != NULL) {
                    memset(preloads_data, 0, fsize+10);
                    ifs.read(preloads_data, fsize);
                    js_exec.execute_string(preloads_data, "", true, true);
#ifdef _DEBUG
                    LOG4CXX_TRACE(logger, "JavaScript preloads execution result: " << js_exec.get_results());
                    LOG4CXX_TRACE(logger, "JavaScript preloads execution result (dump): " << js_exec.dump("Object"));
#endif // _DEBUG
                }
            } catch(std::exception& e) {
                LOG4CXX_WARN(logger, "JavaScript preloads execution failed " << e.what());
            }
        }
    }
}

void audit_jscript::stop(task* tsk)
{
    LOG4CXX_DEBUG(logger, _T("audit_jscript::stop"));

    {
        boost::unique_lock<boost::mutex> lock(data_access);
        if (jscript_tasks.process_list.size() > 0 || jscript_tasks.task_list.size() > 0) {
            LOG4CXX_WARN(logger, _T("audit_jscript::stop - stop requested, but ") << jscript_tasks.task_list.size() <<
                         _T(" task(s) and ") << jscript_tasks.process_list.size() << _T(" process(es) are still in the queue"));
        }
        jscript_tasks.process_list.clear();
        jscript_tasks.task_list.clear();
    }
    {
        boost::lock_guard<boost::mutex> lock(thread_synch);
        thread_event.notify_all();
    }
}

void audit_jscript::process( task* tsk, boost::shared_ptr<ScanData>scData )
{
    LOG4CXX_TRACE(logger, _T("audit_jscript::process"));
    entity_list lst;
    html_document_ptr parsed;

    try {
        parsed = boost::shared_dynamic_cast<html_document>(scData->parsed_data);
    } catch(bad_cast) {
        LOG4CXX_ERROR(logger, _T("audit_jscript::process - can't process given document as html_document"));
        return;
    }

    if (opt_use_js) {
        base_path = scData->object_url;
        parent_task = tsk;
        if (parsed) {
            lst = parsed->FindTags("script");
            LOG4CXX_DEBUG(logger, _T("audit_jscript: found ") << lst.size() << _T(" scripts in ") << scData->object_url);
            if (lst.size() > 0) {
                // something to process
                ajs_to_process_ptr tp(new ajs_to_process(scData, parsed));

                // search for scripts to be downloaded
                for (size_t i = 0; i < lst.size(); i++) {
                    base_entity_ptr ent = lst[i];
                    if (ent != NULL) {
                        std::string attr = ent->attr("src");
                        if (attr != "") {
                            transport_url url;
                            url.assign_with_referer(attr, &base_path);
                            LOG4CXX_DEBUG(logger, _T("audit_jscript: need to download ") << url.tostring());
                            if (url.is_valid()) {
                                boost::unique_lock<boost::mutex> lock(data_access);
                                i_request_ptr req = jscript_tasks.add(url.tostring(), tp, ent);
                                if (req) {
                                    req->context = this;
                                    req->processor = i_audit::response_dispatcher;
                                    parent_task->get_request_async(req);
                                } // is not in list
                            } // is valid url
                            else {
                                LOG4CXX_WARN(logger, _T("audit_jscript: need to download - URL not valid"));
                            }
                        } // if need to download
                    } // if entity found
                } // for each entity
                ClearEntityList(lst);
                if (tp->pending_requests > 0) {
                    LOG4CXX_DEBUG(logger, _T("audit_jscript: need to download ") << tp->pending_requests << _T(" scripts, deffered processing"));
                } else { // to_process must be processed on next thread iteration
                    boost::unique_lock<boost::mutex> lock(data_access);
                    jscript_tasks.add(tp);
                }

                LOG4CXX_DEBUG(js_logger, _T("audit_jscript::process - tasks in queue: ") << jscript_tasks.task_list.size());
                LOG4CXX_DEBUG(js_logger, _T("audit_jscript::process - processes in queue: ") << jscript_tasks.process_list.size());

                if (tp->pending_requests == 0) {
                    // run thread, if it not started yet
                    if (!thread_running) {
                        boost::thread thrd(parser_thread, this);
                    }
                    // or wake it up
                    else {
                        boost::lock_guard<boost::mutex> lock(thread_synch);
                        thread_event.notify_all();
                    }
                } // if no pending requests

            } // if something to process
        }// if parsed_data
    } // if js_exec
    LOG4CXX_TRACE(logger, _T("audit_jscript::process - finished"));
}

void audit_jscript::process_response( i_response_ptr resp )
{
    string sc;
    string rurl = resp->RealUrl().tostring();
    HttpResponse *ht_resp = dynamic_cast<HttpResponse*>(resp.get());

    LOG4CXX_DEBUG(logger, _T("audit_jscript::process_response: ") << rurl);
    LOG4CXX_TRACE(logger, _T("audit_jscript::process_response: ENTER; to download: ") << jscript_tasks.task_list.size() << _T("; documents: ") << jscript_tasks.process_list.size() );

    {
        // auto-release mutex scope
        boost::unique_lock<boost::mutex> lock(data_access);
        ajs_download_queue::iterator mit = jscript_tasks.task_list.find(rurl);

        if (ht_resp->HttpCode() > 399) {
            LOG4CXX_WARN(logger, _T("audit_jscript::process_response - bad respose: ") << ht_resp->HttpCode() << _T(" ") << rurl);
        }
        if (mit == jscript_tasks.task_list.end()) {
            LOG4CXX_DEBUG(logger, _T("audit_jscript::process_response: unregistered URL ") << resp->RealUrl().tostring());
            for (mit = jscript_tasks.task_list.begin(); mit != jscript_tasks.task_list.end(); mit++) {
                if (AJS_DOWNLOAD_ID(mit) == resp->ID()) {
                    LOG4CXX_DEBUG(logger, _T("audit_jscript::process_response: responce found by identifier"));
                    break;
                } // if found
            } // foreach ajs_download_queue
        } // if URL not found
        // set entity data and decrement pending_requests counter
        if (mit != jscript_tasks.task_list.end()) {
            string code = "";
            if (ht_resp->HttpCode() >= 200 && ht_resp->HttpCode() < 300) {
                code.assign((char*)&(resp->Data()[0]), resp->Data().size());
            }

            LOG4CXX_DEBUG(logger, _T("audit_jscript::process_response: clear download task for ") << AJS_DOWNLOAD_URL(mit));
            for (size_t i = 0; i < AJS_DOWNLOAD_LIST(mit).size(); i++) {
                AJS_DOWNLOAD_LIST(mit)[i].first->attr("src", "");
                AJS_DOWNLOAD_LIST(mit)[i].first->attr("#code", code);
                AJS_DOWNLOAD_LIST(mit)[i].second->pending_requests--;
            }
            jscript_tasks.task_list.erase(mit);
        } else {
            LOG4CXX_WARN(logger, _T("audit_jscript::process_response: unregistered URL ") << resp->RealUrl().tostring());
            string dmp = "";
            for (mit = jscript_tasks.task_list.begin(); mit != jscript_tasks.task_list.end(); mit++) {
                dmp += mit->first;
                dmp += "\n\r";
            }
            LOG4CXX_TRACE(logger, _T("audit_jscript::process_response - the list \n\r") << dmp);
        }
    } // release mutex
    // process documents with no pending requests in separate thread

    // run thread, if it not started yet
    if (!thread_running) {
        thread_running = true;
        boost::thread thrd(parser_thread, this);
    }
    // or wake it up
    else {
        boost::lock_guard<boost::mutex> lock(thread_synch);
        thread_event.notify_all();
    }

    LOG4CXX_TRACE(logger, _T("audit_jscript::process_response: EXIT; to download: ") << jscript_tasks.task_list.size() << _T("; documents: ") << jscript_tasks.process_list.size() );
    return;
}

// clone of the http_inventory::add_url
void audit_jscript::add_url( webEngine::transport_url link, boost::shared_ptr<ScanData> sc )
{
    i_plugin* plg = parent_task->get_active_plugin("httpInventory");
    if (plg != NULL) {
        // just cast to save resources
        http_inventory *inv = (http_inventory*)plg;
        HttpResponse fake;
        // fill required field in the fake response
        fake.RealUrl(sc->object_url);
        fake.depth(sc->scan_depth);
        LOG4CXX_TRACE(logger, _T("audit_jscript::add_url - fall into the http_inventory::add_url"));
        inv->add_url(link, &fake, sc);
    } else {
        LOG4CXX_ERROR(logger, _T("audit_jscript::add_url can't find http_inventory plugin, can't add url ") << link.tostring());
    }
}

void audit_jscript::extract_links(boost::shared_ptr<ScanData> sc, const std::string &text)
{
    boost::smatch mres;
    std::string::const_iterator strt, end;
    boost::match_flag_type flags = boost::match_default;
    transport_url lurl;
    transport_url burl;

        burl.assign(sc->object_url);
        strt = text.begin();
        end = text.end();
        // 1. search strings that looks like URL
    boost::regex re1("([\\s\"\'^])(\\w+://[^\\s\"\']+)([\\s\"\'$])"); //, boost::regex_constants::icase

        while(regex_search(strt, end, mres, re1, flags)) {
            string tres = mres[2];
            lurl.assign_with_referer(tres, &burl);
        LOG4CXX_DEBUG(logger, _T("audit_jscript::extract_links: found url=") << tres << _T("; ") << lurl.tostring());
            add_url(lurl, sc);

            // update search position:
            strt = mres[0].second;
            // update flags:
            /*flags |= boost::match_prev_avail;
            flags |= boost::match_not_bob;*/
        }

        strt = text.begin();
        end = text.end();
        flags = boost::match_default;
        // 2. search for <a...> tags
        boost::regex re2("\\<a.*?href\\s*=\\s*(\"|')?([^\\s\"\']+).*?\\>"); //, boost::regex_constants::icase

        while(regex_search(strt, end, mres, re2, flags)) {
            string tres = mres[2];
            lurl.assign_with_referer(tres, &burl);
        LOG4CXX_DEBUG(logger, _T("audit_jscript::extract_links: found <a...> url=") << tres << _T("; ") << lurl.tostring());
            add_url(lurl, sc);

            // update search position:
            strt = mres[0].second;
            // update flags:
            /*flags |= boost::match_prev_avail;
            flags |= boost::match_not_bob;*/
        }

        strt = text.begin();
        end = text.end();
        flags = boost::match_default;
        // 2. search for "href: ..." output
        boost::regex re3("\\shref:\\s*([^\\s\"\']+)"); //, boost::regex_constants::icase

        while(regex_search(strt, end, mres, re3, flags)) {
            string tres = mres[1];
            lurl.assign_with_referer(tres, &burl);
        LOG4CXX_DEBUG(logger, _T("audit_jscript::extract_links: found 'href' url=") << tres << _T("; ") << lurl.tostring());
            add_url(lurl, sc);

            // update search position:
            strt = mres[0].second;
            // update flags:
            /*flags |= boost::match_prev_avail;
            flags |= boost::match_not_bob;*/
        }
}

void audit_jscript::extract_links( boost::shared_ptr<ScanData> sc, boost::shared_ptr<v8_wrapper::jsDocument> doc )
{
    try {
        for(v8_wrapper::iterator_dfs node = doc->begin_dfs(); node != doc->end_dfs(); ++node) {
            string text = (*node)->get_dump();
            if(text.empty())
                continue;
            extract_links(sc, text);
        }
    } catch (std::exception &e) {
        LOG4CXX_ERROR(logger, _T("audit_jscript::extract_links - exception: ") << std::string(e.what()))
    }
}

void audit_jscript::parse_scripts(boost::shared_ptr<ScanData> sc, boost::shared_ptr<webEngine::html_document> parser)
{
    LOG4CXX_TRACE(logger, _T("audit_jscript::parse_scripts for ") << (size_t)sc->parsed_data.get());
    entity_list lst;
    string source;

    if (!opt_use_js) {
        LOG4CXX_ERROR(logger, _T("No JsExecutor given - exit"));
        return;
    }
    parent_task->add_thread();
    if (parser) {
	v8::Locker lock;
        v8::HandleScope hscope;
        jsBrowser jse;
        v8::Persistent<v8::Context> ctx = jse.get_child_context();
        v8::Context::Scope scope(ctx);
        // execute preloads
        LOG4CXX_TRACE(logger, "audit_jscript::parse_scripts execute preloads");
        jse.execute_string(preloads_data, "", true, true);
        // fix location object
        jse.window->history->push_back(sc->object_url);
        jse.window->location->url.assign_with_referer(sc->object_url);
        // fix document object
        jse.window->assign_document(parser);
        // execute all scripts
        if (opt_allow_network) {
            jse.allow_network(parent_task);
        }
        LOG4CXX_DEBUG(logger, _T("audit_jscript::parse_scripts execute scripts for parsed_data ") << (size_t)parser.get());

        v8_wrapper::tree_node_list scripts;
        for(v8_wrapper::iterator_dfs it = jse.window->document->begin_dfs(); it != jse.window->document->end_dfs(); ++it) {
            if((*it)->m_tag == HTML_TAG_script && (*it)->m_entity) {
                scripts.push_back(*it);
            }
        }

        size_t i = 0;
        for(v8_wrapper::tree_node_list::iterator it = scripts.begin(); it != scripts.end(); ++it) {
            v8_wrapper::tree_node_list new_scripts;
            v8_wrapper::tree_node_ptr current_script = *it;
            while(true) {
                jse.window->document->v8_wrapper::Registrator<v8_wrapper::jsDocument>::m_data.m_execution_point = current_script;
                source = current_script->m_entity->attr("#code");
#ifdef _DEBUG
                LOG4CXX_TRACE(logger, _T("audit_jscript::parse_scripts execute script #") << i++ << L"; Source \"_T(" << current_script->m_entity->attr(")src_T(") << L")\":\n" << source);
#endif
                // @todo: add V8 synchronization - only one script can be processed at same time
                jse.execute_string(source, "", true, true);
#ifdef _DEBUG
                LOG4CXX_TRACE(logger, _T("ExecuteScript results: ") << jse.get_results());
#endif

                for(v8_wrapper::iterator_dfs it2 = current_script->begin_dfs(); it2 != current_script->end_dfs(); ++it2) {
                    if((*it2)->m_tag == HTML_TAG_script && (*it2)->m_entity) {
                        if(std::find(scripts.begin(), scripts.end(), *it2) == scripts.end())
                            new_scripts.push_back(*it2);
                    }
                }

                if(new_scripts.empty())
                    break;
                current_script = *new_scripts.begin();
                new_scripts.erase(new_scripts.begin());

                std::string src_url = current_script->m_entity->attr("src");
                source = current_script->m_entity->attr("#code");
                if(src_url != "" && source == "") {
                    transport_url url;
                    url.assign_with_referer(src_url, &base_path);
                    if (url.is_valid()) {
                        HttpResponse* hresp_;
                        i_response_ptr resp_;
                        while (true) {
                            HttpRequest* hreq_ = new HttpRequest(url.tostring());
                            hreq_->Method(HttpRequest::wemGet);
                            hreq_->depth(0);
                            hreq_->SetReferer(sc->object_url);
                            i_request_ptr req_(hreq_);
                            resp_ = jse.http_request(req_);
                            hresp_ = (HttpResponse*)resp_.get();
                            if (hresp_->HttpCode() < 300 || hresp_->HttpCode() >= 400) {
                                break;
                            }
                        }
                        if (hresp_->HttpCode() < 300) {
                            source.assign((const char*)&resp_->Data()[0], resp_->Data().size());
                            current_script->m_entity->attr("#code", source);
                        }
                    } // is valid url
                    else {
                        LOG4CXX_WARN(logger, _T("audit_jscript: need to download - URL not valid"));
                    }
            }
        }
        }

        LOG4CXX_DEBUG(logger, _T("audit_jscript::parse_scripts process events"));
        //html_document* doc_entity = sc->parsed_data.get();

        for(v8_wrapper::iterator_dfs it = jse.window->document->begin_dfs(); it!=jse.window->document->end_dfs(); ++it) {
            if ((*it)->m_tag != HTML_TAG___UNKNOWN_TAG__) {
                jse.window->document->v8_wrapper::Registrator<v8_wrapper::jsDocument>::m_data.m_execution_point = *it;
                const string event_result = process_events(&jse, ctx, *it);
                if(!event_result.empty())
                    extract_links(sc, event_result);
            }
        }

        jse.close_child_context(ctx);
        extract_links(sc, jse.window->document);
    }
    parent_task->remove_thread();
    LOG4CXX_TRACE(logger, _T("audit_jscript::parse_scripts finished"));
}

std::string audit_jscript::process_events(jsBrowser* jse, v8::Persistent<v8::Context> ctx, v8_wrapper::tree_node_ptr entity)
{
    ostringstream strout;
    v8::Context::Scope scope(ctx);
    /* simplest variant - recursive descent through DOM-tree */
    std::string src;
    std::string name;
    // get on... events
    LOG4CXX_DEBUG(iLogger::GetLogger(), _T("audit_jscript::process_events entity = ") << entity->m_entity->Name());
    if(entity->m_entity) {
        AttrMap::iterator attrib = entity->m_entity->attr_list().begin();
        ctx->Global()->Set(v8::String::New("__evt_target__"), entity->m_this);

        while(attrib != entity->m_entity->attr_list().end() ) {
        name = (*attrib).first;
        src = (*attrib).second;
            bool executed = false;

        if (boost::istarts_with(name, "on")) {
            // attribute name started with "on*" - this is the event
                LOG4CXX_INFO(iLogger::GetLogger(), _T("audit_jscript::process_events <") << entity->m_entity->Name() << _T(">.") << name << _T(" source = ") << src);
            boost::trim(src);
            if (! boost::istarts_with(src, "function")) {
                src = "__evt_target__.__event__" + name + "=function(){" + src + "}";
            } else {
                src = "__evt_target__.__event__" + name + "=" + src;
            }
            jse->execute_string(src, "", true, true);
                jse->execute_string("__evt_target__.__evt_result__ = __evt_target__.__event__" + name + "()", "", true, true);
                executed = true;
        }
        if (boost::istarts_with(src, "javascript:")) {
                LOG4CXX_INFO(iLogger::GetLogger(), _T("audit_jscript::process_events (proto) <") << entity->m_entity->Name() << _T(">.") << name << _T(" source = ") << src);
            src = src.substr(11); // skip "javascript:"
            src = "__evt_target__.__event__=function(){ " + src + " }";
            jse->execute_string(src, "", true, true);
                jse->execute_string("__evt_target__.__evt_result__ = __evt_target__.__event__()", "", true, true);
                executed = true;
            }
            if(executed) {
                v8::Local<v8::Value> result = entity->m_this->Get(v8::String::New("__evt_result__"));
                if(!result.IsEmpty()) {
                    if(result->IsString()) {
                        strout << value_to_string(result);
                    } else if(result->IsObject()) {
                        v8::Local<v8::Object> obj = result->ToObject();
                        v8::Local<v8::Array> props = obj->GetPropertyNames();
                        for(uint32_t i = 0; i < props->Length(); ++i) {
                            v8::Local<v8::Value> prop = props->Get(v8::Number::New(i));
                            if(!prop.IsEmpty()) {
                                if (prop->IsString())
                                    strout << value_to_string(prop);
                                v8::Local<v8::Value> val = obj->Get(prop);
                                if(!val.IsEmpty() && val->IsString())
                                    strout << "=\"" << value_to_string(val) << "\" ";
                            }
                        }
                    }
                }
            }
            ++attrib;
        }
    }
    return strout.str();
}

namespace webEngine {
void parser_thread( audit_jscript* object )
{
    bool in_loop = true;
    size_t task_list;
    ajs_to_process_list local_list;

    LOG4CXX_DEBUG(js_logger, _T("audit_jscript::parser_thread started"));
    object->thread_running = true;
    object->parent_task->add_thread();

    local_list.clear();
    while (in_loop) {
        {
            // wait scope: check for completion
            boost::unique_lock<boost::mutex> lock(object->data_access);
            task_list = object->jscript_tasks.task_list.size() + object->jscript_tasks.process_list.size();
            if (task_list == 0) {
                LOG4CXX_TRACE(js_logger, _T("audit_jscript::parser_thread no data in task lists - exiting"));
                in_loop = false;
            }
        } // wait scope - auto release mutex
        if (!in_loop) {
            break;
        }

        // perform actions

        {
            // wait scope: extract data from process_list
            boost::unique_lock<boost::mutex> lock(object->data_access);
            for (size_t i = 0; i < object->jscript_tasks.process_list.size(); i++) {
                if (object->jscript_tasks.process_list[i]->pending_requests == 0) {
                    local_list.push_back(object->jscript_tasks.process_list[i]);
                }
            }
            object->jscript_tasks.clean_done();
        } // wait scope - auto release mutex
        LOG4CXX_DEBUG(js_logger, _T("audit_jscript::parser_thread: to download: ") << object->jscript_tasks.task_list.size() <<
                      _T("; documents: ") << object->jscript_tasks.process_list.size() );

        if (local_list.size() > 0) {
            LOG4CXX_DEBUG(js_logger, _T("audit_jscript::parser_thread: ") << local_list.size() << _T(" documents to process"));
            for (size_t i = 0; i < local_list.size(); i++) {
                object->parse_scripts(local_list[i]->sc_data, local_list[i]->parsed_data);
            }
            local_list.clear();
        } else {
            // we don't found any documents to process
            boost::unique_lock<boost::mutex> lock(object->data_access);
            task_list = object->jscript_tasks.task_list.size() + object->jscript_tasks.process_list.size();
            LOG4CXX_DEBUG(js_logger, _T("audit_jscript::parser_thread waiting for data: ") << object->jscript_tasks.task_list.size() <<
                          _T(" to download and ") << object->jscript_tasks.process_list.size() << _T(" documents still in the list"));
            in_loop = false;
        }
        if (!in_loop) {
            break;
        }

//         { // get data size
//             boost::unique_lock<boost::mutex> lock(object->data_access);
//             task_list = object->jscript_tasks.task_list.size() + object->jscript_tasks.process_list.size();
//             LOG4CXX_TRACE(js_logger, _T("audit_jscript::parser_thread waiting for data: ") << task_list << _T(" processes in list"));
//         } // wait scope - auto release mutex
//         if (task_list > 0){ // wait for data
//             boost::unique_lock<boost::mutex> thrd_lock(object->thread_synch);
//             object->thread_event.wait(thrd_lock);
//         } // wait scope - auto release mutex
    } // main thread loop

    object->parent_task->remove_thread();
    object->thread_running = false;
    LOG4CXX_DEBUG(js_logger, _T("audit_jscript::parser_thread finished"));
}
}

ajs_to_process::ajs_to_process( webEngine::scan_data_ptr sc , boost::shared_ptr<html_document> pd )
    : sc_data(sc),
      parsed_data(pd),
      pending_requests(0) {}

i_request_ptr ajs_download_queue::add( string url, ajs_to_process_ptr tp, webEngine::base_entity_ptr ent )
{
    HttpRequest* result = NULL;

    LOG4CXX_TRACE(js_logger, _T("ajs_download_queue::add new task for URL: ") << url);
    ajs_download_queue::iterator mit = task_list.find(url);

    if (mit == task_list.end()) {
        // generate new identifier
        boost::uuids::basic_random_generator<boost::mt19937> gen;
        boost::uuids::uuid tag = gen();
        string req_uuid = boost::lexical_cast<string>(tag);
        // create request
        result = new webEngine::HttpRequest(url);
        result->depth(0);
        result->ID(req_uuid);
        // save download task
        task_list[url] = ajs_download_task(req_uuid, ajs_download_list(1, ajs_download(ent, tp)));

    } else {
        AJS_DOWNLOAD_LIST(mit).push_back(ajs_download(ent, tp));
    }
    LOG4CXX_DEBUG(js_logger, _T("ajs_download_queue::add - processes in queue: ") << process_list.size());
    ajs_to_process_list::iterator lit = std::find(process_list.begin(), process_list.end(), tp);
    if (lit == process_list.end()) {
        LOG4CXX_DEBUG(js_logger, _T("ajs_download_queue::add - processes in queue add!!!"));
        process_list.push_back(tp);
    }
    tp->pending_requests++;
    LOG4CXX_TRACE(js_logger, _T("ajs_download_queue::add - pending requests for this task: ") << tp->pending_requests);
    LOG4CXX_DEBUG(js_logger, _T("ajs_download_queue::add - tasks in queue: ") << task_list.size());
    LOG4CXX_TRACE(js_logger, _T("ajs_download_queue::add - processes in queue: ") << process_list.size());

    return i_request_ptr(result);
}

bool ajs_download_queue::add( ajs_to_process_ptr tp )
{
    bool result = false;
    LOG4CXX_TRACE(js_logger, _T("ajs_download_queue::add new task to execute"));
    LOG4CXX_DEBUG(js_logger, _T("ajs_download_queue::add - processes in queue: ") << process_list.size());
    ajs_to_process_list::iterator lit = std::find(process_list.begin(), process_list.end(), tp);
    if (lit == process_list.end()) {
        LOG4CXX_DEBUG(js_logger, _T("ajs_download_queue::add - processes in queue add!!!"));
        process_list.push_back(tp);
        result = true;
    }
    LOG4CXX_DEBUG(js_logger, _T("ajs_download_queue::add - processes in queue: ") << process_list.size());
    return result;
}

void ajs_download_queue::remove_request( string req_id )
{
}

void ajs_download_queue::clean_done( )
{
    process_list.erase(std::remove_if(process_list.begin(), process_list.end(), remove_empty), process_list.end());
}
