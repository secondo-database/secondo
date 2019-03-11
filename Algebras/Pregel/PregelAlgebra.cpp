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

Definition of PregelAlgebra class

*/
#include <Symbols.h>
#include "PregelAlgebra.h"
#include "Operators/OnMaster/InitPregelMessages.h"
#include "Operators/OnMaster/SetPregelFunction.h"
#include "Operators/OnMaster/StartPregel.h"
#include "Operators/OnMaster/SetupPregel.h"
#include "Operators/OnMaster/RemotePregelCommand.h"
#include "Operators/OnMaster/ClosePregelWorkers.h"
#include "Operators/OnWorker/SetPregelFunctionWorker.h"
#include "Operators/Messaging/MessageDistribute.h"
#include "Operators/Messaging/StartMessageServer.h"
#include "Operators/Messaging/StartMessageClient.h"
#include "Operators/Messaging/MessageFeed.h"
#include "Operators/OnWorker/ResetPregel.h"
#include "Operators/OnWorker/ExpectPregelMessages.h"
#include "Operators/OnWorker/StartLoopbackMessageClient.h"
#include "Operators/OnWorker/PregelStatus.h"
#include "Operators/OnWorker/PregelStatus2.h"
#include "Operators/OnWorker/GetMessages.h"
#include "Operators/OnWorker/StartPregelWorker.h"
#include "PregelContext.h"
#include "Helpers/Metrics.h"

namespace pregel {
 extern QueryProcessor *qp;

 PregelAlgebra::PregelAlgebra() {
  /*
   * Operators
   * */
  AddOperator(&StartMessageClient::startMessageClient);
  AddOperator(&StartMessageServer::startMessageServer);
  AddOperator(&MessageDistribute::messageDistribute);
  AddOperator(&MessageFeed::messageFeed);

  AddOperator(&SetupPregel::setupPregel);
  AddOperator(&InitPregelMessages::initPregelMessages);
  AddOperator(&SetPregelFunction::setPregelFunction);
  SetPregelFunction::setPregelFunction.SetUsesArgsInTypeMapping();
  AddOperator(&StartPregel::startPregel);

  AddOperator(&ResetPregel::resetPregel);
  AddOperator(&ExpectPregelMessages::expectPregelMessages);
  AddOperator(&StartLoopbackMessageClient::startLoopbackMessageClient);
  AddOperator(&PregelStatus::pregelStatus);
  AddOperator(&PregelStatus2::pregelStatus2);
  AddOperator(&GetMessages::getMessages);
  AddOperator(&StartPregelWorker::startPregelWorker);
  AddOperator(&SetPregelFunctionWorker::setPregelFunctionWorker);
  SetPregelFunctionWorker::setPregelFunctionWorker.SetUsesArgsInTypeMapping();

  AddOperator(&RemotePregelCommand::remotePregelCommand);
  AddOperator(&ClosePregelWorkers::closePregelWorkers);
 }

 PregelAlgebra::~PregelAlgebra() {
    reset(false);
 };

 PregelAlgebra *PregelAlgebra::getAlgebra() {
  return algebra;
 }

 PregelAlgebra *PregelAlgebra::algebra = nullptr;

 void PregelAlgebra::setAlgebra(PregelAlgebra *algebra) {
  assert(PregelAlgebra::algebra == nullptr); // singleton`
  PregelAlgebra::algebra = algebra;
 }

 bool PregelAlgebra::amITheMaster() {
  return MessageBroker::get().numberOfServers() == 0;
 }

 void PregelAlgebra::healthReport(std::stringstream &sstream) {
  if (amITheMaster()) {
   sstream << "=== Pregel Health Report MASTER ===" << std::endl;
   sstream << std::endl;
  } else {
   sstream << "=== Pregel Health Report WORKER ===" << std::endl;
   sstream << std::endl;
  }
  MessageBroker::get().healthReport(sstream);
  PregelContext::get().healthReport(sstream);
  #ifdef GATHER_PREGEL_METRICS
  Metrics::get().report().print(sstream);
  #endif
 }

 void PregelAlgebra::healthReport(PregelStatus2Helper& ps2h) {
  PregelContext::get().healthReport(ps2h);
  #ifdef GATHER_PREGEL_METRICS
  Metrics::get().report().fill(ps2h);
  #endif
 }




 void PregelAlgebra::reset( bool informWorkers) {
  BOOST_LOG_TRIVIAL(debug) << "Reset Algebra";

   if(informWorkers){

     auto workers = PregelContext::get().getWorkers();
     std::string query("query resetPregel();");

     for (auto worker = workers(); worker != nullptr; worker = workers()) {
      BOOST_LOG_TRIVIAL(info) << "Reset Worker";
      try {
       Commander::remoteQuery(worker->connection,
                           query,
                           Commander::throwWhenFalse);
      } catch (RemoteExecutionException &e) {
       BOOST_LOG_TRIVIAL(warning) << "Reset worker failed: Error during query";
      }
     }
  }

  BOOST_LOG_TRIVIAL(info) << "Reset Broker";
  MessageBroker::get().reset(false);

  BOOST_LOG_TRIVIAL(info) << "Reset Context";
  PregelContext::get().reset();
 }
}

extern "C"
Algebra *InitializePregelAlgebra(NestedList *nlRef, QueryProcessor *qpRef) {
 pregel::PregelAlgebra *algebra = new pregel::PregelAlgebra();
 pregel::PregelAlgebra::setAlgebra(algebra);
 return algebra;
}
