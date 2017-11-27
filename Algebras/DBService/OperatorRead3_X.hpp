/*

1.1 ~OperatorRead3_X~

This operator reads a tuple stream from a relation and requests its replica
from the ~DBService~ if the relation is not available locally. It allows passing
a function that is executed on the replica so that only the matching tuples have
to be transferred.

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
#ifndef ALGEBRAS_DBSERVICE_OperatorRead3_X_HPP_
#define ALGEBRAS_DBSERVICE_OperatorRead3_X_HPP_

#include "Operator.h"
#include "StringUtils.h"
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/Stream/Stream.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "StandardTypes.h"
#include "Algebras/Distributed2/FileRelations.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/DBService/DBServiceClient.hpp"

namespace DBService {

/*

1.1.1 Operator Specification

*/
template<int X>
struct Read3_XInfo: OperatorInfo
{
    Read3_XInfo()
    {   std::string x = stringutils::int2str(X);
        name = "read3_"+x;
        signature = "rel(tuple)  x Index^" + x + " x fun -> stream(tuple)";
        syntax = "Rel x Index read3_1[fun] ";
        meaning = "read a tuple stream from a relation and fall back to the "
                  "replica provided by the DBService if necessary";
        example = "query myRelation myIndex  read3_1[ .. . exactmatch[4000]  "
                "project[Host, Port] ] consume";
        remark = "requires a DBService system";
        usesArgsInTypeMapping = true;
    }
};

/*

1.1.1 Class Definition

*/

template<int X>
class OperatorRead3_X
{
public:

/*

1.1.1.1 Type Mapping Function

*/
    static ListExpr mapType(ListExpr nestedList);

/*

1.1.1.1 Value Mapping Function

*/
    static int mapValue(Word* args,
                        Word& result,
                        int message,
                        Word& local,
                        Supplier s);
};



template<int X>
ListExpr OperatorRead3_X<X>::mapType(ListExpr args)
{
    printFunction(__PRETTY_FUNCTION__);
    print(args);

    if(!nl->HasLength(args, 2 + X )) // rel x Index^X x  fun
    {
        ErrorReporter::ReportError(
                "expected " + stringutils::int2str(2+X) + "  arguments");
        return nl->TypeError();
    }

    // check for correct application of UsesArgsInTypeMapping
    ListExpr Rest = args;
    while(!nl->IsEmpty(Rest)){
      if(!nl->HasLength(nl->First(Rest),2)){
         return listutils::typeError("internal error");
      }
      Rest = nl->Rest(Rest);
    }

    // structure collection
    // typeInfo, locallyAvailable, Name
    std::vector<std::pair<ListExpr, std::pair<bool, std::string> > > types;
    ListExpr rel = nl->First(args);
    if(nl->AtomType(nl->Second(rel)) != SymbolType){
       return listutils::typeError("First argument must be a basic relation.");
    }
    std::string relName = nl->SymbolValue(nl->Second(rel));
    bool locallyAvailable;
    ListExpr relType;
    if(nl->Equal(nl->First(rel), nl->Second(rel))){
       // type could not be extracted, meaning relation is not available locally
       locallyAvailable = false;
       relType = OperatorCommon::getRelType(nl->First(rel), locallyAvailable); 
    } else {
       locallyAvailable = true;
       relType = nl->First(rel);
    }
    types.push_back(std::make_pair(relType, make_pair(locallyAvailable,relName)));
    bool allAvailable = locallyAvailable;

    // in the same way collect all index information
    ListExpr ind = nl->Rest(args); // jump over relation
    for(int i=0;i<X; i++){
       ListExpr index = nl->First(ind);
       ind = nl->Rest(ind);
       if(nl->AtomType(nl->Second(index))!=SymbolType){
          return listutils::typeError("argument number " + stringutils::int2str(i+1)
                                      + " is not a basic object description");
       }
       std::string indexName = nl->SymbolValue(nl->Second(index));
       ListExpr indexType;
       if(nl->Equal(nl->First(index), nl->Second(index))){
           locallyAvailable = false;
           indexType = OperatorCommon::getDerivedType(args,X,locallyAvailable);
       } else {
           indexType = nl->First(index);
           locallyAvailable = true;
       }
       types.push_back(std::make_pair(indexType, make_pair(locallyAvailable,indexName)));
       allAvailable &= locallyAvailable;
    } 

    if(!allAvailable){
       // check whether there exists a replica server holding all required objects
       std::string databasename = SecondoSystem::GetInstance()->GetDatabaseName();
       std::string relName = types[0].second.second;
       std::vector<std::string> derivates;
       for(size_t i=1;i<types.size();i++){
           derivates.push_back(types[i].second.second);
       }
       if(!OperatorCommon::allExists(databasename, relName, derivates)){
          return listutils::typeError("All required objects are neight locally "
                   "available nor provided by DBService");
       }
    }



    if(!nl->HasLength(ind,1)){
      return listutils::typeError("internal counting error");
    }
    ListExpr fun = nl->First(ind);

    if(!listutils::isMap<X+1>(nl->First(fun))){
      return listutils::typeError("last argument is not a function with "
                                  "expected cardinality");
    }

    ListExpr fargs = nl->Rest(nl->First(fun));
    for(int i=0;i<X+1;i++){
       ListExpr farg = nl->First(fargs);
       fargs = nl->Rest(fargs);
       if(!nl->Equal(farg, types[i].first)){
          std::stringstream err;
          err << "type mismatch in function argument " << (i+1)
              << " , expected is " << nl->ToString(farg) 
              << " provided is " << nl->ToString(types[i].first);
          return listutils::typeError(err.str());
       }
    }
    if(!nl->HasLength(fargs,1)){
      return listutils::typeError("internal counting error in function "
                                  "arguments");
    } 
    ListExpr res = nl->First(fargs);
    if(!Stream<Tuple>::checkType(res)){
       return listutils::typeError("function result is not a tuple stream");
    }

        


    // replace the function argument types (may be a type constructor) 
    // by the real type
    ListExpr funq = nl->Second(fun);
    ListExpr newfunq = nl->OneElemList(nl->First(funq));
    ListExpr last = newfunq;
    funq = nl->Rest(funq); // jump over map symbol
    for(int i=0;i<X+1;i++){
       ListExpr farg = nl->First(funq);
       funq = nl->Rest(funq);
       if(!nl->HasLength(farg,2)){ // (name type)
          return listutils::typeError("invalid function definition");
       }
       last = nl->Append(last, nl->TwoElemList( nl->First(farg), 
                                                types[i].first));
    }
    if(!nl->HasLength(funq,1)){
      return listutils::typeError("internal counting error in function "
                                  "definition");
    }   
    last = nl->Append(last,funq);

    // the appendlist will consist of 3 parts
    // 1: one boolean value: true if all objects are present locally
    // 2: the function as text
    // 3: a list of object names
    ListExpr objectNames = // at least 1 elem in vector (relation)
        nl->OneElemList(nl->StringAtom(types[0].second.second));
    last = objectNames;
    for(size_t i=1;i<types.size();i++){
       last = nl->Append(last, nl->StringAtom(types[i].second.second));
    }
    ListExpr appendList = nl->ThreeElemList(
                                  nl->BoolAtom(allAvailable),
                                  nl->TextAtom(nl->ToString(newfunq)),
                                  objectNames);
    return nl->ThreeElemList(
                  nl->SymbolAtom(Symbols::APPEND()),
                  appendList,
                  res);

}


