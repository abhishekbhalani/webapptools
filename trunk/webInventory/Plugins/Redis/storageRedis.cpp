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
#include <weTagScanner.h>
#include <strstream>
#ifdef WIN32
#include <winsock2.h>
#endif
#include "storageRedis.h"
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
    pluginInfo.IfaceName = "RedisStorage";
    pluginInfo.IfaceList.push_back("RedisStorage");
    pluginInfo.PluginDesc = "Redis Database storage";
    pluginInfo.PluginId = "5DB5DC11FF58"; // {62C6BD58-6A2C-4a9e-8208-5DB5DC11FF58}
    pluginInfo.PluginIcon = WeXpmToStringList(redis_xpm, sizeof(redis_xpm) / sizeof(char*) );
    lastId = 0;
    db_host = "localhost";
    db_port = 6379;
    db_index = 0;
    db_auth = "";
    db_cli = NULL;
#ifdef WIN32
    //----------------------
    // Initialize Winsock
    WSADATA wsaData;
    int iResult;
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != NO_ERROR) {
        LOG4CXX_FATAL(logger, "WSAStartup failed: " << iResult);
    }
#endif
    LOG4CXX_TRACE(logger, "RedisStorage plugin created");
}

RedisStorage::~RedisStorage(void)
{
    if (db_cli != NULL)
    {
        delete db_cli;
    }
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
    LOG4CXX_TRACE(logger, "RedisStorage::InitStorage");
    StrStream ss(params.c_str());
    TagScanner sc(ss);
    int token;
    bool inParsing = true;
    string dat;
    string name;

    while (inParsing)
    {
        token = sc.GetToken();
        switch(token)
        {
        case wstError:
            LOG4CXX_WARN(iLogger::GetLogger(), "RedisStorage::InitStorage parsing error");
            inParsing = false;
            break;
        case wstEof:
            LOG4CXX_TRACE(iLogger::GetLogger(), "RedisStorage::InitStorage - EOF");
            inParsing = false;
            break;
        case wstTagStart:
            name = sc.GetTagName();
            dat = "";
            break;
        case wstTagEnd:
            name = sc.GetTagName();
            if (name == "server")
            {
                db_host = dat;
            }
            if (name == "port")
            {
                db_port = lexical_cast<int>(dat);
            }
            if (name == "db_num")
            {
                db_index = lexical_cast<int>(dat);
            }
            if (name == "auth")
            {
                db_auth = dat;
            }
            break;
        case wstWord: 
        case wstSpace:
            dat += sc.GetValue();
            break;
        };
    }
    if (db_cli != NULL)
    {
        delete db_cli;
    }
    dat = "init";
    try
    {
        db_cli = new redis::client(db_host, db_port);
        if (db_auth != "")
        {
            dat = "authentication";
            db_cli->auth(db_auth);
        }
        dat = "select DB";
        db_cli->select(db_index);
    }
    catch(redis::redis_error& e)
    {
    	LOG4CXX_FATAL(iLogger::GetLogger(), "Redis DB connection error (" << dat << "): " << (string)e);
        retval = false;
        if (db_cli != NULL) {
            delete db_cli;
        }
        db_cli = NULL;
    }

    return retval;
}

void RedisStorage::Flush(const string& params /*= ""*/)
{
    if (db_cli)
    {
        try{
            db_cli->bgsave();
        }
        catch(redis::redis_error& e) {
            LOG4CXX_ERROR(iLogger::GetLogger(), "Redis DB save: " << (string)e);
        }
    }
}

string RedisStorage::GenerateID(const string& objType /*= ""*/)
{
    string retval = "";

    if (db_cli)
    {
        try{
            lastId = db_cli->incr("index");
            retval = lexical_cast<string>(lastId);
        }
        catch(redis::redis_error& e) {
            LOG4CXX_ERROR(iLogger::GetLogger(), "Redis DB get index: " << (string)e);
            retval = lexical_cast<string>(++lastId);
            try{
                db_cli->set("index", retval);
            }
            catch(redis::redis_error& e) {
                LOG4CXX_ERROR(iLogger::GetLogger(), "Redis DB set index: " << (string)e);
            }
        }
    }

    return retval;
}

