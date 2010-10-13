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
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/tokenizer.hpp>
#include <weTagScanner.h>
#include <strstream>
#ifdef WIN32
#include <winsock2.h>
#endif
#include "storageRedis.h"
#include "redisDbstruct.h"
#include "version.h"
#include "redis.xpm"

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
    if (db_cli != NULL) {
        delete db_cli;
    }
    LOG4CXX_TRACE(logger, "redis_storage plugin destroyed");
}

i_plugin* redis_storage::get_interface( const string& ifName )
{
    LOG4CXX_TRACE(logger, "redis_storage::get_interface " << ifName);
    if (boost::iequals(ifName, "redis_storage")) {
        LOG4CXX_DEBUG(logger, "redis_storage::get_interface found!");
        usageCount++;
        return (this);
    }
    return i_storage::get_interface(ifName);
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

    for (tokenizer::iterator tok_iter = tok_param.begin(); tok_iter != tok_param.end(); ++tok_iter) {
        dat = *tok_iter;
        pos = dat.find('=');
        if (pos != string::npos) {
            name = dat.substr(0, pos);
            dat = dat.substr(pos + 1);
            LOG4CXX_TRACE(logger, "redis_storage::init_storage found parameter: " << name << "; value: " << dat);
            if (name == "host") {
                db_host = dat;
            }
            if (name == "port") {
                db_port = boost::lexical_cast<int>(dat);
            }
            if (name == "dbnum") {
                db_index = boost::lexical_cast<int>(dat);
            }
            if (name == "auth") {
                db_auth = dat;
            }
        }
    }

    boost::lock_guard<boost::mutex> lock(db_lock);

    if (db_cli != NULL) {
        delete db_cli;
    }
    dat = "connection creation";
    try {
        db_cli = new redis::client(db_host, db_port);
        if (db_auth != "") {
            dat = "authentication";
            db_cli->auth(db_auth);
        }
        dat = "select DB";
        db_cli->select(db_index);
        retval = true;
    } catch(redis::redis_error& e) {
        LOG4CXX_FATAL(logger, "RedisDB connection error (" << dat << "): " << (string)e);
        retval = false;
        if (db_cli != NULL) {
            delete db_cli;
        }
        db_cli = NULL;
    }

    if (retval) {
        // check table presence
        size_t i = 0;
        size_t j;
        std::vector<std::string> inits;
        redis::client::string_set tables;

        try {
            db_cli->smembers("tables", tables);
        } catch(redis::redis_error&) {
            tables.clear();
        }

        while (idb_struct[i] != NULL) {
            boost::split(inits, idb_struct[i], boost::is_any_of(":"));
            if (inits.size() > 0) {
                if (tables.find(inits[0]) == tables.end()) {
                    // create table
                    db_cli->sadd("tables", inits[0]);
                    db_cli->set(inits[0] + ".rowid", "0");
                    for (j = 1; j < inits.size(); j++) {
                        string fld = inits[j];
                        size_t pos = fld.find(' ');
                        if (pos != string::npos) {
                            fld = fld.substr(0, pos);
                        }
                        db_cli->rpush(inits[0] + ".struct", fld);
                    }
                } else {
                    /// @todo check the table consistence
                }
            } // if inits.size() > 0
            ++i;
        } // idb_struct[i] != NULL
        if (! db_cli->exists("index")) {
            db_cli->set("index", "0");
        }
    }

    return retval;
}

void redis_storage::flush(const string& params /*= ""*/)
{
    if (db_cli) {
        boost::lock_guard<boost::mutex> lock(db_lock);
        try {
            db_cli->bgsave();
        } catch(redis::redis_error& e) {
            LOG4CXX_ERROR(logger, "RedisDB save: " << (string)e);
        }
    }
}

string redis_storage::generate_id(const string& objType /*= ""*/)
{
    if (db_cli) {
        boost::lock_guard<boost::mutex> lock(db_lock);
        try {
            last_id = db_cli->incr("index");
        } catch(redis::redis_error& e) {
            LOG4CXX_ERROR(logger, "RedisDB can't get index for " << objType << ": " << (string)e);
            try {
                ++last_id;
                db_cli->set("index", boost::lexical_cast<string>(last_id));
            } catch(redis::redis_error& e) {
                LOG4CXX_ERROR(logger, "RedisDB can't get index: " << (string)e);
            }
        }
    }

    return boost::lexical_cast<string>(last_id);
}

int redis_storage::get(db_query& query, db_recordset& results)
{
    LOG4CXX_TRACE(logger, "redis_storage::get");

    results.clear();
    results.set_names(query.what);

    if (db_cli == NULL) {
        LOG4CXX_FATAL(logger, "redis_storage::get RedisDB not initialized yet!");
        return 0;
    }

    boost::lock_guard<boost::mutex> lock(db_lock);

    std::set<string> ns_list;
    std::set<string>::iterator ns_it;
    vector<string>::iterator rec_it;
    db_cursor record;
    redis_cursor db_view;

    query.where.get_namespaces(ns_list);
    // @todo implement db_recordset.join to perform queries from more than one table
    if (ns_list.size() != 1) {
        LOG4CXX_ERROR(logger, "redis_storage::get - query wants data from " << ns_list.size() << " tables; Only one table supported.");
    }
    if (ns_list.size() > 0) {
        ns_it = ns_list.begin();
        db_view = redis_cursor::begin(db_cli, *ns_it);
        while (db_view != redis_cursor::end(db_cli, *ns_it)) {
            if (query.where.eval(db_view)) {
                record = results.push_back();
                for(rec_it = query.what.begin(); rec_it != query.what.end(); rec_it++) {
                    try {
                        record[*rec_it] = db_view[*rec_it];
                    } catch(out_of_range &e) {
                        LOG4CXX_ERROR(logger, "redis_storage::get - can't get filed " << *rec_it << ": source doesn't contains this field. " << e.what());
                    }
                } // foreach field
            } // if filter is true
            ++db_view;
        } // foreach record
    } // if namespace exists
    return (int)results.size();
}

