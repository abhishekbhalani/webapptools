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
#include <webEngine.h>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/regex.hpp>
#include <weDbstruct.h>

using namespace webEngine;
using namespace boost;

db_record::db_record( db_recordset* parent_, record_* r ) : parent(parent_), rec(r)
{
    if (parent != NULL && rec != NULL) {
        free_ = false;
        rec->resize(parent->record_size());
    }
    else {
        if (rec == NULL) {
            rec = new record_;
        }
        if (parent != NULL) {
            rec->resize(parent->record_size());
        }
        free_ = true;
    }
}

db_record::~db_record()
{
    if (rec != NULL && free_) {
        delete rec;
        rec = NULL;
    }
}

db_record& db_record::operator=( const db_record& rhs )
{
    if (rhs.rec != NULL)
    {   // make assign
        if (rec == NULL) {
            rec = new record_(*rhs.rec);
            free_ = true;
        }
        else {
            rec->assign(rhs.rec->begin(), rhs.rec->end());
        }
    }
    parent = rhs.parent;
    return *this;
}

we_variant& db_record::operator[]( int index )
{
    if (parent == NULL) {
        throw_exception(out_of_range("cursor not dereferencable: no recordset associated"));
    }
    if (rec == NULL) {
        throw_exception(out_of_range("cursor not dereferencable"));
    }
    return (*rec)[index];
}

const we_variant& db_record::operator[]( int index ) const
{
    if (parent == NULL) {
        throw_exception(out_of_range("cursor not dereferencable: no recordset associated"));
    }
    if (rec == NULL) {
        throw_exception(out_of_range("cursor not dereferencable"));
    }
    return (*rec)[index];
}

we_variant& db_record::operator[]( const string& name )
{
    int index = 0;

    if (parent == NULL) {
        // throw exception
        throw_exception(out_of_range("cursor not dereferencable: no recordset associated"));
    }
    vector<string>::iterator fn_it = std::find(parent->field_names.begin(), parent->field_names.end(), name);
    if (fn_it == parent->field_names.end())
    { // throw exception
        throw_exception(out_of_range("cursor not dereferencable: field name found - " + name));
    }
    else {
        index = std::distance(parent->field_names.begin(), fn_it);
    }
    if (rec == NULL) {
        throw_exception(out_of_range("cursor not dereferencable"));
    }
    return (*rec)[index];
}

const we_variant& db_record::operator[]( const string& name ) const
{
    int index = 0;

    if (parent == NULL) {
        // throw exception
        throw_exception(out_of_range("cursor not dereferencable: no recordset associated"));
    }
    vector<string>::iterator fn_it = std::find(parent->field_names.begin(), parent->field_names.end(), name);
    if (fn_it == parent->field_names.end())
    { // throw exception
        throw_exception(out_of_range("cursor not dereferencable: field name found - " + name));
    }
    else {
        index = std::distance(parent->field_names.begin(), fn_it);
    }
    if (rec == NULL) {
        throw_exception(out_of_range("cursor not dereferencable"));
    }
    return (*rec)[index];
}

db_cursor_base::db_cursor_base( db_recordset* rs, base_iter& it ) :
    parent_(rs), record(parent_, NULL), iter_(it)
{
    if (parent_->is_valid_iter(it)) {
        if (record.rec != NULL && record.free_) {
            delete record.rec;
            record.rec = NULL;
        }
        update();
    }
}

db_recordset::db_recordset(void)
{

}

db_recordset::db_recordset(const vector<string>& fld_names)
{
    field_names = fld_names;
}

db_recordset::~db_recordset(void)
{

}

db_cursor db_recordset::erase(db_cursor& at)
{
    return db_cursor(this, records.erase(at.iter_));
}

// db_cursor db_recordset::push_back(db_record& rec)
// {
//     // @todo Verify data types
//     size_t sz = records.size();
// 
//     rec.resize(field_names.size());
//     records.push_back(rec);
// 
//     // get the iterator points to the first added element
//     vector<db_record>::iterator it = records.begin();
//     return db_cursor(this, it + sz);
// }

