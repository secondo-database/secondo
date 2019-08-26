cmake_minimum_required(VERSION 3.14)
project(Kafka)

set(CMAKE_CXX_STANDARD 14)

include_directories(.
        ../../
        ../../include
        ../../Tools/ListsAndTables
        )

add_executable(Kafka
        KafkaAlgebra.cpp
        KafkaClient.cpp
        KafkaClient.h
        KafkaConsumer.cpp
        KafkaConsumer.h
        KafkaSource.cpp
        KafkaSource.h
        StreamStopping.cpp
        StreamStopping.h
        log.cpp
        log.hpp
        Utils.cpp
        Utils.h)
