Working with Kafka from secondo

 For all tests, first open some database, e.g.:
     open database berlintest;

    Write data to topic "test" on localhost:
        query plz feed writetokafka["localhost","test"] count;

    Write data to topic "test" on newton server:
        query plz feed writetokafka["newton1.fernuni-hagen.de:9093","test"] count

    Read data from topic "test" on localhost:
        query readfromkafka("localhost", "test", FALSE) count;

    Read data from topic "test" on newton server:
        query readfromkafka("newton1.fernuni-hagen.de:9093", "test", FALSE) count;

    Read existing data from topic "test" on localhost and waiting&reading for new data :
        query readfromkafka("localhost", "test", TRUE) finishStream[8080] consoleConsumer consume;

    From other instances new data can be added to topic, e.g.
        query plz feed filter [.Ort="Karlsruhe"] writetokafka["localhost","test"] consume;
        query plz feed filter [.PLZ=76189] writetokafka["localhost","test"] consume;

    The waiting can be finished by calling from other terminal:
        query signalFinish("localhost", 8080);

