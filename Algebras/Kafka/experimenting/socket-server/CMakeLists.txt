cmake_minimum_required(VERSION 3.14)
project(socket_server)

set(CMAKE_CXX_STANDARD 14)

SET(CMAKE_CXX_FLAGS -pthread)

add_executable(socket_server main.cpp)