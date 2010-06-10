// dbtest.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <weHelper.h>
#include <weiStorage.h>

using namespace std;
using namespace webEngine;

int main(int argc, char* argv[])
{
    vector<string> fnames;

    fnames.push_back("id");
    fnames.push_back("name");
    fnames.push_back("value");

    db_recordset rs(fnames);
    db_record rec;
    rec.push_back(1);
    rec.push_back(string("test"));
    rec.push_back(2.5);
    rs.push_back(rec);

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

	return 0;
}
