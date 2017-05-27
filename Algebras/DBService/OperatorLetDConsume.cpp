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


//[$][\$]
//[_][\_]

*/
#include <sstream>

#include "Algebras/Distributed2/DArray.h"
#include "Algebras/Relation-C++/OperatorConsume.h"

#include "Algebras/DBService/DBServiceConnector.hpp"
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/OperatorLetDConsume.hpp"


using namespace std;
using namespace distributed2;

namespace DBService
{

ListExpr OperatorLetDConsume::mapType(ListExpr nestedList)
{
    printFunction("OperatorLetDConsume::mapType");
    print("nestedList", nestedList);

    if (nl->ListLength(nestedList) != 2)
    {
        ErrorReporter::ReportError(
                "expected signature: stream(tuple(...)) x string");
        return nl->TypeError();
    }

    if (!Stream<Tuple>::checkType(nl->First(nestedList)) &&
            !DArray::checkType(nl->First(nestedList)))
    {
        ErrorReporter::ReportError(
                "first argument must be: stream(tuple(...))"
                " or DArray(rel(tuple))");
        return nl->TypeError();
    }

    // TODO more checks in case of DArray (-> rel(tuple(...)))

    if(!CcString::checkType(nl->Second(nestedList)))
    {
        ErrorReporter::ReportError(
                "second argument must be: string");
        return nl->TypeError();
    }

    ListExpr consumeTypeMapInput = nl->OneElemList(
            nl->First(nestedList));
    print("consumeTypeMapInput", consumeTypeMapInput);

    ListExpr consumeTypeMapResult = OperatorConsume::ConsumeTypeMap<false>(
            consumeTypeMapInput);

    return consumeTypeMapResult;
}

int OperatorLetDConsume::selectFunction(ListExpr nestedList)
{
    if(DArray::checkType(nl->First(nestedList)))
    {
        return 1;
    }
    return 0;
}

template<bool isDArray>
int OperatorLetDConsume::mapValue(Word* args,
                                  Word& result,
                                  int message,
                                  Word& local,
                                  Supplier s)
{
    if(isDArray)
    {
        printFunction("OperatorLetDConsume::mapValue<isDArray=true>");
        // TODO
        // 1. initialize servers on worker via connection from array
        // 2. DBServiceConnector::replicate for DArray

        return 0;
    }else
    {
        printFunction("OperatorLetDConsume::mapValue<isDArray=false>");

        CcString* relationName = static_cast<CcString*>(args[1].addr);
        print(relationName->GetValue());
        print("relationName", relationName->GetValue());

        int consumeValueMappingResult = OperatorConsume::Consume(args, result,
                message, local, s);

        bool replicationTriggered =
                DBServiceConnector::getInstance()->triggerReplication(
                SecondoSystem::GetInstance()->GetDatabaseName(),
                relationName->GetValue());
        if(!replicationTriggered)
        {
            // TODO
        }
        return consumeValueMappingResult;
    }
}

template int OperatorLetDConsume::mapValue<true>(Word* args,
        Word& result,
        int message,
        Word& local,
        Supplier s);
template int OperatorLetDConsume::mapValue<false>(Word* args,
        Word& result,
        int message,
        Word& local,
        Supplier s);

ValueMapping OperatorLetDConsume::letdconsumeVM[] = {
        mapValue<false>,
        mapValue<true>
};

} /* namespace DBService */
