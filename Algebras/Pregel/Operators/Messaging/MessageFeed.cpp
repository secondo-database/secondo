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

This file contains definitions of the members of class MessageFeed

*/

#include <ListUtils.h>
#include <../../../Relation-C++/RelationAlgebra.h>
#include <Stream.h>
#include "MessageFeed.h"
#include "../../MessageBroker/MessageBroker.h"
#include "../../PregelContext.h"

namespace pregel {

 ListExpr MessageFeed::typeMapping(ListExpr args) {
  if (!nl->HasLength(args, 0)) {
   return listutils::typeError("You must provide no arguments.");
  }
  std::string savedType = PregelContext::get().getMessageType();
  ListExpr tupleType;
  if (!nl->ReadFromString(savedType, tupleType)) {
   return listutils::typeError("Can't parse saved tuple type \"" +
   savedType + "\". Was it set?");
  }

  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                         tupleType);
 }

 int
 MessageFeed::valueMapping(Word *, Word &result, int streamOp, Word &local,
                           Supplier) {
  supplier2<MessageWrapper> *messageSupplier;

  switch (streamOp) {
   case OPEN : {
    MessageBroker &broker = MessageBroker::get();

    const int lastRound = SuperstepCounter::get() - 1;
    messageSupplier = broker.inboxSupplier(lastRound);
    local.addr = messageSupplier;
    return 0;
   }

   case REQUEST : {
    messageSupplier = (supplier2<MessageWrapper> *) local.addr;
    std::shared_ptr<MessageWrapper> message = (*messageSupplier)();
    if (message != nullptr) {
     result.setAddr(message->getBody1());
     //delete message; 
     return YIELD;
    } else {
     return CANCEL;
    }
   }

   case CLOSE :
    if (local.addr) {
     messageSupplier = (supplier2<MessageWrapper> *) local.addr;
     delete messageSupplier;
     local.addr = nullptr;
    }
    return 0;

   default:
    BOOST_LOG_TRIVIAL(warning) << "DEFAULT case. Abort.";
    break;
  }
  return 0;
 }

 OperatorSpec MessageFeed::operatorSpec(
  "() -> bool",
  "#",
  "This operator consumes the message buffer of the message broker that is a "
  "component of the Pregel system."
  "Each message is a tuple and is provided as a stream as the result "
  "of the operator.",
  "query messageFeed() consume;",
  "This operator belongs to the Pregel API."
  "It may require knowledge of the system to effectively understand and use "
  "all the operators that are provided."
  "CAUTION: This operator is used internally by the Pregel system. "
  "Hence you must not use it in queries yourself."
  "Doing so may lead to inconsistent states of the Pregel system."
 );

 Operator MessageFeed::messageFeed(
  "messageFeed",
  MessageFeed::operatorSpec.getStr(),
  MessageFeed::valueMapping,
  Operator::SimpleSelect,
  MessageFeed::typeMapping
 );
}
