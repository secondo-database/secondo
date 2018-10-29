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

#include <ListUtils.h>
#include <StandardTypes.h>
#include <regex>
#include "SetPregelFunction.h"
#include "QueryProcessor.h"
#include "../../Helpers/Commander.h"
#include "../../typedefs.h"
#include "../../../FText/FTextAlgebra.h"

namespace pregel {
 ListExpr SetPregelFunction::typeMapping(ListExpr args) {
  if (!nl->HasLength(args, 2)) {
   return listutils::typeError("You must provide 2 arguments.");
  }
  const ListExpr function = nl->First(args);
  auto messageSlotAttribute = nl->Second(args);

  // Uses args in type mapping
  if (!nl->HasLength(function, 2) || !nl->HasLength(messageSlotAttribute, 2)) {
   return listutils::typeError("Internal Failure");
  }
  auto functionType = nl->First(function);
  auto functionValue = nl->Second(function);

  if (!listutils::isMap<1>(functionType)) {
   return listutils::typeError(
    "The first argument must be a function");
  }

  if (!listutils::isSymbol(nl->First(messageSlotAttribute))) {
   return listutils::typeError(
    "The second argument must be a symbol");
  }

  auto argType = nl->Second(functionType);
  auto returnType = nl->Third(functionType);
  if (!listutils::isTupleStream(argType) ||
      !listutils::isTupleStream(returnType)) {
   return listutils::typeError(
    "The function must take a tuple stream as parameter");
  }
  if (!nl->Equal(argType, returnType)) {
   return listutils::typeError(
    "The function must produce a stream of the same type of tuple");
  }

  auto tupleType = nl->Second(argType);
  PregelContext::get().setMessageType(tupleType);

  std::string attributeName = nl->SymbolValue(nl->First(messageSlotAttribute));

  int index;
  try {
   index = findAttribute(attributeName, nl->Second(tupleType));
  } catch (std::exception &e) {
   return listutils::typeError(
    "\"" + attributeName +
    "\" doesn't name an attribute in the tuple type, or isn't an int");
  }

  std::string functionText = nl->ToString(functionValue);

  return nl->ThreeElemList(
   nl->SymbolAtom(Symbols::APPEND()),
   nl->TwoElemList(nl->TextAtom(functionText), nl->IntAtom(index - 1)),
   nl->SymbolAtom(CcBool::BasicType()));
 }

 int SetPregelFunction::valueMapping(Word *args,
                                     Word &result,
                                     int,
                                     Word &,
                                     Supplier s) {
  result = qp->ResultStorage(s);
  // ignore actual function at args[0]
  // ignore attribute name at args[1]
  auto functionText = (FText *) args[2].addr;
  auto addressIndexInt = (CcInt *) args[3].addr;

  PRECONDITION(PregelContext::get().isSetUp(),
   "Please run \"query setupPregel(...) first.\"")

  std::string function = functionText->GetValue();
  int addressIndex = addressIndexInt->GetValue();

  PregelContext::get().setAddressIndex(addressIndex);
  PregelContext::get().setFunction(function);

  bool success = remoteQueryCall(function, addressIndex);

  PregelContext::get().ready();
  ((CcBool *) result.addr)->Set(true, success);
  return 0;
 }

 OperatorSpec SetPregelFunction::operatorSpec(
  "map(stream(tuple) x string -> stream(tuple)) -> bool",
  "# (_)",
  "compute function -> success",
  "query preparePregel(fun(Stream:stream(tuple), stream(tuple)):true);"
 );

 Operator SetPregelFunction::setPregelFunction(
  "setPregelFunction",
  SetPregelFunction::operatorSpec.getStr(),
  SetPregelFunction::valueMapping,
  Operator::SimpleSelect,
  SetPregelFunction::typeMapping
 );

 bool SetPregelFunction::remoteQueryCall(std::string &function, int index) {
  const supplier<pregel::WorkerConfig> workers =
   PregelContext::get().getWorkers();
  std::string query = "query setPregelFunctionWorker(" + function + ", " +
                      std::to_string(index) + ");";

  for (auto worker = workers(); worker != nullptr; worker = workers()) {
   try {
    Commander::remoteQuery(worker->connection, query,
                           Commander::throwWhenFalse);
   } catch (RemoteExecutionException &e) {
    BOOST_LOG_TRIVIAL(debug) << "failed to set function: " << e.getMessage();
    return false;
   }
  }
  return true;
 }

 int SetPregelFunction::findAttribute(const std::string &attributeName,
                                      const ListExpr tupleType)
                                      noexcept(false) {
  ListExpr attributeType;
  int index = listutils::findAttribute(tupleType, attributeName, attributeType);

  if (index == 0 ||
      !nl->Equal(attributeType, nl->SymbolAtom(CcInt::BasicType()))) {
   throw std::exception();
  }

  return index;
 }
}