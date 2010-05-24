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
#ifndef __WEITRANSPORT_H__
#define __WEITRANSPORT_H__

#include <vector>
#include "weUrl.h"
#include "weBlob.h"
#include "weOptions.h"
#include "weiPlugin.h"
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

//using namespace boost;

namespace webEngine {
    // defined in weHelper.h
    extern int LockedIncrement(int *val);
    extern int LockedDecrement(int *val);

    class i_transport;

    class i_operation;
    class i_response;
    typedef boost::shared_ptr<i_operation> iweOperationPtr;
    typedef void (fnProcessResponse)(boost::shared_ptr<i_response> resp, void* context);


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  i_operation
    ///
    /// @brief  Abstraction for transport operation.
    ///
    /// This class implements basis for transport operations. It holds data to store the
    /// "request -> response" sequences.
    ///
    /// @author A. Abramov
    /// @date   16.06.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class i_operation : public boost::enable_shared_from_this<i_operation> 
    {
    public:
        i_operation() { usage_count = 0; context = NULL; processor = NULL; depth_level = 0; };
        virtual ~i_operation();

        virtual void AddChild(iweOperationPtr* chld);
        virtual void RemoveChild(iweOperationPtr* chld);
        //@{
        /// @brief  Access the BaseUrl property
        ///
        /// This property represents the base transport_url of the document, as it's given by user.
        transport_url &BaseUrl(void)            { return(baseUrl);	};
        void BaseUrl(const string &url) { baseUrl = url;    };
        void BaseUrl(const transport_url &url)  { baseUrl = url;    };
        //@}

        //@{
        /// @brief  Access the ID property
        ///
        /// This property represents the base transport_url of the document, as it's given by user.
        void ID(const string &id)   { identifier = id;  };
        const string & ID()         { return identifier;};
        //@}

        void* context;
        fnProcessResponse* processor;

        //@{
        /// @brief  Access the depth_level property.
        virtual int depth(void) { return depth_level; };
        virtual void depth(int lvl) { depth_level = lvl; };
        //@}

#ifndef __DOXYGEN__
    protected:
        int usage_count;
        transport_url baseUrl;
        i_operation* previous;
        vector<iweOperationPtr*> children;
        string identifier;
        int depth_level;
    private:
        i_operation(i_operation&) {};               ///< Avoid object coping
        i_operation& operator=(i_operation&) { return *this; };    ///< Avoid object coping
#endif //__DOXYGEN__
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @interface  i_request
    ///
    /// @brief  request to i_transport to receive data.
    ///
    /// @author A. Abramov
    /// @date   10.06.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class i_request : virtual public i_operation
    {
    public:
        i_request() {};
        i_request(string url) {};
        virtual ~i_request() {};

        //@{
        /// @brief  Access the ReqUrl property.
        ///
        /// @throw  WeError if given transport_url is malformed and can't be reconstructed from the WeHttpResponse
        virtual transport_url  &RequestUrl(void)  { return(baseUrl);   };
        virtual void RequestUrl(const string &ReqUrl, i_operation* resp = NULL) = 0;
        virtual void RequestUrl(const transport_url &ReqUrl, i_operation* resp = NULL) = 0;
        //@}
    };

    typedef boost::shared_ptr<i_request> i_request_ptr;
    typedef vector<i_request_ptr> request_list;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  i_response
    ///
    /// @brief  Response from the i_transport.
    ///
    /// @author A. Abramov
    /// @date   10.06.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class i_response : public i_operation
    {
    public:
        i_response() {};
        virtual ~i_response() {};

        //@{
        /// @brief  Access the RealUrl property
        ///
        /// This property represents the transport_url of the document, from which the resulting
        /// data was downloaded. It may differs from the base_url, if relocations
        /// presents.
        transport_url &RealUrl(void)            { return(realUrl);	};
        void RealUrl(const string &url) { realUrl = url;    };
        void RealUrl(const transport_url &url)  { realUrl = url;    };
        //@}

        //@{
        /// @brief  Access the RelocCount property
        ///
        /// This property represents the number of relocations to reach RealUrl from base_url
        int RelocCount(void)        { return(relocCount);   };
        void RelocCount(int count)  { relocCount = count;   };
        //@}

        //@{
        /// @brief  Access the Processed property
        ///
        /// Whatever the request already processed.
        bool Processed(void)        { return(processed);	};
        void Processed(bool proc)   { processed = proc;     };
        //@}

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Unstructured data for the request
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        blob& Data()  { return data;  };

        int DownloadTime(void)        { return(downTime);   };

        virtual void Process(i_transport* proc) = 0;

#ifndef __DOXYGEN__
    protected:
        transport_url realUrl;
        unsigned int relocCount;
        blob data;
        int downTime;
        bool processed;
#endif //__DOXYGEN__
    };

    typedef boost::shared_ptr<i_response> i_response_ptr;
    typedef vector<i_response_ptr> response_list;

    // forward declaration
    class transport_factory;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @interface  i_transport
    ///
    /// @brief  Transport for data receiving.
    ///
    /// @author A. Abramov
    /// @date   10.06.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class i_transport : public i_plugin
    {
    public:
        i_transport(engine_dispatcher* krnl, void* handle = NULL );
        virtual ~i_transport() {};

        virtual i_response_ptr request(string url, i_response_ptr resp = i_response_ptr((i_response*)NULL) ) = 0;
        virtual i_response_ptr request(i_request* req, i_response_ptr resp = i_response_ptr((i_response*)NULL) ) = 0;

        /// @brief  Process pending requests
        virtual const int process_requests(void) = 0;

        // @brief Initialize default values
        virtual void load_settings(i_options_provider *data_provider, string key = "") = 0;
        virtual bool is_set(const string& name) = 0;

        // i_plugin functions
        virtual i_plugin* get_interface(const string& ifName);

        //@{
        /// @brief  Access the base_url property
        ///
        /// This property represents the base transport_url for the analyzing and may be used for
        /// domain control and transport_url reconstructing. Will be overwrite by the request transport_url
        /// if not valid.
        /// @param bUrl - transport_url to set as base
        /// @throw  WeError if given transport_url is malformed
        const transport_url &base_url(void) const    { return(baseUrl);  };
        void base_url(const transport_url &bUrl);
        void base_url(const string &bUrl);
        //@}

        //@{
        /// @brief  Access the Int relocCount property
        ///
        /// This property represents the number of continuous relocations which causes
        /// relocation processing stop.
        const unsigned int &relocation_count(void) const     { return(relocCount);       };
        void relocation_count(const unsigned int& relCount)  { relocCount = relCount;    };
        //@}

        //@{
        /// @brief  Access the Int siteDepth property
        ///
        /// This property represents the number of continuous links which causes
        /// links processing stop.
        const unsigned int& site_depth(void) const   { return(siteDepth);    };
        void site_depth(const unsigned int& sDepth)  { siteDepth = sDepth;   };
        //@}

        virtual string& get_name() = 0;
        virtual bool is_own_protocol(string& proto) = 0;

#ifndef __DOXYGEN__
    protected:
        transport_url baseUrl;
        unsigned int relocCount;
        unsigned int siteDepth;
        response_list responces;
        i_options_provider* parent;

    private:
#endif //__DOXYGEN__
    };

} // namespace webEngine

webEngine::i_transport* create_named_transport(string name, webEngine::engine_dispatcher* krnl);

#endif // __WEITRANSPORT_H__
