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
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "weTagScanner.h"
#include "weMemStorage.h"
#include "weiBase.h"

namespace webEngine {

MemStorage::MemStorage( Dispatch* krnl, void* handle /*= NULL*/ ) :
    iStorage(krnl, handle)
{
    pluginInfo.IfaceName = "MemStorage";
    pluginInfo.IfaceList.push_back("MemStorage");
    pluginInfo.PluginDesc = "In-memory storage";
    pluginInfo.PluginId = "D82B31419339";
    fileName = "";
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

int MemStorage::Query( const string& objType, const string& objId, Operation op, const string& xmlData )
{
    int retval = 0;
    if (iequals(objType, weObjTypeTask))
    {
        if (op == iStorage::remove)
        {
            if (objId == "*")
            {
                retval = tasks.size();
                tasks.clear();
            }
            else {
                StringMap::iterator tsk = tasks.find(objId);
                if (tsk != tasks.end())
                {
                    tasks.erase(tsk);
                    retval = 1;
                }
            }
        }
        else {
            tasks[objId] = xmlData;
            retval = 1;
        }
    }
    else {
        /// @todo Implement other types of data except task
        LOG4CXX_WARN(iLogger::GetLogger(), "MemStorage::Query: Not implemented: " << objType);
    }
    return retval;
}

int MemStorage::Report( const string& repType, const string& objId, const string& xmlData, string& result )
{
    int retval = 0;
    if (iequals(repType, weObjTypeTask))
    {
        StringMap::iterator tsk;
        result = "";
        if (objId == "*")
        {
            for (tsk = tasks.begin(); tsk != tasks.end(); tsk++)
            {
                result += tsk->second;
                retval++;
            }
        }
        else
        {
            if (tasks.find(objId) != tasks.end())
            {
                result += tasks[objId];
                retval++;
            }
        }
    }
    else {
        /// @todo Implement other types of data except task
        LOG4CXX_WARN(iLogger::GetLogger(), "MemStorage::Report: Not implemented: " << repType);
    }
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
            oa << BOOST_SERIALIZATION_NVP(tasks);
            oa << BOOST_SERIALIZATION_NVP(dicts);
            oa << BOOST_SERIALIZATION_NVP(auths);
            oa << BOOST_SERIALIZATION_NVP(sysopts);
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
            ia >> BOOST_SERIALIZATION_NVP(tasks);
            ia >> BOOST_SERIALIZATION_NVP(dicts);
            ia >> BOOST_SERIALIZATION_NVP(auths);
            ia >> BOOST_SERIALIZATION_NVP(sysopts);
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
