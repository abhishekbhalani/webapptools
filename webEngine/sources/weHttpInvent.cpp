/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew Abramov aabramov@ptsecurity.ru

    This file is part of webEngine

    webEngine is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <webEngine.h>

#include <weHelper.h>
#include <weDispatch.h>
#include <weTask.h>
#include <weHttpInvent.h>
#include <weScan.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

static string xrc = "<plugin id='httpInventory'>\
<category name='crawl' label='Crawling mode'>\
<option name='httpInventory/stay_in_dir' label='Stay in directory' type='2' control='checkbox'>1</option>\
<option name='httpInventory/stay_in_host' label='Stay in host' type='2'  control='checkbox'>0</option>\
<option name='httpInventory/stay_in_ip' label='Stay in domain and IP' type='2'  control='checkbox'>0</option>\
<option name='httpInventory/stay_in_domain' label='Stay in domain' type='2'  control='checkbox'>0</option>\
<option name='httpInventory/stay_in_domain_list' label='Stay in the list of domains' type='2'  control='checkbox'>0</option>\
<option name='httpInventory/domains_allow' label='List of allowed domains' type='4'  control='textarea'>*</option>\
</category>\
<option name='httpInventory/scan_depth' label='Scan depth' type='1' control='text'>-1</option>\
<option name='httpInventory/url_param' label='Ignore URL parameters' type='1' composed='true'>\
<option name='0' label='None' control='radio'>1</option>\
<option name='1' label='Ignore all' control='radio'>0</option>\
<option name='2' label='Ignore values' control='radio'>0</option>\
</option>\
<option name='httpInventory/ignore_urls' label='Ignore URLs' type='4' control='textarea'></option>\
<option name='httpInventory/denied_file_types' label='Denied file types' type='4' control='text'></option>\
<option name='httpInventory/AllowedCType' label='Allowed content-type' type='2' control='select'>\
<select_option value='0'>Any</select_option>\
<select_option value='1'>empty and \"text/*\"</select_option>\
<select_option value='2'>only \"text/*\"</select_option>\
<select_option value='3' selected='true'>empty and \"text/html\"</select_option>\
<select_option value='4'>only \"text/html\"</select_option>\
</option>\
<category label='Authentication' name='auth'>\
<option name='' label='Authentication methods' composed='true'>\
<option name='httpInventory/Auth/Base' label='Basic HTTP' type='2' control='checkbox'>0</option>\
<option name='httpInventory/Auth/Digest' label='HTTP Digest' type='2' control='checkbox'>0</option>\
<option name='httpInventory/Auth/NTLM' label='NTLM' type='2' control='checkbox'>0</option>\
<option name='httpInventory/Auth/Forms' label='Form-based' type='2' control='checkbox'>0</option>\
</option>\
<option name='httpInventory/Auth/domain' label='Domain' type='4' control='text'></option>\
<option name='httpInventory/Auth/username' label='Username' type='4' control='text'></option>\
<option name='httpInventory/Auth/password' label='Password' type='4' control='password'></option>\
<option name='httpInventory/Auth/form-list' label='Forms identities' type='4' control='textarea'></option>\
</category></plugin>";

using namespace boost;

