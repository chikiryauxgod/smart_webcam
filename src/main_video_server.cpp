#include "video_server.hpp"
#include <string>

int main() 
{
    VideoServer server("localhost:50051");
    server.Start();
    server.Wait();
    return 0;
}