int redis_storage::set(db_query& query, db_recordset& data)
{
    int retval = 0;

    LOG4CXX_TRACE(logger, "redis_storage::set(db_record, db_record)");
    if (db_cli == NULL) {
        LOG4CXX_FATAL(logger, "redis_storage::get RedisDB not initialized yet!");
        return 0;
    }

    boost::lock_guard<boost::mutex> lock(db_lock);

    std::set<string> ns_list;
    std::set<string>::iterator ns_it;
    redis_cursor cursor;
    db_cursor record;

    query.where.get_namespaces(ns_list);
    // @todo implement db_recordset.join to perform queries from more than one table
    if (ns_list.size() != 1) {
        LOG4CXX_ERROR(iLogger::GetLogger(), "mem_storage::get - query wants data from " << ns_list.size() << " tables; Only one table supported.");
    }
    if (ns_list.size() > 0) {
        ns_it = ns_list.begin();
        cursor = redis_cursor::begin(db_cli, *ns_it);
        vector<string> rec_fields = data.get_names();
        while (cursor != redis_cursor::end(db_cli, *ns_it)) {
            if (query.where.eval(cursor)) {
                // update record
                // @todo only firs record from argument used for update
                record = data.begin();
                try {
                    for (size_t i = 0; i < rec_fields.size(); i++) {
                        cursor[rec_fields[i]] = record[rec_fields[i]];
                    } // foreach field
                    retval++;
                } catch(out_of_range &) {};
                cursor.commit();
                retval++;
            } // if need to update
            ++cursor;
        } // foreach record
        if (retval == 0) {
            // no records updated
            db_fw_cursor record;
            for (record = data.fw_begin(); record != data.fw_end(); ++record) {
                retval += insert(*ns_it, rec_fields, *record);
            } // foreach record

        }

    } // if namespace exists

    return (int)retval;
}

int redis_storage::set(db_recordset& data)
{
    int retval = 0;

    LOG4CXX_TRACE(logger, "redis_storage::set(db_recordset)");
    boost::lock_guard<boost::mutex> lock(db_lock);

    vector<string> rec_fields = data.get_names();
    if (rec_fields.size() > 0) {
        string ns_name = rec_fields[0];
        size_t i = ns_name.find_last_of('.');
        if (i != string::npos) {
            ns_name = ns_name.substr(i + 1);
            db_fw_cursor record;
            for (record = data.fw_begin(); record != data.fw_end(); ++record) {
                retval += insert(ns_name, rec_fields, *record);
            } // foreach record
        } // if namespace found
    } // if data exists
    return retval;
}

int redis_storage::del(db_filter& filter)
{
    LOG4CXX_TRACE(logger, "redis_storage::del");

    if (db_cli == NULL) {
        LOG4CXX_FATAL(logger, "redis_storage::del RedisDB not initialized yet!");
        return 0;
    }

    boost::lock_guard<boost::mutex> lock(db_lock);

    int retval = 0;
    std::set<string> ns_list;
    std::set<string>::iterator ns_it;
    redis_cursor cursor;

    filter.get_namespaces(ns_list);
    for (ns_it = ns_list.begin(); ns_it != ns_list.end(); ns_it++) {
        cursor = redis_cursor::begin(db_cli, *ns_it);
        vector<string> fields = cursor.field_names();
        while (cursor != redis_cursor::end(db_cli, *ns_it)) {
            if (filter.eval(cursor)) {
                string rowid = *ns_it + "." + boost::lexical_cast<string>(cursor.row_id());
                db_cli->del(rowid);
                retval++;
            } // if applied filter
            ++cursor;
        } // foreach record
    } // foreach table
    return retval;
}

int redis_storage::insert( string ns, vector<string> fields, db_record& rec )
{
    int retval = 1;
    int id = db_cli->incr(ns + ".rowid");
    string row_id = boost::lexical_cast<string>(id);
    string key = ns + "." + row_id;
    for (size_t i = 0; i < fields.size(); i++) {
        try {
            string val = boost::lexical_cast<string>(rec[fields[i]].which()) + ":" + boost::lexical_cast<string>(rec[fields[i]]);
            try {
                db_cli->rpush(key, val);
            } catch(redis::redis_error& e) {
                retval = 0;
                LOG4CXX_ERROR(logger, "RedisDB can't save " << fields[i] << " for RowID = " << row_id << " - " << (string)e);
            }
        } catch(out_of_range &) {
            db_cli->rpush(key, "5:");
        };
    } // foreach field

    return retval;
}