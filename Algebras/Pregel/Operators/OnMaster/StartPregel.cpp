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

#include <StandardTypes.h>
#include <regex>
#include "StartPregel.h"
#include "../../Helpers/Commander.h"

namespace pregel {

 ListExpr StartPregel::typeMapping(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
   return listutils::typeError("You must provide 1 argument.");
  }
  const ListExpr rounds = nl->First(args);

  if (!CcInt::checkType(rounds)) {
   return listutils::typeError(
    "The first argument must be of type int");
  }

  return nl->SymbolAtom(CcBool::BasicType());
 }

 int
 StartPregel::valueMapping(Word *args, Word &result, int message, Word &local,
                           Supplier s) {
  result = qp->ResultStorage(s);
  CcInt *rounds = (CcInt *) args[0].addr;

  if (!rounds->IsDefined()) {
   BOOST_LOG_TRIVIAL(error) << "rounds is undefined";
   ((CcBool *) result.addr)->Set(true, false);
   return -1;
  }

  auto connections = PregelContext::get().getConnections();
  auto query =
   "query startPregelWorker(" + std::to_string(rounds->GetValue()) + ");";
  supplier<Runner> runners = [rounds, &connections, &query]() -> Runner * {
    WorkerConnection *connection;
    if ((connection = connections()) != nullptr) {
     return new Runner(connection, query);
    }
    return nullptr;
  };

  auto queryResponses = Commander::broadcast(runners, Commander::isTrue, true);

  bool allSuccessful = true;
  bool *response;
  while ((response = queryResponses()) != nullptr) {
   if (*response == false) {
    allSuccessful = false;
   }
   delete response;
  }

  ((CcBool *) result.addr)->Set(true, allSuccessful);
  return 0;
 }

 OperatorSpec StartPregel::operatorSpec(
  "int -> bool",
  "# (_)",
  "rounds (or negative for indefinite) -> success",
  "query startPregel(10);"
 );

 Operator StartPregel::startPregel(
  "startPregel",
  StartPregel::operatorSpec.getStr(),
  StartPregel::valueMapping,
  Operator::SimpleSelect,
  StartPregel::typeMapping
 );
}