db_cursor db_recordset::push_back(size_t num/* = 1*/)
{
    // @todo Verify data types
    size_t sz = records.size();

    if (num > 0) {
        record_ rec;
        rec.resize(field_names.size());
        records.insert(records.end(), num, rec);
    }
    else {
        // to point to last element
        sz--;
    }

    // get the iterator points to the last element
    recordset_::iterator it = records.begin();
    return db_cursor(this, it + sz);
}

db_cursor db_recordset::insert(db_cursor& before, size_t num/* = 1*/)
{
    size_t off = records.size() == 0 ? 0 : before - records.begin();

    if (num > 0) {
        record_ rec;
        rec.resize(field_names.size());
        records.insert(before.iter_, num, rec);
    }
    return db_cursor(this, records.begin() + off);
}

// db_cursor db_recordset::insert(db_cursor& before, db_record& rec)
// {
//     size_t off = records.size() == 0 ? 0 : before - records.begin();
// 
//     rec.resize(field_names.size());
//     records.insert(before, rec);
// 
//     return db_cursor(this, records.begin() + off);
// }

we_variant& db_cursor_base::operator[]( string name )
{
    int index = 0;

    if (parent_ == NULL) {
        // throw exception
        throw_exception(out_of_range("cursor not dereferencable: no recordset associated"));
    }
    vector<string>::iterator fn_it = std::find(parent_->field_names.begin(), parent_->field_names.end(), name);
    if (fn_it == parent_->field_names.end())
    { // throw exception
        throw_exception(out_of_range("cursor not dereferencable: field name found - " + name));
    }
    else {
        index = std::distance(parent_->field_names.begin(), fn_it);
    }
    if (iter_ == parent_->records.end()) {
        throw_exception(out_of_range("cursor not dereferencable"));
    }
    return (*iter_)[index];
}

const we_variant& db_cursor_base::operator[]( string name ) const
{
    int index = 0;

    if (parent_ == NULL) {
        // throw exception
        throw_exception(out_of_range("cursor not dereferencable: no recordset associated"));
    }
    vector<string>::iterator fn_it = std::find(parent_->field_names.begin(), parent_->field_names.end(), name);
    if (fn_it == parent_->field_names.end())
    { // throw exception
        throw_exception(out_of_range("cursor not dereferencable: field name found - " + name));
    }
    else {
        index = std::distance(parent_->field_names.begin(), fn_it);
    }
    if (iter_ == parent_->records.end()) {
        throw_exception(out_of_range("cursor not dereferencable"));
    }
    return (*iter_)[index];
}

we_variant& db_cursor_base::operator[]( int index )
{
    if (parent_ == NULL) {
        throw_exception(out_of_range("cursor not dereferencable: no recordset associated"));
    }
    if (iter_ == parent_->records.end()) {
        throw_exception(out_of_range("cursor not dereferencable"));
    }
    return (*iter_)[index];
}

const we_variant& db_cursor_base::operator[]( int index ) const
{
    if (parent_ == NULL) {
        throw_exception(out_of_range("cursor not dereferencable: no recordset associated"));
    }
    if (iter_ == parent_->records.end()) {
        throw_exception(out_of_range("cursor not dereferencable"));
    }
    return (*iter_)[index];
}

const size_t db_cursor_base::record_size()
{
    if (parent_ == NULL) {
        throw_exception(out_of_range("cursor not dereferencable: no recordset associated"));
    }
    return parent_->record_size();
}

void db_cursor_base::update()
{
    if (parent_->is_valid_iter(iter_)) {
        if (record.rec != NULL && record.free_) {
            delete record.rec;
            record.rec = NULL;
        }
        // else - just drop pointer to existing vector element
        record.parent = parent_;
        record.rec = &(*iter_);
        record.free_ = false;
    }
}

