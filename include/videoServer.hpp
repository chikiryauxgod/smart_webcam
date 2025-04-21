#ifndef VIDEO_SERVER_H
#define VIDEO_SERVER_H

#include <opencv4/opencv2/opencv.hpp>

#include <grpcpp/grpcpp.h>
#include "result_service.grpc.pb.h"

using namespace cv;
using namespace grpc;
using result_service::VideoStream;
using result_service::Frame;
using result_service::Result;

class VideoServerService final : public VideoStream::Service {
public:
    Status StreamVideo(ServerContext* context, ServerReaderWriter<Result, Frame>* stream) override;
};

class VideoServer {
private:
    std::unique_ptr<Server> server_;
    VideoCapture cap_;
    bool is_running_;

public:
    VideoServer(const std::string& address);
    ~VideoServer();
    void Start();
    void Stop();
};

#endif // VIDEO_SERVER_H