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
#include <boost/tokenizer.hpp>
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

log4cxx::LoggerPtr scan_logger;

typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

redis_storage::redis_storage( engine_dispatcher* krnl, void* handle /*= NULL*/ ) :
    i_storage(krnl, handle)
{
    {
        stringstream ost;
        ost << VERSION_MAJOR << "." << VERSION_MINOR << "." <<
            VERSION_BUILDNO << "." << VERSION_EXTEND << 
#ifdef _DEBUG
            " build at " << VERSION_DATE <<
            " " << VERSION_TIME << " " << VERSION_SVN <<
#endif
#ifdef WIN32
            " (Windows)" <<
#else
            " (Linux)" <<
#endif
            " ";
        plugin_version = ost.str();
    }

    pluginInfo.interface_name = "redis_storage";
    pluginInfo.interface_list.push_back("redis_storage");
    pluginInfo.plugin_desc = "Redis Database storage. Version: " + plugin_version;
    pluginInfo.plugin_id = "5DB5DC11FF58"; // {62C6BD58-6A2C-4a9e-8208-5DB5DC11FF58}
    pluginInfo.plugin_icon = WeXpmToStringList(redis_xpm, sizeof(redis_xpm) / sizeof(char*) );
    last_id = 0;
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
#ifndef NDEBUG
    scan_logger = logger;
#endif
    LOG4CXX_TRACE(logger, "redis_storage plugin created");
}

redis_storage::~redis_storage(void)
{
    if (db_cli != NULL)
    {
        delete db_cli;
    }
    LOG4CXX_TRACE(logger, "redis_storage plugin destroyed");
}

void* redis_storage::get_interface( const string& ifName )
{
    LOG4CXX_TRACE(logger, "redis_storage::get_interface " << ifName);
    if (boost::iequals(ifName, "redis_storage"))
    {
        LOG4CXX_DEBUG(logger, "redis_storage::get_interface found!");
        usageCount++;
        return (void*)(this);
    }
    return i_storage::get_interface(ifName);
}

const string redis_storage::get_setup_ui( void )
{
    /// @todo: change XRC to set the fields values
    return "";
}

void redis_storage::apply_settings( const string& xmlData )
{

}

// parameter is: host=address;port=number;dbnum=num;auth=string
// each parameter may be skipped to use default values
bool redis_storage::init_storage(const string& params)
{
    bool retval = false;
    LOG4CXX_DEBUG(logger, "redis_storage::init_storage");
    boost::char_separator<char> sep_params(";");
    tokenizer tok_param(params, sep_params);
    int pos;
    string dat;
    string name;

    for (tokenizer::iterator tok_iter = tok_param.begin(); tok_iter != tok_param.end(); ++tok_iter)
    {
        dat = *tok_iter;
        pos = dat.find('=');
        if (pos != string::npos) {
            name = dat.substr(0, pos);
            dat = dat.substr(pos + 1);
            LOG4CXX_TRACE(logger, "redis_storage::init_storage found parameter: " << name << "; value: " << dat);
            if (name == "host")
            {
                db_host = dat;
            }
            if (name == "port")
            {
                db_port = boost::lexical_cast<int>(dat);
            }
            if (name == "dbnum")
            {
                db_index = boost::lexical_cast<int>(dat);
            }
            if (name == "auth")
            {
                db_auth = dat;
            }
        }
    }

    boost::lock_guard<boost::mutex> lock(db_lock);

    if (db_cli != NULL)
    {
        delete db_cli;
    }
    dat = "connection creation";
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

void redis_storage::flush(const string& params /*= ""*/)
{
    if (db_cli)
    {
        boost::lock_guard<boost::mutex> lock(db_lock);
        try{
            db_cli->bgsave();
        }
        catch(redis::redis_error& e) {
            LOG4CXX_ERROR(iLogger::GetLogger(), "Redis DB save: " << (string)e);
        }
    }
}

string redis_storage::generate_id(const string& objType /*= ""*/)
{
    string retval = "";

    if (db_cli)
    {
        boost::lock_guard<boost::mutex> lock(db_lock);
        try{
            last_id = db_cli->incr("index");
            retval = boost::lexical_cast<string>(last_id);
        }
        catch(redis::redis_error& e) {
            LOG4CXX_ERROR(iLogger::GetLogger(), "Redis DB get index: " << (string)e);
            retval = boost::lexical_cast<string>(++last_id);
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

int redis_storage::get(db_record& filter, db_record& respFilter, db_recordset& results)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "redis_storage::get");

    results.clear();

    if (db_cli == NULL)
    {
        LOG4CXX_FATAL(iLogger::GetLogger(), "redis_storage::get redis DB not initialized yet!");
        return 0;
    }

    boost::lock_guard<boost::mutex> lock(db_lock);

    string_list* objIdxs;
    string_list filterNames;
    string_list reportNames;
    string_list nonflNames;
    wOption opt;
    size_t i, j;
    string key, sdata;

    if (filter.OptionSize() > 0)
    {
        objIdxs = search_db(filter);
    }
    else {
        objIdxs = get_namespace_idxs(filter.objectID);
    }
    if (objIdxs->size() > 0) {
        // prepare filter list
        reportNames = respFilter.OptionsList();
        if (reportNames.size() == 0)
        {
            string_list* strct;
            strct = get_struct(filter.objectID);
            if (strct != NULL)
            {
                reportNames = *strct;
                delete strct;
            }
        }
        for (i = 0; i < objIdxs->size(); i++) {
            db_record* objRes = new db_record;
            objRes->objectID = filter.objectID;
            for (j = 0; j < reportNames.size(); j++)
            {
                key = (*objIdxs)[i] + "_" + reportNames[j];
                try {
                    sdata = db_cli->get(key);
                }
                catch (redis::redis_error& e) {
                    LOG4CXX_WARN(iLogger::GetLogger(), "redis_storage::get error on get '" << key << "': " << (string)e);
                    sdata = "";
                }
                objRes->Option(reportNames[j], sdata);
            } // and of attribute filters
            results.push_back(*objRes);
        } // end of objects search
    } // has objects

    return (int)results.size();
}

int redis_storage::set(db_record& filter, db_record& data)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "redis_storage::set(db_record, db_record)");
    if (db_cli == NULL)
    {
        LOG4CXX_FATAL(iLogger::GetLogger(), "redis_storage::get redis DB not initialized yet!");
        return 0;
    }

    boost::lock_guard<boost::mutex> lock(db_lock);

    string_list* lst = NULL;
    wOption opt;
    string idx;
    string_list objProps;
    size_t i, j;
    string stData, stValue;

    if (filter.OptionSize() > 0)
    {   // try to update
        lst = search_db(filter);
    }
    else {
        lst = new string_list;
    }
    if (lst->size() == 0)
    {   // insert data
        opt = data.Option(weoID);
        SAFE_GET_OPTION_VAL(opt, idx, "");
        if (idx == "")
        {   // get next index from list
            idx = generate_id(data.objectID);
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
                LOG4CXX_WARN(iLogger::GetLogger(), "redis_storage::set error at '" << stData << "' db_record: " << (string)e);
            }
        }
        try {
            stData = data.objectID + "_keys";
            db_cli->sadd(stData, (*lst)[i]);

        }
        catch (redis::redis_error& e) {
            LOG4CXX_WARN(iLogger::GetLogger(), "redis_storage::set error saving '" << (*lst)[i] <<
                "' index in '" << data.objectID << "'namespace: " << (string)e);
        }
    }
    fix_struct(data);
    i = lst->size();
    delete lst;
    return (int)i;
}

