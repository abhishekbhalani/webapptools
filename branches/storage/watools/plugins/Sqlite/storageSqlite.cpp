/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew Abramov aabramov@ptsecurity.ru

    This file is part of webEngine

    webEngine is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/regex.hpp>

#include "storageSqlite.h"
#include "version.h"
#include "sqliteStorage.xpm"
#include "sqlite3.h"

struct sqlite_handle
{
    sqlite3 *db;
    int resp_size;
    log4cxx::LoggerPtr logger;
};

static int sqlite_callback(void* object, int argc, char **argv, char **azColName)
{
    string report = "";
    int i;
    sqlite_handle *handle = (sqlite_handle*)object;

    for (i = 0; i < argc; ++i) {
        report += azColName[i];
        report += " = ";
        report += argv[i] ? argv[i] : "NULL";
        report += "; ";
    }
    LOG4CXX_TRACE(handle->logger, "sqlite_callback: " << report);
    handle->resp_size++;

    return 0;
}

static int sqlite_query_to_rs(sqlite_handle* handle, const string& query, db_recordset& results)
{
    int retval = 0;
    int rc, dtype;
    sqlite3_stmt *pStmt;
    int rcs, cls, i;
    db_cursor rec;
    string sval;

    rc = sqlite3_prepare(handle->db, query.c_str(), query.length() + 1, &pStmt, NULL);
    if ( rc == SQLITE_OK) {
        // get minimal allowed number of columns
        rcs = results.record_size();
        cls = sqlite3_column_count(pStmt);
        if (rcs > cls) {
            rcs = cls;
        }
        // extract rows
        rc = sqlite3_step(pStmt);
        while (rc == SQLITE_ROW) {
            rec = results.push_back();
            for (i = 0; i < rcs; ++i) {
                dtype = sqlite3_column_type(pStmt, i);
                switch(dtype) {
                    case SQLITE_INTEGER:
                        rec[i] = sqlite3_column_int(pStmt, i);
                        break;
                    case SQLITE_FLOAT:
                        rec[i] = sqlite3_column_double(pStmt, i);
                        break;
                    case SQLITE_NULL:
                        rec[i] = boost::blank();
                        break;
                    // case SQLITE_BLOB:
                    // case SQLITE_TEXT:
                    default:
                        sval = (const char*)sqlite3_column_text(pStmt, i);
                        rec[i] = sval;
                        break;
                }
            }
            retval++;
            // get next row
            rc = sqlite3_step(pStmt);
        }
        if ( rc == SQLITE_BUSY ) {
            LOG4CXX_WARN(handle->logger, "sqlite3_step: database busy");
        }
        if ( rc == SQLITE_ERROR || rc == SQLITE_MISUSE) {
            LOG4CXX_ERROR(handle->logger, "sqlite3_step error: " << rc << "; " << sqlite3_errmsg(handle->db));
        }
        // finalize query
        rc = sqlite3_finalize(pStmt);
        if (rc != SQLITE_OK) {
            LOG4CXX_ERROR(handle->logger, "sqlite3_finalize error: " << rc << "; " << sqlite3_errmsg(handle->db));
        }
    }
    else {
        LOG4CXX_ERROR(handle->logger, "sqlite3_prepare error: " << rc << "; " << sqlite3_errmsg(handle->db));
    }

    return retval;
}

