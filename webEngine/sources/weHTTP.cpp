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
#include <boost/lexical_cast.hpp>
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

#ifndef __DOXYGEN__
string HttpTransport::protoName = "http";
#endif // __DOXYGEN__

//////////////////////////////////////////////////////////////////////////
// Transport creation functions
//////////////////////////////////////////////////////////////////////////
static iTransport* weCreateHttp(engine_dispatcher* krnl)
{
    LOG4CXX_DEBUG(iLogger::GetLogger(), "TransportFactory: create HttpTransport");
    return new HttpTransport(krnl); //new HttpTransport();
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

void HttpRequest::RequestUrl( const string &ReqUrl, iOperation* resp /*= NULL*/ )
{
    transport_url   weurl;

    weurl = ReqUrl;
    return RequestUrl(weurl, resp);
}

void HttpRequest::RequestUrl( const transport_url &ReqUrl, iOperation* resp /*= NULL*/ )
{
    reqUrl = ReqUrl;
    if (resp != NULL) {
        /// @todo Make referers and other options for linked request
    }
}

void HttpRequest::ComposePost( int method /*= composeOverwrite*/ )
{
    if (method == wemPost || method == wemPut) {
        /// @todo Compose POST data
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	HttpTransport::HttpTransport()
///
/// @brief  Default constructor.
/// @throw  WeError if the curl initialization failed
////////////////////////////////////////////////////////////////////////////////////////////////////
HttpTransport::HttpTransport(engine_dispatcher* krnl, void* handle /*= NULL*/) :
    iTransport(krnl, handle)
{
    transferHandle = curl_multi_init();
    if (transferHandle == NULL) {
        throw WeError("curl_multi_init failed!");
    }
    responces.clear();
    // iwePlugin structure
    pluginInfo.interface_name = "HttpTransport";
    pluginInfo.interface_list.push_back("HttpTransport");
    pluginInfo.plugin_desc = "HTTP transport interface";
    pluginInfo.plugin_id = "A44A9A1E7C25";
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	HttpTransport::~HttpTransport()
///
/// @brief  Destructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
HttpTransport::~HttpTransport()
{
    ResponseList::iterator hnd;
    HttpResponse* point;

    if (transferHandle != NULL) {
        for (hnd = responces.begin(); hnd != responces.end(); hnd++) {
            try {
                point = dynamic_cast<HttpResponse*>(*hnd);
                curl_multi_remove_handle(transferHandle, point->CURLHandle());
                delete point;
            }
            catch (...) {}; // just skip
        }
        curl_multi_cleanup(transferHandle);
    }
}

void* HttpTransport::GetInterface( const string& ifName )
{
    if (iequals(ifName, "HttpTransport"))
    {
        usageCount++;
        return (void*)(this);
    }
    if (iequals(ifName, "iTransport"))
    {
        usageCount++;
        return (void*)((iTransport*)this);
    }
    return iTransport::GetInterface(ifName);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn iResponse* HttpTransport::Request( iRequest* req,
///     iResponse* resp )
///
/// @brief  Requests.
///
/// @param  req  - If non-null, the HTTP request.
/// @param  resp - If non-null, the HTTP response to store results, else - the new HttpResponse
///                will be creaded.
/// @retval	null if it fails, pointer to the HttpResponse else.
////////////////////////////////////////////////////////////////////////////////////////////////////
iResponse* HttpTransport::Request( iRequest* req, iResponse* resp /*= NULL*/ )
{
    HttpResponse* retval;
    wOption opt;
    int portNum;

    if (!req->RequestUrl().is_valid()) {
        if (resp != NULL) {
            string url = req->RequestUrl().tostring();
            req->RequestUrl().assign_with_referer(url, &(resp->RealUrl()));
        }
    }

    if (!req->RequestUrl().is_valid()) {
        if (req->RequestUrl().protocol == "") {
            req->RequestUrl().protocol = protoName;
        }
        if (req->RequestUrl().port < 1 || req->RequestUrl().port > 65535) {
            opt = Option("httpTransport/Port");
            SAFE_GET_OPTION_VAL(opt, portNum, 0);
            req->RequestUrl().port = portNum;
        }
    }
    LOG4CXX_DEBUG(iLogger::GetLogger(), "HttpTransport::Request: url=" << req->RequestUrl().tostring());

    if (!req->RequestUrl().is_valid()) {
        // bad luck! still not valid request... :(
        /// @todo Add error processing. Throw exception or return NULL
    }

    if (resp == NULL) {
        retval = new HttpResponse();
        retval->BaseUrl(req->RequestUrl());
        retval->RelocCount(0);
    }
    else {
        retval = dynamic_cast<HttpResponse*>(resp);
        if (!retval->BaseUrl().is_valid()) {
            retval->BaseUrl(req->RequestUrl());
        }
        if (!retval->CurlInit()) {
            /// @todo may be throw exception&
            return NULL;
        }
    }

    try {
        HttpRequest* r = dynamic_cast<HttpRequest*>(req);
        retval->CurlSetOpts(r);
    }
    catch (...) {}; // just skip
    retval->Processed(false);

    responces.push_back(retval);

    if (transferHandle != NULL) {
        curl_multi_add_handle(transferHandle, retval->CURLHandle());
        ProcessRequests();
    }
    return (iResponse*)retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn iResponse* HttpTransport::Request( string url,
///     iResponse* resp )
///
/// @brief  Requests.
/// @param	url	 - The url.
/// @param	resp - If non-null, the resp.
/// @retval	null if it fails, pointer to the HttpResponse else.
/// @throw  WeError if given transport_url is malformed and can't be reconstructed from the HttpResponse
////////////////////////////////////////////////////////////////////////////////////////////////////
iResponse* HttpTransport::Request( string url, iResponse* resp /*= NULL*/ )
{
    HttpRequest* req = new HttpRequest;

    if (req == NULL) {
        throw WeError(string("Can't create HttpRequest! ") + __FILE__ + boost::lexical_cast<std::string>(__LINE__));
        return NULL;
    }
    req->RequestUrl(url, resp);
    if (!req->RequestUrl().is_valid()) {
        if (resp != NULL) {
            req->RequestUrl().assign_with_referer(url, &(resp->RealUrl()));
        }
    }
    req->Method(HttpRequest::wemGet);
    return Request(req, resp);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn iResponse* HttpTransport::Request( transport_url& url,
///     iResponse* resp )
///
/// @brief  Requests.
/// @param	url	 - The url.
/// @param	resp - If non-null, the resp.
/// @retval	null if it fails, pointer to the HttpResponse else.
/// @throw  WeError if given transport_url is malformed and can't be reconstructed from the HttpResponse
////////////////////////////////////////////////////////////////////////////////////////////////////
iResponse* HttpTransport::Request( transport_url& url, iResponse* resp /*= NULL*/ )
{
    HttpRequest* req = new HttpRequest;

    if (req == NULL) {
        throw WeError(string("Can't create HttpRequest! ") + __FILE__ + boost::lexical_cast<std::string>(__LINE__));
        return NULL;
    }
    req->RequestUrl(url, resp);
    req->Method(HttpRequest::wemGet);
    return Request(req, resp);
}

const int HttpTransport::ProcessRequests( void )
{
    int running_handles;
    int msgs_in_queue;
    CURLMsg *cmsg;
    ResponseList::iterator hnd;
    HttpResponse* resp;

    if (transferHandle != NULL) {
        lastError = curl_multi_perform(transferHandle, &running_handles);

        /// @todo Implements accurate busy-loop
        while (lastError == CURLM_CALL_MULTI_PERFORM) {
            lastError = curl_multi_perform(transferHandle, &running_handles);
        }

        cmsg = curl_multi_info_read(transferHandle, &msgs_in_queue);
        while (cmsg != NULL)
        {
            if (cmsg->msg == CURLMSG_DONE) {
                // find the response
                for (hnd = responces.begin(); hnd != responces.end();) {
                    try {
                        resp = dynamic_cast<HttpResponse*>(*hnd);
                        if ( resp->CURLHandle() == cmsg->easy_handle) {
                            responces.erase(hnd);
                            hnd = responces.begin();
                            curl_multi_remove_handle(transferHandle, resp->CURLHandle());
                            resp->Process(this);
                            break;
                        }
                    }
                    catch (...) {}; // just skip
                    hnd++;
                }
            }
            // delete cmsg;
            cmsg = curl_multi_info_read(transferHandle, &msgs_in_queue);
        }
    }
    else {
        lastError = CURLM_BAD_HANDLE;
    }
    return lastError;
}

const string HttpTransport::GetSetupUI( void )
{
    return xrc;
}

} // namespace webEngine