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
#include "server.h"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <weLogger.h>
#include "messages.h"
#include "version.h"

int ProcessMessage(boost::asio::streambuf* buff, size_t bufSize, session* sess)
{
    int retval = 0; // close connection
    bool processed = false;
    Message msg;

    try
    {
        buff->commit(bufSize);
        istream data(buff);
        boost::archive::xml_iarchive ia(data);
        ia >> BOOST_SERIALIZATION_NVP(msg);
        LOG4CXX_TRACE(WeLogger::GetLogger(), "[" << sess->socket().remote_endpoint().address() << "] ProcessMessage: " <<  msg.cmd);
        retval = 0;
        sess->last = posix_time::second_clock::local_time();
        if (iequals(msg.cmd, "exit"))
        {
            retval = -1;
            processed = true;
        }
        if (iequals(msg.cmd, "close"))
        {
            retval = 0;
            processed = true;
        }
        if (iequals(msg.cmd, "version"))
        {
            LOG4CXX_INFO(WeLogger::GetLogger(), "[" << sess->socket().remote_endpoint().address() << "] version requested");
            msg.data = AutoVersion::FULLVERSION_STRING;
            msg.data += " ";
            msg.data += AutoVersion::STATUS;
#ifdef _WIN32_WINNT
            msg.data += " (Windows)";
#else
            msg.data += " (Linux)";
#endif
            retval = 1;
            processed = true;
        }
        if (iequals(msg.cmd, "ping"))
        {
            msg.cmd = "pong";
            msg.data = posix_time::to_simple_string(sess->last);
            retval = 1;
            processed = true;
        }

        if (!processed)
        {
            LOG4CXX_WARN(WeLogger::GetLogger(), "[" << sess->socket().remote_endpoint().address() << "] unknown command: " <<  msg.cmd);
        }
        buff->consume(bufSize);
        {
            ostream oss(buff);
            boost::archive::xml_oarchive oa(oss);
            oa << BOOST_SERIALIZATION_NVP(msg);
        }
        size_t how = buff->size();
//        buff->assign((unsigned char *)(oss.rdbuf()->str()), oss.rdbuf()->pcount());
    }
    catch (std::exception &e)
    {
        LOG4CXX_ERROR(WeLogger::GetLogger(), "[" << sess->socket().remote_endpoint().address() << "ProcessMessage failed: " <<  e.what());
        retval = 0;
    }

    return retval;
}

