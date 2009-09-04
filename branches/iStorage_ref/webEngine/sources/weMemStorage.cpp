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
#include <fstream>
#include <strstream>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "weTagScanner.h"
#include "weMemStorage.h"
#include "weiBase.h"

namespace webEngine {

static void OptionFromString(wOption& opt, const string& str)
{
    try
    {
        istrstream is(str.c_str(), str.length());
        boost::archive::text_iarchive ia(is);
        // read class instance from archive
        ia >> opt;
        // archive and stream closed when destructor are called
    }
    catch (std::exception& e)
    {
        LOG4CXX_ERROR(iLogger::GetLogger(), "::OptionFromString error: " << e.what());
    }
}

static void OptionToString(wOption& opt, string& str)
{
    try
    {
        ostrstream os;
        boost::archive::text_oarchive oa(os);
        // read class instance from archive
        oa << opt;
        // archive and stream closed when destructor are called
        str = string(os.rdbuf()->str(), os.rdbuf()->pcount());
    }
    catch (std::exception& e)
    {
        LOG4CXX_ERROR(iLogger::GetLogger(), "::OptionFromString error: " << e.what());
    }
}

MemStorage::MemStorage( Dispatch* krnl, void* handle /*= NULL*/ ) :
    iStorage(krnl, handle)
{
    pluginInfo.IfaceName = "MemStorage";
    pluginInfo.IfaceList.push_back("MemStorage");
    pluginInfo.PluginDesc = "In-memory storage";
    pluginInfo.PluginId = "D82B31419339";
    fileName = "";
    lastId = 1;
}

MemStorage::~MemStorage(void)
{
    Flush(fileName);
}

void* MemStorage::GetInterface( const string& ifName )
{
    if (iequals(ifName, "iStorage"))
    {
        usageCount++;
        return (void*)((iStorage*)this);
    }
    if (iequals(ifName, "MemStorage"))
    {
        usageCount++;
        return (void*)((MemStorage*)this);
    }
    return iStorage::GetInterface(ifName);
}

int MemStorage::Get(Record& filter, Record& respFilter, RecordSet& results)
{
    int retval = 0;
    string fID, stData, key;
    wOption opt;
    StringMap::iterator obj;
    StringList* objIdxs;
    StringList filterNames;
    StringList reportNames;
    StringList nonflNames;
    StringList::iterator lstIt;
    size_t i, j;

    LOG4CXX_TRACE(iLogger::GetLogger(), "MemStorage::Get");

    results.clear();

    if (filter.OptionSize() > 0)
    {
        objIdxs = Search(filter);
    }
    else {
        objIdxs = GetNamespaceIdxs(filter.objectID);
    }
    if (objIdxs->size() > 0) {
        // prepare filter list
        reportNames = respFilter.OptionsList();
        opt = filter.Option(weoID);
        SAFE_GET_OPTION_VAL(opt, fID, "");

        for (i = 0; i < objIdxs->size(); i++) {
            Record* objRes = new Record;
            for (j = 0; j < reportNames.size(); j++)
            {
                key = (*objIdxs)[i] + "_" + reportNames[j];
                obj = storage.find(key);
                if (obj != storage.end())
                {
                    objRes->Option(reportNames[j], obj->second);
                } 
                else {
                    objRes->Option(reportNames[j], string(""));
                }
            } // and of attribute filters
            results.push_back(*objRes);
        } // end of objects search
    } // has objects

    return retval;
}

int MemStorage::Set(Record& filter, Record& data)
{
    wOption opt;
    string idx;
    StringList* lst;
    StringList* objIdxs;
    StringList objProps;
    size_t i, j;
    string stData, stValue;
    StringMap::iterator obj;
    StringList::iterator idIt;

    LOG4CXX_TRACE(iLogger::GetLogger(), "MemStorage::Set(filter, Record)");

    if (filter.OptionSize() > 0)
    {   // try to update
        lst = Search(filter);
    }
    else
    {   // insert data
        lst = new StringList;
        opt = data.Option(weoID);
        SAFE_GET_OPTION_VAL(opt, idx, "");
        if (idx == "")
        {   // get next index from list
            idx = GenerateID(data.objectID);
        }
        lst->push_back(idx);
    }
    
    objProps = data.OptionsList();

    objIdxs = GetNamespaceIdxs(data.objectID);
    for (i = 0; i < lst->size(); i++)
    {
        for (j = 0; j < objProps.size(); j++)
        {
            wOption opt = data.Option(objProps[j]);
            stValue = boost::lexical_cast<std::string>(opt.Value());
            stData = (*lst)[i] + "_" + objProps[j];
            storage[stData] = stValue;
        }
        idIt = find(objIdxs->begin(), objIdxs->end(), (*lst)[i]);
        if (idIt == objIdxs->end())
        {
            objIdxs->push_back((*lst)[i]);
        }
    }
    FixNamespaceStruct(data);

    i = lst->size();
    SetNamespaceIdxs(data.objectID, objIdxs);

    delete lst;
    delete objIdxs;
    return (int)i;
}

int MemStorage::Set(RecordSet& data)
{
    int retval;
    size_t i;
    Record filt;

    LOG4CXX_TRACE(iLogger::GetLogger(), "MemStorage::Set(RecordSet)");
    filt.Clear();
    retval = 0;
    for (i = 0; i < data.size(); i++)
    {
        filt.objectID = data[i].objectID;
        retval += Set(filt, data[i]);
    }

    return retval;
}

int MemStorage::Delete(Record& filter)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "MemStorage::Delete");

    StringList* lst = Search(filter);
    StringList* objs = GetNamespaceIdxs(filter.objectID);
    int retval;
    size_t i;
    StringMap::iterator obj;
    StringList::iterator idxIt;

    retval = (int)lst->size();
    for (i = 0; i < lst->size(); i++)
    {
        obj = storage.find((*lst)[i]);
        if (obj != storage.end())
        {
            storage.erase(obj);
            /// @todo: find all properties and delete them to
        }
        idxIt = find(objs->begin(), objs->end(), (*lst)[i]);
        objs->erase(idxIt);
    }
    SetNamespaceIdxs(filter.objectID, objs);
    delete lst;
    delete objs;
    return retval;
}


