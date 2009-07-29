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

WeMemStorage::WeMemStorage( WeDispatch* krnl, void* handle /*= NULL*/ ) :
    iweStorage(krnl, handle)
{
    pluginInfo.IfaceName = "WeMemStorage";
    pluginInfo.IfaceList.push_back("WeMemStorage");
    pluginInfo.PluginDesc = "In-memory storage";
    pluginInfo.PluginId = "D82B31419339";
    fileName = "";
}

WeMemStorage::~WeMemStorage(void)
{
    Flush(fileName);
}

void* WeMemStorage::GetInterface( const string& ifName )
{
    if (iequals(ifName, "iweStorage"))
    {
        usageCount++;
        return (void*)((iweStorage*)this);
    }
    if (iequals(ifName, "WeMemStorage"))
    {
        usageCount++;
        return (void*)((WeMemStorage*)this);
    }
    return iweStorage::GetInterface(ifName);
}

int WeMemStorage::Query( const string& objType, const string& objId, Operation op, const string& xmlData )
{
    int retval = 0;
    if (iequals(objType, weObjTypeTask))
    {
        if (op == iweStorage::remove)
        {
            if (objId == "*")
            {
                retval = tasks.size();
                tasks.clear();
            }
            else {
                WeStringMap::iterator tsk = tasks.find(objId);
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
        LOG4CXX_WARN(WeLogger::GetLogger(), "WeMemStorage::Query: Not implemented: " << objType);
    }
    return retval;
}

int WeMemStorage::Report( const string& repType, const string& objId, const string& xmlData, string& result )
{
    int retval = 0;
    if (iequals(repType, weObjTypeTask))
    {
        WeStringMap::iterator tsk;
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
        LOG4CXX_WARN(WeLogger::GetLogger(), "WeMemStorage::Report: Not implemented: " << repType);
    }
    return retval;
}

void WeMemStorage::Save( const string& fileName )
{
    try {
        LOG4CXX_TRACE(WeLogger::GetLogger(), "WeMemStorage::Save");
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
        LOG4CXX_ERROR(WeLogger::GetLogger(), "WeMemStorage::Save error: " << e.what());
    }
    return;
}

void WeMemStorage::Load( const string& fileName )
{
    try{
        LOG4CXX_TRACE(WeLogger::GetLogger(), "WeMemStorage::Load");
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
        LOG4CXX_ERROR(WeLogger::GetLogger(), "WeMemStorage::Load error: " << e.what());
    }
    return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn bool WeMemStorage::InitStorage( const string& params )
///
/// @brief  Initializes the storage. 
///
/// @param  params - Pathname for the storage file.
/// @retval	true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool WeMemStorage::InitStorage( const string& params )
{
    fileName = params;
    Load(fileName);
    return true;
}

void WeMemStorage::Flush( const string& params /*= ""*/)
{
    if (params != "")
    {
        fileName = params;
    }
    if (fileName != "")
    {
        LOG4CXX_TRACE(WeLogger::GetLogger(), "WeMemStorage::Flush: filename not empty, save data");
        Save(fileName);
    }
}
