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
#ifndef __SERVER_H__
#define __SERVER_H__

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

using namespace boost;
using namespace boost::asio;
using boost::asio::ip::tcp;

//typedef boost::asio::ssl::stream<ip::tcp::socket> ssl_socket;
class session
{
public:
    session(boost::asio::io_service& io_service)
        : socket_(io_service)
    {
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void start();
    void handle_read(const boost::system::error_code& error,
        size_t bytes_transferred);
    void handle_write(const boost::system::error_code& error);

private:
    tcp::socket socket_;
    enum { max_length = 4096 };
    char data_[max_length];
};

class server
{
public:
    server(boost::asio::io_service& io_service, short port)
        : io_service_(io_service),
        acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    {
        session* new_session = new session(io_service_);
        acceptor_.async_accept(new_session->socket(),
            boost::bind(&server::handle_accept, this, new_session,
            boost::asio::placeholders::error));
    }

    void handle_accept(session* new_session,
        const boost::system::error_code& error);

private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
};

extern int ProcessMessage(char* buff, size_t bufSize, session* sess);

#endif //__SERVER_H__
