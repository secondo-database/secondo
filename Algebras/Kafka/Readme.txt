This readme describes installing, building and running Kafka Algebra and it dependencies

- Install Secondo RDBS
The installation of Secondo in documented on http://dna.fernuni-hagen.de/Secondo.html/index.html
For our work we used the instructions in /General/User Manual
(http://dna.fernuni-hagen.de/Secondo.html/files/Documentation/General/SecondoManual.pdf)

- Kafka Algebra depends on some external libraries which should be installed before building it

-- Installing librdkafka - the Apache Kafka C/C++ client library
    https://github.com/edenhill/librdkafka

    sudo apt install librdkafka-dev

-- Installing WebSocket libraries
    ssl and boost are required, the easiest way to install all needed:

    sudo apt-get install libcpprest-dev

- Enable Algebra

    activate the algebra by modifying the file makefile.algebras in Secondo's main directory.
    Insert the 3 lines:
    ALGEBRA_DIRS += Kafka
    ALGEBRAS     += KafkaAlgebra
    ALGEBRA_DEPS += rdkafka++ ssl crypto boost_system

    After that, insert a new entry in the file Algebras/Management/AlgebraList.i.cfg
    e.g: ALGEBRA_INCLUDE(160,KafkaAlgebra)

- Running Kafka cluster on local machine

    There are two possibilities of installing Kafka manual and using a script/operator.
    We definitely recommend install Kafka manually.

    -- Installing Kafka manually
        Download and install Kafka as described on official site - https://kafka.apache.org/quickstart
    -- Running Kafka manually
        Running the zookeeper and Kafka server is also documented there, as example:
            cd /home/gstancul/work/kafka/kafka_2.12-2.2.0/
            bin/zookeeper-server-start.sh config/zookeeper.properties
            bin/kafka-server-start.sh config/server.properties

    -- Installing by script/operator
    Run installLocalKafka in Secondo: query installLocalKafka();
    Or run the script: ${SECONDO_BUILD_DIR}/Algebras/Kafka/scripts/installKafka.sh

    -- Running by script/operator
        Use the Secondo operators startLocalKafka, stopLocalKafka, statusLocalKafka
        or the script ${SECONDO_BUILD_DIR}/Algebras/Kafka/scripts/kafkaStartup.sh
        This script can work also with manually installed Kafka if KAFKA_HOME variable is set to
        installation folder.
        Example:
            cd ${SECONDO_BUILD_DIR}
            export KAFKA_HOME=/home/grisha/work/kafka/kafka_2.12-2.2.0
            bin/Scripts/kafkaStartup.sh start
            bin/Scripts/kafkaStartup.sh status
            bin/Scripts/kafkaStartup.sh stop


