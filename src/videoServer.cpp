#include "videoServer.h"
#include <thread>

Status VideoServerService::StreamVideo(ServerContext* context, ServerReaderWriter<Result, Frame>* stream) {
    Frame frame;
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        return Status(StatusCode::INTERNAL, "Cannot open camera");
    }

    Mat image;
    while (cap.read(image) && !context->IsCancelled()) {
        std::vector<uchar> buffer;
        imencode(".jpg", image, buffer);

        frame.set_image_data(buffer.data(), buffer.size());
        frame.set_width(image.cols);
        frame.set_height(image.rows);
        frame.set_channels(image.channels());

        if (!stream->Write(frame)) {
            break;
        }

        waitKey(33); // ~30 FPS
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