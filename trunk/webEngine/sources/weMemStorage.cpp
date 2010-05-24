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
#include "weOptions.h"
#include "weTagScanner.h"
#include "weMemStorage.h"
#include "weiBase.h"

using namespace boost;

namespace webEngine {

static void OptionFromString(wOption& opt, const string& str)
{
    try
    {
        istrstream is(str.c_str(), str.length());
        boost::archive::text_iarchive ia(is);
        // read class instance from archive
        //ia >> opt;
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
        //oa << opt;
        // archive and stream closed when destructor are called
        str = string(os.rdbuf()->str(), os.rdbuf()->pcount());
    }
    catch (std::exception& e)
    {
        LOG4CXX_ERROR(iLogger::GetLogger(), "::OptionFromString error: " << e.what());
    }
}

mem_storage::mem_storage( engine_dispatcher* krnl, void* handle /*= NULL*/ ) :
    i_storage(krnl, handle)
{
    pluginInfo.interface_name = "mem_storage";
    pluginInfo.interface_list.push_back("mem_storage");
    pluginInfo.plugin_desc = "In-memory storage";
    pluginInfo.plugin_id = "D82B31419339";
    file_name = "";
    last_id = 1;
}

mem_storage::~mem_storage(void)
{
    flush(file_name);
}

i_plugin* mem_storage::get_interface( const string& ifName )
{
    if (iequals(ifName, "i_storage"))
    {
        usageCount++;
        return ((i_storage*)this);
    }
    if (iequals(ifName, "mem_storage"))
    {
        usageCount++;
        return ((mem_storage*)this);
    }
    return i_storage::get_interface(ifName);
}

int mem_storage::get(db_record& filter, db_record& respFilter, db_recordset& results)
{
    string fID, stData, key;
    wOption opt;
    StringMap::iterator obj;
    string_list* objIdxs;
    string_list filterNames;
    string_list reportNames;
    string_list nonflNames;
    string_list::iterator lstIt;
    size_t i, j;

    LOG4CXX_TRACE(iLogger::GetLogger(), "mem_storage::get");

    results.clear();

    if (filter.OptionSize() > 0)
    {
        objIdxs = search_db(filter);
    }
    else {
        objIdxs = get_namespace_idxs(filter.objectID);
    }
    if (objIdxs != NULL) {
        if (objIdxs->size() > 0) {
            // prepare filter list
            reportNames = respFilter.OptionsList();
            if (reportNames.size() == 0)
            {
                string_list* strct;
                strct = get_namespace_struct(filter);
                if (strct != NULL)
                {
                    reportNames = *strct;
                    delete strct;
                }
            }
            opt = filter.Option(weoID);
            SAFE_GET_OPTION_VAL(opt, fID, "");

            for (i = 0; i < objIdxs->size(); i++) {
                //db_record* objRes = new db_record;
                db_record objRes;
                objRes.objectID = filter.objectID;
                for (j = 0; j < reportNames.size(); j++)
                {
                    key = (*objIdxs)[i] + "_" + reportNames[j];
                    obj = storage_db.find(key);
                    if (obj != storage_db.end())
                    {
                        objRes.Option(reportNames[j], obj->second);
                    } 
                    else {
                        objRes.Option(reportNames[j], string(""));
                    }
                } // and of attribute filters
                results.push_back(objRes);
            } // end of objects search
        } // has objects
        delete objIdxs;
    } // received indexes

    return results.size();
}

int mem_storage::set(db_record& filter, db_record& data)
{
    wOption opt;
    string idx;
    string_list* lst = NULL;
    string_list* objIdxs = NULL;
    string_list objProps;
    size_t i, j;
    string stData, stValue;
    StringMap::iterator obj;
    string_list::iterator idIt;

    LOG4CXX_TRACE(iLogger::GetLogger(), "mem_storage::set(filter, db_record)");

    if (filter.OptionSize() > 0)
    {   // try to update
        lst = search_db(filter);
    }
    if (lst->size() == 0) {
        delete lst;
        lst = NULL;
    }


    if (filter.OptionSize() == 0 || lst == NULL)
    {   // insert data
        lst = new string_list;
        opt = data.Option(weoID);
        SAFE_GET_OPTION_VAL(opt, idx, "");
        if (idx == "")
        {   // get next index from list
            idx = generate_id(data.objectID);
        }
        lst->push_back(idx);
    }
    
    objProps = data.OptionsList();

    objIdxs = get_namespace_idxs(data.objectID);
    for (i = 0; i < lst->size(); i++)
    {
        for (j = 0; j < objProps.size(); j++)
        {
            wOption opt = data.Option(objProps[j]);
            stValue = boost::lexical_cast<std::string>(opt.Value());
            stData = (*lst)[i] + "_" + objProps[j];
            storage_db[stData] = stValue;
        }
        idIt = find(objIdxs->begin(), objIdxs->end(), (*lst)[i]);
        if (idIt == objIdxs->end())
        {
            objIdxs->push_back((*lst)[i]);
        }
    }
    fix_namespace_struct(data);

    i = lst->size();
    set_namespace_idxs(data.objectID, objIdxs);

    delete lst;
    delete objIdxs;
    return (int)i;
}

int mem_storage::set(db_recordset& data)
{
    int retval;
    size_t i;
    db_record filt;

    LOG4CXX_TRACE(iLogger::GetLogger(), "mem_storage::set(db_recordset)");
    filt.Clear();
    retval = 0;
    for (i = 0; i < data.size(); i++)
    {
        filt.objectID = data[i].objectID;
        retval += set(filt, data[i]);
    }

    return retval;
}

int mem_storage::del(db_record& filter)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "mem_storage::delete");

    string_list* lst = search_db(filter);
    string_list* objs = get_namespace_idxs(filter.objectID);
    int retval;
    size_t i;
    StringMap::iterator obj;
    string_list::iterator idxIt;

    retval = (int)lst->size();
    for (i = 0; i < lst->size(); i++)
    {
        obj = storage_db.find((*lst)[i]);
        if (obj != storage_db.end())
        {
            storage_db.erase(obj);
            /// @todo: find all properties and delete them to
        }
        idxIt = find(objs->begin(), objs->end(), (*lst)[i]);
        objs->erase(idxIt);
    }
    set_namespace_idxs(filter.objectID, objs);
    delete lst;
    delete objs;
    return retval;
}


