cmake_minimum_required(VERSION 3.14)
project(socket_client)

set(CMAKE_CXX_STANDARD 14)

add_executable(socket_client
        main.cpp
        ../../SignalingSockets.h
        )