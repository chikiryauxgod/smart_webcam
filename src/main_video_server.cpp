#include "video_server.hpp"
#include <string>

int main() {
    result_service::VideoServer server("localhost:50051");
    server.Start();
    return 0;
}