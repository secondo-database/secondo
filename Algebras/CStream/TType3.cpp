/*
----
This file is part of SECONDO.

Copyright (C) 2018, University in Hagen, 
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

1 Type Map Operator ~TType3~

*/
#include "TupleDescr.h"
#include "NestedList.h"
#include "Operator.h"
#include "Algebras/Stream/Stream.h"
#include "ListUtils.h"
#include "StandardTypes.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"

using namespace std;

namespace cstream{


ListExpr TTYPE3TM(ListExpr args){

  if(!nl->HasMinLength(args,3)){
    return listutils::typeError("at least 3 arguments required");
  }
  ListExpr td = nl->Third(args);
  if(!nl->HasLength(td,2)){
    return listutils::typeError("internal error");
  }
  ListExpr tt = nl->First(td);
  if(!TupleDescr::checkType(tt)){
    return listutils::typeError("expected tuple description as third argument");
  }
   ListExpr expression = nl->Second(td);

   // we need some variables for feeding the ExecuteQuery function
   Word queryResult;
   string typeString = "";
   string errorString = "";
   bool correct;
   bool evaluable;
   bool defined;
   bool isFunction;
   // use the queryprocessor for executing the expression
   qp->ExecuteQuery(expression, queryResult, 
                    typeString, errorString, correct, 
                    evaluable, defined, isFunction);
   // check correctness of the expression
   if(!correct || !evaluable || !defined || isFunction){
      assert(queryResult.addr == 0);
      return listutils::typeError("could not extract evaluate the "
                                  "type description");
   }
   TupleDescr* fn = (TupleDescr*) queryResult.addr;
   assert(fn);
   if(!fn->IsDefined()){
      fn->DeleteIfAllowed();
      return listutils::typeError("filename undefined");
   }
   ListExpr attrList = fn->GetTupleTypeExpr();
   delete fn;
   ListExpr res = nl->TwoElemList(listutils::basicSymbol<Tuple>(),attrList);
   if(!Tuple::checkType(res)){
     cout << nl->ToString(res) << endl;
     return listutils::typeError("Tuple desscription represents "
                                 "not a valid tuple");
   }
   return res; 
}

OperatorSpec TTYPE3Spec(
 "ANY x ANY x tupledecscr x ... -> Tuple(TUPLEDESCR)",
 " op(_) ",
 "Extracts the tuple type from a tupledescr thet is the third arg.",
 "Type Mapping operator"
);

Operator TTYPE3_Op(
  "TTYPE3",
  TTYPE3Spec.getStr(),
  0,
  Operator::SimpleSelect,
  TTYPE3TM
);





} // end of namespace
