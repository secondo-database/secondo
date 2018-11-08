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

This file contains definitions of the members of class StartMessageClient

*/

#include <ListUtils.h>
#include <StandardTypes.h>
#include "StartMessageClient.h"
#include "../../Helpers/RemoteEndpoint.h"
#include "../../MessageBroker/MessageBroker.h"

namespace pregel {
 ListExpr StartMessageClient::typeMapping(ListExpr args) {
  if (!nl->HasLength(args, 3)) {
   return listutils::typeError("You must provide 3 arguments.");
  }
  const ListExpr slot = nl->First(args);
  const ListExpr host = nl->Second(args);
  const ListExpr port = nl->Third(args);

  if (!CcInt::checkType(slot)) {
   return listutils::typeError(
    "The first argument (slot) must be of type int");
  }
  if (!CcString::checkType(host)) {
   return listutils::typeError(
    "The second argument (host) must be of type string");
  }
  if (!CcInt::checkType(port)) {
   return listutils::typeError(
    "The second argument (port) must be of type int");
  }

  return nl->SymbolAtom(CcBool::BasicType());
 }

 int StartMessageClient::valueMapping(Word *args, Word &result, int message,
                                      Word &local, Supplier s) {
  result = qp->ResultStorage(s);
  CcInt *slotNoObj = (CcInt *) args[0].addr;
  CcString *hostObj = (CcString *) args[1].addr;
  CcInt *portWrapper = (CcInt *) args[2].addr;

  PRECONDITION(slotNoObj->IsDefined(), "slot is undefined");
  PRECONDITION(hostObj->IsDefined(), "host is undefined");
  PRECONDITION(portWrapper->IsDefined(), "port is undefined");
  const int slot = slotNoObj->GetValue();
  const std::string &hostName = hostObj->GetValue();
  const int port = portWrapper->GetValue();
  PRECONDITION(port > 0, "port must be grater than 0")
  PRECONDITION(slot >= 0, "slot must be grater or equal to 0")
  PRECONDITION(!hostName.empty(), "host must not be empty")

  const RemoteEndpoint host(hostName, port);
  MessageBroker &broker = MessageBroker::get();
  bool clientStarted = broker.startClient(slot, host);

  ((CcBool *) result.addr)->Set(true, clientStarted);
  return 0;
 }

 OperatorSpec StartMessageClient::operatorSpec(
  "int x string x int -> bool",
  "# (_,_,_)",
  "worker no x host x port no -> success",
  "query startMessageClient(2, \"localhost\", 8001);"
 );

 Operator StartMessageClient::startMessageClient(
  "startMessageClient",
  StartMessageClient::operatorSpec.getStr(),
  StartMessageClient::valueMapping,
  Operator::SimpleSelect,
  StartMessageClient::typeMapping
 );
}