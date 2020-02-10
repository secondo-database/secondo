cmake_minimum_required(VERSION 3.14)
project(Kafka)

set(CMAKE_CXX_STANDARD 14)

include_directories(.
        ./include
        ../../
        ../../include
        ../../Tools/ListsAndTables
        )

add_executable(Kafka
        KafkaAlgebra.cpp
        KafkaClient.cpp
        KafkaClient.h
        WriteToKafkaOperator.cpp
        WriteToKafkaOperator.h
        ReadFromKafkaOperator.cpp
        ReadFromKafkaOperator.h
        ReadFromWsOperator.cpp
        ReadFromWsOperator.h
        StreamStopping.cpp
        StreamStopping.h
        SignalingSockets.h
        ConsoleConsumerOperator.cpp
        ConsoleConsumerOperator.h
        log.cpp
        log.hpp
        Utils.cpp
        Utils.h
        WebSocketGateway.cpp
        WebSocketGateway.h
        WebSocketClient.cpp
        WebSocketClient.h
        LocalKafkaManagement.cpp
        LocalKafkaManagement.h)
