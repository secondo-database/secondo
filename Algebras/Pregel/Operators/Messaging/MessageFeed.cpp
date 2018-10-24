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
#include <../../../Stream/Stream.h>
#include "MessageFeed.h"
#include "../../MessageBroker/MessageBroker.h"
#include "../../PregelContext.h"

namespace pregel {

 ListExpr MessageFeed::typeMapping(ListExpr args) {
//  FORCE_LOG
  if (!nl->HasLength(args, 0)) {
   return listutils::typeError("You must provide no arguments.");
  }
  auto savedType = PregelContext::get().getMessageType();
  ListExpr tupleType;
  if (!nl->ReadFromString(savedType, tupleType)) {
   return listutils::typeError("Wrong tuple type.");
  }

  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                         tupleType);
 }

 int
 MessageFeed::valueMapping(Word *, Word &result, int streamOp, Word &local,
                           Supplier) {
  supplier<MessageWrapper> *messageSupplier;

  switch (streamOp) {
   case OPEN : {
    MessageBroker &broker = MessageBroker::get();

    const int lastRound = SuperstepCounter::get() - 1;
    messageSupplier = broker.inboxSupplier(lastRound);
    local.addr = messageSupplier;
    return 0;
   }

   case REQUEST : {
    messageSupplier = (supplier<MessageWrapper> *) local.addr;
    MessageWrapper *message;
    if ((message = (*messageSupplier)()) != nullptr) {
     result.setAddr(message->getBody());
     return YIELD;
    } else {
     return CANCEL;
    }
   }

   case CLOSE :
    if (local.addr) {
     messageSupplier = (supplier<MessageWrapper> *) local.addr;
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
  "() -> success",
  "query messageFeed() consume;"
 );

 Operator MessageFeed::messageFeed(
  "messageFeed",
  MessageFeed::operatorSpec.getStr(),
  MessageFeed::valueMapping,
  Operator::SimpleSelect,
  MessageFeed::typeMapping
 );
}