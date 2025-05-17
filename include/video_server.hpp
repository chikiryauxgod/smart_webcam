#ifndef VIDEO_SERVER_HPP
#define VIDEO_SERVER_HPP

#include <opencv4/opencv2/opencv.hpp>
#include <grpcpp/grpcpp.h>
#include "result_service.grpc.pb.h"

using namespace cv;
using namespace grpc;

class VideoServerService final : public ::result_service::VideoStream::Service {
public:
    ::grpc::Status StreamVideo(::grpc::ServerContext* context, 
                               ::grpc::ServerReaderWriter<::result_service::Result, ::result_service::Frame>* stream) override;
};

class VideoServer
 {
private:
    std::unique_ptr<Server> server_;
    VideoCapture cap_;
    bool is_running_;
    std::string address_;

public:
    explicit VideoServer(const std::string_view address);
    ~VideoServer();
    void Start();
    void Stop();
    void Wait();
};

#endif // VIDEO_SERVER_HPP