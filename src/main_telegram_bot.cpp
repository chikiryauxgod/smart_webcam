#include "telegram_bot.hpp"

int main() {
    std::string telegram_token = "TOKEN";
    int64_t chat_id = CHAT_ID; 
    TelegramBot bot(telegram_token, chat_id, "localhost:50053");
    bot.Start();
    std::this_thread::sleep_for(std::chrono::hours(24)); // Keep running
    bot.Stop();
    return 0;
}