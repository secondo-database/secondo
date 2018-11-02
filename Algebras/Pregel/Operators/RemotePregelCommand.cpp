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

This file defines the members of class RemotePregelCommand

*/

#include "RemotePregelCommand.h"
#include "../typedefs.h"
#include "../Helpers/Commander.h"
#include "../Helpers/WorkerConfig.h"
#include <StandardTypes.h>
#include <ListUtils.h>
#include "../../FText/FTextAlgebra.h"

namespace pregel {
 ListExpr RemotePregelCommand::typeMapping(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
   return listutils::typeError("You must provide 1 argument.");
  }

  auto expression = nl->First(args);

  if (!FText::checkType(expression)) {
   return listutils::typeError("second argument (command) must be a text.");
  }

  return nl->SymbolAtom(CcBool::BasicType());
 }

 int RemotePregelCommand::valueMapping(Word *args, Word &result, int, Word &,
                                       void *s) {
  result = qp->ResultStorage(s);
  auto commandText = (FText *) args[0].addr;

  PRECONDITION(commandText->IsDefined(), "commandText must be defined");

  std::string query = commandText->GetValue();

  const supplier<pregel::WorkerConfig> &workers =
   PregelContext::get().getWorkers();
  BOOST_LOG_TRIVIAL(debug) << "query: " << query;

  std::function<std::string *(std::string &)> mapper = [](std::string &result) {
    return new std::string(result);
  };

  for (auto worker = workers(); worker != nullptr; worker = workers()) {
   try {
    auto result = Commander::remoteQuery(worker->connection, query, mapper);
    std::cout << "result of command: " << *result;
    delete result;
   } catch (RemoteExecutionException &e) {
    BOOST_LOG_TRIVIAL(error) << "Command failed: " << query.c_str();
    ((CcBool *) result.addr)->Set(true, false);
    return 0;
   }
  }

  ((CcBool *) result.addr)->Set(true, true);
  return 0;
 }

 OperatorSpec RemotePregelCommand::operatorSpec(
  "text -> bool",
  "# (_)",
  "expression -> success",
  "query remotePregelCommand('memload(Pages)');"
 );

 Operator RemotePregelCommand::remotePregelCommand(
  "remotePregelCommand",
  RemotePregelCommand::operatorSpec.getStr(),
  RemotePregelCommand::valueMapping,
  Operator::SimpleSelect,
  RemotePregelCommand::typeMapping
 );
}
