/*

1.1.1 Class Implementation

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

#include "Algebras/DBService/DBServiceManager.hpp"
#include "Algebras/DBService/OperatorAddNode.hpp"
#include "Algebras/DBService/DebugOutput.hpp"

namespace DBService
{

ListExpr OperatorAddNode::mapType(ListExpr nestedList)
{
    print(nestedList);

    if (!nl->HasLength(nestedList, 3))
    {
        ErrorReporter::ReportError(
                "expected signature: host x port x config");
                return nl->TypeError();
    }

    if(!CcString::checkType(nl->First(nestedList)))
    {
        ErrorReporter::ReportError(
                "first argument must be: string");
        return nl->TypeError();
    }

    if(!CcInt::checkType(nl->Second(nestedList)))
    {
        ErrorReporter::ReportError(
                "second argument must be: int");
        return nl->TypeError();
    }

    if(!CcString::checkType(nl->Third(nestedList)))
    {
        ErrorReporter::ReportError(
                "third argument must be: string");
        return nl->TypeError();
    }

    return listutils::basicSymbol<CcBool>();
}

int OperatorAddNode::mapValue(Word* args,
                              Word& result,
                              int message,
                              Word& local,
                              Supplier s)
{
    CcString* host = static_cast<CcString*>(args[0].addr);
    CcInt* port = static_cast<CcInt*>(args[1].addr);
    CcString* config = static_cast<CcString*>(args[2].addr);

    print(host->GetValue());
    print(port->GetValue());
    print(config->GetValue());

    bool success =
            DBServiceManager::getInstance()->addNode(
                    host->GetValue(),
                    port->GetValue(),
                    config->getCsvStr());

    result = qp->ResultStorage(s);
    static_cast<CcBool*>(result.addr)->Set(true,success);
    return 0;
}

} /* namespace DBService */
