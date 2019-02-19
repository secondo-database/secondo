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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header File of the class of operator ~setPregelFunction~

November 2018, J. Mende


[TOC]

1 Overview

This header file contains definitions of type mapping, vallue mapping and the operator specification.

2 Defines and includes

*/

#include <ListUtils.h>
#include <../../../Relation-C++/RelationAlgebra.h>
#include <StandardTypes.h>
#include "../../../Stream/Stream.h"
#include "InitPregelMessages.h"
#include "../../MessageBroker/MessageWrapper.h"
#include "../../PregelContext.h"
#include "../../Helpers/WorkerConfig.h"
#include "../Messaging/MessageDistribute.h"
#include "../../Helpers/Commander.h"
#include "../../typedefs.h"

namespace pregel {

 ListExpr InitPregelMessages::typeMapping(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
   return listutils::typeError("You must provide 1 argument.");
  }
  auto stream = nl->First(args);

  if (!listutils::isTupleStream(stream)) {
   return listutils::typeError(
    "The first argument must be a stream of type tuple");
  }

  ListExpr tupleType = nl->Second(stream);

  std::string &savedMessageType = PregelContext::get().getMessageType();
  if (savedMessageType.empty() ||
      savedMessageType != nl->ToString(tupleType)) {
   return listutils::typeError(
    "Tuple type doesn't match that of the compute function "
    "(or wasn't yet set)");
  }

  return listutils::basicSymbol<CcBool>();
 }

 int InitPregelMessages::valueMapping(Word *args, Word &result, int,
                                      Word &, Supplier s) {
  result = qp->ResultStorage(s);
  Stream<Tuple> messageStream(args[0]);

  MessageBroker &broker = MessageBroker::get();

  const supplier<pregel::WorkerConfig> &workers =
   PregelContext::get().getWorkers();
  std::string query(
   "query expectPregelMessages()");

  for (WorkerConfig *worker = workers();
       worker != nullptr; worker = workers()) {
   try {
    Commander::remoteQuery(worker->connection, query,
                           Commander::throwWhenFalse);
   } catch (RemoteExecutionException &e) {
    BOOST_LOG_TRIVIAL(error) << "Query FAILED: " << query.c_str();
    ((CcBool *) result.addr)->Set(true, false);
    return 0;
   }
  }

  MessageDistribute::distributeMessages(messageStream, broker);

  broker.broadcastInitDoneMessage();

  ((CcBool *) result.addr)->Set(true, true);
  return 0;
 }

 OperatorSpec InitPregelMessages::operatorSpec(
  "stream(tuple) -> bool",
  "_ #",
  "This operator initializes the Pregel system with messages from the "
  "(logical) superstep '0'."
  "It takes a stream of messages and sends them to the workers via the message "
  "clients. The type of the tuples on the stream must adhere to "
  "the type that was defined beforehand."
  "The operator returns TRUE, as long as the workers are reachable and set up."
  "NOTE: You must run 'setPregelFunction(...) first.'",
  "query InitMessages feed initPregelMessages;",
  "This operator belongs to the Pregel API."
  "It may require knowledge of the system to effectively understand and "
  "use all the operators that are provided."
 );

 Operator InitPregelMessages::initPregelMessages(
  "initPregelMessages",
  InitPregelMessages::operatorSpec.getStr(),
  InitPregelMessages::valueMapping,
  Operator::SimpleSelect,
  InitPregelMessages::typeMapping
 );
}


