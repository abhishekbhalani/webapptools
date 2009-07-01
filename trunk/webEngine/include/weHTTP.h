/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew Abramov aabramov@ptsecurity.ru

    This file is part of webEngine

    webEngineis free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngineis distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __WEHTTP_H__
#define __WEHTTP_H__

#include "weStrings.h"
#include "weBlob.h"
#include "weUrl.h"
#include "weiTransport.h"
#include <curl/curl.h>
#include <boost/algorithm/string/predicate.hpp>

using namespace boost::algorithm;

class WeHTTP;
class WeHttpRequest;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  WeProxy
///
/// @brief  Proxy definition for WeHttpRequest.
///
/// @author A. Abramov
/// @date   01.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class WeProxy
{
public:
    WeURL   proxyAddr;
    curl_proxytype type;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  WeHttpResponse
///
/// @brief  HTTP response.
///
/// Just the HTTP response - code and header and unstructured data. To structure the response data
/// use the WeDocument instead.
/// @author A. Abramov
/// @date   29.05.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class WeHttpResponse : public iweResponse
{
public:
    WeHttpResponse();
    ~WeHttpResponse();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief  HTTP headers of the document.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    WeStringLinks& Headers(void)     { return (headers); };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief  HTTP cookies of the document.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    WeStringLinks& Cookies(void)     { return (cookies); };

    //@{
    /// @brief  HTTP response code for the document.
    ///
    ///
    int HttpCode(void)      { return httpCode;  };
    void HttpCode(int code) { httpCode = code;  };
    //@}

    //@{
    /// @brief  cURL transfer handle
    ///
    CURL* CURLHandle(void)  { return curlHandle; };
    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Sets the options of CURL handle
    /// @param req  - The WeHttpRequest, if not NULL - sets the request options, else sets only
    ///               default options
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void CurlSetOpts(WeHttpRequest* req = NULL);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief  Gets the CURLcode - the last cURL operation status.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    const CURLcode &GetLastError(void) const   { return(lastError);    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief  Gets the human readable error message.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    const char* GetErrorMessage(void) const   { return(errorBuff);    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief  Gets the human readable error message.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    bool CurlInit(void);

    void Process(iweTransport* proc);

protected:
    static size_t Receiver(void *ptr, size_t size, size_t nmemb, void *ourpointer);
    static size_t HeadersRecv(void *ptr, size_t size, size_t nmemb, void *ourpointer);

#ifndef __DOXYGEN__
protected:
    WeStringLinks headers;
    WeStringLinks cookies;
    int httpCode;
    WeBlob headData;
    CURL* curlHandle;
    CURLcode lastError;
    char errorBuff[CURL_ERROR_SIZE];

// private:
//     WeHttpResponse(WeHttpResponse&) {};             ///< Avoid object coping
//     WeHttpResponse& operator=(WeHttpResponse&) {};  ///< Avoid object coping
#endif //__DOXYGEN__
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  WeHttpRequest
///
/// @brief  HTTP request.
///
/// @author A. Abramov
/// @date   29.05.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class WeHttpRequest : public iweRequest
{
public:
    /* types and constants */
    enum weHttpMethod {wemGet, wemPost, wemPut, wemHead, wemTrace};
    enum {composeOverwrite, composeAdd};
public:
    WeHttpRequest();
    WeHttpRequest(string url, weHttpMethod meth = wemGet, WeHttpResponse* resp = NULL);

    virtual WeURL  &RequestUrl(void)  { return(reqUrl);   };
    virtual void RequestUrl(const string &ReqUrl, iweOperation* resp = NULL);
    virtual void RequestUrl(const WeURL &ReqUrl, iweOperation* resp = NULL);

    void ComposePost(int method = composeOverwrite);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Unstructured data for the request
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    WeBlob& Data()  { return data;  };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief POST method structured data
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    WeStringLinks& PostData()    { return postData;  };

    //@{
    /// @brief  Access the Method property.
    ///
    ///
    const weHttpMethod &Method(void) const  { return(method);   };
    void Method(const weHttpMethod &meth)   { method = meth;    };
    //@}

    // Access the Proxy
    WeProxy *Proxy(void) const  { return(proxy);    };
    void Proxy(WeProxy *prx)    { proxy = prx;      };

#ifndef __DOXYGEN__
protected:
    weHttpMethod    method;
    WeURL           reqUrl;
    WeBlob          data;
    WeStringLinks    postData;
    WeProxy         *proxy;
#endif //__DOXYGEN__
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  WeHTTP
///
/// @brief  The HTTP processor.
///
/// @author A. Abramov
/// @date   29.05.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class WeHTTP : public iweTransport
{
public:
    WeHTTP();
    ~WeHTTP();

    virtual iweResponse* Request(iweRequest* req, iweResponse* resp = NULL);
    virtual iweResponse* Request(string url, iweResponse* resp = NULL);
    virtual iweResponse* Request(WeURL& url, iweResponse* resp = NULL);

    /// @brief  Gets the CURLMcode - the last cURL operation status.
    const CURLMcode &GetLastError(void) const   { return(lastError);    };

    /// @brief  Process pending requests
    const CURLMcode ProcessRequests(void);

    virtual string& GetName()                   { return protoName; };
    virtual bool IsOwnProtocol(string& proto)   {return iequals(proto, protoName); };

#ifndef __DOXYGEN__
protected:
    CURLM*          transferHandle;
    CURLMcode       lastError;

private:
    static string   protoName;
    WeHTTP(WeHTTP&) {};             ///< Avoid object coping
    WeHTTP& operator=(WeHTTP&) { return *this; };  ///< Avoid object coping
#endif //__DOXYGEN__
};

#endif //__WEHTTP_H__