void mem_storage::save_db( const string& fname )
{
    try {
        LOG4CXX_TRACE(iLogger::GetLogger(), "mem_storage::save_db");
        std::ofstream ofs(fname.c_str());
        // save data to archive
        {
            boost::archive::xml_oarchive oa(ofs);
            // write class instance to archive
            oa << BOOST_SERIALIZATION_NVP(last_id);
            oa << BOOST_SERIALIZATION_NVP(storage_db);
            // archive and stream closed when destructor are called
        }
    }
    catch(std::exception& e)
    {
        LOG4CXX_ERROR(iLogger::GetLogger(), "mem_storage::save_db error: " << e.what());
    }
    return;
}

void mem_storage::load_db( const string& fname )
{
    try{
        LOG4CXX_TRACE(iLogger::GetLogger(), "mem_storage::load_db");
        std::ifstream ifs(fname.c_str());

        // save data to archive
        {
            boost::archive::xml_iarchive ia(ifs);
            // write class instance to archive
            ia >> BOOST_SERIALIZATION_NVP(last_id);
            ia >> BOOST_SERIALIZATION_NVP(storage_db);
            // archive and stream closed when destructor are called
        }
    }
    catch(std::exception& e)
    {
        LOG4CXX_ERROR(iLogger::GetLogger(), "mem_storage::load_db error: " << e.what());
    }
    return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn bool mem_storage::init_storage( const string& params )
///
/// @brief  Initializes the storage_db. 
///
/// @param  params - Pathname for the storage_db file.
/// @retval	true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool mem_storage::init_storage( const string& params )
{
    file_name = params;
    load_db(file_name);
    return true;
}

void mem_storage::flush( const string& params /*= ""*/)
{
    if (params != "")
    {
        file_name = params;
    }
    if (file_name != "")
    {
        LOG4CXX_TRACE(iLogger::GetLogger(), "mem_storage::flush: filename not empty, save data");
        save_db(file_name);
    }
}

string_list* mem_storage::search_db(db_record& filter)
{
    string_list* retlist = new string_list;
    StringMap::iterator obj;
    string stData, stValue;
    string_list* objIdxs;
    string_list objProps;
    size_t i, j;

    objIdxs = get_namespace_idxs(filter.objectID);

    for (i = 0; i < objIdxs->size(); i++) {
        bool skip = false;
        objProps = filter.OptionsList();
        for (j = 0; j < objProps.size(); j++)
        {
            wOption opt = filter.Option(objProps[j]);
            stValue = boost::lexical_cast<std::string>(opt.Value());

            stData = (*objIdxs)[i] + "_" + objProps[j];
            obj = storage_db.find(stData);
            if (obj != storage_db.end())
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

string_list* mem_storage::get_namespace_idxs(const string& objType)
{
    string_list* objIdxs = new string_list;
    StringMap::iterator obj;
    string stData;

    obj = storage_db.find(objType);

    if (obj != storage_db.end()) {
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
            LOG4CXX_ERROR(iLogger::GetLogger(), "mem_storage::get_namespace_idxs namespace search error: " << e.what());
            objIdxs->clear();
        }
    }
    return objIdxs;
}

void mem_storage::set_namespace_idxs(const string& objType, string_list* lst)
{
    string stData;

    try {
        ostrstream os;
        boost::archive::text_oarchive oa(os);
        // read class instance from archive
        oa << (*lst);
        // archive and stream closed when destructor are called
        stData = string(os.str(), os.pcount());
    }
    catch(std::exception& e)
    {
        LOG4CXX_ERROR(iLogger::GetLogger(), "mem_storage::set_namespace_idxs namespace search error: " << e.what());
        return;
    }
    storage_db[objType] = stData;
}

void mem_storage::fix_namespace_struct(db_record& filter)
{
    StringMap::iterator obj;
    string_list::iterator lst;
    string stData;
    string_list structNames;
    string_list flNames;
    size_t i;


    obj = storage_db.find(filter.objectID + "_struct");
    if (obj != storage_db.end()) {
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
            LOG4CXX_WARN(iLogger::GetLogger(), "mem_storage::fix_namespace_struct " << filter.objectID << " structure error: " << e.what());
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
        stData = string(os.str(), os.pcount());
    }
    catch(std::exception& e)
    {
        LOG4CXX_WARN(iLogger::GetLogger(), "mem_storage::fix_namespace_struct save " << filter.objectID << " structure error: " << e.what());
        return;
    }
    storage_db[filter.objectID + "_struct"] = stData;
}

string_list* mem_storage::get_namespace_struct(db_record& filter)
{
    StringMap::iterator obj;
    string_list *structNames;
    string stData;

    structNames = new string_list;
    obj = storage_db.find(filter.objectID + "_struct");
    if (obj != storage_db.end()) {
        stData = obj->second;
        try
        {
            istrstream is(stData.c_str(), stData.length());
            boost::archive::text_iarchive ia(is);
            // read class instance from archive
            ia >> (*structNames);
            // archive and stream closed when destructor are called
        }
        catch(std::exception& e)
        {
            LOG4CXX_WARN(iLogger::GetLogger(), "mem_storage::get_namespace_struct " << filter.objectID << " structure error: " << e.what());
            delete structNames;
            structNames = NULL;
        }
    }
    return structNames;
}

} // namespace webEngine