static int sqlite_query_from_rs(sqlite_handle* handle, const string& query, db_record& data, int record_size)
{
    sqlite3_stmt *pStmt;
    int rc;
    size_t i;
    int retval = 0;
    string sval;

    rc = sqlite3_prepare(handle->db, query.c_str(), query.length() + 1, &pStmt, NULL);
    if ( rc == SQLITE_OK ) {
        sqlite3_reset(pStmt);
        for (i = 0; i < record_size; ++i) {
            switch (data[i].which()) {
            case 1: // int
            case 2: // bool
                rc = sqlite3_bind_int(pStmt, i+1, boost::lexical_cast<int>(data[i]));
                break;
            case 3: // double
                rc = sqlite3_bind_double(pStmt, i+1, data[i].get<double>());
                break;
            case 0: // char
            case 4: // string
                sval = boost::lexical_cast<string>(data[i]);
                rc = sqlite3_bind_text(pStmt, i+1, sval.c_str(), -1, SQLITE_TRANSIENT);
                break;
            case 5: // blank
                rc = sqlite3_bind_null(pStmt, i+1);
                break;
            }
            if ( rc != SQLITE_OK ) {
                LOG4CXX_ERROR(handle->logger, "sqlite_query_from_rs sqlite3_step error: " << rc << "; " << sqlite3_errmsg(handle->db));
            }
        } // foreach field
        rc = sqlite3_step(pStmt);
        if ( rc != SQLITE_OK ) {
            LOG4CXX_ERROR(handle->logger, "sqlite_query_from_rs sqlite3_step error: " << rc << "; " << sqlite3_errmsg(handle->db));
        }
        // finalize query
        rc = sqlite3_finalize(pStmt);
        if (rc != SQLITE_OK) {
            LOG4CXX_ERROR(handle->logger, "sqlite_query_from_rs sqlite3_finalize error: " << rc << "; " << sqlite3_errmsg(handle->db));
        }
        retval = sqlite3_changes(handle->db);
    } // if statement prepared
    else {
        LOG4CXX_ERROR(handle->logger, "sqlite_query_from_rs sqlite3_prepare error: " << rc << "; " << sqlite3_errmsg(handle->db));
    }
    return retval;
}

static string sqlite_fix_filter(const string& filter)
{
    string result;
//     // \s[\w\d_]+\.([\w\d_]+)(\s+[^\)]) => \s[$1]$2
//     boost::regex srch1("\\s[\\w\\d_]+\\.([\\w\\d_]+)(\\s+[^\\)])");
//     string repl1 = " [$1]$2";
//     result = boost::regex_replace(filter, srch1, repl1, boost::match_default | boost::format_all);

    // fix string values
    /// @todo implement smart screening (select between " and ')
    // ([=><]\s+)([\S]+[^\d\s][\S]+)\s+ => $1"$2" )
    boost::regex srch1("([=><]\\s+)([\\S]+[^\\d\\s][\\S]+)\\s+");
    string repl1 = "$1\"$2\" ";
    result = boost::regex_replace(filter, srch1, repl1, boost::match_default | boost::format_all);

    // fix "like" statement
    // like\s+([^\)]+)\s+\) => like '%$1%' )
    boost::regex srch2("like\\s+([^\\)]+)\\s+\\)");
    string repl2 = " like '%$1%' )";
    result = boost::regex_replace(result, srch2, repl2, boost::match_default | boost::format_all);

    return result;
}

sqlite_storage::sqlite_storage( engine_dispatcher* krnl, void* handle /*= NULL*/ ) :
    i_storage(krnl, handle)
{
    pluginInfo.interface_name = "sqlite_storage";
    pluginInfo.interface_list.push_back("sqlite_storage");
    pluginInfo.plugin_desc = "Sqlite3 storage";
    pluginInfo.plugin_id = "9FBCC44C52A1";
    pluginInfo.plugin_icon = WeXpmToStringList(sqliteStorage_xpm, sizeof(sqliteStorage_xpm) / sizeof(char*) );
    // initialize internal data
    db_handle = new sqlite_handle;
    if (db_handle != NULL) {
        db_handle->db = NULL;
        db_handle->logger = logger;
        db_handle->resp_size = 0;
    }
    else {
        boost::throw_exception(bad_alloc("sqlite_storage: can't create internal storage"));
    }
    LOG4CXX_TRACE(logger, "sqlite_storage plugin created; version " << AutoVersion::FULLVERSION_STRING);
}

