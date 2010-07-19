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

    class http_transport;
    class HttpRequest;

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
    class HttpResponse : public i_response
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

        void Process(i_transport* proc);
        virtual void timeout();

    protected:
        static size_t Receiver(void *ptr, size_t size, size_t nmemb, void *ourpointer);
        static size_t HeadersRecv(void *ptr, size_t size, size_t nmemb, void *ourpointer);

#ifndef __DOXYGEN__
    protected:
        StringLinks headers;
        StringLinks cookies;
        int httpCode;
        blob headData;
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
    class HttpRequest : public i_request
    {
    public:
        /* types and constants */
        enum weHttpMethod {wemGet, wemPost, wemPut, wemHead, wemTrace};
        enum {composeOverwrite, composeAdd};
    public:
        HttpRequest();
        HttpRequest(string url, weHttpMethod meth = wemGet, HttpResponse* resp = NULL);

        virtual transport_url  &RequestUrl(void)  { return(reqUrl);   };
        virtual void RequestUrl(const string &ReqUrl, i_operation* resp = NULL);
        virtual void RequestUrl(const transport_url &ReqUrl, i_operation* resp = NULL);

        void ComposePost(int method = composeOverwrite);

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Unstructured data for the request
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        blob& Data()  { return data;  };

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

        virtual i_response_ptr abort_request();

#ifndef __DOXYGEN__
    protected:
        weHttpMethod    method;
        transport_url   reqUrl;
        blob            data;
        StringLinks     postData;
#endif //__DOXYGEN__
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  http_transport
    ///
    /// @brief  The HTTP processor.
    ///
    /// @author A. Abramov
    /// @date   29.05.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class http_transport : public i_transport
    {
    public:
        http_transport(engine_dispatcher* krnl, void* handle = NULL);
        ~http_transport();

        virtual i_plugin* get_interface( const string& ifName );
        virtual const string get_setup_ui( void );
        virtual void init(task* tsk) {}
        virtual void pause(task* tsk, bool paused = true) {}
        virtual void stop(task* tsk) {}

        // @brief Initialize default values
        virtual void load_settings(i_options_provider *data_provider, string key = "");
        virtual bool is_set(const string& name);


        virtual i_response_ptr request(i_request* req, i_response_ptr resp = i_response_ptr((i_response*)NULL) );
        virtual i_response_ptr request(string url, i_response_ptr resp = i_response_ptr((i_response*)NULL) );
        virtual i_response_ptr request(transport_url& url, i_response_ptr resp = i_response_ptr((i_response*)NULL) );

        /// @brief  Gets the CURLMcode - the last cURL operation status.
        const CURLMcode &get_last_error(void) const   { return(lastError);    };

        /// @brief  Process pending requests
        virtual const int process_requests(void);

        virtual string& get_name()                   { return proto_name; };
        virtual bool is_own_protocol(string& proto)  { return iequals(proto, proto_name); };

#ifndef __DOXYGEN__
    protected:
        CURLM*      transferHandle;
        CURLMcode   lastError;
        int         default_port;
        int         default_timeout;
        string      proto_name;
        int         max_doc_size;
        bool        use_localport;
        int         local_port;
        int         local_range;
        int         use_proxy; /// one of weoHttpProxyType* constant
        int         proxy_auth_type;  /// 0 - none, 1 - basic, 2 - digest, 3 - NTLM
        string      proxy_host;
        int         proxy_port;
        string      proxy_domain;
        string      proxy_username;
        string      proxy_password;
        map<string, bool>   options;
#endif //__DOXYGEN__
    };

} // namespace webEngine

#define weoHttpProxyTypeNone 0
#define weoHttpProxyTypeHTTP 1
#define weoHttpProxyTypeSocks4 2
#define weoHttpProxyTypeSocks4a 3
#define weoHttpProxyTypeSocks5 4

#define weoHttpProxyAuthNone 0
#define weoHttpProxyAuthBasic 1
#define weoHttpProxyAuthDigest 2
#define weoHttpProxyAuthNTLM 3

// HttpTransport options
#define weoHttpPort "httpTransport/port"
#define weoHttpProto "httpTransport/protocol"
#define weoHttpTimeout "httpTransport/timeout"
#define weoHttpSizeLimit "httpTransport/size_Limit"
#define weoHttpAcceptCookies "httpTransport/cookies"
#define weoHttpUseLocalPort "httpTransport/use_localport"
#define weoHttpLocalPort "httpTransport/local_port"
#define weoHttpLocalRange "httpTransport/local_range"
#define weoHttpProxy "httpTransport/Proxy"
#define weoHttpProxyAuth "httpTransport/ProxyAuth"
#define weoHttpProxyHost "httpTransport/Proxy/host"
#define weoHttpProxyPort "httpTransport/Proxy/port"
#define weoHttpProxyDomain "httpTransport/Proxy/domain"
#define weoHttpProxyUname "httpTransport/Proxy/username"
#define weoHttpProxyPswd "httpTransport/Proxy/password"

#endif //__WEHTTP_H__
