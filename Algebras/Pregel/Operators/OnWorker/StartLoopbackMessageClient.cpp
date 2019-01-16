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

This file contains definitions of the members of class StartLoopbackMessageClient

*/

#include <ListUtils.h>
#include <StandardTypes.h>
#include "StartLoopbackMessageClient.h"
#include "../../MessageBroker/MessageBroker.h"

namespace pregel {
 ListExpr StartLoopbackMessageClient::typeMapping(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
   return listutils::typeError("You must provide 1 argument.");
  }
  const ListExpr slotNr = nl->First(args);

  if (!CcInt::checkType(slotNr)) {
   return listutils::typeError(
    "The first argument must be an int");
  }

  return nl->SymbolAtom(CcBool::BasicType());
 }

 int StartLoopbackMessageClient::valueMapping(Word *args, Word &result, int,
                                              Word &, Supplier s) {
  result = qp->ResultStorage(s);
  CcInt *slotNrWrapper = (CcInt *) args[0].addr;

  PRECONDITION(slotNrWrapper->IsDefined(), "slotNr is undefined")
  int slotNr = slotNrWrapper->GetIntval();
  PRECONDITION(slotNr >= 0, "slotNr must not be negative");


  bool loopbackProxyStarted = MessageBroker::get().startLoopbackProxy(slotNr);

  if (!loopbackProxyStarted) {
   BOOST_LOG_TRIVIAL(error) << "Couldn't start loopback proxy";
   ((CcBool *) result.addr)->Set(true, false);
   return 0;
  }

  ((CcBool *) result.addr)->Set(true, true);
  return 0;
 }

 OperatorSpec StartLoopbackMessageClient::operatorSpec(
  "int -> bool",
  "# (_)",
  "This operator is used analogous to 'startMessageClient(...)'."
  "It also starts a message client, although the proxy doesn't send messages "
  "to other hosts. It queues the messages directly into the input queue of the "
  "local message broker.",
  "query startLoopbackMessageClient(1);",
  "This operator belongs to the Pregel API."
  "It may require knowledge of the system to effectively understand and "
  "use all the operators that are provided."
  "CAUTION: This operator is used internally by the Pregel system. "
  "Hence you must not use it in queries yourself."
  "Doing so may lead to inconsistent states of the Pregel system."
 );

 Operator StartLoopbackMessageClient::startLoopbackMessageClient(
  "startLoopbackMessageClient",
  StartLoopbackMessageClient::operatorSpec.getStr(),
  StartLoopbackMessageClient::valueMapping,
  Operator::SimpleSelect,
  StartLoopbackMessageClient::typeMapping
 );
}