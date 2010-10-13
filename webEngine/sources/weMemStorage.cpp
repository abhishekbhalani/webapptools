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

#include <fstream>
#include <strstream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include "weOptions.h"
#include "weTagScanner.h"
#include "weMemStorage.h"
#include "weiBase.h"
#include "weHelper.h"

using namespace boost;
namespace bfs = boost::filesystem;

namespace webEngine {

mem_storage::mem_storage( engine_dispatcher* krnl, void* handle /*= NULL*/ ) :
    i_storage(krnl, handle)
{
    pluginInfo.interface_name = "mem_storage";
    pluginInfo.interface_list.push_back("mem_storage");
    pluginInfo.plugin_desc = "In-memory storage";
    pluginInfo.plugin_id = "D82B31419339";
    file_name = "";
    last_id = 1;
}

mem_storage::~mem_storage(void)
{
    flush(file_name);
}

i_plugin* mem_storage::get_interface( const string& ifName )
{
    if (iequals(ifName, "i_storage")) {
        usageCount++;
        return ((i_storage*)this);
    }
    if (iequals(ifName, "mem_storage")) {
        usageCount++;
        return ((mem_storage*)this);
    }
    return i_storage::get_interface(ifName);
}

int mem_storage::get(db_query& query, db_recordset& results)
{
    std::set<string> ns_list;
    std::set<string>::const_iterator ns_it;
    vector<string>::const_iterator rec_it;
    db_filter_cursor cursor;

    LOG4CXX_TRACE(iLogger::GetLogger(), "mem_storage::get");

    boost::unique_lock<boost::mutex> locker(data_access);

    results.clear_data();
    //results.set_names(query.what);
    query.where.get_namespaces(ns_list);

    // @todo implement db_recordset.join to perform queries from more than one table
    if (ns_list.size() != 1) {
        LOG4CXX_ERROR(iLogger::GetLogger(), "mem_storage::get - query wants data from " << ns_list.size() << " tables; Only one table supported.");
    }
    if (ns_list.size() > 0) {
        ns_it = ns_list.begin();
        mem_tables::iterator mit = name_spaces.find(*ns_it);
        if (mit != name_spaces.end()) {
            cursor = mit->second->filter_begin(query.where);
            while (cursor != name_spaces[*ns_it]->filter_end(query.where)) {
                db_cursor record = results.push_back();
                for(rec_it = results.get_names().begin(); rec_it != results.get_names().end(); rec_it++) {
                    try {
                        record[*rec_it] = (*cursor)[*rec_it];
                    } catch(out_of_range &e) {
                        LOG4CXX_ERROR(iLogger::GetLogger(), "mem_storage::get - can't get filed " << *rec_it << ": source doesn't contains this field. " << e.what());
                    }
                } // foreach field
                ++cursor;
            } // foreach record
        } // if namespase exist
        else {
            LOG4CXX_ERROR(iLogger::GetLogger(), "mem_storage::set - can't find namespace " << *ns_it);
        }
    }
    return results.size();
}

int mem_storage::set(db_query& query, db_recordset& data)
{
    int retval = 0;
    bool updated;
    std::set<string> ns_list;
    std::set<string>::iterator ns_it;

    LOG4CXX_TRACE(iLogger::GetLogger(), "mem_storage::set(query, db_record)");

    boost::unique_lock<boost::mutex> locker(data_access);

    query.where.get_namespaces(ns_list);
    // @todo implement db_recordset.join to perform queries from more than one table
    if (ns_list.size() != 1) {
        LOG4CXX_ERROR(iLogger::GetLogger(), "mem_storage::get - query wants data from " << ns_list.size() << " tables; Only one table supported.");
    }
    if (ns_list.size() > 0) {
        ns_it = ns_list.begin();
        mem_tables::iterator mit = name_spaces.find(*ns_it);
        if (mit != name_spaces.end()) {
            db_cursor cursor = mit->second->begin();
            updated = false;
            while (cursor != name_spaces[*ns_it]->end()) {
                if (query.where.eval(cursor)) {
                    updated = true;
                    // update record
                    // @todo only firs record from argument used for update
                    db_cursor record = data.begin();
                    try {
                        vector<string> rec_fields = data.get_names();
                        for (size_t i = 0; i < rec_fields.size(); i++) {
                            cursor[rec_fields[i]] = record[rec_fields[i]];
                        } // foreach field
                        retval++;
                    } catch(out_of_range &) {};
                } // if need to update
                ++cursor;
            } // foreach record
            if (!updated) {
                // insert new record
                cursor = mit->second->push_back();
                db_cursor record = data.begin();
                try {
                    vector<string> rec_fields = data.get_names();
                    for (size_t i = 0; i < rec_fields.size(); i++) {
                        cursor[rec_fields[i]] = record[rec_fields[i]];
                    } // foreach field
                    retval++;
                } catch(out_of_range &) {};
            } // if not updated
        } else {
            LOG4CXX_ERROR(iLogger::GetLogger(), "mem_storage::set - can't find namespace " << *ns_it);
        }
    }

    return retval;
}

int mem_storage::set(db_recordset& data)
{
    int retval = 0;
    string ns_name;
    size_t i;

    LOG4CXX_TRACE(iLogger::GetLogger(), "mem_storage::set(db_recordset)");

    boost::unique_lock<boost::mutex> locker(data_access);

    // get fields and extract namespace
    vector<string> rec_fields = data.get_names();
    if (rec_fields.size() > 0) {
        ns_name = rec_fields[0];
        i = ns_name.find_last_of('.');
        if (i != string::npos) {
            ns_name = ns_name.substr(i + 1);
            mem_tables::iterator mit = name_spaces.find(ns_name);
            if (mit != name_spaces.end()) {
                db_cursor cursor = mit->second->push_back();
                db_cursor record = data.begin();
                try {
                    vector<string> rec_fields = data.get_names();
                    for (size_t i = 0; i < rec_fields.size(); i++) {
                        cursor[rec_fields[i]] = record[rec_fields[i]];
                    } // foreach field
                    retval++;
                } catch(out_of_range &) {};
            }
        } // namespace name found
    } // request has fields

    return retval;
}

int mem_storage::del(db_filter& filter)
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "mem_storage::delete");

    boost::unique_lock<boost::mutex> locker(data_access);

    int retval = 0;
    std::set<string> ns_list;
    std::set<string>::iterator ns_it;

    filter.get_namespaces(ns_list);
    for (ns_it = ns_list.begin(); ns_it != ns_list.end(); ns_it++) {
        if (name_spaces.find(*ns_it) != name_spaces.end()) {
            db_cursor cursor = name_spaces[*ns_it]->begin();
            while (cursor != name_spaces[*ns_it]->end()) {
                if (filter.eval(cursor)) {
                    cursor = name_spaces[*ns_it]->erase(cursor);
                    retval++;
                } else {
                    ++cursor;
                }
            } // foreach record
        } // if table exist
    } // foreach table
    return retval;
}

