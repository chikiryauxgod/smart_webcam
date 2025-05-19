#include "telegram_bot.hpp"
#include <cstdlib>
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

static std::atomic<bool> running{true};

void signal_handler(int sig) {
    if (sig == SIGINT) running = false;
}

std::string get_env(const char* var)
 {
    const char* v = std::getenv(var);
    if (!v) {
        std::cerr << "Env var " << var << " not set\n";
        std::exit(1);
    }
    return v;
}

int main() {
    std::signal(SIGINT, signal_handler);

    std::string token   = get_env("TG_SMART_TOKEN");
    int64_t     chat_id = std::stoll(get_env("TG_CHAT_ID"));
    std::string srv     = "localhost:50051";

    TelegramBot bot(token, chat_id, srv);
    bot.Start();

    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    bot.Stop();
    std::cout << "Telegram bot stopped\n";
    return 0;
}
