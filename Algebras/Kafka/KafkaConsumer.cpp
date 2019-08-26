/*
----
This file is part of SECONDO.

Copyright (C) 2015,
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

#include "KafkaConsumer.h"
#include "KafkaSource.h"
#include "KafkaClient.h"

using namespace std;

namespace kafka {

    void writeTypeString(string typeString, string string1);

    ListExpr kafkaConsumerTM(ListExpr args) {
        if (!nl->HasLength(args, 2)) {
            return listutils::typeError(" wrong number of args ");
        }

        ListExpr tupleStreamArg = nl->First(args);
        // the list is coded as (<type> <query part>)
        if (!nl->HasLength(tupleStreamArg, 2)) {
            return listutils::typeError(
                    "internal error, tupleStreamArg invalid");
        }

        if (!Stream<Tuple>::checkType(nl->First(tupleStreamArg))) {
            return listutils::typeError(" stream(Tuple) expected ");
        }

        std::string typeString;
        nl->WriteToString(typeString, nl->First(tupleStreamArg));
        cout << "typeString: " << typeString << endl;


        ListExpr topicArg = nl->Second(args);
        ListExpr error = validateTopicArg(topicArg);
        if (error)
            return error;
        string topic = nl->StringValue(nl->Second(topicArg));
        std::string topicTypeString = readTypeString(topic);
        cout << "topicTypeString: " << topicTypeString << endl;

        if (topicTypeString.empty()) {
            writeTypeString(topic, typeString);
        } else if (topicTypeString.compare(typeString)) {
            return listutils::typeError("The kafka topic has a"
                                        " different type");
        }

        return nl->First(tupleStreamArg);
    }

    void writeTypeString(string topic, string typeString) {
        std::cout << "Writing Type Sting: " << typeString << " to topic"
                  << topic << std::endl;

        KafkaProducerClient kafkaProducerClient;
        kafkaProducerClient.Open("localhost", topic);
        kafkaProducerClient.Write(typeString);
        kafkaProducerClient.Close();

        std::cout << "Writing Type Sting done" << std::endl;
    }

    class KafkaKonsumerLI {
    public :
// s is the stream argument , st the string argument
        KafkaKonsumerLI(Word s, CcString *st) : stream(s), topic(" ") {
            def = st->IsDefined();
            if (def) {
                topic = st->GetValue();
                kafkaProducerClient.Open("localhost", topic);
            }
            stream.open();
        }

        ~ KafkaKonsumerLI() {
            stream.close();
            if (def) {
                kafkaProducerClient.Close();
            }
        }

        Tuple *getNext() {
            Tuple *k = stream.request();
            if (k) {
                writeToKafka(k);
                return k;
            }
            return 0;
        }

        void writeToKafka(Tuple *k) {
            if (def) {
                kafkaProducerClient.Write(k->WriteToBinStr());
            }
        }

    private :
        Stream<Tuple> stream;
        string topic;
        KafkaProducerClient kafkaProducerClient;
        bool def;
    };


    int kafkaConsumerVM(Word *args, Word &result, int message,
                        Word &local, Supplier s) {
        KafkaKonsumerLI *li = (KafkaKonsumerLI *) local.addr;
        switch (message) {
            case OPEN :
                if (li) {
                    delete li;
                }
                local.addr = new KafkaKonsumerLI(args[0],
                                                 (CcString *) args[1].addr);
                return 0;
            case REQUEST :
                result.addr = li ? li->getNext() : 0;
                return result.addr ? YIELD : CANCEL;
            case CLOSE :
                if (li) {
                    delete li;
                    local.addr = 0;
                }
                return 0;
        }
        return 0;
    }


    OperatorSpec kafkaConsumerSpec(
            " stream ( Tuple ) x KafkaTopic -> stream ( Topic ) ",
            " _ kafka[_]",
            " All tuples in the stream are written out to Kafka topic ",
            " query plz feed kafka(\"KT\") count "
    );

    Operator kafkaConsumerOp(
            "kafka",
            kafkaConsumerSpec.getStr(),
            kafkaConsumerVM,
            Operator::SimpleSelect,
            kafkaConsumerTM
    );


}

