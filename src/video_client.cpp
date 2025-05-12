#include "video_client.hpp"
#include <thread>

VideoClientService::VideoClientService(const std::string& ai_server_address)
    : ai_stub_(VideoProcessor::NewStub(CreateChannel(ai_server_address, InsecureChannelCredentials()))) {}

Status VideoClientService::StreamVideo(ServerContext* context, ServerReaderWriter<result_service::Result, result_service::Frame>* stream) {
    ClientContext ai_context;
    std::unique_ptr<ClientReaderWriter<video_processor::Frame, video_processor::Result>> ai_stream(ai_stub_->ProcessVideo(&ai_context));

    result_service::Frame frame;
    video_processor::Frame ai_frame;
    while (stream->Read(&frame)) {
        // Copy from result_service::Frame to video_processor::Frame
        ai_frame.set_image_data(frame.image_data());
        ai_frame.set_width(frame.width());
        ai_frame.set_height(frame.height());
        ai_frame.set_channels(frame.channels());
        ai_stream->Write(ai_frame); 
    }
    ai_stream->WritesDone();

    video_processor::Result ai_result;
    result_service::Result result;
    while (ai_stream->Read(&ai_result)) {
        // Copy from video_processor::Result to result_service::Result
        result.set_data(ai_result.data());
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