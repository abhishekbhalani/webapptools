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
	 * But the db_query class proposed to use with the i_storage interfase implementations.
	 * BOOST_SERIALIZATION_NVP serialization mechanism implemented in the db_record and
	 * db_recordset to use in various schemes.
	 */

    class db_cursor;
    class db_recordset;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  db_record
    ///
    /// @brief  set of fields as zero-based array of values.
    ///
    /// db_record is the set of we_variant values. This is the internal class to store database
    /// records as the zero-based array of columns. This class implements serialization to
	/// allow db_recordset class to be serialized.
    ///
    /// @author A. Abramov
    /// @date	09.06.2010
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class db_record : protected vector< we_variant >
    {
    public:
        /// construct the array
        db_record() : vector< we_variant>() {}
        /// construct the array and reserve space for values
        db_record(size_t sz) : vector< we_variant>() { reserve(sz); }
    protected:
        friend class db_cursor;
        friend class db_recordset;
        friend class ::boost::serialization::access;
        BOOST_SERIALIZATION_SPLIT_MEMBER();
        /// save instance to the given archive
        template<class Archive>
        void save(Archive & ar, const unsigned int version) const { 
            size_t vsize;
            we_variant value;
            vsize = size();
            ar & BOOST_SERIALIZATION_NVP(vsize);
            for (size_t i = 0; i < vsize; i++) {
                value = (*this)[i];
                ar & BOOST_SERIALIZATION_NVP(value);
            }
        }
        /// load instance from the given archive
        template<class Archive>
        void load(Archive & ar, const unsigned int version) { 
            size_t vsize;
            we_variant value;
            clear();
            ar & BOOST_SERIALIZATION_NVP(vsize);
            for (size_t i = 0; i < vsize; i++) {
                ar & BOOST_SERIALIZATION_NVP(value);
                push_back(value);
            }
        }
    };

    /*///////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  db_fw_cursor
    ///
    /// @brief  Provide forward-only, read-only access to records and fields in the recordset.
    ///
    /// @author A. Abramov
    /// @date	09.06.2010
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class db_fw_cursor
    {
    }; */

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  db_cursor
    ///
    /// @brief  Provide access to records and fields in the recordset.
    ///
    /// Extend the vector::iterator to access record columns by index or by name. db_cursor is the
    /// iterator-like accessor to the db_recordset. This is only correct way to access the data
    /// stored in the db_recordset.
    ///
    /// @author A. Abramov
    /// @date	09.06.2010
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class db_cursor : public vector<db_record>::iterator {
    public:
        /// default constructor
        db_cursor() : vector<db_record>::iterator(), parent(NULL) {}
        /// copy constructor
        db_cursor(const db_cursor& cp);
        /// explicit constructor to create cursor for specified db_recordset and vector::iterator
        explicit db_cursor(db_recordset* rs, vector<db_record>::iterator it);

        /// copy opreator
        db_cursor& operator=(const db_cursor& cp);
        /// access to current record fields by record name
        we_variant& operator[](string name);
        /// access to current record fields by record index
        we_variant& operator[](int index);

        /// get the numbers of columns in the recordset
        const size_t record_size();

    protected:
        db_recordset* parent;
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
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class db_recordset : public boost::noncopyable
    {
    public:

    public:
        db_recordset();
        /// explicit constructor with predefined set of columns names
        explicit db_recordset(vector<string> fld_names);
        ~db_recordset();

        /// remove record from specified position
        void erase(db_cursor& at);
        /// clear all records and fields names
        void clear() { records.clear(); field_names.clear(); }
        /// replace fields names with new set
        void set_names(vector<string> fld_names) { field_names.assign(fld_names.begin(), fld_names.end()); }
        /// get fileds names
        vector<string> get_names() { return field_names; }
        /// append one or more empty records to the end of dataset
        db_cursor push_back(size_t num = 1);
        /// insert one or more empty records at the specified position
        db_cursor insert(db_cursor& before, size_t num = 1);

        /// get the number of records in the dataset
        const size_t size() { return records.size(); }
        /// get the number of columns in the datatset
        const size_t record_size() { return field_names.size(); }
        /// get the db_cursor that points to the first record in the dataset
        db_cursor begin();
        /// get the db_cursor that points to outside the last record
        db_cursor end();

    protected:
        friend class db_cursor;

        db_cursor push_back(db_record& rec);
        db_cursor insert(db_cursor& before, db_record& rec);

        vector<string> field_names;
        vector<db_record> records;

        friend class ::boost::serialization::access;

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
        virtual db_filter_base* copy() = 0;
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
        db_condition(const db_condition& c);
        explicit db_condition(string s);
        ~db_condition() {}

        db_condition& operator=(db_condition& c);

        string& field() { return field_name; }
        opcode& operation() { return op_code; }
        we_variant& value() { return test_value; }

        virtual bool eval(db_cursor& data);
        virtual string tostring();
        virtual void get_namespaces(std::set<string>& ns_list);

    protected:
        virtual db_filter_base* copy();

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

        db_filter& set(db_condition& cond);
        db_filter& set(db_filter& cond);
        db_filter& or(db_condition& cond);
        db_filter& or(db_filter& cond);
        db_filter& and(db_condition& cond);
        db_filter& and(db_filter& cond);

        virtual bool eval(db_cursor& data);
        virtual string tostring();
        virtual void get_namespaces(std::set<string>& ns_list);

    protected:
        virtual db_filter_base* copy();

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
        db_query() {}

        db_filter& where() { return filter; }
        vector<string>& what() { return output; }
        //    db_recordset& values() { return data_set; }

    protected:
        db_filter filter;
        vector<string> output;
        //    db_recordset data_set;
    };

} // namespace webEngine
BOOST_CLASS_TRACKING(webEngine::db_record, boost::serialization::track_never)
BOOST_CLASS_TRACKING(webEngine::db_recordset, boost::serialization::track_never)

#endif // __WEDBSTRUCT_H__
