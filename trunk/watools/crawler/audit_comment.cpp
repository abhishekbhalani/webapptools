#include "audit_comment.h"
#include <weHelper.h>
#include <weTask.h>
// for add_http_url
#include <weHttpInvent.h>
#include <boost/regex.hpp>

using namespace webEngine;
using namespace boost;

audit_comment::audit_comment(engine_dispatcher* krnl, void* handle /*= NULL*/) :
    i_audit(krnl, handle)
{
    pluginInfo.interface_name = "audit_comment";
    pluginInfo.interface_list.push_back("audit_comment");
    pluginInfo.plugin_desc = "Search HTML data for links in comments and plain text";
    pluginInfo.plugin_id = "D00E80357E27"; //{397D47CC-7F22-46ec-9F46-D00E80357E27}
}

audit_comment::~audit_comment(void)
{
}

i_plugin* audit_comment::get_interface( const string& ifName )
{
    LOG4CXX_TRACE(logger, "audit_comment::get_interface " << ifName);
    if (iequals(ifName, "audit_comment"))
    {
        LOG4CXX_DEBUG(logger, "audit_comment::get_interface found!");
        usageCount++;
        return (this);
    }
    return i_audit::get_interface(ifName);
}

void audit_comment::start( task* tsk, shared_ptr<ScanData> scData )
{
    LOG4CXX_TRACE(logger, "audit_comment::start");
    entity_list lst;
    transport_url baseUrl;
    string text;

    parent_task = tsk;
    baseUrl.assign(scData->object_url);
    string cType = scData->content_type;
    bool ctype_process = false;
    switch(ctype_method)
    {
    case 0: // any content-type
        ctype_process = true;
        break;
    case 1: // empty and "text/*"
        if (cType == "" || starts_with(cType, "text/"))
        {
            ctype_process = true;
        }
        break;
    case 2: // only "text/*"
        if (starts_with(cType, "text/"))
        {
            ctype_process = true;
        }
        break;
    case 3: // empty and "text/html"
        if (cType == "" || starts_with(cType, "text/html"))
        {
            ctype_process = true;
        }
        break;
    case 4: // only "text/*"
        if (starts_with(cType, "text/html"))
        {
            ctype_process = true;
        }
        break;
    default:
        ctype_process = false;
        LOG4CXX_WARN(logger, "audit_comment::init: unknown content-type analyze method = " << ctype_method);
        break;
    }
    if (ctype_process) {
        // 1. Get all comments
        if (scData->parsed_data) {
            //scData->parsed_data->add_ref();
            lst = scData->parsed_data->FindTags("#comment");
            if (lst.size() > 0)
            {
                webEngine::base_entity* ent = NULL;
                webEngine::entity_list::iterator iEnt;
                for (iEnt = lst.begin(); iEnt != lst.end(); iEnt++) {
                    text = (*iEnt)->InnerText();
                    if (text != "") {
                        extract_links(text, &baseUrl, scData);
                    } // end InnerText processing
                } // end comment loop
            } // end comment tags processing
            ClearEntityList(lst);

            // 2. Get page as plain text
            text = scData->parsed_data->InnerText();
            extract_links(text, &baseUrl, scData);

            // may be need to search in plain data?
            /*if (scData->parsed_data != NULL) {
            text.assign(scData->parsed_data->Data().begin(), scData->parsed_data->Data().end());
            extract_links(text, &baseUrl, scData);
            }*/
        } // if data parsed

    } // if need to process
    else {
        LOG4CXX_DEBUG(logger, "audit_comment: skip processing content-type = " << cType);
    }
    return;
}

void audit_comment::process_response( i_response_ptr resp )
{
    LOG4CXX_TRACE(logger, "audit_comment::process_response");
    return;
}

