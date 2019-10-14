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

*/

#include <ListUtils.h>
#include <../../../Relation-C++/RelationAlgebra.h>
#include <StandardTypes.h>
#include "Stream.h"
#include "ClosePregelWorkers.h"
#include "../../MessageBroker/MessageWrapper.h"
#include "../../PregelContext.h"
#include "../../Helpers/WorkerConfig.h"
#include "../Messaging/MessageDistribute.h"
#include "../../Helpers/Commander.h"
#include "../../typedefs.h"

namespace pregel {

 ListExpr ClosePregelWorkers::typeMapping(ListExpr args) {
   if(!nl->IsEmpty(args)){
     return listutils::typeError("No arguments expected");
   }  
   return  listutils::basicSymbol<CcInt>();
 }

 int ClosePregelWorkers::valueMapping(Word *args, Word &result, int,
                                      Word &, Supplier s) {
  result = qp->ResultStorage(s);
  CcInt* res = static_cast<CcInt*>(result.addr);
  int count = PregelContext::get().closeConnections();
  res->Set(true,count);
  return 0;
 }

 OperatorSpec ClosePregelWorkers::operatorSpec(
  " -> int",
  "closePregelWorkers ()",
  "This operator closes all existing connections to workers."
  "It returns the number of closed Connections.",
  "query closePregelConnections()"
 );

 Operator ClosePregelWorkers::closePregelWorkers(
  "closePregelConnections",
  ClosePregelWorkers::operatorSpec.getStr(),
  ClosePregelWorkers::valueMapping,
  Operator::SimpleSelect,
  ClosePregelWorkers::typeMapping
 );

}


