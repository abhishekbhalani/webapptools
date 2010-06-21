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
#include <webEngine.h>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/regex.hpp>
#include <weDbstruct.h>

using namespace webEngine;
using namespace boost;

db_recordset::db_recordset(void)
{

}

db_recordset::db_recordset(vector<string> fld_names)
{
    field_names.assign(fld_names.begin(), fld_names.end());
}

db_recordset::~db_recordset(void)
{

}

void db_recordset::erase(db_cursor& at)
{
    records.erase(static_cast< vector<db_record>::iterator >(at));
}

db_cursor db_recordset::push_back(db_record& rec)
{
    // @todo Verify data types
    size_t sz = records.size();

    rec.resize(field_names.size());
    records.push_back(rec);

    // get the iterator points to the first added element
    vector<db_record>::iterator it = records.begin();
    return db_cursor(this, it + sz);
}

db_cursor db_recordset::push_back(size_t num/* = 1*/)
{
    // @todo Verify data types
    size_t sz = records.size();

    if (num > 0) {
        db_record rec;
        rec.resize(field_names.size());
        records.insert(records.end(), num, rec);
    }
    else {
        // to point to last element
        sz--;
    }

    // get the iterator points to the last element
    vector<db_record>::iterator it = records.begin();
    return db_cursor(this, it + sz);
}

db_cursor db_recordset::insert(db_cursor& before, size_t num/* = 1*/)
{
    size_t off = records.size() == 0 ? 0 : before - records.begin();

    if (num > 0) {
        db_record rec;
        rec.resize(field_names.size());
        records.insert(before, num, rec);
    }
    return db_cursor(this, records.begin() + off);
}

db_cursor db_recordset::insert(db_cursor& before, db_record& rec)
{
    size_t off = records.size() == 0 ? 0 : before - records.begin();

    rec.resize(field_names.size());
    records.insert(before, rec);

    return db_cursor(this, records.begin() + off);
}

db_cursor db_recordset::begin()
{
    return db_cursor(this, records.begin());
}

db_cursor db_recordset::end()
{
    return db_cursor(this, records.end());
}

db_cursor::db_cursor( db_recordset* rs, vector<db_record>::iterator it )
{
    parent = rs;

    if (parent != NULL) {
        vector<db_record>::iterator::operator=(it);
    }
}

db_cursor::db_cursor( const db_cursor& cp )
{
    parent = cp.parent;
    vector<db_record>::iterator::operator=(static_cast< vector<db_record>::iterator >(cp));
}

db_cursor& db_cursor::operator=( const db_cursor& cp )
{
    parent = cp.parent;
    vector<db_record>::iterator::operator=(static_cast< vector<db_record>::iterator >(cp));
    return *this;
}

we_variant& db_cursor::operator[]( string name )
{
    int index = 0;

    if (parent == NULL) {
        // throw exception
        throw_exception(out_of_range("cursor not dereferencable: no recordset associated"));
    }
    vector<string>::iterator fn_it;
    for (fn_it = parent->field_names.begin(); fn_it != parent->field_names.end(); fn_it++) {
        if (*fn_it == name) {
            break;
        }
        index++;
    }
    if (fn_it == parent->field_names.end())
    {
        // throw exception
        throw_exception(out_of_range("cursor not dereferencable: field name found"));
    }
    if (static_cast< vector<db_record>::iterator >(*this) == parent->records.end()) {
        throw_exception(out_of_range("cursor not dereferencable"));
    }
    return (*operator->())[index];
}

we_variant& db_cursor::operator[]( int index )
{
    if (parent == NULL) {
        throw_exception(out_of_range("cursor not dereferencable: no recordset associated"));
    }
    if (static_cast< vector<db_record>::iterator >(*this) == parent->records.end()) {
        throw_exception(out_of_range("cursor not dereferencable"));
    }
    return (*operator->())[index];
}

const size_t db_cursor::record_size()
{
    if (parent == NULL) {
        throw_exception(out_of_range("cursor not dereferencable: no recordset associated"));
    }
    return parent->record_size();
}

db_condition::db_condition()
{
    field_name = "";
    op_code = db_condition::equal;
    test_value.clear();
}

db_condition::db_condition( const db_condition& c )
{
    field_name = c.field_name;
    op_code = c.op_code;
    test_value = c.test_value;
}

