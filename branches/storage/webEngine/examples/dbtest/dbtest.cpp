// dbtest.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <weHelper.h>
#include <weiStorage.h>

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
    catch(std::exception &e) {
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

    cursor = rs.begin();
    cout << "After second insert:" << endl << "Size: " << rs.size() << endl;
    for (r = 0; r < rs.size(); r++)
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
    }

    // conditions
    cout << "Test conditions" << endl;
    db_condition cond;
    cond.field() = "id";
    cond.operation() = db_condition::equal;
    cond.value() = 1;
    cursor = rs.begin();
    cout << "Condition: id == 1" << endl;
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
    cout << "Condition: value > 2.3" << endl;
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
    cout << "Condition: name like es" << endl;
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
    cout << "Condition: value is_null" << endl;
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

    return 0;
}
