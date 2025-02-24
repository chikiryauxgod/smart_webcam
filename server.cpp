#include "server.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <opencv4/opencv2/opencv.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Session : public std::enable_shared_from_this<Session>
{
public:

	Session(tcp::socket socket, ssl::context& ssl) : stream_(std::move(socket), ssl), cap_(0) {}

private:

	ssl::stream<tcp::socket> stream_;
	cv::Video 					cap_;	

};
