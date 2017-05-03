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
#include <sstream>

#include "Algebras/Relation-C++/OperatorConsume.h"

#include "Algebras/DBService/OperatorLetDConsume.hpp"
#include "Algebras/DBService/DBServiceConnector.hpp"
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/SecondoUtils.hpp"


using namespace std;

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

    if (!Stream<Tuple>::checkType(nl->First(nl->First(nestedList))))
    {
        ErrorReporter::ReportError(
                "first argument must be: stream(tuple(...))");
        return nl->TypeError();
    }

    if(!CcString::checkType(nl->First(nl->Second(nestedList))))
    {
        ErrorReporter::ReportError(
                "second argument must be: string");
        return nl->TypeError();
    }

    ListExpr consumeTypeMapInput = nl->OneElemList(
            nl->First(nl->First(nestedList)));
    print("consumeTypeMapInput", consumeTypeMapInput);

    ListExpr consumeTypeMapResult = OperatorConsume::ConsumeTypeMap<false>(
            consumeTypeMapInput);

    print("consumeTypeMapResult", consumeTypeMapResult);
    if (consumeTypeMapResult == nl->TypeError())
    {
        return consumeTypeMapResult;
    }

    ListExpr appendList = nl->OneElemList(nl->Second(nl->Second(nestedList)));
    print("appendList", appendList);

    ListExpr typeMapResult = nl->ThreeElemList(
            nl->SymbolAtom(Symbols::APPEND()), appendList,
            consumeTypeMapResult);

    print("typeMapResult", typeMapResult);

    return typeMapResult;
}

int OperatorLetDConsume::mapValue(Word* args,
                                  Word& result,
                                  int message,
                                  Word& local,
                                  Supplier s)
{
    printFunction("OperatorLetDConsume::mapValue");

    CcString* relationName = static_cast<CcString*>(args[0].addr);
    print(relationName->GetValue());
    print("relationName", relationName->GetValue());

    int consumeValueMappingResult = OperatorConsume::Consume(args, result,
                                                             message, local, s);

    DBServiceConnector::getInstance()->replicateRelation(
            relationName->GetValue());
    return consumeValueMappingResult;
}

} /* namespace DBService */