void MemStorage::Save( const string& fileName )
{
    try {
        LOG4CXX_TRACE(iLogger::GetLogger(), "MemStorage::Save");
        std::ofstream ofs(fileName.c_str());
        // save data to archive
        {
            boost::archive::xml_oarchive oa(ofs);
            // write class instance to archive
            oa << BOOST_SERIALIZATION_NVP(lastId);
            oa << BOOST_SERIALIZATION_NVP(storage);
            // archive and stream closed when destructor are called
        }
    }
    catch(std::exception& e)
    {
        LOG4CXX_ERROR(iLogger::GetLogger(), "MemStorage::Save error: " << e.what());
    }
    return;
}

void MemStorage::Load( const string& fileName )
{
    try{
        LOG4CXX_TRACE(iLogger::GetLogger(), "MemStorage::Load");
        std::ifstream ifs(fileName.c_str());

        // save data to archive
        {
            boost::archive::xml_iarchive ia(ifs);
            // write class instance to archive
            ia >> BOOST_SERIALIZATION_NVP(lastId);
            ia >> BOOST_SERIALIZATION_NVP(storage);
            // archive and stream closed when destructor are called
        }
    }
    catch(std::exception& e)
    {
        LOG4CXX_ERROR(iLogger::GetLogger(), "MemStorage::Load error: " << e.what());
    }
    return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn bool MemStorage::InitStorage( const string& params )
///
/// @brief  Initializes the storage. 
///
/// @param  params - Pathname for the storage file.
/// @retval	true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool MemStorage::InitStorage( const string& params )
{
    fileName = params;
    Load(fileName);
    return true;
}

void MemStorage::Flush( const string& params /*= ""*/)
{
    if (params != "")
    {
        fileName = params;
    }
    if (fileName != "")
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "MemStorage::Flush: filename not empty, save data");
        Save(fileName);
    }
}

