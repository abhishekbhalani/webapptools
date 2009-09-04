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
#include <weHelper.h>
#include <weOptions.h>
#include <weiStorage.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace webEngine {

#ifndef __DOXYGEN__
static const wOption empty_option("_empty_");
#endif //__DOXYGEN__


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn std::string WeTask::ToXml( void )
///
/// @brief  Converts this object to an XML. 
///
/// This function realizes alternate serialization mechanism. It generates more compact and
/// simplified XML representation. This representation is used for internal data exchange
/// (for example to store data through iweStorage interface).
///
/// @retval This object as a std::string. 
////////////////////////////////////////////////////////////////////////////////////////////////////
RecordSet* iOptionsProvider::ToRS( const string& parentID/* = ""*/ )
{
    RecordSet* res = new RecordSet;
    Record* rec;
    wOptions::iterator it;
    wOptionVal optVal;
    string strData;

    for (it = options.begin(); it != options.end(); it++) {
        strData = it->first;
        optVal = it->second->Value();
        rec = new Record;
        rec->objectID = weObjTypeSysOption;
        rec->Option(weoName, strData);
        rec->Option(weoParentID, parentID);
        rec->Option(weoTypeID, it->second->Which());
        rec->Option(weoValue, optVal);
        res->push_back(*rec);
    }

    return res;
}

void iOptionsProvider::FromRS( RecordSet *rs )
{
    Record rec;
    size_t r;
    wOptionVal optVal;
    wOption opt;
    string strData;

    for (r = 0; r < rs->size(); r++)
    {
        rec = (*rs)[r];
        if (rec.objectID == weObjTypeSysOption) {
            opt = rec.Option(weoName);
            SAFE_GET_OPTION_VAL(opt, strData, "");
            // type doesn't matter here
            // opt = rec.Option(weoTypeID);
            opt = rec.Option(weoValue);
            optVal = opt.Value();
            Option(strData, optVal);
        }
    }
}

wOption& iOptionsProvider::Option( const string& name )
{
    wOptions::iterator it;

    LOG4CXX_TRACE(iLogger::GetLogger(), "iOptionsProvider::Option(" << name << ")");
    it = options.find(name);
    if (it != options.end())
    {
        return *(it->second);
    }
    return *((wOption*)&empty_option);
}

void iOptionsProvider::Option( const string& name, wOptionVal val )
{
    wOptions::iterator it;
    wOption* opt;

    LOG4CXX_TRACE(iLogger::GetLogger(), "iOptionsProvider::Option(" << name << ") set value=" << val);
    it = options.find(name);
    if (it != options.end())
    {
        opt = it->second;
        opt->SetValue(val);
    }
    else {
        opt = new wOption();
        opt->Name(name);
        opt->SetValue(val);
        options[name] = opt;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	bool WeTask::IsSet(const string& name )
///
/// @brief	Query if bool options 'name' is set to true.
///
/// @param	name - The option name.
///
/// @retval	true if set, false if not.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool iOptionsProvider::IsSet( const string& name )
{
    wOptions::iterator it;
    wOption* opt;
    bool retval = false;

    it = options.find(name);
    if (it != options.end())
    {
        opt = it->second;
        try
        {
            opt->GetValue(retval);
        }
        catch (...)
        {
            retval = false;
        }
    }
    LOG4CXX_TRACE(iLogger::GetLogger(), "iOptionsProvider::IsSet(" << name << ") value=" << retval);
    return retval;
}

void iOptionsProvider::CopyOptions( iOptionsProvider* cpy )
{
    wOptions::iterator it;

    for (it = cpy->options.begin(); it != cpy->options.end(); it++)
    {
        options[it->first] = it->second;
    }
}

StringList iOptionsProvider::OptionsList()
{
    StringList retval;

    retval.clear();
    wOptions::iterator it;

    for (it = options.begin(); it != options.end(); it++)
    {
        retval.push_back(it->first);
    }

    return retval;
}

} // namespace webEngine