db_condition::db_condition( string s )
{
    string tmp;
    // ([^=\<\>!\s]+)\s+([\S]+)\s+(.*?)\s*$
    boost::regex re("([^=\\<\\>!\\s]+)\\s+([\\S]+)\\s+(.*?)\\s*$");
    boost::smatch mres;
    /*char cv;
    unsigned char ucv;
    int iv;
    unsigned int uiv;
    long lv;
    unsigned long ulv;
    bool bv;
    double dv;*/

    if (regex_match(s, mres, re, match_default)) {
        field_name = mres[1];
        tmp = mres[2];
        if (tmp == "==") {
            op_code = db_condition::equal;
        }
        else if (tmp == "<") {
            op_code = db_condition::less;
        }
        else if (tmp == ">") {
            op_code = db_condition::great;
        }
        else if (tmp == "<=") {
            op_code = db_condition::less_or_equal;
        }
        else if (tmp == ">=") {
            op_code = db_condition::great_or_equal;
        }
        else if (iequals(tmp, "like")) {
            op_code = db_condition::like;
        }
        else if (iequals(tmp, "is_null")) {
            op_code = db_condition::is_null;
        }
        else if (iequals(tmp, "not_null")) {
            op_code = db_condition::not_null;
        }
        else {
            // throw exception
            tmp = "db_condition: can't recognize condition - " + tmp;
            throw_exception(bad_cast(tmp.c_str()));
        }
        tmp = mres[3];
        if (op_code != db_condition::is_null && op_code != db_condition::not_null) {
            // try to cast string to value
            if (iequals(tmp, "true")) {
                // boolean TRUE
                test_value = true;
            }
            else if (iequals(tmp, "false")) {
                test_value = false;
            }
            else if (all(tmp.c_str(), is_digit())) {
                // int, uint, long, ulong ... hmm how determine?
                int iv;
                try{
                    iv = boost::lexical_cast<int>(tmp);
                    test_value = iv;
                }
                catch(bad_cast &) {
                    // fail - convert to string
                    test_value = tmp;
                }
            }
            else if (all(tmp.c_str(), is_digit() || is_any_of("."))) {
                // double value
                double dv;
                try{
                    dv = boost::lexical_cast<double>(tmp);
                    test_value = dv;
                }
                catch(bad_cast &) {
                    // fail - convert to string
                    test_value = tmp;
                }
            }
            //             else if (tmp.length() == 1) {
            //                 // char value
            //                 char ch = tmp[0];
            //                 test_value = ch;
            //             }
            else {
                // string
                test_value = tmp;
            }
        } // if op_code no is_null nor not_null
    }
    else {
        // throw exception
        tmp = "db_condition: can't parse string - " + s;
        throw_exception(bad_cast(tmp.c_str()));
    }
}

db_condition& db_condition::operator=( db_condition& c )
{
    field_name = c.field_name;
    op_code = c.op_code;
    test_value = c.test_value;

    return *this;
}

bool db_condition::eval( db_cursor& data )
{
    bool result = false;
    we_variant val;
    string s_t, s_a;
    size_t pos;

    val = data[field_name];
    if (val.which() != test_value.which() && 
        (op_code != db_condition::is_null && op_code != db_condition::not_null)) {
            return false;
    }
    switch(op_code) {
        case db_condition::equal:
            result = (test_value == val);
            break;
        case db_condition::not_equal:
            result = !(test_value == val);
            break;
        case db_condition::less:
            result = (val < test_value);
            break;
        case db_condition::great:
            result = (val > test_value);
            break;
        case db_condition::less_or_equal:
            result = (val <= test_value);
            break;
        case db_condition::great_or_equal:
            result = (val <= test_value);
            break;
        case db_condition::like:
            if (val.type() != typeid(string) || test_value.type() != typeid(string)) {
                s_t  = "db_condition::eval::like (";
                s_t += tostring();
                s_t += ") - not a string values";
                throw_exception(bad_cast(s_t.c_str()));
            }
            s_t = boost::get<string>(test_value);
            s_a = boost::get<string>(val);
            pos = s_a.find(s_t);
            result = (pos != string::npos);
            break;
        case db_condition::is_null:
            result = val.empty();
            break;
        case db_condition::not_null:
            result = !val.empty();
            break;
    }
    return result;
}

