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

This header file contains the declaration of the classes Commander, Runner and RemoteExecutionException.

2 Defines and includes

*/

#ifndef SECONDO_COMMANDER_H
#define SECONDO_COMMANDER_H

#include "../typedefs.h"
#include <utility>
#include "../../Distributed2/ConnectionInfo.h"
#include "LoggerFactory.h"
#include "../PregelContext.h"

namespace pregel {
 template<typename result_type>
 using resultMapper = std::function<result_type *(std::string &)>;

 class Runner {
 private:
  WorkerConnection *connection;
  std::string query;
  bool hasRun = false;
  int err = 0;
  double runtime = 0.0;
  bool rewriteQuery = true;
  bool printCommands = true;

  distributed2::CommandLog commandLog;

 public:
  Runner(WorkerConnection *connection, std::string &query);

  virtual ~Runner();

  std::string run() noexcept(false);

  bool inline successful() {
   return err == 0;
  }
 };

 class RemoteExecutionException : public std::exception {
 private:
  std::string message;

 public:
  RemoteExecutionException(const std::string &message);

  const std::string &getMessage() const;
 };

 class Commander {
 public:
  const static resultMapper<bool> isTrue;
  const static resultMapper<void> throwWhenFalse;

  template<typename result_type>
  static result_type *remoteQuery(WorkerConnection *connection,
                                  std::string &query,
                                  const resultMapper<result_type> &mapper)
  noexcept(false) {
   Runner runner(connection, query);
   std::string response = runner.run();
   return mapper(response);
  }

  template<typename result_type>
  static supplier<result_type> broadcast(supplier<Runner> runners,
                                         resultMapper<result_type> mapper,
                                         bool parallel = false) {
   if (parallel) {
    return multiThreaded<result_type>(runners, mapper);
   } else {
    return singleThreaded<result_type>(runners, mapper);
   }
  }

 private:
  template<typename result_type>
  static supplier<result_type> singleThreaded(
   supplier<Runner> &runners,
   resultMapper<result_type> &mapper) {
   auto result = [&runners, &mapper]() -> result_type * {
     try {
      Runner *runner;
      if ((runner = runners()) == nullptr) {
       return nullptr;
      }
      std::string response = runner->run();
      delete runner;
      return mapper(response);
     } catch (RemoteExecutionException &e) {
      return nullptr;
     }
   };

   return (supplier<result_type>) result;
  }

  template<typename result_type>
  static supplier<result_type> multiThreaded(
   supplier<Runner> &runners,
   resultMapper<result_type> &mapper) {
   boost::mutex resultLock;
   std::vector<boost::thread *> threads;
   std::vector<result_type *> *resultStore = new std::vector<result_type *>();

   auto runAsynchronous = [&mapper, &resultLock, resultStore](
    Runner *runner,
    bool &asynchExceptions) {
     result_type *result;
     try {
      std::string response = runner->run();
      delete runner;
      result = mapper(response);
     } catch (RemoteExecutionException &e) {
      asynchExceptions = true;
      delete runner;
      return;
     }
     resultLock.lock();
     resultStore->push_back(result);
     resultLock.unlock();
   };

   Runner *runner;
   bool asynchExceptions = false;
   while ((runner = runners()) != nullptr) {
    auto runnable = new boost::thread(boost::bind<void>(runAsynchronous,
                                                        runner,
                                                        boost::ref(
                                                         asynchExceptions)));
    threads.push_back(runnable);
   }

   for (boost::thread *thread : threads) {
    thread->join();
    delete thread;
   }

   auto returnSupplier =
    [resultStore, &asynchExceptions]() mutable -> result_type * {
      if (asynchExceptions) {
       for (auto result : *resultStore) {
        delete result;
       }
       resultStore->clear();
       delete resultStore;
       resultStore = nullptr;
       throw RemoteExecutionException("There were failing remote queries.");
      }
      if (resultStore == nullptr) {
       return nullptr;
      }
      if (resultStore->empty()) {
       delete resultStore;
       return nullptr;
      }
      auto returnValue = resultStore->back();
      resultStore->pop_back();
      return returnValue;
    };
   return (supplier<result_type>) returnSupplier;

  }
 };
}


#endif //SECONDO_COMMANDER_H
