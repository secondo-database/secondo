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

#include "KafkaProducer.h"

using namespace std;

namespace kafka {

    ListExpr KafkaProducerTM(ListExpr args) {
        // check number of arguments
        if (!nl->HasLength(args, 1)) {
            return listutils::typeError("wrong number of arguments");
        }
        // argument must be of type string
        if (!CcString::checkType(nl->First(args))) {
            return listutils::typeError("KafkaTopic expected");
        }
        // create the result type (stream Tuple)
        return nl->TwoElemList(listutils::basicSymbol<Stream<Tuple> >(),
                               listutils::basicSymbol<Tuple>());
    }

    class KafkaProducerLI {
    public:
        // constructor: initializes the class from the string argument
        KafkaProducerLI(CcString *arg) : input(""), pos(0) {
            if (arg->IsDefined()) {
                input = arg->GetValue();
            }
        }

        // destructor
        ~KafkaProducerLI() {}

        // this function returns the next result or null if the input is
        // exhausted
        Tuple *getNext(Supplier s) {
            ListExpr resultType = GetTupleResultType(s);
            TupleType *tupleType = new TupleType(nl->Second(resultType));
            Tuple *res = new Tuple(tupleType);
            std::string source = "FgAAABAAnoYBADELAAAABTFHcmFiZQ==";
            res->ReadFromBinStr(0, source);
            return res;
        }

    private:
        string input;  // input string
        size_t pos;    // current position
    };

    int KafkaProducerVM(Word *args, Word &result, int message,
                        Word &local, Supplier s) {
        KafkaProducerLI *li = (KafkaProducerLI *) local.addr;
        switch (message) {
            case OPEN :
                if (li) {
                    delete li;
                }
                local.addr = new KafkaProducerLI((CcString *) args[0].addr);
                return 0;
            case REQUEST:
                result.addr = li ? li->getNext(s) : 0;
                return result.addr ? YIELD : CANCEL;
            case CLOSE:
                if (li) {
                    delete li;
                    local.addr = 0;
                }
                return 0;
        }
        return 0;
    }

    OperatorSpec KafkaProducerSpec(
            " string -> stream(string)",
            " kafkastream(_) ",
            " Reads steam of tuples from kafka topic ",
            " query  kafkastream(\"KM\") count"
    );

    Operator KafkaProducerOp(
            "kafkastream",
            KafkaProducerSpec.getStr(),
            KafkaProducerVM,
            Operator::SimpleSelect,
            KafkaProducerTM
    );

/*
As usual, the final steps are:

  * add the operator to the algebra

  * define the syntax in the ~spec~ file

  * give an example in the ~examples~ file

  * test the operator in Secondo

*/



}