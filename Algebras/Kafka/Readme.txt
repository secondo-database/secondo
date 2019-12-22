
- Installing librdkafka - the Apache Kafka C/C++ client library
    https://github.com/edenhill/librdkafka

    sudo apt install librdkafka-dev

- Installing WebSocket libraries
    ssl and boost are required, the easiest way to install all needed:

    sudo apt-get install libcpprest-dev

- Enable Algebra

    activate the algebra by modifying the file makefile.algebras in Secondo’s main directory.
    Insert the 3 lines:
    ALGEBRA_DIRS += Kafka
    ALGEBRAS     += KafkaAlgebra
    ALGEBRA_DEPS += rdkafka++ ssl crypto boost_system

    After that, insert a new entry in the file Algebras/Management/AlgebraList.i.cfg
    e.g: ALGEBRA_INCLUDE(160,KafkaAlgebra)

 - Running kafka service

    Download and install Kafka: https://kafka.apache.org/quickstart
    Run the zookeeper and server:
        cd /home/gstancul/work/kafka/kafka_2.12-2.2.0/
        bin/zookeeper-server-start.sh config/zookeeper.properties
        bin/kafka-server-start.sh config/server.properties

    Or set KAFKA_HOME variable and execute bin/Scripts/kafkaStartup.sh from secondo directory
    Example:
        export KAFKA_HOME=/home/grisha/work/kafka/kafka_2.12-2.2.0
        bin/Scripts/kafkaStartup.sh
     To stop use bin/Scripts/kafkaStartup.sh stop

- Working with Kafka from secondo:

     open database berlintest;

