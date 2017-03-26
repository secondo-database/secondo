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
#include "OperatorAddNode.hpp"
#include "DebugOutput.hpp"
#include "DBServiceManager.hpp"

#include "NestedList.h"
#include "StandardTypes.h"

namespace DBService
{

ListExpr OperatorAddNode::mapType(ListExpr nestedList)
{
    print(nestedList);

    if (!nl->HasLength(nestedList, 4))
    {
        ErrorReporter::ReportError(
                "expected signature: host x port x config x commPort");
                return nl->TypeError();
    }

    if (!nl->HasLength(nl->First(nestedList), 2))
    {
        ErrorReporter::ReportError("first argument"
                                   " should be a (type, expression) pair");
        return nl->TypeError();
    }

    if(!CcString::checkType(nl->First(nl->First(nestedList))))
    {
        ErrorReporter::ReportError(
                "first argument must be: string");
        return nl->TypeError();
    }

    if (!nl->HasLength(nl->Second(nestedList), 2))
    {
        ErrorReporter::ReportError("second argument"
                                   " should be a (type, expression) pair");
        return nl->TypeError();
    }

    if(!CcInt::checkType(nl->First(nl->Second(nestedList))))
    {
        ErrorReporter::ReportError(
                "second argument must be: int");
        return nl->TypeError();
    }

    if (!nl->HasLength(nl->Third(nestedList), 2))
    {
        ErrorReporter::ReportError("third argument"
                                   " should be a (type, expression) pair");
        return nl->TypeError();
    }

    if(!CcString::checkType(nl->First(nl->Third(nestedList))))
    {
        ErrorReporter::ReportError(
                "third argument must be: string");
        return nl->TypeError();
    }

    if (!nl->HasLength(nl->Fourth(nestedList), 2))
    {
        ErrorReporter::ReportError("fourth argument"
                                   " should be a (type, expression) pair");
        return nl->TypeError();
    }

    if(!CcInt::checkType(nl->First(nl->Fourth(nestedList))))
    {
        ErrorReporter::ReportError(
                "fourth argument must be: int");
        return nl->TypeError();
    }

    ListExpr appendList = nl->FourElemList(nl->Second(nl->First(nestedList)),
                                            nl->Second(nl->Second(nestedList)),
                                            nl->Second(nl->Third(nestedList)),
                                            nl->Second(nl->Fourth(nestedList)));

    ListExpr typeMapResult = nl->ThreeElemList(
            nl->SymbolAtom(Symbols::APPEND()), appendList,
            listutils::basicSymbol<CcBool>());

    print(typeMapResult);
    return typeMapResult;
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
    CcInt* commPort = static_cast<CcInt*>(args[3].addr);

    print(host->GetValue());
    print(port->GetValue());
    print(config->GetValue());
    print(commPort->GetValue());

    DBServiceManager::getInstance()->addNode(host->GetValue(),
                                             port->GetValue(),
                                             config->getCsvStr(),
                                             commPort->GetValue());

    result = qp->ResultStorage(s);
    static_cast<CcBool*>(result.addr)->Set(true,true);
    return 0;
}

} /* namespace DBService */
