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
#ifndef __WEITRANSPORT_H__
#define __WEITRANSPORT_H__

#include <vector>
#include "weUrl.h"
#include "weBlob.h"
#include "weOptions.h"
#include <boost/shared_ptr.hpp>

using namespace boost;
class iweTransport;

class iweOperation;
typedef shared_ptr<iweOperation> iweOperationPtr;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  iweOperation
///
/// @brief  Abstraction for transport operation.
///
/// This class implements basis for transport operations. It holds data to store the
/// "request -> response" sequences.
///
/// @author A. Abramov
/// @date   16.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class iweOperation
{
public:
    iweOperation() { usageCount = 0; };
    virtual ~iweOperation();

    virtual iweOperationPtr& GetRef() { return (* new iweOperationPtr(this)); };

    virtual void AddChild(iweOperationPtr* chld);
    virtual void RemoveChild(iweOperationPtr* chld);
    //@{
    /// @brief  Access the BaseUrl property
    ///
    /// This property represents the base URL of the document, as it's given by user.
    WeURL &BaseUrl(void)            { return(baseUrl);	};
    void BaseUrl(const string &url) { baseUrl = url;    };
    void BaseUrl(const WeURL &url)  { baseUrl = url;    };
    //@}

    //@{
    /// @brief  Access the ID property
    ///
    /// This property represents the base URL of the document, as it's given by user.
    void ID(const string &id)   { identifier = id;  };
    const string & ID()         { return identifier;};
    //@}

#ifndef __DOXYGEN__
protected:
    int usageCount;
    WeURL baseUrl;
    iweOperation* previous;
    vector<iweOperationPtr*> children;
    string identifier;
private:
    iweOperation(iweOperation&) {};               ///< Avoid object coping
    iweOperation& operator=(iweOperation&) { return *this; };    ///< Avoid object coping
#endif //__DOXYGEN__
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @interface  iweRequest
///
/// @brief  Request to iweTransport to receive data.
///
/// @author A. Abramov
/// @date   10.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class iweRequest : virtual public iweOperation
{
public:
    iweRequest() {};
    iweRequest(string url) {};
    virtual ~iweRequest() {};

    //@{
    /// @brief  Access the ReqUrl property.
    ///
    /// @throw  WeError if given URL is malformed and can't be reconstructed from the WeHttpResponse
    virtual WeURL  &RequestUrl(void)  { return(baseUrl);   };
    virtual void RequestUrl(const string &ReqUrl, iweOperation* resp = NULL) = 0;
    virtual void RequestUrl(const WeURL &ReqUrl, iweOperation* resp = NULL) = 0;
    //@}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  iweResponse
///
/// @brief  Response from the iweTransport.
///
/// @author A. Abramov
/// @date   10.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class iweResponse : virtual public iweOperation
{
public:
    iweResponse() {};
    virtual ~iweResponse() {};

    //@{
    /// @brief  Access the RealUrl property
    ///
    /// This property represents the URL of the document, from which the resulting
    /// data was downloaded. It may differs from the BaseUrl, if relocations
    /// presents.
    WeURL &RealUrl(void)            { return(realUrl);	};
    void RealUrl(const string &url) { realUrl = url;    };
    void RealUrl(const WeURL &url)  { realUrl = url;    };
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
    WeBlob& Data()  { return data;  };

    virtual void Process(iweTransport* proc) = 0;

#ifndef __DOXYGEN__
protected:
    WeURL realUrl;
    unsigned int relocCount;
    WeBlob data;
    bool processed;
#endif //__DOXYGEN__
};

typedef vector<iweResponse*> WeResponseList;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @interface  iweTransport
///
/// @brief  Transport for data receiving.
///
/// @author A. Abramov
/// @date   10.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class iweTransport : virtual public iweOptionsProvider
{
public:
    iweTransport();
    virtual ~iweTransport() {};

    virtual iweResponse* Request(string url, iweResponse *resp = NULL) = 0;

    virtual WeOption& Option(const string& name);
    virtual bool IsSet(const string& name);
    virtual void Option(const string& name, WeOptionVal val);

    //@{
    /// @brief  Access the BaseUrl property
    ///
    /// This property represents the base URL for the analyzing and may be used for
    /// domain control and URL reconstructing. Will be overwrite by the request URL
    /// if not valid.
    /// @param bUrl - URL to set as base
    /// @throw  WeError if given URL is malformed
    const WeURL &BaseUrl(void) const    { return(baseUrl);  };
    void BaseUrl(const WeURL &bUrl);
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

    void SetOptionProvider(iweOptionsProvider* prov)    { parent = prov; };
    virtual string& GetName() = 0;
    virtual bool IsOwnProtocol(string& proto) = 0;

#ifndef __DOXYGEN__
protected:
    WeURL           baseUrl;
    unsigned int    relocCount;
    unsigned int    siteDepth;
    WeResponseList  responces;
    iweOptionsProvider* parent;

private:
    //@{
    /// @brief Avoid object coping
    iweTransport(iweTransport & source) {};
    iweTransport & operator=(iweTransport & source) { return *this; };
    //@}
#endif //__DOXYGEN__
};

iweTransport* WeCreateNamedTransport(string name);

#endif // __WEITRANSPORT_H__
