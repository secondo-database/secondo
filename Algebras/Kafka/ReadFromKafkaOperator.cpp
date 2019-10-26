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

#include "ReadFromKafkaOperator.h"
#include "KafkaClient.h"
#include "log.hpp"

using namespace std;

namespace kafka {

    ListExpr ReadFromKafkaTM(ListExpr args) {
        // check number of arguments
        if (!nl->HasLength(args, 3)) {
            return listutils::typeError("wrong number of arguments");
        }

        ListExpr brokerArg = nl->First(args);
        ListExpr brokerError = validateBrokerArg(brokerArg);
        if (brokerError)
            return brokerError;
        string broker = nl->StringValue(nl->Second(brokerArg));

        ListExpr topicArg = nl->Second(args);
        ListExpr error = validateTopicArg(topicArg);
        if (error)
            return error;
        string topic = nl->StringValue(nl->Second(topicArg));

        ListExpr booleanArg = nl->Third(args);
        ListExpr booleanError = validateBooleanArg(booleanArg);
        if (booleanError)
            return booleanError;

        std::string typeString = readTypeString(broker, topic);
        LOG(DEBUG) << "topicTypeString: " << typeString;

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

    ListExpr validateBrokerArg(ListExpr topicArg) {
        if (!nl->HasLength(topicArg, 2)) {
            return listutils::typeError("internal error, "
                                        "BrokerArg invalid");
        }

        if (!CcString::checkType(nl->First(topicArg))) {
            return listutils::typeError(
                    "String (as type for Broker name) expected");
        }

        ListExpr fn = nl->Second(topicArg);
        if (nl->AtomType(fn) != StringType) {
            return listutils::typeError("Broker name not constant");
        }
        return 0;
    }

    ListExpr validateBooleanArg(ListExpr booleanArg) {
        if (!nl->HasLength(booleanArg, 2)) {
            return listutils::typeError("internal error, "
                                        "boolean invalid");
        }

        if (!CcBool::checkType(nl->First(booleanArg))) {
            return listutils::typeError(
                    "Boolean expected");
        }

        ListExpr fn = nl->Second(booleanArg);
        if (nl->AtomType(fn) != BoolType) {
            return listutils::typeError("Boolean not BoolType");
        }
        return 0;
    }

    std::string readTypeString(string broker, string topic) {
        LOG(DEBUG) << "readTypeString started. topic:" << topic;
        KafkaReaderClient kafkaReaderClient;
        kafkaReaderClient.Open(broker, topic);
        std::string *source = kafkaReaderClient.ReadSting();
        if (source == nullptr) {
            LOG(DEBUG) << "readTypeString is null";
            return "";
        }
        std::string result = *source;
        delete source;
        kafkaReaderClient.Close();
        LOG(DEBUG) << "readTypeString:" << result;
        return result;
    }

    class KafkaSourceLI {
    public:
        // constructor: initializes the class from the string argument
        KafkaSourceLI(CcString *brokerArg, CcString *topicArg,
                      CcBool *continuousArg) : topic("") {
            def = topicArg->IsDefined();
            if (def) {
                topic = topicArg->GetValue();
                std::string broker = brokerArg->GetValue();
                continuous = continuousArg->GetValue();
                kafkaReaderClient.Open(broker, topic);
                std::string *typeString = kafkaReaderClient.ReadSting();
                delete typeString;
                kafkaReaderClient.setExitOnTimeout(continuous);
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

        bool isContinuous() {
            return continuous;
        }

    private:
        string topic;
        KafkaReaderClient kafkaReaderClient;
        bool def;
        bool continuous;
    };

    int ReadFromKafkaVM(Word *args, Word &result, int message,
                        Word &local, Supplier s) {
        KafkaSourceLI *li = (KafkaSourceLI *) local.addr;
        switch (message) {
            case OPEN :
                LOG(DEBUG) << "ReadFromKafkaVM open";
                if (li) {
                    delete li;
                }
                local.addr = new KafkaSourceLI((CcString *) args[0].addr,
                                               (CcString *) args[1].addr,
                                               (CcBool *) args[2].addr
                );
                LOG(DEBUG) << "ReadFromKafkaVM opened";
                return 0;
            case REQUEST:
                LOG(TRACE) << "ReadFromKafkaVM request";
                if (li) {
                    result.addr = li->getNext(s);
                    if (li->isContinuous())
                        return YIELD;
                    return result.addr ? YIELD : CANCEL;
                } else {
                    result.addr = 0;
                    return CANCEL;
                }
            case CLOSE:
                LOG(DEBUG) << "ReadFromKafkaVM closing";
                if (li) {
                    delete li;
                    local.addr = 0;
                }
                LOG(DEBUG) << "ReadFromKafkaVM closed";
                return 0;
        }
        return 0;
    }

    OperatorSpec ReadFromKafkaOpSpec(
            " string,string,boolean -> stream(string)",
            " readfromkafka(_,_,_) ",
            " Reads steam of tuples from kafka topic ",
            " query  readfromkafka(\"localhost\", \"KM\", false) count"
    );

    Operator readFromKafkaOp(
            "readfromkafka",
            ReadFromKafkaOpSpec.getStr(),
            ReadFromKafkaVM,
            Operator::SimpleSelect,
            ReadFromKafkaTM
    );


}