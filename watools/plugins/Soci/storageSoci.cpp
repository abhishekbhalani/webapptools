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

#define VERSION_PRODUCTSTR "$Revision$"
#ifdef _MSC_VER
#pragma warning(disable:4275 4251 4996)
#endif

#include <fstream>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/regex.hpp>

#include "storageSoci.h"
#include "sociStorage.xpm"

#define SOCI_USE_BOOST
#define SOCI_USE_LOG4CXX
#include <soci.h>

typedef std::pair<webEngine::db_record*, soci::blob*>  db_record_soci;

namespace soci {
template<>
struct type_conversion<db_record_soci> {
    typedef values base_type;
    static void from_base(values const & v, indicator /* ind */, db_record_soci & data_soci) {
        try {
            db_record &data = *(data_soci.first);
            const size_t record_size = data.record_size();
            if(record_size == 0) {
                //LOG4CXX_TRACE(logger, _T("type_conversion<db_record>: record_size is zero, you must to initialize db_record with recordset!"));
                return;
            }
            for(size_t i = 0, j = data_soci.second ? 1 : 0; j < record_size; i++, j++) {
                if(v.get_indicator(i)) {
                    data[j].clear();
                } else {
                    switch (v.get_properties(i).get_data_type()) {
                    case dt_unsigned_long:
                    case dt_long_long:
                    case dt_integer:
                        data[j] = v.get<int>(i);
                        break;
                    case dt_double:
                        data[j] = v.get<double>(i);
                        break;
                    case dt_date:
                    case dt_string:
                        data[j] = v.get<string>(i);
                        break;
                    }
                }
            }
        } catch(soci::soci_error e) {
            //LOG4CXX_TRACE(logger, e.what());
        }
    }
    static void to_base(const db_record_soci & data_soci, values & v, indicator & ind) {
        const db_record &data = *(data_soci.first);
        ind = i_ok;
        const size_t record_size = data.record_size();
        if(record_size == 0) {
            //LOG4CXX_TRACE(logger, _T("type_conversion<db_record>: record_size is zero, you must to initialize db_record with recordset!"));
            return;
        }
        for(size_t i = 0; i < record_size; i++) {
            const we_types& val = data[i];
            switch (data[i].which()) {
            case 0: // char
                v.set(boost::get<char>(val));
                break;
            case 1: // int
                v.set(boost::get<int>(val));
                break;
            case 2: // bool
                v.set((int)boost::get<bool>(val));
                break;
            case 3: // double
                v.set(boost::get<double>(val));
                break;
            case 4: // string
                v.set(boost::get<string>(val));
                break;
            case 5: // blank
                v.set(boost::optional<int>(false));
                break;
            }
        }
    }
};
}

namespace soci_db {

soci_storage::soci_storage( engine_dispatcher* krnl, void* handle /*= NULL*/ ) :
    i_storage(krnl, handle)
{
    pluginInfo.interface_name = "soci_storage";
    pluginInfo.interface_list.push_back("soci_storage");
    pluginInfo.plugin_desc = "Soci storage ";
    pluginInfo.plugin_desc += VERSION_PRODUCTSTR;
    pluginInfo.plugin_id = "9FBCCA23CD84";
    pluginInfo.plugin_icon = WeXpmToStringList(sqliteStorage_xpm, sizeof(sqliteStorage_xpm) / sizeof(char*) );
    // initialize internal data
    try{
        m_sql.reset(new soci::session());
		//TODO: convert log4cxx to std::ostream
		if(m_logger)
			m_sql->set_log_stream(m_logger.get());
    } catch(soci::soci_error e) {
		LOG4CXX_ERROR(logger, std::string(e.what()));
    }
    LOG4CXX_TRACE(logger, _T("soci_storage plugin created; version ") << VERSION_PRODUCTSTR);
}

soci_storage::~soci_storage(void)
{
    boost::unique_lock<boost::mutex> locker(data_access);
    if (m_sql) {
        m_sql->close();
        m_sql.reset();
    }
    LOG4CXX_TRACE(logger, _T("soci_storage plugin destroyed"));
}

i_plugin* soci_storage::get_interface( const string& ifName )
{
    if (boost::iequals(ifName, "i_storage")) {
        usageCount++;
        return ((i_storage*)this);
    }
    if (boost::iequals(ifName, "soci_storage")) {
        usageCount++;
        return ((soci_storage*)this);
    }
    return i_storage::get_interface(ifName);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn bool sqlite_storage::init_storage( const string& params )
///
/// @brief  Initializes the storage_db.
///
/// @param  params - Pathname for the storage_db file.
/// @retval	true if it succeeds, false if it fails.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool soci_storage::init_storage( const string& params )
{
    bool result = false;
    //	int rc;
    char *err_msg;
    string sval;

    boost::unique_lock<boost::mutex> locker(data_access);
    LOG4CXX_TRACE(logger, _T("soci_storage::init_storage"));
    try {
        m_sql->open(params);
    } catch(soci::soci_error e) {
        LOG4CXX_ERROR(logger, std::string(e.what()));
        return false;
    }
    // loads existing tables
    vector<string> tables;
    vector<string> names;

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
            try {
                *m_sql << tbl_query;
            } catch(soci::soci_error e) {
				LOG4CXX_DEBUG(logger, std::string(e.what()));
            }
        }
        ++i;
    }
    // create internal parameters table
    tbl_query = "CREATE TABLE IF NOT EXISTS [_internals_] ([name] TEXT, [value] VARIANT)";
    try {
        *m_sql << tbl_query;
    } catch(soci::soci_error e) {
        LOG4CXX_DEBUG(logger, std::string(e.what()));
    }

