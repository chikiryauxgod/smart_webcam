#include "video_server.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

using namespace cv;
using namespace grpc;
using namespace result_service; // Для взаимодействия с C++ клиентами
using namespace video_processor; // Для взаимодействия с Python-сервером

std::atomic<bool> running(true);

void signal_handler(int signal) {
    if (signal == SIGINT) {
        running = false;
    }
}

class PythonProcess {
public:
    PythonProcess(const std::string& port) {
        std::string python_port = port;
        pid_t pid = fork();
        if (pid == 0) {
            execlp("python3", "python3", "server.py", python_port.c_str(), nullptr);
            std::cerr << "Failed to start Python process on port " << python_port << std::endl;
            exit(1);
        } else if (pid > 0) {
            pid_ = pid;
            port_ = python_port;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            auto channel = CreateChannel("localhost:" + python_port, InsecureChannelCredentials());
            stub_ = video_processor::VideoProcessor::NewStub(channel); // Используем video_processor
        } else {
            std::cerr << "Fork failed" << std::endl;
        }
    }

    ~PythonProcess() {
        if (pid_ > 0) {
            kill(pid_, SIGTERM);
            waitpid(pid_, nullptr, 0);
        }
    }

    std::unique_ptr<video_processor::VideoProcessor::Stub>& getStub() {
        return stub_;
    }

private:
    pid_t pid_ = 0;
    std::string port_;
    std::unique_ptr<video_processor::VideoProcessor::Stub> stub_;
};

Status VideoServerService::StreamVideo(ServerContext* context, ServerReaderWriter<result_service::Result, result_service::Frame>* stream) {
    static int port_counter = 50052;
    std::string python_port = std::to_string(port_counter++);

    PythonProcess python_process(python_port);
    auto& python_stub = python_process.getStub();

    ClientContext python_context;
    std::unique_ptr<ClientReaderWriter<video_processor::Frame, video_processor::Result>> python_stream(python_stub->ProcessVideo(&python_context));

    result_service::Frame frame;
    while (running && stream->Read(&frame)) {
        if (context->IsCancelled()) {
            return Status(StatusCode::CANCELLED, "Stream cancelled by client");
        }

        if (frame.image_data().size() != static_cast<size_t>(frame.height() * frame.width() * 3)) {
            return Status(StatusCode::INVALID_ARGUMENT, "Invalid frame data size");
        }

        video_processor::Frame py_frame;
        py_frame.set_width(frame.width());
        py_frame.set_height(frame.height());
        py_frame.set_image_data(frame.image_data());
        python_stream->Write(py_frame);

        video_processor::Result python_result;
        if (python_stream->Read(&python_result)) {
            result_service::Result result;
            result.set_data(python_result.data());
            if (!stream->Write(result)) {
                return Status(StatusCode::INTERNAL, "Failed to write result to stream");
            }
        }
    }

    python_stream->WritesDone();
    Status python_status = python_stream->Finish();
    if (!python_status.ok()) {
        std::cerr << "Python stream failed: " << python_status.error_message() << std::endl;
    }

    return Status::OK;
}

VideoServer::VideoServer(std::string_view address) : address_(std::string(address)), is_running_(false) {}

void VideoServer::Start() {
    if (is_running_) {
        std::cerr << "Server already running" << std::endl;
        return;
    }

    signal(SIGINT, signal_handler);

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

void VideoServer::Stop() {
    if (is_running_ && server_) {
        running = false;
        server_->Shutdown();
        is_running_ = false;
        std::cout << "Video server stopped" << std::endl;
    }
}