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

This file contains definitions of the members of class StartPregelWorker

*/

#include <ListUtils.h>
#include <StandardTypes.h>
#include "StartPregelWorker.h"
#include "../../MessageBroker/MessageBroker.h"
#include "../../PregelContext.h"
#include <boost/thread.hpp>
#include "../../../../include/SecParser.h"

namespace pregel {

 ListExpr StartPregelWorker::typeMapping(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
   return listutils::typeError("You must provide 1 argument.");
  }
  const ListExpr rounds = nl->First(args);

  if (!CcInt::checkType(rounds)) {
   return listutils::typeError(
    "The first argument must be an int");
  }

  return nl->SymbolAtom(CcBool::BasicType());
 }

 int StartPregelWorker::valueMapping(Word *args, Word &result, int,
                                     Word &local, Supplier s) {
  result = qp->ResultStorage(s);
  CcInt *roundWrapper = (CcInt *) args[0].addr;

  if (!roundWrapper->IsDefined()) {
   BOOST_LOG_TRIVIAL(error) << "roundWrapper is undefined";
   ((CcBool *) result.addr)->Set(true, false);
   return -1;
  }
  const int rounds = roundWrapper->GetIntval();
  int round = 0;

  std::string function = PregelContext::get().getFunction();
  PRECONDITION(!function.empty(),
               "Function not defined. "
               "Please initialize with \"setPregelFunction(...) first.");

  QueryProcessor *queryProcessor = new QueryProcessor(
   SecondoSystem::GetNestedList(), SecondoSystem::GetAlgebraManager());

  while (round < rounds || rounds < 0) {
   SuperstepCounter::increment();

   const int lastRound = SuperstepCounter::get() - 1;
   unsigned long messagesToProcess = MessageBroker::get()
    .howManyMessagesInInbox(lastRound);

   bool allEmpty = messagesToProcess == 0;
   boost::thread receiver(
    boost::bind(&StartPregelWorker::startReceivingMessages,
                boost::ref(allEmpty)));

   compute(queryProcessor, function);

   if (messagesToProcess <= 0) {
    MessageBroker::get().broadcastEmptyMessage();
   } else {
    MessageBroker::get().broadcastFinishMessage();
   }

   receiver.join();

   if (allEmpty) {
    break;
   }
   ++round;
  }

  delete queryProcessor;
  ((CcBool *) result.addr)->Set(true, true);
  return 0;
 }

 void StartPregelWorker::startReceivingMessages(bool &allEmpty) {
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
  while (!receivedFromAll) {
   synch.wait(unique_lock);
  };
 }

 bool StartPregelWorker::compute(QueryProcessor *queryProcessor,
                                 std::string &function) {
  ListExpr query = nl->Second(convertToList(function));

  bool correct = false;
  bool evaluable = false;
  bool defined = false;
  bool isFunction = false;
  OpTree opTree;
  ListExpr resultType;

  queryProcessor->Construct(query, correct, evaluable, defined, isFunction,
                            opTree, resultType);

  if (!correct || !evaluable || !defined) {
   BOOST_LOG_TRIVIAL(error) << "Invalid Function: correct: " << correct
                            << ", evaluable: " << evaluable << ", defined: "
                            << defined << "; Abort.";
   return false;
  }

  auto result = queryProcessor->Request(opTree);

  if (result.addr == nullptr) {
   BOOST_LOG_TRIVIAL(error) << "Query returned nullptr. Abort.";
   return false;
  }

  auto successBool = ((CcBool *) result.addr);
  if (!successBool->IsDefined()) {
   BOOST_LOG_TRIVIAL(error) << "Query returned undefined. Abort.";
   return false;
  }
  if (successBool->GetValue() == false) {
   BOOST_LOG_TRIVIAL(error) << "Query returned FALSE. Abort.";
   return false;
  }

  return true;
 }

 ListExpr StartPregelWorker::convertToList(std::string &function) {
  ListExpr asList;
  if (!nl->ReadFromString(function, asList)) {
   BOOST_LOG_TRIVIAL(error) << "SecondoParser produced an invalid nested list";
   return nl->Empty();
  }
  return asList;
 }

 OperatorSpec StartPregelWorker::operatorSpec(
  "int -> bool",
  "# (_)",
  "rounds (negative for indefinite) -> success",
  "query startPregelWorker(0);"
 );

 Operator StartPregelWorker::startPregelWorker(
  "startPregelWorker",
  StartPregelWorker::operatorSpec.getStr(),
  StartPregelWorker::valueMapping,
  Operator::SimpleSelect,
  StartPregelWorker::typeMapping
 );
}