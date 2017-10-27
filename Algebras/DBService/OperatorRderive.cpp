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
#include "Algebras/DBService/OperatorRderive.hpp"
#include "Algebras/DBService/ReplicationUtils.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"
#include "DBServiceClient.hpp"
#include "Algebras/Stream/Stream.h"

using namespace std;

namespace DBService {

ListExpr OperatorRderive::mapType(ListExpr args)
{
  // rel x string x fun

  if(!nl->HasLength(args,3)){
    return listutils::typeError("expected 3 arguments");
  }
  if(  !nl->HasLength(nl->First(args),2)
     ||!nl->HasLength(nl->Second(args),2)
     ||!nl->HasLength(nl->Third(args),2)){
    return listutils::typeError("internal error");
  }
  string err = "rel x string x fun(rel->object) expected";
  if(!Relation::checkType(nl->First(nl->First(args)))){
    return listutils::typeError(err + " (first arg is not a relation)");
  }
  if(!CcString::checkType(nl->First(nl->Second(args)))){
    return listutils::typeError(err + " (second arg is not a string)");
  }
  if(!listutils::isMap<1>(nl->First(nl->Third(args)))){
    return listutils::typeError(err + " (third arg is not an unary function");
  }
  // the relation must be a database object
  if(nl->AtomType(nl->Second(nl->First(args)))!=SymbolType){
    return listutils::typeError("first argument must be a relation stored "
                                "in database");
  }
  string relname = nl->SymbolValue(nl->Second(nl->First(args)));

  // the remote name (second argument) must be a constant string
  if(nl->AtomType(nl->Second(nl->Second(args)))!=StringType){
    return listutils::typeError("the second arg must be a constant string");
  }
  string remoteName = nl->StringValue(nl->Second(nl->Second(args)));
  if(!stringutils::isIdent(remoteName)){
    return listutils::typeError("the second arg is not a valid identifier");
  }
  ListExpr funtd = nl->Third(args);
  ListExpr funt = nl->First(funtd);
  ListExpr funarg = nl->Second(funt);
  if(!nl->Equal(funarg, nl->First(nl->First(args)))){
    return listutils::typeError("function argument and relation type differ");
  }
  ListExpr funres = nl->Third(funt);
  if(nl->HasLength(funres,2) && nl->IsEqual(nl->First(funres),"stream")){
    return listutils::typeError("function result cannot be a stream");
  }
  SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
  int algid; 
  int tid;
  string tname;
  if(!ctlg->LookUpTypeExpr(funres,tname,algid,tid)){
    return listutils::typeError("function result is not a valid Secondo type"); 
  }
  
  ListExpr fundef = nl->Second(funtd);
 
  // replace  argumenttype in definition by the type given in type description
  ListExpr fa = nl->Second(fundef);
  fa = nl->TwoElemList(nl->First(fa), nl->Second(funt));
  fundef = nl->ThreeElemList(nl->First(fundef), fa, nl->Third(fundef));
  // we need a new QueryProcessor to avoid overwriting the internal tree
  QueryProcessor* qp2 = new QueryProcessor(nl, 
                                       SecondoSystem::GetAlgebraManager());
  bool correct, evaluable,defined,isFunction;
  OpTree tree=0;
  ListExpr resType;
  try{
     qp2->Construct(fundef,correct, evaluable, defined,isFunction,tree,
                    resType);
  } catch(...){
    correct = false;
  }
  vector<string> dbobjects;
  if(correct){
     qp2->GetDBObjects(tree,dbobjects);
  } 
  if(tree){
     qp2->Destroy(tree,true);
  }
  delete qp2;
  if(!correct){
     return listutils::typeError("Error in function");
  }
  if(!dbobjects.empty()){
     return listutils::typeError("function definition cannot use any "
                                 "database objects");
  }
  return nl->ThreeElemList(
                nl->SymbolAtom(Symbol::APPEND()),
                nl->TwoElemList( nl->StringAtom(relname),
                                 nl->TextAtom(nl->ToString(fundef))),
                funres);           
}

int OperatorRderive::mapValue(Word* args,
                            Word& result,
                            int message,
                            Word& local,
                            Supplier s)
{
   string relname = ((CcString*)args[3].addr)->GetValue();
   string fundef = ((FText*) args[4].addr)->GetValue();
   string remotename = ((CcString*)args[1].addr)->GetValue();

   cout << "relname" << relname << endl;
   cout << "fundef" << fundef << endl;
   cout << "remotename " << remotename << endl;
   // TODO: trigger object creation on dbservice

   // local function evaluation
   Relation* rel = (Relation*) args[0].addr;
   Supplier fun = args[2].addr;
   ArgVectorPointer funArg = qp->Argument(fun);
   (*funArg)[0] = rel;
   qp->Request(fun,result);
   return 0; 
}

} /* namespace DBService */
