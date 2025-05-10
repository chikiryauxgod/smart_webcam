#include "video_server.hpp"
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace grpc;
using namespace result_service;

Status VideoServerService::StreamVideo(ServerContext* context, ServerReaderWriter<Result, Frame>* stream) {
    Frame frame;
    VideoCapture cap(0); // Открываем веб-камеру
    if (!cap.isOpened()) {
        return Status(StatusCode::INTERNAL, "Failed to open webcam");
    }

    while (stream->Read(&frame)) {
        Mat img(frame.height(), frame.width(), CV_8UC3);
        memcpy(img.data, frame.image_data().data(), frame.image_data().size());

        
        Result result;
        result.add_detections()->set_data("Processed frame");
        stream->Write(result); // Отправляем Result
    }

    return Status::OK;
}

VideoServer::VideoServer(const std::string& address) : is_running_(false) {
    ServerBuilder builder;
    builder.AddListeningPort(address, InsecureServerCredentials());
    builder.RegisterService(new VideoServerService());
    server_ = builder.BuildAndStart();
}

VideoServer::~VideoServer() {
    Stop();
}

void VideoServer::Start() {
    if (!is_running_) {
        is_running_ = true;
        std::cout << "Video server started" << std::endl;
    }
}

void VideoServer::Stop() {
    if (is_running_) {
        server_->Shutdown();
        is_running_ = false;
        std::cout << "Video server stopped" << std::endl;
    }
}