sqlite_storage::~sqlite_storage(void)
{
    if (db_handle != NULL) {
        // at last - destroy sqlite_handle
        delete db_handle;
    }
    LOG4CXX_TRACE(logger, "sqlite_storage plugin destroyed");
}

i_plugin* sqlite_storage::get_interface( const string& ifName )
{
	if (boost::iequals(ifName, "i_storage"))
    {
        usageCount++;
        return ((i_storage*)this);
    }
    if (boost::iequals(ifName, "sqlite_storage"))
    {
        usageCount++;
        return ((sqlite_storage*)this);
    }
    return i_storage::get_interface(ifName);
}

int sqlite_storage::get(db_query& query, db_recordset& results)
{
    int retval = 0;
    size_t i;
    string qstr;
    std::set<string> ns_list;
    std::set<string>::iterator ns_it;

    LOG4CXX_TRACE(logger, "sqlite_storage::get");
    boost::unique_lock<boost::mutex> locker(data_access);
    if (db_handle->db != NULL) {
        results.clear();
        results.set_names(query.what);
        query.where.get_namespaces(ns_list);
        if (ns_list.size() > 0) {
            qstr = "SELECT ";
            for (i = 0; i < query.what.size(); ++i) {
                if (i > 0) qstr += ", ";
                qstr += query.what[i];
            }
            qstr += " FROM ";
            for (ns_it = ns_list.begin(), i = 0; ns_it != ns_list.end(); ++ns_it, ++i) {
                if (i > 0) qstr += ", ";
                qstr += "[" + *ns_it + "]";
            }
            qstr += " WHERE " + sqlite_fix_filter(query.where.tostring());
            retval = sqlite_query_to_rs(db_handle, qstr, results);
        }
        else {
            LOG4CXX_ERROR(iLogger::GetLogger(), "sqlite_storage::get can't determine tables for query");
        }
    }
    return retval;
}

int sqlite_storage::set(db_query& query, db_recordset& data)
{
    int retval = 0;
    string table_name;
    size_t i, p;
    int rc;
    vector<string> rec_fields;
    string qstr_upd, qstr_ins, binder;

    // UPDATE [profile] SET type=1, value=11 WHERE (profile.profile_id == 0 AND name=="test_param" )
    LOG4CXX_TRACE(iLogger::GetLogger(), "sqlite_storage::set(db_query, db_recordset)");
    boost::unique_lock<boost::mutex> locker(data_access);
    if (db_handle->db != NULL) {
        rec_fields = data.get_names();
        if (rec_fields.size() > 0) {
            table_name = rec_fields[0];
            p = table_name.find_last_of('.');
            if (p != string::npos) {
                table_name = table_name.substr(0, p);
                qstr_upd = "UPDATE [" + table_name + "] SET ";
                qstr_ins = "INSERT INTO " + table_name + " (";
                binder = "";
                for ( i = 0; i < rec_fields.size(); ++i) {
                    if ( i > 0 ) {
                        qstr_upd += ", ";
                        qstr_ins += ", ";
                        binder += ", ";
                    }
                    table_name = rec_fields[i];
                    p = table_name.find_last_of('.');
                    if (p != string::npos) {
                        table_name = table_name.substr(p + 1);
                    }
                    qstr_upd += table_name + "=?";
                    qstr_ins += table_name;
                    binder += "?";
                }
                /// @todo append filter as inserted data
                qstr_ins += ") VALUES (" + binder + ")";
                qstr_upd += " WHERE " + sqlite_fix_filter(query.where.tostring());

                // manual performing the query :(
                db_fw_cursor cur;
                for (cur = data.fw_begin(); cur != data.fw_end(); ++cur) {
                    rc = sqlite_query_from_rs(db_handle, qstr_upd, *cur, cur.record_size());
                    if (rc == 0) {
                        // need to insert
                        rc = sqlite_query_from_rs(db_handle, qstr_ins, *cur, cur.record_size());
                    }
                    retval += rc;
                } // for each source record
            } // if table name found
        } // if record isn't empty
    } // if db initialized
    return retval;
}

