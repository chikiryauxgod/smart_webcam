#include "video_server.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace grpc;
using namespace result_service;

Status VideoServerService::StreamVideo(ServerContext* context, ServerReaderWriter<result_service::Result, result_service::Frame>* stream) 
{
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        return Status(StatusCode::INTERNAL, "Failed to open webcam");
    }

    result_service::Frame frame;
    while (stream->Read(&frame)) 
    {
        if (context->IsCancelled()) {
            return Status(StatusCode::CANCELLED, "Stream cancelled by client");
        }

        if (frame.image_data().size() != static_cast<size_t>(frame.height() * frame.width() * 3)) {
            return Status(StatusCode::INVALID_ARGUMENT, "Invalid frame data size");
        }

        Mat img(frame.height(), frame.width(), CV_8UC3);
        memcpy(img.data, frame.image_data().data(), frame.image_data().size());

        result_service::Result result;
        result.set_data("Processed frame");
        if (!stream->Write(result)) {
            return Status(StatusCode::INTERNAL, "Failed to write result to stream");
        }
    }

    return Status::OK;
}

VideoServer::VideoServer(std::string_view address) : address_(std::string(address)), is_running_(false) {}

void VideoServer::Start()
 {
    if (is_running_) {
        std::cerr << "Server already running" << std::endl;
        return;
    }

    ServerBuilder builder;
    builder.AddListeningPort(address_, InsecureServerCredentials());
    builder.RegisterService(new VideoServerService());
    server_ = builder.BuildAndStart();
    if (!server_) {
        std::cerr << "Failed to start server" << std::endl;
        return;
    }

    is_running_ = true;
    std::cout << "Video server started on " << address_ << std::endl;
}

void VideoServer::Wait() {
    if (is_running_ && server_) {
        server_->Wait();
    }
}

VideoServer::~VideoServer() {
    Stop();
}

void VideoServer::Stop() 
{
    if (is_running_ && server_) 
    {
        server_->Shutdown();
        is_running_ = false;
        std::cout << "Video server stopped" << std::endl;
    }
}