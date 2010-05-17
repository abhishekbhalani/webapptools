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

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

using namespace std;
using namespace webEngine;

int main(int argc, char* argv[])
{
    wOption     opt1, opt2, opt3;
    string      str("sample");
    string      st2;
    int         iVal;
    char        cVal;

    LibInit(); // for initialize logging

    opt1.name("testStr");
    opt1.SetValue(str);

    opt2.name("testInt");
    opt2.SetValue(2);

    opt3.name("testChar");
    opt3.SetValue(char(2));

    cout << "Opt1 type = " << opt1.GetTypeName() << endl;
    cout << "Opt2 type = " << opt2.GetTypeName() << endl;
    cout << "Opt3 type = " << opt3.GetTypeName() << endl;

    try
    {
        opt1.GetValue(st2);
        cout << opt1.name() << ": " << st2 << endl;
        opt2.GetValue(st2);
        cout << opt2.name() << ": " << st2 << endl;
        opt3.GetValue(st2);
        cout << opt3.name() << ": " << st2 << endl;
    }
    catch (...)
    {
    	cout << "Exception!!!" << endl;
    }

    try
    {
        opt2.GetValue(iVal);
        cout << opt2.name() << ": " << iVal << endl;
        opt3.GetValue(iVal);
        cout << opt3.name() << ": " << iVal << endl;
        opt1.GetValue(iVal);
        cout << opt1.name() << ": " << iVal << endl;
    }
    catch (...)
    {
        cout << "Exception!!!" << endl;
    }


    try
    {
        opt3.GetValue(cVal);
        cout << opt3.name() << ": " << cVal << endl;
        opt2.GetValue(cVal);
        cout << opt2.name() << ": " << cVal << endl;
        opt1.GetValue(cVal);
        cout << opt1.name() << ": " << cVal << endl;
    }
    catch (...)
    {
        cout << "Exception!!!" << endl;
    }

    std::ofstream ofs("options");

    // save data to archive
    {
        boost::archive::text_oarchive oa(ofs);
        // write class instance to archive
        oa << opt1;
        oa << opt2;
        oa << opt3;
        // archive and stream closed when destructor are called
    }

    task tsk;

    try
    {
        std::ifstream itfs("task");
        {
            boost::archive::xml_iarchive ia(itfs);
            // write class instance to archive
            // ia >> BOOST_SERIALIZATION_NVP(tsk);
            // archive and stream closed when destructor are called
        }
    }
    catch (...)
    {
    	cout << "archive reading exception!" << endl;
    }

    string testOpt("test");
    cout << "Read \"test\" option: " << tsk.Option(testOpt).name() << " (" << tsk.Option(testOpt).GetTypeName() << ") -> ";
    if (!tsk.Option(testOpt).IsEmpty()) {
        tsk.Option(testOpt).GetValue(iVal);
        cout << iVal;
    }
    else {
        cout << "{empty}";
    }
    cout << endl;

    tsk.Option(testOpt, 3);

    cout << "Read \"test\" option: " << tsk.Option("test").name() << " (" << tsk.Option("test").GetTypeName() << ") -> ";
    if (!tsk.Option("test").IsEmpty()) {
        tsk.Option("test").GetValue(iVal);
        cout << iVal;
    }
    else {
        cout << "{empty}";
    }
    cout << endl;

    cout << "Read \"fakename\" option: " << tsk.Option("fakename").name() << " (" << tsk.Option("fakename").GetTypeName() << ") -> ";
    if (!tsk.Option("fakename").IsEmpty()) {
        tsk.Option("fakename").GetValue(iVal);
        cout << iVal;
    }
    else {
        cout << "{empty}";
    }
    cout << endl;


    // save data to archive
    {
        std::ofstream tfs("task");
        boost::archive::xml_oarchive oa(tfs);
        // write class instance to archive
        // oa << BOOST_SERIALIZATION_NVP(tsk);
        // archive and stream closed when destructor are called
    }

    // save data to simple XML
    tsk.Option(weoName, string("<test task>"));
    tsk.Option(weoID, string("&24'u\"3ou"));
    {
        std::ofstream tfs("task.xml");
        std::string xml = ""; //tsk.ToXml();
        // write class instance to archive
        tfs << xml;
        // archive and stream closed when destructor are called
    }
    // restore data from simple XML
    {
        std::ifstream tfs("task.xml");
        std::string xml;
        // read class instance from archive
        if (!tfs.bad())
        {
            ostrstream oss;
            oss << tfs.rdbuf();
            xml = string(oss.str(), oss.rdbuf()->pcount());
        }
        //tsk.FromXml(xml);
        // archive and stream closed when destructor are called
    }
    // save data to archive
    {
        std::ofstream tfs("task2");
        boost::archive::xml_oarchive oa(tfs);
        // write class instance to archive
        // oa << BOOST_SERIALIZATION_NVP(tsk);
        // archive and stream closed when destructor are called
    }

    StringLinks lst;

    try
    {
        std::ifstream ilfs("list");
        {
            boost::archive::xml_iarchive ia(ilfs);
            // write class instance to archive
            ia >> BOOST_SERIALIZATION_NVP(lst);
            // archive and stream closed when destructor are called
        }
    }
    catch (...)
    {
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

    LibClose();

    return 0;
}

