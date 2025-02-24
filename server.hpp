// 24.02.2025 
// Server.hpp
// io = io_context
// ssl = ssl_context


#ifndef SERVER_HPP
#define SERVER_HPP

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

using boost::asio::ip::tcp;
using ssl = boost::asio::ssl;


class Server
{
public:

	Server(boost::asio::asio::io_context& io, unsigned short port, ssl::context& ssl);
	void run();

private:
	void do_accept();

	tcp::acceptor acceptor_;
	ssl::context& 	   ssl_;
};

#endif 
