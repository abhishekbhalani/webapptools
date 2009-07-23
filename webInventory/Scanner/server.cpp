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
// #include <boost/algorithm/string/predicate.hpp>
// #include <boost/archive/xml_iarchive.hpp>#include <strstream>
#include <weLogger.h>
// #include "messages.h"

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

    boost::asio::async_read_until(socket_, data_, "\0",
        //boost::asio::transfer_at_least(1),
        boost::bind(&session::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code& error,
                 size_t bytes_transferred)
{
    if (!error)
    {
        int res = process_message(&data_, bytes_transferred, this);
        if (res == 1)
        {
            boost::asio::async_write(socket_, data_,
                boost::bind(&session::handle_write, this,
                boost::asio::placeholders::error));
        }
        else if (res == 0)
        {
            LOG4CXX_WARN(WeLogger::GetLogger(), "Session: close session");
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
        // finalize message
        short nul = 0;
        boost::asio::write(socket_, boost::asio::buffer(&nul, sizeof(nul)));
        // clear buffer
        data_.consume(data_.size());
        // Start reading remaining data until EOF.
        boost::asio::async_read_until(socket_, data_, "\0",
            //boost::asio::transfer_at_least(1),
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

