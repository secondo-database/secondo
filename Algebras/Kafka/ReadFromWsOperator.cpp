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
#include "log.hpp"
#include "Utils.h"
#include "WebSocketGateway.h"

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

        std::vector<std::string> attributes = kafka::split(wsOperatorType, ",");
        for (std::string attribute : attributes) {
            // Normalize string
            std::replace(std::begin(attribute), std::end(attribute), '\t', ' ');
            std::replace(std::begin(attribute), std::end(attribute), ':', ' ');
            std::replace(std::begin(attribute), std::end(attribute), '\n', ' ');
            kafka::removeMultipleSpaces(attribute);
            std::vector<std::string> parts = kafka::split(attribute, " ");
            if (parts.size() != 3)
                return result;

            result.emplace_back(
                    parts.at(0),
                    websocketpp::utility::to_lower(parts.at(1)),
                    parts.at(2)
            );
        }
        return result;
    }

    ListExpr validateTextArg(ListExpr topicArg) {
        if (!nl->HasLength(topicArg, 2)) {
            return listutils::typeError("internal error: argument invalid");
        }

        if (!FText::checkType(nl->First(topicArg))) {
            return listutils::typeError(
                    "Text expected");
        }

        ListExpr fn = nl->Second(topicArg);
        if (nl->AtomType(fn) != TextType) {
            return listutils::typeError("value not constant");
        }
        return 0;
    }

    string buildSecondoType(const vector<AttributeDescription> &attributes);

    ListExpr validateAttributes(const vector<AttributeDescription> &attributes);

    ListExpr ReadFromWebSocketTM(ListExpr args) {
        // check number of arguments
        if (!nl->HasLength(args, 3)) {
            return listutils::typeError("wrong number of arguments");
        }

        ListExpr uriArg = nl->First(args);
        ListExpr error = validateTextArg(uriArg);
        if (error)
            return error;
        string uri = nl->TextValue(nl->Second(uriArg));

        ListExpr subscribingArg = nl->Second(args);
        error = validateTextArg(subscribingArg);
        if (error)
            return error;
        string subscribing = nl->TextValue(nl->Second(subscribingArg));

        ListExpr typeArg = nl->Third(args);
        error = validateTextArg(typeArg);
        if (error)
            return error;
        string wsOperatorType = nl->TextValue(nl->Second(typeArg));

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

    ListExpr
    validateAttributes(const vector<AttributeDescription> &attributes) {
        if (attributes.empty()) {
            return listutils::typeError("no attributes defined");
        }
        for (AttributeDescription attribute : attributes) {

        }
        return 0;
    }


    string buildSecondoType(const vector<AttributeDescription> &attributes) {
        string result = "(stream (tuple (";
        for (const AttributeDescription &attribute : attributes) {
            result = result + "(" + attribute.name + " " + attribute.type + ")";
        }
        return result + ")))";
    }

    class WebSocketsSourceLI {
    public:
        // constructor: initializes the class from the string argument
        WebSocketsSourceLI(FText *uriArg, FText *subscribingArg,
                           FText *typeArg) {
            def = typeArg->IsDefined();
            if (def) {
                // Types
                string wsOperatorType = typeArg->GetValue();
                LOG(DEBUG) << "WebSocketsSourceLI: mapping=" << wsOperatorType;
                attributes = parseAttributeDescriptions(wsOperatorType);

                // WS Client
                ErrorCode errorCode = webSocketClient.Open(uriArg->GetValue());
                if (errorCode != OK)
                    LOG(ERROR) << "Connection error code:" << errorCode
                               << " - " << getErrorText(errorCode);

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
                return nullptr;
            }

            if (count++ == 10)
                return nullptr;

            // Get data
            std::string data = webSocketClient.ReadSting();
            LOG(TRACE) << "WebSocketsSourceLI: DataSting=" << data;
            if (data.empty())
                return nullptr;

            jsoncons::json jdata;
            try {
                jdata = jsoncons::json::parse(data);
            } catch (const std::exception &e) {
                LOG(ERROR)
                        << "Parsing " << data << " error. \n"
                        << "what(): " << e.what();
                return nullptr;
            }

            // Prepare result
            ListExpr resultType = GetTupleResultType(s);
            TupleType *tupleType = new TupleType(nl->Second(resultType));
            Tuple *res = new Tuple(tupleType);

            int index = 0;
            for (const AttributeDescription &attribute : attributes) {
                std::string value;
                try {
                    value = jsoncons::jsonpointer::get(
                            jdata, attribute.path)
                            .as<std::string>();
                } catch (const std::exception &e) {
                    LOG(ERROR)
                            << "Geting pointer " << attribute.path << " from "
                            << data << " error. \n"
                            << "what(): " << e.what();
                    value = "json error";
                }
                if (attribute.type == "string")
                    res->PutAttribute(index++, new CcString(true, value));
                else if (attribute.type == "real") {
                    double dValue = kafka::parseDouble(value);
                    res->PutAttribute(index++, new CcReal(true, dValue));
                } else if (attribute.type == "int") {
                    int iValue = kafka::parseInt(value);
                    res->PutAttribute(index++, new CcInt(true, iValue));
                } else if (attribute.type == "bool") {
                    bool bValue = kafka::parseBoolean(value);
                    res->PutAttribute(index++, new CcBool(true, bValue));
                } else
                    res->PutAttribute(index++,
                                      new CcString(true, "type error"));
            }
            return res;
        }


    private:
        bool def;
        int count = 0;
        std::vector<AttributeDescription> attributes;
        WebSocketGateway webSocketClient;
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
                local.addr = new WebSocketsSourceLI((FText *) args[0].addr,
                                                    (FText *) args[1].addr,
                                                    (FText *) args[2].addr
                );
                LOG(DEBUG) << "ReadFromWebSocketsVM opened";
                return 0;
            case REQUEST:
                LOG(TRACE) << "ReadFromWebSocketsVM request";
                if (li) {
                    result.addr = li->getNext(s);
//                    return result.addr ? YIELD : CANCEL;
                    return YIELD;
                } else {
                    result.addr = nullptr;
                    return CANCEL;
                }
            case CLOSE:
                LOG(DEBUG) << "ReadFromWebSocketsVM closing";
                if (li) {
                    delete li;
                    local.addr = nullptr;
                }
                LOG(DEBUG) << "ReadFromWebSocketsVM closed";
                return 0;
        }
        return 0;
    }

    OperatorSpec ReadFromWebSocketOpSpec(
            "string,string,string -> stream(tuple)",
            "readfromwebsocket(uri,subscription,mapping) ",
            "Reads steam of tuples from web socket on uri. "
            "subscription - is a string sent to the server just after creating"
            " a connection, can be used in the case the server requires a "
            "subscription request. Mapping - defines the list of result "
            "attributes and JSON Pointer to populate these attributes ",
            R"( query readfromwebsocket("wss://ws.blockchain.info/inv",
    "{\"op\":\"unconfirmed_sub\"}",
   'Name string /op,
    Size string /x/size,
    Addr: string /x/inputs/0/prev_out/addr')
   finishStream[8080] consoleConsumer head[10] count;
)"
    );

    Operator readFromWebSocketOp(
            "readfromwebsocket",
            ReadFromWebSocketOpSpec.getStr(),
            ReadFromWebSocketsVM,
            Operator::SimpleSelect,
            ReadFromWebSocketTM
    );


}