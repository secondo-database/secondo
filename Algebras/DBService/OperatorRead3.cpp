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
#include "Algebras/DBService/DBServiceClient.hpp"
#include "Algebras/Stream/Stream.h"

using namespace std;

namespace DBService {

ListExpr OperatorRead3::mapType(ListExpr nestedList)
{
    printFunction("OperatorRead3::mapType");
    print(nestedList);

    if(!nl->HasLength(nestedList, 2)) // rel x fun
    {
        ErrorReporter::ReportError(
                "expected two arguments");
        return nl->TypeError();
    }

    // args in type mapping, each argument consists of
    // type and query part

    if(   !nl->HasLength(nl->First(nestedList),2) 
       || !nl->HasLength(nl->Second(nestedList),2)){
      return listutils::typeError("internal error");
    }


    print("nl->First(nestedList)", nl->First(nestedList));
    print("nl->Second(nestedList)", nl->Second(nestedList));

    ListExpr fun = nl->First(nl->Second(nestedList));
    if(!listutils::isMap<1>(fun))
    {
        ErrorReporter::ReportError(
                "no map found");
        return nl->TypeError();
    }

    bool relationLocallyAvailable;
    ListExpr relType = OperatorCommon::getRelType(
            nl->OneElemList(nl->First(nl->First(nestedList))), 
                            relationLocallyAvailable);

    string relationName;
    if(nl->AtomType(nl->Second(nl->First(nestedList)))!=SymbolType)
    {
       ErrorReporter::ReportError(
                 "expected relation name (a symbol)");
       return nl->TypeError();
    }
    relationName = nl->SymbolValue(nl->Second(nl->First(nestedList)));
        print("relationName", relationName);

    // check: type of the relation is the argument type of the 
    //  function
    ListExpr funarg = nl->Second(fun);
    if(!nl->Equal(funarg, relType))
    {
       return listutils::typeError("function argument and relation type"
                                   " are not equal");
    } 
    // check: result of the function must be a tuple stream
    ListExpr funres = nl->Third(fun);
    if(!Stream<Tuple>::checkType(funres))
    {
      return listutils::typeError("function result is not a tuple stream");
    }

    // replace the function type (may be a type constructor) by the real type
    ListExpr funq = nl->Second(nl->Second(nestedList));
    ListExpr funargs = nl->Second(funq);
    ListExpr rfunargs = nl->TwoElemList(
                           nl->First(funargs),
                           funarg);

    ListExpr rfun = nl->ThreeElemList(
                      nl->First(funq),
                      rfunargs,
                      nl->Third(funq)
                    ); 

    string funtext = nl->ToString(rfun);

    ListExpr readTypeMapResult = nl->ThreeElemList(
            nl->SymbolAtom(Symbols::APPEND()),
            nl->TwoElemList((relationLocallyAvailable 
                    ? nl->StringAtom("-" + relationName) 
                    : nl->StringAtom(relationName)),
                    nl->TextAtom(funtext)),
                    funres);
    print("readTypeMapResult", readTypeMapResult);
    return readTypeMapResult;
}



int OperatorRead3::mapValue(Word* args,
                            Word& result,
                            int message,
                            Word& local,
                            Supplier s)
{
    //printFunction("OperatorRead3::mapValue");
    string relationName =
            static_cast<CcString*>(args[2].addr)->GetValue();
    //print("relationName", relationName);
    if(relationName.size()>0 && relationName[0]=='-') 
        // relation locally available
    {
        switch (message)
        {
        case OPEN:
        {   Relation* rel = (Relation*) args[0].addr;
            Supplier fun = args[1].addr;
            ArgVectorPointer funArg = qp->Argument(fun);
            (*funArg)[0] = rel;
            qp->Open(fun);
            return 0;
        }
        case REQUEST:
        {
           Supplier fun = args[1].addr;
           qp->Request(fun,result);
           return qp->Received(fun)?YIELD:CANCEL;
        }
        case CLOSE:
           qp->Close(args[1].addr);
           return 0;
        }
        return 0;
    }
    else
    {
        ffeed5Info* info = (ffeed5Info*) local.addr;
        switch(message){
        case OPEN:{
            if(info){
                delete info;
                local.addr = 0;
            }
            const string databaseName =
                     SecondoSystem::GetInstance()->GetDatabaseName();
            string funText = ((FText*) args[3].addr)->GetValue();
            vector<string> otherObjects;
            string fileName =
                DBServiceClient::getInstance()->
                retrieveReplicaAndGetFileName(
                        databaseName,
                        relationName,
                        otherObjects,
                        funText);
            if(fileName.empty())
            {
               print("Did not receive file");
               return CANCEL; 
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
            // check whether reltype in file and result type are equal
            ListExpr resType = qp->GetType(s);
            if(!nl->Equal(nl->Second(relType), nl->Second(resType))){
               print("result type and type in file differ");
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
