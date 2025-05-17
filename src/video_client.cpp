#include "video_client.hpp"
#include <iostream>

using namespace grpc;

VideoClientService::VideoClientService(std::string_view ai_server_address)
    : ai_stub_(VideoProcessor::NewStub(CreateChannel(std::string(ai_server_address), InsecureChannelCredentials()))) {
}

Status VideoClientService::StreamVideo(ServerContext* context, ServerReaderWriter<result_service::Result, result_service::Frame>* stream) {
    // need to declarate
    return Status::OK;
}

VideoClient::VideoClient(std::string_view server_address, std::string_view video_server_address)
    : is_running_(false) 
    {
    ServerBuilder builder;
    builder.AddListeningPort(std::string(server_address), InsecureServerCredentials());
    builder.RegisterService(new VideoClientService(video_server_address));
    server_ = builder.BuildAndStart();
}

VideoClient::~VideoClient() {
    Stop();
}

void VideoClient::Start()
 {
    if (!is_running_) {
        is_running_ = true;
        std::cout << "Video client started" << std::endl;
    }
}

void VideoClient::Stop()
 {
    if (is_running_) {
        server_->Shutdown();
        is_running_ = false;
        std::cout << "Video client stopped" << std::endl;
    }
}

void VideoClient::StreamVideo() {
    // need to declarate
}