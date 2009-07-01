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
#include <strstream>
#include <weLogger.h>
#include "messages.h"

int ProcessMessage(char* buff, size_t bufSize, session* sess)
{
    int retval = 0; // close connection
    Message msg;

    try
    {
        istrstream data(buff, bufSize);
        boost::archive::xml_iarchive ia(data);
        ia >> BOOST_SERIALIZATION_NVP(msg);
        LOG4CXX_INFO(WeLogger::GetLogger(), "ProcessMessage: " <<  msg.cmd);
        retval = 1;
        if (iequals(msg.cmd, "exit")) {
            retval = -1;
        }
    }
    catch (std::exception &e)
    {
    	LOG4CXX_ERROR(WeLogger::GetLogger(), "ProcessMessage failed: " <<  e.what());
        retval = 0;
    }

    return retval;
}

void server::handle_accept(session* new_session,
                   const boost::system::error_code& error)
{
    if (!error)
    {
        new_session->start();
        new_session = new session(io_service_);
        acceptor_.async_accept(new_session->socket(),
            boost::bind(&server::handle_accept, this, new_session,
            boost::asio::placeholders::error));
    }
    else
    {
        LOG4CXX_WARN(WeLogger::GetLogger(), "Server: accept failed - " << error);
        delete new_session;
    }
}


void session::start()
{
    LOG4CXX_INFO(WeLogger::GetLogger(), "Session: connection accepted - " << socket_.remote_endpoint().address());

    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code& error,
                 size_t bytes_transferred)
{
    if (!error)
    {
        int res = ProcessMessage(data_, bytes_transferred, this);
        if (res == 1)
        {
            boost::asio::async_write(socket_,
                boost::asio::buffer(data_, bytes_transferred),
                boost::bind(&session::handle_write, this,
                boost::asio::placeholders::error));
        }
        else if (res == 0)
        {
            LOG4CXX_WARN(WeLogger::GetLogger(), "Session: processing failed");
            delete this;
        }
        else // res == -1
        {
            LOG4CXX_WARN(WeLogger::GetLogger(), "Session: exit requested");
            socket_.io_service().stop();
        }
        
    }
    else
    {
        LOG4CXX_WARN(WeLogger::GetLogger(), "Session: read failed - " << error);
        delete this;
    }
}

void session::handle_write(const boost::system::error_code& error)
{
    if (!error)
    {
        //std::cout << "message write" << std::endl;
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&session::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        LOG4CXX_WARN(WeLogger::GetLogger(), "Session: write failed - " << error);
        delete this;
    }
}