void mem_storage::save_db( const string& fname )
{
    size_t i, j;
    string s;

    boost::unique_lock<boost::mutex> locker(data_access);

    try {
        LOG4CXX_TRACE(iLogger::GetLogger(), "mem_storage::save_db");
        std::ofstream ofs(fname.c_str());
        // save data to archive

        // 1. write header
        ofs << "<database last_id='" << last_id << "'>" << endl;

        // 2. save namespaces
        mem_tables::iterator mit = name_spaces.begin();
        while (mit != name_spaces.end()) {
            ofs << "\t<namespace name='" << mit->first << "'>" << endl;
            // 3. save namespace structure
            ofs << "\t\t<columns>" << endl;
            vector<string> cols = mit->second->get_names();
            for (i = 0; i < cols.size(); i++) {
                s = cols[i];
                if ( starts_with(s, mit->first) ) {
                    s = s.substr(mit->first.length()+1);
                }
                ofs << "\t\t\t<column name='" << s << "'/>" << endl;
            }
            ofs << "\t\t</columns>" << endl;
            // 4. save namespace data
            ofs << "\t\t<records>" << endl;
            db_cursor rec = mit->second->begin();
            j = 0;
            while (rec != mit->second->end()) {
                ofs << "\t\t\t<record idx='" << j << "'>" << endl;
                for (i = 0; i < cols.size(); i++) {
                    ofs << "\t\t\t\t<column idx='" << i << "' type='" << rec[i].which() << "'>";
                    try {
                        s = boost::lexical_cast<string>(rec[i]);
                    } catch (bad_lexical_cast &e) {
                        s = "";
                        LOG4CXX_ERROR(iLogger::GetLogger(), "mem_storage::save_db - can't save value: " << e.what() << "\n" <<
                                      "namespace: " << mit->first << " record: " << j << " column: " << i);
                    }
                    s = ScreenXML(s);
                    ofs << s << "</column>" << endl;
                }
                ofs << "\t\t\t</record>" << endl;
                ++rec;
                j++;
            }
            ofs << "\t\t</records>" << endl;
            // close namespace
            ofs << "\t</namespace>" << endl;
            mit++;
        }
        ofs << "</database>" << endl;
    } catch(std::exception& e) {
        LOG4CXX_ERROR(iLogger::GetLogger(), "mem_storage::save_db error: " << e.what());
    }
    return;
}

