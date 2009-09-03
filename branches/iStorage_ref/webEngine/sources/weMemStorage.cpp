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

int MemStorage::Get(const string& objType, Record& filters, Record& respFilter, RecordSet& results)
{
    int retval = 0;
    string fID, stData, key;
    wOption opt;
    StringMap::iterator obj;
    StringList objIdxs;
    StringList filterNames;
    StringList reportNames;
    StringList nonflNames;
    StringList::iterator lstIt;
    size_t i, j;

    results.clear();

    obj = storage.find(objType);
    try {
        if (obj != storage.end()) {
            stData = obj->second;
            {
                istrstream is(stData.c_str(), stData.length());
                boost::archive::text_iarchive ia(is);
                // read class instance from archive
                ia >> objIdxs;
                // archive and stream closed when destructor are called
            }
            if (objIdxs.size() > 0) {
                // prepare filter list
                filterNames = filters.OptionsList();
                opt = filters.Option(weoID);
                SAFE_GET_OPTION_VAL(opt, fID, "");

                // prepare results list
                reportNames = respFilter.OptionsList();
                if (reportNames.size() == 0) {
                    obj = storage.find(objType + "_struct");
                    if (obj != storage.end()) {
                        stData = obj->second;
                        try
                        {
                            istrstream is(stData.c_str(), stData.length());
                            boost::archive::text_iarchive ia(is);
                            // read class instance from archive
                            ia >> reportNames;
                            // archive and stream closed when destructor are called
                        }
                        catch(std::exception& e)
                        {
                            LOG4CXX_WARN(iLogger::GetLogger(), "MemStorage::Get " << objType << "structure error: " << e.what());
                            retval = 0;
                            results.clear();
                            return retval;
                        }
                    }
                }

                // subtract sets
                nonflNames.clear();
                for (i = 0; i < reportNames.size(); i++)
                {
                    lstIt = find(filterNames.begin(), filterNames.end(), reportNames[i]);
                    if (lstIt == filterNames.end())
                    {
                        nonflNames.push_back(reportNames[i]);
                    }
                }

                for (i = 0; i < objIdxs.size(); i++) {
                    Record* objRes = new Record;
                    if (fID == "" || fID == "*" || fID == objIdxs[i])
                    {   // index in filter
                        bool skip = true;
                        for (j = 0; j < filterNames.size(); j++)
                        {
                            key = objIdxs[i] + "_" + filterNames[j];
                            obj = storage.find(key);
                            if (obj != storage.end())
                            {
                                wOption flOpt;
                                OptionFromString(opt, obj->second);
                                flOpt = filters.Option(filterNames[j]);
                                if (flOpt == opt) {
                                    skip = false;
                                    lstIt = find(reportNames.begin(), reportNames.end(), filterNames[j]);
                                    if (lstIt != reportNames.end())
                                    {
                                        objRes->Option(filterNames[j], opt.Value());
                                    }
                                }
                            } // option exist
                        } // and of attribute filters
                        if (! skip)
                        {
                            for (j = 0; j < nonflNames.size(); j++)
                            {
                                key = objIdxs[i] + "_" + nonflNames[j];
                                obj = storage.find(key);
                                if (obj != storage.end())
                                {
                                    OptionFromString(opt, obj->second);
                                    objRes->Option(nonflNames[j], opt.Value());
                                }
                            } // end of assignment non-filter options
                            results.push_back(*objRes);
                            retval++;
                        } // not skip this object
                        else {
                            delete objRes;
                        }
                    } // index in the list
                } // end of objects search
            }
        } // end of namespace search
    } // en od try
    catch(std::exception& e)
    {
        LOG4CXX_ERROR(iLogger::GetLogger(), "MemStorage::Get error: " << e.what());
        retval = 0;
        results.clear();
    }

    return retval;
}

int MemStorage::Set(const string& objType, Record& filters, Record& data)
{
    return 0;
}

int MemStorage::Set(const string& objType, RecordSet& data)
{
    return 0;
}

int MemStorage::Delete(const string& objType, Record& filters)
{
    return 0;
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

} // namespace webEngine