    try {
        // create indexes
        *m_sql << "CREATE UNIQUE INDEX IF NOT EXISTS [scan_data_index] ON [scan_data] ([id] ASC, [task_id] ASC)";
        *m_sql << "CREATE UNIQUE INDEX IF NOT EXISTS [task_index] ON [task] ([id] ASC, [profile_id] ASC)";
        *m_sql << "CREATE UNIQUE INDEX IF NOT EXISTS [profile_idx] ON [profile] ([profile_id] ASC, [name] ASC)";
        *m_sql << "CREATE UNIQUE INDEX IF NOT EXISTS [profile_ui_index] ON [profile_ui] ([plugin_id] ASC, [locale] ASC)";
        *m_sql << "CREATE UNIQUE INDEX IF NOT EXISTS [modules_index] ON [modules] ([id] ASC, [instance] ASC, [class] ASC)";

        // not a good idea to turn off journaling, but we need to speed :(
        if(m_sql->get_backend_name() == "sqlite3") {
            *m_sql << "PRAGMA journal_mode = OFF";
        }
    } catch(soci::soci_error e) {
        LOG4CXX_DEBUG(logger, std::string(e.what()));
    }

    soci::row r;
    try {
        *m_sql << "SELECT [value] FROM [_internals_] WHERE [name] = 'last_id'", soci::into(r);
    } catch(soci::soci_error e) {
        LOG4CXX_DEBUG(logger, std::string(e.what()));
    }

    if (r.size() < 1) {
        try {
            *m_sql << "INSERT INTO _internals_ ([name], [value]) VALUES ('last_id', 0)";
        } catch(soci::soci_error e) {
            LOG4CXX_DEBUG(logger, std::string(e.what()));
        }
    } else {
        //last_id = r.get;
    }

#ifdef _DEBUG
    try {
        *m_sql << "CREATE TABLE [test] ([Id] INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL  UNIQUE , [Value] TEXT)";
    } catch(soci::soci_error e) {
		LOG4CXX_DEBUG(logger, std::string(e.what()));
    }
#endif

    return true;
}

string soci_storage::generate_id( const string& objType /*= ""*/ )
{
    int last_id = 0;
    boost::unique_lock<boost::mutex> locker(data_access);
    try {
        m_sql->begin();
        soci::statement st = (m_sql->prepare << "UPDATE _internals_ SET value=value+1 WHERE name == 'last_id'");
        st.execute(true);
        if(st.get_affected_rows()) {
            *m_sql << "SELECT [value] FROM [_internals_] WHERE [name] == 'last_id'", soci::into(last_id);
        } else {
            *m_sql << "INSERT INTO _internals_ ([name], [value]) VALUES ('last_id', 0)";
        }
        m_sql->commit();
    } catch(soci::soci_error e) {
        LOG4CXX_DEBUG(logger, std::string(e.what()));
    }
    return boost::lexical_cast<string>(last_id);
}