// clone of the HttpInventory::add_url
void audit_comment::add_url( transport_url link, transport_url *base_url, shared_ptr<ScanData> sc )
{
    bool allowed = true;

    if (!link.is_host_equal(sc->object_url))
    {
        if (opt_in_host)
        {
            allowed = false;
        }
        LOG4CXX_TRACE(logger, "audit_comment::add_url: weoStayInHost check " << allowed << " (" << link.tostring() << ")");
    }
    if (!link.is_domain_equal(sc->object_url))
    {
        if (opt_in_domain)
        {
            allowed = false;
        }
        LOG4CXX_TRACE(logger, "audit_comment::add_url: weoStayInDomain check << " << allowed << " (" << link.tostring() << ")");
    }
    if ( opt_max_depth > 0 && sc->scan_depth >= opt_max_depth) {
        LOG4CXX_DEBUG(logger, "audit_comment::add_url: maximum scanning depth reached! (" << opt_max_depth << ")");
        allowed = false;
    }
    if (allowed)
    {    // verify blocked file types
        string path = link.request;
        int pos = path.find_last_of('.');
        if (pos != string::npos && ext_deny.size() > 0)
        {
            path = path.substr(pos+1);
            LOG4CXX_TRACE(logger, "audit_comment::add_url: Found extension: " << path << "; Deny list size is " << ext_deny.size());
            for (size_t i = 0; i < ext_deny.size(); i++) {
                if (path == ext_deny[i]) {
                    allowed = false;
                    LOG4CXX_DEBUG(logger, "audit_comment::add_http_url: not need to download " << link.tostring());
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
        LOG4CXX_TRACE(logger, "audit_comment::add_http_url: weoIgnoreUrlParam check << " << u_req);
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

void audit_comment::extract_links( string text, webEngine::transport_url *base_url, shared_ptr<ScanData> scData )
{
    boost::smatch mres;
    std::string::const_iterator strt, end; 
    boost::match_flag_type flags = boost::match_default; 
    webEngine::transport_url lurl;

    strt = text.begin(); 
    end = text.end();
    // 1. search strings that looks like URL
    boost::regex re1("(\\s|^)(\\w+://[^\\s\"\'\\)\\(]+)(\\s|$)"); //, boost::regex_constants::icase
    
    while(regex_search(strt, end, mres, re1, flags)) {
        string tres = mres[2];
        LOG4CXX_DEBUG(logger, "audit_comment::extract_links: found url=" << tres);
        lurl.assign_with_referer(tres, base_url);
        add_url(lurl, base_url, scData);

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
    // \<a.*?href\s*=\s*("|')?([^\s"'\)\(]+).*?\>
    boost::regex re2("\\<a.*?href\\s*=\\s*(\"|')?([^\\s\"\'\\)\\(]+).*?\\>");// , boost::regex_constants::icase

    while(regex_search(strt, end, mres, re2, flags)) {
        string tres = mres[2];
        LOG4CXX_DEBUG(logger, "audit_comment::extract_links: found <a...> url=" << tres);
        lurl.assign_with_referer(tres, base_url);
        add_url(lurl, base_url, scData);

        // update search position: 
        strt = mres[0].second; 
        // update flags: 
        /*flags |= boost::match_prev_avail; 
        flags |= boost::match_not_bob;*/ 
    }
}

void audit_comment::init( task* tsk )
{
    string text;
    wOption opt;

    parent_task = tsk;
    // 0. check preconditions
    // create list of the blocked extension
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

    // check data type
    opt = parent_task->Option("httpInventory/AllowedCType");
    SAFE_GET_OPTION_VAL(opt, ctype_method, 0); // default - any type
    LOG4CXX_TRACE(logger, "audit_comment::init: content-type analyze method = " << ctype_method);
    // processing options
    opt_in_host = parent_task->IsSet("httpInventory/"weoStayInHost);
    opt_in_domain = parent_task->IsSet("httpInventory/"weoStayInDomain);
    opt_ignore_param = parent_task->IsSet("httpInventory/"weoIgnoreUrlParam);
    opt = parent_task->Option("httpInventory/"weoScanDepth);
    SAFE_GET_OPTION_VAL(opt, opt_max_depth, 0);
}