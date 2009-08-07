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
#include <list>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>

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

typedef vector<TaskRecord> TaskList;

class PluginInfo
{
public:
    string  PluginId;
    string  PluginDesc;
    string  IfaceName;
    vector<string> IfaceList;
    vector<string> PluginIcon;
    int     PluginStatus;
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(PluginId);
        ar & BOOST_SERIALIZATION_NVP(PluginDesc);
        ar & BOOST_SERIALIZATION_NVP(IfaceName);
        ar & BOOST_SERIALIZATION_NVP(IfaceList);
        ar & BOOST_SERIALIZATION_NVP(PluginIcon);
        ar & BOOST_SERIALIZATION_NVP(PluginStatus);
    };
};

typedef vector<PluginInfo> PluginList;

class ScanInfo
{
public:
    string  ScanId;
    string  ObjectId;
    string  StartTime;
    string  FinishTime;
    int status;
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(ScanId);
        ar & BOOST_SERIALIZATION_NVP(ObjectId);
        ar & BOOST_SERIALIZATION_NVP(StartTime);
        ar & BOOST_SERIALIZATION_NVP(FinishTime);
        ar & BOOST_SERIALIZATION_NVP(status);
    };
};

typedef vector<ScanInfo> ScanList;

class ObjectInfo
{
public:
    string  ObjectId;
    string  Name;
    string  Address;
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(ObjectId);
        ar & BOOST_SERIALIZATION_NVP(Name);
        ar & BOOST_SERIALIZATION_NVP(Address);
    };
};

typedef vector<ObjectInfo> ObjectList;

#endif //__MESSAGES_H__
