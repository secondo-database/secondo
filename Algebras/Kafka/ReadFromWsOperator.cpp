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

namespace ws {

    ListExpr validateStringArg(ListExpr topicArg) {
        if (!nl->HasLength(topicArg, 2)) {
            return listutils::typeError("internal error, "
                                        "argument invalid");
        }

        if (!CcString::checkType(nl->First(topicArg))) {
            return listutils::typeError(
                    "String expected");
        }

        ListExpr fn = nl->Second(topicArg);
        if (nl->AtomType(fn) != StringType) {
            return listutils::typeError("value not constant");
        }
        return 0;
    }

    string buildSecondoType(string wsOperatorType);

    ListExpr ReadFromWebSocketTM(ListExpr args) {
        // check number of arguments
        if (!nl->HasLength(args, 3)) {
            return listutils::typeError("wrong number of arguments");
        }

        ListExpr uriArg = nl->First(args);
        ListExpr error = validateStringArg(uriArg);
        if (error)
            return error;
        string uri = nl->StringValue(nl->Second(uriArg));

        ListExpr subscribingArg = nl->Second(args);
        error = validateStringArg(subscribingArg);
        if (error)
            return error;
        string subscribing = nl->StringValue(nl->Second(subscribingArg));

        ListExpr typeArg = nl->Third(args);
        error = validateStringArg(typeArg);
        if (error)
            return error;
        string wsOperatorType = nl->StringValue(nl->Second(typeArg));

        string secondoTypeString = buildSecondoType(wsOperatorType);
        ListExpr res = 0;
        if (!nl->ReadFromString(secondoTypeString, res)) {
            cout << "Error reading type line: " << secondoTypeString << endl;
        };
        return res;
    }

    string buildSecondoType(string wsOperatorType) {
        return "(stream (tuple ((Name string))))";
    }

    class WebSocketsSourceLI {
    public:
        // constructor: initializes the class from the string argument
        WebSocketsSourceLI(CcString *uriArg, CcString *subscribingArg,
                           CcString *typeArg) {
            def = typeArg->IsDefined();
            if (def) {
            }
        }

        // destructor
        ~WebSocketsSourceLI() {
            if (def) {
            }
        }

        // this function returns the next result or null if the input is
        // exhausted
        Tuple *getNext(Supplier s) {
            if (!def) { return NULL; }
            std::string source = "";

            ListExpr resultType = GetTupleResultType(s);
            TupleType *tupleType = new TupleType(nl->Second(resultType));
            Tuple *res = new Tuple(tupleType);
            res->ReadFromBinStr(0, source);
            return res;
        }

        bool isContinuous() {
            return continuous;
        }

    private:
        string uri;
        bool def;
        bool continuous;
    };

    int ReadFromWebSocketsVM(Word *args, Word &result, int message,
                             Word &local, Supplier s) {
        WebSocketsSourceLI *li = (WebSocketsSourceLI *) local.addr;
        switch (message) {
            case OPEN :
                LOG(DEBUG) << "ReadFromWebSocketsVM open";
                if (li) {
                    delete li;
                }
                local.addr = new WebSocketsSourceLI((CcString *) args[0].addr,
                                                    (CcString *) args[1].addr,
                                                    (CcString *) args[2].addr
                );
                LOG(DEBUG) << "ReadFromWebSocketsVM opened";
                return 0;
            case REQUEST:
                LOG(TRACE) << "ReadFromWebSocketsVM request";
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
                LOG(DEBUG) << "ReadFromWebSocketsVM closing";
                if (li) {
                    delete li;
                    local.addr = 0;
                }
                LOG(DEBUG) << "ReadFromWebSocketsVM closed";
                return 0;
        }
        return 0;
    }

    OperatorSpec ReadFromWebSocketOpSpec(
            " string,string,string -> stream(tuple)",
            " readfromwebsocket(_,_,_) ",
            " Reads steam of tuples from web socket ",
            R"( query readfromwebsocket("wss://ws.blockchain.info/inv",
     "{\"op\":\"unconfirmed_sub\"}", "Name,string,$.name") count)"
    );

    Operator readFromWebSocketOp(
            "readfromwebsocket",
            ReadFromWebSocketOpSpec.getStr(),
            ReadFromWebSocketsVM,
            Operator::SimpleSelect,
            ReadFromWebSocketTM
    );


}