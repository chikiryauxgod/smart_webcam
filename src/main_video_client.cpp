#include "video_client.hpp"
#include <signal.h>
#include <atomic>
#include <thread>

std::atomic<bool> running(true);

void signal_handler(int) {
    running = false;
}

int main() 
{
    VideoClient client("localhost:50051"); 
    client.Start();

    signal(SIGINT, signal_handler);

    client.StreamVideo();

    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    client.Stop();
    return 0;
}