// dbtest.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <fstream>
#include <weDbstruct.h>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/split_member.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

using namespace std;
using namespace webEngine;

int main(int argc, char* argv[])
{
    size_t r;
    vector<string> fnames;

    fnames.push_back("id");
    fnames.push_back("name");
    fnames.push_back("value");

    db_recordset rs(fnames);
    db_cursor rec = rs.push_back();
    rec["id"] = 1;
    rec["name"] = string("test");
    rec[2] = 2.5;

    db_cursor cursor;
    cout << "Try to use uninitialized cursor..." << endl;
    try {
        cout << "Value of [0][0] is " << cursor[0] << endl;
    }
    catch(std::exception &e) {
        cout << "Exception: " << e.what() << endl;
    }

    cursor = rs.begin();

    cout << "Value of [0][0] is " << cursor[0] << endl;
    cout << "Value of [0][name] is " << cursor["name"] << endl;

    cout << "Try to exit out of rage..." << endl;
    cursor++;
    try {
        cout << "Value of [0][0] is " << cursor[0] << endl;
    }
    catch(out_of_range &e) {
        cout << "Exception: " << e.what() << endl;
    }

    // insert record
    cursor = rs.begin();
    cout << "Before insert:" << endl << "Size: " << rs.size() << endl;
    for (r = 0; r < rs.size(); r++)
    {
        cout << "Record [" << r << "] is:" << endl;
        for (size_t i = 0; i < cursor.record_size(); i ++) {
            cout << "\t[" << i << "] = " << cursor[i] << endl;
        }
        cursor++;
    }

    cursor = rs.begin();
    rec = rs.insert(cursor);
    rec["id"] = 2;
    rec["name"] = string("values");
    rec[2] = 3.1415;   

    cursor = rs.begin();
    cout << "After first insert:" << endl << "Size: " << rs.size() << endl;
    for (r = 0; r < rs.size(); r++)
    {
        cout << "Record [" << r << "] is:" << endl;
        for (size_t i = 0; i < cursor.record_size(); i ++) {
            cout << "\t[" << i << "] = " << cursor[i] << endl;
        }
        cursor++;
    }

    cursor = rs.begin();
    rec = rs.insert(cursor, 2);
    rec["id"] = 3;
    rec["name"] = string("news");
    rec[2] = 2.17;
    rec++;
    rec["id"] = 4;

    cursor = rs.begin();
    cout << "After second insert:" << endl << "Size: " << rs.size() << endl;
    r = 0;
    while (cursor != rs.end())
    {
        cout << "Record [" << r << "] is:" << endl;
        for (size_t i = 0; i < cursor.record_size(); i ++) {
            if (cursor[i].empty()) {
                cout << "\t[" << i << "] = <empty>" << endl;
            }
            else {
                cout << "\t[" << i << "] = " << cursor[i] << endl;
            }
        }
        cursor++;
        r++;
    }

    // conditions
    cout << "Test conditions" << endl;
    db_condition cond;
    cond.field() = "id";
    cond.operation() = db_condition::equal;
    cond.value() = 1;
    cursor = rs.begin();
    cout << "Condition: " << cond.tostring() << endl;
    for (r = 0; r < rs.size(); r++)
    {
        try{
            cout << "Record " << r << " condition is " << cond.eval(cursor) << endl;
        }
        catch (bad_cast &e) {
            cout << "Record " << r << " can't compare: " << e.what() << endl;
        }
        cursor++;
    }

    cond.field() = "value";
    cond.operation() = db_condition::great;
    cond.value() = 2.3;
    cursor = rs.begin();
    cout << "Condition: " << cond.tostring() << endl;
    for (r = 0; r < rs.size(); r++)
    {
        try{
            cout << "Record " << r << " condition is " << cond.eval(cursor) << endl;
        }
        catch (bad_cast &e) {
            cout << "Record " << r << " can't compare: " << e.what() << endl;
        }
        cursor++;
    }

    cond.field() = "name";
    cond.operation() = db_condition::like;
    cond.value() = string("es");
    cursor = rs.begin();
    cout << "Condition: " << cond.tostring() << endl;
    for (r = 0; r < rs.size(); r++)
    {
        try{
            cout << "Record " << r << " condition is " << cond.eval(cursor) << endl;
        }
        catch (bad_cast &e) {
            cout << "Record " << r << " can't compare: " << e.what() << endl;
        }
        cursor++;
    }

    cond.field() = "value";
    cond.operation() = db_condition::is_null;
    cursor = rs.begin();
	cout << "Condition: " << cond.tostring() << endl;
    for (r = 0; r < rs.size(); r++)
    {
        try{
            cout << "Record " << r << " condition is " << cond.eval(cursor) << endl;
        }
        catch (bad_cast &e) {
            cout << "Record " << r << " can't compare: " << e.what() << endl;
        }
        cursor++;
    }

    // construct filters
    db_condition c1("id > 2");
    db_condition c2("value > 3.0");
    db_filter filter;

    filter.set(db_condition("name like es")).or(c1);
    cursor = rs.begin();
    cout << "Filter: " << filter.tostring() << endl;
    for (r = 0; r < rs.size(); r++)
    {
        try{
            cout << "Record " << r << " condition is " << filter.eval(cursor) << endl;
        }
        catch (bad_cast &e) {
            cout << "Record " << r << " can't compare: " << e.what() << endl;
        }
        cursor++;
    }

    db_filter filter2;
    filter2.set(cond).and(filter);
    cursor = rs.begin();
    cout << "Filter: " << filter2.tostring() << endl;
    for (r = 0; r < rs.size(); r++)
    {
        try{
            cout << "Record " << r << " condition is " << filter2.eval(cursor) << endl;
        }
        catch (bad_cast &e) {
            cout << "Record " << r << " can't compare: " << e.what() << endl;
        }
        cursor++;
    }

    // delete record
    filter.set(db_condition("id == 2")).or(db_condition("id == 4"));
    cursor = rs.begin();
    while (cursor != rs.end())
    {
        size_t nxt = cursor - rs.begin();
        if (filter.eval(cursor)) {
            rs.erase(cursor);
            cursor = rs.begin();
            cursor += nxt;
        }
        else {
            cursor++;
        }
    }
    cout << "After deletion " << filter.tostring() << endl << "Size: " << rs.size() << endl;
    cursor = rs.begin();
    r = 0;
    while (cursor != rs.end())
    {
        cout << "Record [" << r << "] is:" << endl;
        for (size_t i = 0; i < cursor.record_size(); i ++) {
            if (cursor[i].empty()) {
                cout << "\t[" << i << "] = <empty>" << endl;
            }
            else {
                cout << "\t[" << i << "] = " << cursor[i] << endl;
            }
        }
        cursor++;
        r++;
    }

    // save data to archive
    {
        std::ofstream ofs("saved.db");
        boost::archive::text_oarchive oa(ofs);
        // write class instance to archive
        oa << rs;
        // archive and stream closed when destructor are called
    }
    // save data to archive
    {
        std::ofstream ofs("saved.xml");
        boost::archive::xml_oarchive oa(ofs);
        // write class instance to archive
        oa << BOOST_SERIALIZATION_NVP(rs);
        // archive and stream closed when destructor are called
    }

    return 0;
}
