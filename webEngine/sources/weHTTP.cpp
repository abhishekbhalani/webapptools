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

#include <boost/lexical_cast.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <algorithm>
#include <weHelper.h>
#include <weHTTP.h>
#include <weDbstruct.h>
#include <weDispatch.h>
#include <weTask.h>

static string xrc = "<plugin id='httpTransport'>\
<option name='httpTransport/port' label='HTTP Port number' type='1' control='text'>80</option>\
<option name='httpTransport/sport' label='HTTPS Port number' type='1' control='text'>443</option>\
<option name='httpTransport/protocol' label='Protocol' type='4' control='text'>http</option>\
<option name='httpTransport/timeout' label='Timeout (sec.)' type='1' control='text'>10</option>\
<option name='httpTransport/size_Limit' label='Document size limit' type='1' control='text'>-1</option>\
<option name='httpTransport/user_agent' label='Use UserAgent value' type='4' control='text'>WebEngine "WE_VERSION_PRODUCTSTR"</option>\
<option name='httpTransport/cookies' type='1' label='Accept cookies' composed='true'>\
<option name='0' label='No' control='radio'>1</option>\
<option name='1' label='Yes' control='radio'>0</option>\
</option>\
<option name='' label='Local port settings' composed='true'>\
<option name='httpTransport/use_localport' label='Use local port binding' type='2' control='checkbox'>0</option>\
<option name='httpTransport/local_port' label='Local port' type='1' control='text'>0</option>\
<option name='httpTransport/local_range' label='Local port pool' type='1' control='text'>0</option>\
</option>\
<category label='Proxy' name='proxy'>\
<option name='httpTransport/Proxy' type='1' label='Proxy type' composed='true'>\
<option name='0' label='None' control='radio'>1</option>\
<option name='1' label='HTTP' control='radio'>0</option>\
<option name='2' label='Socks 4' control='radio'>0</option>\
<option name='3' label='Socks 4A' control='radio'>0</option>\
<option name='4' label='Socks 5' control='radio'>0</option>\
</option>\
<option name='httpTransport/ProxyAuth' type='1' label='Proxy Authentication' composed='true'>\
<option name='0' label='None' control='radio'>1</option>\
<option name='1' label='Basic' control='radio'>0</option>\
<option name='2' label='Diges' control='radio'>0</option>\
<option name='3' label='NTLM' control='radio'>0</option>\
</option>\
<option name='httpTransport/Proxy/host' label='Proxy host' type='4' control='text'></option>\
<option name='httpTransport/Proxy/port' label='Proxy port' type='1' control='text'>8080</option>\
<option name='httpTransport/Proxy/domain' label='Domain' type='4' control='text'></option>\
<option name='httpTransport/Proxy/username' label='Username' type='4' control='text'></option>\
<option name='httpTransport/Proxy/password' label='Password' type='4' control='password'></option>\
</category></plugin>";


