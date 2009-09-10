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
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <fstream>
#include <strstream>
#include "storageFS.h"
#include "version.h"
#include "redis.xpm"

static char* tables[] = {weObjTypeTask,
                        weObjTypeSysOption,
                        weObjTypeDictionary,
                        weObjTypeAuthInfo,
                        weObjTypeScan,
                        weObjTypeScanData,
                        weObjTypeObject,
                        weObjTypeProfile,
                        NULL}; // close the list with NULL

RedisStorage::RedisStorage( Dispatch* krnl, void* handle /*= NULL*/ ) :
    iStorage(krnl, handle)
{
    pluginInfo.IfaceName = "Redis";
    pluginInfo.IfaceList.push_back("Redis");
    pluginInfo.PluginDesc = "Redis Database storage";
    pluginInfo.PluginId = "1FBAB8DCF440";
    pluginInfo.PluginIcon = WeXpmToStringList(redis_xpm, sizeof(redis_xpm) / sizeof(char*) );
    db_dir = "";
    lastId = 0;
    LOG4CXX_TRACE(logger, "RedisStorage plugin created");
}

RedisStorage::~RedisStorage(void)
{
    LOG4CXX_TRACE(logger, "RedisStorage plugin destroyed");
}

void* RedisStorage::GetInterface( const string& ifName )
{
    LOG4CXX_TRACE(logger, "RedisStorage::GetInterface " << ifName);
    if (iequals(ifName, "RedisStorage"))
    {
        LOG4CXX_DEBUG(logger, "RedisStorage::GetInterface found!");
        usageCount++;
        return (void*)(this);
    }
    return iStorage::GetInterface(ifName);
}

const string RedisStorage::GetSetupUI( void )
{
    /// @todo: change XRC to set the fields values
    return "";
}

void RedisStorage::ApplySettings( const string& xmlData )
{

}

bool RedisStorage::InitStorage(const string& params)
{
    bool retval = false;

    LOG4CXX_TRACE(logger, "FsStorage::InitStorage params = " << params);
    try {
        db_dir = params;
        fs::path dir_path(db_dir);
        if ( !fs::exists(dir_path) ) {
            fs::create_directory(db_dir.c_str());
        }
        else {
            if ( ! fs::is_directory(dir_path) ) {
                dir_path = dir_path.remove_filename();
                db_dir = dir_path.string();
            }
        }
        LOG4CXX_TRACE(logger, "FsStorage::InitStorage: base dir is " << db_dir);

        int i = 0;
        while (tables[i] != NULL)
        {
            // check storage presence
            dir_path = db_dir;
            dir_path /= tables[i];
            if ( !fs::exists(dir_path) ) {
                fs::create_directory(dir_path);
        }
        else {
            if ( ! fs::is_directory(dir_path) ) {
                string msg = dir_path.string() + "isn't a directory";
                    throw std::runtime_error(msg.c_str());
                }
            }
            LOG4CXX_TRACE(logger, "FsStorage::InitStorage: " << tables[i] << " storage dir is " << dir_path.string());
            i++;
        }
    }
    catch(std::exception& e) {
        LOG4CXX_ERROR(logger, "FsStorage::InitStorage: " << e.what());
        retval = false;
    }
    return retval;
}

void RedisStorage::Flush(const string& params /*= ""*/)
{
    // nothing to do. Just reimplement base class method
}

string RedisStorage::GenerateID(const string& objType /*= ""*/)
{
    string retval = "";
    fs::path dir_path(db_dir);

    dir_path /= "index";

    try {
        ifstream ifs(dir_path.string().c_str());
        ifs >> lastId;
    }
    catch ( const std::exception& )
    {
        LOG4CXX_WARN(logger, "FsStorage::GenerateID no index for " << objType);
        lastId = 1;
    }
    retval = lexical_cast<string>(++lastId);
    try {
        ofstream ofs(dir_path.string().c_str());
        ofs << lastId;
    }
    catch ( const std::exception & )
    {
        LOG4CXX_ERROR(logger, "FsStorage::GenerateID can't save index for " << objType);
    }

    return retval;
}

