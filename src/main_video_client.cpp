#include "video_client.hpp"
#include <signal.h>
#include <atomic>
#include <thread>

std::atomic<bool> running(true);

void signal_handler(int) {
    running = false;
}

int main() {
    VideoClient client("localhost:50052", "localhost:50051"); // Клиент слушает на 50052, соединяется с сервером на 50051
    client.Start();

    signal(SIGINT, signal_handler); // Обработка Ctrl+C

    client.StreamVideo(); // Запуск стриминга

    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Ожидание, пока не прервут
    }

    client.Stop();
    return 0;
}