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

#include "Algebras/Distributed2/FileRelations.h"

#include "Algebras/DBService/DBServiceClient.hpp"
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/OperatorDBSARG.hpp"
#include "Algebras/DBService/ReplicationUtils.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"

using namespace std;

namespace DBService {

ListExpr OperatorDBSARG::mapType(ListExpr nestedList)
{
    printFunction("OperatorDBSARG::mapType");
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

    if(!relationLocallyAvailable)
    {
        if((nl->AtomType(nl->First(nestedList)) == SymbolType))
        {
            ErrorReporter::ReportError(
                    "expected symbol atom");
                    return nl->TypeError();
        }
        const string relationName = nl->ToString(nl->First(nestedList));
        string nestedListString;
        DBServiceClient::getInstance()->getStreamType(
                SecondoSystem::GetInstance()->GetDatabaseName(),
                relationName,
                nestedListString);

        if(!nl->ReadFromString(nestedListString, feedTypeMapResult))
        {
            return nl->TypeError();
        }
    }
    return feedTypeMapResult;

//    string fileName;
//    if(!relationLocallyAvailable)
//    {
//        print("Trying to retrieve relation from DBService");
//        const string databaseName =
//                SecondoSystem::GetInstance()->GetDatabaseName();
//        const string relationName = nl->ToString(nl->First(nestedList));
//        print("databaseName", databaseName);
//        print("relationName", relationName);
//        fileName =
//                DBServiceClient::getInstance()->
//                retrieveReplicaAndGetFileName(
//                        databaseName,
//                        relationName,
//                        string(""));
//        if(fileName.empty())
//        {
//            print("Did not receive file");
//            return listutils::typeError("File does not exist");
//        }
//        print("fileName", fileName);
//        ffeed5Info info(fileName);
//        if(info.isOK()){
//           feedTypeMapResult = nl->TwoElemList(
//                   nl->SymbolAtom(Symbol::STREAM()),
//                   nl->Second(info.getRelType()));
//        }else
//        {
//            print("Could not determine relation type from file");
//            return listutils::typeError("Unreadable file");
//        }
//    }
//    print("feedTypeMapResult", feedTypeMapResult);
//
//    ListExpr readTypeMapResult = nl->ThreeElemList(
//            nl->SymbolAtom(Symbols::APPEND()),
//            nl->OneElemList((relationLocallyAvailable ?
//                    nl->StringAtom("") : nl->StringAtom(fileName))),
//                    feedTypeMapResult);
//    print("readTypeMapResult", readTypeMapResult);
//    return readTypeMapResult;
}

} /* namespace DBService */
