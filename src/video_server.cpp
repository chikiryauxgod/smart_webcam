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
using namespace result_service;


std::atomic<bool> running(true);

void signal_handler(int signal)
{
    if (signal == SIGINT) {
        running = false;
    }
}


class PythonProcess {
public:

    PythonProcess(const std::string& port) 
    {
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
            stub_ = VideoStream::NewStub(channel);
        } else {
            std::cerr << "Fork failed" << std::endl;
        }
    }

    ~PythonProcess() 
    {
        if (pid_ > 0) {
            kill(pid_, SIGTERM);
            waitpid(pid_, nullptr, 0);
        }
    }

    std::unique_ptr<VideoStream::Stub>& getStub() {
        return stub_;
    }

private:
    pid_t pid_ = 0;
    std::string port_;
    std::unique_ptr<VideoStream::Stub> stub_;
};

Status VideoServerService::StreamVideo(ServerContext* context, ServerReaderWriter<result_service::Result, result_service::Frame>* stream)
{
    // generate unique port for python server
    static int port_counter = 50052;
    std::string python_port = std::to_string(port_counter++);

    // python service init 
    PythonProcess python_process(python_port);
    auto& python_stub = python_process.getStub();

    ClientContext python_context;
    std::unique_ptr<ClientReaderWriter<Frame, Result>> python_stream(python_stub->StreamVideo(&python_context));

    Frame frame;
    while (running && stream->Read(&frame)) 
    {
        if (context->IsCancelled()) {
            return Status(StatusCode::CANCELLED, "Stream cancelled by client");
        }

        if (frame.image_data().size() != static_cast<size_t>(frame.height() * frame.width() * 3)) {
            return Status(StatusCode::INVALID_ARGUMENT, "Invalid frame data size");
        }

        // transfer frame to python service 
        python_stream->Write(frame);

        // result of python service 
        Result python_result;
        if (python_stream->Read(&python_result)) 
        {
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

void VideoServer::Start() 
{
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

void VideoServer::Stop()
{
    if (is_running_ && server_)
    {
        running = false;
        server_->Shutdown();
        is_running_ = false;
        std::cout << "Video server stopped" << std::endl;
    }
}