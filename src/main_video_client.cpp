#include "video_client.hpp"

int main() 
{
    VideoClient client("localhost:50051");
    client.Start();
    client.StreamVideo();
    return 0;
}