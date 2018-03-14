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
#include "Algebras/DBService/OperatorStartDBService.hpp"

using namespace std;

namespace DBService
{

ListExpr OperatorStartDBService::mapType(ListExpr nestedList)
{
    print(nestedList);

    if (!nl->HasLength(nestedList, 0))
    {
        ErrorReporter::ReportError(
                "expected signature: (empty signature)");
        return nl->TypeError();
    }

    return listutils::basicSymbol<CcBool>();
}

int OperatorStartDBService::mapValue(Word* args,
                               Word& result,
                               int message,
                               Word& local,
                               Supplier s)
{
    result = qp->ResultStorage(s);
    CcBool* res = (CcBool*) result.addr;
    if( DBServiceManager::isActive()){
        res->Set(true,false);
        return 0;
    }
    bool started =  DBServiceManager::getInstance() != 0;
    res->Set(true,started);
    return 0;
}

}
