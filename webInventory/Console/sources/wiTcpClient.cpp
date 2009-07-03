/*
    webInventory is the web-application audit programm
    Copyright (C) 2009 Andrew Abramov stinger911@gmail.com

    This file is part of webInventory

    webInventory is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webInventory is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with webInventory.  If not, see <http://www.gnu.org/licenses/>.
*/
/***************************************************************
 * @file      wiTcpClient.cpp
 * @brief     Code for wiTcpClient class
 * @author    Andrew "Stinger" Abramov (stinger911@gmail.com)
 * @date      02.07.2009
 **************************************************************/
#include "wiTcpClient.h"

#include <iostream>
#include <strstream>
#include <boost/asio.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

using namespace std;
using namespace boost;
using boost::asio::ip::tcp;

class wiInternalTcp
{
public:
    wiInternalTcp(const char* host, const char* port);
    ~wiInternalTcp();

    boost::asio::io_service ioService;
    tcp::resolver tcpResolver;
    tcp::resolver::query tcpQuery;
    tcp::resolver::iterator tcpIterator;
    tcp::socket sock;
    unsigned timeout;
};

wiInternalTcp::wiInternalTcp(const char* host, const char* port) :
    tcpResolver(ioService),
    tcpQuery(tcp::v4(), host, port),
    sock(ioService)
{
    tcpIterator = tcpResolver.resolve(tcpQuery);
    timeout = 10000; // 10 seconds
}

wiInternalTcp::~wiInternalTcp()
{
    sock.close();
}

wiTcpClient::wiTcpClient(const char* host, const char* port)
{
    lastError = wxT("");
    client = new wiInternalTcp(host, port);
}

wiTcpClient::~wiTcpClient()
{
    delete client;
}

bool wiTcpClient::Connect()
{
    boost::system::error_code ec;

    client->sock.connect(*(client->tcpIterator), ec);

    return (bool)ec;
}

bool wiTcpClient::Ping()
{
    return !(DoCmd(wxT("ping"), wxT("")).IsEmpty());
}

wxString wiTcpClient::GetScannerVersion()
{
    return DoCmd(wxT("version"), wxT(""));
}

wxString wiTcpClient::DoCmd(const wxString& cmd, const wxString& payload)
{
    wxString result = wxT("");
    try
    {
        Message msg;
        char *databuff;

        msg.cmd = (char*)cmd.ToAscii().data();
        msg.data = (char*)payload.To8BitData().data();
        ostrstream oss;
        {
            boost::archive::xml_oarchive oa(oss);
            oa << BOOST_SERIALIZATION_NVP(msg);
        }
        std::string plain = string(oss.str(), oss.pcount());
        size_t request_length = plain.length();
        boost::asio::write(client->sock, boost::asio::buffer(plain.c_str(), request_length));
        boost::asio::streambuf reply;
        size_t reply_length = boost::asio::read_until(client->sock, reply, '\0');
        databuff = new char(reply_length + 1);
        istream iss(&reply);
        {
            boost::archive::xml_iarchive ia(iss);
            ia >> BOOST_SERIALIZATION_NVP(msg);
        }
        result = wxString::FromAscii(msg.data.c_str());
        //delete databuff;
    }
    catch (std::exception& e)
    {
        if (cmd != wxT("close") && cmd != wxT("exit")) {
            lastError = wxString::FromAscii(e.what());
            result = wxT("");
            // try to reconnect
            try {
                client->sock.close();
                client->sock.connect(*(client->tcpIterator));
            } catch (std::exception& e2) {
                lastError = wxString::FromAscii(e2.what());
                result = wxT("");
            }
        }
    }
    return result;
}

TaskList* wiTcpClient::GetTaskList(const wxString& criteria/* = wxT("")*/)
{
    TaskList* lst = NULL;
    wxString reply;
    char* data;
    try
    {
        reply = DoCmd(wxT("tasks"), criteria);
        if (!reply.IsEmpty()) {
            lst = new TaskList;
            data = strdup((char*)reply.ToAscii().data());
            istrstream iss(data);
            {
                boost::archive::xml_iarchive ia(iss);
                ia >> BOOST_SERIALIZATION_NVP(*lst);
            }
            delete data;
        }
    }
    catch (std::exception& e)
    {
        lastError = wxString::FromAscii(e.what());
        if (lst != NULL) {
            delete lst;
        }
        lst = NULL;
    }
    return lst;
}
