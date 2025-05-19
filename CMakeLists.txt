cmake_minimum_required(VERSION 3.10)
project(smart_webcam LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH "$ENV{HOME}/.local" ${CMAKE_PREFIX_PATH})

# protobuf & grpc
find_package(Protobuf REQUIRED)
find_package(gRPC    CONFIG REQUIRED)
find_package(OpenCV  REQUIRED)

find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin HINTS /usr/bin)
message(STATUS "Using protoc:      ${Protobuf_PROTOC_EXECUTABLE}")
message(STATUS "Using grpc_cpp_plugin: ${GRPC_CPP_PLUGIN}")

# Telegram bot deps
find_package(TgBot   REQUIRED)
find_package(CURL    REQUIRED)              
find_package(Boost   REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

# include dirs
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/generated/proto
  ${OpenCV_INCLUDE_DIRS}
  ${CURL_INCLUDE_DIRS}
  ${TgBot_INCLUDE_DIRS}
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

add_executable(main_telegram_bot
  src/main_telegram_bot.cpp
  src/telegram_bot.cpp
)

target_link_libraries(main_telegram_bot
  PRIVATE
    proto_lib                  
    gRPC::grpc++
    ${CURL_LIBRARIES}          
    TgBot::TgBot
    Boost::system
    Threads::Threads
    ${OpenCV_LIBS}             
)

