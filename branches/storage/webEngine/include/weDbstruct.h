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
#ifndef __WEDBSTRUCT_H__
#define __WEDBSTRUCT_H__

#include <weVariant.h>
#include <boost/utility.hpp>
#include <set>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/weak_ptr.hpp>

using namespace std;

namespace webEngine {

	/**
	 * @page db_layer Database Abstraction Layer
	 *
	 * Database abstraction layer is the set of classes to implement database-related
	 * structures. It provides universal classes to access structured data.
	 * List of classes:
	 * @li db_record
	 * @li db_recordset
	 * @li db_cursor
	 * @li db_condition
	 * @li db_filter
	 * @li db_query
	 *
	 * All of these classes except db_query may be used independently of webEngine system.
	 * But the db_query class proposed to use with the i_storage interface implementations.
	 * BOOST_SERIALIZATION_NVP serialization mechanism implemented in the db_record and
	 * db_recordset to use in various schemes.
	 */

    class db_cursor_base;
    class db_recordset;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  db_record
    ///
    /// @brief  set of fields as zero-based array of values.
    ///
    /// db_record is the set of we_variant values. This is the internal class to store database
    /// records as the zero-based array of columns. This class implements serialization
    /// to allow db_recordset class to be serialized.
    ///
    /// @author A. Abramov
    /// @date	09.06.2010
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class db_record
    {
    public:
        // all defaults
    protected:
        friend class db_cursor_base;
        friend class db_recordset;

        we_variant& operator[](size_t n) { return data[n]; }
        we_variant const& operator[](size_t n) const { return data[n]; }
        void push_back(we_variant val) { data.push_back(val); }
        void resize(size_t n) { data.resize(n); }

        friend class boost::serialization::access;
        /// serialize instance to the given archive
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) { 
            ar & BOOST_SERIALIZATION_NVP(data);
        }
    private:
        vector< we_variant > data;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  db_cursor_base
    ///
    /// @brief  Base class for all database cursors.
    ///
    /// This class defines iterator-like semantics for access to database recordset.
    /// It hides direct access to db_record, but allows named or index-based access to record's
    /// fields. This class provides forward-only access to db-recordset.
    ///
    /// @author A. Abramov
    /// @date   25.06.2010
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class db_cursor_base
    {
    public:
        typedef std::vector< db_record >::difference_type diff_type;
        typedef std::vector< db_record >::iterator base_iter;

        /// default constructor
        db_cursor_base() : iter_(), parent(NULL) {}
        /// constructor for use inside db_recordset
        db_cursor_base(db_recordset* rs, base_iter& it) : iter_(it), parent(rs) {}
        virtual ~db_cursor_base() {}

        /// prefix increment to go through db_recordset
        db_cursor_base& operator++() { ++iter_; return *this; }
        /// calculate distance between two cursors
        const diff_type operator-(const db_cursor_base& rit) const { return (iter_ - rit.iter_); }
        /// calculate distance between cursor and native iterator
        const diff_type operator-(const base_iter& brit) const { return (iter_ - brit); }

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @fn we_variant& operator[](string name)
        ///
        /// @brief  Access to named field. 
        ///
        /// @author A. Abramov
        /// @date   25.06.2010
        ///
        /// @return we_variant - value of fields
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        we_variant& operator[](string name);
        const we_variant& operator[](string name) const;
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @fn we_variant& operator[](string name)
        ///
        /// @brief  Access to zero-based indexed field. 
        ///
        /// @author A. Abramov
        /// @date   25.06.2010
        ///
        /// @return we_variant - value of fields
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        we_variant& operator[](int index);
        const we_variant& operator[](int index) const;

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @fn const size_t record_size()
        ///
        /// @brief  Record size. 
        ///
        /// @author A. Abramov
        /// @date   25.06.2010
        ///
        /// @return size_t - number of fields in the recordset
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        const size_t record_size();

        const bool operator==(const db_cursor_base& rhs);
        const bool operator!=(const db_cursor_base& rhs) { return !(*this == rhs); }

    protected:
        base_iter iter_;
        db_recordset* parent;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  db_fw_cursor
    ///
    /// @brief  Provide forward-only, read-only access to records and fields in the recordset.
    ///
    /// @author A. Abramov
    /// @date	09.06.2010
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class db_fw_cursor : public db_cursor_base
    {
    public:
        db_fw_cursor() : db_cursor_base() {}
        db_fw_cursor(db_recordset* rs, db_cursor_base::base_iter& it) : db_cursor_base(rs, it) {}
        const we_variant& operator[](string name) const { return db_cursor_base::operator[](name); }
        const we_variant& operator[](int index) const { return db_cursor_base::operator[](index); }
    protected:
//         we_variant& operator[](string name) {};
//         we_variant& operator[](int index) {};
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  db_cursor
    ///
    /// @brief  Provide random access to records and fields in the recordset.
    ///
    /// @author A. Abramov
    /// @date	09.06.2010
    /// @example dbtest.cpp
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class db_cursor : public db_cursor_base {
    public:
        db_cursor() : db_cursor_base() {}
        db_cursor(db_recordset* rs, db_cursor_base::base_iter& it) : db_cursor_base(rs, it) {}
        db_cursor_base& operator+=(size_t num) { iter_ += num; return *this; }
        /// dereference iterator for recordset manipulation
        const std::vector< db_record >::iterator& iterator() { return iter_; }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  db_recordset
    ///
    /// @brief  array of db_record objects. Implement iterations, structure control and others.
    ///
    /// Two-demension array of data to store slice of database storage. Data can be accessed
    /// through the db_cursor variations. To acces may be used zero-based indexes of the columns,
    /// or columns names.
    ///
    /// @author A. Abramov
    /// @date	09.06.2010
    /// @example dbtest.cpp
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class db_recordset
    {
    public:

    public:
        db_recordset();
        /// explicit constructor with predefined set of columns names
        explicit db_recordset(const vector<string>& fld_names);
        ~db_recordset();

        /// remove record from specified position
        void erase(db_cursor& at);
        /// clear all records and fields names
        void clear() { records.clear(); field_names.clear(); }
        /// replace fields names with new set
        void set_names(const vector<string>& fld_names) { field_names = fld_names; }
        /// get fileds names
        const vector<string>& get_names() const { return field_names; }
        /// append one or more empty records to the end of dataset
        db_cursor push_back(size_t num = 1);
        /// insert one or more empty records at the specified position
        db_cursor insert(db_cursor& before, size_t num = 1);

        /// get the number of records in the dataset
        const size_t size() { return records.size(); }
        /// get the number of columns in the datatset
        const size_t record_size() { return field_names.size(); }
        /// get the db_cursor that points to the first record in the dataset
        db_cursor begin() { return db_cursor(this, records.begin()); }
        /// get the db_cursor that points to beyond the last record
        db_cursor end() { return db_cursor(this, records.end()); }
        /// get the forward-only read-only cursor that points to the first record in the dataset
        db_fw_cursor fw_begin() { return db_fw_cursor(this, records.begin()); }
        /// get the forward-only read-only cursor that points to beyond the last record
        db_fw_cursor fw_end() { return db_fw_cursor(this, records.end()); }

    protected:
        friend class db_cursor_base;

        vector<string> field_names;
        vector<db_record> records;

        friend class boost::serialization::access;

        /// serialize data to the archive
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & BOOST_SERIALIZATION_NVP(field_names);
            ar & BOOST_SERIALIZATION_NVP(records);
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  db_filter_base
    ///
    /// @brief  Abstract base class to construct logical queries.
    ///
    /// @author A. Abramov
    /// @date	15.06.2010
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class db_filter;

    class db_filter_base
    {
    public:
        db_filter_base() {}
        virtual ~db_filter_base() {}

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @fn virtual bool eval(db_cursor& data)
        ///
        /// @brief	Evaluate the logical expression on the given data. 
        ///
        /// @param  data - the db_cursor to access the data 
        ///
        /// @retval	bool value of the expression. 
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual bool eval(db_cursor& data) = 0;
        /// generates string representation of the stored condition
        virtual string tostring() = 0;
        /// extract set of namepspaces from the condition fields
        virtual void get_namespaces(std::set<string>& ns_list) = 0;

    protected:
        friend class db_filter;
        /// create copy of the object for the internal usage
        virtual db_filter_base* clone() = 0;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  db_condition
    ///
    /// @brief  atomic logical operation for DB queries.
    ///
    /// @author A. Abramov
    /// @date	09.06.2010
    /// @example dbtest.cpp
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class db_condition : public db_filter_base
    {
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
        explicit db_condition(string s);
        ~db_condition() {}

        string& field() { return field_name; }
        opcode& operation() { return op_code; }
        we_variant& value() { return test_value; }

        virtual bool eval(db_cursor& data);
        virtual string tostring();
        virtual void get_namespaces(std::set<string>& ns_list);

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
    /// @example dbtest.cpp
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class db_filter : public db_filter_base
    {
    public:
        db_filter() {}
        explicit db_filter(const db_filter& filt);
        explicit db_filter(const db_condition& cond);
        ~db_filter();

        db_filter& operator=(const db_filter& cpy);

        db_filter& set(db_filter_base& cond);
        db_filter& or(db_filter_base& cond);
        db_filter& and(db_filter_base& cond);

        virtual bool eval(db_cursor& data);
        virtual string tostring();
        virtual void get_namespaces(std::set<string>& ns_list);

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

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  db_query
    ///
    /// @brief  set of data for execute DB query
    ///
    /// @author A. Abramov
    /// @date	09.06.2010
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class db_query
    {
    public:
        /// defines logical filter for resulting dataset
        db_filter where;
        /// defines subset of fields which are expected in the results
        vector<string> what;
    };

} // namespace webEngine
BOOST_CLASS_TRACKING(webEngine::db_record, boost::serialization::track_never)
BOOST_CLASS_TRACKING(webEngine::db_recordset, boost::serialization::track_never)

#endif // __WEDBSTRUCT_H__
