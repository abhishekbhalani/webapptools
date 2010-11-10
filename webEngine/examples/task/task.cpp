/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew Abramov aabramov@ptsecurity.ru

    This file is part of webEngine

    webEngineis free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngineis distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.

    @file   task.cpp
    @brief  Example for WeDocument class usage.
*/

// include headers that implement a archive in simple text format
#include <iostream>
#include <fstream>
#include <strstream>
#include "weHelper.h"
#include "weTask.h"
#include "weScan.h"
#include "weDispatch.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

using namespace std;
using namespace webEngine;

int main(int argc, char* argv[])
{
    we_option   opt1, opt2, opt3, opt4, opt5;
    string      str("sample");
    string      st2;
    int         iVal;
    char        cVal;

    LibInit(); // for initialize logging

    engine_dispatcher we_dispatcer;
    i_plugin* plg = we_dispatcer.load_plugin("mem_storage");
    if (plg != NULL) {
        i_storage* storage = (i_storage*)plg->get_interface("i_storage");
        storage->init_storage("task_db.txt");
        we_dispatcer.storage(storage);
    }

    opt1.name("testStr");
    opt1.SetValue(str);

    opt2.name("testInt");
    opt2.SetValue(2);

    opt3.name("testChar");
    opt3.SetValue(char(2));

    opt4.name("testInt");
    opt4.SetValue(2);

    opt5.name("testInt");
    opt5.SetValue(char(2));

    cout << "Opt1 type = " << opt1.GetTypeName() << endl;
    cout << "Opt2 type = " << opt2.GetTypeName() << endl;
    cout << "Opt3 type = " << opt3.GetTypeName() << endl;

    try {
        opt1.GetValue(st2);
        cout << opt1.name() << ": " << st2 << endl;
        opt2.GetValue(st2);
        cout << opt2.name() << ": " << st2 << endl;
        opt3.GetValue(st2);
        cout << opt3.name() << ": " << st2 << endl;
    } catch (...) {
        cout << "Exception!!!" << endl;
    }

    try {
        opt2.GetValue(iVal);
        cout << opt2.name() << ": " << iVal << endl;
        opt3.GetValue(iVal);
        cout << opt3.name() << ": " << iVal << endl;
        opt1.GetValue(iVal);
        cout << opt1.name() << ": " << iVal << endl;
    } catch (...) {
        cout << "Exception!!!" << endl;
    }


    try {
        opt3.GetValue(cVal);
        cout << opt3.name() << ": " << cVal << endl;
        opt2.GetValue(cVal);
        cout << opt2.name() << ": " << cVal << endl;
        opt1.GetValue(cVal);
        cout << opt1.name() << ": " << cVal << endl;
    } catch (...) {
        cout << "Exception!!!" << endl;
    }

    try {
        if (opt2 == opt4) {
            cout << "opt2 == opt4" << endl;
        } else {
            cout << "opt2 != opt4" << endl;
        }
        if (opt1 == opt4) {
            cout << "opt1 == opt4" << endl;
        } else {
            cout << "opt1 != opt4" << endl;
        }
        if (opt2 == opt5) {
            cout << "opt2 == opt5" << endl;
        } else {
            cout << "opt2 != opt5" << endl;
        }
    } catch (std::exception &ex) {
        cout << "Exception!!! " << ex.what() << endl;
    }

    std::ofstream ofs("options");

    // save data to archive
    {
        boost::archive::text_oarchive oa(ofs);
        // write class instance to archive
        // oa << opt1;
        // oa << opt2;
        // oa << opt3;
        // archive and stream closed when destructor are called
    }

    task tsk(&we_dispatcer);

    try {
        std::ifstream itfs("task");
        {
            boost::archive::xml_iarchive ia(itfs);
            // write class instance to archive
            // ia >> BOOST_SERIALIZATION_NVP(tsk);
            // archive and stream closed when destructor are called
        }
    } catch (...) {
        cout << "archive reading exception!" << endl;
    }

    string testOpt("test");
    cout << "Read \"test\" option: " << tsk.Option(testOpt).name() << " (" << tsk.Option(testOpt).GetTypeName() << ") -> ";
    if (!tsk.Option(testOpt).IsEmpty()) {
        tsk.Option(testOpt).GetValue(iVal);
        cout << iVal;
    } else {
        cout << "{empty}";
    }
    cout << endl;

    tsk.Option(testOpt, 3);

    cout << "Read \"test\" option: " << tsk.Option("test").name() << " (" << tsk.Option("test").GetTypeName() << ") -> ";
    if (!tsk.Option("test").IsEmpty()) {
        tsk.Option("test").GetValue(iVal);
        cout << iVal;
    } else {
        cout << "{empty}";
    }
    cout << endl;

    tsk.Option(testOpt, 20);

    cout << "Read \"test\" option: " << tsk.Option("test").name() << " (" << tsk.Option("test").GetTypeName() << ") -> ";
    if (!tsk.Option("test").IsEmpty()) {
        tsk.Option("test").GetValue(iVal);
        cout << iVal;
    } else {
        cout << "{empty}";
    }
    cout << endl;

    cout << "Read \"fakename\" option: " << tsk.Option("fakename").name() << " (" << tsk.Option("fakename").GetTypeName() << ") -> ";
    if (!tsk.Option("fakename").IsEmpty()) {
        tsk.Option("fakename").GetValue(iVal);
        cout << iVal;
    } else {
        cout << "{empty}";
    }
    cout << endl;

    // save data to simple XML
    tsk.Option(weoName, string("<test task>"));
    tsk.Option(weoID, string("&24'u\"3ou"));

    // direct access to DB
    if (we_dispatcer.storage() != NULL) {
        db_condition p_cond;

        p_cond.field() = weObjTypeProfile "." weoProfileID;
        p_cond.operation() = db_condition::great;
        p_cond.value() = string("");

        vector<string> fields;
        fields.push_back(weObjTypeProfile "." weoProfileID);
        fields.push_back(weObjTypeProfile "." weoName);
        fields.push_back(weObjTypeProfile "." weoTypeID);
        fields.push_back(weObjTypeProfile "." weoValue);

        db_cursor cursor = we_dispatcer.storage()->get(p_cond, fields);
        int r = 0;
        while (cursor.is_not_end()) {
            cout << "Record " << r << ":" << endl;
            for (size_t i = 0; i < fields.size(); i++) {
                cout << "\tField[" << i << "] = " << cursor[i] << endl;
            }
            ++cursor;
            ++r;
        }

    }

    scan_data_ptr scData = tsk.get_scan_data("http://www.ru");
    if (scData->data_id == "") {
        scData->data_id = we_dispatcer.storage()->generate_id(weObjTypeScan);
        scData->resp_code = 404;
        scData->download_time = 0;
        scData->data_size = 0;
        scData->scan_depth = 0;
        scData->content_type = "text/html";
    }
    tsk.set_scan_data("http://www.ru", scData);
    tsk.save_to_db();

    StringLinks lst;

    try {
        std::ifstream ilfs("list");
        {
            boost::archive::xml_iarchive ia(ilfs);
            // write class instance to archive
            ia >> BOOST_SERIALIZATION_NVP(lst);
            // archive and stream closed when destructor are called
        }
    } catch (...) {
        cout << "archive reading exception!" << endl;
    }

    lst.append("key1", "value1");
    lst.append("key2", "value2");
    lst.append("key3", "value3");

    std::ofstream lfs("list");

    // save data to archive
    {
        boost::archive::xml_oarchive oa(lfs);
        // write class instance to archive
        oa << BOOST_SERIALIZATION_NVP(lst);
        // archive and stream closed when destructor are called
    }

    if (we_dispatcer.storage() != NULL) {
        we_dispatcer.storage()->flush();
    }
    LibClose();

    return 0;
}

