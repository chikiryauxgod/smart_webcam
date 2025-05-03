#ifndef VIDEO_CLIENT_HPP
#define VIDEO_CLIENT_HPP

#include <grpcpp/grpcpp.h>
#include "video_processor.grpc.pb.h"
#include "result_service.grpc.pb.h"

using namespace grpc;

using video_processor::VideoProcessor;
using video_processor::Frame;
using video_processor::Result;
using result_service::VideoStream;

class VideoClientService final : public VideoStream::Service {
private:
    std::unique_ptr<VideoProcessor::Stub> ai_stub_;
    void ForwardResults(ServerReaderWriter<Result, Frame>* stream);

public:
    VideoClientService(const std::string& ai_server_address);
    Status StreamVideo(ServerContext* context, ServerReaderWriter<Result, Frame>* stream) override;
};

class VideoClient {
private:
    std::unique_ptr<Server> server_;
    std::unique_ptr<result_service::VideoStream::Stub> video_stub_;
    bool is_running_;

public:
    VideoClient(const std::string& server_address, const std::string& video_server_address);
    ~VideoClient();
    void Start();
    void Stop();
    void StreamVideo();
};

#endif // VIDEO_CLIENT_HPP