int RedisStorage::Get(Record& filter, Record& respFilter, RecordSet& results)
{
    return 0;
}

int RedisStorage::Set(Record& filter, Record& data)
{
    return 0;
}

int RedisStorage::Set(RecordSet& data)
{
    wOption opt;
    string fName;
    RecordSet* retlist = NULL;
    fs::path fp;
    Record sv;
    int retval = 0;
    size_t i;
    Record fl;

    LOG4CXX_TRACE(iLogger::GetLogger(), "RedisStorage::Set(RecordSet)");
    for (i = 0; i < data.size(); i++)
    {
        fl.Clear();
        fl.objectID = data[i].objectID;
        opt = data[i].Option(weoID);
        fl.Option(weoID, opt.Value());
        retval += Set(fl, data[i]);
    }
    return retval;
}

int RedisStorage::Delete(Record& filter)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "RedisStorage::Delete");
    return 0;
}

RecordSet* RedisStorage::Search(Record& filter, bool all/* = false*/)
{
    RecordSet* retlist = new RecordSet;
    RecordSet* datalist = NULL;
    fs::path dir_path(db_dir);
    Record* data;
    StringList objProps;
    wOption opt;
    string stValue, stData;
    size_t j;

    dir_path /= filter.objectID;

    fs::directory_iterator end_itr; // default construction yields past-the-end
    for ( fs::directory_iterator itr( dir_path ); itr != end_itr; ++itr )
    {
        if (starts_with(itr->path().filename(), "data"))
        {
            data = FileRead(itr->path().string());
            bool skip = false;
            if (!all)
            {
                objProps = filter.OptionsList();
                for (j = 0; j < objProps.size(); j++)
                {
                    opt = filter.Option(objProps[j]);
                    stValue = boost::lexical_cast<std::string>(opt.Value());

                    opt = data->Option(objProps[j]);
                    stData = boost::lexical_cast<std::string>(opt.Value());

                    if (stData != stValue)
                    {
                        skip = true;
                    }
                } // compare options
            } // take all files
            if (!skip)
            {
                data->objectID = filter.objectID;
                retlist->push_back(*data);
            }
            else {
                delete data;
            }

        }
    }

    return retlist;
}

StringList* RedisStorage::GetStruct(const string& nspace)
{
    fs::path fp(db_dir);
    StringList *structNames;

    fp /= nspace;
    fp /= "struct";
    structNames = new StringList;
    try
    {
        ifstream is(fp.string().c_str());
        boost::archive::text_iarchive ia(is);
        // read class instance from archive
        ia >> (*structNames);
        // archive and stream closed when destructor are called
    }
    catch(std::exception& e)
    {
        LOG4CXX_WARN(iLogger::GetLogger(), "FsStorage::GetStruct " << nspace << " structure error: " << e.what());
        delete structNames;
        structNames = NULL;
    }

    return structNames;
}

void RedisStorage::FixStruct(const string& nspace, Record& strt)
{
    StringMap::iterator obj;
    StringList::iterator lst;
    string stData;
    StringList* structNames;
    StringList flNames;
    size_t i;

    fs::path fp(db_dir);
    fp /= nspace;
    fp /= "struct";

    structNames = GetStruct(nspace);
    if (structNames == NULL)
    {
        structNames = new StringList;
    }
    flNames = strt.OptionsList();
    for (i = 0 ; i < flNames.size(); i++)
    {
        lst = find(structNames->begin(), structNames->end(), flNames[i]);
        if (lst == structNames->end())
        {
            structNames->push_back(flNames[i]);
        }
    }
    try
    {
        ofstream os(fp.string().c_str());
        boost::archive::text_oarchive oa(os);
        // read class instance from archive
        oa << (*structNames);
        // archive and stream closed when destructor are called
    }
    catch(std::exception& e)
    {
        LOG4CXX_WARN(iLogger::GetLogger(), "FsStorage::FixStruct save " << nspace << " structure error: " << e.what());
        return;
    }
}

