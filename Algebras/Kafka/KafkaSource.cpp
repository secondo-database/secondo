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

#include "KafkaSource.h"
#include "KafkaClient.h"

using namespace std;

namespace kafka {

    ListExpr KafkaSourceTM(ListExpr args) {
        // check number of arguments
        if (!nl->HasLength(args, 1)) {
            return listutils::typeError("wrong number of arguments");
        }
        // argument must be of type string
        if (!CcString::checkType(nl->First(args))) {
            return listutils::typeError("KafkaTopic expected");
        }

        std::string typeString = "(stream (tuple ((PLZ int) (Ort string))))";
        ListExpr res = 0;
        if (!nl->ReadFromString(typeString, res)) {
            cout << "Error reading type line: " << typeString << endl;
        };
        return res;
    }

    class KafkaSourceLI {
    public:
        // constructor: initializes the class from the string argument
        KafkaSourceLI(CcString *arg) : input(""), pos(0) {
            if (arg->IsDefined()) {
                input = arg->GetValue();
            }

            kafkaReaderClient.Open("localhost", "test");
        }

        // destructor
        ~KafkaSourceLI() {
            kafkaReaderClient.Close();
        }

        // this function returns the next result or null if the input is
        // exhausted
        Tuple *getNext(Supplier s) {
            cout << "get Next called" << endl;


            if (pos > 0)
                return 0;
            pos++;
            ListExpr resultType = GetTupleResultType(s);
            TupleType *tupleType = new TupleType(nl->Second(resultType));
            cout << "tupleType generated" << endl;
            Tuple *res = new Tuple(tupleType);
            cout << "tuple created" << endl;
            std::string source = "FgAAABAAnoYBADELAAAABTFHcmFiZQ==";
            res->ReadFromBinStr(0, source);
            cout << "returning" << endl;
            return res;
        }

    private:
        string input;  // input string
        size_t pos;    // current position
        KafkaReaderClient kafkaReaderClient;
    };

    int KafkaSourceVM(Word *args, Word &result, int message,
                      Word &local, Supplier s) {
        KafkaSourceLI *li = (KafkaSourceLI *) local.addr;
        switch (message) {
            case OPEN :
                if (li) {
                    delete li;
                }
                local.addr = new KafkaSourceLI((CcString *) args[0].addr);
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

    OperatorSpec KafkaSourceSpec(
            " string -> stream(string)",
            " kafkastream(_) ",
            " Reads steam of tuples from kafka topic ",
            " query  kafkastream(\"KM\") count"
    );

    Operator kafkaSourceOp(
            "kafkastream",
            KafkaSourceSpec.getStr(),
            KafkaSourceVM,
            Operator::SimpleSelect,
            KafkaSourceTM
    );

/*
As usual, the final steps are:

  * add the operator to the algebra

  * define the syntax in the ~spec~ file

  * give an example in the ~examples~ file

  * test the operator in Secondo

*/



}