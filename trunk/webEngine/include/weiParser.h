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
#ifndef __WEIDOCUMENT_H__
#define __WEIDOCUMENT_H__

/// @file   weiParser.h
/// @brief  Processing subsystem declaration

#include <stdexcept>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "weStrings.h"
#include "weTagScanner.h"
#include "weBlob.h"
#include "weDiffLib.h"
#include "weiPlugin.h"
#include <html_tags.h>

using namespace std;

namespace webEngine {

class i_transport;
class i_response;
class base_entity;
class i_document;

typedef boost::shared_ptr<base_entity> base_entity_ptr;
typedef boost::weak_ptr<base_entity> base_entity_wptr;
typedef vector<base_entity_ptr> entity_list;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @interface  base_entity
///
/// @brief  Processing Entity interface.
///
/// @author A. Abramov
/// @date   08.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class base_entity : public boost::enable_shared_from_this<base_entity> {
public:
    base_entity() {};
    virtual ~base_entity() {
        ClearChildren();
    };

    virtual const string attr(string);
    virtual void attr(string, string);
    virtual AttrMap& attr_list() {
        return attributes;
    }
    void ClearAttr(void)    {
        attributes.clear();
    };        ///< Clears all attributes

    virtual base_entity_ptr Child(string type);
    virtual base_entity_ptr Child(int idx);
    virtual entity_list& Children() {
        return chldList;
    };  ///< Direct access to the children collection
    void ClearChildren(void);

    virtual const string InnerText(void) = 0;
    virtual const string OuterText(void) = 0;

    base_entity_ptr FindID(string id);
    entity_list FindTags(string tag);

    virtual scanner_token Parse(string tagName, tag_scanner& scanner, i_transport* processor = NULL) {
        return wstError;
    };

    virtual CmpResults* Diff(base_entity& cmp, weCmpMode mode = weCmpDefault) = 0;
    virtual weCmpState Compare(base_entity& cmp, weCmpMode mode = weCmpDefault) = 0;
    virtual bool operator==(base_entity& cmp) {
        return (Compare(cmp) == weCmpEqual);
    };

    i_document* GetRootDocument(void);

    //@{
    /// @brief Access the Parent
    const base_entity_wptr Parent(void) const	{
        return(parent);
    };
    void Parent(base_entity_wptr prnt)  {
        parent = prnt;
    };
    //}

    //@{
    /// @brief  Name property access
    ///
    /// Name property represents the entity TAG name. Some names defined for the special entities. These
    /// names are starts with '#' sign (for example @e \#text for inner text).
    const string &Name(void) const      {
        return(entityName);
    };
    void Name(const string &EntityName) {
        entityName = EntityName;
        htmlTag = find_tag_by_name(EntityName.c_str());
    };

    const HTML_TAG HtmlTag(void) const {
        return htmlTag;
    }

    //@}

    //@{
    /// @brief  CompareMode property access
    ///
    /// CompareMode property defines how the entities will be compared by default, i.e by the
    /// @b == operator, or if mode not set for the IsEqual function @n
    /// WeCompareDefault for default comparison means the strict
    /// comparison mode (WeCompatreStrict). But is not recommended
    /// to set the WeCompareDefault as the default mode. @n
    /// The CompareMode property of the WeHtmlEntity used to access
    /// this variable.
    static const weCmpMode &CompareMode(void)       {
        return(compareMode);
    };
    static void CompareMode(const weCmpMode &cMode) {
        compareMode = cMode;
    };
    //@}

    /// @brief  Gets the entity unique ID
    const string ID(void) const {
        return entity_id;
    };

protected:
    virtual void GenerateId(void);
    bool IsParentTag(string tag);

#ifndef __DOXYGEN__
protected:
    base_entity_wptr parent;
    AttrMap attributes;
    entity_list chldList;
    string entity_id;
    static weCmpMode compareMode;
    int startPos, endPos;
private:
    string entityName;
    HTML_TAG htmlTag;
#endif //__DOXYGEN__
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @interface	i_document
///
/// @brief  Parsing results document.
///
/// @author A. Abramov
/// @date   08.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class i_document : virtual public base_entity {
public:
    virtual bool ParseData(boost::shared_ptr<i_response> resp, i_transport* processor = NULL) = 0;
    virtual blob& Data(void) = 0;
};

typedef boost::shared_ptr<i_document> i_document_ptr;

void ClearEntityList(entity_list &lst);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @interface	i_parser
///
/// @brief  Abstract parser interface
///
/// @author A. Abramov
/// @date   31.05.2010
////////////////////////////////////////////////////////////////////////////////////////////////////
class i_parser : public i_plugin {
public:
    i_parser(engine_dispatcher* krnl, void* handle = NULL);
    virtual ~i_parser(void) {}

    // i_plugin functions
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual void* get_interface(const string& ifName)
    ///
    /// @brief  Gets an interface.
    ///
    /// Returns abstracted pointer to the requested interface, or NULL if the requested interface isn't
    /// provided by this object. The interface name depends on the plugin implementation.
    ///
    /// @param  ifName - Name of the interface.
    ///
    /// @retval	null if it fails, else the interface.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual i_plugin* get_interface(const string& ifName);

    virtual i_document_ptr parse(boost::shared_ptr<i_response> input) = 0;
};

class html_parser : public i_parser {
public:
    html_parser(engine_dispatcher* krnl, void* handle = NULL);
    virtual ~html_parser(void) {}

    // i_plugin functions
    virtual i_plugin* get_interface(const string& ifName);
    virtual void init(task* tsk);
    virtual void pause(task* tsk, bool paused = true) {}
    virtual void stop(task* tsk) {}

    // i_parser functions
    virtual i_document_ptr parse(boost::shared_ptr<i_response> input);

protected:
    int opt_ctype_method;
};

} // namespace webEngine

#endif //__WEIDOCUMENT_H__
