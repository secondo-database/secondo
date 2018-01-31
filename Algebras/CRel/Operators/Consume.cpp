/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

#include "Consume.h"

#include "CRel.h"
#include "CRelTC.h"
#include <cstdint>
#include <exception>
#include "LogMsg.h"
#include "OperatorUtils.h"
#include "QueryProcessor.h"
#include "Stream.h"
#include <string>
#include "TBlock.h"
#include "TBlockTC.h"
#include "TypeUtils.h"
#include "ToTuples.h"
#include "StreamValueMapping.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::exception;
using std::string;

extern NestedList *nl;
extern QueryProcessor *qp;

Consume::Consume() :
  Operator(info, ValueMapping, TypeMapping)
{
}

const OperatorInfo Consume::info = OperatorInfo(
  "consume", "stream(tblock) -> rel",
  "_ consume",
  "Creates an ordinary relation from a stream of tuple blocks.",
  "query cities feed consume");

ListExpr Consume::TypeMapping(ListExpr args)
{
   ListExpr tupleStream = ToTuples::TypeMapping(args);
   if(!Stream<Tuple>::checkType(tupleStream)){
     // an error
     return tupleStream;
   }
   ListExpr tuple = nl->Second(tupleStream);
   return nl->TwoElemList(listutils::basicSymbol<Relation>(),
                          tuple);
   
   
}

int Consume::ValueMapping(Word* args, Word &result, int message , 
                          Word& local, Supplier s)
{
 result = qp->ResultStorage(s);
 GenericRelation* rel = (GenericRelation*) result.addr;
 if(rel->GetNoTuples() > 0)
 {
     rel->Clear();
 }

 Word res;

 ::ValueMapping vm = StreamValueMapping<ToTuples::State>;

 vm(args,res,OPEN,local,s);

 while(vm(args,res,REQUEST,local,s)==YIELD){
    Tuple* tup = (Tuple*) res.addr;
    rel->AppendTuple(tup);
    tup->DeleteIfAllowed();
 }
 vm(args,res,CLOSE,local,s);
 result.setAddr(rel);
 return 0;
}
