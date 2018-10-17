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

[1] Header File of the class ~PregelAlgebra~

November 2018, J. Mende


[TOC]

1 Overview

This file contains definitions of the members of classes MessageDistribute

*/

#include <ListUtils.h>
#include <../../../Relation-C++/RelationAlgebra.h>
#include "MessageDistribute.h"
#include "../../PregelContext.h"

namespace pregel {
 ListExpr pregel::MessageDistribute::typeMapping(ListExpr args) {
//  FORCE_LOG
  if (!nl->HasLength(args, 1)) {
   return listutils::typeError("You must provide 1 argument.");
  }
  auto stream = nl->First(args);

  if (!Stream<Tuple>::checkType(stream)) {
   return listutils::typeError(
    "The first argument must be a stream of type tuple");
  }

  auto tupleType = nl->Second(stream);
  ListExpr savedType;
  auto couldParse = nl->ReadFromString(PregelContext::get().getMessageType(),
                                       savedType);
  if (!couldParse || !nl->Equal(savedType, tupleType)) {
   return listutils::typeError(
    "The tuple type must be coherent. Maybe it wasn't set yet. "
    "Call \"initPregelMessages\" to set it.");
  }

  return nl->SymbolAtom(CcBool::BasicType());
 }

 int MessageDistribute::valueMapping(Word *args, Word &result, int ignored,
                                     Word &local, Supplier s) {
  FORCE_LOG
  result = qp->ResultStorage(s);
  Stream<Tuple> messageStream(args[0]);

  MessageBroker &broker = MessageBroker::get();
  if (!broker.clientsAlive()) {
   BOOST_LOG_TRIVIAL(error) << "Clients not ready. Abort.";
   return -1;
  }

  distributeMessages(messageStream, broker);

  ((CcBool *) result.addr)->Set(true, true);
  return 0;
 }

 OperatorSpec MessageDistribute::operatorSpec(
  "stream(tuple) x int -> bool",
  "_ # [_]",
  "message stream x slot no -> success",
  "query messageFeed() messageDistribute[1];"
 );

 Operator MessageDistribute::messageDistribute(
  "messageDistribute",
  MessageDistribute::operatorSpec.getStr(),
  MessageDistribute::valueMapping,
  Operator::SimpleSelect,
  MessageDistribute::typeMapping
 );

 void MessageDistribute::distributeMessages(Stream<Tuple> stream,
                                            MessageBroker &broker) {
  const int round = SuperstepCounter::get();
  stream.open();
  Tuple *tuple;
  while ((tuple = stream.request()) != nullptr) {
   MessageWrapper *message = MessageWrapper::fromTuple(tuple, round);
   broker.sendMessage(message);
  }

  stream.close();
 }
}
