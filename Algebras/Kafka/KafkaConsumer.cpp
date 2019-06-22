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

using namespace std;

namespace kafka {

    ListExpr kafkaConsumerTM(ListExpr args) {
        if (!nl->HasLength(args, 2)) {
            return listutils::typeError(" wrong number of args ");
        }

        if (!Stream<Tuple>::checkType(nl->First(args))
            || !CcString::checkType(nl->Second(args))) {
            return listutils::typeError(" stream (string) x string expected ");
        }
        return nl->First(args);
    }

    class KafkaKonsumerLI {
    public :
// s is the stream argument , st the string argument
        KafkaKonsumerLI(Word s, CcString *st) : stream(s), start(" ") {
            def = st->IsDefined();
            if (def) { start = st->GetValue(); }
            stream.open();
        }

        ~ KafkaKonsumerLI() {
            stream.close();
        }

        CcString *getNext() {
            if (!def) { return 0; }
            CcString *k;
            while ((k = stream.request())) {
                if (k->IsDefined()
                    && stringutils::startsWith(k->GetValue(), start)) {
                    return k;
                }
                k->DeleteIfAllowed();
            }
            return 0;
        }

    private :
        Stream<CcString> stream;
        string start;
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
            " _ kafka [ _ ] ",
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

