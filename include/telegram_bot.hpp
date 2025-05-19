#ifndef TELEGRAM_BOT_HPP
#define TELEGRAM_BOT_HPP

#include "result_service.grpc.pb.h"
#include <tgbot/tgbot.h>
#include <grpcpp/grpcpp.h>
#include <atomic>
#include <thread>

class TelegramBot 
{
public:
    TelegramBot(const std::string& token,
                int64_t chat_id,
                const std::string& server_address);
    ~TelegramBot();

    void Start();
    void Stop();

private:
    void ReceiveResults();

    TgBot::Bot bot_;
    int64_t chat_id_;
    std::string server_address_;
    std::atomic<bool> is_running_;
    std::unique_ptr<result_service::VideoStream::Stub> stub_;
    std::thread result_thread_;
};

#endif // TELEGRAM_BOT_HPP
