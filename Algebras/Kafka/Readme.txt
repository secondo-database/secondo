- Installing librdkafka - the Apache Kafka C/C++ client library
    https://github.com/edenhill/librdkafka

    sudo apt install librdkafka-dev


- Enable Algebra

    After that, insert a new entry in the file Algebras/Management/AlgebraList.i.cfg

    activate the algebra by modifying the file makefile.algebras in Secondo’s main directory.
    Insert the two lines:
    ALGEBRA_DIRS += Kafka
    ALGEBRAS     += KafkaAlgebra
    If the algebra uses third party libraries, add the line:
    ALGEBRA_DEPS += rdkafka++


- How to use makefiles in CLion

    https://www.jetbrains.com/help/clion/managing-makefile-projects.html
    https://blog.jetbrains.com/clion/2018/08/working-with-makefiles-in-clion-using-compilation-db/

    pip install compiledb
     add export PATH="$HOME/.local/bin:$PATH" to .bashrc, to make it work as described in
     https://github.com/nickdiego/compiledb/issues/37

    Usage:
     compiledb make

- Working with db

    make TTY

     Start SecondoTTYBDB
        cd $SECONDO_BUILD_DIR/bin
        SecondoTTYBDB

     restore database berlintest from berlintest;
     open database berlintest;

     list algebra KafkaAlgebra;

     query Staedte;


     query plz feed kafka["KT"] count;

     query kafkastream("KT") count;

 - Running kafka service

    https://kafka.apache.org/quickstart

    cd /home/gstancul/work/kafka/kafka_2.12-2.2.0/

    bin/zookeeper-server-start.sh config/zookeeper.properties
    bin/kafka-server-start.sh config/server.properties

    -- Create a topic
    bin/kafka-topics.sh --create --bootstrap-server localhost:9092 --replication-factor 1 --partitions 1 --topic test
    bin/kafka-topics.sh --list --bootstrap-server localhost:9092

    -- Start a consumer
    bin/kafka-console-consumer.sh --bootstrap-server localhost:9092 --topic test --from-beginning
