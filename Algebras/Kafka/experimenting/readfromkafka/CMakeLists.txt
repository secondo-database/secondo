cmake_minimum_required(VERSION 3.14)
project(readfromkafka)

set(CMAKE_CXX_STANDARD 14)

add_executable(readfromkafka main.cpp)

#ADD_LIBRARY(rdkafka++)
target_link_libraries(readfromkafka rdkafka++)