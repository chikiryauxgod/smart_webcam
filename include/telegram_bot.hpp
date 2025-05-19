#ifndef TELEGRAM_BOT_HPP
#define TELEGRAM_BOT_HPP

#include <tgbot/tgbot.h>
#include <grpcpp/grpcpp.h>
#include "result_service.grpc.pb.h"

using namespace TgBot;
using result_service::VideoStream;
using result_service::Frame;
using result_service::Result;

class TelegramBot 
{
private:
    Bot bot_;
    int64_t chat_id_;
    std::unique_ptr<result_service::VideoStream::Stub> stub_;
    bool is_running_;

public:
    TelegramBot(const std::string& token, int64_t chat_id, const std::string& client_address);
    ~TelegramBot();
    void Start();
    void Stop();
    void ReceiveResults();
};

#endif // TELEGRAM_BOT_HPP