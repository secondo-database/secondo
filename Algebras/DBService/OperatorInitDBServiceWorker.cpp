/*
----
This file is part of SECONDO.

Copyright (C) 2016,
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


//[$][\$]
//[_][\_]

*/
#include "OperatorInitDBServiceWorker.hpp"
#include "ServerRunnable.hpp"
#include "DebugOutput.hpp"

#include "NestedList.h"
#include "StandardTypes.h"

namespace DBService {

ListExpr OperatorInitDBServiceWorker::mapType(ListExpr nestedList)
{
    if (!nl->HasLength(nestedList, 1))
    {
        ErrorReporter::ReportError(
                "expected signature: port");
        return nl->TypeError();
    }

    if (!nl->HasLength(nl->First(nestedList), 2))
    {
        ErrorReporter::ReportError("first argument"
                " should be a (type, expression) pair");
        return nl->TypeError();
    }

    if(!CcInt::checkType(nl->First(nl->First(nestedList))))
    {
        ErrorReporter::ReportError(
                "first argument must be: int");
        return nl->TypeError();
    }

    ListExpr typeMapResult = nl->ThreeElemList(
            nl->SymbolAtom(Symbols::APPEND()),
            nl->Second(nl->First(nestedList)),
            listutils::basicSymbol<CcBool>());

    print(typeMapResult);
    return typeMapResult;
}

int OperatorInitDBServiceWorker::mapValue(Word* args,
        Word& result,
        int message,
        Word& local,
        Supplier s)
{
    CcInt* port = static_cast<CcInt*>(args[0].addr);
    print(port->GetValue());

    ServerRunnable runnable(port->GetValue());
    runnable.run();

    result = qp->ResultStorage(s);
    static_cast<CcBool*>(result.addr)->Set(true,true);
    return 0;
}

} /* namespace DBService */
