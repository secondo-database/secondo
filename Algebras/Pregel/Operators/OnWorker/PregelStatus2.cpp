/*
----
This file is part of SECONDO.

Copyright (C) 2019, 
University in Hagen, 
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header File of the class ~PregelAlgebra~

November 2018, J. Mende


[TOC]

1 Overview

This file contains definitions of the members of class PregelStatus2

*/

#include <ListUtils.h>
#include <regex>
#include "Stream.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include <StandardTypes.h>
#include "PregelStatus2.h"
#include "../../Helpers/PregelStatus2Helper.h"
#include <boost/log/trivial.hpp>
#include "../../PregelAlgebra.h"
#include "../../../FText/FTextAlgebra.h"
#include "../../Helpers/Commander.h"
#include "Algebras/Distributed2/CommandLog.h"

namespace pregel {

 ListExpr PregelStatus2::typeMapping(ListExpr args) {
  if (!nl->HasLength(args, 0)) {
   return listutils::typeError("You must provide no arguments.");
  }
  PregelStatus2Helper ps2h;
  ListExpr tt = ps2h.getTupleType();
  return Stream<Tuple>::wrap(tt);
 }


class pregelStatus2Info{

  public:
      pregelStatus2Info(ListExpr _tupleType){
         tupleType = _tupleType;
         numTupleType = SecondoSystem::GetCatalog()->NumericType(tupleType);
         numTupleType = nl->TwoElemList( numTupleType, 
                                      nl->ListLength(nl->Second(numTupleType)));
         local = true;
         workers = PregelContext::get().getWorkers();
         lastRelValue = nl->TheEmptyList();
         query = "query pregelStatus2() consume";
      }

      ~pregelStatus2Info(){

      }

      Tuple* next() {
         if(local){
           Tuple* res = getLocal();
           local = false;
           return res;
         }                 
         return getTupleForWorker();
      }              

  private:
     ListExpr tupleType;
     ListExpr numTupleType;
     bool local;
     PregelStatus2Helper ps2h;
     supplier<WorkerConfig> workers; 
     ListExpr lastRelValue;
     std::string query;
     distributed2::CommandLog cmdLog;
     WorkerConnection* con; // last used connection


     Tuple* getLocal(){
        auto algebra = PregelAlgebra::getAlgebra();
        ps2h.reset();
        algebra->healthReport(ps2h);
        return ps2h.getTuple();
     }             

     Tuple* getTupleForWorker(){
         while(nl->IsEmpty(lastRelValue)){
            WorkerConfig* wc = workers();
            if(wc==nullptr) return 0; // no more workers available
            con = wc->connection;
            // send query to worker
            int error;
            std::string errorMsg;
            ListExpr resList;
            double runTime;
            con->simpleCommand(query,
                   error, errorMsg,
                   resList,
                   false,
                   runTime,
                   false,
                   cmdLog,
                   true,
                   0);
            if(!error){
               assert(nl->HasLength(resList,2));
               ListExpr resType = nl->First(resList);
               lastRelValue = nl->Second(resList);
               assert(Relation::checkType(resType));
               assert(nl->Equal(tupleType,nl->Second(resType)));
            } else {
               std::cerr << "error during requesting pregel status from worker"
                         << std::endl;
               std::cerr << errorMsg << std::endl;
            }
         }                
         ListExpr tupleValue = nl->First(lastRelValue);
         lastRelValue = nl->Rest(lastRelValue);
         // read in tuple
         int errorPos=0;
         ListExpr errorInfo = listutils::emptyErrorInfo();
         bool correct;
         Tuple* res = Tuple::In(numTupleType, tupleValue,errorPos,
                                errorInfo, correct);
         assert(correct);
         ps2h.setHost(res,con->getHost());
         ps2h.setPort(res, con->getPort());
         ps2h.setPID(res, con->serverPid());
         return res;  
     }



};


 int PregelStatus2::valueMapping(Word*, Word &result, int message,
                                Word & local, Supplier s) {

    pregelStatus2Info* li = (pregelStatus2Info*) local.addr;
    switch(message){
       case OPEN :  if(li){
                       delete li;
                    }
                    local.addr = new pregelStatus2Info(
                                        nl->Second(qp->GetType(s)));
                    return 0;
       case REQUEST : result.addr = li?li->next():0;
                      return result.addr?YIELD:CANCEL;
       case CLOSE : if(li){
                      delete li;
                      local.addr = 0;
                    }                
                    return 0;            
    }            
    return -1; // should never be reached

 }



 OperatorSpec PregelStatus2::operatorSpec(
  " -> stream(tuple)",
  "_ ()",
  "Returns a relation containing information about the "
  "current state of the pregel system.",
  "query pregelStatus2() consume;"
 );

 Operator PregelStatus2::pregelStatus2(
  "pregelStatus2",
  PregelStatus2::operatorSpec.getStr(),
  PregelStatus2::valueMapping,
  Operator::SimpleSelect,
  PregelStatus2::typeMapping
 );
}
