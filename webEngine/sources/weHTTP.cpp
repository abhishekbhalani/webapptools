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
#include <webEngine.h>

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <algorithm>
#include "weHelper.h"
#include "weHTTP.h"

static string xrc = "<plugin><category label='Basic settings' name='generic'>\
 <option name='httpTransport/Port' label='Port number' type='2' control='int'>80</option>\
 <option name='httpTransport/SizeLimit' label='Document size limit' type='2' control='int'>-1</option>\
 <category label='Authentication' name='auth'>\
  <option name='' label='Authentication methods'>&lt;composed&gt;\
   <option name='httpTransport/Auth/Base' label='Basic HTTP' type='6' control='bool' chkbox='1'>0</option>\
   <option name='httpTransport/Auth/NTLM' label='NTLM' type='6' control='bool' chkbox='1'>0</option>\
  </option>\
  <option name='httpTransport/Auth/Username' label='Username' type='8' control='string'></option>\
  <option name='httpTransport/Auth/Password' label='Password' type='8' control='string' mode='password'></option>\
 </category>\
 <category label='Proxy' name='proxy'>\
  <option name='' label='Proxy type'>&lt;composed&gt;\
   <option name='httpTransport/Proxy/HTTP' label='HTTP' type='6' control='bool' chkbox='1'>0</option>\
   <option name='httpTransport/Proxy/Socks' label='Socks' type='6' control='bool' chkbox='1'>0</option>\
  </option>\
  <option name='httpTransport/Proxy/Host' label='Proxy host' type='8' control='string'></option>\
  <option name='httpTransport/Proxy/Port' label='Proxy port' type='2' control='int'>8080</option>\
  <option name='httpTransport/Proxy/Username' label='Username' type='8' control='string'></option>\
  <option name='httpTransport/Proxy/Password' label='Password' type='8' control='string' mode='password'></option>\
 </category>\
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
    /// @todo set appropriate proxy
    proxy = NULL;
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
    /// @todo set appropriate proxy
    proxy = NULL;
    method = meth;
    RequestUrl(url, resp);
    /// @todo Implement this!
    //LOG4CXX_WARN(iLogger::GetLogger(), "HttpRequest::HttpRequest(string, weHttpMethod, HttpResponse*) - Not implemented");
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
                delete point;
            }
            catch (...) {}; // just skip
        }
        curl_multi_cleanup(transferHandle);
    }
}

i_plugin* http_transport::get_interface( const string& ifName )
{
    if (iequals(ifName, "http_transport"))
    {
        usageCount++;
        return this;
    }
    if (iequals(ifName, "i_transport"))
    {
        usageCount++;
        return this;
    }
    return i_transport::get_interface(ifName);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn i_response* http_transport::request( i_request* req,
///     i_response* resp )
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
    wOption opt;

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
            req->RequestUrl().port = default_port;
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
    }
    else {
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
    }
    catch (...) {}; // just skip
    ht_retval->Processed(false);

    responces.push_back(i_response_ptr(retval));

    if (transferHandle != NULL && ht_retval->CURLHandle() != NULL) {
        try {
            // set the timeout
            ht_retval->start_time(boost::posix_time::second_clock::local_time());
            curl_easy_setopt(ht_retval->CURLHandle(), CURLOPT_TIMEOUT, default_timeout);
            curl_multi_add_handle(transferHandle, ht_retval->CURLHandle());
            process_requests();
        }
        catch (std::exception &e) {
            LOG4CXX_ERROR(iLogger::GetLogger(), "http_transport::request - curl_multi_add_handle failed: " << e.what());
        }
    }
    else {
        LOG4CXX_ERROR(iLogger::GetLogger(), "http_transport::request curl_transfer_handle: " << transferHandle <<
            "; curl_request_handle: " << ht_retval->CURLHandle());
    }
    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn i_response* http_transport::request( string url,
///     i_response* resp )
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
/// @fn i_response* http_transport::request( transport_url& url,
///     i_response* resp )
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
        while (cmsg != NULL)
        {
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
                    }
                    catch (std::exception &e) {
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
    }
    else {
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
        }
        else {
            hnd++;
        }
    }

    return lastError;
}

const string http_transport::get_setup_ui( void )
{
    return xrc;
}

void http_transport::load_settings( i_options_provider *data_provider, string key /*= ""*/ )
{
    if (key == "") {
        key = "httpTransport";
    }
    wOption opt;
    bool val;

    opt = data_provider->Option(key + "/port");
    SAFE_GET_OPTION_VAL(opt, default_port, 80);

    opt = data_provider->Option(key + "/protocol");
    SAFE_GET_OPTION_VAL(opt, proto_name, "http");

    opt = data_provider->Option(key + weoCollapseSpaces);
    SAFE_GET_OPTION_VAL(opt, val, false);
    options[weoCollapseSpaces] = val;

    opt = data_provider->Option(key + weoFollowLinks);
    SAFE_GET_OPTION_VAL(opt, val, false);
    options[weoFollowLinks] = val;

    opt = data_provider->Option(key + "/timeout");
    SAFE_GET_OPTION_VAL(opt, default_timeout, 10);
}

bool http_transport::is_set( const string& name )
{
    map<string, bool>::iterator it;
    bool retval = false;

    it = options.find(name);
    if (it != options.end())
    {
        retval = it->second;
    }
    LOG4CXX_TRACE(iLogger::GetLogger(), "http_transport::is_set(" << name << ") value=" << retval);
    return retval;
}

} // namespace webEngine
