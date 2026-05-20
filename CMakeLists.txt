cmake_minimum_required(VERSION 3.10)
project(tcpclient VERSION 1.0)
set(CMAKE_CXX_STANDARD 14)
include_directories(include)
set(SOURCES
    src/Application.cpp
    src/TCPConnection.cpp
    src/TCPCommunicator.cpp
)
add_executable(tcpclient ${SOURCES})
target_link_libraries(tcpclient -pthread dlt)