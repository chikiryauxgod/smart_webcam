// src/telegram_bot.cpp
#include "telegram_bot.hpp"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <opencv2/opencv.hpp>

using grpc::ClientContext;
using grpc::ClientReaderWriter;
using grpc::CreateChannel;
using grpc::InsecureChannelCredentials;
using namespace result_service;

TelegramBot::TelegramBot(const std::string& token,
                         int64_t chat_id,
                         const std::string& server_address)
    : bot_(token),
      chat_id_(chat_id),
      server_address_(server_address),
      is_running_(false)
{
    auto channel = CreateChannel(server_address_, InsecureChannelCredentials());
    stub_ = VideoStream::NewStub(channel);
}

TelegramBot::~TelegramBot() {
    Stop();
}

void TelegramBot::Start() {
    is_running_ = true;
    result_thread_ = std::thread(&TelegramBot::ReceiveResults, this);
}

void TelegramBot::Stop() 
{
    is_running_ = false;
    if (result_thread_.joinable()) {
        result_thread_.join();
    }
}

void TelegramBot::ReceiveResults() 
{
    ClientContext context;
    auto stream = stub_->StreamVideo(&context);

    std::thread writer([&]()
    {
        cv::VideoCapture cap("/dev/video0", cv::CAP_V4L2);
        if (!cap.isOpened())
        {
            std::cerr << "TelegramBot: fail to connect /dev/video0\n";
            stream->WritesDone();
            return;
        }
        result_service::Frame frame_proto;
        cv::Mat frame;
        while (is_running_ && cap.read(frame)) 
        {
            frame_proto.set_width(frame.cols);
            frame_proto.set_height(frame.rows);
            frame_proto.set_image_data(
                reinterpret_cast<const char*>(frame.data),
                frame.total() * frame.elemSize()
            );
            if (!stream->Write(frame_proto)) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }
        stream->WritesDone();
    });

    result_service::Result res;
    while (is_running_ && stream->Read(&res)) 
    {
        const auto& msg = res.data();
        if (msg.empty()) continue;
        std::cout << "[TelegramBot] " << msg << "\n";
        try {
            bot_.getApi().sendMessage(chat_id_, msg);
        } catch (const std::exception& e) {
            std::cerr << "Telegram error: " << e.what() << "\n";
        }
    }

    writer.join();
    auto status = stream->Finish();
    if (!status.ok()) {
        std::cerr << "gRPC stream error: " << status.error_message() << "\n";
    }
}
