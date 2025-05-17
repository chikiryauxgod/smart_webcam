cmake_minimum_required(VERSION 3.10)
project(smart_webcam LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH "$ENV{HOME}/.local" ${CMAKE_PREFIX_PATH})
set(Protobuf_PROTOC_EXECUTABLE "$ENV{HOME}/.local/bin/protoc")


# protobuf & grpc
set(Protobuf_PROTOC_EXECUTABLE /usr/bin/protoc)
set(Protobuf_USE_STATIC_LIBS OFF)
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(OpenCV REQUIRED)


# debug
message(STATUS "Using protoc: ${Protobuf_PROTOC_EXECUTABLE}")
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin HINTS /usr/bin)
message(STATUS "Using grpc_cpp_plugin: ${GRPC_CPP_PLUGIN}")

# include dirs
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/generated/proto
  ${OpenCV_INCLUDE_DIRS}
)

# proto_lib
set(PROTO_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generated/proto)
set(PROTO_SRCS
  ${PROTO_SRC_DIR}/result_service.pb.cc
  ${PROTO_SRC_DIR}/result_service.grpc.pb.cc
  ${PROTO_SRC_DIR}/video_processor.pb.cc
  ${PROTO_SRC_DIR}/video_processor.grpc.pb.cc
)

add_library(proto_lib STATIC ${PROTO_SRCS})
target_link_libraries(proto_lib
  PUBLIC
    gRPC::grpc++       
    protobuf::libprotobuf
)

add_library(video_server_lib src/video_server.cpp)
target_link_libraries(video_server_lib
  PUBLIC
    proto_lib
    ${OpenCV_LIBS}
)

add_library(video_client_lib src/video_client.cpp)
target_link_libraries(video_client_lib
  PUBLIC
    proto_lib
    ${OpenCV_LIBS}
)

add_executable(main_video_server src/main_video_server.cpp)
target_link_libraries(main_video_server
  PRIVATE
    video_server_lib
)

add_executable(main_video_client src/main_video_client.cpp)
target_link_libraries(main_video_client
  PRIVATE
    video_client_lib
)
