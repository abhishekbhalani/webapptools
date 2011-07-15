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
#include <set>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/regex.hpp>
#include <weDbstruct.h>

using namespace webEngine;
using namespace boost;

db_record::db_record(const vector<string> &fields_, record_* r)
{
    rec.reset(r);
    fields = boost::shared_ptr< vector<string> >(new vector<string>(fields_) );
    if (fields && rec) {
        free_ = false;
        rec->resize(fields->size());
    } else {
        if (!rec) {
            rec.reset(new record_);
        }
        if (fields) {
            rec->resize(fields->size());
        }
        free_ = true;
    }
}

db_record::db_record( boost::shared_ptr<std::vector<std::string> > fields_, record_* r ) : fields(fields_)
{
    rec.reset(r);
    if (fields && rec) {
        free_ = false;
        rec->resize(fields->size());
    } else {
        if (!rec) {
            rec.reset(new record_);
        }
        if (fields) {
            rec->resize(fields->size());
        }
        free_ = true;
    }
}

db_record& db_record::operator=( const db_record& rhs )
{
    if (rhs.rec) {
        // make assign
        if (!rec) {
            rec.reset(new record_(*rhs.rec));
            free_ = true;
        } else {
            rec->assign(rhs.rec->begin(), rhs.rec->end());
        }
    }
    fields = rhs.fields;
    return *this;
}

we_variant& db_record::operator[]( int index )
{
    if (!fields) {
        throw_exception(out_of_range("cursor not dereferencable: no recordset associated"));
    }
    if (rec == NULL) {
        throw_exception(out_of_range("cursor not dereferencable"));
    }
    return (*rec)[index];
}

