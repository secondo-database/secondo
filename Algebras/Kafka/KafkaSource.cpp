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

        ListExpr topicArg = nl->First(args);
        ListExpr error = validateTopicArg(topicArg);
        if (error)
            return error;
        string topic = nl->StringValue(nl->Second(topicArg));
        std::string typeString = readTypeString(topic);
        cout << "topicTypeString: " << typeString << endl;

        ListExpr res = 0;
        if (!nl->ReadFromString(typeString, res)) {
            cout << "Error reading type line: " << typeString << endl;
        };
        return res;
    }


    ListExpr validateTopicArg(ListExpr topicArg) {
        if (!nl->HasLength(topicArg, 2)) {
            return listutils::typeError("internal error, "
                                        "topicArg invalid");
        }

        if (!CcString::checkType(nl->First(topicArg))) {
            return listutils::typeError(
                    "String (as type for topic name) expected");
        }

        ListExpr fn = nl->Second(topicArg);
        if (nl->AtomType(fn) != StringType) {
            return listutils::typeError("topic name not constant");
        }
        return 0;
    }

    std::string readTypeString(string topic) {
        cout << "readTypeString started. topic:" << topic << endl;
        KafkaReaderClient kafkaReaderClient;
        kafkaReaderClient.Open("localhost", topic);
        std::string *source = kafkaReaderClient.ReadSting();
        if (source == nullptr) {
            cout << "readTypeString is null" << endl;
            return "";
        }
        std::string result = *source;
        delete source;
        kafkaReaderClient.Close();
        cout << "readTypeString:" << result << endl;
        return result;
    }

    class KafkaSourceLI {
    public:
        // constructor: initializes the class from the string argument
        KafkaSourceLI(CcString *arg) : topic("") {
            def = arg->IsDefined();
            if (def) {
                topic = arg->GetValue();
                kafkaReaderClient.Open("localhost", topic);
                std::string *typeString = kafkaReaderClient.ReadSting();
                delete typeString;
            }
        }

        // destructor
        ~KafkaSourceLI() {
            if (def) {
                kafkaReaderClient.Close();
            }
        }

        // this function returns the next result or null if the input is
        // exhausted
        Tuple *getNext(Supplier s) {
            if (!def) { return NULL; }
            std::string *source = kafkaReaderClient.ReadSting();
            if (source == NULL) {
                return NULL;
            }

            ListExpr resultType = GetTupleResultType(s);
            TupleType *tupleType = new TupleType(nl->Second(resultType));
            Tuple *res = new Tuple(tupleType);
            res->ReadFromBinStr(0, *source);
            delete source;
            return res;
        }

    private:
        string topic;
        KafkaReaderClient kafkaReaderClient;
        bool def;
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


}