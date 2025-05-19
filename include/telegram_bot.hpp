#ifndef TELEGRAM_BOT_HPP
#define TELEGRAM_BOT_HPP

#include "result_service.grpc.pb.h"
#include <tgbot/tgbot.h>
#include <grpcpp/grpcpp.h>
#include <memory>

class TelegramBot {
public:
    TelegramBot(const std::string& token, int64_t chat_id, const std::string& client_address);
    ~TelegramBot();
    void Start();
    void Stop();
    void Wait();

private:
    void ReceiveResults();
    TgBot::Bot bot_;
    int64_t chat_id_;
    std::string client_address_;
    bool is_running_;
    std::unique_ptr<result_service::VideoStream::Stub> stub_;
    std::shared_ptr<grpc::Channel> channel_;
    grpc::ClientContext* context_ = nullptr;
    std::thread result_thread_;
};

#endif // TELEGRAM_BOT_HPP