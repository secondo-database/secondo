https://github.com/edenhill/librdkafka
sudo apt install librdkafka-dev

 - Examples
    https://github.com/edenhill/librdkafka/blob/master/examples/rdkafka_consumer_example.cpp
    https://github.com/edenhill/librdkafka/blob/master/examples/rdkafka_example.cpp



 - Running kafka service

    https://kafka.apache.org/quickstart

    cd /home/gstancul/work/kafka/kafka_2.12-2.2.0/

    bin/zookeeper-server-start.sh config/zookeeper.properties
    bin/kafka-server-start.sh config/server.properties

    -- Create a topic
    bin/kafka-topics.sh --create --bootstrap-server localhost:9092 --replication-factor 1 --partitions 1 --topic test
    bin/kafka-topics.sh --list --bootstrap-server localhost:9092

    -- Send some messages
    bin/kafka-console-producer.sh --broker-list localhost:9092 --topic test

    -- Start a consumer
    bin/kafka-console-consumer.sh --bootstrap-server localhost:9092 --topic test --from-beginning
