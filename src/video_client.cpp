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

void VideoClientService::ForwardResults(ServerReaderWriter<Result, Frame>* stream) 
{
    Frame frame;
    while (stream->Read(&frame)) {
        if (context_ && context_->IsCancelled()) {
            return;
        }
        std::cout << "Received frame with size: " << frame.image_data().size() << std::endl;
    }
}

Status VideoClientService::StreamVideo(ServerContext* context, ServerReaderWriter<Result, Frame>* stream) 
{
    context_ = context;
    if (context->IsCancelled()) {
        return Status(StatusCode::CANCELLED, "Stream cancelled by client");
    }
    ForwardResults(stream);
    return Status::OK;
}

VideoClient::VideoClient(std::string_view video_server_address)
    : is_running_(false) {
    auto channel = CreateChannel(std::string(video_server_address), InsecureChannelCredentials());
    video_stub_ = VideoStream::NewStub(channel);
}

VideoClient::~VideoClient() {
    Stop();
}

void VideoClient::Start()
 {
    if (!is_running_) {
        is_running_ = true;
        std::cout << "Video client started" << std::endl;
    }
}

void VideoClient::Stop()
 {
    if (is_running_) {
        is_running_ = false;
        std::cout << "Video client stopped" << std::endl;
    }
}

void VideoClient::StreamVideo() 
{
    if (!is_running_) return;

    ClientContext context;
    std::unique_ptr<ClientReaderWriter<result_service::Frame, Result>> stream(video_stub_->StreamVideo(&context));

    VideoCapture cap(1); 
    if (!cap.isOpened()) 
    {
        std::cerr << "Failed to open webcam at index 1 (/dev/video1)" << std::endl;
        // Тестовые данные
        Mat frame(480, 640, CV_8UC3, Scalar(0, 255, 0)); // Green test frame 
        ClientReaderWriter<Frame, Result>* raw_stream = stream.get();

        std::thread writer([raw_stream, &frame]() 
        { 
            for (int i = 0; i < 100; ++i) // 100 test frames 
            {
                Frame proto_frame;
                proto_frame.set_width(frame.cols);
                proto_frame.set_height(frame.rows);
                proto_frame.set_image_data(frame.data, frame.total() * frame.elemSize());
                if (!raw_stream->Write(proto_frame)) {
                    std::cerr << "Failed to write frame to stream" << std::endl;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(33));
            }
            raw_stream->WritesDone();
        });

        std::thread reader([raw_stream]()
        {
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
        return;
    }

    ClientReaderWriter<Frame, Result>* raw_stream = stream.get();

    std::thread writer([raw_stream, &cap]() 
    {
        Mat frame;
        while (cap.read(frame))
         {
            Frame proto_frame;
            proto_frame.set_width(frame.cols);
            proto_frame.set_height(frame.rows);
            proto_frame.set_image_data(frame.data, frame.total() * frame.elemSize());
            if (!raw_stream->Write(proto_frame)) {
                std::cerr << "Failed to write frame to stream" << std::endl;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }
        raw_stream->WritesDone();
    });

    std::thread reader([raw_stream]()
    {
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