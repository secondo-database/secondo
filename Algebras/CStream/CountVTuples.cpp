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


1 Operator ~count~

This operator count the number of vtuples within a stream

*/


#include "VTuple.h"
#include "NestedList.h"
#include "Operator.h"
#include "Stream.h"
#include "ListUtils.h"
#include "StandardTypes.h"

namespace cstream{

ListExpr countTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("One element expected"); 
  }
  std::string err = "stream(vtuple) expected";
  ListExpr a1 = nl->First(args);
  if(!nl->HasLength(a1,2)){
    return listutils::typeError(err);
  }
  if(!listutils::isSymbol(nl->First(a1), Stream<VTuple>::BasicType())){
    return listutils::typeError(err);
  }
  ListExpr vt = nl->Second(a1);
  if(!nl->HasLength(vt,1)){
    return listutils::typeError(err);
  }
  if(!VTuple::CheckType(nl->First(vt))){
    return listutils::typeError(err);
  }

  return listutils::basicSymbol<CcInt>();
}

int countVM(Word* args, Word& result, int message,
              Word& local, Supplier s) {

  result = qp->ResultStorage(s);
  CcInt* res = (CcInt*) result.addr;
  Stream<VTuple> stream(args[0]);
  stream.open();
  VTuple* t;
  int c = 0;
  while ((t=stream.request())){
     c++;
     t->DeleteIfAllowed();
  }
  stream.close();
  res->Set(true,c);
  return 0;
}

OperatorSpec countSpec(
  "stream(vtuple) -> int",
  "_ count",
  "Count the number of v-tuples in a stream.",
  "query receivestream('ais.txt',\"ais\") count"
);

Operator count_Op(
  "count",
   countSpec.getStr(),
   countVM,
   Operator::SimpleSelect,
   countTM
);





} // end of namespace


