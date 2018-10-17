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

#include "SetupPregel.h"
#include <ListUtils.h>
#include <vector>
#include <regex>
#include <QueryProcessor.h>
#include "../../../Distributed2/DArray.h"
#include "../../../Distributed2/Distributed2Algebra.h"
#include "../../../Distributed2/ConnectionInfo.h"
#include "../../Helpers/LoggerFactory.h"
#include "../../Helpers/Commander.h"
#include "../../MessageBroker/MessageBroker.h"
#include "../../PregelAlgebra.h"

#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/thread.hpp>

namespace pregel {
 ListExpr SetupPregel::typeMapping(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
   return listutils::typeError("You must provide 1 argument.");
  }
  const ListExpr workerRelation = nl->First(args);

  if (!Relation::checkType(workerRelation)) {
   return listutils::typeError(
    "The first argument must be of type relation");
  }

  ListExpr tupleType = nl->Second(nl->Second(workerRelation));

  ListExpr hostType;
  int hostPosition = listutils::findAttribute(tupleType, "Host", hostType);
  if (!hostPosition) {
   BOOST_LOG_TRIVIAL(error) << "No \"Host\" attribute in relation";
  }
  if (!CcString::checkType(hostType) /*&& !FText::checkType(hostType)*/) {
   BOOST_LOG_TRIVIAL(error) << "\"Host\" attribute not of type string";
  }

  ListExpr configType;
  int configPosition = listutils::findAttribute(tupleType, "Config",
                                                configType);
  if (!configPosition) {
   BOOST_LOG_TRIVIAL(error) << "No \"Config\" attribute in relation";
  }
  if (!CcString::checkType(configType) /*&& !FText::checkType(configType)*/) {
   BOOST_LOG_TRIVIAL(error) << "\"Config\" attribute not of type string";
  }

  ListExpr portType;
  int portPosition = listutils::findAttribute(tupleType, "Port", portType);
  if (!portPosition) {
   BOOST_LOG_TRIVIAL(error) << "No \"Port\" attribute in relation";
  }
  if (!CcInt::checkType(portType)) {
   BOOST_LOG_TRIVIAL(error) << "\"Port\" attribute not of type int";
  }

  ListExpr messageServerPortType;
  int messageServerPortPosition = listutils::findAttribute(tupleType,
                                                           "MessageServerPort",
                                                           messageServerPortType
                                                           );
  if (!messageServerPortPosition) {
   BOOST_LOG_TRIVIAL(error) << "No \"MessageServerPort\" attribute in relation";
  }
  if (!CcInt::checkType(messageServerPortType)) {
   BOOST_LOG_TRIVIAL(error)
    << "\"MessageServerPort\" attribute not of type int";
  }

