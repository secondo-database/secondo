/*
----
This file is part of SECONDO.

Copyright (C) 2015,
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


#include "StandardTypes.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "../Relation-C++/RelationAlgebra.h"
#include "../FText/FTextAlgebra.h"
#include "../Stream/Stream.h"
#include "../Distributed2/CommandLogger.h"
#include "../Distributed2/ConnectionInfo.h"
#include "../Distributed2/DArray.h"



/*
Operator ~dmapcommands~

This operator returns the commands sended by a dmap operator to the workers.

Because we use some types of the Distributed2Algebra, we can assume that the
Distributed2Algebra is present. This is also forced by the makefile.

We reuse the Type mapping of the dmap operator to check whether the 
arguments are correct or not. We just change the return type.

*/

namespace distributed2{
   ListExpr dmapTM(ListExpr args);
   template<class A>
   void dmapcommandsVMT(Word* args, Supplier s, CommandLogger* log );
} // end of namespace distributed2



namespace distributed4{

ListExpr dmapcommandsTM(ListExpr args){
  ListExpr result = distributed2::dmapTM(args);
  ListExpr typeerror = listutils::typeError();
  if(nl->Equal(typeerror, result)){
    return typeerror;
  }
  // the result list
  ListExpr attrList = nl->FiveElemList(
               nl->TwoElemList( nl->SymbolAtom("Host"),
                                listutils::basicSymbol<FText>()),
               nl->TwoElemList( nl->SymbolAtom("Port"),
                                listutils::basicSymbol<CcInt>()),
               nl->TwoElemList( nl->SymbolAtom("Worker"),
                                listutils::basicSymbol<CcInt>()),
               nl->TwoElemList( nl->SymbolAtom("CmdNr"),
                                listutils::basicSymbol<CcInt>()),
               nl->TwoElemList( nl->SymbolAtom("Command"),
                                listutils::basicSymbol<FText>()));

  ListExpr streamType = 
              nl->TwoElemList(
                     listutils::basicSymbol<Stream<Tuple> >(),
                     nl->TwoElemList(
                         listutils::basicSymbol<Tuple>(),
                         attrList));

  
  // TODO: wrap it into some code type

  // we append the appendlist returned by the dmapTM, 
  // appended are
  // 1: the function as a text
  // 2: isRel: the result of the function is a relation
  // 3: isStream : the function result is a tuple stream
 
  return nl->ThreeElemList(
           nl->SymbolAtom(Symbols::APPEND()),
           nl->Second(result),
           streamType
         ); 
}

template<class A>
void dmapcommandsVMT(Word* args, Supplier s, distributed2::CommandLogger* log );

template<class A>
int dmapcommandsVMT(Word* args, Word& result, int message,
            Word& local, Supplier s ){
   typedef std::pair<distributed2::CommandLogger*, size_t> InfoT;
   InfoT* li = (InfoT*) local.addr;
   TupleType* tt = (TupleType*) qp->GetLocal2(s).addr;

   switch(message){
      case INIT: {
         tt = new TupleType(nl->Second(GetTupleResultType(s)));
         qp->GetLocal2(s).addr= tt;
         return 0;
      }
      case FINISH: {
         if(tt){
            tt->DeleteIfAllowed();
            qp->GetLocal2(s).addr = 0;
         }
         return 0;
      }

      case OPEN: if(li){
                   li->first->reset();
                   li->second=0;
                 } else {
                   distributed2::CommandLogger* log = 
                               new distributed2::CommandLogger();
                   size_t pos = 0;
                   li = new InfoT(log,pos); 
                   local.addr = li;
                 }
                 distributed2::dmapcommandsVMT<A>(args,s,li->first);
                 return 0;
      case REQUEST: {
           if(!li) return CANCEL;
           if(li->second >= li->first->size()){
              return CANCEL;
           }
           std::pair<distributed2::ConnectionInfo*, std::string> p 
                                             = (*(li->first))[li->second];
           li->second++;
           Tuple* res = new Tuple(tt);
           res->PutAttribute(0, new FText(true,p.first->getHost()));
           res->PutAttribute(1, new CcInt(true,p.first->getPort()));
           res->PutAttribute(2, new CcInt(true,p.first->getNum()));
           res->PutAttribute(3, new CcInt(true,li->second));
           res->PutAttribute(4, new FText(true,p.second));
           result.addr = res;
           return YIELD;
      }
      case CLOSE:{
         if(li){
           delete li->first;
           delete li;
           local.addr = 0;
         }
         return 0;
      }
   }
   return -1;
}

ValueMapping dmapcommandsVM[] = {
   dmapcommandsVMT<distributed2::DArray>,
   dmapcommandsVMT<distributed2::DFArray>
};

int dmapcommandsSelect(ListExpr args){
  return distributed2::DArray::checkType(nl->First(args))?0:1;
}

OperatorSpec dmapcommandsSpec(
  "d[f]array(A)  x string x (A->B) -> stream(tuple)",
  "_ dmapcommands[_,_]",
  "Returns the commands that would be executed on workers during "
  "a dmap operation of the distributed2 algebra. The commands are "
  "not executed but worker connections must be available  to "
  "get some informations from the workers, e.g. the database "
  "directory.",
  "query da2 dmapcommands[\"dan2\", . * 2] consume"
);

Operator dmapcommandsOp(
  "dmapcommands",
  dmapcommandsSpec.getStr(),
  2,
  dmapcommandsVM,
  dmapcommandsSelect,
  dmapcommandsTM  
);





class Distributed4Algebra : public Algebra {
   public:
     Distributed4Algebra() : Algebra() {

      AddOperator(&dmapcommandsOp);
      dmapcommandsOp.enableInitFinishSupport();
      dmapcommandsOp.SetUsesArgsInTypeMapping();
        
     }
};

} // end of namespace distributed4


extern "C"
Algebra*
   InitializeDistributed4Algebra( NestedList* nlRef,
                           QueryProcessor* qpRef ) {
   return new distributed4::Distributed4Algebra;
}












