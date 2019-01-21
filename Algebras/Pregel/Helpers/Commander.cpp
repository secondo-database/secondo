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

This file contains definitions of the members of classes Commander,
 Runner and RemoteExecutionException

*/

#include "Commander.h"
#include <boost/regex.hpp>

namespace pregel {
 Runner::Runner(WorkerConnection *connection, std::string &query)
  : connection(connection), query(query), commandLog() {}

 Runner::~Runner() {}

 const resultMapper<bool> Commander::isTrue([](std::string &result) {
   return new bool(boost::regex_match(result, boost::regex(".*TRUE\\)?$")));
 });

 const resultMapper<void> Commander::throwWhenFalse(
  [](std::string &result) -> void * {

    boost::regex re(".*TRUE\\)?$");
    if (!boost::regex_match(result, re)) {
     throw RemoteExecutionException("Remote query returned FALSE");
    }
    return (void *) nullptr;
  });


 std::string Runner::run() noexcept(false) {
  if (hasRun) {
   BOOST_LOG_TRIVIAL(warning) << "But I have already run!?";
  }
  if (!connection->check(false, commandLog)) {
   if (!connection->reconnect(false, commandLog)) {
    throw RemoteExecutionException(
     "Couldn't execute remote command: Worker not alive!");
   }
  }

  std::string errorMessage;
  std::string resultStore;
  connection->simpleCommand(query, err, errorMessage, resultStore, rewriteQuery,
                            runtime, printCommands, commandLog);

  if (err != 0) {
   throw RemoteExecutionException(
    "Error executing remote command: " + errorMessage);
  }
  hasRun = true;

  return resultStore;
 }

 RemoteExecutionException::RemoteExecutionException(const std::string &message)
  : message(message) {}

 const std::string &RemoteExecutionException::getMessage() const {
  return message;
 }
}

