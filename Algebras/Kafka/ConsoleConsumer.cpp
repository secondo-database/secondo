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

#include "ConsoleConsumer.h"

#include "Attribute.h" // implementation of attribute types
#include "NestedList.h" // required at many places
#include "Operator.h" // for operator creation
#include "ListUtils.h" // useful functions for nested lists
#include "Stream.h" // wrapper for secondo streams
#include "Algebras/Relation-C++/RelationAlgebra.h" // use of tuples
#include "log.hpp"
#include <stack>

namespace kafka {

    ListExpr consoleConsumerTM(ListExpr args) {
        if (!nl->HasLength(args, 1)) {
            return listutils::typeError(" wrong number of args ");
        }

        if (!Stream<Tuple>::checkType(nl->First(args))) {
            return listutils::typeError(" stream expected ");
        }
        return nl->First(args);
    }

    class ConsoleConsumerLI {
    public :
        ConsoleConsumerLI(Word streamParam) : stream(streamParam) {
            stream.open();
        }

        ~ ConsoleConsumerLI() {
            stream.close();
        }

        Tuple *getNext() {
            Tuple *k = stream.request();
            if (k)
                k->Print(cout);
            return k;
        }

    private :
        Stream<Tuple> stream;
    };


    int consoleConsumerVM(Word *args, Word &result, int message,
                          Word &local, Supplier s) {
        ConsoleConsumerLI *li = (ConsoleConsumerLI *) local.addr;
        switch (message) {
            case OPEN :
                if (li) {
                    delete li;
                }
                local.addr = new ConsoleConsumerLI(args[0]);
                return 0;
            case REQUEST :
                result.addr = li ? li->getNext() : 0;
                return result.addr ? YIELD : CANCEL;
            case CLOSE :
                LOG(DEBUG) << "consoleConsumerVM closing";
                if (li) {
                    delete li;
                    local.addr = 0;
                }
                LOG(DEBUG) << "consoleConsumerVM closed";
                return 0;
        }
        return 0;
    }


    OperatorSpec consoleConsumerSpec(
            " stream(Tuple) -> stream(Tuple) ",
            " _ consoleConsumer ",
            " All tuples in the stream are written to the output "
            "and to the console",
            "query plz feed consoleConsumer count"
    );

    Operator consoleConsumerOp(
            "consoleConsumer",
            consoleConsumerSpec.getStr(),
            consoleConsumerVM,
            Operator::SimpleSelect,
            consoleConsumerTM
    );
}
