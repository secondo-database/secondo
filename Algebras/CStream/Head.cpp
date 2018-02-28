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


1 Operator ~headnp~

Generic head operator without progress support


*/


#include "VTuple.h"
#include "NestedList.h"
#include "Operator.h"
#include "Algebras/Stream/Stream.h"
#include "ListUtils.h"
#include "StandardTypes.h"

extern QueryProcessor* qp;

namespace cstream{

ListExpr headTM(ListExpr args){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("2 args expected");
  }
  if(!listutils::isStream(nl->First(args))){
    return listutils::typeError("first arg must be a stream");
  }
  if(!CcInt::checkType(nl->Second(args))){
    return listutils::typeError("second arg must be of type int");
  }
  return nl->First(args);
}

class headInfo{
 public:
  headInfo(Word _stream, int count):stream(_stream), count(count){
     qp->Open(stream.addr);
  }

  ~headInfo(){
    qp->Close(stream.addr);
  }

  int next(Word& result){
     if(count<=0){
        result.addr = 0;
        return CANCEL;
     }
     qp->Request(stream.addr, result);
     count--;
     return qp->Received(stream.addr)?YIELD: CANCEL;
  }


 private:
   Word stream;
   int count;
};


int headVM(Word* args, Word& result, int message,
              Word& local, Supplier s) {

   headInfo* li = (headInfo*) local.addr;
   switch(message){
     case OPEN : {
                    if(li) delete li;
                    CcInt* h = (CcInt*) args[1].addr;
                    int n = h->IsDefined()?h->GetValue():0;
                    local.addr = new headInfo(args[0],n);
                    return 0;
                }
     case REQUEST: if(!li) return CANCEL;
                   return li->next(result);
     case CLOSE: {
                   if(li) {
                     delete li;
                     local.addr = 0;
                   }
                   return 0;
                 }

   }
   return -1;
}

OperatorSpec headSpec(
  "stream(ANY) x int -> stream(ANY)",
  "_ head[_]",
  "Cuts a stream after a certain number of elements",
  "query receivestream('ais.txt',\"ais\") head[10] count"
);

Operator head_Op(
  "headnp",
   headSpec.getStr(),
   headVM,
   Operator::SimpleSelect,
   headTM
);





} // end of namespace


