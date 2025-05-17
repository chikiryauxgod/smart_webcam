#include "video_client.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>

using namespace cv;
using namespace grpc;
using namespace result_service;

VideoClientService::VideoClientService(std::string_view ai_server_address)
    : ai_stub_(VideoProcessor::NewStub(CreateChannel(std::string(ai_server_address), InsecureChannelCredentials()))) {
}

void VideoClientService::ForwardResults(ServerReaderWriter<Result, Frame>* stream) {
    Frame frame;
    while (stream->Read(&frame)) {
        if (context_ && context_->IsCancelled()) {
            return;
        }
        std::cout << "Received frame with size: " << frame.image_data().size() << std::endl;
    }
}

Status VideoClientService::StreamVideo(ServerContext* context, ServerReaderWriter<Result, Frame>* stream) {
    context_ = context; // Сохраняем указатель на контекст
    if (context->IsCancelled()) {
        return Status(StatusCode::CANCELLED, "Stream cancelled by client");
    }
    ForwardResults(stream);
    return Status::OK;
}

VideoClient::VideoClient(std::string_view server_address, std::string_view video_server_address)
    : is_running_(false) {
    ServerBuilder builder;
    builder.AddListeningPort(std::string(server_address), InsecureServerCredentials());
    builder.RegisterService(new VideoClientService(video_server_address));
    server_ = builder.BuildAndStart();
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

void VideoClient::StreamVideo() {
    if (!is_running_) return;

    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    std::unique_ptr<VideoStream::Stub> stub = VideoStream::NewStub(channel);

    ClientContext context;
    std::unique_ptr<ClientReaderWriter<Frame, Result>> stream(stub->StreamVideo(&context));

    VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Failed to open webcam" << std::endl;
        return;
    }

    // Используем сырой указатель вместо unique_ptr в лямбда-выражениях
    ClientReaderWriter<Frame, Result>* raw_stream = stream.get();

    std::thread writer([raw_stream, &cap]() {
        Mat frame;
        while (cap.read(frame)) {
            Frame proto_frame;
            proto_frame.set_width(frame.cols);
            proto_frame.set_height(frame.rows);
            proto_frame.set_image_data(frame.data, frame.total() * frame.elemSize());
            if (!raw_stream->Write(proto_frame)) {
                std::cerr << "Failed to write frame to stream" << std::endl;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(33)); // ~30 FPS
        }
        raw_stream->WritesDone();
    });

    std::thread reader([raw_stream]() {
        Result result;
        while (raw_stream->Read(&result)) {
            std::cout << "Received: " << result.data() << std::endl;
        }
    });

    writer.join();
    reader.join();

    Status status = stream->Finish();
    if (!status.ok()) {
        std::cerr << "Stream failed: " << status.error_message() << std::endl;
    }
}