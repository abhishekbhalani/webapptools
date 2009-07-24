//
// blocking_tcp_echo_client.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <strstream>
#include <boost/asio.hpp>
//#include <boost/serialization.hpp>
#include <boost/archive/xml_oarchive.hpp>

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

using boost::asio::ip::tcp;

enum { max_length = 8192 };

int main(int argc, char* argv[])
{
    try
    {
        Message msg;
        string plain;
        if (argc != 3)
        {
            std::cerr << "Usage: blocking_tcp_echo_client <host> <port>\n";
            return 1;
        }

        boost::asio::io_service io_service;

        tcp::resolver resolver(io_service);
        tcp::resolver::query query(tcp::v4(), argv[1], argv[2]);
        tcp::resolver::iterator iterator = resolver.resolve(query);

        tcp::socket s(io_service);
        s.connect(*iterator);

        using namespace std; // For strlen.
        std::cout << "Enter commnad: ";
        char *request = new char[max_length];
        std::cin.getline(request, max_length);
        msg.cmd = request;
        std::cout << "Enter parameter: ";
        std::cin.getline(request, max_length);
        msg.data = request;
        ostrstream oss;
        {
            boost::archive::xml_oarchive oa(oss);
            oa << BOOST_SERIALIZATION_NVP(msg);
        }
        plain = string(oss.str(), oss.pcount());
        size_t request_length = plain.length();
        boost::asio::write(s, boost::asio::buffer(plain.c_str(), request_length));
        boost::asio::streambuf reply;
        size_t reply_length = boost::asio::read_until(s, reply, '\0');
        delete request;
        request = new char[reply_length + 10];
        istream is(&reply);
        is.read(request, reply_length);
        std::cout << "Reply is: ";
        std::cout << request;
        std::cout << "\n";
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
