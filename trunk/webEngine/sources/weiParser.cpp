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

#include <time.h>
#include <boost/algorithm/string/predicate.hpp>
#include "weiParser.h"

using namespace boost::algorithm;

namespace webEngine {

#ifndef __DOXYGEN__
weCmpMode base_entity::compareMode = weCmpDefault;
#endif // __DOXYGEN__

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	string base_entity::attr(string )
///
/// @brief  Gets the attribute value.
///
/// @param  name - The attribute name.
///
/// @retval attribute value if the attribute exist,
///         empty string otherwise.
////////////////////////////////////////////////////////////////////////////////////////////////////
const string base_entity::attr(string name)
{
    AttrMap::iterator it;

    it = attributes.find(name);
    if (it != attributes.end())
    {
        return attributes.val(it);
    }
    return string("");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void base_entity::attr(string , string )
///
/// @brief  Sets the attribute.
///
/// @param  name - The attribute name.
/// @param  value - The attribute value.
////////////////////////////////////////////////////////////////////////////////////////////////////
void base_entity::attr(string name, string value)
{
    attributes[name] = value;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void base_entity::Child(string type)
///
/// @brief  Access to children by entity name.
/// @param  type - name of the child in the collection
/// @retval null if it no children of such type, the pointer to the WeHtmlEntity else.
////////////////////////////////////////////////////////////////////////////////////////////////////
base_entity_ptr base_entity::Child(string type)
{
    base_entity_ptr retval;
    entity_list::iterator  it;

    for(it = chldList.begin(); it != chldList.end(); it++) {
        if((*it)->Name() == type) {
            retval = (*it);
            break;
        }
    }
    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	base_entity* base_entity::Child(int idx)
///
/// @brief  Access to children by index.
/// @param  idx - index of the child in the collection
/// @retval null if it fails, the pointer to the WeHtmlEntity else.
////////////////////////////////////////////////////////////////////////////////////////////////////
base_entity_ptr base_entity::Child(int idx)
{
    if (idx < 0 || idx >= (int)chldList.size()) {
        return base_entity_ptr((base_entity*)NULL);
    }
    return chldList[idx];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void base_entity::GenerateId(void)
///
/// @brief  Generates an identifier.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma warning(push)
#pragma warning(disable : 4311)
#pragma warning(disable : 4996)
void base_entity::GenerateId(void)
{
    // Entity format (like GUID): {B46FF8A5-F2E9-4297-9F73-2894AABBB740}
/*    unsigned int first;
    unsigned int second;
    unsigned int third;
    char buff[40];

    m_entityId = "{";

    first = (unsigned int)this;
    while ((first & 0xF0000000) == 0)
    {
        first += ((unsigned int)rand() << 16);
    }

    second = (unsigned int)time(NULL);
    while ((second & 0xF0000000) == 0)
    {
        second += ((unsigned int)rand() << 16);
    }

    third = ((unsigned int)rand() << 8);
    third ^= first;
    third ^= second;
    sprintf(buff, "%08X-%04X-%04X-9798-A2%08X9F", first, (unsigned short)(second & 0xFFFF), (unsigned short)(second >> 16), third);

    m_entityId += buff;
    m_entityId += "}";
    srand(third);*/
}
#pragma warning(pop)

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	base_entity* base_entity::FindID(string id)
///
/// @brief  Finds the entity with given ID.
///
/// Searches through children for entity with given id. Returns point to itself, if given ID is the
/// ID of the called entity.
/// @param  id - The entity identifier.
/// @retval	null if it no objects found, pointer to the object else.
////////////////////////////////////////////////////////////////////////////////////////////////////
base_entity_ptr base_entity::FindID(string id)
{
    entity_list::iterator  chld;
    base_entity_ptr retval;

    if (id == m_entityId) {
        return shared_from_this();
    }
    for (chld = chldList.begin(); chld != chldList.end(); chld++) {
        retval = (*chld)->FindID(id);
        if (retval != NULL) {
            return retval;
        }
    }
    return base_entity_ptr((base_entity*)NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	entity_list& base_entity::FindTags(string tag)
///
/// @brief  Finds the entity with given tag name.
///
/// Searches through children for entity with given tag name. Returns the collection of entities
/// which tag names coincide with the given tag name.
/// @param  tag - The tag name to search.
/// @retval	empty list if it no objects found, list of the objects else.
////////////////////////////////////////////////////////////////////////////////////////////////////
entity_list base_entity::FindTags(string tag)
{
    entity_list retval;
    entity_list::iterator  chld;

    if (iequals(tag, entityName)) {
        retval.push_back(shared_from_this());
    }
    for (size_t i = 0; i < chldList.size(); i++) {
        entity_list chlds = chldList[i]->FindTags(tag);
        for (size_t j = 0; j < chlds.size(); j++) {
            retval.push_back(chlds[j]);
        }
        chlds.clear();
    }
    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void base_entity::ClearChildren( void )
///
/// @brief  Clears a children.
///
/// @author A. Abramov
/// @date   28.05.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
void base_entity::ClearChildren( void )
{
//     for (size_t i = 0; i < chldList.size(); i++) {
//         chldList[i].reset(); 
//     }
    chldList.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	WeDocument* base_entity::GetRootDocument( void )
///
/// @brief  Gets the root document.
/// @retval	null if no root document found, else the pointer to root document.
////////////////////////////////////////////////////////////////////////////////////////////////////
i_document* base_entity::GetRootDocument( void )
{
    if (entityName == "#document") {
        return dynamic_cast<i_document*>(this);
    }
    if (parent.expired()) {
        return (i_document*)NULL;
    }
    base_entity_ptr pl = parent.lock();
    i_document* res = pl->GetRootDocument();
    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn bool base_entity::IsParentTag( string tag )
///
/// @brief  Query if 'tag' is parent tag.
///
/// @param  tag	 - The tag name.
/// @retval	true if tag is in the parent tree, false if not.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool base_entity::IsParentTag( string tag )
{
    if (iequals(tag, entityName)) {
        return true;
    }
    if (parent.expired()) {
        return false;
    }
    base_entity_ptr pl = parent.lock();
    bool res = pl->IsParentTag(tag);
    return res;
}

void ClearEntityList( entity_list &lst )
{
//     for (size_t i = 0; i < lst.size(); i++) {
//         base_entity_ptr pt = lst[i];
//         lst[i].reset();
//     }
    lst.clear();    
}

i_parser::i_parser(engine_dispatcher* krnl, void* handle /*= NULL*/) :
    i_plugin(krnl, handle)
{
    pluginInfo.interface_name = "i_parser";
    pluginInfo.interface_list.push_back("i_parser");
    pluginInfo.plugin_desc = "Abstract parser interface";
    pluginInfo.plugin_id = "4E90373C65BA"; //{4C57EC48-C3BC-4d41-9D16-4E90373C65BA}
}

i_plugin* i_parser::get_interface( const string& ifName )
{
    LOG4CXX_TRACE(logger, "i_parser::get_interface " << ifName);
    if (iequals(ifName, "i_parser"))
    {
        LOG4CXX_DEBUG(logger, "i_parser::get_interface found!");
        usageCount++;
        return (this);
    }
    return i_plugin::get_interface(ifName);
}

html_parser::html_parser(engine_dispatcher* krnl, void* handle /*= NULL*/) :
    i_parser(krnl, handle)
{
    pluginInfo.interface_name = "html_parser";
    pluginInfo.interface_list.push_back("html_parser");
    pluginInfo.plugin_desc = "HTML parser";
    pluginInfo.plugin_id = "7467A5250777"; //{9E53EE57-7456-4b32-8574-7467A5250777}
}

i_plugin* html_parser::get_interface( const string& ifName )
{
    LOG4CXX_TRACE(logger, "html_parser::get_interface " << ifName);
    if (iequals(ifName, "html_parser"))
    {
        LOG4CXX_DEBUG(logger, "html_parser::get_interface found!");
        usageCount++;
        return (this);
    }
    return i_parser::get_interface(ifName);
}

i_document_ptr html_parser::parse(boost::shared_ptr<i_response> input)
{
    return i_document_ptr();
}

} // namespace webEngine
