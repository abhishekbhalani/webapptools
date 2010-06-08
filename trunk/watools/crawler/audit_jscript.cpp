#include "audit_jscript.h"
#include <weHelper.h>
#include <weTask.h>
// for add_http_url
#include <weHttpInvent.h>
#include <boost/regex.hpp>
#include <boost/thread.hpp>
//! @todo: upgrade to the boost_1.42 to use native version
#include "boost/uuid.hpp"
#include "boost/uuid_generators.hpp"
#include "boost/uuid_io.hpp"

using namespace webEngine;

jsExecutor* audit_jscript::js_exec = NULL;
static log4cxx::LoggerPtr js_logger;

static bool remove_empty(ajs_to_process_ptr obj)
{
    return (obj->pending_requests == 0);
}

audit_jscript::audit_jscript(webEngine::engine_dispatcher* krnl, void* handle /*= NULL*/) :
    i_audit(krnl, handle)
{
    pluginInfo.interface_name = "audit_jscript";
    pluginInfo.interface_list.push_back("audit_jscript");
    pluginInfo.plugin_desc = "Search JavaScript content for links";
    pluginInfo.plugin_id = "9251BAB1B2C8"; //{70ED0090-75AF-4925-A546-9251BAB1B2C8}
    js_logger = logger;
    thread_running = false;
}

audit_jscript::~audit_jscript(void)
{
}

i_plugin* audit_jscript::get_interface( const string& ifName )
{
    LOG4CXX_TRACE(logger, "audit_jscript::get_interface " << ifName);
    if (iequals(ifName, "audit_jscript"))
    {
        LOG4CXX_DEBUG(logger, "audit_jscript::get_interface found!");
        usageCount++;
        return (this);
    }
    return i_audit::get_interface(ifName);
}

void audit_jscript::init( task* tsk )
{
    string text;
    wOption opt;

    // create list of the blocked extension
    parent_task = tsk;
    opt = tsk->Option("httpInventory/"weoDeniedFileTypes);
    SAFE_GET_OPTION_VAL(opt, text, 1);
    if (text != "")
    {
        size_t pos = text.find(';');
        ext_deny.clear();
        while(pos != string::npos) {
            string ext = text.substr(0, pos);
            ext_deny.push_back(ext);
            if (pos < text.length())
            {
                text = text.substr(pos+1);
            }
            else {
                text = "";
            }
            pos = text.find(';');
        }
    }
    // processing options
    opt_in_host = parent_task->IsSet("httpInventory/"weoStayInHost);
    opt_in_domain = parent_task->IsSet("httpInventory/"weoStayInDomain);
    opt_ignore_param = parent_task->IsSet("httpInventory/"weoIgnoreUrlParam);
    opt = parent_task->Option("httpInventory/"weoScanDepth);
    SAFE_GET_OPTION_VAL(opt, opt_max_depth, 0);
}

void audit_jscript::stop(webEngine::task* tsk)
{
    LOG4CXX_DEBUG(logger, "audit_jscript::stop");

    {
        boost::unique_lock<boost::mutex> lock(data_access);
        if (jscript_tasks.process_list.size() > 0 || jscript_tasks.task_list.size() > 0) {
            LOG4CXX_WARN(logger, "audit_jscript::stop - stop requested, but " << jscript_tasks.task_list.size() <<
                " task(s) and " << jscript_tasks.process_list.size() << " process(es) are still in the queue");
        }
        jscript_tasks.process_list.clear();
        jscript_tasks.task_list.clear();
    }
    {
        boost::lock_guard<boost::mutex> lock(thread_synch);
        thread_event.notify_all();
    }
}

