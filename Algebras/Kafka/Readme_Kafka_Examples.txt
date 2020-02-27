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
