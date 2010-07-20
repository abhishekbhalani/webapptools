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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <algorithm>
#include <weHelper.h>
#include <weHTTP.h>
#include <weDbstruct.h>
#include <weiStorage.h>
#include <weOptions.h>

using namespace std;
namespace webEngine {

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	HttpResponse::HttpResponse()
///
/// @brief  Default constructor.
/// @throw  WeError if curl_easy_init failed
////////////////////////////////////////////////////////////////////////////////////////////////////
HttpResponse::HttpResponse() :
headers(": ", "[\\n\\r]+"), cookies("=", "; ")
{
    curlHandle = NULL;
    baseUrl = "";
    realUrl = "";
    relocCount = 0;
    processed = false;
    httpCode = 0;
    data.clear();
    contentType = "";
    headData.clear();
    if (!CurlInit()) {
        throw WeError("curl_easy_init failed!");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	HttpResponse::~HttpResponse()
///
/// @brief  Destructor. 
////////////////////////////////////////////////////////////////////////////////////////////////////
HttpResponse::~HttpResponse()
{
    if (curlHandle != NULL) {
        curl_easy_cleanup(curlHandle);
    }
    headers.clear();
    cookies.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn bool HttpResponse::CurlInit( void )
///
/// @brief  CURL initialize. 
///
/// @retval	true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool HttpResponse::CurlInit( void )
{
    /// @todo reset handle without recreate connection
    if (curlHandle != NULL) {
        curl_easy_cleanup(curlHandle);
    }

    curlHandle = curl_easy_init();
    memset(errorBuff, 0, CURL_ERROR_SIZE);
    if (curlHandle == NULL) {
        return false;
    }
    curl_easy_reset(curlHandle);
    curl_easy_setopt(curlHandle, CURLOPT_ERRORBUFFER, errorBuff);
    curl_easy_setopt(curlHandle, CURLOPT_HTTP_TRANSFER_DECODING, 1);
    curl_easy_setopt(curlHandle, CURLOPT_PROXY, "");
    curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, &HttpResponse::Receiver);
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(curlHandle, CURLOPT_HEADERFUNCTION, &HttpResponse::HeadersRecv);
    curl_easy_setopt(curlHandle, CURLOPT_WRITEHEADER, this);
    return true;
}

void HttpResponse::CurlSetOpts( HttpRequest* req /*= NULL*/ )
{
    if (req != NULL) {
        /// @todo Sets the request options
        curl_easy_setopt(curlHandle, CURLOPT_URL, req->RequestUrl().tostring().c_str());
        if (req->Method() == HttpRequest::wemGet) {
            curl_easy_setopt(curlHandle, CURLOPT_HTTPGET, 1);
        }
        if (req->Method() == HttpRequest::wemPost) {
            curl_easy_setopt(curlHandle, CURLOPT_POST, 1);
            curl_easy_setopt(curlHandle, CURLOPT_POSTFIELDS, &(req->Data()[0]));
            curl_easy_setopt(curlHandle, CURLOPT_POSTFIELDSIZE, req->Data().size());
        }
        if (req->Method() == HttpRequest::wemPut) {
            curl_easy_setopt(curlHandle, CURLOPT_UPLOAD, 1);
            curl_easy_setopt(curlHandle, CURLOPT_READDATA, &(req->Data()[0]));
            curl_easy_setopt(curlHandle, CURLOPT_INFILESIZE, req->Data().size());
        }
    }
}

void HttpResponse::Process(i_transport* trans)
{
    string c_value;
    string dom;
    string pth;
    string expr;
    char *st;
    double tminf;

    if (curlHandle) {
        lastError = curl_easy_getinfo(curlHandle, CURLINFO_CONTENT_TYPE, &st);
        if (lastError == CURLE_OK && st != NULL) {
            contentType = st;
        }
        lastError = curl_easy_getinfo(curlHandle, CURLINFO_RESPONSE_CODE, &httpCode);
        lastError = curl_easy_getinfo(curlHandle, CURLINFO_EFFECTIVE_URL, &st);
        lastError = curl_easy_getinfo(curlHandle, CURLINFO_TOTAL_TIME, &tminf);
        downTime = (int)tminf;
        if (lastError == CURLE_OK) {
            realUrl = st;
            //delete str;
        }
        /// lastError = curl_easy_getinfo(curlHandle, CURLINFO_REDIRECT_COUNT, &relocCount);
        processed = true;
    }
    data.push_back(0);
    headData.push_back(0);
    headers.Parse((char*)&headData[0]);
    if (trans != NULL) {
        /// @todo Process options
        // process cookies
        if (trans->is_set(weoHttpAcceptCookies)) {
            c_value = headers.find_first("Set-Cookie");
            cookies.Parse(c_value);
            dom = cookies.ifind_first("domain");
            if (dom == "") {
                cookies.append("domain", realUrl.host);
                dom = realUrl.host;
            }
            pth = cookies.ifind_first("path");
            if (pth == "") {
                cookies.append("path", "/");
                pth = "/";
            }
            expr = cookies.ifind_first("expires");
            if (trans->storage() != NULL) {
                cookies.ierase("domain");
                cookies.ierase("path");
                cookies.ierase("expires");
                int tmout = -1;
                try {
                    boost::posix_time::ptime  tm_boost;
                    tm_boost = boost::posix_time::time_from_string(expr);
                    struct tm tm_time;
                    tm_time = boost::posix_time::to_tm(tm_boost); 
                    tmout = (int)mktime( &tm_time );
                }
                catch(...) {};
                StringLinks::data_list::iterator it;
                db_recordset dataset;
                dataset.set_names(trans->storage()->get_namespace_struct("auth_data"));
                db_cursor rec = dataset.push_back();
                db_query query;
                query.what = trans->storage()->get_namespace_struct("auth_data");
                db_condition cq, cn, cd, cp;
                cq.field() = "auth_data.data_type";
                cq.operation() = db_condition::equal;
                cq.value() = 0;

                cn.field() = "auth_data.name";
                cn.operation() = db_condition::equal;
                cn.value() = string("");

                cd.field() = "auth_data.domain";
                cd.operation() = db_condition::equal;
                cd.value() = dom;

                cp.field() = "auth_data.path";
                cp.operation() = db_condition::equal;
                cp.value() = pth;

                for (it = cookies.begin(); it != cookies.end(); ++it)
                {   // foreach cookie value
                    cn.value() = it->first;
                    query.where.set(cq).and(cn).and(cd).and(cp);

                    rec["auth_data.task_id"] = 0;
                    rec["auth_data.data_type"] = 0; // 0 == cookie
                    rec["auth_data.name"] = it->first;
                    rec["auth_data.value"] = it->second;
                    rec["auth_data.timeout"] = tmout;
                    rec["auth_data.path"] = pth;
                    rec["auth_data.domain"] = dom;

                    trans->storage()->set(query, dataset);
                }
            }
        }
        if (httpCode >= 300 && httpCode < 400 && trans->is_set(weoFollowLinks) && relocCount < trans->relocation_count()) {
            // redirections
            relocCount++;
            string url = headers.find_first("Location");
            if (!url.empty()) {
                /// @todo process options: weoStayInDomain, weoStayInHost, weoStayInDir and request blocking
                headData.clear();
                data.clear();
                trans->request(url, boost::shared_dynamic_cast<i_response>(shared_from_this()));
            }
        }
    }
}

void HttpResponse::timeout( )
{
    processed = true;
    httpCode = 418; // HTTP: timeout
    realUrl = baseUrl;
    data.clear();
    headData.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn size_t HttpResponse::Receiver( void *ptr,
/// 	        size_t size, size_t nmemb, void *ourpointer )
///
/// @brief  Receives the document body. 
///
/// @param  ptr			 - The pointer to received data.
/// @param  size		 - The size of the data element. 
/// @param  nmemb		 - The number of the elements in the data.
/// @param  ourpointer	 - The pointer to the HttpResponse object, that receives data. 
///
/// @retval	amount of processed data 
////////////////////////////////////////////////////////////////////////////////////////////////////
size_t HttpResponse::Receiver( void *ptr, size_t size, size_t nmemb, void *ourpointer )
{
    if (ourpointer == NULL)
        return 0;

    HttpResponse* object = (HttpResponse*)ourpointer;
    size_t last = object->data.size();
    object->data.insert(object->data.end(), (unsigned char*)ptr, (unsigned char*)ptr + (size * nmemb));
    return (object->data.size() - last);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn size_t HttpResponse::HeadersRecv( void *ptr,
/// 	        size_t size, size_t nmemb, void *ourpointer )
///
/// @brief  Receives the document headers. 
///
/// @param  ptr			 - The pointer to received data.
/// @param  size		 - The size of the data element. 
/// @param  nmemb		 - The number of the elements in the data.
/// @param  ourpointer	 - The pointer to the HttpResponse object, that receives data. 
///
/// @retval	amount of processed data 
////////////////////////////////////////////////////////////////////////////////////////////////////
size_t HttpResponse::HeadersRecv( void *ptr, size_t size, size_t nmemb, void *ourpointer )
{
    if (ourpointer == NULL)
        return 0;

    HttpResponse* object = (HttpResponse*)ourpointer;
    size_t last = object->headData.size();
    object->headData.resize(last + (size * nmemb));
    memcpy(&(object->headData[last]), (unsigned char*)ptr, (size * nmemb));
    return (object->headData.size() - last);
}

} // namespace webEngine
