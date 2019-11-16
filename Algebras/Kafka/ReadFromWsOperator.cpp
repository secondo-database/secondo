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
#include "Utils.h"
#include "WebSocketClient.h"

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>


using namespace std;

namespace ws {

    class AttributeDescription {
    public:
        AttributeDescription(basic_string<char> name,
                             basic_string<char> type,
                             basic_string<char> path) {
            this->name = name;
            this->type = type;
            this->path = path;
        }

        string name;
        string type;
        string path;
    };

    std::vector<AttributeDescription>
    parseAttributeDescriptions(const string &wsOperatorType) {
        std::vector<AttributeDescription> result;

        std::vector<std::string> attributes = kafka::split(wsOperatorType, ";");
        for (std::string attribute : attributes) {
            std::vector<std::string> parts = kafka::split(attribute, ",");
            if (parts.size() != 3)
                return result;

            result.emplace_back(
                    parts.at(0),
                    parts.at(1),
                    parts.at(2)
            );
        }
        return result;
    }


    ListExpr validateStringArg(ListExpr topicArg) {
        if (!nl->HasLength(topicArg, 2)) {
            return listutils::typeError("internal error: argument invalid");
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

    string buildSecondoType(vector<AttributeDescription> wsOperatorType);

    ListExpr validateAttributes(vector<AttributeDescription> vector);

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

        std::vector<AttributeDescription> attributes =
                parseAttributeDescriptions(wsOperatorType);
        error = validateAttributes(attributes);
        if (error)
            return error;

        string secondoTypeString = buildSecondoType(attributes);
        LOG(DEBUG) << "secondoTypeString:" << secondoTypeString;
        if (secondoTypeString.empty())
            return listutils::typeError("type string argument is invalid");

        ListExpr res = 0;
        if (!nl->ReadFromString(secondoTypeString, res)) {
            cout << "Error reading type line: " << secondoTypeString << endl;
        };
        return res;
    }

    ListExpr validateAttributes(vector<AttributeDescription> attributes) {
        if (attributes.size() == 0) {
            return listutils::typeError("no attributes defined");
        }
        for (AttributeDescription attribute : attributes) {

        }
        return 0;
    }


    string buildSecondoType(vector<AttributeDescription> attributes) {
        string result = "(stream (tuple (";
        for (AttributeDescription attribute : attributes) {
            result = result + "(" + attribute.name + " " + attribute.type + ")";
        }
        return result + ")))";
    }

    class WebSocketsSourceLI {
    public:
        // constructor: initializes the class from the string argument
        WebSocketsSourceLI(CcString *uriArg, CcString *subscribingArg,
                           CcString *typeArg) {
            def = typeArg->IsDefined();
            if (def) {
                // Types
                string wsOperatorType = typeArg->GetValue();
                LOG(DEBUG) << "WebSocketsSourceLI: mapping=" << wsOperatorType;
                attributes = parseAttributeDescriptions(wsOperatorType);

                // WS Client
                webSocketClient.Open(uriArg->GetValue());
                webSocketClient.Subscribe(subscribingArg->GetValue());
            }
        }

        // destructor
        ~WebSocketsSourceLI() {
            if (def) {
                webSocketClient.Close();
            }
        }

        // this function returns the next result or null if the input is
        // exhausted
        Tuple *getNext(Supplier s) {
            if (!def) {
                return NULL;
            }

            if (count++ == 10)
                return NULL;

            std::string data = webSocketClient.ReadSting();
            jsoncons::json jdata = jsoncons::json::parse(data);

            ListExpr resultType = GetTupleResultType(s);
            TupleType *tupleType = new TupleType(nl->Second(resultType));
            Tuple *res = new Tuple(tupleType);

            int index = 0;
            for (AttributeDescription attribute : attributes) {
                std::string value = jsoncons::jsonpointer::get(
                        jdata, attribute.path)
                        .as<std::string>();
                res->PutAttribute(index++, new CcString(true, value));
            }
            return res;
        }

    private:
        bool def;
        int count = 0;
        std::vector<AttributeDescription> attributes;
        WebSocketClient webSocketClient;
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