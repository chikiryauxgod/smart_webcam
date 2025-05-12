
cmake_minimum_required(VERSION 3.10)
project(smart_webcam LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(gRPC REQUIRED)
find_package(Protobuf REQUIRED)
find_package(OpenCV REQUIRED)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated/proto)
include_directories(${OpenCV_INCLUDE_DIRS})


set(PROTO_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/proto/result_service.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/proto/video_processor.proto
)

set(PROTO_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/proto)
file(MAKE_DIRECTORY ${PROTO_SRC_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC_DIR} ${PROTO_FILES})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_SRC_DIR} ${PROTO_FILES})


add_library(proto_lib STATIC
    ${PROTO_SRCS}
    ${PROTO_HDRS}
    ${GRPC_SRCS}
    ${GRPC_HDRS}
)
target_link_libraries(proto_lib
    gRPC::grpc++
    protobuf::libprotobuf
)


add_executable(video_server
    src/video_server.cpp
)
add_executable(video_client
    src/video_client.cpp
)

target_link_libraries(video_server
    proto_lib
    gRPC::grpc++
    protobuf::libprotobuf
    ${OpenCV_LIBS}
)
target_link_libraries(video_client
    proto_lib
    gRPC::grpc++
    protobuf::libprotobuf
    ${OpenCV_LIBS}
)

add_dependencies(video_server proto_lib)
add_dependencies(video_client proto_lib)