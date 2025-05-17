#ifndef VIDEO_CLIENT_HPP
#define VIDEO_CLIENT_HPP

#include <grpcpp/grpcpp.h>
#include "video_processor.grpc.pb.h"
#include "result_service.grpc.pb.h"

using grpc::ServerContext;
using grpc::ServerReaderWriter;
using grpc::Status;
using video_processor::VideoProcessor;
using result_service::Frame;
using result_service::Result;
using result_service::VideoStream;

class VideoClientService final : public VideoStream::Service {
private:
    std::unique_ptr<VideoProcessor::Stub> ai_stub_;
    void ForwardResults(ServerReaderWriter<Result, Frame>* stream);
    ServerContext* context_ = nullptr; // Поле для хранения контекста

public:
    explicit VideoClientService(std::string_view ai_server_address);
    Status StreamVideo(ServerContext* context, ServerReaderWriter<Result, Frame>* stream) override;
};

class VideoClient {
private:
    std::unique_ptr<grpc::Server> server_;
    std::unique_ptr<result_service::VideoStream::Stub> video_stub_;
    bool is_running_;

public:
    explicit VideoClient(std::string_view server_address, std::string_view video_server_address);
    ~VideoClient();
    void Start();
    void Stop();
    void StreamVideo();
};

#endif // VIDEO_CLIENT_HPP