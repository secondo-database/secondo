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
#include "ListUtils.h"
#include "NestedList.h"
#include "StandardTypes.h"

#include "Algebras/Relation-C++/RelationAlgebra.h"

#include "Algebras/Distributed2/FileRelations.h"

#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/OperatorCommon.hpp"
#include "Algebras/DBService/OperatorRead3.hpp"
#include "Algebras/DBService/ReplicationUtils.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"
#include "DBServiceClient.hpp"

using namespace std;

namespace DBService {

ListExpr OperatorRead3::mapType(ListExpr nestedList)
{
    printFunction("OperatorRead3::mapType");
    print(nestedList);

    if(!nl->HasLength(nestedList, 2))
    {
        ErrorReporter::ReportError(
                "expected two arguments");
        return nl->TypeError();
    }

    print("nl->First(nestedList)", nl->First(nestedList));
    print("nl->Second(nestedList)", nl->Second(nestedList));

//    if(!listutils::isAnyMap(nl->First(nl->Second(nestedList)))) //usesArgsInTM
    if(!listutils::isAnyMap(nl->Second(nestedList)))
    {
        ErrorReporter::ReportError(
                "no map found");
        return nl->TypeError();
    }

//    ListExpr feedTypeMapResult =
//            OperatorFeed::FeedTypeMap( //usesArgsInTM
//                    nl->OneElemList(nl->First(nl->First(nestedList))));
//            OperatorFeed::FeedTypeMap(
//                    nl->OneElemList(nl->First(nestedList)));

    bool relationLocallyAvailable;
    print("nl->First(nestedList)", nl->First(nestedList));
    ListExpr feedTypeMapResult = OperatorCommon::getStreamType(
            nl->OneElemList(nl->First(nestedList)), relationLocallyAvailable);
    print("feedTypeMapResult", feedTypeMapResult);

    print("relationLocallyAvailable",
            string(relationLocallyAvailable ? "TRUE" : "FALSE"));

    string relationName;
    if(!relationLocallyAvailable)
    {
        print("Relation not available locally");
        if(!nl->IsAtom(nl->First(nestedList)))
        {
            ErrorReporter::ReportError(
                    "expected symbol atom");
            return nl->TypeError();
        }
        relationName = nl->ToString(nl->First(nestedList));
        print("relationName", relationName);
    }

    ListExpr readTypeMapResult = nl->ThreeElemList(
            nl->SymbolAtom(Symbols::APPEND()),
            nl->OneElemList((relationLocallyAvailable ?
                    nl->StringAtom("") : nl->StringAtom(relationName))),
                    feedTypeMapResult);
    print("readTypeMapResult", readTypeMapResult);
    return readTypeMapResult;
}

int OperatorRead3::mapValue(Word* args,
                            Word& result,
                            int message,
                            Word& local,
                            Supplier s)
{
    printFunction("OperatorRead3::mapValue");
    string relationName =
            static_cast<CcString*>(args[2].addr)->GetValue();
    print("relationName", relationName);
//    Word fun = args[1];
//    print("fun", fun.list);
    if(relationName.empty()) // relation locally available
    {
        GenericRelation* r;
        GenericRelationIterator* rit;

        switch (message)
        {
        case OPEN:
            r = (GenericRelation*)args[0].addr;
            rit = r->MakeScan();

            local.addr = rit;
            return 0;

        case REQUEST:
        {
            rit = (GenericRelationIterator*)local.addr;
            Tuple* tuple;
            bool found = false;
            ArgVectorPointer funArg = qp->Argument(args[1].addr);
            while(!found && ((tuple = rit->GetNextTuple()) != 0))
            {
                print("no matching tuple found yet");
                (*funArg)[0].addr = tuple;
                Word funResult;
                qp->Request(args[1].addr, funResult);
                if(funResult.addr)
                {
                    print("matching tuple found");
                    found = true;
                    result.setAddr(tuple);
                }else
                {
                    print("tuple does not match");
                    tuple->DeleteIfAllowed();
                }
                if(found)
                {
                    print("yield");
                    return YIELD;
                }
            }
            if(!tuple)
            {
                print("cancel");
                return CANCEL;
            }
            break;
        }
        case CLOSE:
        {
            if(local.addr)
            {
                rit = (GenericRelationIterator*)local.addr;
                delete rit;
                local.addr = 0;
            }
            return 0;
        }
        }
        return 0;
    }else
    {
        print("Trying to retrieve relation from DBService");
        const string databaseName =
                SecondoSystem::GetInstance()->GetDatabaseName();
        print("databaseName", databaseName);
        print("relationName", relationName);
        string fileName =
                DBServiceClient::getInstance()->
                retrieveReplicaAndGetFileName(
                        databaseName,
                        relationName,
                        string("TODO")
                        /*nl->ToString(fun.list)*/);
        if(fileName.empty())
        {
            print("Did not receive file");
            return listutils::typeError("File does not exist");
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
}

} /* namespace DBService */