void audit_jscript::process( webEngine::task* tsk, boost::shared_ptr<ScanData>scData )
{
    LOG4CXX_TRACE(logger, "audit_jscript::process");
    entity_list lst;
    html_document_ptr parsed;

    try {
        parsed = boost::shared_dynamic_cast<html_document>(scData->parsed_data);
    }
    catch(bad_cast) {
        LOG4CXX_ERROR(logger, "audit_jscript::process - can't process given document as html_document");
        return;
    }

    if (js_exec != NULL) {
        base_path = scData->object_url;
        parent_task = tsk;
        if (parsed) {
            lst = parsed->FindTags("script");
            LOG4CXX_DEBUG(logger, "audit_jscript: found " << lst.size() << " scripts in " << scData->object_url);
            if (lst.size() > 0)
            {
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
                            LOG4CXX_DEBUG(logger, "audit_jscript: need to download " << url.tostring());
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
                                LOG4CXX_WARN(logger, "audit_jscript: need to download - URL not valid");
                            }
                        } // if need to download
                    } // if entity found
                } // for each entity
                ClearEntityList(lst);
                if (tp->pending_requests > 0) {
                    LOG4CXX_DEBUG(logger, "audit_jscript: need to download " << tp->pending_requests << " scripts, deffered processing");
                }
                else { // to_process must be processed on next thread iteration
                    boost::unique_lock<boost::mutex> lock(data_access);
                    jscript_tasks.add(tp);
                }

                LOG4CXX_DEBUG(js_logger, "audit_jscript::process - tasks in queue: " << jscript_tasks.task_list.size());
                LOG4CXX_DEBUG(js_logger, "audit_jscript::process - processes in queue: " << jscript_tasks.process_list.size());
                
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
    LOG4CXX_TRACE(logger, "audit_jscript::process - finished");
}

