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


1 Operator ~printstream~

This operator prints out the  vtuples within a stream.

*/


#include "VTuple.h"
#include "NestedList.h"
#include "Operator.h"
#include "Algebras/Stream/Stream.h"
#include "ListUtils.h"
#include "StandardTypes.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"

namespace cstream{

ListExpr printstreamTM(ListExpr args){
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
  return nl->First(args);
}

int printstreamVM(Word* args, Word& result, int message,
                  Word& local, Supplier s) {

  Stream<VTuple>* stream = (Stream<VTuple>*) local.addr;


  switch(message){
   case OPEN:
           if(stream){
             stream->close();
             delete stream;
           }
           stream = new Stream<VTuple>(args[0]);
           stream->open();
           local.addr = stream;
           return 0;
   case REQUEST:{
            if(!stream) return CANCEL;
            VTuple* tup = stream->request();
            result.addr = tup;
            if(tup){
              cout << "received VTUPLE: " << endl;
              //tup->getTupleDescr()->Print(cout) << endl;
              //cout << "-------------------- " << endl;
              tup->getTuple()->Print(cout) << endl;
              cout << "==================== " << endl << endl;
            }
            return result.addr?YIELD:CANCEL;
        }
   case CLOSE:
          if(stream){
             stream->close();
             delete stream;
             local.addr = 0;
          } 
          return 0;
   }
   return -1;

  }


OperatorSpec printstreamSpec(
  "stream(vtuple) -> stream(vtuple)",
  "_ printstream",
  "prints out the  v-tuples in a stream.",
  "query receivestream('ais.txt',\"ais\") printstream count"
);

Operator printstream_Op(
  "printstream",
   printstreamSpec.getStr(),
   printstreamVM,
   Operator::SimpleSelect,
   printstreamTM
);





} // end of namespace


