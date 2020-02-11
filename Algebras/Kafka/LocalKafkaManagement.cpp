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

#include "LocalKafkaManagement.h"

#include "StandardTypes.h"
#include "NestedList.h" // required at many places
#include "Operator.h" // for operator creation
#include "ListUtils.h" // useful functions for nested lists

#include "Algebras/Relation-C++/RelationAlgebra.h" // use of tuples
#include "log.hpp"

#include <iostream>
#include <memory>

namespace kafka {

    bool exists_test1(const std::string &name) {
        if (FILE *file = fopen(name.c_str(), "r")) {
            fclose(file);
            return true;
        } else {
            return false;
        }
    }

    std::string getScriptFile(std::string scriptSubPath) {
        const char *secondoDir = std::getenv("SECONDO_BUILD_DIR");
        if (secondoDir == nullptr) {
            std::cout
                    << "The environment variable SECONDO_BUILD_DIR is not set."
                       " This variable is needed to find the starting script"
                    << std::endl;
            return "";
        }

        std::string fileName = "";
        fileName += secondoDir;
        fileName += scriptSubPath;

        if (!exists_test1(fileName)) {
            std::cout
                    << "Script file to run local Kafka not found. "
                       "Should be under "
                    << fileName << std::endl;
            return "";
        }
        return fileName;
    }

    std::string exec(const char *cmd) {
        LOG(INFO) << "Sarting " << cmd;

        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
            std::cout << buffer.data() << std::flush;
        }
//        std::cout << std::endl;
        return result;
    }

    ListExpr scriptCallingTM(ListExpr args) {
        LOG(DEBUG) << "scriptCallingTM called";
        if (!nl->HasLength(args, 0)) {
            return listutils::typeError(
                    "Operators calling scripts like startLocalKafka"
                    " have no arguments ");
        }

        // TODO: Some empty result
        return listutils::basicSymbol<CcReal>();
    }

    int startLocalKafkaVM(Word *args, Word &result, int message,
                          Word &local, Supplier s) {
        LOG(DEBUG) << "startLocalKafkaVM called";

        std::string scriptFile = getScriptFile(
                "/Algebras/Kafka/scripts/kafkaStartup.sh");
        if (!scriptFile.empty()) {
            scriptFile += " start";

            const std::string &output = exec(scriptFile.c_str());
//            std::cout << output << std::endl;
        }

        // TODO: Remove when the result in signalFinishTM is fixed
        result = qp->ResultStorage(s);
        CcReal *res = (CcReal *) result.addr;
        res->Set(true, 0);

        return 0;
    }

    int stopLocalKafkaVM(Word *args, Word &result, int message,
                          Word &local, Supplier s) {
        LOG(DEBUG) << "startLocalKafkaVM called";

        std::string scriptFile = getScriptFile(
                "/Algebras/Kafka/scripts/kafkaStartup.sh");
        if (!scriptFile.empty()) {
            scriptFile += " stop";

            const std::string &output = exec(scriptFile.c_str());
        }

        // TODO: Remove when the result in signalFinishTM is fixed
        result = qp->ResultStorage(s);
        CcReal *res = (CcReal *) result.addr;
        res->Set(true, 0);

        return 0;
    }

    int statusLocalKafkaVM(Word *args, Word &result, int message,
                           Word &local, Supplier s) {
        LOG(DEBUG) << "statusLocalKafkaVM called";

        std::string scriptFile = getScriptFile(
                "/Algebras/Kafka/scripts/kafkaStartup.sh");
        if (!scriptFile.empty()) {
            scriptFile += " status";

            const std::string &output = exec(scriptFile.c_str());
        }

        // TODO: Remove when the result in signalFinishTM is fixed
        result = qp->ResultStorage(s);
        CcReal *res = (CcReal *) result.addr;
        res->Set(true, 0);

        return 0;
    }

    OperatorSpec startLocalKafkaSpec(
            " empty -> empty? ",
            " signalFinish(host, port)",
            " Sends finish signal to finishStream operator ",
            " query signalFinish(\"127.0.0.1\", 8080)"
    );
    OperatorSpec statusLocalKafkaSpec(
            " empty -> empty? ",
            " signalFinish(host, port)",
            " Sends finish signal to finishStream operator ",
            " query signalFinish(\"127.0.0.1\", 8080)"
    );
    OperatorSpec stopLocalKafkaSpec(
            " empty -> empty? ",
            " signalFinish(host, port)",
            " Sends finish signal to finishStream operator ",
            " query signalFinish(\"127.0.0.1\", 8080)"
    );

    Operator installLocalKafkaOp(
            "installLocalKafka",
            startLocalKafkaSpec.getStr(),
            startLocalKafkaVM,
            Operator::SimpleSelect,
            scriptCallingTM
    );

    Operator startLocalKafkaOp(
            "startLocalKafka",
            startLocalKafkaSpec.getStr(),
            startLocalKafkaVM,
            Operator::SimpleSelect,
            scriptCallingTM
    );

    Operator stopLocalKafkaOp(
            "stopLocalKafka",
            stopLocalKafkaSpec.getStr(),
            stopLocalKafkaVM,
            Operator::SimpleSelect,
            scriptCallingTM
    );

    Operator statusLocalKafkaOp(
            "statusLocalKafka",
            statusLocalKafkaSpec.getStr(),
            statusLocalKafkaVM,
            Operator::SimpleSelect,
            scriptCallingTM
    );

    // TODO:
    // operator to list queues and stophard as general LocalKafka('stophard')
    // and LocalKafka('topics')

    // describe
}