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
weCmpMode iEntity::compareMode = weCmpDefault;
#endif // __DOXYGEN__

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	string iEntity::Attr(string )
///
/// @brief  Gets the attribute value.
///
/// @param  name - The attribute name.
///
/// @retval attribute value if the attribute exist,
///         empty string otherwise.
////////////////////////////////////////////////////////////////////////////////////////////////////
const string iEntity::Attr(string name)
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
/// @fn	void iEntity::Attr(string , string )
///
/// @brief  Sets the attribute.
///
/// @param  name - The attribute name.
/// @param  value - The attribute value.
////////////////////////////////////////////////////////////////////////////////////////////////////
void iEntity::Attr(string name, string value)
{
    attributes[name] = value;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void iEntity::Child(string type)
///
/// @brief  Access to children by entity name.
/// @param  type - name of the child in the collection
/// @retval null if it no children of such type, the pointer to the WeHtmlEntity else.
////////////////////////////////////////////////////////////////////////////////////////////////////
iEntity* iEntity::Child(string type)
{
    iEntity* retval = NULL;
    EntityList::iterator  it;

    for(it = chldList.begin(); it != chldList.end(); it++) {
        if((*it)->Name() == type) {
            retval = (*it);
            break;
        }
    }
    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	iEntity* iEntity::Child(int idx)
///
/// @brief  Access to children by index.
/// @param  idx - index of the child in the collection
/// @retval null if it fails, the pointer to the WeHtmlEntity else.
////////////////////////////////////////////////////////////////////////////////////////////////////
iEntity* iEntity::Child(int idx)
{
    if (idx < 0 || idx >= (int)chldList.size()) {
        return(NULL);
    }
    return chldList[idx];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void iEntity::GenerateId(void)
///
/// @brief  Generates an identifier.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma warning(push)
#pragma warning(disable : 4311)
#pragma warning(disable : 4996)
void iEntity::GenerateId(void)
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
/// @fn	iEntity* iEntity::FindID(string id)
///
/// @brief  Finds the entity with given ID.
///
/// Searches through children for entity with given id. Returns point to itself, if given ID is the
/// ID of the called entity.
/// @param  id - The entity identifier.
/// @retval	null if it no objects found, pointer to the object else.
////////////////////////////////////////////////////////////////////////////////////////////////////
iEntity* iEntity::FindID(string id)
{
    EntityList::iterator  chld;
    iEntity* retval;

    if (id == m_entityId) {
        return this;
    }
    for (chld = chldList.begin(); chld != chldList.end(); chld++) {
        retval = (*chld)->FindID(id);
        if (retval != NULL) {
            return retval;
        }
    }
    return (NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	EntityList& iEntity::FindTags(string tag)
///
/// @brief  Finds the entity with given tag name.
///
/// Searches through children for entity with given tag name. Returns the collection of entities
/// which tag names coincide with the given tag name.
/// @param  tag - The tag name to search.
/// @retval	empty list if it no objects found, list of the objects else.
////////////////////////////////////////////////////////////////////////////////////////////////////
EntityList iEntity::FindTags(string tag)
{
    EntityList retval;
    EntityList::iterator  chld;

    if (iequals(tag, entityName)) {
        retval.push_back(this->add_ref());
    }
    for (size_t i = 0; i < chldList.size(); i++) {
        EntityList chlds = chldList[i]->FindTags(tag);
        for (size_t j = 0; j < chlds.size(); j++) {
            retval.push_back(chlds[j]);
        }
        chlds.clear();
    }
    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void iEntity::ClearChildren( void )
///
/// @brief  Clears a children.
///
/// @author A. Abramov
/// @date   28.05.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
void iEntity::ClearChildren( void )
{
    EntityList::iterator  chld;

    for (size_t i = 0; i < chldList.size(); i++) {
        chldList[i]->release(); 
    }
    chldList.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	WeDocument* iEntity::GetRootDocument( void )
///
/// @brief  Gets the root document.
/// @retval	null if no root document found, else the pointer to root document.
////////////////////////////////////////////////////////////////////////////////////////////////////
iDocument* iEntity::GetRootDocument( void )
{
    if (entityName == "#document") {
        return dynamic_cast<iDocument*>(this);
    }
    if (parent == NULL) {
        return (iDocument*)NULL;
    }
    return parent->GetRootDocument();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn bool iEntity::IsParentTag( string tag )
///
/// @brief  Query if 'tag' is parent tag.
///
/// @param  tag	 - The tag name.
/// @retval	true if tag is in the parent tree, false if not.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool iEntity::IsParentTag( string tag )
{
    if (iequals(tag, entityName)) {
        return true;
    }
    if (parent == NULL) {
        return false;
    }
    return parent->IsParentTag(tag);
}

void ClearEntityList( EntityList &lst )
{
    EntityList::iterator  chld;

    for (size_t i = 0; i < lst.size(); i++) {
        lst[i]->release();
    }
    lst.clear();    
}

} // namespace webEngine
