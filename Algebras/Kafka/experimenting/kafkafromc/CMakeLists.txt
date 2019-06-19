cmake_minimum_required(VERSION 3.14)
project(kafkafromc)

set(CMAKE_CXX_STANDARD 14)

add_executable(kafkafromc main.cpp)
#ADD_LIBRARY(rdkafka++)
target_link_libraries(kafkafromc rdkafka++)