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
#include "NestedList.h"
#include "StandardTypes.h"

#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/OperatorGetConfigParam.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"

using namespace std;

namespace DBService
{
ListExpr OperatorGetConfigParam::mapType(ListExpr nestedList)
{
    print(nestedList);

    if (!nl->HasLength(nestedList, 2))
    {
        ErrorReporter::ReportError(
                "expected signature: section x key");
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
        ErrorReporter::ReportError("first argument"
                                   " should be a (type, expression) pair");
        return nl->TypeError();
    }

    if(!CcString::checkType(nl->First(nl->Second(nestedList))))
    {
        ErrorReporter::ReportError(
                "second argument must be: string");
        return nl->TypeError();
    }

    ListExpr appendList = nl->TwoElemList(nl->Second(nl->First(nestedList)),
                                          nl->Second(nl->Second(nestedList)));

    ListExpr typeMapResult = nl->ThreeElemList(
            nl->SymbolAtom(Symbols::APPEND()), appendList,
            listutils::basicSymbol<CcString>());

    print(typeMapResult);
    return typeMapResult;
}

int OperatorGetConfigParam::mapValue(Word* args,
                                     Word& result,
                                     int message,
                                     Word& local,
                                     Supplier s)
{
    CcString* section = static_cast<CcString*>(args[0].addr);
    CcString* key = static_cast<CcString*>(args[1].addr);

    print(section->GetValue());
    print(key->GetValue());

    string resultValue;
    SecondoUtilsLocal::readFromConfigFile(resultValue,
                                       section->GetValue().c_str(),
                                       key->GetValue().c_str(),
                                       "");

    result = qp->ResultStorage(s);
    static_cast<CcString*>(result.addr)->Set(true, resultValue);
    return 0;
}

} /* namespace DBService */
