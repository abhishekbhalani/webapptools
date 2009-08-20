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
// #include <boost/archive/xml_iarchive.hpp>
#include <strstream>
#include <weLogger.h>
#include <weBlob.h>
// #include "messages.h"

using namespace webEngine;

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
        LOG4CXX_WARN(iLogger::GetLogger(), "Server: accept failed - " << error);
        delete new_session;
    }
}


void session::start()
{
    LOG4CXX_INFO(iLogger::GetLogger(), "Session: connection accepted - " << socket_.remote_endpoint().address());

    socket_.async_read_some(boost::asio::buffer(&datalen, sizeof(size_t)),
        boost::bind(&session::handle_read_datalen, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void session::handle_read_datalen(const boost::system::error_code& error, size_t bytes_transferred)
{
    if (datalen > 0 && !error)
    {
        data_ = new char[datalen];
        socket_.async_read_some(boost::asio::buffer(data_, datalen),
            boost::bind(&session::handle_read_data, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        LOG4CXX_WARN(iLogger::GetLogger(), "Session: read failed - " << error);
        delete this;
    }
}

void session::handle_read_data(const boost::system::error_code& error, size_t bytes_transferred)
{
    if (datalen > 0 && !error)
    {
        int res = process_message(data_, datalen, this);
        if (res == 1)
        {
            boost::asio::write(socket_, boost::asio::buffer(&datalen, sizeof(size_t)));
            boost::asio::async_write(socket_, boost::asio::buffer(data_, datalen),
                boost::bind(&session::handle_write, this,
                boost::asio::placeholders::error));
        }
        else if (res == 0)
        {
            LOG4CXX_WARN(iLogger::GetLogger(), "Session: close session");
            delete this;
        }
        else // res == -1
        {
            LOG4CXX_WARN(iLogger::GetLogger(), "Session: exit requested");
            socket_.io_service().stop();
        }
    }
    else
    {
        LOG4CXX_WARN(iLogger::GetLogger(), "Session: read failed - " << error);
        delete this;
    }
}

void session::handle_write(const boost::system::error_code& error)
{
    if (!error)
    {
        reset_buffer(0);
        // Start reading remaining data until EOF.
        socket_.async_read_some(boost::asio::buffer(&datalen, sizeof(size_t)),
            boost::bind(&session::handle_read_datalen, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        LOG4CXX_WARN(iLogger::GetLogger(), "Session: write failed - " << error);
        delete this;
    }
}

void session::reset_buffer(size_t sz)
{
    datalen = sz;
    if (data_ != NULL)
    {
        delete data_;
        data_ = NULL;
        if (datalen > 0)
        {
            data_ = new char[datalen];
        }
    }
}

void session::assign_buffer(size_t sz, char* buff)
{
    reset_buffer(sz);
    if (data_ != NULL)
    {
        memcpy(data_, buff, sz);
    }
}
