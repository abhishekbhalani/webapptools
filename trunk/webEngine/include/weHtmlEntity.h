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

    class HtmlDocument;
    class HtmlEntity;

    /// @file   weHtmlEntity.h
    /// @brief  HTML processing classes declarations
    /// @example httpget.cpp
    /// @example document.cpp

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief	HTML entity.
    ///
    /// This class implements basic functionality for the HTML elements inside WebEngine library.
    /// It holds the collection of the attributes, child entities, and other necessary information.
    /// Some functions such as comparisons are implemented inside this class too.
    /// @author	A.Abramov
    /// @date 26.05.2009.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class HtmlEntity: virtual public iEntity
    {
    public:
        HtmlEntity(iEntity* prnt = NULL);
        HtmlEntity(HtmlEntity& entity);
        ~HtmlEntity();

        virtual const string &InnerText(void);
        virtual const string &OuterText(void);

        virtual CmpResults* Diff(iEntity& cmp, weCmpMode mode);
        virtual weCmpState Compare(iEntity& cmp, weCmpMode mode);

        virtual ScannerToken Parse(string tagName, TagScanner& scanner, iTransport* processor = NULL);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  WeInnerText
    ///
    /// @brief  HTML entity to store plain text .
    ///
    /// Special kind of HtmlEntity - plain text inside other entities. It contains only one attribute
    /// and no children. The attribute name is ignored in the user calls and stored as '\#text' in the
    /// collection.
    ///
    /// @author A. Abramov
    /// @date   26.05.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class WeInnerText : public HtmlEntity {
    public:
        WeInnerText(iEntity* prnt = NULL);
        WeInnerText(WeInnerText& entity);
        ~WeInnerText();

        virtual const string Attr(string);
        virtual void Attr(string, string);

        virtual const string &InnerText(void);
        virtual const string &OuterText(void);

        virtual CmpResults* Diff(iEntity& cmp, weCmpMode mode);
        virtual weCmpState Compare(iEntity& cmp, weCmpMode mode);

        virtual HtmlEntity* Child(int idx) {return (NULL);}       ///< Placeholder to avoid children manipulations
        virtual HtmlEntity* Child(string type) {return (NULL);}   ///< Placeholder to avoid children manipulations
        /// @brief Placeholder to avoid children manipulations
        /// @throw runtime_error with description
        virtual EntityList& Children() {
            throw runtime_error("WeInnerText: Children property is not accessible");
            return *((EntityList*)NULL);
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  WeHtmlComment
    ///
    /// @brief  HTML entity to store comments.
    ///
    /// @author A. Abramov
    /// @date   28.05.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class WeHtmlComment : public WeInnerText {
    public:
        WeHtmlComment(iEntity* prnt = NULL) : WeInnerText(prnt) { entityName = "#comment"; };
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  WeCData
    ///
    /// @brief  HTML entity to store CDATA.
    ///
    /// @author A. Abramov
    /// @date   28.05.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class WeCData : public WeInnerText {
    public:
        WeCData(iEntity* prnt = NULL) : WeInnerText(prnt) { entityName = "#cdata"; };
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  WePhpInclude
    ///
    /// @brief  HTML entity to store included PHP script.
    ///
    /// @author A. Abramov
    /// @date   28.05.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class WePhpInclude : public WeInnerText {
    public:
        WePhpInclude(iEntity* prnt = NULL) : WeInnerText(prnt) { entityName = "#php"; };
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  HtmlDocument
    ///
    /// @brief  Web document. Entry point to parsed document.
    ///
    /// Special kind of HtmlEntity - whole web document. It may contains some other HtmlEntity, such
    /// as page layout elements, or (and) other HtmlDocument's. For example, if the root document contains
    /// frames, the frames sources will be added to root document as the child documents.
    /// @author	A. Abramov
    /// @date	26.05.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma warning (disable: 4250)
    class HtmlDocument : public iDocument, public HtmlEntity
    {
    public:
        HtmlDocument(iEntity* prnt = NULL);
        HtmlDocument(HtmlDocument& entity);
        ~HtmlDocument();

        virtual const string &InnerText(void);
        virtual const string &OuterText(void);

        virtual CmpResults* Diff(iEntity& cmp, weCmpMode mode);
        virtual weCmpState Compare(iEntity& cmp, weCmpMode mode);

        /// @brief  Access the Data
        ///
        /// Data property represents downloaded data of the linked object.
        /// Images, script sources and other data may be stored here. @n
        virtual Blob& Data(void);

        virtual bool ParseData(iResponse* resp, iTransport* processor = NULL);

#ifndef __DOXYGEN__
    protected:
        HttpResponse* response;
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
    class WeRefrenceObject : public HtmlDocument
    {
    public:
        WeRefrenceObject(iEntity* prnt = NULL);
        WeRefrenceObject(WeRefrenceObject& entity);
        ~WeRefrenceObject();

        //@{
        /// @brief  Access the LocalLink property
        ///
        /// This property allows to use the local copy of the linked objects, and have
        /// following format: @n
        /// - @b empty - no linked data stored, only remote copy is available
        /// - @b data - linked object is stored inside the blob
        /// - @b \<EntityID\> - linked object stored in the HtmlEntity. The ID of the
        ///   HtmlEntity points the object that contains data.
        string LocalLink(void) const    { return(m_localLink);  };
        void LocalLink(string local)    { m_localLink = local;  };
        //@}

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @fn	bool const IsLocal(void) const
        ///
        /// @brief  Is the object's data local.
        /// @retval	true if object has locally linked data.
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        bool const IsLocal(void) const  { return m_localLink.empty(); };

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
    class WeScript : public WeRefrenceObject
    {
    public:
        WeScript(iEntity* prnt = NULL);
        WeScript(WeScript& entity);
        ~WeScript();

        virtual ScannerToken Parse(string tagName, TagScanner& scanner, HttpTransport* processor = NULL);

        bool SetEngine(void *engine = NULL);
        void* Execute();

    protected:
#ifndef __DOXYGEN__
        void*  scriptEngine;
#endif //__DOXYGEN__
    };

} // namespace webEngine

#endif //__HTMLENTITY_H__