int RedisStorage::Get(Record& filter, Record& respFilter, RecordSet& results)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "RedisStorage::Get");

    results.clear();

    if (db_cli == NULL)
    {
        LOG4CXX_FATAL(iLogger::GetLogger(), "RedisStorage::Get redis DB not initialized yet!");
        return 0;
    }

    StringList* objIdxs;
    StringList filterNames;
    StringList reportNames;
    StringList nonflNames;
    wOption opt;
    size_t i, j;
    string key, sdata;

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
        if (reportNames.size() == 0)
        {
            StringList* strct;
            strct = GetStruct(filter.objectID);
            if (strct != NULL)
            {
                reportNames = *strct;
                delete strct;
            }
        }
        for (i = 0; i < objIdxs->size(); i++) {
            Record* objRes = new Record;
            objRes->objectID = filter.objectID;
            for (j = 0; j < reportNames.size(); j++)
            {
                key = (*objIdxs)[i] + "_" + reportNames[j];
                try {
                    sdata = db_cli->get(key);
                }
                catch (redis::redis_error& e) {
                    LOG4CXX_WARN(iLogger::GetLogger(), "RedisStorage::Get error on get '" << key << "': " << (string)e);
                    sdata = "";
                }
                objRes->Option(reportNames[j], sdata);
            } // and of attribute filters
            results.push_back(*objRes);
        } // end of objects search
    } // has objects

    return (int)results.size();
}

int RedisStorage::Set(Record& filter, Record& data)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "RedisStorage::Set(Record, Record)");
    if (db_cli == NULL)
    {
        LOG4CXX_FATAL(iLogger::GetLogger(), "RedisStorage::Get redis DB not initialized yet!");
        return 0;
    }

    StringList* lst = NULL;
    wOption opt;
    string idx;
    StringList objProps;
    size_t i, j;
    string stData, stValue;

    if (filter.OptionSize() > 0)
    {   // try to update
        lst = Search(filter);
    }
    else {
        lst = new StringList;
    }
    if (lst->size() == 0)
    {   // insert data
        opt = data.Option(weoID);
        SAFE_GET_OPTION_VAL(opt, idx, "");
        if (idx == "")
        {   // get next index from list
            idx = GenerateID(data.objectID);
        }
        lst->push_back(idx);
    }

    objProps = data.OptionsList();

    for (i = 0; i < lst->size(); i++)
    {
        for (j = 0; j < objProps.size(); j++)
        {
            wOption opt = data.Option(objProps[j]);
            stValue = boost::lexical_cast<std::string>(opt.Value());
            stData = (*lst)[i] + "_" + objProps[j];
            try{
                db_cli->set(stData, stValue);
            }
            catch (redis::redis_error& e) {
                LOG4CXX_WARN(iLogger::GetLogger(), "RedisStorage::Set error at '" << stData << "' record: " << (string)e);
            }
        }
        try {
            stData = data.objectID + "_keys";
            db_cli->sadd(stData, (*lst)[i]);

        }
        catch (redis::redis_error& e) {
            LOG4CXX_WARN(iLogger::GetLogger(), "RedisStorage::Set error saving '" << (*lst)[i] <<
                "' index in '" << data.objectID << "'namespace: " << (string)e);
        }
    }
    FixStruct(data);
    i = lst->size();
    delete lst;
    return (int)i;
}

int RedisStorage::Set(RecordSet& data)
{
    wOption opt;
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

    if (db_cli == NULL)
    {
        LOG4CXX_FATAL(iLogger::GetLogger(), "RedisStorage::Delete redis DB not initialized yet!");
        return 0;
    }

    StringList* lst = Search(filter);
    int retval;
    size_t i, j;
    string kname, tmp;
    StringList ks;

    retval = (int)lst->size();
    for (i = 0; i < lst->size(); i++)
    {
        try{
            kname = (*lst)[i];
            tmp = kname + "_*";
            ks.clear();
            db_cli->keys(tmp, ks);
            for (j = 0; j < ks.size(); j++)
            {
                try{
                    db_cli->del(ks[j]);
                }
                catch (redis::redis_error& e) {
                    LOG4CXX_WARN(iLogger::GetLogger(), "RedisStorage::Delete deleting '" << ks[j] << "' error: " << (string)e);
                }
            }
            tmp = filter.objectID + "_keys";
            db_cli->srem(tmp, kname);
        }
        catch (redis::redis_error& e) {
            LOG4CXX_WARN(iLogger::GetLogger(), "RedisStorage::Delete error at '" << (*lst)[i] << "' record: " << (string)e);
        }
    }
    delete lst;
    return retval;
}

