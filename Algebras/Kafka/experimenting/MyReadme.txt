- Open points/ TODOs
 -- Connect delay probably because of version
 https://github.com/edenhill/librdkafka/issues/1597
 check version :
 apt-cache policy librdkafka-dev
 apt list librdkafka-dev
 Solution: https://github.com/edenhill/librdkafka/blob/master/INTRODUCTION.md#broker-version-compatibility

 -- DbEnv: BDB2055 Lock table is out of available lock entries

 -- Implement progress request

 -- Rename
  kafka -> writetokafka
  kafkastream -> readfromkafka

- Working with db

    Build
        cd $SECONDO_BUILD_DIR
        make TTY

     Start SecondoTTYBDB

        cd $SECONDO_BUILD_DIR/bin
        SecondoTTYBDB

     restore database berlintest from berlintest;

     open database berlintest;

     list algebra KafkaAlgebra;

     query Staedte;


     query plz feed kafka["localhost","test"] count;

     query kafkastream("localhost", "test", FALSE) count;
     % query kafkastream("localhost", "test", TRUE) count;

     query kafkastream("localhost", "test", TRUE) finishStream[8080] consume;
     query signalFinish("localhost", 8080);

    query plz feed consoleConsumer count;

    query kafkastream("localhost", "test", TRUE) finishStream[8080] consoleConsumer consume;
    query kafkastream("localhost", "test", TRUE) finishStream[8080] consoleConsumer count;

    query plz feed filter [.Ort="Karlsruhe"] kafka["localhost","test"] consume;
    query plz feed filter [.PLZ=76189] kafka["localhost","test"] consume;

 - Running kafka service

    https://kafka.apache.org/quickstart

    cd /home/gstancul/work/kafka/kafka_2.12-2.2.0/
    cd /home/grisha/work/kafka/kafka_2.12-2.2.0/

    bin/zookeeper-server-start.sh config/zookeeper.properties
    bin/kafka-server-start.sh config/server.properties

    -- Create a topic
    bin/kafka-topics.sh --create --bootstrap-server localhost:9092 --replication-factor 1 --partitions 1 --topic test
    bin/kafka-topics.sh --list --bootstrap-server localhost:9092

    -- Test Producer
    bin/kafka-console-producer.sh --broker-list localhost:9092 --topic test1

    -- Start a consumer
    bin/kafka-console-consumer.sh --bootstrap-server localhost:9092 --topic test --from-beginning

- Multiuser secondo

    cd $SECONDO_BUILD_DIR/bin
    SecondoMonitor
    > STARTUP

    SecondoTTYCS

- Other Instance
    export SECONDO_CONFIG=${HOME}/work/thirdInstance/SecondoConfig.ini

- Big data
 query Roads feed kafka["localhost","roads2"] count;
 query Roads feed head[10000] kafka["localhost","roads2"] count;
 bin/kafka-run-class.sh kafka.tools.GetOffsetShell --broker-list localhost:9092 --topic roads2

