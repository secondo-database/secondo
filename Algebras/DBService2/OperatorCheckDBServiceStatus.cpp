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

#include "Algebras/DBService2/DBServiceManager.hpp"
#include "Algebras/DBService2/OperatorCheckDBServiceStatus.hpp"
#include "Algebras/DBService2/DebugOutput.hpp"
#include "Algebras/FText/FTextAlgebra.h"
#include <sstream>

extern boost::recursive_mutex nlparsemtx;

namespace DBService
{

ListExpr OperatorCheckDBServiceStatus::mapType(ListExpr nestedList)
{
    print(nestedList, std::cout);

    // ensure to have only one access to the catalog
    boost::lock_guard<boost::recursive_mutex> guard(nlparsemtx);

    if (!nl->HasLength(nestedList, 0))
    {
        ErrorReporter::ReportError(
                "expected signature: (empty signature)");
        return nl->TypeError();
    }

    return listutils::basicSymbol<FText>();
}

int OperatorCheckDBServiceStatus::mapValue(Word* args,
                              Word& result,
                              int message,
                              Word& local,
                              Supplier s)
{
    bool dbServiceStarted = DBServiceManager::isActive();
    std::string r;
    if(dbServiceStarted)
    {
        DBServiceManager* dbService = DBServiceManager::getInstance();
        std::stringstream out;
        if(dbService){
           dbService->printMetadata(out);
        } else {
           out << "DBService is started, but instance not found." 
               << endl; 
        }
        r = out.str();
    }

    //TODO Is here locking needed when accessing the qp?
    result = qp->ResultStorage(s);
    static_cast<FText*>(result.addr)->Set(dbServiceStarted,r);
    return 0;
}

} /* namespace DBService */