void mem_storage::load_db( const string& fname )
{
    char* buff = NULL;

    boost::unique_lock<boost::mutex> locker(data_access);

    try {
        LOG4CXX_TRACE(iLogger::GetLogger(), "mem_storage::load_db");
        std::ifstream ifs(fname.c_str());

        // load data from archive
        if (ifs.is_open()) {
            size_t fsize = size_t(bfs::file_size(bfs::path(fname)));
            buff = new char[fsize + 10];
            if(buff != NULL) {
                memset(buff, 0, fsize+10);
                ifs.read(buff, fsize);
            } else {
                // can't read file - exit
                LOG4CXX_ERROR(iLogger::GetLogger(), "mem_storage::load_db can't load file " << fname);
                return;
            }

            str_tag_stream ss(buff);
            tag_scanner sc(ss);

            bool in_parsing = true;
            bool in_records = false;
            int  parse_level = 0;
            int token;
            int tp = -1;
            int rec_num;
            string name, val, dat;
            string ns_name;
            auto_ptr<db_recordset> ns_recs;
            auto_ptr<db_cursor> record;
            vector<string> fnames;

            mem_tables::iterator mit = name_spaces.begin();
            while (mit != name_spaces.end()) {
                delete mit->second;
                mit++;
            }
            name_spaces.clear();

            while (in_parsing) {
                token = sc.get_token();
                switch(token) {
                case wstError:
                    LOG4CXX_WARN(iLogger::GetLogger(), "mem_storage::load_db parsing error");
                    in_parsing = false;
                    break;
                case wstEof:
                    LOG4CXX_TRACE(iLogger::GetLogger(), "mem_storage::load_db - EOF");
                    in_parsing = false;
                    break;
                case wstTagStart:
                    name = sc.get_tag_name();
                    switch (parse_level) {
                    case 0:
                        if ( !iequals(name, "database") ) {
                            in_parsing = false;
                        }
                        break;
                    case 1:
                        if ( iequals(name, "namespace") ) {
                            ns_name = "";
                        } else {
                            in_parsing = false;
                        }
                        break;
                    case 2:
                        if ( iequals(name, "columns") ) {
                            in_records = false;
                            fnames.clear();
                        } else if ( iequals(name, "records") ) {
                            in_records = true;
                        } else {
                            in_parsing = false;
                        }
                        break;
                    case 3:
                        if ( iequals(name, "column") ) {
                            in_records = false;
                        } else if ( iequals(name, "record") ) {
                            in_parsing = true;
                            rec_num = 0;
                            if (ns_recs.get() != NULL) {
                                record = auto_ptr<db_cursor>(&(ns_recs->push_back()));
                            }
                        } else {
                            in_parsing = false;
                        }
                        break;
                    case 4:
                        if ( !iequals(name, "column") ) {
                            in_parsing = false;
                        }
                        break;
                    default:
                        in_parsing = false;
                    }
                    if (in_parsing) {
                        parse_level++;
                        val = "";
                        dat = "";
                        tp = -1;
                    } else {
                        LOG4CXX_WARN(iLogger::GetLogger(), "mem_storage::load_db parsing error - unexpected tag " << name);
                    }
                    break;
                case wstTagEnd:
                    name = sc.get_tag_name();
                    parse_level--;
                    if (parse_level == 0) {
                        in_parsing = false;
                    }
                    if ( iequals(name, "columns") ) {
                        ns_recs = get_recordset_by_fields(fnames);
                    }
                    if ( iequals(name, "namespace") && ns_recs.get() != NULL ) {
                        name_spaces[ns_name] = ns_recs.release();
                    }
                    if ( iequals(name, "column") && ns_recs.get() != NULL && in_records ) {
                        try {
                            switch(tp) {
                            case 0:
                                (*record)[rec_num] = boost::lexical_cast<char>(dat);
                                break;
                            case 1:
                                (*record)[rec_num] = boost::lexical_cast<int>(dat);
                                break;
                            case 2:
                                (*record)[rec_num] = boost::lexical_cast<bool>(dat);
                                break;
                            case 3:
                                (*record)[rec_num] = boost::lexical_cast<double>(dat);
                                break;
                            case 4:
                                (*record)[rec_num] = dat;
                                break;
                            default:
                                (*record)[rec_num] = boost::blank();
                                break;
                            }
                        } catch(bad_lexical_cast &e) {
                            LOG4CXX_WARN(iLogger::GetLogger(), "mem_storage::load_db can't store '" << dat << "' as value of column " << rec_num <<
                                         " in namespace " << ns_name << endl << e.what());
                        }
                        rec_num++;
                    }
                    break;
                case wstAttr:
                    name = sc.get_attr_name();
                    val = sc.get_value();
                    val = UnscreenXML(val);
                    if (parse_level == 1 && iequals(name, "last_id") ) {
                        try {
                            last_id = boost::lexical_cast<int>(val);
                        } catch (bad_lexical_cast &e) {
                            LOG4CXX_WARN(iLogger::GetLogger(), "mem_storage::load_db can't parse " << val << " as last_id value: " << e.what());
                        }
                    }
                    if (parse_level == 2 && iequals(name, "name") ) {
                        ns_name = val;
                    }
                    if (parse_level == 4 && iequals(name, "name") && !in_records ) {
                        // <column name=""/>
                        val = ns_name + "." + val;
                        fnames.push_back(val);
                    }
                    if (parse_level == 5 && iequals(name, "type") ) {
                        try {
                            tp = boost::lexical_cast<int>(val);
                        } catch (bad_lexical_cast &e) {
                            tp = -1;
                            LOG4CXX_WARN(iLogger::GetLogger(), "mem_storage::load_db can't parse " << val << " as type_id value: " << e.what());
                        }
                    }
                    break;
                case wstWord:
                case wstSpace:
                    dat += sc.get_value();
                    break;
                };
            }
        } // if file open
    } catch(std::exception& e) {
        LOG4CXX_ERROR(iLogger::GetLogger(), "mem_storage::load_db error: " << e.what());
    }
    if (buff != NULL) {
        delete buff;
    }
    return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn bool mem_storage::init_storage( const string& params )
///
/// @brief  Initializes the storage_db.
///
/// @param  params - Pathname for the storage_db file.
/// @retval	true if it succeeds, false if it fails.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool mem_storage::init_storage( const string& params )
{
    file_name = params;
    load_db(file_name);
    size_t i = 0;
    std::vector<std::string> inits;
    std::vector<std::string> fields;

    boost::unique_lock<boost::mutex> locker(data_access);

    while (idb_struct[i] != NULL) {
        boost::split(inits, idb_struct[i], boost::is_any_of(":"));
        if (inits.size() > 0) {
            if (name_spaces.find(inits[0]) == name_spaces.end()) {
                //create namespace
                fields.clear();
                for (size_t j = 1; j < inits.size(); j++) {
                    string fld = inits[j];
                    size_t pos = fld.find(' ');
                    if (pos != string::npos) {
                        fld = fld.substr(0, pos);
                    }
                    if (fld.length() > 0) {
                        fields.push_back(inits[0] + "." + fld);
                    }
                } // for each field
                if (fields.size() > 0) {
                    name_spaces[inits[0]] = get_recordset_by_fields(fields).release();
                }
            } // not namespace found
        } // if inits parsed
        i++;
    } // while exist initializer
    return true;
}

void mem_storage::flush( const string& params /*= ""*/)
{
    if (params != "") {
        file_name = params;
    }
    if (file_name != "") {
        LOG4CXX_TRACE(iLogger::GetLogger(), "mem_storage::flush: filename not empty, save data");
        save_db(file_name);
    }
}
namespace {
class mem_storage_recordset: public db_recordset {
public:
    explicit mem_storage_recordset(const vector<string>& fields):db_recordset(fields) {};
};
}

std::auto_ptr<db_recordset> mem_storage::get_recordset_by_namespace(const std::string& ns)
{
    return get_recordset_by_fields(i_storage::get_namespace_struct(ns));
}

std::auto_ptr<db_recordset> mem_storage::get_recordset_by_fields(const vector<string>& fields)
{
    return std::auto_ptr<db_recordset>((db_recordset*)(new mem_storage_recordset(fields)));
}

} // namespace webEngine
