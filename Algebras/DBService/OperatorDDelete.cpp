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
#include "FileSystem.h"
#include "NestedList.h"
#include "StandardTypes.h"

#include "Algebras/DBService/DBServiceConnector.hpp"
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/OperatorDDelete.hpp"
#include "Algebras/DBService/ReplicationUtils.hpp"

using namespace std;

namespace DBService
{

ListExpr OperatorDDelete::mapType(ListExpr nestedList)
{
    print(nestedList);

    if (!nl->HasLength(nestedList, 2))
    {
        ErrorReporter::ReportError(
                "expected signature: string x bool");
                return nl->TypeError();
    }

    if(!CcString::checkType(nl->First(nestedList)))
    {
        ErrorReporter::ReportError(
                "first argument must be: string");
        return nl->TypeError();
    }

    if(!CcBool::checkType(nl->Second(nestedList)))
    {
        ErrorReporter::ReportError(
                "second argument must be: bool");
        return nl->TypeError();
    }

    return listutils::basicSymbol<CcBool>();
}

int OperatorDDelete::mapValue(Word* args,
                              Word& result,
                              int message,
                              Word& local,
                              Supplier s)
{
    string relationName = static_cast<CcString*>(args[0].addr)->GetValue();
    bool deleteLocalRelation = static_cast<CcBool*>(args[1].addr)->GetValue();

    print("relationName", relationName);
    print("deleteLocalRelation", deleteLocalRelation);

    FileSystem::DeleteFileOrFolder(
            ReplicationUtils::getFileName(
                    SecondoSystem::GetInstance()->GetDatabaseName(),
                    relationName));

    bool success =
            DBServiceConnector::getInstance()->deleteReplicas(
                    SecondoSystem::GetInstance()->GetDatabaseName(),
                    relationName);

    if(deleteLocalRelation)
    {
        SecondoCatalog* catalog = SecondoSystem::GetCatalog();
        SecondoSystem::BeginTransaction();

        result = qp->ResultStorage(s);
        if (!catalog->DeleteObject(relationName))
        {
            success &= false;
        }
    }
    static_cast<CcBool*>(result.addr)->Set(true, success);
    return 0;
}

} /* namespace DBService */