namespace webEngine {

//////////////////////////////////////////////////////////////////////////
// Transport creation functions
//////////////////////////////////////////////////////////////////////////
static i_transport* weCreateHttp(engine_dispatcher* krnl)
{
    LOG4CXX_DEBUG(iLogger::GetLogger(), "TransportFactory: create http_transport");
    return new http_transport(krnl); //new http_transport();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn HttpRequest::HttpRequest()
///
/// @brief  Default constructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
HttpRequest::HttpRequest()
{
    auth_method = CURLAUTH_NONE;
    username = "";
    password = "";
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn HttpRequest::HttpRequest( string url,
///     weHttpMethod meth , HttpResponse* resp )
///
/// @brief  Constructs the object from the previous HttpResponse.
/// @param	url	 - The url.
/// @param	meth - The request method. Default - wemGet.
/// @param	resp - If non-null, the HttpResponse, to create chained HttpRequest to.
/// @throw  WeError if given transport_url is malformed and can't be reconstructed from the HttpResponse
////////////////////////////////////////////////////////////////////////////////////////////////////
HttpRequest::HttpRequest( string url, HttpRequest::weHttpMethod meth /*= wemGet*/, HttpResponse* resp /*= NULL*/ )
{
    method = meth;
    auth_method = CURLAUTH_NONE;
    username = "";
    password = "";
    RequestUrl(url, resp);
    /// @todo Implement this!
    //LOG4CXX_WARN(iLogger::GetLogger(), "HttpRequest::HttpRequest(string, weHttpMethod, HttpResponse*) - Not implemented");
}

i_request_ptr HttpRequest::restore_request(we_iarchive& ar)
{
    HttpRequest* result = new HttpRequest();
    ar >> boost::serialization::make_nvp("HttpRequest", *result);
    return i_request_ptr(result);
    //return i_request_ptr();
}

void HttpRequest::RequestUrl( const string &ReqUrl, i_operation* resp /*= NULL*/ )
{
    transport_url   weurl;

    weurl = ReqUrl;
    return RequestUrl(weurl, resp);
}

void HttpRequest::RequestUrl( const transport_url &ReqUrl, i_operation* resp /*= NULL*/ )
{
    reqUrl = ReqUrl;
    if (resp != NULL) {
        /// @todo Make referers and other options for linked request
        depth_level = resp->depth() + 1;
    }
}

void HttpRequest::ComposePost( int method /*= composeOverwrite*/ )
{
    if (method == wemPost || method == wemPut) {
        /// @todo Compose POST data
    }
}

webEngine::i_response_ptr HttpRequest::abort_request()
{
    HttpResponse *resp = new HttpResponse;

    resp->HttpCode(406); // Not Acceptable
    resp->RealUrl(reqUrl);
    resp->BaseUrl(reqUrl);
    resp->Data().clear();
    resp->Processed(true);

    return i_response_ptr(resp);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	http_transport::http_transport(engine_dispatcher* krnl, void* handle = NULL)
///
/// @brief  Default constructor.
/// @throw  WeError if the curl initialization failed
////////////////////////////////////////////////////////////////////////////////////////////////////
http_transport::http_transport(engine_dispatcher* krnl, void* handle /*= NULL*/) :
    i_transport(krnl, handle)
{
    transferHandle = curl_multi_init();
    if (transferHandle == NULL) {
        throw WeError("curl_multi_init failed!");
    }
    responces.clear();
    // iwePlugin structure
    pluginInfo.interface_name = "http_transport";
    pluginInfo.interface_list.push_back("http_transport");
    pluginInfo.plugin_desc = "HTTP transport interface";
    pluginInfo.plugin_id = "A44A9A1E7C25";

    default_port = 80;
    default_sport = 443;
    proto_name = "http";
    default_timeout = 10;
    options.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	http_transport::~http_transport()
///
/// @brief  Destructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
http_transport::~http_transport()
{
    response_list::iterator hnd;
    HttpResponse* point;

    if (transferHandle != NULL) {
        for (hnd = responces.begin(); hnd != responces.end(); hnd++) {
            try {
                point = dynamic_cast<HttpResponse*>((*hnd).get());
                curl_multi_remove_handle(transferHandle, point->CURLHandle());
            } catch (...) {}; // just skip
        }
        curl_multi_cleanup(transferHandle);
    }
}

i_plugin* http_transport::get_interface( const string& ifName )
{
    if (iequals(ifName, "http_transport")) {
        usageCount++;
        return this;
    }
    if (iequals(ifName, "i_transport")) {
        usageCount++;
        return this;
    }
    return i_transport::get_interface(ifName);
}

bool http_transport::is_own_protocol( string& proto )
{
    // return TRUE for http & https
    return istarts_with(proto_name, proto);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn i_response_ptr http_transport::request( i_request* req, i_response_ptr resp )
///
/// @brief  Requests.
///
/// @param  req  - If non-null, the HTTP request.
/// @param  resp - If non-null, the HTTP response to store results, else - the new HttpResponse
///                will be creaded.
/// @retval	null if it fails, pointer to the HttpResponse else.
////////////////////////////////////////////////////////////////////////////////////////////////////
i_response_ptr http_transport::request( i_request* req, i_response_ptr resp /*= i_response_ptr(0)*/ )
{
    i_response_ptr retval;
    HttpResponse* ht_retval;
    we_option opt;

    if (!req->RequestUrl().is_valid()) {
        if (resp != NULL) {
            string url = req->RequestUrl().tostring();
            req->RequestUrl().assign_with_referer(url, &(resp->RealUrl()));
        }
    }

    if (!req->RequestUrl().is_valid()) {
        if (req->RequestUrl().protocol == "") {
            req->RequestUrl().protocol = proto_name;
        }
        if (req->RequestUrl().port < 1 || req->RequestUrl().port > 65535) {
            if (iequals(proto_name, req->RequestUrl().protocol)) {
                req->RequestUrl().port = default_port;
            } else {
                req->RequestUrl().port = default_sport;
            }
        }
    }
    LOG4CXX_DEBUG(iLogger::GetLogger(), "http_transport::request: url=" << req->RequestUrl().tostring());

    if (!req->RequestUrl().is_valid()) {
        // bad luck! still not valid request... :(
        /// @todo Add error processing. Throw exception or return NULL
    }

    if (!resp) {
        ht_retval = new HttpResponse();
        retval = i_response_ptr(ht_retval);
        retval->BaseUrl(req->RequestUrl());
        retval->RelocCount(0);
    } else {
        retval = resp;
        ht_retval = dynamic_cast<HttpResponse*>(retval.get());
        if (!ht_retval->BaseUrl().is_valid()) {
            ht_retval->BaseUrl(req->RequestUrl());
        }
        if (!ht_retval->CurlInit()) {
            /// @todo may be throw exception&
            return i_response_ptr((i_response*)NULL);
        }
    }
    ht_retval->depth(req->depth());

    try {
        HttpRequest* r = dynamic_cast<HttpRequest*>(req);
        ht_retval->CurlSetOpts(r);
    } catch (...) {}; // just skip
    ht_retval->Processed(false);

    responces.push_back(i_response_ptr(retval));

    if (transferHandle != NULL && ht_retval->CURLHandle() != NULL) {
        try {
            // set the timeout
            ht_retval->start_time(boost::posix_time::second_clock::local_time());
            curl_easy_setopt(ht_retval->CURLHandle(), CURLOPT_TIMEOUT, default_timeout);
            if (use_proxy > 0) {
                // set appropriate proxy
                int p_type = CURLPROXY_HTTP;
                switch (use_proxy) {
                case weoHttpProxyTypeHTTP:
                    p_type = CURLPROXY_HTTP;
                    break;
                case weoHttpProxyTypeSocks4:
                    p_type = CURLPROXY_SOCKS4;
                    break;
                case weoHttpProxyTypeSocks4a:
                    p_type = CURLPROXY_SOCKS4A;
                    break;
                case weoHttpProxyTypeSocks5:
                    p_type = CURLPROXY_SOCKS5;
                    break;
                }
                curl_easy_setopt(ht_retval->CURLHandle(), CURLOPT_PROXYTYPE, p_type);
                curl_easy_setopt(ht_retval->CURLHandle(), CURLOPT_PROXY, proxy_host.c_str());
                curl_easy_setopt(ht_retval->CURLHandle(), CURLOPT_PROXYPORT, proxy_port);
                string uname = proxy_username;
                switch (proxy_auth_type) {
                case weoHttpProxyAuthBasic:
                    p_type = CURLAUTH_BASIC;
                    break;
                case weoHttpProxyAuthDigest:
                    p_type = CURLAUTH_DIGEST;
                    break;
                case weoHttpProxyAuthNTLM:
                    p_type = CURLAUTH_NTLM;
                    uname = proxy_domain + "\\" + proxy_username;
                    break;
                default:
                    p_type = CURLAUTH_ANY;
                    break;
                }
                curl_easy_setopt(ht_retval->CURLHandle(), CURLOPT_PROXYAUTH, p_type);
                curl_easy_setopt(ht_retval->CURLHandle(), CURLOPT_PROXYUSERNAME, uname.c_str());
                curl_easy_setopt(ht_retval->CURLHandle(), CURLOPT_PROXYPASSWORD, proxy_password.c_str());
            }
            if (use_localport && local_port > 0 && local_port < 65536) {
                curl_easy_setopt(ht_retval->CURLHandle(), CURLOPT_LOCALPORT, local_port);
                curl_easy_setopt(ht_retval->CURLHandle(), CURLOPT_LOCALPORTRANGE, local_range);
            }
            // set cookies
            // CURLOPT_COOKIE
            if (options[weoHttpAcceptCookies]) {
                time_t cookie_time = time(NULL);
                db_condition cq, ct1, ct2, cp;
                db_filter ctm;
                cq.field() = "auth_data.data_type";
                cq.operation() = db_condition::equal;
                cq.value() = 0;

                ct1.field() = "auth_data.timeout";
                ct1.operation() = db_condition::great_or_equal;
                ct1.value() = (int)cookie_time;

                ct2.field() = "auth_data.timeout";
                ct2.operation() = db_condition::equal;
                ct2.value() = -1;

                ctm.set(ct1)._or_(ct2);
                string cookie_value = "";
                if (kernel != NULL && kernel->storage() != NULL) {
                    db_filter cookie_query;
                    cookie_query.set(cq)._and_(ctm);
                    transport_url cookie_dom;
                    for (db_cursor cur = kernel->storage()->get(cookie_query, "auth_data"); cur.is_not_end(); ++cur) {
                        if (cookie_value != "") {
                            cookie_value += "; ";
                        }
                        /// @todo verify domain and path for cookie
                        cookie_dom.assign("http://" + cur["auth_data.domain"].get<string>());
                        if (ht_retval->BaseUrl().is_domain_equal(cookie_dom) ||
                                ht_retval->BaseUrl().is_host_equal(cookie_dom)) {
                            cookie_value += cur["auth_data.name"].get<string>();
                            cookie_value += "=";
                            cookie_value += cur["auth_data.value"].get<string>();
                        }
                    }
                    LOG4CXX_DEBUG(iLogger::GetLogger(), "Cookie: " << cookie_value);
                    curl_easy_setopt(ht_retval->CURLHandle(), CURLOPT_COOKIE, cookie_value.c_str());
                }
            }
            // set useragent
            string uag = ht_retval->GetUseragent();
            if (uag == "") {
                uag = useragent;
            }
            curl_easy_setopt(ht_retval->CURLHandle(), CURLOPT_USERAGENT, uag.c_str());
            curl_multi_add_handle(transferHandle, ht_retval->CURLHandle());
            process_requests();
        } catch (std::exception &e) {
            LOG4CXX_ERROR(iLogger::GetLogger(), "http_transport::request - curl_multi_add_handle failed: " << e.what());
        }
    } else {
        LOG4CXX_ERROR(iLogger::GetLogger(), "http_transport::request curl_transfer_handle: " << transferHandle <<
                      "; curl_request_handle: " << ht_retval->CURLHandle());
    }
    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn i_response_ptr http_transport::request( string url, i_response_ptr resp )
///
/// @brief  Requests.
/// @param	url	 - The url.
/// @param	resp - If non-null, the resp.
/// @retval	null if it fails, pointer to the HttpResponse else.
/// @throw  WeError if given transport_url is malformed and can't be reconstructed from the HttpResponse
////////////////////////////////////////////////////////////////////////////////////////////////////
i_response_ptr http_transport::request( string url, i_response_ptr resp /*= i_response_ptr(0)*/ )
{
    HttpRequest* req = new HttpRequest;

    if (req == NULL) {
        throw WeError(string("Can't create HttpRequest! ") + __FILE__ + boost::lexical_cast<std::string>(__LINE__));
        return boost::shared_ptr<i_response>((i_response*)NULL);
    }
    req->RequestUrl(url, resp.get());
    if (!req->RequestUrl().is_valid()) {
        if (resp != NULL) {
            req->RequestUrl().assign_with_referer(url, &(resp->RealUrl()));
        }
    }
    req->Method(HttpRequest::wemGet);
    return request(req, resp);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn i_response_ptr http_transport::request( transport_url& url, i_response_ptr resp )
///
/// @brief  Requests.
/// @param	url	 - The url.
/// @param	resp - If non-null, the resp.
/// @retval	null if it fails, pointer to the HttpResponse else.
/// @throw  WeError if given transport_url is malformed and can't be reconstructed from the HttpResponse
////////////////////////////////////////////////////////////////////////////////////////////////////
i_response_ptr http_transport::request( transport_url& url, i_response_ptr resp /*= i_response_ptr(0)*/ )
{
    HttpRequest* req = new HttpRequest;

    if (req == NULL) {
        throw WeError(string("Can't create HttpRequest! ") + __FILE__ + boost::lexical_cast<std::string>(__LINE__));
        return boost::shared_ptr<i_response>((i_response*)NULL);
    }
    req->RequestUrl(url, resp.get());
    req->Method(HttpRequest::wemGet);
    return request(req, resp);
}

const int http_transport::process_requests( void )
{
    int running_handles;
    int msgs_in_queue;
    CURLMsg *cmsg;
    response_list::iterator hnd;
    HttpResponse* resp;

    if (transferHandle != NULL) {
#ifdef _DEBUG
        LOG4CXX_TRACE(iLogger::GetLogger(), "http_transport::process_requests - curl_multi_perform");
#endif
        lastError = curl_multi_perform(transferHandle, &running_handles);

        /// @todo Implements accurate busy-loop
        while (lastError == CURLM_CALL_MULTI_PERFORM) {
#ifdef _DEBUG
            LOG4CXX_TRACE(iLogger::GetLogger(), "http_transport::process_requests - curl_multi_perform in the loop");
#endif
            boost::this_thread::sleep(boost::posix_time::millisec(10));
            lastError = curl_multi_perform(transferHandle, &running_handles);
        }

#ifdef _DEBUG
        LOG4CXX_TRACE(iLogger::GetLogger(), "http_transport::process_requests - curl_multi_info_read");
#endif
        cmsg = curl_multi_info_read(transferHandle, &msgs_in_queue);
        while (cmsg != NULL) {
            if (cmsg->msg == CURLMSG_DONE) {
                // find the response
                for (hnd = responces.begin(); hnd != responces.end();) {
                    try {
                        resp = dynamic_cast<HttpResponse*>((*hnd).get());
                        if ( resp->CURLHandle() == cmsg->easy_handle) {
#ifdef _DEBUG
                            LOG4CXX_TRACE(iLogger::GetLogger(), "http_transport::process_requests - curl_multi_remove_handle");
#endif
                            curl_multi_remove_handle(transferHandle, resp->CURLHandle());
                            resp->Process(this);
                            // and remove it from list
                            responces.erase(hnd);
                            hnd = responces.begin();
                            break;
                        }
                    } catch (std::exception &e) {
                        LOG4CXX_ERROR(iLogger::GetLogger(), "http_transport::process_requests - curl exception: " << e.what());
                    }; // just skip
                    hnd++;
                }
            }
            // delete cmsg;
#ifdef _DEBUG
            LOG4CXX_TRACE(iLogger::GetLogger(), "http_transport::process_requests - curl_multi_info_read, extract msg");
#endif
            cmsg = curl_multi_info_read(transferHandle, &msgs_in_queue);
        }
    } else {
        lastError = CURLM_BAD_HANDLE;
    }

    // check for timeouts
    boost::posix_time::ptime curr_tm = boost::posix_time::second_clock::local_time();
    curr_tm -= boost::posix_time::seconds(default_timeout);
    for (hnd = responces.begin(); hnd != responces.end();) {
        if ((*hnd)->start_time() < curr_tm) {
            // remove from list
            LOG4CXX_DEBUG(iLogger::GetLogger(), "http_transport::process_requests - timeout for request " << (*hnd)->BaseUrl().tostring());
            resp = dynamic_cast<HttpResponse*>((*hnd).get());
            // remove handle from cURL
            curl_multi_remove_handle(transferHandle, resp->CURLHandle());
            resp->timeout();
            // ... and remove from processing queue
            responces.erase(hnd);
            hnd = responces.begin();
        } else {
            hnd++;
        }
    }

    return lastError;
}

const string http_transport::get_setup_ui( void )
{
    return xrc;
}

void http_transport::init( task *data_provider )
{
    we_option opt;
    string uagent;
    bool val;

    uagent = "WebEngine ";
    uagent += WE_VERSION_PRODUCTSTR;
    uagent += WE_VERSION_PRODUCTSTR;
    uagent += " (cURL: ";
    uagent += curl_version();
    uagent += ")";

    kernel->register_request<HttpRequest>();

    opt = data_provider->Option(weoHttpPort);
    SAFE_GET_OPTION_VAL(opt, default_port, 80);

    opt = data_provider->Option(weoHttpsPort);
    SAFE_GET_OPTION_VAL(opt, default_sport, 443);

    opt = data_provider->Option(weoHttpProto);
    SAFE_GET_OPTION_VAL(opt, proto_name, "http");

    opt = data_provider->Option(weoHttpTimeout);
    SAFE_GET_OPTION_VAL(opt, default_timeout, 10);

    opt = data_provider->Option(weoHttpSizeLimit);
    SAFE_GET_OPTION_VAL(opt, max_doc_size, -1);

    opt = data_provider->Option(weoHttpUserAgent);
    SAFE_GET_OPTION_VAL(opt, useragent, uagent);
    if (useragent == "") {
        useragent = uagent;
    }

    opt = data_provider->Option(weoHttpAcceptCookies);
    SAFE_GET_OPTION_VAL(opt, val, false);
    options[weoHttpAcceptCookies] = val;

    opt = data_provider->Option(weoHttpUseLocalPort);
    SAFE_GET_OPTION_VAL(opt, use_localport, false);

    opt = data_provider->Option(weoHttpLocalPort);
    SAFE_GET_OPTION_VAL(opt, local_port, 0);

    opt = data_provider->Option(weoHttpLocalRange);
    SAFE_GET_OPTION_VAL(opt, local_range, 0);

    opt = data_provider->Option(weoHttpProxy);
    SAFE_GET_OPTION_VAL(opt, use_proxy, false);

    opt = data_provider->Option(weoHttpProxyAuth);
    SAFE_GET_OPTION_VAL(opt, proxy_auth_type, 0);

    opt = data_provider->Option(weoHttpProxyHost);
    SAFE_GET_OPTION_VAL(opt, proxy_host, "");

    opt = data_provider->Option(weoHttpProxyPort);
    SAFE_GET_OPTION_VAL(opt, proxy_port, 8080);

    opt = data_provider->Option(weoHttpProxyDomain);
    SAFE_GET_OPTION_VAL(opt, proxy_domain, "");

    opt = data_provider->Option(weoHttpProxyUname);
    SAFE_GET_OPTION_VAL(opt, proxy_username, "");

    opt = data_provider->Option(weoHttpProxyPswd);
    SAFE_GET_OPTION_VAL(opt, proxy_password, "");

    opt = data_provider->Option(weoCollapseSpaces);
    SAFE_GET_OPTION_VAL(opt, val, false);
    options[weoCollapseSpaces] = val;

    opt = data_provider->Option(weoFollowLinks);
    SAFE_GET_OPTION_VAL(opt, val, false);
    options[weoFollowLinks] = val;

}

bool http_transport::is_set( const string& name )
{
    map<string, bool>::iterator it;
    bool retval = false;

    it = options.find(name);
    if (it != options.end()) {
        retval = it->second;
    }
    LOG4CXX_TRACE(iLogger::GetLogger(), "http_transport::is_set(" << name << ") value=" << retval);
    return retval;
}

} // namespace webEngine
