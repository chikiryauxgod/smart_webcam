#include "telegram_bot.hpp"
#include <thread>
#include <string>
#include <sstream>
#include <iostream>
#include <chrono>

using namespace grpc;
using namespace result_service;

TelegramBot::TelegramBot(const std::string& token, int64_t chat_id, const std::string& client_address)
    : bot_(token), chat_id_(chat_id), client_address_(client_address), is_running_(false), stub_(nullptr), channel_(nullptr), context_(nullptr) {
    std::cout << "TelegramBot constructor called with address: " << client_address << std::endl;
}

TelegramBot::~TelegramBot() {
    Stop();
}

void TelegramBot::Start() {
    if (!is_running_) {
        is_running_ = true;

        // Отправляем тестовое сообщение при запуске
        try {
            bot_.getApi().sendMessage(chat_id_, "Bot get started!");
            std::cout << "Test message 'Bot get started!' sent to Telegram chat ID: " << chat_id_ << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to send test message: " << e.what() << std::endl;
        }

        // Инициализация gRPC
        std::cout << "Creating gRPC channel for " << client_address_ << std::endl;
        channel_ = CreateChannel(client_address_, InsecureChannelCredentials());
        if (!channel_) {
            std::cerr << "Failed to create gRPC channel" << std::endl;
            return;
        }
        std::cout << "gRPC channel created successfully" << std::endl;

        // Ожидание подключения канала с повторными попытками
        int max_attempts = 3;
        int attempt = 0;
        grpc_connectivity_state state = channel_->GetState(false);
        std::cout << "Initial channel state: " << state << std::endl;

        while (state != GRPC_CHANNEL_READY && attempt < max_attempts) {
            std::cout << "Attempt " << (attempt + 1) << ": Waiting for gRPC channel to become ready..." << std::endl;
            channel_->WaitForStateChange(state, gpr_timespec{10, 0, GPR_TIMESPAN}); // Ждём до 10 секунд
            state = channel_->GetState(true);
            std::cout << "Channel state after waiting: " << state << std::endl;
            attempt++;
            if (state != GRPC_CHANNEL_READY && attempt < max_attempts) {
                std::cout << "Retrying in 2 seconds..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        }

        if (state != GRPC_CHANNEL_READY) {
            std::cerr << "Failed to connect to video server after " << max_attempts << " attempts: state = " << state << std::endl;
            return;
        }
        std::cout << "gRPC channel is ready" << std::endl;

        stub_ = std::make_unique<result_service::VideoStream::Stub>(channel_);
        if (!stub_) {
            std::cerr << "Failed to create gRPC stub" << std::endl;
            return;
        }
        std::cout << "gRPC stub created successfully" << std::endl;

        result_thread_ = std::thread(&TelegramBot::ReceiveResults, this);
        std::cout << "Telegram bot started" << std::endl;
    }
}

void TelegramBot::Stop() {
    if (is_running_) {
        is_running_ = false;
        if (context_) {
            context_->TryCancel();
        }
        if (result_thread_.joinable()) {
            result_thread_.join();
        }
        stub_.reset();
        channel_.reset();
        std::cout << "Telegram bot stopped" << std::endl;
    }
}

void TelegramBot::Wait() {
    if (result_thread_.joinable()) {
        result_thread_.join();
    }
}

void TelegramBot::ReceiveResults() {
    std::cout << "ReceiveResults thread started" << std::endl;

    ClientContext context;
    context_ = &context;
    std::cout << "Opening gRPC stream..." << std::endl;
    std::unique_ptr<ClientReaderWriter<result_service::Frame, result_service::Result>> stream(stub_->StreamVideo(&context));
    if (!stream) {
        std::cerr << "Failed to open gRPC stream" << std::endl;
        return;
    }
    std::cout << "gRPC stream opened successfully" << std::endl;

    // Отправляем пустые кадры, чтобы поддерживать поток
    std::thread writer([this, &stream]() {
        std::cout << "Writer thread started" << std::endl;
        result_service::Frame empty_frame;
        empty_frame.set_width(0);
        empty_frame.set_height(0);
        while (is_running_) {
            if (!stream->Write(empty_frame)) {
                std::cerr << "Failed to write empty frame to stream" << std::endl;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        stream->WritesDone();
        std::cout << "Writer thread finished" << std::endl;
    });

    result_service::Result result;
    std::cout << "Reading from gRPC stream..." << std::endl;
    while (stream->Read(&result) && is_running_) {
        std::cout << "Received result from stream: " << result.data() << std::endl;
        std::string message = result.data();
        if (message.empty()) {
            continue;
        }

        std::istringstream iss(message);
        std::string token;
        bool personDetected = false;
        while (iss >> token) {
            if (token.find("person") != std::string::npos || token.find("stranger") != std::string::npos) {
                personDetected = true;
                break;
            }
        }

        if (personDetected) {
            try {
                bot_.getApi().sendMessage(chat_id_, "A person has been detected in the frame!");
                std::cout << "Message sent to Telegram: A person has been detected in the frame!" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Telegram error: " << e.what() << std::endl;
            }
        }
    }

    writer.join();
    Status status = stream->Finish();
    if (!status.ok()) {
        std::cerr << "Result stream failed: " << status.error_message() << std::endl;
    }
    context_ = nullptr;
    std::cout << "ReceiveResults thread finished" << std::endl;
}