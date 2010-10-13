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
#ifndef __HTMLENTITY_H__
#define __HTMLENTITY_H__

#include "weiParser.h"
#include "weHTTP.h"

using namespace std;

namespace webEngine {

class html_document;
class html_entity;
typedef boost::shared_ptr<html_document> html_document_ptr;
typedef boost::shared_ptr<html_entity> html_entity_ptr;

/// @file   weHtmlEntity.h
/// @brief  HTML processing classes declarations

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	HTML entity.
///
/// This class implements basic functionality for the HTML elements inside WebEngine library.
/// It holds the collection of the attributes, child entities, and other necessary information.
/// Some functions such as comparisons are implemented inside this class too.
/// @author	A.Abramov
/// @date 26.05.2009.
////////////////////////////////////////////////////////////////////////////////////////////////////
class html_entity: virtual public base_entity {
public:
    html_entity(base_entity_ptr prnt = base_entity_ptr((base_entity*)NULL));
    html_entity(html_entity& entity);
    ~html_entity();

    virtual const string InnerText(void);
    virtual const string InnerHtml(void);
    virtual const string OuterText(void);

    virtual CmpResults* Diff(base_entity& cmp, weCmpMode mode);
    virtual weCmpState Compare(base_entity& cmp, weCmpMode mode);

    virtual scanner_token Parse(string tagName, tag_scanner& scanner, i_transport* processor = NULL);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  html_textnode
///
/// @brief  HTML entity to store plain text .
///
/// Special kind of html_entity - plain text inside other entities. It contains only one attribute
/// and no children. The attribute name is ignored in the user calls and stored as '\#text' in the
/// collection.
///
/// @author A. Abramov
/// @date   26.05.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class html_textnode : public html_entity {
public:
    html_textnode(base_entity_ptr prnt = base_entity_ptr((base_entity*)NULL));
    html_textnode(html_textnode& entity);
    ~html_textnode();

    virtual const string attr(string);
    virtual void attr(string, string);

    virtual const string InnerText(void);
    virtual const string OuterText(void);

    virtual CmpResults* Diff(base_entity& cmp, weCmpMode mode);
    virtual weCmpState Compare(base_entity& cmp, weCmpMode mode);

    virtual boost::shared_ptr<base_entity> Child(int idx) {
        return boost::shared_ptr<base_entity>((base_entity*)NULL);   ///< Placeholder to avoid children manipulations
    }
    virtual boost::shared_ptr<base_entity> Child(string type) {
        return boost::shared_ptr<base_entity>((base_entity*)NULL);   ///< Placeholder to avoid children manipulations
    }
    /// @brief Placeholder to avoid children manipulations
    /// @throw runtime_error with description
    virtual entity_list& Children() {
        //throw runtime_error("html_textnode: Children property is not accessible");
        return chldList;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  html_comment
///
/// @brief  HTML entity to store comments.
///
/// @author A. Abramov
/// @date   28.05.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class html_comment : public html_textnode {
public:
    html_comment(base_entity_ptr prnt = base_entity_ptr((base_entity*)NULL)) : html_textnode(prnt) {
        entityName = "#comment";
    };
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  html_cdata
///
/// @brief  HTML entity to store CDATA.
///
/// @author A. Abramov
/// @date   28.05.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class html_cdata : public html_textnode {
public:
    html_cdata(base_entity_ptr prnt = base_entity_ptr((base_entity*)NULL)) : html_textnode(prnt) {
        entityName = "#cdata";
    };
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  WePhpInclude
///
/// @brief  HTML entity to store included PHP script.
///
/// @author A. Abramov
/// @date   28.05.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class WePhpInclude : public html_textnode {
public:
    WePhpInclude(base_entity_ptr prnt = base_entity_ptr((base_entity*)NULL)) : html_textnode(prnt) {
        entityName = "#php";
    };
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  html_document
///
/// @brief  Web document. Entry point to parsed document.
///
/// Special kind of html_entity - whole web document. It may contains some other html_entity, such
/// as page layout elements, or (and) other html_document's. For example, if the root document contains
/// frames, the frames sources will be added to root document as the child documents.
/// @author	A. Abramov
/// @date	26.05.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma warning (disable: 4250)
class html_document : public i_document, public html_entity {
public:
    html_document(base_entity_ptr prnt = base_entity_ptr((base_entity*)NULL));
    html_document(html_document& entity);
    ~html_document();

    virtual const string InnerText(void);
    virtual const string OuterText(void);

    virtual CmpResults* Diff(base_entity& cmp, weCmpMode mode);
    virtual weCmpState Compare(base_entity& cmp, weCmpMode mode);

    /// @brief  Access the Data
    ///
    /// Data property represents downloaded data of the linked object.
    /// Images, script sources and other data may be stored here. @n
    virtual blob& Data(void);

    virtual bool ParseData(boost::shared_ptr<i_response> resp, i_transport* processor = NULL);
    virtual scanner_token Parse(string tagName, tag_scanner& scanner, i_transport* processor = NULL);

#ifndef __DOXYGEN__
protected:
    boost::shared_ptr<HttpResponse> response;
#endif //__DOXYGEN__
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  WeRefrenceObject
///
/// @brief  Linked object.
///
/// This is the base class for all entities, which have a linked data, i.e images, frames, scripts
/// etc. Linked data may be stored in blob if downloaded.
///
/// @author A. Abramov
/// @date   27.05.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class WeRefrenceObject : public html_document {
public:
    WeRefrenceObject(base_entity_ptr prnt = base_entity_ptr((base_entity*)NULL));
    WeRefrenceObject(WeRefrenceObject& entity);
    ~WeRefrenceObject();

    //@{
    /// @brief  Access the LocalLink property
    ///
    /// This property allows to use the local copy of the linked objects, and have
    /// following format: @n
    /// - @b empty - no linked data stored, only remote copy is available
    /// - @b data - linked object is stored inside the blob
    /// - @b \<EntityID\> - linked object stored in the html_entity. The ID of the
    ///   html_entity points the object that contains data.
    string LocalLink(void) const    {
        return(m_localLink);
    };
    void LocalLink(string local)    {
        m_localLink = local;
    };
    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn	bool const IsLocal(void) const
    ///
    /// @brief  Is the object's data local.
    /// @retval	true if object has locally linked data.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    bool const IsLocal(void) const  {
        return m_localLink.empty();
    };

    virtual scanner_token Parse(string tagName, tag_scanner& scanner, i_transport* processor = NULL);

protected:
#ifndef __DOXYGEN__
    string  m_localLink;
#endif //__DOXYGEN__
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  WeScript
///
/// @brief	Script object.
///
/// Base class to implement scripting engine operations.
///
/// @author A. Abramov
/// @date   02.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class WeScript : public WeRefrenceObject {
public:
    WeScript(base_entity_ptr prnt = base_entity_ptr((base_entity*)NULL));
    WeScript(WeScript& entity);
    ~WeScript();

    virtual scanner_token Parse(string tagName, tag_scanner& scanner, i_transport* processor = NULL);

    bool SetEngine(void *engine = NULL);
    void* Execute();

protected:
#ifndef __DOXYGEN__
    void*  scriptEngine;
#endif //__DOXYGEN__
};

} // namespace webEngine

#endif //__HTMLENTITY_H__