void audit_jscript::process_response( webEngine::i_response_ptr resp )
{
    string sc;
    string rurl = resp->RealUrl().tostring();
    HttpResponse *ht_resp = dynamic_cast<HttpResponse*>(resp.get());

    LOG4CXX_DEBUG(logger, "audit_jscript::process_response: " << rurl);
    LOG4CXX_TRACE(logger, "audit_jscript::process_response: ENTER; to download: " << jscript_tasks.task_list.size() << "; documents: " << jscript_tasks.process_list.size() );

    { // auto-release mutex scope
        boost::unique_lock<boost::mutex> lock(data_access);
        ajs_download_queue::iterator mit = jscript_tasks.task_list.find(rurl);

        if (ht_resp->HttpCode() > 399)
        {
            LOG4CXX_WARN(logger, "audit_jscript::process_response - bad respose: " << ht_resp->HttpCode() << " " << rurl);
        }
        if (mit == jscript_tasks.task_list.end()) {
            LOG4CXX_DEBUG(logger, "audit_jscript::process_response: unregistered URL " << resp->RealUrl().tostring());
            for (mit = jscript_tasks.task_list.begin(); mit != jscript_tasks.task_list.end(); mit++) {
                if (AJS_DOWNLOAD_ID(mit) == resp->ID()) {
                    LOG4CXX_DEBUG(logger, "audit_jscript::process_response: responce found by identifier");
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

            LOG4CXX_DEBUG(logger, "audit_jscript::process_response: clear download task for " << AJS_DOWNLOAD_URL(mit));
            for (size_t i = 0; i < AJS_DOWNLOAD_LIST(mit).size(); i++) {
                AJS_DOWNLOAD_LIST(mit)[i].first->attr("src", "");
                AJS_DOWNLOAD_LIST(mit)[i].first->attr("#code", code);
                AJS_DOWNLOAD_LIST(mit)[i].second->pending_requests--;
            }
            jscript_tasks.task_list.erase(mit);
        }
        else {
            LOG4CXX_WARN(logger, "audit_jscript::process_response: unregistered URL " << resp->RealUrl().tostring());
            string dmp = "";
            for (mit = jscript_tasks.task_list.begin(); mit != jscript_tasks.task_list.end(); mit++) {
                dmp += mit->first;
                dmp += "\n\r";
            }
            LOG4CXX_TRACE(logger, "audit_jscript::process_response - the list \n\r" << dmp);
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

    LOG4CXX_TRACE(logger, "audit_jscript::process_response: EXIT; to download: " << jscript_tasks.task_list.size() << "; documents: " << jscript_tasks.process_list.size() );
    return;
}

// clone of the HttpInventory::add_url
void audit_jscript::add_url( webEngine::transport_url link, boost::shared_ptr<ScanData> sc )
{
    bool allowed = true;

    if (!link.is_host_equal(sc->object_url))
    {
        if (opt_in_host)
        {
            allowed = false;
        }
        LOG4CXX_TRACE(logger, "audit_jscript::add_url: weoStayInHost check " << allowed << " (" << link.tostring() << ")");
    }
    if (!link.is_domain_equal(sc->object_url))
    {
        if (opt_in_domain)
        {
            allowed = false;
        }
        LOG4CXX_TRACE(logger, "audit_jscript::add_url: weoStayInDomain check << " << allowed << " (" << link.tostring() << ")");
    }
    if ( opt_max_depth > 0 && sc->scan_depth >= opt_max_depth) {
        LOG4CXX_DEBUG(logger, "audit_jscript::add_url: maximum scanning depth reached! (" << opt_max_depth << ")");
        allowed = false;
    }
    if (allowed)
    {    // verify blocked file types
        string path = link.request;
        int pos = path.find_last_of('.');
        if (pos != string::npos && ext_deny.size() > 0)
        {
            path = path.substr(pos+1);
            LOG4CXX_TRACE(logger, "audit_jscript::add_url: Found extension: " << path << "; Deny list size is " << ext_deny.size());
            for (size_t i = 0; i < ext_deny.size(); i++) {
                if (path == ext_deny[i]) {
                    allowed = false;
                    LOG4CXX_DEBUG(logger, "audit_jscript::add_http_url: not need to download " << link.tostring());
                    // make the pseudo-response
                    boost::shared_ptr<ScanData> scn = parent_task->GetScanData(link.tostring());
                    if (scn->data_id == "")
                    {
                        //scData->data_id = ;
                        scn->resp_code = 204; // 204 No Content;  The server successfully processed the request, but is not returning any content
                        scn->download_time = 0;
                        scn->data_size = 0;
                        scn->scan_depth = sc->scan_depth + 1;
                        scn->content_type = "application/octet-stream";
                        scn->parsed_data.reset();
                        parent_task->SetScanData(scn->object_url, scn);
                    }
                    break;
                } // if path == ext_deny[i]
            } // for deny list size
        } // if file extension found
    } // if allowed
    if (allowed)
    {
        string u_req = link.tostring();
        if (opt_ignore_param) {
            u_req = link.tostring_noparam();
        }
        LOG4CXX_TRACE(logger, "audit_jscript::add_http_url: weoIgnoreUrlParam check << " << u_req);
        if (!parent_task->is_url_processed(u_req))
        {
            HttpRequest* new_url = new HttpRequest(u_req);
            new_url->depth(sc->scan_depth + 1);
            new_url->ID(sc->data_id);
            // to send response to the inventories
            new_url->processor = NULL;
            new_url->context = NULL; 
            parent_task->get_request_async(i_request_ptr(new_url));
        }
        else
        {
            // add parent to existing scan data
        }
        parent_task->register_url(u_req);
    }
}

void audit_jscript::parse_scripts(boost::shared_ptr<ScanData> sc, boost::shared_ptr<webEngine::html_document> parser)
{
    LOG4CXX_TRACE(logger, "audit_jscript::parse_scripts for " << sc->parsed_data);
    entity_list lst;

    if (js_exec == NULL) {
        LOG4CXX_ERROR(logger, "No JsExecutor given - exit");
        return;
    }
    parent_task->add_thread();
    if (parser) {
        // execute all scripts
        LOG4CXX_DEBUG(logger, "audit_jscript::parse_scripts execute scripts for parsed_data " << parser);
        lst = parser->FindTags("script");
        for (size_t i = 0; i < lst.size(); i++) {
            base_entity_ptr ent = lst[i];
            if (ent != NULL) {
                string source = ent->attr("#code");
#ifdef _DEBUG
                LOG4CXX_TRACE(logger, "audit_jscript::parse_scripts execute script #" << i << "; Source:\n" << source);
#endif
                // @todo: add V8 synchronization - only one script can be processed at same time
                js_exec->execute_string(source, "", true, true);
#ifdef _DEBUG
                LOG4CXX_TRACE(logger, "ExecuteScript results: " << js_exec->get_results());
#endif
            }
        }
        ClearEntityList(lst);
        LOG4CXX_DEBUG(logger, "audit_jscript::parse_scripts process events");
        //html_document* doc_entity = sc->parsed_data.get();
        process_events(parser);
        // process results
        string res;
        res = js_exec->get_results();
        // res += js_exec->dump("Object");
        js_exec->reset_results();
#ifdef _DEBUG
        LOG4CXX_TRACE(logger, "audit_jscript::parse_scripts results:\n" << res);
#endif
        extract_links(res, sc);
    }
    parent_task->remove_thread();
    LOG4CXX_INFO(logger, "TRACE: audit_jscript::parse_scripts finished");
}

void audit_jscript::extract_links( string text, boost::shared_ptr<ScanData> sc )
{
    boost::smatch mres;
    std::string::const_iterator strt, end; 
    boost::match_flag_type flags = boost::match_default; 
    transport_url lurl;

    try {
        strt = text.begin(); 
        end = text.end();
        // 1. search strings that looks like URL
        boost::regex re1("(\\s|^)(\\w+://[^\\s\"\']+)(\\s|$)"); //, boost::regex_constants::icase

        while(regex_search(strt, end, mres, re1, flags)) {
            string tres = mres[2];
            LOG4CXX_DEBUG(logger, "audit_jscript::extract_links: found url=" << tres);
            lurl.assign_with_referer(tres, &base_path);
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
            LOG4CXX_DEBUG(logger, "audit_jscript::extract_links: found <a...> url=" << tres);
            lurl.assign_with_referer(tres, &base_path);
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
            LOG4CXX_DEBUG(logger, "audit_jscript::extract_links: found 'href' url=" << tres);
            lurl.assign_with_referer(tres, &base_path);
            add_url(lurl, sc);

            // update search position: 
            strt = mres[0].second; 
            // update flags: 
            /*flags |= boost::match_prev_avail; 
            flags |= boost::match_not_bob;*/
        }
    }
    catch (std::exception &e) {
        LOG4CXX_ERROR(logger, "audit_jscript::extract_links - exception: " << e.what())
    }
}

void audit_jscript::process_events( webEngine::base_entity_ptr entity )
{
/* simple variant - recursive descent through DOM-tree */
    std::string src;
    std::string name;
    // get on... events
    LOG4CXX_DEBUG(logger, "audit_jscript::process_events entity = " << entity->Name());

    webEngine::AttrMap::iterator attrib = entity->attr_list().begin();

    while(attrib != entity->attr_list().end() ) {
        name = entity->attr_list().key(attrib);
        src = entity->attr_list().val(attrib);

        if (boost::istarts_with(name, "on")) {
            // attribute name started with "on*" - this is the event
#ifdef _DEBUG
            LOG4CXX_TRACE(logger, "audit_jscript::process_events - " << name << " source = " << src);
#else
            LOG4CXX_DEBUG(logger, "audit_jscript::process_events - " << name);
#endif
            src = "(function(){ " + src + " })()";
            js_exec->execute_string(src, "", true, true);
        }
        if (boost::istarts_with(src, "javascript:")) {
#ifdef _DEBUG
            LOG4CXX_TRACE(logger, "audit_jscript::process_events - " << name << " source = " << src);
#else
            LOG4CXX_DEBUG(logger, "audit_jscript::process_events - " << name);
#endif
            src = src.substr(11); // skip "javascript:"
            src = "(function(){ " + src + " })()";
            js_exec->execute_string(src, "", true, true);
        }

        attrib++;
    }

    // and process all children
    entity_list chld = entity->Children();
    entity_list::iterator  it;

    for(size_t i = 0; i < chld.size(); i++) {
        std::string nm = chld[i]->Name();
        if (nm[0] != '#') {
            process_events(chld[i]);
        }
    }
    // all results will be processed in the caller parse_scripts function
}

void parser_thread( audit_jscript* object )
{
    bool in_loop = true;
    size_t task_list;
    ajs_to_process_list local_list;

    LOG4CXX_DEBUG(js_logger, "audit_jscript::parser_thread started");
    object->thread_running = true;
    object->parent_task->add_thread();

    local_list.clear();
    while (in_loop) {
        { // wait scope: check for completion
            boost::unique_lock<boost::mutex> lock(object->data_access);
            task_list = object->jscript_tasks.task_list.size() + object->jscript_tasks.process_list.size();
            if (task_list == 0) {
                LOG4CXX_TRACE(js_logger, "audit_jscript::parser_thread no data in task lists - exiting");
                in_loop = false;
            }
        } // wait scope - auto release mutex
        if (!in_loop) {
            break;
        }

        // perform actions

        { // wait scope: extract data from process_list
            boost::unique_lock<boost::mutex> lock(object->data_access);
            for (size_t i = 0; i < object->jscript_tasks.process_list.size(); i++) {
                if (object->jscript_tasks.process_list[i]->pending_requests == 0) {
                    local_list.push_back(object->jscript_tasks.process_list[i]);
                }
            }
            object->jscript_tasks.clean_done();
        } // wait scope - auto release mutex
        LOG4CXX_INFO(js_logger, "DEBUG: audit_jscript::parser_thread: to download: " << object->jscript_tasks.task_list.size() <<
            "; documents: " << object->jscript_tasks.process_list.size() );

        if (local_list.size() > 0)
        {
            LOG4CXX_DEBUG(js_logger, "audit_jscript::parser_thread: " << local_list.size() << " documents to process");
            for (size_t i = 0; i < local_list.size(); i++) {
                object->parse_scripts(local_list[i]->sc_data, local_list[i]->parsed_data);
            }
            local_list.clear();
        }
        else {
            // we don't found any documents to process
            boost::unique_lock<boost::mutex> lock(object->data_access);
            task_list = object->jscript_tasks.task_list.size() + object->jscript_tasks.process_list.size();
            LOG4CXX_INFO(js_logger, "DEBUG: audit_jscript::parser_thread waiting for data: " << object->jscript_tasks.task_list.size() <<
                " to download and " << object->jscript_tasks.process_list.size() << " documents still in the list");
            in_loop = false;
        }
        if (!in_loop) {
            break;
        }

//         { // get data size
//             boost::unique_lock<boost::mutex> lock(object->data_access);
//             task_list = object->jscript_tasks.task_list.size() + object->jscript_tasks.process_list.size();
//             LOG4CXX_TRACE(js_logger, "audit_jscript::parser_thread waiting for data: " << task_list << " processes in list");
//         } // wait scope - auto release mutex
//         if (task_list > 0){ // wait for data
//             boost::unique_lock<boost::mutex> thrd_lock(object->thread_synch);
//             object->thread_event.wait(thrd_lock);
//         } // wait scope - auto release mutex
    } // main thread loop

    object->parent_task->remove_thread();
    object->thread_running = false;
    LOG4CXX_DEBUG(js_logger, "audit_jscript::parser_thread finished");
}

ajs_to_process::ajs_to_process( webEngine::scan_data_ptr sc /*= webEngine::scan_data_ptr()*/, boost::shared_ptr<html_document> pd /*= boost::shared_ptr<webEngine::html_document>()*/ )
{
    sc_data = sc;
    parsed_data = pd;
    pending_requests = 0;
}

i_request_ptr ajs_download_queue::add( string url, ajs_to_process_ptr tp, webEngine::base_entity_ptr ent )
{
    HttpRequest* result = NULL;

    LOG4CXX_TRACE(js_logger, "ajs_download_queue::add new task for URL: " << url);
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

    }
    else {
        AJS_DOWNLOAD_LIST(mit).push_back(ajs_download(ent, tp));
    }
    LOG4CXX_DEBUG(js_logger, "ajs_download_queue::add - processes in queue: " << process_list.size());
    ajs_to_process_list::iterator lit = std::find(process_list.begin(), process_list.end(), tp);
    if (lit == process_list.end()) {
        LOG4CXX_DEBUG(js_logger, "ajs_download_queue::add - processes in queue add!!!");
        process_list.push_back(tp);
    }
    tp->pending_requests++;
    LOG4CXX_TRACE(js_logger, "ajs_download_queue::add - pending requests for this task: " << tp->pending_requests);
    LOG4CXX_DEBUG(js_logger, "ajs_download_queue::add - tasks in queue: " << task_list.size());
    LOG4CXX_TRACE(js_logger, "ajs_download_queue::add - processes in queue: " << process_list.size());

    return i_request_ptr(result);
}

bool ajs_download_queue::add( ajs_to_process_ptr tp )
{
    bool result = false;
    LOG4CXX_TRACE(js_logger, "ajs_download_queue::add new task to execute");
    LOG4CXX_DEBUG(js_logger, "ajs_download_queue::add - processes in queue: " << process_list.size());
    ajs_to_process_list::iterator lit = std::find(process_list.begin(), process_list.end(), tp);
    if (lit == process_list.end()) {
        LOG4CXX_DEBUG(js_logger, "ajs_download_queue::add - processes in queue add!!!");
        process_list.push_back(tp);
        result = true;
    }
    LOG4CXX_DEBUG(js_logger, "ajs_download_queue::add - processes in queue: " << process_list.size());
    return result;
}

void ajs_download_queue::remove_request( string req_id )
{
}

void ajs_download_queue::clean_done( )
{
    process_list.erase(std::remove_if(process_list.begin(), process_list.end(), remove_empty), process_list.end());
}