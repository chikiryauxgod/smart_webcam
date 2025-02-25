#include "server.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
// #include <opencv4/opencv2/opencv.hpp>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
using json = nlohmann::json;

class Session : public std::enable_shared_from_this<Session>
{
public:

	Session(tcp::socket socket, ssl::context& ssl) : stream_(std::move(socket), ssl), cap_(0) {}
	void start();

private:

	ssl::stream<tcp::socket> stream_;
	cv::Video 					cap_;	
	void start_stream();
	void send_frame();
	// void Server::run();
	// void Server::do_accept();
};

Session::start()
{
	auto self(shared_from_this());
	stream_.async_handshake(ssl::stream_base::server, [this, self] (const boost::system::error_code& ec){
				if (!ec){
					start_stream();}
				else{
				std::cerr << "Handshake error: " << ec.message() << std::endl;}
				});
}


Session::start_stream()
{
	if (!cap_.isOpened())
	{
		cap_.open(0); // 0 - camera device in system
		if (!cap_.isOpened()){
			std::cerr << "Couldn't open the cam." << std::endl;
			return;
		}	
	}
	send_frame();	
}















}



