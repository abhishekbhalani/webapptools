/*
    inventoryScanner is the web-application audit program
    Copyright (C) 2009 Andrew "Stinger" Abramov stinger911@gmail.com

    This file is part of inventoryScanner

    inventoryScanner is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    inventoryScanneris distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with inventoryScanner.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __MESSAGES_H__
#define __MESSAGES_H__
#include <string>
#include <boost/serialization/vector.hpp>

using namespace std;
using namespace boost;

class Message
{
public:
    string cmd;
    string data;
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(cmd);
        ar & BOOST_SERIALIZATION_NVP(data);
    };
};

class TaskRecord
{
public:
    string name;
    int status;
    int completion;
    string id;
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(name);
        ar & BOOST_SERIALIZATION_NVP(status);
        ar & BOOST_SERIALIZATION_NVP(completion);
        ar & BOOST_SERIALIZATION_NVP(id);
    };
};

#define WI_TSK_IDLE     0
#define WI_TSK_RUN      1
#define WI_TSK_PAUSED   2
#define WI_TSK_MAX      3

class TaskList
{
public:
    vector<TaskRecord>  task;
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(task);
    };
};

#endif //__MESSAGES_H__
