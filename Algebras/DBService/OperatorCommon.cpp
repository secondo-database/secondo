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
#include "Stream.h"

using namespace std;

namespace DBService {

ListExpr OperatorCommon::getStreamType(
        ListExpr nestedList,
        bool& locallyAvailable)
{
    printFunction("OperatorCommon::getStreamType");
    print(nestedList);

    if(!nl->HasMinLength(nestedList, 1))
    {
        ErrorReporter::ReportError(
                "expected at least one argument");
                return nl->TypeError();
    }

    ListExpr arg1 = nl->First(nestedList);
    if(Relation::checkType(arg1)){
      locallyAvailable = true;
      return nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> >(),
                              nl->Second(arg1));
    }


    locallyAvailable = false;
    print("Relation not available locally");
    if(nl->AtomType(arg1)!=SymbolType)
    {
        ErrorReporter::ReportError(
                "expected symbol atom");
                return nl->TypeError();
    }
    const string relationName = nl->SymbolValue(arg1);
    print("relationName", relationName);
    string nestedListString;
    DBServiceClient* dbsc = DBServiceClient::getInstance();
    if(!dbsc){
       print("could not start dbs client, check configuration");
    } else if(!DBServiceClient::getInstance()->getStreamType(
            SecondoSystem::GetInstance()->GetDatabaseName(),
            relationName,
            nestedListString))
    {
        ErrorReporter::ReportError(
                "Could not connect to DBService");
                return nl->TypeError();
    }

    print("nestedListString", nestedListString);
    ListExpr result;

    if(!nl->ReadFromString(nestedListString, result))
    {
        print("could not read nested list from string");
        return nl->TypeError();
    }
    print("result", result);
    return result;
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

ListExpr OperatorCommon::getDerivedType(
            ListExpr args, 
            int X,
            bool & locallyAvailable){

  locallyAvailable = false;
  if(!nl->HasMinLength(args,X+1))
  {
     return listutils::typeError("too less arguments");
  }
  // extract relation and the x-th argument
  ListExpr rel = nl->First(args);
  for(int i=0;i<X;i++) 
  {
    args = nl->Rest(args);
  }
  ListExpr arg = nl->First(args);
  if(!nl->HasLength(rel,2) || !nl->HasLength(arg,2))
  {
     // uses args in type mapping
     return listutils::typeError("internal error");
  }
  ListExpr at = nl->First(arg);
  ListExpr aq = nl->Second(arg);
  if(nl->AtomType(aq) != SymbolType)
  {
     return listutils::typeError("The argument is not a database object");
  }
  if(!nl->Equal(at,aq))
  { // type is extracted from symbol =>
    // object is  locally present
    locallyAvailable = true;
    return at;
  }
  // type must be retrieved from dbservice
  rel = nl->Second(rel); // name of the relation
  if(nl->AtomType(rel) != SymbolType)
  {
     return listutils::typeError("first argument is not a database "
                                 "object name");
  }
  string relName = nl->SymbolValue(rel);
  string argName = nl->SymbolValue(aq);
  string nestedListString;
  DBServiceClient* dbsc = DBServiceClient::getInstance();
  if(!dbsc || !dbsc->getDerivedType(
                SecondoSystem::GetInstance()->GetDatabaseName(),
                relName,
                argName,
                nestedListString)) 
  {
    return listutils::typeError("Could not connect to DBService");
  }

  print("nestedListString", nestedListString);
  ListExpr resultType;
  if(!nl->ReadFromString(nestedListString, resultType))
  {
     return listutils::typeError("could not read nested list from string");
  }
  return  resultType;
}


bool OperatorCommon::allExists( 
                          const std::string& dbName,
                          const std::string& relName,
                          const std::vector<std::string>& derivates) {
   DBServiceClient* client = DBServiceClient::getInstance();
   if(!client){
     return false;
   }
   return client->allExists( dbName, relName, derivates); 
}



} /* namespace DBService */
