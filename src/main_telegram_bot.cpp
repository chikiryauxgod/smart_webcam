#include "telegram_bot.hpp"
#include <cstdlib>


std::string get_var(const std::string& var_name)
{
    const char* value = std::getenv(var_name.c_str());
    if (!value){
        std::cerr << "Error: environment variable" << var_name << "isn't expected" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return std::string(value);
}

int main() 
{
    std::string telegram_token = get_var("TG_SMART_TOKEN");
    std::string chat_id_str = get_var("TG_CHAT_ID");

    int64_t chat_id = 0;
    try {
        chat_id = std::stoll(chat_id_str);
    }
    catch (const std::exception& e){
        std::cerr << "Error: TG_CHAT_ID must be a valid integer, got" << chat_id_str << std::endl;
        std::exit(EXIT_FAILURE);
    }

    TelegramBot bot(telegram_token, chat_id, "localhost:50100");
    bot.Start();
    std::this_thread::sleep_for(std::chrono::hours(24)); // Keep running
    bot.Stop();
    return 0;
}