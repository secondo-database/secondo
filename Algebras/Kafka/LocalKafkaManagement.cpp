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

namespace kafka {

    void doIt() {
        const char *env_p = std::getenv("SECONDO_BUILD_DIR");
        if (env_p == nullptr) {
            cout
                    << "The environment variable SECONDO_BUILD_DIR is not set."
                       " This variable is needed to find the starting script";
            return;
        }

        std::string fileName = "";
        std::system("./prog");

    }

    ListExpr startLocalKafkaTM(ListExpr args) {
        cout << "startLocalKafkaTM called" << endl;
        if (!nl->HasLength(args, 0)) {
            return listutils::typeError(
                    "Operator startLocalKafka has no arguments ");
        }

        // TODO: Some empty result
        return listutils::basicSymbol<CcReal>();
    }

    int startLocalKafkaVM(Word *args, Word &result, int message,
                          Word &local, Supplier s) {

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

    Operator startLocalKafkaOp(
            "startLocalKafka",
            startLocalKafkaSpec.getStr(),
            startLocalKafkaVM,
            Operator::SimpleSelect,
            startLocalKafkaTM
    );

    Operator stopLocalKafkaOp(
            "stopLocalKafka",
            startLocalKafkaSpec.getStr(),
            startLocalKafkaVM,
            Operator::SimpleSelect,
            startLocalKafkaTM
    );

    Operator statusLocalKafkaOp(
            "statusLocalKafka",
            startLocalKafkaSpec.getStr(),
            startLocalKafkaVM,
            Operator::SimpleSelect,
            startLocalKafkaTM
    );

}