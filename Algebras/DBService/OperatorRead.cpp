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

#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/OperatorRead.hpp"
#include "Algebras/DBService/ReplicationUtils.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"
#include "DBServiceClient.hpp"

using namespace std;

namespace DBService {

ListExpr OperatorRead::mapType(ListExpr nestedList)
{
    print(nestedList);

    if(!nl->HasLength(nestedList, 1))
    {
        ErrorReporter::ReportError(
                "expected one argument");
                return nl->TypeError();
    }

    ListExpr feedTypeMapResult = OperatorFeed::FeedTypeMap(nestedList);

    bool relationLocallyAvailable = (feedTypeMapResult != nl->TypeError());
    print("relationLocallyAvailable",
            string(relationLocallyAvailable ? "TRUE" : "FALSE"));
    string fileName;
    if(!relationLocallyAvailable)
    {
        print("Trying to retrieve relation from DBService");
        const string databaseName =
                SecondoSystem::GetInstance()->GetDatabaseName();
        const string relationName = nl->ToString(nl->First(nestedList));
        print("databaseName", databaseName);
        print("relationName", relationName);
        vector<string> otherObjects;
        DBServiceClient* client = DBServiceClient::getInstance();
        if(!client){
          print("could not create client");
          return listutils::typeError("Could not create client, "
                                      "check dbs configuration");
        }
        fileName = client-> retrieveReplicaAndGetFileName(
                        databaseName,
                        relationName,
                        otherObjects,
                        string(""));
        if(fileName.empty())
        {
            print("Did not receive file");
            return listutils::typeError("File does not exist");
        }
        print("fileName", fileName);
        ffeed5Info info(fileName);
        if(info.isOK()){
           feedTypeMapResult = nl->TwoElemList(
                   nl->SymbolAtom(Symbol::STREAM()),
                   nl->Second(info.getRelType()));
        }else
        {
            print("Could not determine relation type from file");
            return listutils::typeError("Unreadable file");
        }
    }
    print("feedTypeMapResult", feedTypeMapResult);

    ListExpr readTypeMapResult = nl->ThreeElemList(
            nl->SymbolAtom(Symbols::APPEND()),
            nl->OneElemList((relationLocallyAvailable ?
                    nl->StringAtom("") : nl->StringAtom(fileName))),
                    feedTypeMapResult);
    print("readTypeMapResult", readTypeMapResult);
    return readTypeMapResult;
}

int OperatorRead::mapValue(Word* args,
                            Word& result,
                            int message,
                            Word& local,
                            Supplier s)
{
    string fileName =
            static_cast<CcString*>(args[1].addr)->GetValue();
    if(fileName.empty())
    {
        return OperatorFeed::Feed(args, result,
                message, local, s);
    }
    ffeed5Info* info = (ffeed5Info*) local.addr;
    switch(message){
    case OPEN:{
        if(info){
            delete info;
            local.addr = 0;
        }
        print("Reading tuple stream from file", fileName);
        info = new ffeed5Info(fileName);
        if(!info->isOK())
        {
            print("Could not read file");
            delete info;
            return 0;
        }
        ListExpr relType = info->getRelType();
        print("relType", relType);
        if(!Relation::checkType(relType))
        {
            delete info;
            return 0;
        }
        local.addr = info;
        return 0;
    }
    case REQUEST:
        result.addr = info ? info->next() : 0;
        return result.addr? YIELD : CANCEL;
    case CLOSE:
        if(info)
        {
            delete info;
            local.addr = 0;
        }
        return 0;
    }
    return -1;
}

} /* namespace DBService */
