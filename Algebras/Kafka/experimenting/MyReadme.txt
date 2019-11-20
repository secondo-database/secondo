- Open points/ TODOs
 -- Connect delay probably because of version
 https://github.com/edenhill/librdkafka/issues/1597
 check version :
 apt-cache policy librdkafka-dev
 apt list librdkafka-dev
 Solution: https://github.com/edenhill/librdkafka/blob/master/INTRODUCTION.md#broker-version-compatibility


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


     query plz feed writetokafka["localhost","test"] count;

     query readfromkafka("localhost", "test", FALSE) count;
     % query readfromkafka("localhost", "test", TRUE) count;

     query readfromkafka("localhost", "test", TRUE) finishStream[8080] consume;
     query signalFinish("localhost", 8080);

    query plz feed consoleConsumer count;

    query readfromkafka("localhost", "test", TRUE) finishStream[8080] consoleConsumer consume;
    query readfromkafka("localhost", "test", TRUE) finishStream[8080] consoleConsumer count;

    query plz feed filter [.Ort="Karlsruhe"] writetokafka["localhost","test"] consume;
    query plz feed filter [.PLZ=76189] writetokafka["localhost","test"] consume;

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
 - Config /home/grisha/work/repository/secondo/bin/SecondoConfig.ini:
    MaxLocks=3000000
    MaxLockObjects=3000000

 query Roads feed writetokafka["localhost","roads1"] count;
 query Roads feed head[10000] writetokafka["localhost","roads2"] count;
 bin/kafka-run-class.sh kafka.tools.GetOffsetShell --broker-list localhost:9092 --topic roads2

 query readfromkafka("localhost", "roads1", FALSE) count;


-- WebSockets
query signalFinish("localhost", 8080);

query readfromwebsocket("mock://data", "hello", 'Name,string,/reputons/0/rated') finishStream[8080] consoleConsumer count;
query readfromwebsocket("mock://data", "hello", 'Name,string,/reputons/0/rated') finishStream[8080] head[10] consume;

query readfromwebsocket("wss://ws.blockchain.info/inv", "{\"op\":\"unconfirmed_sub\"}", 'Name,string,/op') finishStream[8080] consoleConsumer count;

query readfromwebsocket("wss://ws.blockchain.info/inv", "{\"op\":\"unconfirmed_sub\"}", 'Size,string,/op;Size,string,/op/x/size;Name,string,/op/x/inputs/0/addr') finishStream[8080] consoleConsumer count;


