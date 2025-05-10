#include "video_client.hpp"
#include <thread>

VideoClientService::VideoClientService(const std::string& ai_server_address)
    : ai_stub_(VideoProcessor::NewStub(CreateChannel(ai_server_address, InsecureChannelCredentials()))) {}

Status VideoClientService::ProcessVideo(ServerContext* context, ServerReaderWriter<Result, Frame>* stream) {
    ClientContext ai_context;
    std::unique_ptr<ClientReaderWriter<Frame, Result>> ai_stream(ai_stub_->ProcessVideo(&ai_context));

    Frame frame;
    while (stream->Read(&frame)) {
        ai_stream->Write(frame); 
    }
    ai_stream->WritesDone();

    Result result;
    while (ai_stream->Read(&result)) {
        stream->Write(result); 
    }

    Status status = ai_stream->Finish();
    if (!status.ok()) {
        std::cerr << "AI stream failed: " << status.error_message() << std::endl;
    }
    return Status::OK;
}

VideoClient::VideoClient(const std::string& server_address, const std::string& video_server_address)
    : is_running_(false) {
    ServerBuilder builder;
    builder.AddListeningPort(server_address, InsecureServerCredentials());
    builder.RegisterService(new VideoClientService("localhost:50051"));
    server_ = builder.BuildAndStart();

    video_stub_ = VideoStream::NewStub(CreateChannel(video_server_address, InsecureChannelCredentials()));
}

VideoClient::~VideoClient() {
    Stop();
}

void VideoClient::Start() {
    if (!is_running_) {
        is_running_ = true;
        std::cout << "Video client started" << std::endl;
    }
}

void VideoClient::Stop() {
    if (is_running_) {
        server_->Shutdown();
        is_running_ = false;
        std::cout << "Video client stopped" << std::endl;
    }
}