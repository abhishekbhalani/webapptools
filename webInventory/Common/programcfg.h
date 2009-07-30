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
#ifndef __PROGRAMCFG_H__
#define __PROGRAMCFG_H__
#include <string>
#include <list>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>

using namespace std;
using namespace boost;

class ProgramConfig
{
public:
    short   port;
    string  dbDir;
    string  storageIface;
    string  fileDB;
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(port);
        ar & BOOST_SERIALIZATION_NVP(dbDir);
        ar & BOOST_SERIALIZATION_NVP(storageIface);
        ar & BOOST_SERIALIZATION_NVP(fileDB);
    };
};

#endif //__PROGRAMCFG_H__
