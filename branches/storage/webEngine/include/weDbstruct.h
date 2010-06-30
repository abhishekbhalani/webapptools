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

#include <weVariant.h>
#include <boost/utility.hpp>
#include <set>
#include <boost/serialization/vector.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/filter_iterator.hpp>
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
	 * @li db_recordset
	 * @li db_cursor
     * @li db_fw_cursor
     * @li ::db_filter_cursor
	 * @li db_condition
	 * @li db_filter
	 * @li db_query
	 *
	 * All of these classes except db_query may be used independently of webEngine system.
	 * But the db_query class proposed to use with the i_storage interface implementations.
	 * BOOST_SERIALIZATION_NVP serialization mechanism implemented in the db_recordset to
     * use in various schemes.
     *
     * @example dbtest.cpp
     * Demonstrates operation with @ref db_layer "database abstraction layer" classes 
     */

    class db_cursor;
    class db_recordset;
    class db_cursor_base;
    typedef vector< we_variant > record_;
    typedef vector< record_ > recordset_;

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
    class db_record
    {
    public:
        db_record() : parent(NULL), rec(new record_), free_(true) {}
        db_record(db_recordset* parent_, record_* r);
        db_record(const db_record& r) : parent(NULL), rec(NULL) { *this = r; }
        ~db_record();
        db_record& operator=(const db_record& rhs);

        we_variant& operator[](int index);
        const we_variant& operator[](int index) const;
        we_variant& operator[](const string& name);
        const we_variant& operator[](const string& name) const;
    private:
        friend class db_cursor_base;

        db_recordset* parent;
        record_* rec;
        bool free_;
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
        typedef recordset_::iterator base_iter;

        db_cursor_base(db_recordset* rs) : parent_(rs), record(), iter_() {}
        db_cursor_base(db_recordset* rs, base_iter& it);
        virtual ~db_cursor_base() {}

        db_cursor_base& operator=(const db_cursor_base& cpy) { iter_ = cpy.iter_; parent_ = cpy.parent_; clear(); return *this; }

        //@{
        /// @fn we_variant& operator[](string name)
        ///
        /// @brief  Access to named field. 
        ///
        /// @return we_variant - value of fields
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        we_variant& operator[](string name);
        const we_variant& operator[](string name) const;
        //@}

        //@{
        /// @fn we_variant& operator[](int index)
        ///
        /// @brief  Access to zero-based indexed field. 
        ///
        /// @return we_variant - value of fields
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        we_variant& operator[](int index);
        const we_variant& operator[](int index) const;
        //@}

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @fn const size_t record_size()
        ///
        /// @brief  Record size. 
        ///
        /// @return size_t - number of fields in the recordset
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        const size_t record_size();

        const bool operator==(const db_cursor_base& rhs) const;
        const bool operator!=(const db_cursor_base& rhs) const { return !(this->operator==( rhs )); }

        virtual db_record& dereference() const { return *(const_cast<db_record*>(&record)); }

    protected:
        void update() { record.rec = &(*iter_); record.free_ = false; }
        void clear();

        db_recordset* parent_;
        base_iter iter_;
        db_record record;
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
        /// @fn virtual bool eval(db_cursor_base& data) const
        ///
        /// @brief	Evaluate the logical expression on the given data. 
        ///
        /// @param  data - the db_cursor to access the data 
        ///
        /// @retval	bool value of the expression. 
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        bool eval(db_cursor_base& data) const { return this->operator()(data.dereference()); }

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
        virtual bool operator()(db_record& data) const { return false; }
        /// generates string representation of the stored condition
        virtual string tostring() const {return ""; }
        /// extract set of namepspaces from the condition fields
        virtual void get_namespaces(std::set<string>& ns_list) {}

    protected:
        friend class db_filter;
        /// create copy of the object for the internal usage
        virtual db_filter_base* clone() { return NULL; }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  db_condition
    ///
    /// @brief  atomic logical operation for DB queries.
    ///
    /// @author A. Abramov
    /// @date	09.06.2010
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

        virtual bool operator()(db_record& data) const;
        virtual string tostring() const;
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

        virtual bool operator()(db_record& data) const;
        virtual string tostring() const;
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

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  db_cursor
    ///
    /// @brief  Provide random access to records and fields in the recordset.
    ///
    /// @author A. Abramov
    /// @date	09.06.2010
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class db_cursor :
        public boost::iterator_facade <
            db_cursor,
            db_record,
            boost::random_access_traversal_tag
        >,
        public db_cursor_base
    {
	private:
		struct enabler {};  // a private type avoids misuse

	public:
		/// default constructor
		db_cursor() : db_cursor_base(NULL) {}
		/// constructor for use inside db_recordset
		db_cursor(db_recordset* rs, db_cursor_base::base_iter& it) : db_cursor_base(rs, it) {}

		we_variant& operator[](string name) { return db_cursor_base::operator[](name); }
		const we_variant& operator[](string name) const { return db_cursor_base::operator[](name); }
		we_variant& operator[](int index) { return db_cursor_base::operator[](index); }
		const we_variant& operator[](int index) const { return db_cursor_base::operator[](index); }
		
		db_cursor operator-(const int val) { return db_cursor(parent_, iter_ - val); }
		const difference_type operator-(const db_cursor& rit) const { return (iter_ - rit.iter_); }
		const difference_type operator-(const base_iter& brit) const { return (iter_ - brit); }

	protected:
		friend class boost::iterator_core_access;
		friend class db_recordset;

		bool equal(db_cursor const& other) const { return this->operator==(other); }

		void increment() { ++iter_; }
		void decrement() { --iter_; }

		db_record& dereference() const { const_cast<db_cursor*>(this)->update(); return db_cursor_base::dereference(); }
		difference_type distance_to(const db_cursor& f2) const { return (f2.iter_ - iter_); }
		void advance(difference_type v) { iter_ += v; }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  db_fw_cursor
    ///
    /// @brief  Provide forward-only, read-only access to records and fields in the recordset.
    ///
    /// @author A. Abramov
    /// @date	09.06.2010
    ////////////////////////////////////////////////////////////////////////////////////////////////////
	class db_fw_cursor :
		public boost::iterator_facade <
		    db_fw_cursor,
		    db_record,
		    boost::forward_traversal_tag
		>,
		public db_cursor_base
	{
	private:
		struct enabler {};  // a private type avoids misuse

	public:
		/// default constructor
		db_fw_cursor() : db_cursor_base(NULL) {}
		/// constructor for use inside db_recordset
		db_fw_cursor(db_recordset* rs, db_cursor_base::base_iter& it) : db_cursor_base(rs, it) {}

		const we_variant& operator[](string name) const { return db_cursor_base::operator[](name); }
		const we_variant& operator[](int index) const { return db_cursor_base::operator[](index); }


		db_fw_cursor operator-(const int val) { return db_fw_cursor(parent_, iter_ - val); }
		const difference_type operator-(const db_fw_cursor& rit) const { return (iter_ - rit.iter_); }
		const difference_type operator-(const base_iter& brit) const { return (iter_ - brit); }

	protected:
		friend class boost::iterator_core_access;
		friend class db_recordset;

		bool equal(db_fw_cursor const& other) const { return this->operator==(other); }
		void increment() { ++iter_; }
		db_record& dereference() const { const_cast<db_fw_cursor*>(this)->update(); return db_cursor_base::dereference(); }
	};

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @typedef    boost::filter_iterator<db_filter_base, db_fw_cursor> db_filter_cursor
    ///
    /// @brief  Defines the forward-only read-only cursor with filter abilities.
    /// 
    /// Get the cursor by db_recordset::filter_begin() and db_recordset::filter_end() functions This
    /// class isn't directly inherit the db_cursor_base class. By this way the db_cursor_base::record_size
    /// function is not accessible. Also, this iterator must be dereferenced before using
    /// [] for access to the record fields. See examples for details. 
    ///
    /// @author A. Abramov
    /// @date	30.06.2010
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    typedef boost::filter_iterator<db_filter_base, db_fw_cursor> db_filter_cursor;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  db_recordset
    ///
    /// @brief  array of db_record objects. Implement iterations, structure control and others.
    ///
    /// Two-dimension array of data to store slice of database storage. Data can be accessed
    /// through the db_cursor variations. To access may be used zero-based indexes of the columns,
    /// or columns names.
    ///
    /// @author A. Abramov
    /// @date	09.06.2010
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
        /// get the filtering cursor that points to the first record in the dataset
        db_filter_cursor filter_begin(db_filter& filter) { return db_filter_cursor(filter, fw_begin(), fw_end()); }
        /// get the filtering cursor that points to beyond the last record
        db_filter_cursor filter_end(db_filter& filter) { return db_filter_cursor(filter, fw_end(), fw_end()); }

    protected:
        friend class db_record;
        friend class db_cursor_base;

        vector<string> field_names;
        recordset_ records;

        bool is_valid_iter(const db_cursor_base::base_iter& iter_) const { return iter_ != records.end(); }

        friend class boost::serialization::access;

        /// serialize data to the archive
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & BOOST_SERIALIZATION_NVP(field_names);
            ar & BOOST_SERIALIZATION_NVP(records);
        }
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

BOOST_CLASS_TRACKING(webEngine::db_recordset, boost::serialization::track_never)

#endif // __WEDBSTRUCT_H__
