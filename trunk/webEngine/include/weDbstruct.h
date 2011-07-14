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
#ifndef __WEDBSTRUCT_H__
#define __WEDBSTRUCT_H__

#include "weVariant.h"
#include <boost/utility.hpp>
#include <set>
#include <boost/serialization/vector.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/scoped_ptr.hpp>

using namespace std;

namespace webEngine {

/**
 * @page db_layer Database Abstraction Layer
 *
 * Database abstraction layer is the set of classes to implement database-related
 * structures. It provides universal classes to access structured data.
 * List of classes:
 * @li db_cursor
 * @li db_condition
 * @li db_filter
 *
 * All of these classes except db_query may be used independently of webEngine system.
 * But the db_query class proposed to use with the i_storage interface implementations.
 * BOOST_SERIALIZATION_NVP serialization mechanism implemented in the db_recordset to
 * use in various schemes.
 *
 * Example Usage:
 * @code
 * db_cursor cur = kernel->storage()->get(
 *     db_condition().field("task_list.scan_id").operation(db_condition::equal).value(scan_id)
 *     && db_condition().field("task_list.request_type").operation(db_condition::not_equal).value(task_params_str),
 *     fields);
 * for(; cur.is_not_end(); ++cur) {
 *   cout << cur["task_list.request_type"] << "\t" << "task_list.request" << endl;
 * }
 * @endcode
 */

typedef vector< we_variant > record_;

namespace details {
class db_cursor_detail;
}
class db_cursor;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  db_record
///
/// @brief  set of fields as zero-based array of values.
///
/// db_record is the set of we_variant values. This is the internal class to store database
/// records as the zero-based array of columns.
///
/// @author A. Abramov
/// @date	09.06.2010
////////////////////////////////////////////////////////////////////////////////////////////////////
class db_record {
protected:
    db_record() : rec(new record_), free_(true) {}
    explicit db_record(const vector<string> &fields, record_* r = NULL);
    explicit db_record(boost::shared_ptr<vector<string> > fields, record_* r = NULL);
public:
    virtual ~db_record() {};

    /** @brief copying constructor */
    db_record(const db_record& r) : rec(NULL) {
        *this = r;
    }

    /** @brief copying operator */
    virtual db_record& operator=(const db_record& rhs);

    /** @{ @brief Accessing for db_record elements */
    virtual we_variant& operator[](int index);
    virtual const we_variant& operator[](int index) const;
    virtual we_variant& operator[](const string& name);
    virtual const we_variant& operator[](const string& name) const;
    /** @} */

    /** @brief Returns number of fields in record*/
    virtual const size_t record_size() const;
private:
    friend class details::db_cursor_detail;

    boost::shared_ptr<vector<string> > fields;
    boost::scoped_ptr<record_> rec;
    bool free_;
};

////////////////////////// CURSOR ////////////////////
namespace details {

/**
*  @brief This class uses for inheritance from a i_storage implementations
*/
class db_cursor_detail:	public boost::noncopyable {
public:
    db_cursor_detail():m_isEnd(true), m_affected_rows(0) {}
    explicit db_cursor_detail(const std::vector<std::string> &fields):m_isEnd(true), m_record(fields) {}
protected:
    friend class webEngine::db_cursor;
    virtual void increment() {}
    virtual db_record& dereference() const {
        if(m_isEnd)
            throw std::runtime_error("End of iterator");
        return *const_cast<db_record*>(&m_record);
    }
    virtual bool close() {
        bool result = true;
        if(!m_isEnd) {
            increment();
            result = !m_isEnd;
            m_isEnd = true;
        }
        return result;
    }
    virtual int get_affected_rows() {
        return m_affected_rows;
    }
    db_record m_record;
    bool m_isEnd;
    int m_affected_rows;
};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  db_cursor
///
/// @brief  iterator for sql statements created by i_storage
/// @see i_storage
///
/// @author S. Antonov
/// @date	14.09.2010
////////////////////////////////////////////////////////////////////////////////////////////////////
class db_cursor: public boost::iterator_facade <db_cursor, db_record, boost::forward_traversal_tag> {
public:
    db_cursor() {}
    db_cursor(const db_cursor &other):m_cursor(other.m_cursor) {}
    explicit db_cursor(boost::shared_ptr<details::db_cursor_detail> cursor):m_cursor(cursor) {}

    /** @brief After close() you can't use this cursor. It send all data and closes statements. */
    bool close() {
        return m_cursor->close();
    }

    /** @brief You can't use this cursor if it returns true. It indicates end of 'select' dataset, or closed cursor. */
    bool is_not_end() const {
        return m_cursor ? !m_cursor->m_isEnd : false;
    }

    /** @{ @brief Accessing for db_record elements */
    we_variant& operator[](int n) {
        return (m_cursor->dereference())[n];
    }
    const we_variant& operator[](int n) const {
        return (m_cursor->dereference())[n];
    }
    we_variant& operator[](const string& n) {
        return (m_cursor->dereference())[n];
    }
    const we_variant& operator[](const string& n) const {
        return (m_cursor->dereference())[n];
    }
    /** @} */