StringList* MemStorage::Search(Record& filter)
{
    StringList* retlist = new StringList;
    StringMap::iterator obj;
    string stData, stValue;
    StringList* objIdxs;
    StringList objProps;
    size_t i, j;

    objIdxs = GetNamespaceIdxs(filter.objectID);

    for (i = 0; i < objIdxs->size(); i++) {
        bool skip = false;
        objProps = filter.OptionsList();
        for (j = 0; j < objProps.size(); j++)
        {
            wOption opt = filter.Option(objProps[j]);
            stValue = boost::lexical_cast<std::string>(opt.Value());

            stData = (*objIdxs)[i] + "_" + objProps[j];
            obj = storage.find(stData);
            if (obj != storage.end())
            {
                stData = obj->second;
            }
            else {
                stData = "";
            }
            if (stData != stValue)
            {
                skip = true;
            }
        }
        if (!skip)
        {
            retlist->push_back((*objIdxs)[i]);
        }
    }

    delete objIdxs;

    return retlist;
}

StringList* MemStorage::GetNamespaceIdxs(const string& objType)
{
    StringList* objIdxs = new StringList;
    StringMap::iterator obj;
    string stData;

    obj = storage.find(objType);

    if (obj != storage.end()) {
        stData = obj->second;
        try {
            istrstream is(stData.c_str(), stData.length());
            boost::archive::text_iarchive ia(is);
            // read class instance from archive
            ia >> (*objIdxs);
            // archive and stream closed when destructor are called
        }
        catch(std::exception& e)
        {
            LOG4CXX_ERROR(iLogger::GetLogger(), "MemStorage::GetNamespaceIdxs namespace search error: " << e.what());
            objIdxs->clear();
        }
    }
    return objIdxs;
}

void MemStorage::SetNamespaceIdxs(const string& objType, StringList* lst)
{
    string stData;

    try {
        ostrstream os;
        boost::archive::text_oarchive oa(os);
        // read class instance from archive
        oa << (*lst);
        // archive and stream closed when destructor are called
        stData = os.str();
    }
    catch(std::exception& e)
    {
        LOG4CXX_ERROR(iLogger::GetLogger(), "MemStorage::SetNamespaceIdxs namespace search error: " << e.what());
        return;
    }
    storage[objType] = stData;
}

void MemStorage::FixNamespaceStruct(Record& filter)
{
    StringMap::iterator obj;
    StringList::iterator lst;
    string stData;
    StringList structNames;
    StringList flNames;
    size_t i;


    obj = storage.find(filter.objectID + "_struct");
    if (obj != storage.end()) {
        stData = obj->second;
        try
        {
            istrstream is(stData.c_str(), stData.length());
            boost::archive::text_iarchive ia(is);
            // read class instance from archive
            ia >> structNames;
            // archive and stream closed when destructor are called
        }
        catch(std::exception& e)
        {
            LOG4CXX_WARN(iLogger::GetLogger(), "MemStorage::FixNamespaceStruct " << filter.objectID << " structure error: " << e.what());
            return;
        }
    }
    flNames = filter.OptionsList();
    for (i = 0 ; i < flNames.size(); i++)
    {
        lst = find(structNames.begin(), structNames.end(), flNames[i]);
        if (lst == structNames.end())
        {
            structNames.push_back(flNames[i]);
        }
    }
    try
    {
        ostrstream os;
        boost::archive::text_oarchive oa(os);
        // read class instance from archive
        oa << structNames;
        // archive and stream closed when destructor are called
        stData = os.str();
    }
    catch(std::exception& e)
    {
        LOG4CXX_WARN(iLogger::GetLogger(), "MemStorage::FixNamespaceStruct save " << filter.objectID << " structure error: " << e.what());
        return;
    }
    storage[filter.objectID + "_struct"] = stData;
}

} // namespace webEngine
