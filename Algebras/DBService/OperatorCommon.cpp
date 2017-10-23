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

#include "Algebras/Relation-C++/OperatorFeed.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"

#include "Algebras/DBService/DBServiceClient.hpp"
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/OperatorCommon.hpp"
#include "Algebras/Stream/Stream.h"

using namespace std;

namespace DBService {

ListExpr OperatorCommon::getStreamType(
        ListExpr nestedList,
        bool& locallyAvailable)
{
    printFunction("OperatorCommon::getStreamType");
    print(nestedList);

    if(!nl->HasLength(nestedList, 1))
    {
        ErrorReporter::ReportError(
                "expected one argument");
                return nl->TypeError();
    }

    ListExpr feedTypeMapResult = OperatorFeed::FeedTypeMap(nestedList);
    print("feedTypeMapResult", feedTypeMapResult);

    bool relationLocallyAvailable = (feedTypeMapResult != nl->TypeError());
    print("relationLocallyAvailable",
            string(relationLocallyAvailable ? "TRUE" : "FALSE"));

    locallyAvailable = relationLocallyAvailable;

    if(!relationLocallyAvailable)
    {
        print("Relation not available locally");
        if(!nl->IsAtom(nl->First(nestedList)))
        {
            ErrorReporter::ReportError(
                    "expected symbol atom");
                    return nl->TypeError();
        }
        const string relationName = nl->ToString(nl->First(nestedList));
        print("relationName", relationName);
        string nestedListString;
        if(!DBServiceClient::getInstance()->getStreamType(
                SecondoSystem::GetInstance()->GetDatabaseName(),
                relationName,
                nestedListString))
        {
            ErrorReporter::ReportError(
                    "Could not connect to DBService");
                    return nl->TypeError();
        }

        print("nestedListString", nestedListString);

        if(!nl->ReadFromString(nestedListString, feedTypeMapResult))
        {
            print("could not read nested list from string");
            return nl->TypeError();
        }
    }
    print("feedTypeMapResult", feedTypeMapResult);
    return feedTypeMapResult;
}


ListExpr OperatorCommon::getRelType(
        ListExpr nestedList,
        bool& locallyAvailable)
{
  ListExpr tr = getStreamType(nestedList, locallyAvailable);
  if(!Stream<Tuple>::checkType(tr)){
     return tr;
  }
  return nl->TwoElemList( listutils::basicSymbol<Relation>(),
                          nl->Second(tr)); 

}

} /* namespace DBService */