int sqlite_storage::set(db_recordset& data)
{
    int retval = 0;
    string table_name;
    size_t i, p;
    vector<string> rec_fields;
    string qstr, binder;

    LOG4CXX_TRACE(iLogger::GetLogger(), "sqlite_storage::set(db_recordset)");
    boost::unique_lock<boost::mutex> locker(data_access);
    if (db_handle->db != NULL) {
        rec_fields = data.get_names();
        if (rec_fields.size() > 0) {
            table_name = rec_fields[0];
            p = table_name.find_last_of('.');
            if (p != string::npos) {
                table_name = table_name.substr(0, p);
                qstr = "INSERT INTO " + table_name + " (";
                binder = "";
                for ( i = 0; i < rec_fields.size(); ++i) {
                    if ( i > 0 ) {
                        qstr += ", ";
                        binder += ", ";
                    }
                    table_name = rec_fields[i];
                    p = table_name.find_last_of('.');
                    if (p != string::npos) {
                        table_name = table_name.substr(p + 1);
                    }
                    qstr += table_name;
                    binder += "?";
                }
                qstr += ") VALUES (" + binder + ")";

                // manual performing the query :(
                db_fw_cursor cur;
                for (cur = data.fw_begin(); cur != data.fw_end(); ++cur) {
                    retval += sqlite_query_from_rs(db_handle, qstr, *cur, cur.record_size());
                } // foreach source record
            } // if table name found
        } // if record isn't empty
    } // if db initialized
    return retval;
}