    /** @brief Return rows affected by last 'insert'/'update' query */
    int get_affected_rows() {
        return m_cursor->get_affected_rows();
    }

protected:
    friend class boost::iterator_core_access;
    virtual void increment() {
        m_cursor->increment();
        if(m_cursor->m_isEnd)
            m_cursor.reset();
    }
    virtual db_record& dereference() const {
        return m_cursor->dereference();
    }
    virtual bool equal(db_cursor const& other) const {
        return other.m_cursor == m_cursor;
    }
private:
    boost::shared_ptr<details::db_cursor_detail> m_cursor;
};

/// @class  db_filter_base
///
/// @brief  Abstract base class to construct logical queries.
///
/// @author A. Abramov
/// @date	15.06.2010
////////////////////////////////////////////////////////////////////////////////////////////////////
class db_filter;

class db_filter_base {
public:
    db_filter_base() {}
    virtual ~db_filter_base() {}

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual bool operator()(db_record& data) const
    ///
    /// @brief  eval synonym for using class as boolean predicate.
    ///
    /// Allows to use this class (and it's children) as predicate in standard operations from
    /// @b boost and @b std libraries
    ///
    /// @return The result of the operation.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual bool operator()(db_record&) const {
        return false;
    }
    /// generates string representation of the stored condition
    virtual string tostring() const {
        return "";
    }
    /// extract set of namepspaces from the condition fields
    virtual void get_namespaces(std::set<string>&) const {}

protected:
    friend class db_filter;
    /// create copy of the object for the internal usage
    virtual db_filter_base* clone() {
        return NULL;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  db_condition
///
/// @brief  atomic logical operation for DB queries.
///
/// @author A. Abramov
/// @date	09.06.2010
////////////////////////////////////////////////////////////////////////////////////////////////////
class db_condition : public db_filter_base {
public:
    typedef enum {
        equal,
        not_equal,
        less,
        great,
        less_or_equal,
        great_or_equal,
        like,
        is_null,
        not_null
    } opcode;
    db_condition();
    explicit db_condition(const string& s) {
        *this = s;
    }
    ~db_condition() {}

    /** @{ direct access to members */
    string& field() {
        return field_name;
    }
    opcode& operation() {
        return op_code;
    }
    we_variant& value() {
        return test_value;
    }
    /** @} */

    /** @{ @brief setters for nice syntax */
    db_condition& field(const string& field_) {
        field_name = field_;
        return *this;
    }
    db_condition& operation(const opcode op_) {
        op_code = op_;
        return *this;
    }
    db_condition& value(const we_variant& val) {
        test_value = val;
        return *this;
    }
    /** @} */

    /** @brief copying operator */
    db_condition& operator=(const db_condition& cpy) {
        field_name = cpy.field_name;
        op_code = cpy.op_code;
        test_value = cpy.test_value;
        return *this;
    }

    /** @brief string parser */
    db_condition& operator=(const string& s);

    /** @brief check db_record */
    virtual bool operator()(db_record& data) const;

    /** @brief print condition to string */
    virtual string tostring() const;

    /** @brief add namespace from field to ns_list */
    virtual void get_namespaces(std::set<string>& ns_list) const;

protected:
    virtual db_condition* clone();

    string field_name;
    opcode op_code;
    we_variant test_value;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  db_filter
///
/// @brief  logical set of conditions and filters
///
/// @author A. Abramov
/// @date	09.06.2010
////////////////////////////////////////////////////////////////////////////////////////////////////
class db_filter : public db_filter_base {
public:
    db_filter() {}
    db_filter(const db_filter& filt) {
        *this = filt;
    }
    db_filter(const db_condition& cond);

    ~db_filter();

    /** @brief copying operator */
    db_filter& operator=(const db_filter& cpy);

    /** @{ @brief logical operation on conditions */
    db_filter& set(db_filter_base& cond);
    db_filter& _or_(db_filter_base& cond);
    db_filter& _and_(db_filter_base& cond);
    /** @} */

    /** @brief test record */
    virtual bool operator()(db_record& data) const;

    /** @brief print condition to string */
    virtual string tostring() const;

    /** @brief add namespaces from field to ns_list */
    virtual void get_namespaces(std::set<string>& ns_list) const;

protected:
    virtual db_filter* clone();

    typedef enum {
        link_null,
        link_or,
        link_and
        //        link_not
    } link_code;
    typedef pair<link_code, db_filter_base*> element;

    vector< element > condition;
};

/// SQL generators from db_filter and fields list
namespace sql_constructor {
const std::string get_sql_select(const db_filter &where, const std::vector<std::string> &fields);
const std::string get_sql_update(const db_filter &where, const std::vector<std::string> &fields);
const std::string get_sql_insert(const std::vector<std::string> &fields);
const std::string get_sql_count(const db_filter &where, const std::vector<std::string> &fields);
const std::string get_sql_delete(const db_filter &where);
};

} // namespace webEngine

/** @{ @brief operator for nice syntax */
inline webEngine::db_filter& operator && (webEngine::db_filter& filt, webEngine::db_filter_base& cond)
{
    return filt._and_(cond);
}

inline webEngine::db_filter& operator || (webEngine::db_filter& filt, webEngine::db_filter_base& cond)
{
    return filt._or_(cond);
}

inline webEngine::db_filter operator && (webEngine::db_condition& filt, webEngine::db_filter_base& cond)
{
    return webEngine::db_filter(filt)._and_(cond);
}

inline webEngine::db_filter operator || (webEngine::db_condition& filt, webEngine::db_filter_base& cond)
{
    return webEngine::db_filter(filt)._or_(cond);
}
/** @} */


#endif // __WEDBSTRUCT_H__
