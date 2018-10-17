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

This file contains definitions of the members of class PregelStatus

*/

#include <ListUtils.h>
#include <regex>
#include <StandardTypes.h>
#include "PregelStatus.h"
#include "../../Helpers/LoggerFactory.h"
#include "../../PregelAlgebra.h"
#include "../../../FText/FTextAlgebra.h"
#include "../../Helpers/Commander.h"

namespace pregel {

 ListExpr PregelStatus::typeMapping(ListExpr args) {
  if (!nl->HasLength(args, 0)) {
   return listutils::typeError("You must provide no arguments.");
  }

  return nl->SymbolAtom(FText::BasicType());
 }

 int PregelStatus::valueMapping(Word *, Word &result, int,
                                Word &, Supplier s) {
  FORCE_LOG
  result = qp->ResultStorage(s);
  auto algebra = PregelAlgebra::getAlgebra();

  std::stringstream sstream;
  algebra->healthReport(sstream);

  auto workers = PregelContext::get().getWorkers();
  for (WorkerConfig *worker = workers();
       worker != nullptr; worker = workers()) {
   sstream << "+++Worker" << std::endl;
   std::string query("query pregelHealth()");

   std::function<std::string *(std::string &)> trimListStyle = [](
    std::string &response) -> std::string * {
     const std::regex regex("(^\\(text ')|( '\\)$)");
     auto trimmed = new std::string(std::regex_replace(response, regex, ""));
     return trimmed;
   };

   std::string *response;
   try {
    response = Commander::remoteQuery<std::string>(worker->connection, query,
                                                   trimListStyle);
   } catch (RemoteExecutionException &e) {
    BOOST_LOG_TRIVIAL(error) << "Query FAILED: " << query.c_str();
    ((FText *) result.addr)->Set(true, "Remote Health check failed");
    return 0;
   }

   sstream << response->c_str() << std::endl;
   delete response;
  }

  ((FText *) result.addr)->Set(true, sstream.str());
  return 0;
 }

 OperatorSpec PregelStatus::operatorSpec(
  "() -> bool",
  "# ()",
  "() -> TRUE",
  "query pregelHealth();"
 );

 Operator PregelStatus::pregelHealth(
  "pregelHealth",
  PregelStatus::operatorSpec.getStr(),
  PregelStatus::valueMapping,
  Operator::SimpleSelect,
  PregelStatus::typeMapping
 );
}