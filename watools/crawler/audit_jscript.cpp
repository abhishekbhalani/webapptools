#include "audit_jscript.h"
#include <weHelper.h>
#include <weTask.h>
// for add_http_url
#include <weHttpInvent.h>
#include <boost/regex.hpp>
#include <boost/thread.hpp>

using namespace webEngine;

jsExecutor* audit_jscript::js_exec = NULL;

typedef struct {
    string url;
    boost::shared_ptr<ScanData> sc_data;
    boost::shared_ptr<HtmlDocument> parser;
    iEntityPtr ent;
} download_data;

typedef struct {
    audit_jscript* object;
    boost::shared_ptr<ScanData> sc_data;
    boost::shared_ptr<HtmlDocument> parser;
    log4cxx::LoggerPtr logger;
} thread_data;

static bool remove_empty(void* obj)
{
    return (obj == NULL);
}

audit_jscript::audit_jscript(webEngine::engine_dispatcher* krnl, void* handle /*= NULL*/) :
    i_audit(krnl, handle)
{
    pluginInfo.interface_name = "audit_jscript";
    pluginInfo.interface_list.push_back("audit_jscript");
    pluginInfo.plugin_desc = "Search JavaScript content for links";
    pluginInfo.plugin_id = "9251BAB1B2C8"; //{70ED0090-75AF-4925-A546-9251BAB1B2C8}
}

audit_jscript::~audit_jscript(void)
{
    for(size_t i = 0; i < to_download.size(); i++) {
        if (to_download[i] != NULL) {
            delete to_download[i];
        }
    }
    to_download.clear();
    scandatas.clear();
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

void audit_jscript::start( webEngine::task* tsk, boost::shared_ptr<ScanData>scData )
{
    LOG4CXX_TRACE(logger, "audit_jscript::start");
    EntityList lst;
    int is_download;

    if (js_exec != NULL) {
        base_path = scData->object_url;
        parent_task = tsk;
        if (scData->parsed_data != NULL) {
            lst = scData->parsed_data->FindTags("script");
            LOG4CXX_DEBUG(logger, "audit_jscript: found " << lst.size() << " scripts in " << scData->object_url);
            // search for scripts to be downloaded
            is_download = 0;
            for (size_t i = 0; i < lst.size(); i++) {
                iEntityPtr ent = lst[i];
                if (ent != NULL) {
                    std::string attr = ent->Attr("src");
                    if (attr != "") {
                        transport_url url;
                        url.assign_with_referer(attr, &base_path);
                        LOG4CXX_DEBUG(logger, "audit_jscript: need to download " << url.tostring());
                        if (scandatas.find(scData->object_url) == scandatas.end() ) {
                            scandatas[scData->object_url] = 0;
                        }
                        download_data *dnld = new download_data;
                        dnld->url = url.tostring();
                        dnld->sc_data = scData;
                        dnld->parser = scData->parsed_data;
                        dnld->ent = ent;
                        to_download.push_back(dnld);
                        scandatas[scData->object_url]++;
                        is_download++;

                        webEngine::HttpRequest *req = new webEngine::HttpRequest(dnld->url);
                        req->depth(scData->scan_depth + 1);
                        req->context = this;
                        req->processor = i_audit::response_dispatcher;
                        parent_task->get_request_async(req);
                    }
                }
            }
            ClearEntityList(lst);
            if (is_download > 0) {
                LOG4CXX_DEBUG(logger, "audit_jscript: need to download " << is_download << " scripts, deffered processing");
            }
            else {
                vector<thread_data>* th_args = new vector<thread_data>;
                thread_data tdata;
                tdata.object = this;
                tdata.sc_data = scData;
                tdata.logger = logger;
                th_args->push_back(tdata);
                boost::thread process(parser_thread, th_args);
            }
        }// if parsed_data
    } // if js_exec
    LOG4CXX_TRACE(logger, "audit_jscript::start - finished");
}

void audit_jscript::process_response( webEngine::i_response_ptr resp )
{
    string sc;
    vector<thread_data>* th_args = new vector<thread_data>;

    LOG4CXX_DEBUG(logger, "audit_jscript::process_response: " << resp->RealUrl().tostring());
    size_t i;
    string rurl = resp->RealUrl().tostring();
    map<string, int>::iterator mit;

    LOG4CXX_DEBUG(logger, "audit_jscript::process_response: ENTER to download: " << to_download.size() << "; scandatas: " << scandatas.size() );
    th_args->clear();
    for (i = 0; i < to_download.size(); i++) {
        download_data *dnld = (download_data*)to_download[i];
        if (dnld != NULL) {
            string code((char*)&(resp->Data()[0]), resp->Data().size());
            if (dnld->url == rurl) {
                dnld->ent->Attr("src", "");
                dnld->ent->Attr("#code", code);
                scandatas[dnld->sc_data->object_url]--;
                // run parser thread for ready object
                if (scandatas[dnld->sc_data->object_url] <= 0) {
                    thread_data tdata;
                    tdata.object = this;
                    tdata.sc_data = dnld->sc_data;
                    tdata.parser = dnld->parser;
                    tdata.logger = logger;
                    th_args->push_back(tdata);
                    scandatas.erase(dnld->sc_data->object_url);
                }
                delete dnld;
                to_download[i] = NULL;
            }
        }
    }
    to_download.erase(std::remove_if(to_download.begin(), to_download.end(), remove_empty), to_download.end());
    LOG4CXX_DEBUG(logger, "audit_jscript::process_response: EXIT; to download: " << to_download.size() << "; scandatas: " << scandatas.size() );
    if (th_args->size() > 0) {
        boost::thread process(parser_thread, th_args);
    }
    else {
        delete th_args;
    }
//     for(mit = scandatas.begin(); mit != scandatas.end(); mit++) {
//         if (mit->second == 0) {
//             sc = mit->first;
//             thread_data* tdata = new thread_data;
//             tdata->object = this;
//             tdata->sc_data = parent_task->GetScanData(sc);
//             tdata->logger = logger;
//             boost::thread process(parser_thread, tdata);
//             scandatas.erase(mit);
//             mit = scandatas.begin();
//             if (mit == scandatas.end()) {
//                 break;
//             }
//         }
//     }
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
            parent_task->get_request_async(new_url);
        }
        else
        {
            // add parent to existing scan data
        }
        parent_task->register_url(u_req);
    }
}

