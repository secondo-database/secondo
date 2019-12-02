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
#include "Algebras/Distributed2/Dist2Helper.h"

#include "Algebras/DBService/DBServiceManager.hpp"
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/OperatorUseIncrementalMetadataUpdate.hpp"

using namespace std;

namespace DBService
{

ListExpr OperatorUseIncrementalMetadataUpdate::mapType(ListExpr nestedList)
{
    print(nestedList, std::cout);

    if (!nl->HasLength(nestedList, 1))
    {
        ErrorReporter::ReportError(
                "expected one argument");
        return nl->TypeError();
    }

    if(!CcBool::checkType(nl->First(nestedList)))
    {
        ErrorReporter::ReportError(
                "first argument must be bool");
        return nl->TypeError();
    }

    return listutils::basicSymbol<CcBool>();
}

int OperatorUseIncrementalMetadataUpdate::mapValue(Word* args,
                               Word& result,
                               int message,
                               Word& local,
                               Supplier s)
{
    bool currentValue = DBServiceManager::isUsingIncrementalMetadataUpdate();
    print("currentValue", currentValue, std::cout);

    bool newValue = static_cast<CcBool*>(args[0].addr)->GetValue();
    print("newValue", newValue, std::cout);

    if(currentValue != newValue)
    {
        DBServiceManager::useIncrementalMetadataUpdate(newValue);
    }

    result = qp->ResultStorage(s);
    // return whether the update mode was changed
    static_cast<CcBool*>(result.addr)->Set(true, (currentValue != newValue));
    return 0;
}

}
