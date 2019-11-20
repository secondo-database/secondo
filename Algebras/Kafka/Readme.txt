
- Installing librdkafka - the Apache Kafka C/C++ client library
    https://github.com/edenhill/librdkafka

    sudo apt install librdkafka-dev


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

- Working with Kafka from secondo:

     open database berlintest;

 - TODO:
  Debug json extraction