int sqlite_storage::del(db_filter& filter)
{
    int retval = 0;
    int rc;
    char *err_msg;
    string query;
    std::set<string> ns_list;
    std::set<string>::iterator ns_it;

    LOG4CXX_TRACE(iLogger::GetLogger(), "sqlite_storage::del");
    boost::unique_lock<boost::mutex> locker(data_access);
    if (db_handle->db != NULL) {
        filter.get_namespaces(ns_list);
        for (ns_it = ns_list.begin(); ns_it != ns_list.end(); ns_it++) {
            query = "DELETE FROM [" + *ns_it + "] WHERE " + sqlite_fix_filter(filter.tostring());
            LOG4CXX_DEBUG(iLogger::GetLogger(), "sqlite_storage::del execute: " << query);
            db_handle->resp_size = 0;
            rc = sqlite3_exec(db_handle->db, query.c_str(), sqlite_callback, (void*)db_handle, &err_msg);
            if (rc != SQLITE_OK) {
                LOG4CXX_ERROR(logger, "sqlite_storage::del sqlite3_exec error: " << rc << "; " << err_msg);
            }
            retval += sqlite3_changes(db_handle->db);
        } // foreach table
    }
    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn bool sqlite_storage::init_storage( const string& params )
///
/// @brief  Initializes the storage_db. 
///
/// @param  params - Pathname for the storage_db file.
/// @retval	true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool sqlite_storage::init_storage( const string& params )
{
	bool result = false;
    int rc;
    char *err_msg;
    string sval;

    boost::unique_lock<boost::mutex> locker(data_access);
    LOG4CXX_TRACE(iLogger::GetLogger(), "sqlite_storage::init_storage");
    rc = sqlite3_open(params.c_str(), &(db_handle->db));
    if ( rc ) {
        LOG4CXX_FATAL(iLogger::GetLogger(), "sqlite_storage::init_storage can't open database: " << sqlite3_errmsg(db_handle->db));
        sqlite3_close(db_handle->db);
        db_handle->db = NULL;
    }
    else {
        // loads existing tables
        db_recordset rs;
        vector<string> tables;
        vector<string> names;

        names.push_back("name");
        rs.set_names(names);
        sqlite_query_to_rs(db_handle, "SELECT name FROM sqlite_master WHERE type='table';", rs);
        for (db_fw_cursor c = rs.fw_begin(); c != rs.fw_end(); ++c) {
            tables.push_back(c["name"].get<string>());
        }

        // check table presence
        size_t i = 0;
        size_t j;
        size_t p;
        std::vector<std::string> inits;
        std::vector<std::string> fields;
        string tbl_query;

        while (idb_struct[i] != NULL) {
            boost::split(inits, idb_struct[i], boost::is_any_of(":"));
            if (inits.size() > 0) {
                if (find(tables.begin(), tables.end(), inits[0]) == tables.end()) {
                    // create table
                    tbl_query = "CREATE TABLE IF NOT EXISTS [";
                    tbl_query += inits[0];
                    tbl_query += "] (";
                    // add columns
                    for (j = 1; j < inits.size(); j++) {
                        if (j > 1) {
                            tbl_query += ", ";
                        }
                        sval = inits[j];
                        // fold column name into []
                        sval = "[" + sval;
                        p = sval.find(' ');
                        if (p != string::npos) {
                            sval.insert(p, "]");
                        }
                        tbl_query += sval;
                    }
                    // create table!
                    tbl_query += ");";
                    err_msg = NULL;
                    db_handle->resp_size = 0;
                    rc = sqlite3_exec(db_handle->db, tbl_query.c_str(), sqlite_callback, (void*)db_handle, &err_msg);
                    if (rc != SQLITE_OK) {
                        LOG4CXX_ERROR(logger, "sqlite_storage::init_storage sqlite3_exec error: " << rc << "; " << err_msg);
                    }
                }
                else {
                    /// @todo check the table consistence
                }
            }
            ++i;
        }
        // create internal parameters table
        tbl_query = "CREATE TABLE IF NOT EXISTS [_internals_] ([name] TEXT, [value] VARIANT)";
        rc = sqlite3_exec(db_handle->db, tbl_query.c_str(), sqlite_callback, (void*)db_handle, &err_msg);

        names.clear();
        names.push_back("value");
        rs.clear();
        rs.set_names(names);
        tbl_query = "SELECT [value] FROM [_internals_] WHERE [name] = 'last_id'";
        sqlite_query_to_rs(db_handle, tbl_query, rs);
        if (rs.size() < 1) {
            last_id = 0;
            tbl_query = "INSERT INTO _internals_ ([name], [value]) VALUES ('last_id', 0)";
            sqlite3_exec(db_handle->db, tbl_query.c_str(), sqlite_callback, (void*)db_handle, &err_msg);
        }
        else {
            last_id = boost::lexical_cast<int>(rs.begin()[0]);
        }

    }
	return result;
}

string sqlite_storage::generate_id( const string& objType /*= ""*/ )
{
    db_recordset rs;
    vector<string> names;
    string tbl_query;
    char *err_msg;

    names.clear();
    names.push_back("value");
    rs.clear();
    rs.set_names(names);
    tbl_query = "SELECT [value] FROM [_internals_] WHERE [name] == 'last_id'";
    sqlite_query_to_rs(db_handle, tbl_query, rs);
    if (rs.size() < 1) {
        // must never happens
        last_id = 0;
        tbl_query = "INSERT INTO _internals_ ([name], [value]) VALUES ('last_id', 0)";
        sqlite3_exec(db_handle->db, tbl_query.c_str(), sqlite_callback, (void*)db_handle, &err_msg);
    }
    else {
        db_cursor cur = rs.begin();
        last_id = boost::lexical_cast<int>(cur[0]);
        last_id++;
        cur[0] = last_id;
        tbl_query = "UPDATE [_internals_] SET [value]=? WHERE [name] == 'last_id'";
        sqlite_query_from_rs(db_handle, tbl_query, *cur, cur.record_size());
    }
    return boost::lexical_cast<string>(last_id);
}
//void sqlite_storage::flush( const string& params /*= ""*/)
//{
//}