string db_condition::tostring( )
{
    string res, op, val;

    res = field_name;
    op = "";
    val = "";
    switch(op_code) {
        case db_condition::equal:
            op = " == ";
            val = boost::lexical_cast<string>(test_value);
            break;
        case db_condition::not_equal:
            op = " != ";
            val = boost::lexical_cast<string>(test_value);
            break;
        case db_condition::less:
            op = " < ";
            val = boost::lexical_cast<string>(test_value);
            break;
        case db_condition::great:
            op = " > ";
            val = boost::lexical_cast<string>(test_value);
            break;
        case db_condition::less_or_equal:
            op = " <= ";
            val = boost::lexical_cast<string>(test_value);
            break;
        case db_condition::great_or_equal:
            op = " >= ";
            val = boost::lexical_cast<string>(test_value);
            break;
        case db_condition::like:
            op = " like ";
            val = boost::lexical_cast<string>(test_value);
            break;
        case db_condition::is_null:
            op = " is_null";
            val = "";
            break;
        case db_condition::not_null:
            op = " not_null";
            val = "";
            break;
    }
    res += op;
    res += val;
    return res;
}

void db_condition::get_namespaces( std::set<string>& ns_list )
{
    string name = field_name;
    size_t pos = name.find_last_of('.');
    if (pos != string::npos) {
        name = name.substr(0, pos);
    }
    else {
        name = "";
    }
    ns_list.insert(name);
}

db_filter_base* db_condition::copy()
{
    db_condition* cpy = new db_condition(*this);
    return cpy;
}

db_filter::db_filter( const db_filter& filt )
{
    db_filter *elem = new db_filter();
    *elem = filt;
    condition.clear();
    condition.push_back(element(link_null, elem));
}

db_filter::db_filter( const db_condition& cond )
{
    db_condition *elem = new db_condition(cond);
    condition.clear();
    condition.push_back(element(link_null, elem));
}

db_filter::~db_filter()
{
    for (size_t i = 0; i < condition.size(); i++) {
        delete condition[i].second;
    }
}

db_filter& db_filter::operator=( const db_filter& cpy )
{
    condition.clear();
    for (size_t i = 0 ; i < cpy.condition.size(); i++) {
        condition.push_back(element(cpy.condition[i].first, cpy.condition[i].second->copy()));
    }
    return *this;
}

db_filter& db_filter::set( db_condition& cond )
{
    db_condition *elem = new db_condition(cond);
    condition.clear();
    condition.push_back(element(link_null, elem));
    return *this;
}

db_filter& db_filter::set( db_filter& cond )
{
    db_filter *elem = new db_filter();
    *elem = cond;
    condition.clear();
    condition.push_back(element(link_null, elem));
    return *this;
}

db_filter& db_filter::or( db_condition& cond )
{
    db_condition *elem = new db_condition(cond);
    condition.push_back(element(link_or, elem));
    return *this;
}

db_filter& db_filter::or( db_filter& cond )
{
    db_filter *elem = new db_filter();
    *elem = cond;
    condition.push_back(element(link_or, elem));
    return *this;
}

db_filter& db_filter::and( db_condition& cond )
{
    db_condition *elem = new db_condition(cond);
    condition.push_back(element(link_and, elem));
    return *this;
}

db_filter& db_filter::and( db_filter& cond )
{
    db_filter *elem = new db_filter();
    *elem = cond;
    condition.push_back(element(link_and, elem));
    return *this;
}

bool db_filter::eval( db_cursor& data )
{
    bool result = false;

    if (condition.size() > 0) {
        result = condition[0].second->eval(data);
        for (size_t i = 1; i < condition.size(); i++) {
            if (condition[i].first == link_or) {
                if (!result) {
                    // evaluate condition only if need
                    result = result || condition[i].second->eval(data);
                }
            } // if link_or
            if (condition[i].first == link_and) {
                if (result) {
                    // evaluate condition only if need
                    result = result && condition[i].second->eval(data);
                }
            } // if link_and
        } // for all conditions
    } // if need to evaluate

    return result;
}

string db_filter::tostring()
{
    string res = "";

    if (condition.size() > 0) {
        res = "( " + condition[0].second->tostring();
        for (size_t i = 1; i < condition.size(); i++) {
            switch (condition[i].first) {
            case db_filter::link_or:
                res += " OR ";
                break;
            case db_filter::link_and:
                res += " AND ";
                break;
                //             case db_filter::link_not:
                //                 res += " NOT ";
                //                 break;
            default:
                // something strange!!!
                break;
            }
            res += condition[i].second->tostring();
        }
        res += " )";
    }

    return res;
}

db_filter_base* db_filter::copy()
{
    db_filter* cpy = new db_filter();
    *cpy = *this;
    return cpy;
}

void db_filter::get_namespaces( std::set<string>& ns_list )
{
    for (size_t i = 0; i < condition.size(); i++) {
        condition[i].second->get_namespaces(ns_list);
    }
}