namespace details {

////////////////////////// CURSOR_SOCI ////////////////////

class db_cursor_base: public webEngine::details::db_cursor_detail {
    friend class soci_storage;
public:
    virtual ~db_cursor_base() {
        if(!m_isEnd)
            close();
    }
protected:
    db_cursor_base(	soci_storage &storage, const std::vector<std::string> &fields)
        :m_storage(storage), webEngine::details::db_cursor_detail(fields), m_soci_record(&m_record, (soci::blob*)NULL) {}
    soci_storage &m_storage;
    soci::values m_values;
    boost::scoped_ptr<soci::blob> m_blob;
    db_record_soci m_soci_record;
};

class db_cursor_get : public db_cursor_base {
    friend class soci_storage;
public:
    virtual ~db_cursor_get() {
        boost::shared_ptr<soci::statement> st = m_statement.lock();
        m_storage.free_statement(st);
    }
protected:
    virtual void increment() {
        if(!m_isEnd)
            m_storage.get_next(*this);
    }
private:
    db_cursor_get(soci_storage &storage, const std::vector<std::string> &fields):db_cursor_base(storage, fields) {}
    boost::weak_ptr<soci::statement> m_statement;
};

class db_cursor_set : public db_cursor_base {
    friend class soci_storage;
protected:
    virtual void increment() {
        if(!m_isEnd)
            m_storage.set_next(*this);
    }
private:
    db_cursor_set(soci_storage &storage, const std::vector<std::string> &fields):db_cursor_base(storage, fields) {}
    string m_query_upd;
    string m_query_ins;
};

class db_cursor_ins : public db_cursor_base {
    friend class soci_storage;
protected:
    virtual void increment() {
        if(!m_isEnd)
            m_storage.ins_next(*this);
    }
private:
    db_cursor_ins(soci_storage &storage, const std::vector<std::string> &fields):db_cursor_base(storage, fields) {}
    string m_query_ins;
};
}

db_cursor soci_storage::get(const string &query, const std::vector<std::string> &fields, bool need_blob)
{
    details::db_cursor_get* cursor = new details::db_cursor_get(*this, fields);
    boost::unique_lock<boost::mutex> locker(data_access);
    try{
        boost::shared_ptr<soci::statement> st(new soci::statement(*m_sql));
        st->exchange(soci::into(cursor->m_soci_record));
        if(need_blob){
            cursor->m_blob.reset(new soci::blob(*m_sql));
            cursor->m_soci_record.second = cursor->m_blob.get();
            st->exchange(soci::into(*cursor->m_blob));
        }
        st->alloc();
        st->prepare(query);
        st->define_and_bind();
        cursor->m_statement = st;
        st->execute();
        cursor->m_isEnd = !st->fetch();
        if(!cursor->m_isEnd && cursor->m_blob){
            webEngine::blob tmp_blob(cursor->m_blob->get_len());
            cursor->m_blob->read(0, (char*)&tmp_blob[0], cursor->m_blob->get_len());
            cursor->m_record[0] = tmp_blob;
        }
        m_statements.insert(st);
    } catch(soci::soci_error e) {
        LOG4CXX_ERROR(logger, std::string(e.what()));
    }
    return db_cursor( boost::shared_ptr<webEngine::details::db_cursor_detail>((webEngine::details::db_cursor_detail*)cursor));
}

db_cursor soci_storage::set(const string &query_update, const string &query_insert, const std::vector<std::string> &fields, bool need_blob)
{
    details::db_cursor_set* cursor = new details::db_cursor_set(*this, fields);
    boost::unique_lock<boost::mutex> locker(data_access);
    try{
        if(need_blob){
            cursor->m_blob.reset(new soci::blob(*m_sql));
        }
        cursor->m_query_ins = query_insert;
        cursor->m_query_upd = query_update;
        cursor->m_isEnd = false;
    } catch(soci::soci_error e) {
        LOG4CXX_ERROR(logger, std::string(e.what()));
    }
    return db_cursor( boost::shared_ptr<webEngine::details::db_cursor_detail>((webEngine::details::db_cursor_detail*)cursor));
}

db_cursor soci_storage::ins(const string &query_insert, const std::vector<std::string> &fields, bool need_blob)
{
    details::db_cursor_ins* cursor = new details::db_cursor_ins(*this, fields);
    boost::unique_lock<boost::mutex> locker(data_access);
    try{
        if(need_blob){
            cursor->m_blob.reset(new soci::blob(*m_sql));
        }
        cursor->m_query_ins = query_insert;
        cursor->m_isEnd = false;
    } catch(soci::soci_error e) {
        LOG4CXX_ERROR(logger, std::string(e.what()));
    }
    return db_cursor( boost::shared_ptr<webEngine::details::db_cursor_detail>((webEngine::details::db_cursor_detail*)cursor));
}

