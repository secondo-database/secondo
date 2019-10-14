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
#include <StandardTypes.h>
#include "InitPregelMessagesWorker.h"
#include "../../MessageBroker/MessageBroker.h"
#include "../../PregelContext.h"
#include "../../Helpers/Metrics.h"
#include <boost/thread.hpp>
#include "../../../../include/SecParser.h"
#include "../Messaging/MessageDistribute.h"
#include "Stream.h"
#include "../../PregelAlgebra.h"
#include "StartPregelWorker.h"

namespace pregel {

 ListExpr InitPregelMessagesWorker::typeMapping(ListExpr args) {
  if(PregelAlgebra::getAlgebra()->amITheMaster()){
    return listutils::typeError("this is a worker only operator, "
                                "please use it in combination with"
                                " remotePregelCommand");
  }

  if (!nl->HasLength(args, 1)) {
   return listutils::typeError("You must provide 1 argument.");
  }

  // the argument must be a stream of message type
  std::string msgtype = PregelContext::get().getMessageType();

  if(msgtype.length()==0){
    return listutils::typeError("Message type is not known yet");
  }
  ListExpr mt = nl->TheEmptyList();
  if(!nl->ReadFromString(msgtype, mt)){
    return listutils::typeError("Internal error, message type can't be parsed");
  }

  ListExpr msgStream = Stream<Tuple>::wrap(mt);
  if(!nl->Equal(nl->First(args), msgStream)){
     return listutils::typeError("The argument is not a "
                                 "stream of message type");
  }
  return listutils::basicSymbol<CcInt>();
  
 }

 int InitPregelMessagesWorker::valueMapping(Word *args, Word &result, int,
                                     Word &local, Supplier s) {

  result = qp->ResultStorage(s);
  CcInt* res = static_cast<CcInt*>(result.addr);
  Stream<Tuple> stream(args[0]);

  bool allEmpty = false;
  boost::thread receiver(
  boost::bind(&startReceivingMessages,
                boost::ref(allEmpty)));

  size_t noMsg = MessageDistribute::distributeMessages(stream,
                                                       MessageBroker::get());

  MessageBroker::get().broadcastFinishMessage();

  receiver.join();


  res->Set(true, noMsg);
  return 0;
 }

 void InitPregelMessagesWorker::startReceivingMessages(bool &allEmpty) {
  MessageBroker &broker = MessageBroker::get();

  bool receivedFromAll = false;
  boost::mutex lock;
  boost::condition_variable synch;

  executable callMeWhenYoureDone = [&receivedFromAll, &lock, &synch]() {
    {
     boost::lock_guard<boost::mutex> guard(lock);
     receivedFromAll = true;
    }
    synch.notify_one();
  };

  broker.startNewRound(allEmpty, callMeWhenYoureDone);

  boost::unique_lock<boost::mutex> unique_lock(lock);
  synch.wait(unique_lock, [&receivedFromAll] () {
   return receivedFromAll;
  });
 }


 OperatorSpec InitPregelMessagesWorker::operatorSpec(
  "stream(messagetype) -> int",
  "_ #",
  "This operator distributes messages created on workers "
  "without using the predefined function but a "
  "given tuple stream. ",
  "query remotePregelCommand( myMessages feed initPregelWorker)"
 );

 Operator InitPregelMessagesWorker::initPregelMessagesWorker(
  "initPregelMessagesWorker",
  InitPregelMessagesWorker::operatorSpec.getStr(),
  InitPregelMessagesWorker::valueMapping,
  Operator::SimpleSelect,
  InitPregelMessagesWorker::typeMapping
 );
}