const we_variant& db_record::operator[]( int index ) const
{
    if (!fields) {
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

    if (!fields) {
        // throw exception
        throw_exception(out_of_range("cursor not dereferencable: no recordset associated"));
    }
    vector<string>::iterator fn_it = std::find(fields->begin(), fields->end(), name);
    if (fn_it == fields->end()) {
        // throw exception
        throw_exception(out_of_range("cursor not dereferencable: field name found - " + name));
    } else {
        index = std::distance(fields->begin(), fn_it);
    }
    if (rec == NULL) {
        throw_exception(out_of_range("cursor not dereferencable"));
    }
    return (*rec)[index];
}

const we_variant& db_record::operator[]( const string& name ) const
{
    int index = 0;

    if (!fields) {
        // throw exception
        throw_exception(out_of_range("cursor not dereferencable: no recordset associated"));
    }
    vector<string>::iterator fn_it = std::find(fields->begin(), fields->end(), name);
    if (fn_it == fields->end()) {
        // throw exception
        throw_exception(out_of_range("cursor not dereferencable: field name found - " + name));
    } else {
        index = std::distance(fields->begin(), fn_it);
    }
    if (rec == NULL) {
        throw_exception(out_of_range("cursor not dereferencable"));
    }
    return (*rec)[index];
}

const size_t db_record::record_size() const
{
    return fields ? fields->size() : 0;
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
        } else if (tmp == "<") {
            op_code = db_condition::less;
        } else if (tmp == ">") {
            op_code = db_condition::great;
        } else if (tmp == "<=") {
            op_code = db_condition::less_or_equal;
        } else if (tmp == ">=") {
            op_code = db_condition::great_or_equal;
        } else if (iequals(tmp, "like")) {
            op_code = db_condition::like;
        } else if (iequals(tmp, "is_null")) {
            op_code = db_condition::is_null;
        } else if (iequals(tmp, "not_null")) {
            op_code = db_condition::not_null;
        } else {
            // throw exception
            tmp = "db_condition: can't recognize condition - " + tmp;
            throw_exception(runtime_error(tmp.c_str()));
        }
        tmp = mres[3];
        if (op_code != db_condition::is_null && op_code != db_condition::not_null) {
            // try to cast string to value
            if (iequals(tmp, "true")) {
                // boolean TRUE
                test_value = true;
            } else if (iequals(tmp, "false")) {
                test_value = false;
            } else if (all(tmp.c_str(), is_digit())) {
                // int, uint, long, ulong ... hmm how determine?
                int iv;
                try {
                    iv = boost::lexical_cast<int>(tmp);
                    test_value = iv;
                } catch(bad_cast &) {
                    // fail - convert to string
                    test_value = tmp;
                }
            } else if (all(tmp.c_str(), is_digit() || is_any_of("."))) {
                // double value
                double dv;
                try {
                    dv = boost::lexical_cast<double>(tmp);
                    test_value = dv;
                } catch(bad_cast &) {
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
    } else {
        // throw exception
        tmp = "db_condition: can't parse string - " + s;
        throw_exception(runtime_error(tmp.c_str()));
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
            throw_exception(runtime_error(s_t.c_str()));
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
        op = " is null";
        val = "";
        break;
    case db_condition::not_null:
        op = " not null";
        val = "";
        break;
    }
    if ( test_value.which() == 4) {
        // string value
        boost::replace_all(val, "'", "''");
        // surround by '
        val = "'" + val + "'";
    }
    res += op;
    res += val;
    return res;
}

namespace {
void get_namespaces( const string& str, std::set<string>& ns_list )
{
    size_t pos = str.find_last_of('.');
    if (pos != string::npos) {
        ns_list.insert(str.substr(0, pos));
    }
}
void get_namespaces( const vector<string>& fields, std::set<string>& ns_list )
{
    for(vector<string>::const_iterator it = fields.begin(); it!= fields.end(); ++it)
        get_namespaces(*it, ns_list);
}
}

void db_condition::get_namespaces( std::set<string>& ns_list ) const
{
    ::get_namespaces(field_name, ns_list);
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

db_filter& db_filter::_or_( db_filter_base& cond )
{
    db_filter_base *elem = cond.clone();
    condition.push_back(element(link_or, elem));
    return *this;
}

db_filter& db_filter::_and_( db_filter_base& cond )
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

void db_filter::get_namespaces( std::set<string>& ns_list ) const
{
    for (size_t i = 0; i < condition.size(); i++) {
        condition[i].second->get_namespaces(ns_list);
    }
}

const std::string sql_constructor::get_sql_select(const db_filter &where, const std::vector<std::string> &fields)
{
    string qstr;
    size_t i;
    std::set<string>::const_iterator ns_it;
    if(fields.size()) {
        qstr = "SELECT ";
        for (i = 0; i < fields.size(); ++i) {
            if (i > 0) qstr += ", ";
            qstr += fields.at(i);
        }
        qstr += " FROM ";
        std::set<string> ns_list;
        where.get_namespaces(ns_list);
        ::get_namespaces(fields, ns_list);
        for (ns_it = ns_list.begin(), i = 0; ns_it != ns_list.end(); ++ns_it, ++i) {
            if (i > 0) qstr += ", ";
            qstr += "[" + *ns_it + "]";
        }
        string qwhere(where.tostring());
        if(qwhere.size()>0)
            qstr += " WHERE " + qwhere;
    }
    return qstr;
}

const std::string sql_constructor::get_sql_update(const db_filter &where, const std::vector<std::string> &fields)
{
    string qstr;
    if (fields.size()) {
        string table_name = fields[0];
        size_t p = table_name.find_last_of('.');
        if (p != string::npos) {
            table_name = table_name.substr(0, p);
            qstr = "UPDATE [" + table_name + "] SET ";
            for (size_t i = 0; i < fields.size(); ++i) {
                if ( i > 0 ) {
                    qstr += ", ";
                }
                table_name = fields[i];
                p = table_name.find_last_of('.');
                if (p != string::npos) {
                    table_name = table_name.substr(p + 1);
                }
                qstr += table_name + "=?";
            }
            string qwhere(where.tostring());
            if(qwhere.size()>0)
                qstr += " WHERE " + qwhere;
        }
    }
    return qstr;
}

const std::string sql_constructor::get_sql_insert(const std::vector<std::string> &fields)
{
    string qstr;
    if (fields.size()) {
        string table_name = fields[0];
        size_t p = table_name.find_last_of('.');
        if (p != string::npos) {
            table_name = table_name.substr(0, p);
            qstr = "INSERT INTO " + table_name + " (";
            string binder;
            for ( size_t i = 0; i < fields.size(); ++i) {
                if ( i > 0 ) {
                    qstr += ", ";
                    binder += ", ";
                }
                table_name = fields[i];
                p = table_name.find_last_of('.');
                if (p != string::npos) {
                    table_name = table_name.substr(p + 1);
                }
                qstr += table_name;
                binder += "?";
            }
            qstr += ") VALUES (" + binder + ")";
        }
    }
    return qstr;
}

const std::string sql_constructor::get_sql_count(const db_filter &where, const std::vector<std::string> &fields)
{
    if (fields.size()) {
        string table_name = fields[0];
        size_t p = table_name.find_last_of('.');
        if (p != string::npos) {
            table_name = table_name.substr(0, p);
            return "SELECT COUNT(1) FROM " + table_name + " WHERE " + where.tostring();
        }
    }
    return string();
}

const std::string sql_constructor::get_sql_delete(const db_filter &where)
{
    string query;
    std::set<string> ns_list;
    where.get_namespaces(ns_list);
    for (std::set<string>::const_iterator ns_it = ns_list.begin(); ns_it != ns_list.end(); ++ns_it) {
        query += "DELETE FROM [" + *ns_it + "] ";
        string qwhere(where.tostring());
        if(qwhere.size()>0)
            query += " WHERE " + qwhere;
        query += "; ";
    }
    return query;
}

