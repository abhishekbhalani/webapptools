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

//using namespace boost;

namespace webEngine {

    class iTransport;

    class iOperation;
    class iResponse;
    typedef boost::shared_ptr<iOperation> iweOperationPtr;
    typedef void (fnProcessResponse)(iResponse *resp, void* context);


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  iOperation
    ///
    /// @brief  Abstraction for transport operation.
    ///
    /// This class implements basis for transport operations. It holds data to store the
    /// "request -> response" sequences.
    ///
    /// @author A. Abramov
    /// @date   16.06.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class iOperation
    {
    public:
        iOperation() { usageCount = 0; context = NULL; processor = NULL; depth_level = 0; };
        virtual ~iOperation();

        virtual iweOperationPtr& GetRef() { return (* new iweOperationPtr(this)); };

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
        int usageCount;
        transport_url baseUrl;
        iOperation* previous;
        vector<iweOperationPtr*> children;
        string identifier;
        int depth_level;
    private:
        iOperation(iOperation&) {};               ///< Avoid object coping
        iOperation& operator=(iOperation&) { return *this; };    ///< Avoid object coping
#endif //__DOXYGEN__
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @interface  iRequest
    ///
    /// @brief  Request to iTransport to receive data.
    ///
    /// @author A. Abramov
    /// @date   10.06.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class iRequest : virtual public iOperation
    {
    public:
        iRequest() {};
        iRequest(string url) {};
        virtual ~iRequest() {};

        //@{
        /// @brief  Access the ReqUrl property.
        ///
        /// @throw  WeError if given transport_url is malformed and can't be reconstructed from the WeHttpResponse
        virtual transport_url  &RequestUrl(void)  { return(baseUrl);   };
        virtual void RequestUrl(const string &ReqUrl, iOperation* resp = NULL) = 0;
        virtual void RequestUrl(const transport_url &ReqUrl, iOperation* resp = NULL) = 0;
        //@}
    };

    typedef vector<iRequest*> WeRequestList;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  iResponse
    ///
    /// @brief  Response from the iTransport.
    ///
    /// @author A. Abramov
    /// @date   10.06.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class iResponse : virtual public iOperation
    {
    public:
        iResponse() {};
        virtual ~iResponse() {};

        //@{
        /// @brief  Access the RealUrl property
        ///
        /// This property represents the transport_url of the document, from which the resulting
        /// data was downloaded. It may differs from the BaseUrl, if relocations
        /// presents.
        transport_url &RealUrl(void)            { return(realUrl);	};
        void RealUrl(const string &url) { realUrl = url;    };
        void RealUrl(const transport_url &url)  { realUrl = url;    };
        //@}

        //@{
        /// @brief  Access the RelocCount property
        ///
        /// This property represents the number of relocations to reach RealUrl from BaseUrl
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
        Blob& Data()  { return data;  };

        int DownloadTime(void)        { return(downTime);   };

        virtual void Process(iTransport* proc) = 0;

#ifndef __DOXYGEN__
    protected:
        transport_url realUrl;
        unsigned int relocCount;
        Blob data;
        int downTime;
        bool processed;
#endif //__DOXYGEN__
    };

    typedef vector<iResponse*> ResponseList;

    // forward declaration
    class TransportFactory;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @interface  iTransport
    ///
    /// @brief  Transport for data receiving.
    ///
    /// @author A. Abramov
    /// @date   10.06.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class iTransport : virtual public iOptionsProvider,
        public i_plugin
    {
    public:
        iTransport(engine_dispatcher* krnl, void* handle = NULL );
        virtual ~iTransport() {};

        virtual iResponse* Request(string url, iResponse *resp = NULL) = 0;
        virtual iResponse* Request(iRequest* req, iResponse *resp = NULL) = 0;

        /// @brief  Process pending requests
        virtual const int ProcessRequests(void) = 0;

        virtual wOption& Option(const string& name);
        virtual bool IsSet(const string& name);
        virtual void Option(const string& name, wOptionVal val);

        // i_plugin functions
        virtual void* GetInterface(const string& ifName);

        //@{
        /// @brief  Access the BaseUrl property
        ///
        /// This property represents the base transport_url for the analyzing and may be used for
        /// domain control and transport_url reconstructing. Will be overwrite by the request transport_url
        /// if not valid.
        /// @param bUrl - transport_url to set as base
        /// @throw  WeError if given transport_url is malformed
        const transport_url &BaseUrl(void) const    { return(baseUrl);  };
        void BaseUrl(const transport_url &bUrl);
        void BaseUrl(const string &bUrl);
        //@}

        //@{
        /// @brief  Access the Int relocCount property
        ///
        /// This property represents the number of continuous relocations which causes
        /// relocation processing stop.
        const unsigned int &RelocationCount(void) const     { return(relocCount);       };
        void RelocationCount(const unsigned int& relCount)  { relocCount = relCount;    };
        //@}

        //@{
        /// @brief  Access the Int siteDepth property
        ///
        /// This property represents the number of continuous links which causes
        /// links processing stop.
        const unsigned int& SiteDepth(void) const   { return(siteDepth);    };
        void SiteDepth(const unsigned int& sDepth)  { siteDepth = sDepth;   };
        //@}

        void SetOptionProvider(iOptionsProvider* prov)    { parent = prov; };
        virtual string& GetName() = 0;
        virtual bool IsOwnProtocol(string& proto) = 0;

#ifndef __DOXYGEN__
    protected:
        transport_url           baseUrl;
        unsigned int    relocCount;
        unsigned int    siteDepth;
        ResponseList  responces;
        iOptionsProvider* parent;

    private:
#endif //__DOXYGEN__
    };

} // namespace webEngine

webEngine::iTransport* WeCreateNamedTransport(string name, webEngine::engine_dispatcher* krnl);

#endif // __WEITRANSPORT_H__