int redis_storage::set(db_recordset& data)
{
    wOption opt;
    int retval = 0;
    size_t i;
    db_record fl;

    LOG4CXX_TRACE(iLogger::GetLogger(), "redis_storage::set(db_recordset)");
    for (i = 0; i < data.size(); i++)
    {
        fl.Clear();
        fl.objectID = data[i].objectID;
        opt = data[i].Option(weoID);
        fl.Option(weoID, opt.Value());
        retval += set(fl, data[i]);
    }
    return retval;
}

int redis_storage::del(db_record& filter)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "redis_storage::del");

    if (db_cli == NULL)
    {
        LOG4CXX_FATAL(iLogger::GetLogger(), "redis_storage::del redis DB not initialized yet!");
        return 0;
    }

    boost::lock_guard<boost::mutex> lock(db_lock);

    string_list* lst = search_db(filter);
    int retval;
    size_t i, j;
    string kname, tmp;
    string_list ks;

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
                    LOG4CXX_WARN(iLogger::GetLogger(), "redis_storage::del deleting '" << ks[j] << "' error: " << (string)e);
                }
            }
            tmp = filter.objectID + "_keys";
            db_cli->srem(tmp, kname);
        }
        catch (redis::redis_error& e) {
            LOG4CXX_WARN(iLogger::GetLogger(), "redis_storage::del error at '" << (*lst)[i] << "' db_record: " << (string)e);
        }
    }
    delete lst;
    return retval;
}

string_list* redis_storage::search_db(db_record& filter, bool all/* = false*/)
{
    string_list* retlist = new string_list;
    StringMap::iterator obj;
    string stData, stValue, kname;
    string_list* objIdxs;
    string_list objProps;
    size_t i, j;

    objIdxs = get_namespace_idxs(filter.objectID);

    if (db_cli == NULL)
    {
        LOG4CXX_FATAL(iLogger::GetLogger(), "redis_storage::search_db redis DB not initialized yet!");
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
                LOG4CXX_WARN(iLogger::GetLogger(), "redis_storage::search_db get '" << kname << "' error: " << (string)e);
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

string_list* redis_storage::get_struct(const string& nspace)
{
    string_list *structNames;
    redis::client::string_set   smembers;

    structNames = new string_list;
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
            LOG4CXX_WARN(iLogger::GetLogger(), "redis_storage::get_struct '" << nspace << "' error: " << (string)e);
            delete structNames;
            structNames = NULL;
        }
    }
    else {
        LOG4CXX_FATAL(iLogger::GetLogger(), "redis_storage::get_struct redis DB not initialized yet!");
        delete structNames;
        structNames = NULL;
    }

    return structNames;
}

void redis_storage::fix_struct(db_record& strt)
{
    string key = strt.objectID + "_struct";
    string_list flNames;
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
                LOG4CXX_WARN(iLogger::GetLogger(), "redis_storage::fix_struct '" << strt.objectID << "' error: " << (string)e);
                break;
            }
        }
    }
    else {
        LOG4CXX_FATAL(iLogger::GetLogger(), "redis_storage::fix_struct redis DB not initialized yet!");
    }
}

string_list* redis_storage::get_namespace_idxs(const string& objType)
{
    string_list* objIdxs = new string_list;
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
            LOG4CXX_WARN(iLogger::GetLogger(), "redis_storage::get_namespace_idxs '" << objType << "' error: " << (string)e);
            objIdxs->clear();
        }
    }
    else {
        LOG4CXX_FATAL(iLogger::GetLogger(), "redis_storage::get_namespace_idxs redis DB not initialized yet!");
        objIdxs->clear();
    }
    return objIdxs;
}
