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
#include "OperatorLetDConsume.hpp"

#include "OperatorConsume.h"
#include "DBServiceConnector.hpp"
#include "DebugOutput.hpp"

#include <sstream>

using namespace std;

namespace DBService
{

ListExpr OperatorLetDConsume::mapType(ListExpr nestedList)
{
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

    ListExpr consumeTypeMapInput = nl->First(nestedList);
    print(consumeTypeMapInput);

    ListExpr test = nl->First(nl->First(nl->First(nestedList)));
    print(test);

    ListExpr consumeTypeMapResult = OperatorConsume::ConsumeTypeMap<false>(
            consumeTypeMapInput);
    if (consumeTypeMapResult == nl->TypeError())
    {
        return consumeTypeMapResult;
    }

    print("consumeTypeMapResult", consumeTypeMapResult);

    ListExpr appendList = nl->OneElemList(nl->Second(nl->Second(nestedList)));

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
    int consumeValueMappingResult = OperatorConsume::Consume(args, result,
                                                             message, local, s);
    // checking return code of value mapping is noOp?!
    DBServiceConnector::getInstance()->replicateRelation(
            static_cast<CcString*>(args[0].addr)->GetValue());
    return consumeValueMappingResult;
}

} /* namespace DBService */
