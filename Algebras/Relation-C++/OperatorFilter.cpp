/*
----
This file is part of SECONDO.

Copyright (C) 2016,
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


//[$][\$]
//[_][\_]

*/
#include "OperatorFilter.h"

#include "Algebra.h"
#include "StandardTypes.h"
#include "Progress.h"
#include "RelationAlgebra.h"
#include "Stream.h"

#ifdef USE_PROGRESS
#include "../CostEstimation/RelationAlgebraCostEstimation.h"
#endif

using namespace std;

/*
5.8 Operator ~filter~

Only tuples, fulfilling a certain condition are passed on to the
output stream.

5.8.1 Type mapping function of operator ~filter~

Result type of filter operation.

----    ((stream (tuple x)) (map (tuple x) bool))
               -> (stream (tuple x))
----

Type mapping function modified to show the possibility of getting
not only types but also arguments in the type mapping. This happens
when an operator
registers "UsesArgsinTypeMapping". Type list now has the form

----  ( (type1 arg1) (type2 arg2) )
----

that is

----  (
      ( (stream (tuple x))  arg1 )
      ( (map (tuple x) bool)  arg2 )
    )
----

*/
ListExpr OperatorFilter::FilterTypeMap(ListExpr args)
{

  if(!nl->HasLength(args,2)){
    return listutils::typeError("two arguments expected");
  }

  if(!nl->HasLength(nl->First(args),2)){
    ErrorReporter::ReportError("the first argument "
                               " should be a (type, expression) pair");
    return nl->TypeError();
  }

  if(!listutils::isTupleStream(nl->First(nl->First(args)))){
    ErrorReporter::ReportError("first argument must be a stream of tuples");
    return nl->TypeError();
  }

  if(!nl->HasLength(nl->Second(args),2)){
    ErrorReporter::ReportError("the second argument "
                               " should be a (type, expression) pair");
    return nl->TypeError();
  }


  ListExpr map = nl->First(nl->Second(args));
  if(!listutils::isMap<1>(map)){ // checking for a single arg
     ErrorReporter::ReportError("map: tuple -> bool expected as the"
                                " second argument");
     return nl->TypeError();
  }

  ListExpr mapres = nl->Third(map);
  if(!CcBool::checkType(mapres)){
    ErrorReporter::ReportError("map is not a predicate");
    return nl->TypeError();
  }

  if(!nl->Equal(nl->Second(nl->First(nl->First(args))), nl->Second(map))){
    ErrorReporter::ReportError("map and tuple type are not consistent");
    return nl->TypeError();
  }

  //just for demonstrating "UsesArgsInTypeMapping"

  bool showArguments = false;
  if ( showArguments ) {
        cout << "arguments to the filter operator:" << endl;
      cout << "first argument: ";
    nl->WriteListExpr( nl->Second(nl->First(args)), cout, 2 );
    cout << endl;
      cout << "second argument: ";
    nl->WriteListExpr( nl->Second(nl->Second(args)), cout, 2 );
        cout << endl;
    cout << endl;
  }



  return nl->First(nl->First(args));
}

/*
5.8.2 Value mapping function of operator ~filter~

*/


#ifndef USE_PROGRESS

// standard version

int
OperatorFilter::Filter(Word* args, Word& result, int message,
       Word& local, Supplier s)
{
  bool found = false;
  Word elem, funresult;
  ArgVectorPointer funargs;
  Tuple* tuple = 0;

  switch ( message )
  {

    case OPEN:

      qp->Open (args[0].addr);
      return 0;

    case REQUEST:

      funargs = qp->Argument(args[1].addr);
      qp->Request(args[0].addr, elem);
      found = false;
      while (qp->Received(args[0].addr) && !found)
      {
        tuple = (Tuple*)elem.addr;
        (*funargs)[0] = elem;
        qp->Request(args[1].addr, funresult);
        if (((Attribute*)funresult.addr)->IsDefined())
        {
          found = ((CcBool*)funresult.addr)->GetBoolval();
        }
        if (!found)
        {
          tuple->DeleteIfAllowed();
          qp->Request(args[0].addr, elem);
        }
      }
      if (found)
      {
        result.setAddr(tuple);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE:

      qp->Close(args[0].addr);
      return 0;
  }
  return 0;
}

#else
CostEstimation* OperatorFilter::FilterCostEstimationFunc(){
   return new FilterCostEstimation();
}

int
OperatorFilter::Filter(Word* args, Word& result, int message,
       Word& local, Supplier s)
{
  FilterLocalInfo* li = (FilterLocalInfo*) local.addr;
  switch ( message )
  {
    case OPEN:{
      if(li){
        delete li;
      }
      local.addr = new FilterLocalInfo(args[0], args[1],
                             (FilterCostEstimation*) qp->getCostEstimation(s));
      return 0;
    }

    case REQUEST:{
      if(!li){
        result.addr = 0;
      } else {
        result.addr = li->next();
      }
      return result.addr?YIELD:CANCEL;
    }
    case CLOSE:{
      if(li){
        delete li;
        local.addr = 0;
      };
      return 0;
    }
  }
  return -1;
}

#endif
