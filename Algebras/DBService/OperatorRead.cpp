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
#include "NestedList.h"
#include "StandardTypes.h"

#include "Algebras/Relation-C++/OperatorFeed.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"

#include "Algebras/DBService/DBServiceConnector.hpp"
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/OperatorRead.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"

using namespace std;

namespace DBService {

ListExpr OperatorRead::mapType(ListExpr nestedList)
{
    print(nestedList);

    if (!nl->HasLength(nestedList, 1))
    {
        ErrorReporter::ReportError(
                "expected one argument");
                return nl->TypeError();
    }

    ListExpr feedTypeMapResult = OperatorFeed::FeedTypeMap(nestedList);

    if(feedTypeMapResult == nl->TypeError())
    {

        string relationName = nl->ToString(nl->First(nestedList));
        string fileName =
                DBServiceConnector::getInstance()->
                retrieveReplicaAndGetFileName(
                        SecondoSystem::GetInstance()->GetDatabaseName(),
                        relationName,
                        string(""));
        stringstream createCommand;
        createCommand << "let "
                << nl->ToString(nl->First(nestedList))
                << " =  '"
                << fileName
                << "' getObjectFromFile consume";
        print("createCommand", createCommand.str());
        string errorMessage;
        if(!SecondoUtilsLocal::createRelation(
                createCommand.str(),
                errorMessage))
        {
            throw new SecondoException("Could not create relation from file");
        }
    }

    feedTypeMapResult = OperatorFeed::FeedTypeMap(nestedList);
    print("feedTypeMapResult", feedTypeMapResult);
    return feedTypeMapResult;
}

int OperatorRead::mapValue(Word* args,
                            Word& result,
                            int message,
                            Word& local,
                            Supplier s)
{
    return OperatorFeed::Feed(args, result,
            message, local, s);
}

} /* namespace DBService */
