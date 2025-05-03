#include "video_client.hpp"
#include <string>

int main() {
    video_processor::VideoClient client("localhost:50052", "localhost:50051");
    client.Start();
    return 0;
}