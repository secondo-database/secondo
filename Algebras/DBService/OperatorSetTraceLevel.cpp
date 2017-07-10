/*
----
This file is part of SECONDO.

Copyright (C) 2017,
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
#include "NestedList.h"
#include "StandardTypes.h"

#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/OperatorSetTraceLevel.hpp"
#include "Algebras/DBService/TraceSettings.hpp"

namespace DBService {

ListExpr OperatorSetTraceLevel::mapType(ListExpr nestedList)
{
    print(nestedList);

    if (!nl->HasLength(nestedList, 1))
    {
        ErrorReporter::ReportError(
                "expected signature: int");
                return nl->TypeError();
    }

    if(!CcInt::checkType(nl->First(nestedList)))
    {
        ErrorReporter::ReportError(
                "first argument must be: int");
        return nl->TypeError();
    }

    return listutils::basicSymbol<CcBool>();
}

int OperatorSetTraceLevel::mapValue(Word* args,
                              Word& result,
                              int message,
                              Word& local,
                              Supplier s)
{
    CcInt* traceLevel = static_cast<CcInt*>(args[0].addr);

    TraceLevel level = static_cast<TraceLevel>(traceLevel->GetValue());
    print("requested trace level", level);

    bool valid = level >= TraceLevel::OFF && level <= TraceLevel::DEBUG;

    if(valid)
    {
        TraceSettings::getInstance()->setTraceLevel(level);
    }

    result = qp->ResultStorage(s);
    static_cast<CcBool*>(result.addr)->Set(true,valid);
    return 0;
}

} /* namespace DBService */
