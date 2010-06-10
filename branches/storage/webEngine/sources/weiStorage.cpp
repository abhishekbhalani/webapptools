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
#include "weiBase.h"
#include "weTagScanner.h"
#include "weiStorage.h"
#include "iweStorage.xpm"

using namespace webEngine;
using namespace boost;

int i_storage::last_id = 0;

i_storage::i_storage(engine_dispatcher* krnl, void* handle /*= NULL*/) :
    i_plugin(krnl, handle)
{
    pluginInfo.interface_name = "i_storage";
    pluginInfo.interface_list.push_back("i_storage");
    pluginInfo.plugin_desc = "Base plugin interface";
    pluginInfo.plugin_id = "C7F595160595";
    pluginInfo.plugin_icon = WeXpmToStringList(iweStorage_xpm, sizeof(iweStorage_xpm) / sizeof(char*) );
    last_id = rand();
}

i_storage::~i_storage(void)
{
}

i_plugin* i_storage::get_interface( const string& ifName )
{
    if (iequals(ifName, "i_storage"))
    {
        usageCount++;
        return ((i_storage*)this);
    }
    return i_plugin::get_interface(ifName);
}

std::string i_storage::generate_id( const string& objType /*= ""*/ )
{
    return lexical_cast<string>(++last_id);
}

db_recordset::db_recordset(void)
{

}

db_recordset::db_recordset(vector<string> fld_names)
{
    field_names.assign(fld_names.begin(), fld_names.end());
}

db_recordset::db_recordset(db_record& rec)
{
    records.push_back(rec);
}

db_recordset::~db_recordset(void)
{

}

void db_recordset::erase(db_cursor& at)
{
    records.erase(static_cast< vector<db_record>::iterator >(at));
}

void db_recordset::push_back(db_record& rec)
{
    // @todo Verify data types
    records.push_back(rec);
}

void db_recordset::insert(db_cursor& before, db_record& rec)
{
    records.insert(before, rec);
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