StringList* RedisStorage::Search(Record& filter, bool all/* = false*/)
{
    StringList* retlist = new StringList;
    StringMap::iterator obj;
    string stData, stValue, kname;
    StringList* objIdxs;
    StringList objProps;
    size_t i, j;

    objIdxs = GetNamespaceIdxs(filter.objectID);

    if (db_cli == NULL)
    {
        LOG4CXX_FATAL(iLogger::GetLogger(), "RedisStorage::Search redis DB not initialized yet!");
        return retlist;
    }

    for (i = 0; i < objIdxs->size(); i++) {
        bool skip = false;
        objProps = filter.OptionsList();
        for (j = 0; j < objProps.size(); j++)
        {
            wOption opt = filter.Option(objProps[j]);
            stValue = boost::lexical_cast<std::string>(opt.Value());

            kname = (*objIdxs)[i] + "_" + objProps[j];
            try
            {
                stData = db_cli->get(kname);
            }
            catch (redis::redis_error& e)
            {
                LOG4CXX_WARN(iLogger::GetLogger(), "RedisStorage::Search get '" << kname << "' error: " << (string)e);
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

StringList* RedisStorage::GetStruct(const string& nspace)
{
    StringList *structNames;
    redis::client::string_set   smembers;

    structNames = new StringList;
    if (db_cli != NULL)
    {
        try
        {
            string key = nspace + "_struct";
            db_cli->smembers(key, smembers);
            redis::client::string_set::iterator it;
            for (it = smembers.begin(); it != smembers.end(); it++)
            {
                structNames->push_back(*it);
            }
        }
        catch(redis::redis_error& e)
        {
            LOG4CXX_WARN(iLogger::GetLogger(), "RedisStorage::GetStruct '" << nspace << "' error: " << (string)e);
            delete structNames;
            structNames = NULL;
        }
    }
    else {
        LOG4CXX_FATAL(iLogger::GetLogger(), "RedisStorage::GetStruct redis DB not initialized yet!");
        delete structNames;
        structNames = NULL;
    }

    return structNames;
}

void RedisStorage::FixStruct(Record& strt)
{
    string key = strt.objectID + "_struct";
    StringList flNames;
    size_t i;

    if (db_cli != NULL)
    {
        flNames = strt.OptionsList();
        for (i = 0 ; i < flNames.size(); i++)
        {
            try
            {
                db_cli->sadd(key, flNames[i]);
            }
            catch (redis::redis_error& e)
            {
                LOG4CXX_WARN(iLogger::GetLogger(), "RedisStorage::FixStruct '" << strt.objectID << "' error: " << (string)e);
                break;
            }
        }
    }
    else {
        LOG4CXX_FATAL(iLogger::GetLogger(), "RedisStorage::FixStruct redis DB not initialized yet!");
    }
}

StringList* RedisStorage::GetNamespaceIdxs(const string& objType)
{
    StringList* objIdxs = new StringList;
    string key = objType + "_keys";
    redis::client::string_set   smembers;

    if (db_cli != NULL)
    {
        try
        {
            db_cli->smembers(key, smembers); //*objIdxs);
            redis::client::string_set::iterator it;
            for (it = smembers.begin(); it != smembers.end(); it++)
            {
                objIdxs->push_back(*it);
            }
        }
        catch(redis::redis_error& e)
        {
            LOG4CXX_WARN(iLogger::GetLogger(), "RedisStorage::GetNamespaceIdxs '" << objType << "' error: " << (string)e);
            objIdxs->clear();
        }
    }
    else {
        LOG4CXX_FATAL(iLogger::GetLogger(), "RedisStorage::GetNamespaceIdxs redis DB not initialized yet!");
        objIdxs->clear();
    }
    return objIdxs;
}