void db_cursor_base::clear()
{
    if (record.rec != NULL && record.free_) {
        delete record.rec;
        record.rec = NULL;
    }
    // else - just drop pointer to existing vector element
    record.rec = new record_;
    record.parent = parent_;
    if (parent_ != NULL) {
        record.rec->resize(parent_->record_size());
    }
    record.free_ = true;
}

const bool db_cursor_base::operator==( const db_cursor_base& rhs ) const
{
    if ( parent_ != rhs.parent_ ) {
        return false;
    }
    if (iter_ != rhs.iter_) {
        return false;
    }
    return true;
}

db_condition::db_condition()
{
    field_name = "";
    op_code = db_condition::equal;
    test_value.clear();
}

db_condition& db_condition::operator=( const string& s )
{
    string tmp;
    // ([^=\<\>!\s]+)\s+([\S]+)\s+(.*?)\s*$
    boost::regex re("([^=\\<\\>!\\s]+)\\s+([\\S]+)\\s+(.*?)\\s*$");
    boost::smatch mres;

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

    return *this;
}

bool db_condition::operator()(db_record& data) const
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
            result = (val >= test_value);
            break;
        case db_condition::like:
            if (val.type() != typeid(string) || test_value.type() != typeid(string)) {
                s_t  = "db_condition::eval::like (";
                s_t += tostring();
                s_t += ") - not a string values";
                throw_exception(bad_cast(s_t.c_str()));
            }
            s_t = boost::get<string>( (we_types&)test_value );
            s_a = val.get<string>();
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

string db_condition::tostring( ) const
{
    string res, op, val;

    res = field_name;
    op = "";
    val = boost::lexical_cast<string>((we_types&)test_value);
    switch(op_code) {
        case db_condition::equal:
            op = " == ";
            break;
        case db_condition::not_equal:
            op = " != ";
            break;
        case db_condition::less:
            op = " < ";
            break;
        case db_condition::great:
            op = " > ";
            break;
        case db_condition::less_or_equal:
            op = " <= ";
            break;
        case db_condition::great_or_equal:
            op = " >= ";
            break;
        case db_condition::like:
            op = " like ";
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

db_condition* db_condition::clone()
{
    return new db_condition(*this);
}

// db_filter::db_filter( const db_filter& filt )
// {
//     db_filter *elem = new db_filter();
//     *elem = filt;
//     condition.clear();
//     condition.push_back(element(link_null, elem));
// }

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
        condition.push_back(element(cpy.condition[i].first, cpy.condition[i].second->clone()));
    }
    return *this;
}

db_filter& db_filter::set( db_filter_base& cond )
{
    db_filter_base *elem = cond.clone();
    condition.clear();
    condition.push_back(element(link_null, elem));
    return *this;
}

db_filter& db_filter::or( db_filter_base& cond )
{
    db_filter_base *elem = cond.clone();
    condition.push_back(element(link_or, elem));
    return *this;
}

db_filter& db_filter::and( db_filter_base& cond )
{
    db_filter_base *elem = cond.clone();
    condition.push_back(element(link_and, elem));
    return *this;
}

bool db_filter::operator()(db_record& data) const
{
    bool result = false;

    if (condition.size() > 0) {
        result = (*condition[0].second)(data);
        for (size_t i = 1; i < condition.size(); i++) {
            if (condition[i].first == link_or) {
                if (!result) {
                    // evaluate condition only if need
                    result = result || (*condition[i].second)(data);
                }
            } // if link_or
            if (condition[i].first == link_and) {
                if (result) {
                    // evaluate condition only if need
                    result = result && (*condition[i].second)(data);
                }
            } // if link_and
        } // for all conditions
    } // if need to evaluate

    return result;
}

string db_filter::tostring() const
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

db_filter* db_filter::clone()
{
    return new db_filter(*this);
}

void db_filter::get_namespaces( std::set<string>& ns_list )
{
    for (size_t i = 0; i < condition.size(); i++) {
        condition[i].second->get_namespaces(ns_list);
    }
}
