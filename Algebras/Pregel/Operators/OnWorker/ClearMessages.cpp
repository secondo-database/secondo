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



[TOC]

1 Overview

This file contains definitions of the members of class ClearMessages 

*/

#include <ListUtils.h>
#include <StandardTypes.h>
#include "ClearMessages.h"
#include "../../PregelAlgebra.h"
#include "../../MessageBroker/MessageBroker.h"
#include "../OnMaster/QueryRunner.h"


namespace pregel {



 ListExpr ClearMessages::typeMapping(ListExpr args) {
    if(!nl->IsEmpty(args)){
      return listutils::typeError("No argument expected");
    }   
    return listutils::basicSymbol<CcBool>();
 }

 int ClearMessages::valueMapping(Word*, Word &result, int message,
                                Word & local, Supplier s) {

   bool r = true;
   result = qp->ResultStorage(s);
   MessageBroker::get().clearMessages(); // clear local messages
   // when master => class clearPregelMessages on all connected workers
   if(PregelAlgebra::getAlgebra()->amITheMaster()){
     // cout << "On Master => class clearPregelMessages to all workers" << endl;
      const std::string query = "query clearPregelMessages()";
      const supplier<pregel::WorkerConfig> &workers =
         PregelContext::get().getWorkers();

      std::function<std::string *(std::string &)> mapper =
        [](std::string &result) {
            return new std::string(result);
         };

      std::vector<QueryRunner*> runners;

      for (auto worker = workers(); worker != nullptr; worker = workers()) {
         WorkerConnection* w = worker->connection;
         if(w){
            runners.push_back(new QueryRunner(w,query));
         }
      }

      for(QueryRunner* qr : runners){
         if(!qr->successful()){
           r = false;
         }
         delete qr;
      }
   }
   CcBool* res = (CcBool*) result.addr;
   res->Set(true,r); 
   return 0;
 }



 OperatorSpec ClearMessages::operatorSpec(
   " -> bool",
   "_ ()",
   "Removes all queued messages.",
   "query clearPregelMessages();"
 );

 Operator ClearMessages::clearMessages(
  "clearPregelMessages",
  ClearMessages::operatorSpec.getStr(),
  ClearMessages::valueMapping,
  Operator::SimpleSelect,
  ClearMessages::typeMapping
 );
}


