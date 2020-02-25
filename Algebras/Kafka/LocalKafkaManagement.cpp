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
        std::cout << std::endl;
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

    ListExpr scriptCallingWithOneParameterTM(ListExpr args) {
        LOG(DEBUG) << "scriptCallingWithOneParameterTM called";
        if (!nl->HasLength(args, 1)) {
            return listutils::typeError(
                    "Operator requires one string argument");
        }

        // TODO: Some empty result
        return listutils::basicSymbol<CcReal>();
    }

    void builResult(Word &result,
                    Supplier &s) {
        // TODO: Remove when the result in signalFinishTM is fixed
        result = qp->ResultStorage(s);
        CcReal *res = (CcReal *) result.addr;
        res->Set(true, 0);
    }

    int installLocalKafkaVM(Word *args, Word &result, int message,
                            Word &local, Supplier s) {
        LOG(DEBUG) << "startLocalKafkaVM called";

        std::string scriptFile = getScriptFile(
                "/Algebras/Kafka/scripts/installKafka.sh");
        if (!scriptFile.empty()) {
            exec(scriptFile.c_str());
        }

        builResult(result, s);
        return 0;
    }

    int startLocalKafkaVM(Word *args, Word &result, int message,
                          Word &local, Supplier s) {
        LOG(DEBUG) << "startLocalKafkaVM called";

        std::string scriptFile = getScriptFile(
                "/Algebras/Kafka/scripts/kafkaStartup.sh");
        if (!scriptFile.empty()) {
            scriptFile += " start";
            exec(scriptFile.c_str());
        }

        builResult(result, s);
        return 0;
    }

    int stopLocalKafkaVM(Word *args, Word &result, int message,
                         Word &local, Supplier s) {
        LOG(DEBUG) << "startLocalKafkaVM called";

        std::string scriptFile = getScriptFile(
                "/Algebras/Kafka/scripts/kafkaStartup.sh");
        if (!scriptFile.empty()) {
            scriptFile += " stop";
            exec(scriptFile.c_str());
        }

        builResult(result, s);
        return 0;
    }

    int statusLocalKafkaVM(Word *args, Word &result, int message,
                           Word &local, Supplier s) {
        LOG(DEBUG) << "statusLocalKafkaVM called";

        std::string scriptFile = getScriptFile(
                "/Algebras/Kafka/scripts/kafkaStartup.sh");
        if (!scriptFile.empty()) {
            scriptFile += " status";
            exec(scriptFile.c_str());
        }

        builResult(result, s);
        return 0;
    }

    int localKafkaVM(Word *args, Word &result, int message,
                     Word &local, Supplier s) {
        LOG(DEBUG) << "localKafkaVM called";
        CcString *commandArg = (CcString *) args[0].addr;

        std::string scriptFile = getScriptFile(
                "/Algebras/Kafka/scripts/kafkaStartup.sh");
        if (!scriptFile.empty()) {
            scriptFile += " ";
            scriptFile += commandArg->GetValue();
            exec(scriptFile.c_str());
        }
        builResult(result, s);
        return 0;
    }

    OperatorSpec installLocalKafkaSpec(
            "installLocalKafka()",
            "installLocalKafka()",
            "Downloads Apache Kafka version 2.2.0 from one of the "
            "mirrors on the internet and install it into folder "
            "${HOME}/kafka/kafka_dist(/home/<user>/kafka/kafka_dist). "
            "It is recommended to install and start the Kafka cluster by "
            "following the instruction on https://kafka.apache.org/quickstart. "
            "This has the advantage of installing more recent versions, "
            "also additionall mirrors are available. "
            "Nevertheless, we provide this operator here for convenient usage "
            "of Kafka technology for users inexperienced with Apache Kafka "
            "setup. ",
            "query installLocalKafka()"
    );
    OperatorSpec startLocalKafkaSpec(
            "startLocalKafka()",
            "startLocalKafka()",
            "Starts a local instance of Zookeeper and Kafka Server "
            "in ${KAFKA_HOME} or if not set in "
            "${HOME}/kafka/kafka_dist(/home/<user>/kafka/kafka_dist) folder "
            "with configuration files config/zookeeper.properties "
            "zookeeper.log. "
            "The output logs of Zookeeper and Kafka Server are redirected to "
            "${KAFKA_HOME}/zookeeper.log and ${KAFKA_HOME}/kafka.log "
            "correspondingly. "
            "To check the status after the start, use the operator "
            "statusLocalKafka. "
            "The operator is implemented as a bash script "
            "${SECONDO_BUILD_DIR}/Algebras/Kafka/scripts/kafkaStartup.sh. "
            "It is recommended to install and start the Kafka cluster by "
            "following the instruction on https://kafka.apache.org/quickstart. "
            "Nevertheless, we provide this operator here for convenient usage "
            "of Kafka technology for users inexperienced with Apache Kafka "
            "setup. ",
            "query startLocalKafka()"
    );
    OperatorSpec statusLocalKafkaSpec(
            "statusLocalKafka() ",
            "statusLocalKafka() ",
            "Displays the Zookeeper and Kafka Server process ids. It "
            "can be used to see which Kafka processes are running on the host. "
            "Displays not just processes started by startLocalKafka operator, "
            "but also the manually or in other way started processes.",
            "query statusLocalKafka()"
    );
    OperatorSpec stopLocalKafkaSpec(
            "stopLocalKafka()",
            "stopLocalKafka()",
            "Stops all Zookeeper and Kafka Server processes on local "
            "host. Stops not just processes started by startLocalKafka "
            "operator, but also the manually or in other way started "
            "processes. To see the list of processes that will be affected, "
            "use the statusLocalKafka operator",
            "query stopLocalKafka()"
    );
    OperatorSpec localKafkaSpec(
            "command -> empty",
            "localKafka(command)",
            "run the "
            "${SECONDO_BUILD_DIR}/Algebras/Kafka/scripts/kafkaStartup.sh "
            "script with one argument - \"command\". Supported commands are: "
            "start - same as startLocalKafka operator,\n"
            "status - same as statusLocalKafka operator,\n"
            "stop  - same as stopLocalKafka operato,r\n"
            "stophard - similar to stop, but uses kill -9(SIGKILL),"
            "topics - list the topics available on the local host. It can be "
            "\t used as test for functioning Kafka cluster. ",
            "query localKafka(\"topics\")"
    );

    Operator installLocalKafkaOp(
            "installLocalKafka",
            installLocalKafkaSpec.getStr(),
            installLocalKafkaVM,
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

    Operator localKafkaOp(
            "localKafka",
            localKafkaSpec.getStr(),
            localKafkaVM,
            Operator::SimpleSelect,
            scriptCallingWithOneParameterTM
    );

}