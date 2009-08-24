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
#ifndef __WEHTTP_H__
#define __WEHTTP_H__

#include "weStrings.h"
#include "weBlob.h"
#include "weUrl.h"
#include "weiTransport.h"
#include <curl/curl.h>
#include <boost/algorithm/string/predicate.hpp>

using namespace boost::algorithm;

namespace webEngine {

    class HttpTransport;
    class HttpRequest;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  WeProxy
    ///
    /// @brief  Proxy definition for HttpRequest.
    ///
    /// @author A. Abramov
    /// @date   01.06.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class WeProxy
    {
    public:
        URL   proxyAddr;
        curl_proxytype type;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  HttpResponse
    ///
    /// @brief  HTTP response.
    ///
    /// Just the HTTP response - code and header and unstructured data. To structure the response data
    /// use the WeDocument instead.
    /// @author A. Abramov
    /// @date   29.05.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class HttpResponse : public iResponse
    {
    public:
        HttpResponse();
        ~HttpResponse();

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief  HTTP headers of the document.
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        StringLinks& Headers(void)     { return (headers); };

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief  HTTP cookies of the document.
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        StringLinks& Cookies(void)     { return (cookies); };

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
        /// @param req  - The HttpRequest, if not NULL - sets the request options, else sets only
        ///               default options
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        void CurlSetOpts(HttpRequest* req = NULL);

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

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief  Gets the content type.
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        const string ContentType(void) { return(contentType);    };

        void Process(iTransport* proc);

    protected:
        static size_t Receiver(void *ptr, size_t size, size_t nmemb, void *ourpointer);
        static size_t HeadersRecv(void *ptr, size_t size, size_t nmemb, void *ourpointer);

#ifndef __DOXYGEN__
    protected:
        StringLinks headers;
        StringLinks cookies;
        int httpCode;
        Blob headData;
        string contentType;
        CURL* curlHandle;
        CURLcode lastError;
        char errorBuff[CURL_ERROR_SIZE];

        // private:
        //     HttpResponse(HttpResponse&) {};             ///< Avoid object coping
        //     HttpResponse& operator=(HttpResponse&) {};  ///< Avoid object coping
#endif //__DOXYGEN__
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  HttpRequest
    ///
    /// @brief  HTTP request.
    ///
    /// @author A. Abramov
    /// @date   29.05.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class HttpRequest : public iRequest
    {
    public:
        /* types and constants */
        enum weHttpMethod {wemGet, wemPost, wemPut, wemHead, wemTrace};
        enum {composeOverwrite, composeAdd};
    public:
        HttpRequest();
        HttpRequest(string url, weHttpMethod meth = wemGet, HttpResponse* resp = NULL);

        virtual URL  &RequestUrl(void)  { return(reqUrl);   };
        virtual void RequestUrl(const string &ReqUrl, iOperation* resp = NULL);
        virtual void RequestUrl(const URL &ReqUrl, iOperation* resp = NULL);

        void ComposePost(int method = composeOverwrite);

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Unstructured data for the request
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        Blob& Data()  { return data;  };

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief POST method structured data
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        StringLinks& PostData()    { return postData;  };

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
        URL           reqUrl;
        Blob          data;
        StringLinks    postData;
        WeProxy         *proxy;
#endif //__DOXYGEN__
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  HttpTransport
    ///
    /// @brief  The HTTP processor.
    ///
    /// @author A. Abramov
    /// @date   29.05.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class HttpTransport : public iTransport
    {
    public:
        HttpTransport(Dispatch* krnl, void* handle = NULL);
        ~HttpTransport();

        virtual void* GetInterface( const string& ifName );
        virtual const string GetSetupUI( void );

        virtual iResponse* Request(iRequest* req, iResponse* resp = NULL);
        virtual iResponse* Request(string url, iResponse* resp = NULL);
        virtual iResponse* Request(URL& url, iResponse* resp = NULL);

        /// @brief  Gets the CURLMcode - the last cURL operation status.
        const CURLMcode &GetLastError(void) const   { return(lastError);    };

        /// @brief  Process pending requests
        virtual const int ProcessRequests(void);

        virtual string& GetName()                   { return protoName; };
        virtual bool IsOwnProtocol(string& proto)   {return iequals(proto, protoName); };

#ifndef __DOXYGEN__
    protected:
        CURLM*          transferHandle;
        CURLMcode       lastError;

    private:
        static string   protoName;
#endif //__DOXYGEN__
    };

} // namespace webEngine

#endif //__WEHTTP_H__