namespace webEngine {

http_inventory::http_inventory(engine_dispatcher* krnl, void* handle /*= NULL*/) :
    i_inventory(krnl, handle)
{
    pluginInfo.interface_name = "http_inventory";
    pluginInfo.interface_list.push_back("http_inventory");
    pluginInfo.plugin_desc = "Inventory through HTTP protocol";
    pluginInfo.plugin_id = "AB7ED6E5A7B3"; //{7318EAB5-4253-4a31-8284-AB7ED6E5A7B3}
}

http_inventory::~http_inventory(void)
{
}

i_plugin* http_inventory::get_interface( const string& ifName )
{
    LOG4CXX_TRACE(logger, _T("http_inventory::get_interface ") << ifName);
    if (iequals(ifName, "httpInventory")) {
        LOG4CXX_DEBUG(logger, _T("http_inventory::get_interface found!"));
        usageCount++;
        return (this);
    }
    return i_inventory::get_interface(ifName);
}

const string http_inventory::get_setup_ui( void )
{
    return xrc;
}

void http_inventory::init( task* tsk )
{
    transport_url   start_url;
    string url_list;

    if (tsk) {
        parent_task = tsk;
        if (kernel) {
            string host;
            we_option opt = tsk->Option(weoScanHost);
            SAFE_GET_OPTION_VAL(opt, host, "");
            if (host != "") {
                string path;
                // create list of the blocked extension
                opt = tsk->Option(weoDeniedFileTypes);
                SAFE_GET_OPTION_VAL(opt, path, 1);
                if (path != "") {
                    size_t pos = path.find(';');
                    ext_deny.clear();
                    while(pos != string::npos) {
                        string ext = path.substr(0, pos);
                        ext_deny.push_back(ext);
                        if (pos < path.length()) {
                            path = path.substr(pos+1);
                        } else {
                            path = "";
                        }
                        pos = path.find(';');
                    }
                }

                // create list of allowed sub-domains
                opt = tsk->Option(weoDomainsAllow);
                SAFE_GET_OPTION_VAL(opt, path, 1);
                if (path != "") {
                    boost::split(domain_allow, path, boost::is_any_of("\r\n"),  token_compress_on);
                }

                // processing options
                opt_in_dlist = parent_task->IsSet(weoStayInDomainList);
                opt_in_dir = parent_task->IsSet(weoStayInDir);
                opt_in_ip = parent_task->IsSet(weoStayInIP);
                opt_in_host = parent_task->IsSet(weoStayInHost);
                opt_in_domain = parent_task->IsSet(weoStayInDomain);

                opt_auth_methods = CURLAUTH_NONE;
                bool opt_auth;
                opt_auth = parent_task->IsSet(weoHttpAuthBasic);
                if (opt_auth) {
                    opt_auth_methods |= CURLAUTH_BASIC;
                }
                opt_auth = parent_task->IsSet(weoHttpAuthDigest);
                if (opt_auth) {
                    opt_auth_methods |= CURLAUTH_DIGEST;
                }
                opt_auth = parent_task->IsSet(weoHttpAuthNTLM);
                if (opt_auth) {
                    opt_auth_methods |= CURLAUTH_NTLM;
                }
                opt_auth = parent_task->IsSet(weoHttpAuthForms);
                if (opt_auth) {
                    opt_auth_methods |= CURLAUTH_FORMS; // not a really curl
                }

                opt = parent_task->Option(weoHttpAuthUname);
                SAFE_GET_OPTION_VAL(opt, opt_auth_username, "");
                opt = parent_task->Option(weoHttpAuthPassword);
                SAFE_GET_OPTION_VAL(opt, opt_auth_password, "");
                opt = parent_task->Option(weoHttpAuthDomain);
                SAFE_GET_OPTION_VAL(opt, url_list, "");
                if (url_list != "" && (opt_auth_methods & CURLAUTH_NTLM)) {
                    opt_auth_username = url_list + "\\" + opt_auth_username;
                }
                opt = parent_task->Option(weoHttpAuthFormData);
                SAFE_GET_OPTION_VAL(opt, url_list, "");
                boost::split(opt_auth_form_params, url_list, boost::is_any_of("\r\n"));

                opt = parent_task->Option(weoIgnoreUrlParam);
                SAFE_GET_OPTION_VAL(opt, opt_ignore_param, 0);
                opt = parent_task->Option(weoScanDepth);
                SAFE_GET_OPTION_VAL(opt, opt_max_depth, 0);
                opt = parent_task->Option(weoAllowedCTypes);
                SAFE_GET_OPTION_VAL(opt, opt_ctype_method, 0); // default - any type

                opt = tsk->Option(weoBaseURL);
                SAFE_GET_OPTION_VAL(opt, path, "");
                start_url.protocol = "http";
                start_url.port = 80;
                start_url.host = host;
                start_url.assign_with_referer(path);
                LOG4CXX_INFO(logger, _T("http_inventory::init: init scanning from: ") << host << _T(" ==> ") << start_url.tostring());
                // get ignore urls list
                opt = parent_task->Option(weoIgnoreUrlList);
                SAFE_GET_OPTION_VAL(opt, url_list, "");
                if (url_list != "") {
                    vector<string> urls;
                    boost::split(urls, url_list, boost::is_any_of(" \n"));
                    for (size_t i = 0; i < urls.size(); ++i) {
                        transport_url ignore;
                        ignore.assign_with_referer(urls[i], &start_url);
                        parent_task->register_url(ignore.tostring());
                    }
                }


                HttpRequest* req = new HttpRequest;
                req->RequestUrl(start_url);
                if ( (opt_auth_methods & ~CURLAUTH_FORMS) != CURLAUTH_NONE) {
                    // sets auth data
                    req->SetAuth((opt_auth_methods & ~CURLAUTH_FORMS), opt_auth_username, opt_auth_password);
                }

                LOG4CXX_TRACE(logger, _T("http_inventory::init: init request = ") << req->RequestUrl().tostring());
//                req->processor = http_inventory::response_dispatcher;
//                req->context = (void*)this;
                parent_task->register_url(start_url.tostring());
                parent_task->get_request_async(i_request_ptr(req));
            } else {
                LOG4CXX_WARN(logger, _T("http_inventory::init: Can't find hostname. Finishing."));
            }
        } else {
            LOG4CXX_WARN(logger, _T("http_inventory::init: No kernel given - can't read options. Finishing."));
        }
    } else {
        LOG4CXX_WARN(logger, _T("http_inventory::init: No parent task - can't process requests. Finishing."));
    }
}

void http_inventory::process( task* tsk, scan_data_ptr scData )
{
    HttpResponse* htResp;

    try {
        htResp = reinterpret_cast<HttpResponse*>(scData->response.get());
    } catch (...) {
        LOG4CXX_ERROR(logger, _T("http_inventory::process: The response from ") << scData->response->BaseUrl().tostring() << _T(" isn't the HttpResponse!"));
        return;
    }
    if (htResp == NULL) {
        LOG4CXX_TRACE(logger, _T("http_inventory::process: subsequent processing, or transport fail - no responce given."));
        return;
    }
    // process response
    LOG4CXX_TRACE(logger, _T("http_inventory::process: process response with code=") << htResp->HttpCode());
    if (scData->parent_id == "") {
        scData->parent_id = htResp->ID();
    }
    scData->resp_code = htResp->HttpCode();
    scData->download_time = htResp->DownloadTime();
    scData->data_size = htResp->Data().size();
    scData->scan_depth = htResp->depth();
    scData->content_type = htResp->ContentType();

    /// @todo process options
    if (htResp->HttpCode() >= 300 && htResp->HttpCode() < 400) {
        // redirections
        LOG4CXX_TRACE(logger, _T("http_inventory::process: process redirect"));
        string url = htResp->Headers().find_first("Location");
        if (!url.empty()) {
            LOG4CXX_DEBUG(iLogger::GetLogger(), _T("http_inventory::process: redirected to ") << url);
            bool to_process = true;
            transport_url baseUrl = htResp->BaseUrl();
            baseUrl.assign_with_referer(url);
            LOG4CXX_DEBUG(logger, _T("http_inventory::process: reconstructed url is ") << baseUrl.tostring());
            add_url(baseUrl, htResp, scData);
        }
    }
    if ((htResp->HttpCode() > 0 && htResp->HttpCode() < 300) || htResp->Data().size() > 0) {
        boost::shared_ptr<html_document> parser;
        entity_list lst;

        try {
            parser = boost::shared_dynamic_cast<html_document>(scData->parsed_data);
        } catch (bad_cast) {
            LOG4CXX_ERROR(logger, _T("http_inventory::process: can't process given document as html_document"));
            parser.reset();
        }
        if (parser) {
            LOG4CXX_DEBUG(logger, _T("http_inventory::process: search for links"));
            lst = parser->FindTags("a");
            if (lst.size() > 0) {
                base_entity* ent = NULL;
                entity_list::iterator iEnt;
                string href;
                for (iEnt = lst.begin(); iEnt != lst.end(); iEnt++) {
                    href = (*iEnt)->attr("href");
                    if (href != "") {
                        transport_url link;
                        link.assign_with_referer(href, &(htResp->RealUrl()));
                        LOG4CXX_TRACE(logger, _T("http_inventory::process: <a href...> tag. url=") << link.tostring());
                        add_url(link, htResp, scData);
                    } // end href attribute
                } // end <a ...> loop
                ClearEntityList(lst);
            } // end <a ...> tags processing
            // todo: process other kind of links
            // CSS
            // scripts
            // objects
            // images
            lst = parser->FindTags("img");
            if (lst.size() > 0) {
                webEngine::base_entity* ent = NULL;
                webEngine::entity_list::iterator iEnt;
                string href;
                for (iEnt = lst.begin(); iEnt != lst.end(); iEnt++) {
                    href = (*iEnt)->attr("src");
                    if (href != "") {
                        transport_url link;
                        link.assign_with_referer(href, &(htResp->RealUrl()));
                        LOG4CXX_DEBUG(logger, _T("http_inventory::process: IMG url = ") << link.tostring());
                        add_url(link, htResp, scData);
                    } // end src attribute
                } // end <img ...> loop
                ClearEntityList(lst);
            } // end <img ...> tags processing
            // frames
            lst = parser->FindTags("frame");
            if (lst.size() > 0) {
                webEngine::base_entity* ent = NULL;
                webEngine::entity_list::iterator iEnt;
                string href;
                for (iEnt = lst.begin(); iEnt != lst.end(); iEnt++) {
                    href = (*iEnt)->attr("src");
                    if (href != "") {
                        transport_url link;
                        link.assign_with_referer(href, &(htResp->RealUrl()));
                        LOG4CXX_DEBUG(logger, _T("http_inventory::process: FRAME url = ") << link.tostring());
                        add_url(link, htResp, scData);
                    } // end src attribute
                } // end <iframe ...> loop
                ClearEntityList(lst);
            } // end <iframe ...> tags processing
            // iframes
            lst = parser->FindTags("iframe");
            if (lst.size() > 0) {
                webEngine::base_entity* ent = NULL;
                webEngine::entity_list::iterator iEnt;
                string href;
                for (iEnt = lst.begin(); iEnt != lst.end(); iEnt++) {
                    href = (*iEnt)->attr("src");
                    if (href != "") {
                        transport_url link;
                        link.assign_with_referer(href, &(htResp->RealUrl()));
                        LOG4CXX_DEBUG(logger, _T("http_inventory::process: IFRAME url = ") << link.tostring());
                        add_url(link, htResp, scData);
                    } // end src attribute
                } // end <iframe ...> loop
                ClearEntityList(lst);
            } // end <iframe ...> tags processing
            // forms
            // meta
            lst = parser->FindTags("meta");
            if (lst.size() > 0) {
                base_entity* ent = NULL;
                entity_list::iterator iEnt;
                string href;
                for (iEnt = lst.begin(); iEnt != lst.end(); iEnt++) {
                    href = (*iEnt)->attr("http-equiv");
                    if (boost::iequals(href, "refresh")) {
                        href = (*iEnt)->attr("content");
                        if (href != "") {
                            transport_url link;
                            string lc = href;
                            to_lower(lc);
                            size_t np = lc.find("url=");
                            if (np != std::string::npos) {
                                href = href.substr(np+4);
                            }
                            link.assign_with_referer(href, &(htResp->RealUrl()));
                            LOG4CXX_DEBUG(logger, _T("http_inventory::process: META url = ") << link.tostring());
                            add_url(link, htResp, scData);
                        }
                    } // end http-equiv attribute
                } // end <meta ...> loop
                ClearEntityList(lst);
            } // end <meta ...> tags processing
            // etc ???
        } // if document parsed
    } // if HTTP code valid

    // update scan_data
    parent_task->set_scan_data(scData->object_url, scData);
}

void http_inventory::add_url( transport_url link, HttpResponse *htResp, boost::shared_ptr< ScanData > scData )
{
    bool allowed = true;
    if (opt_in_host) {
        if (!link.is_host_equal(htResp->RealUrl())) {
            allowed = false;
        }
        LOG4CXX_TRACE(logger, _T("http_inventory::add_url: weoStayInHost check ") << allowed << _T(" (") << link.tostring() << _T(")"));
    }
    if (opt_in_domain) {
        if (!link.is_domain_equal(htResp->RealUrl())) {
            allowed = false;
        }
        LOG4CXX_TRACE(logger, _T("http_inventory::add_url: weoStayInDomain check << ") << allowed << _T(" (") << link.tostring() << _T(")"));
    }
    if (opt_in_dlist) {
        allowed = false;
        for (size_t i = 0; i < domain_allow.size(); ++i) {
            string host = link.host;
            string doms = domain_allow[i];
            if (istarts_with(host, "www.")) {
                host = host.substr(4);
            }
            if (istarts_with(doms, "www.")) {
                doms = doms.substr(4);
            }
            if (iequals(host, doms)) {
                allowed = true;
                LOG4CXX_DEBUG(logger, _T("http_inventory::add_url: found allowed domain ") << domain_allow[i]);
                break;
            }
        } // foreach domain
        LOG4CXX_TRACE(logger, _T("http_inventory::add_url: weoStayInDomainList check << ") << allowed << _T(" (") << link.tostring() << _T(")"));
    } // if filter by domain list
    if ( opt_max_depth > 0 && htResp->depth() >= opt_max_depth) {
        LOG4CXX_DEBUG(logger, _T("http_inventory::add_url: maximum scanning depth reached! (") << opt_max_depth << _T(")"));
        allowed = false;
    }
    if (allowed) {
        // verify blocked file types
        string path = link.request;
        int pos = path.find_last_of('.');
        if (pos != string::npos && ext_deny.size() > 0) {
            path = path.substr(pos+1);
            LOG4CXX_TRACE(logger, _T("http_inventory::add_url: Found extension: ") << path << _T("; Deny list size is ") << ext_deny.size());
            for (size_t i = 0; i < ext_deny.size(); i++) {
                if (path == ext_deny[i]) {
                    allowed = false;
                    LOG4CXX_DEBUG(logger, _T("http_inventory::add_http_url: not need to download ") << link.tostring());
                    // make the pseudo-response
                    boost::shared_ptr<ScanData> scn = parent_task->get_scan_data(link.tostring());
                    if (scn->data_id == "") {
                        scn->data_id = kernel->storage()->generate_id(weObjTypeScan);
                        scn->parent_id = scData->data_id;
                        scn->resp_code = 204; // 204 No Content;  The server successfully processed the request, but is not returning any content
                        scn->download_time = 0;
                        scn->data_size = 0;
                        scn->scan_depth = htResp->depth() + 1;
                        scn->content_type = "application/octet-stream";
                        scn->parsed_data.reset();
                        parent_task->set_scan_data(scn->object_url, scn);
                    }
                    break;
                } // if path == ext_deny[i]
            } // for deny list size
        } // if file extension found
    } // if allowed
    if (allowed) {
        string u_req = link.tostring();
        if (opt_ignore_param) {
            u_req = link.tostring_noparam();
        }
        LOG4CXX_TRACE(logger, _T("http_inventory::add_http_url: weoIgnoreUrlParam check << ") << u_req);
        if (!parent_task->is_url_processed(u_req)) {
            HttpRequest* new_url = new HttpRequest(u_req);
            new_url->depth(htResp->depth() + 1);
            new_url->ID(scData->data_id);
//            new_url->processor = http_inventory::response_dispatcher;
//            new_url->context = (void*)this;
            if ( (opt_auth_methods & ~CURLAUTH_FORMS) != CURLAUTH_NONE) {
                // sets auth data
                new_url->SetAuth((opt_auth_methods & ~CURLAUTH_FORMS), opt_auth_username, opt_auth_password);
            }
            new_url->SetReferer(scData->object_url);
            parent_task->get_request_async(i_request_ptr(new_url));
        } else {
            // add parent to existing scan data
        }
        parent_task->register_url(u_req);
    }
}
} // namespace webEngine