template<int X>
int OperatorRead3_X<X>::mapValue(Word* args,
                            Word& result,
                            int message,
                            Word& local,
                            Supplier s)
{
    //printFunction(__PRETTY_FUNCTION__);

    // arguments:
    // 0 : relation
    // 1 - X : derived objects
    // X + 1 : function
    // X + 2 : locally available
    // X + 3 : function text
    // X + 4 : list of names (relation + derived objects)

    assert(X + 5 == qp->GetNoSons(s));

    bool locallyAvailable = ((CcBool*) args[X+2].addr )->GetValue();
    int funPos = X+1;

    if(locallyAvailable)
    {
        switch (message)
        {
        case OPEN:
        {
            Supplier fun = args[funPos].addr;
            ArgVectorPointer funArg = qp->Argument(fun);
            // put arguments into function
            for(int i=0;i<=X;i++){
               (*funArg)[i] = args[i];
            }
            qp->Open(fun);
            return 0;
        }
        case REQUEST:
        {
           Supplier fun = args[funPos].addr;
           qp->Request(fun,result);
           return qp->Received(fun)?YIELD:CANCEL;
        }
        case CLOSE:
        {
            qp->Close(args[funPos].addr);
            return 0;
        }
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
            const std::string databaseName =
                     SecondoSystem::GetInstance()->GetDatabaseName();
            std::string funText = ((FText*) args[X+3].addr)->GetValue();
            Supplier names = qp->GetSon(s,X+4);
            assert(qp->GetNoSons(names) == X+1);
            std::string relationName;
            std::vector<std::string> otherObjects;
            for(int i=0; i<= X ; i++){
               Word w;
               qp->Request(qp->GetSon(names,i), w);
               std::string name = ((CcString*) w.addr)->GetValue();
               if(i==0){
                  relationName = name;
               } else {
                  otherObjects.push_back(name);
               }
            }
            std::string fileName =
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

#endif /* ALGEBRAS_DBSERVICE_OPERATORREAD3_X_HPP_ */
