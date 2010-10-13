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

#include "redisDbstruct.h"
#include <boost/algorithm/string/replace.hpp>
#include <sstream>

using namespace std;

redis_cursor::redis_cursor() :
    db_cursor_base(NULL)
{
    db_cli = NULL;
    base = new redis_recordset;
    parent_ = base;
    iter_ = base->data().begin();
    update();
    rowid = -1;
    rec_sz = 0;
}

redis_cursor::redis_cursor( redis::client* dbcli, string nspace, size_t idx ) :
    db_cursor_base(NULL)
{
    db_cli = dbcli;
    name_space = nspace;
    rowid = idx;
    base = new redis_recordset;
    parent_ = base;
    iter_ = base->data().begin();
    update();
    rec_sz = get_record_size();
    if (rowid > size()) {
        rowid = -1;
    }
    try {
        db_cli->lrange(name_space + ".struct", 0, -1, fnames);
    } catch(redis::redis_error& e) {
        ostringstream ss(last_err);
        ss << "RedisDB can't get " << name_space << ".struct - " << (string)e;
    }
    for (size_t i = 0; i < fnames.size(); i++) {
        fnames[i] = name_space + "." + fnames[i];
    }
    base->set_names(fnames);
    base->data()[0].resize(rec_sz);
    update();
}

redis_cursor::redis_cursor( const redis_cursor& cpy ) :
    db_cursor_base(NULL)
{
    db_cli = cpy.db_cli;
    name_space = cpy.name_space;
    rowid = cpy.rowid;
    rec_sz = cpy.rec_sz;
    fnames = cpy.fnames;
    base = new redis_recordset;
    base->set_names(cpy.base->get_names());
    base->data()[0] = const_cast<redis_cursor*>(&cpy)->base->data()[0];
    parent_ = base;
    iter_ = base->data().begin();
    update();
}

redis_cursor::~redis_cursor()
{
#ifdef _DEBUG
    iter_._Orphan_me();
#endif
    delete base;
}

redis_cursor& redis_cursor::operator++()
{
    if (rowid != -1) {
        int max_rowid = size();
        ++rowid;
        while (!is_valid_rowid(rowid) && rowid <= max_rowid) {
            ++rowid;
        }
        if (rowid > max_rowid) {
            rowid = -1;
        } else {
            // read values
            base->data()[0].resize(rec_sz);
            string row_id = boost::lexical_cast<string>(rowid);
            string val, tp;
            row_id = name_space + "." + row_id;
            vector<string> values;
            try {
                db_cli->lrange(row_id, 0, fnames.size(), values);
                for( size_t i = 0; i < fnames.size(); ++i) {
                    // assign value
                    try {
                        val = values[i];
                        size_t p = val.find(':');
                        string tps = "4";
                        if (p != string::npos) {
                            tps = val.substr(0, p);
                            val = val.substr(p+1);
                        }
                        int tp = boost::lexical_cast<int>(tps);
                        switch (tp) {
                        case 0:
                            base->data()[0][i] = boost::lexical_cast<char>(val);
                            break;
                        case 1:
                            base->data()[0][i] = boost::lexical_cast<int>(val);
                            break;
                        case 2:
                            base->data()[0][i] = boost::lexical_cast<bool>(val);
                            break;
                        case 3:
                            base->data()[0][i] = boost::lexical_cast<double>(val);
                            break;
                        case 4:
                            base->data()[0][i] = val;
                            break;
                        default:
                            base->data()[0][i] = boost::blank();
                            break;
                        }
                    } catch (out_of_range &) {
                        base->data()[0][i] = boost::blank();
                    }
                }
            } catch(redis::redis_error& e) {
                ostringstream ss(last_err);
                ss << "RedisDB can't get " << rowid << " - " << (string)e;
            }
            update();
        }
    }
    return *this;
}

bool redis_cursor::commit()
{
    string row_id = boost::lexical_cast<string>(rowid);
    row_id = name_space + "." + row_id;
    for (size_t i = 0; i < fnames.size(); i++) {
        try {
            string val = boost::lexical_cast<string>(base->data()[0][i].which()) + ":" + boost::lexical_cast<string>(base->data()[0][i]);
            try {
                db_cli->lset(row_id, i, val);
            } catch(redis::redis_error& e) {
                ostringstream ss(last_err);
                ss << "RedisDB can't save " << fnames[i] << " for RowID = " << rowid << " - " << (string)e;
            }
        } catch(out_of_range &) {
            db_cli->lset(row_id, i, "5:");
        }
    } // foreach field

    return false;
}

redis_cursor redis_cursor::begin( redis::client* dbcli, string nspace )
{
    return redis_cursor(dbcli, nspace, 0);
}

redis_cursor redis_cursor::end( redis::client* dbcli, string nspace )
{
    return redis_cursor(dbcli, nspace, -1);
}

redis_cursor& redis_cursor::operator=( const redis_cursor& cpy )
{
    db_cli = cpy.db_cli;
    name_space = cpy.name_space;
    rowid = cpy.rowid;
    rec_sz = cpy.rec_sz;
    fnames = cpy.fnames;
    base->set_names(cpy.base->get_names());
    base->data()[0] = const_cast<redis_cursor*>(&cpy)->base->data()[0];
    update();
    return *this;
}

const bool redis_cursor::operator==( const redis_cursor& rhs ) const
{
    bool retval;

    retval = (rowid == rhs.rowid && name_space == rhs.name_space);
    return retval;
}

const size_t redis_cursor::get_record_size()
{
    size_t ret = 0;

    try {
        ret = db_cli->llen(name_space + ".struct");
    } catch(redis::redis_error& e) {
        ostringstream ss(last_err);
        ss << "RedisDB can't get " << name_space << ".struct - " << (string)e;
    }
    return ret;
}

const bool redis_cursor::is_valid_rowid( size_t id ) const
{
    bool retval = false;
    vector<string> keys;
    string idn;

    try {
        idn = name_space + "." + boost::lexical_cast<string>(id);
        if (db_cli->exists(idn)) {
            retval = true;
        }
    } catch (redis::redis_error&) { }

    return retval;
}

const int redis_cursor::size()
{
    int ret = 0;
    string val;

    try {
        val = db_cli->get(name_space + ".rowid");
        ret = boost::lexical_cast<int>(val);
    } catch(redis::redis_error& e) {
        ret = -1;
        ostringstream ss(last_err);
        ss << "RedisDB can't get " << name_space << ".rowid - " << (string)e;
    }
    return ret;
}

// void redis_cursor::update()
// {
//     if (record.rec != NULL && record.free_) {
//         delete record.rec;
//         record.rec = NULL;
//     }
//     // else - just drop pointer to existing vector element
//     record.rec = &(*iter_);
//     record.free_ = false;
// }
