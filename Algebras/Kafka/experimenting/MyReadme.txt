
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


 - For analyses:
 Secondo => query kafkastream("localhost", "test1", FALSE) count;
 command
 'query kafkastream("localhost", "test1", FALSE) count'
 started at: Wed Oct 16 18:07:12 2019

 [DEBUG][Δ 00:00:28:288634] 2019-10-16 18:07:12:99 readTypeString started. topic:test1
 [DEBUG][Δ 00:00:00:000010] 2019-10-16 18:07:12:99 KafkaClient::Open
 Created consumer kafka secondo client#consumer-4
 [DEBUG][Δ 00:00:05:131503] 2019-10-16 18:07:17:257 readTypeString:(stream (tuple ((PLZ int) (Ort string))))
 [DEBUG][Δ 00:00:00:000037] 2019-10-16 18:07:17:257 topicTypeString: (stream (tuple ((PLZ int) (Ort string))))
 noMemoryOperators = 0
 perOperator = 0
 [DEBUG][Δ 00:00:00:000358] 2019-10-16 18:07:17:257 KafkaClient::Open
 Created consumer kafka secondo client#consumer-5
 [DEBUG][Δ 00:00:00:000004] 2019-10-16 18:07:18:412 EOF reached for all 1 partition(s)
 [DEBUG][Δ 00:00:00:000005] 2019-10-16 18:07:18:412 KafkaSourceVM closing
 [DEBUG][Δ 00:00:05:107946] 2019-10-16 18:07:23:520 KafkaSourceVM closed
 Total runtime ...   Times (elapsed / cpu): 11.4213sec / 1.18sec = 9.67908

 41267