void audit_jscript::parse_scripts(boost::shared_ptr<ScanData> sc, boost::shared_ptr<webEngine::HtmlDocument> parser)
{
    LOG4CXX_TRACE(logger, "audit_jscript::parse_scripts for " << sc->parsed_data);
    EntityList lst;

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
            iEntityPtr ent = lst[i];
            if (ent != NULL) {
                string source = ent->Attr("#code");
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
        //HtmlDocument* doc_entity = sc->parsed_data.get();
        process_events(parser);
        // process results
        string res;
        res = js_exec->get_results();
        res += js_exec->dump("Object");
        js_exec->reset_results();
#ifdef _DEBUG
        LOG4CXX_TRACE(logger, "audit_jscript::parse_scripts results:\n" << res);
#endif
        extract_links(res, sc);
    }
    parent_task->remove_thread();
    LOG4CXX_TRACE(logger, "audit_jscript::parse_scripts finished");
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
            LOG4CXX_DEBUG(logger, "audit_jscript::extract_links: found <a...> url=" << tres);
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

void audit_jscript::process_events( webEngine::iEntityPtr entity )
{
/* simple variant - reqursive descent throug DOM-tree
    std::string src;
    // get on... events
    LOG4CXX_DEBUG(logger, "audit_jscript::process_events entity = " << entity->Name());

    src = entity->Attr("onclick");
    if (src != "")
    {
#ifdef _DEBUG
        LOG4CXX_TRACE(logger, "audit_jscript::process_events onclick source = " << src);
#endif
        src = "(function(){" + src + "})()";
        // @todo: add V8 synchronization - only one script can be processed at same time
        js_exec->execute_string(src, "", true, true);
    }
    src = entity->Attr("onload");
    if (src != "")
    {
#ifdef _DEBUG
        LOG4CXX_TRACE(logger, "audit_jscript::process_events onload source = " << src);
#endif
        src = "(function(){" + src + "})()";
        // @todo: add V8 synchronization - only one script can be processed at same time
        js_exec->execute_string(src, "", true, true);
    }
    src = entity->Attr("onsubmit");
    if (src != "")
    {
#ifdef _DEBUG
        LOG4CXX_TRACE(logger, "audit_jscript::process_events onsubmit source = " << src);
#endif
        src = "(function(){" + src + "})()";
        // @todo: add V8 synchronization - only one script can be processed at same time
        js_exec->execute_string(src, "", true, true);
    }

    // and process all children
    EntityList chld = entity->Children();
    EntityList::iterator  it;

    for(size_t i = 0; i < chld.size(); i++) {
        std::string nm = chld[i]->Name();
        if (nm[0] != '#') {
            process_events(chld[i]);
        }
    }*/
    // new variant - parsing with stops on attributes
    scanner_token  state;
    string         txtAttr;
    string         lString;
    HtmlDocument*  doc = NULL;

    try
    {
        doc = dynamic_cast<HtmlDocument*>(entity.get());
    }
    catch (std::exception &e)
    {
        LOG4CXX_WARN(logger, "audit_jscript::process_events given entity can't be casted to HtmlDocument: " << e.what());
        return;
    }

    tag_stream* stream = doc->Data().stream();
    tag_scanner scanner(*stream);

    while (true)
    {
        state = scanner.get_token();
        if (state == wstEof || state == wstError) {
            break;
        }
        switch(state)
        {
        case wstAttr:
            lString = scanner.get_attr_name();
            boost::to_lower(lString);
            txtAttr = scanner.get_value();
            if (boost::starts_with(lString, "on")) {
                // attribute name started with "on*" - this is the event
#ifdef _DEBUG
                LOG4CXX_TRACE(logger, "audit_jscript::process_events - " << lString << " source = " << txtAttr);
#else
                LOG4CXX_DEBUG(logger, "audit_jscript::process_events - " << lString);
#endif
                txtAttr = "(function(){ " + txtAttr + " })()";
                js_exec->execute_string(txtAttr, "", true, true);
            }
            break;
        case wstWord:
        case wstSpace:
            //txtAttr += scanner.get_value();
            break;
        case wstTagEnd:
        case wstTagStart:
        case wstCommentStart:
        case wstCDataStart:
        case wstPiStart:
        case wstData:
        case wstCommentEnd:
        case wstCDataEnd:
        case wstPiEnd:
            txtAttr = "";
            break;
        default:
            break;
        }
    }
    delete stream;

    // all results will be processed in the caller 
    // parse_scripts function
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

void parser_thread( void *context )
{
    vector<thread_data> *th_args = (vector<thread_data>*)context;
    for (size_t i = 0; i < th_args->size(); i++) {
        LOG4CXX_TRACE((*th_args)[0].logger, "audit_jscript thread: process scripts " << i << " from " << th_args->size());
        (*th_args)[i].object->parse_scripts((*th_args)[i].sc_data, (*th_args)[i].parser);
    }
    th_args->clear();
    delete th_args;
}