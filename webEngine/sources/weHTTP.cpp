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
#include "weHttp.xrc"

#ifndef __DOXYGEN__
string WeHTTP::protoName = "http";
#endif // __DOXYGEN__

//////////////////////////////////////////////////////////////////////////
// Transport creation functions
//////////////////////////////////////////////////////////////////////////
static iweTransport* weCreateHttp(WeDispatch* krnl)
{
    FUNCTION;
    LOG4CXX_DEBUG(WeLogger::GetLogger(), "WeTransportFactory: create WeHTTP");
    return new WeHTTP(krnl); //new WeHTTP();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn WeHttpRequest::WeHttpRequest()
///
/// @brief  Default constructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
WeHttpRequest::WeHttpRequest()
{
    /// @todo set appropriate proxy
    proxy = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn WeHttpRequest::WeHttpRequest( string url,
///     weHttpMethod meth , WeHttpResponse* resp )
///
/// @brief  Constructs the object from the previous WeHttpResponse.
/// @param	url	 - The url.
/// @param	meth - The request method. Default - wemGet.
/// @param	resp - If non-null, the WeHttpResponse, to create chained WeHttpRequest to.
/// @throw  WeError if given URL is malformed and can't be reconstructed from the WeHttpResponse
////////////////////////////////////////////////////////////////////////////////////////////////////
WeHttpRequest::WeHttpRequest( string url, WeHttpRequest::weHttpMethod meth /*= wemGet*/, WeHttpResponse* resp /*= NULL*/ )
{
    /// @todo set appropriate proxy
    proxy = NULL;
    method = meth;
    RequestUrl(url, resp);
    /// @todo Implement this!
    LOG4CXX_WARN(WeLogger::GetLogger(), "WeHttpRequest::WeHttpRequest(string, weHttpMethod, WeHttpResponse*) - Not implemented");
}

void WeHttpRequest::RequestUrl( const string &ReqUrl, iweOperation* resp /*= NULL*/ )
{
    WeURL   weurl;

    weurl = ReqUrl;
    return RequestUrl(weurl, resp);
}

void WeHttpRequest::RequestUrl( const WeURL &ReqUrl, iweOperation* resp /*= NULL*/ )
{
    reqUrl = ReqUrl;
    if (resp != NULL) {
        /// @todo Make referers and other options for linked request
    }
}

void WeHttpRequest::ComposePost( int method /*= composeOverwrite*/ )
{
    if (method == wemPost || method == wemPut) {
        /// @todo Compose POST data
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	WeHTTP::WeHTTP()
///
/// @brief  Default constructor.
/// @throw  WeError if the curl initialization failed
////////////////////////////////////////////////////////////////////////////////////////////////////
WeHTTP::WeHTTP(WeDispatch* krnl, void* handle /*= NULL*/) :
    iweTransport(krnl, handle)
{
    transferHandle = curl_multi_init();
    if (transferHandle == NULL) {
        throw WeError("curl_multi_init failed!");
    }
    responces.clear();
    // iwePlugin structure
    pluginInfo.IfaceName = "WeHTTP";
    pluginInfo.IfaceList.push_back("WeHTTP");
    pluginInfo.PluginDesc = "HTTP transport interface";
    pluginInfo.PluginId = "A44A9A1E7C25";
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	WeHTTP::~WeHTTP()
///
/// @brief  Destructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
WeHTTP::~WeHTTP()
{
    WeResponseList::iterator hnd;
    WeHttpResponse* point;

    if (transferHandle != NULL) {
        for (hnd = responces.begin(); hnd != responces.end(); hnd++) {
            try {
                point = dynamic_cast<WeHttpResponse*>(*hnd);
                curl_multi_remove_handle(transferHandle, point->CURLHandle());
                delete point;
            }
            catch (...) {}; // just skip
        }
        curl_multi_cleanup(transferHandle);
    }
}

void* WeHTTP::GetInterface( const string& ifName )
{
    if (iequals(ifName, "WeHTTP"))
    {
        usageCount++;
        return (void*)(this);
    }
    if (iequals(ifName, "iweTransport"))
    {
        usageCount++;
        return (void*)((iweTransport*)this);
    }
    return iweTransport::GetInterface(ifName);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn iweResponse* WeHTTP::Request( iweRequest* req,
///     iweResponse* resp )
///
/// @brief  Requests.
///
/// @param  req  - If non-null, the HTTP request.
/// @param  resp - If non-null, the HTTP response to store results, else - the new WeHttpResponse
///                will be creaded.
/// @retval	null if it fails, pointer to the WeHttpResponse else.
////////////////////////////////////////////////////////////////////////////////////////////////////
iweResponse* WeHTTP::Request( iweRequest* req, iweResponse* resp /*= NULL*/ )
{
    WeHttpResponse* retval;

    if (!req->RequestUrl().IsValid()) {
        if (resp != NULL) {
            string url = req->RequestUrl().ToString();
            req->RequestUrl().Restore(url, &(resp->RealUrl()));
        }
    }

    if (!req->RequestUrl().IsValid()) {
        // bad luck! still not valid request... :(
        /// @todo Add error processing. Throw exception or return NULL
    }

    if (resp == NULL) {
        retval = new WeHttpResponse();
        retval->BaseUrl(req->RequestUrl());
        retval->RelocCount(0);
    }
    else {
        retval = dynamic_cast<WeHttpResponse*>(resp);
        if (!retval->BaseUrl().IsValid()) {
            retval->BaseUrl(req->RequestUrl());
        }
        if (!retval->CurlInit()) {
            /// @todo may be throw exception&
            return NULL;
        }
    }

    try {
        WeHttpRequest* r = dynamic_cast<WeHttpRequest*>(req);
        retval->CurlSetOpts(r);
    }
    catch (...) {}; // just skip
    retval->Processed(false);

    responces.push_back(retval);

    if (transferHandle != NULL) {
        curl_multi_add_handle(transferHandle, retval->CURLHandle());
        ProcessRequests();
    }
    return (iweResponse*)retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn iweResponse* WeHTTP::Request( string url,
///     iweResponse* resp )
///
/// @brief  Requests.
/// @param	url	 - The url.
/// @param	resp - If non-null, the resp.
/// @retval	null if it fails, pointer to the WeHttpResponse else.
/// @throw  WeError if given URL is malformed and can't be reconstructed from the WeHttpResponse
////////////////////////////////////////////////////////////////////////////////////////////////////
iweResponse* WeHTTP::Request( string url, iweResponse* resp /*= NULL*/ )
{
    WeHttpRequest* req = new WeHttpRequest;

    if (req == NULL) {
        throw WeError(string("Can't create WeHttpRequest! ") + __FILE__ + boost::lexical_cast<std::string>(__LINE__));
        return NULL;
    }
    req->RequestUrl(url, resp);
    if (!req->RequestUrl().IsValid()) {
        if (resp != NULL) {
            req->RequestUrl().Restore(url, &(resp->RealUrl()));
        }
    }
    req->Method(WeHttpRequest::wemGet);
    return Request(req, resp);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn iweResponse* WeHTTP::Request( WeURL& url,
///     iweResponse* resp )
///
/// @brief  Requests.
/// @param	url	 - The url.
/// @param	resp - If non-null, the resp.
/// @retval	null if it fails, pointer to the WeHttpResponse else.
/// @throw  WeError if given URL is malformed and can't be reconstructed from the WeHttpResponse
////////////////////////////////////////////////////////////////////////////////////////////////////
iweResponse* WeHTTP::Request( WeURL& url, iweResponse* resp /*= NULL*/ )
{
    WeHttpRequest* req = new WeHttpRequest;

    if (req == NULL) {
        throw WeError(string("Can't create WeHttpRequest! ") + __FILE__ + boost::lexical_cast<std::string>(__LINE__));
        return NULL;
    }
    req->RequestUrl(url, resp);
    req->Method(WeHttpRequest::wemGet);
    return Request(req, resp);
}

const int WeHTTP::ProcessRequests( void )
{
    int running_handles;
    int msgs_in_queue;
    CURLMsg *cmsg;
    WeResponseList::iterator hnd;
    WeHttpResponse* resp;

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
                        resp = dynamic_cast<WeHttpResponse*>(*hnd);
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

void WeHTTP::Register( WeTransportFactory* factory )
{
    factory->Add(protoName, weCreateHttp);
}

const string WeHTTP::GetSetupUI( void )
{
    return xrc;
}