int soci_storage::del(const string &query)
{
    int result = 0;
    boost::unique_lock<boost::mutex> locker(data_access);
    try{
        soci::statement st = (m_sql->prepare << query);
        st.execute(true);
        result = static_cast<int>(st.get_affected_rows());
    } catch(soci::soci_error e) {
        LOG4CXX_ERROR(logger, std::string(e.what()));
    }
    return result;
}

int soci_storage::count(const string &query)
{
    boost::unique_lock<boost::mutex> locker(data_access);
    int i = 0;
    try{
        *m_sql << query , soci::into(i);
    } catch(soci::soci_error e) {
        LOG4CXX_ERROR(logger, std::string(e.what()));
    }
    return i;
}

void soci_storage::get_next(details::db_cursor_get &cursor)
{
    if(cursor.m_isEnd)
        return;
    boost::unique_lock<boost::mutex> locker(data_access);
    cursor.m_affected_rows = 0;
    cursor.m_isEnd = true;
    boost::shared_ptr<soci::statement> st = cursor.m_statement.lock();
    try{
        if(st && st->fetch()){
            cursor.m_isEnd = false;
            if(cursor.m_blob){
                webEngine::blob tmp_blob(cursor.m_blob->get_len());
                cursor.m_blob->read(0, (char*)&tmp_blob[0], cursor.m_blob->get_len());
                cursor.m_record[0] = tmp_blob;
            }
        }
    } catch(soci::soci_error e) {
        LOG4CXX_ERROR(logger, std::string(e.what()));
    }
}

void soci_storage::set_next(details::db_cursor_set &cursor)
{
    if(cursor.m_isEnd)
        return;
    int res = 0;
    boost::unique_lock<boost::mutex> locker(data_access);
    try{
        if(cursor.m_blob){
            const webEngine::we_types &tmp_var = cursor.m_record[cursor.m_record.record_size()];
            const webEngine::blob &tmp_blob = boost::get<webEngine::blob>(tmp_var);
            cursor.m_blob->write(0, (const char *)&tmp_blob[0], tmp_blob.size());
        }

        soci::statement st_upd(*m_sql);
        st_upd.prepare(cursor.m_query_upd);
        st_upd.exchange(soci::use(cursor.m_soci_record));
        if(cursor.m_blob){
            st_upd.exchange(soci::use(*cursor.m_blob));
        }
        st_upd.alloc();
        st_upd.define_and_bind();
        st_upd.execute(true);
        res = static_cast<int>(st_upd.get_affected_rows());
        if(!res) {
            soci::statement st_ins(*m_sql);
            st_ins.prepare(cursor.m_query_ins);
            st_ins.exchange(soci::use(cursor.m_soci_record));
            if(cursor.m_blob){
                st_ins.exchange(soci::use(*cursor.m_blob));
            }
            st_ins.alloc();
            st_ins.define_and_bind();
            st_ins.execute(true);
            res = static_cast<int>(st_ins.get_affected_rows());
        }
    } catch(soci::soci_error e) {
        LOG4CXX_ERROR(logger, std::string(e.what()));
    }
    cursor.m_affected_rows = res;
    cursor.m_isEnd = !res;
}

void soci_storage::ins_next(details::db_cursor_ins &cursor)
{
    if(cursor.m_isEnd)
        return;
    int res = 0;
    boost::unique_lock<boost::mutex> locker(data_access);
    try{
        soci::statement st_ins(*m_sql);
        st_ins.prepare(cursor.m_query_ins);
        st_ins.exchange(soci::use(cursor.m_soci_record));
        if(cursor.m_blob){
            const we_types &var = cursor.m_record[0];
            const webEngine::blob &weBlob = boost::get<webEngine::blob>(var);
            cursor.m_blob->write(0, (const char *)&weBlob[0], weBlob.size());
            st_ins.exchange(soci::use(*cursor.m_blob));
        }
        st_ins.alloc();
        st_ins.define_and_bind();
        st_ins.execute(true);
        res = static_cast<int>(st_ins.get_affected_rows());
    } catch(soci::soci_error e) {
        LOG4CXX_ERROR(logger, std::string(e.what()));
    }
    cursor.m_affected_rows = res;
    cursor.m_isEnd = !res;
}

void soci_storage::free_statement(boost::shared_ptr<soci::statement> st)
{
    boost::unique_lock<boost::mutex> locker(data_access);
    try{
        m_statements.erase(st);
    } catch(soci::soci_error e) {
        LOG4CXX_ERROR(logger, std::string(e.what()));
    }
}

}
