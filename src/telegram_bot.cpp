#include "telegram_bot.hpp"
#include <thread>

using namespace grpc;

TelegramBot::TelegramBot(const std::string& token, int64_t chat_id, const std::string& client_address)
    : bot_(token), chat_id_(chat_id), is_running_(false) {
    stub_ = VideoStream::NewStub(CreateChannel(client_address, InsecureChannelCredentials()));
}

TelegramBot::~TelegramBot() {
    Stop();
}

void TelegramBot::Start()
 {
    if (!is_running_)
     {
        is_running_ = true;
        std::thread result_thread(&TelegramBot::ReceiveResults, this);
        result_thread.detach();
        std::cout << "Telegram bot started" << std::endl;
    }
}

void TelegramBot::Stop() 
{
    if (is_running_)
     {
        is_running_ = false;
        std::cout << "Telegram bot stopped" << std::endl;
    }
}

void TelegramBot::ReceiveResults()
 {
    ClientContext context;
    std::unique_ptr<ClientReaderWriter<Frame, Result>> stream(stub_->StreamVideo(&context));

    Frame dummy_frame; // Telegram bot doesn't send frames, just receives results
    stream->Write(dummy_frame); // Initiate stream
    stream->WritesDone();

    Result result;
    while (stream->Read(&result) && is_running_) 
    {
        try {
            bot_.getApi().sendMessage(chat_id_, "Result: " + result.data());
        } catch (const std::exception& e) {
            std::cerr << "Telegram error: " << e.what() << std::endl;
        }
    }

    Status status = stream->Finish();
    if (!status.ok()) {
        std::cerr << "Result stream failed: " << status.error_message() << std::endl;
    }
}