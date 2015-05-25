/*
----
This file is part of SECONDO.

Copyright (C) 2015, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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

[10] Definition of Auxiliary Functions for the ContinuousUpdate Algebra

Mar 2015 White

*/
#ifndef CONTINUOUSUPDATE_H_
#define CONTINUOUSUPDATE_H_

#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdbool>
#include <iostream>
#include <iterator>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "AlgebraClassDef.h"
#include "AlgebraTypes.h"
#include "ArrayAlgebra.h"
#include "ListUtils.h"
#include "Messages.h"
#include "NestedList.h"
#include "NList.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "SecondoCatalog.h"
#include "SecondoSMI.h"
#include "SecondoSystem.h"
#include "SocketIO.h"
#include "StandardTypes.h"
#include "Symbols.h"
#include "RelationAlgebra.h"
#include "TemporalAlgebra.h"
#include "concurrentqueue.lib"

/*
1 Constants

Constat which controls how long the handler of the providemessages-operator
should sleep before it checks if ther are new entries in its queue (milliseconds)

*/
#define PROVIDEMESSAGES_HANDLER_SLEEP_MS (500)

/*
Constant which controls how often the inserter of the owntransactioninsert-operator
should sleep before it begins inserting into the relation

*/
#define OWNTRANSACTIONINSERT_SLEEP_COUNT (10)
/*
Constant which controls the sleep duration of OWNTRANSACTIONINSERT\_SLEEP\_COUNT
A insert/commit will occur after (OWNTRANSACTIONINSERT\_SLEEP\_COUNT * OWNTRANSACTIONINSERT\_SLEEP\_MS) milliseconds

*/
#define OWNTRANSACTIONINSERT_SLEEP_MS (500)

/*
Constant which controls after how many tuples (in the queue) a insert/commit will occur

*/
#define OWNTRANSACTIONINSERT_COMMIT_TUPLE_COUNT (500)

namespace arrayalgebra{
    void extractIds(const ListExpr,int&,int&);
}

/*
The following function was copied from the HadoopParallelAlgebra
Due to compilation errors in the HadoopParallelAlgebra it could not be included the usual way

*/
ListExpr AntiNumericType(ListExpr type) {
    if (nl->IsEmpty(type)) {
        return type;
    } else if (nl->ListLength(type) == 2) {
        if (nl->IsAtom(nl->First(type)) && nl->IsAtom(nl->Second(type))
                && nl->AtomType(nl->First(type)) == IntType
                && nl->AtomType(nl->Second(type)) == IntType) {

            int algID, typID;
            arrayalgebra::extractIds(type, algID, typID);
            SecondoCatalog* sc = SecondoSystem::GetCatalog();
            if (algID < 0 || typID < 0)
                return nl->SymbolAtom("ERROR");
            return nl->SymbolAtom(sc->GetTypeName(algID, typID));
        } else
            return (nl->Cons(AntiNumericType(nl->First(type)),
                    AntiNumericType(nl->Rest(type))));
    } else if (nl->IsAtom(type)) {
        return type;
    } else {
        return (nl->Cons(AntiNumericType(nl->First(type)),
                AntiNumericType(nl->Rest(type))));
    }
}

/**
Tries to read from the given filedescriptor until "\\n"
Uses a provided buffer
@param fd The filedescriptor to read from
@param line The line which was read from the filedescirptor/buffer
@param a buffer to save overhead data from the fd

*/
bool readLine(int fd, string& line, string& buffer) {

    // Read from fd until buffer contains '\n'.
    string::iterator pos;
    while ((pos = find(buffer.begin(), buffer.end(), '\n')) == buffer.end()
            /* && pos <= 1*/) {
        char buf[1024];
        int n = read(fd, buf, 1024);

        // Error handling
        if (n == -1) {
            line = buffer;
            buffer = "";
            return false;
        }

        // Stream unavailable
        if (n == 0) {
            return false;
        }

        buf[n] = 0;
        buffer += buf;
    }

    // Get the line and return
    line = string(buffer.begin(), pos);
    buffer = string(pos + 1, buffer.end());
    return true;
}

#endif /* CONTINUOUSUPDATE_H_ */
