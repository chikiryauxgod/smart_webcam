#include "telegram_bot.hpp"
#include <cstdlib>
#include <thread>
#include <chrono>
#include <csignal>
#include <iostream>

std::string get_var(const std::string& var_name) {
    const char* value = std::getenv(var_name.c_str());
    if (!value) {
        std::cerr << "Error: environment variable " << var_name << " is not set" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return std::string(value);
}

volatile std::sig_atomic_t running = 1;

void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Received SIGINT, stopping bot..." << std::endl;
        running = 0;
    }
}

int main() {
    std::string telegram_token = get_var("TG_SMART_TOKEN");
    std::string chat_id_str = get_var("TG_CHAT_ID");

    int64_t chat_id = 0;
    try {
        chat_id = std::stoll(chat_id_str);
    } catch (const std::exception& e) {
        std::cerr << "Error: TG_CHAT_ID must be a valid integer, got " << chat_id_str << std::endl;
        std::exit(EXIT_FAILURE);
    }

    TelegramBot bot(telegram_token, chat_id, "localhost:50051");
    bot.Start();

    signal(SIGINT, signal_handler);

    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    bot.Stop();
    std::cout << "Telegram bot terminated gracefully" << std::endl;
    return 0;
}