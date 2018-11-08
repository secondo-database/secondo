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

Defines PregelContext class

*/

#include "PregelContext.h"
#include <GenericTC.h>
#include "StandardTypes.h"
#include "Helpers/Logging.h"
#include "SecParser.h"


namespace pregel {
 PregelContext PregelContext::context;

 const std::string &PregelContext::getFunction() const {
  return this->function;
 }

 void PregelContext::setFunction(std::string &functionText) {
  function.swap(functionText);
  functionText = function;
 }

 std::ostream &operator<<(std::ostream &os, const PregelContext &context) {
  os << " function: " << context.function <<
     " routes: " << context.workers.size() << std::endl;
  for (auto worker : context.workers) {
   os << "route " << worker << std::endl;
  }
  return os;
 }

 int PregelContext::getAddressIndex() const {
  return addressIndex;
 }

 void PregelContext::setAddressIndex(int addressIndex) {
  PregelContext::addressIndex = addressIndex;
 }

 int PregelContext::getMessageServerPort() const {
  return messageServerPort;
 }

 void PregelContext::setMessageServerPort(int messageServerPort) {
  PregelContext::messageServerPort = messageServerPort;
 }

 std::string &PregelContext::getMessageType() {
  return this->messageType;
 }

 void PregelContext::setMessageType(ListExpr messageTypeAsList) {
  std::string messageType = nl->ToString(messageTypeAsList);
  this->messageType.swap(messageType);

  SecondoCatalog *catalog = SecondoSystem::GetCatalog();
  auto numericType = catalog->NumericType(messageTypeAsList);
  setNumericMessageType(numericType);
 }

 void PregelContext::setNumericMessageType(ListExpr numericMessageType) {
  std::string numericTypeToString = nl->ToString(numericMessageType);
  this->numericMessageType.swap(numericTypeToString);
 }

 void PregelContext::setRoundCounter(const int round) {
  this->superstep = round;
  SuperstepCounter::invalidate();
 }

 void PregelContext::increaseSuperstepCounter() {
  ++superstep;
  SuperstepCounter::invalidate();
 }

 PregelContext &PregelContext::get() {
  return context;
 }

 void PregelContext::addWorker(WorkerConfig worker) noexcept(false) {
  if (workerExists(worker.endpoint, worker.messageServerPort)) {
   throw std::exception();
  }
  workers.push_back(worker);
 }

 bool PregelContext::workerExists(RemoteEndpoint &endpoint,
                                  int messageServerPort) {
  for (auto it = workers.begin(); it != workers.end(); ++it) {
   if (endpoint == (*it).endpoint) {
    return true;
   }
   if (endpoint.host == (*it).endpoint.host &&
       messageServerPort == (*it).messageServerPort) {
    return true;
   }
  }

  return false;
 }

 supplier<WorkerConfig> PregelContext::getWorkers() {
  auto it = new std::vector<WorkerConfig>::iterator(workers.begin());
  return (supplier<WorkerConfig>) [this, it]() mutable -> WorkerConfig * {
    if (it != nullptr && *it != workers.end()) {
     WorkerConfig *entry = &(**it);
     ++(*it);
     return entry;
    }

    delete it;
    it = nullptr;
    return nullptr;
  };
 }

 supplier<WorkerConnection> PregelContext::getConnections() {
  auto routes = new supplier<WorkerConfig>(getWorkers());
  return [routes]() mutable -> WorkerConnection * {
    if (routes == nullptr) {
     return nullptr;
    }
    WorkerConfig *entry = (*routes)();
    if (entry == nullptr) {
     delete routes;
     routes = nullptr;
     return nullptr;
    }
    return entry->connection;
  };
 }

 bool superstepValid = false;
 int superstep = 0;

 int SuperstepCounter::get() {
  if (superstepValid) {
   return superstep;
  }
  superstep = PregelContext::get().getCurrentSuperstep();
  superstepValid = true;
  return superstep;
 }

 void SuperstepCounter::invalidate() {
  superstepValid = false;
 }

 int SuperstepCounter::increment() {
  PregelContext::get().increaseSuperstepCounter();
  invalidate();
  return get();
 }
}