  return nl->ThreeElemList(
   nl->SymbolAtom(Symbols::APPEND()),
   nl->FourElemList(
    nl->IntAtom(hostPosition - 1),
    nl->IntAtom(portPosition - 1),
    nl->IntAtom(messageServerPortPosition - 1),
    nl->IntAtom(configPosition - 1)
   ),
   nl->SymbolAtom(CcBool::BasicType())
  );
 }

 int SetupPregel::valueMapping(Word *args, Word &resultStorage,
                               int, Word &,
                               Supplier s) {
  FORCE_LOG
  resultStorage = qp->ResultStorage(s);
  PregelAlgebra::getAlgebra()->reset();
  auto relation = (Relation *) args[0].addr;
  int hostIndex = ((CcInt *) args[1].addr)->GetIntval();
  int portIndex = ((CcInt *) args[2].addr)->GetIntval();
  int messageServerPortIndex = ((CcInt *) args[3].addr)->GetIntval();
  int configIndex = ((CcInt *) args[4].addr)->GetIntval();


  try {
   int slotNumber = 0;
   std::string dbName = SecondoSystem::GetInstance()->GetDatabaseName();

   RelationIterator it(*relation, relation->GetTupleType());
   Tuple *tuple;
   while ((tuple = it.GetNextTuple()) != nullptr) {
    auto worker = workerFromTuple(tuple, hostIndex, portIndex,
                                  messageServerPortIndex, configIndex, dbName,
                                  slotNumber);

    PregelContext::get().addWorker(worker);

    ++slotNumber;
   }

   startMessageServersAndClients();

  } catch (RemoteExecutionException &e) {
   BOOST_LOG_TRIVIAL(error)
    << "Failed to set up Pregel due to error during remote query. Will reset.";
   ((CcBool *) resultStorage.addr)->Set(true, false);
   PregelAlgebra::getAlgebra()->reset();
   return -1;
  } catch (std::exception &e) {
   BOOST_LOG_TRIVIAL(error) << "Failed to start Clients. Will reset.";
   ((CcBool *) resultStorage.addr)->Set(true, false);
   PregelAlgebra::getAlgebra()->reset();
   return -1;
  }

  ((CcBool *) resultStorage.addr)->Set(true, true);
  return 0;
 }

 OperatorSpec SetupPregel::operatorSpec(
  "rel() -> bool",
  "#(_)",
  "worker relation with message server ports -> success",
  "query setupPregel(Workers);"
 );

 Operator SetupPregel::setupPregel(
  "setupPregel",
  SetupPregel::operatorSpec.getStr(),
  SetupPregel::valueMapping,
  Operator::SimpleSelect,
  SetupPregel::typeMapping
 );

 void SetupPregel::startMessageServersAndClients() noexcept(false) {
  MessageBroker &broker = MessageBroker::get();

  {
   auto workers = PregelContext::get().getWorkers();
   for (WorkerConfig *worker = workers();
        worker != nullptr; worker = workers()) {
    std::string query("query startMessageServer(" +
                      std::to_string(worker->messageServerPort) + ")");
    Commander::remoteQuery(worker->connection,
                           query,
                           Commander::throwWhenFalse);
   }
  }

  {
   auto workers = PregelContext::get().getWorkers();
   for (auto worker = workers(); worker != nullptr; worker = workers()) {
    std::string query("query startLoopbackMessageClient(" +
                      std::to_string(worker->slot) + ")");
    Commander::remoteQuery(worker->connection, query,
                           Commander::throwWhenFalse);
   }
  }

  {
   auto workers = PregelContext::get().getWorkers();
   for (auto worker = workers(); worker != nullptr; worker = workers()) {

    auto remoteWorkers = PregelContext::get().getWorkers();
    for (auto remote = remoteWorkers();
         remote != nullptr;
         remote = remoteWorkers()) {

     if (worker->slot == remote->slot) {
      continue;
     }

     const int &slot = remote->slot;
     const std::string &host = remote->endpoint.host;
     const int &port = remote->messageServerPort;

     std::string query("query startMessageClient(" +
                       std::to_string(slot) + ", \"" +
                       host.c_str() + "\", " +
                       std::to_string(port) + ")");
     Commander::remoteQuery(worker->connection,
                            query,
                            Commander::throwWhenFalse);
    }
   }
  }

  {
   auto workers = PregelContext::get().getWorkers();
   for (auto remote = workers(); remote != nullptr; remote = workers()) {

    const int &slot = remote->slot;
    const std::string &host = remote->endpoint.host;
    const int &port = remote->messageServerPort;

    bool successful = broker.startClient(slot, RemoteEndpoint(host, port));
    if (!successful) {
     BOOST_LOG_TRIVIAL(error) << "FAILED starting client";
     throw std::exception();
    }
   }
  }
 }

 WorkerConfig SetupPregel::workerFromTuple(Tuple *tuple,
                                           int hostIndex,
                                           int portIndex,
                                           int messageServerPortIndex,
                                           int configIndex,
                                           std::string &dbName,
                                           int slotNumber) noexcept(false) {
  auto host = ((CcString *) tuple->GetAttribute(hostIndex))->GetValue();
  auto port = ((CcInt *) tuple->GetAttribute(portIndex))->GetIntval();
  auto messageServerPort = ((CcInt *) tuple->GetAttribute(
   messageServerPortIndex))->GetIntval();
  auto configFilePath = ((CcString *) tuple->GetAttribute(
   configIndex))->GetValue();

  auto connection = WorkerConnection::createConnection(host, port,
                                                       configFilePath);

  if (connection == nullptr) {
   throw std::exception();
  }

  connection->switchDatabase(dbName, true, true);
//  BOOST_LOG_TRIVIAL(debug) << "Switched to database \"" << dbName.c_str()
//                           << "\"";

  return WorkerConfig(slotNumber, RemoteEndpoint(host, port),
                      messageServerPort, configFilePath,
                      connection);
 }
}