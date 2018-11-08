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

This file contains definitions of the members of class SetPregelFunctionWorker

*/

#include <ListUtils.h>
#include <StandardTypes.h>
#include <../../../FText/FTextAlgebra.h>
#include "SetPregelFunctionWorker.h"
#include <boost/log/trivial.hpp>
#include "../../PregelContext.h"

namespace pregel {

 ListExpr SetPregelFunctionWorker::typeMapping(ListExpr args) {
  if (!nl->HasLength(args, 2)) {
   return listutils::typeError("You must provide 2 arguments.");
  }
  const ListExpr function = nl->First(args);
  auto addressIndex = nl->Second(args);

  // Uses args in type mapping
  if (!nl->HasLength(function, 2) || !nl->HasLength(addressIndex, 2)) {
   return listutils::typeError("Internal Failure");
  }
  auto functionType = nl->First(function);
  auto functionValue = nl->Second(function);

  if (!listutils::isMap<1>(functionType)) {
   return listutils::typeError(
    "The first argument must be a function");
  }

  if (!CcInt::checkType(nl->First(addressIndex))) {
   return listutils::typeError(
    "The second argument must be an int");
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

  auto compositeFunction = nl->TwoElemList(nl->SymbolAtom("query"),
                                           nl->TwoElemList(
                                            nl->SymbolAtom("messageDistribute"),
                                            nl->TwoElemList(
                                             functionValue,
                                             nl->OneElemList(
                                              nl->SymbolAtom("messageFeed")
                                             )
                                            )
                                           )
  );

  std::string functionText = nl->ToString(compositeFunction);

  return nl->ThreeElemList(
   nl->SymbolAtom(Symbols::APPEND()),
   nl->OneElemList(nl->TextAtom(functionText)),
   nl->SymbolAtom(CcBool::BasicType()));
 }

 int SetPregelFunctionWorker::valueMapping(Word *args, Word &result, int,
                                           Word &, Supplier s) {
  result = qp->ResultStorage(s);
  // ignnore actual function object at args[0]
  auto addressIndexInt = (CcInt *) args[1].addr;
  auto queryText = (FText *) args[2].addr;

  PRECONDITION(addressIndexInt->IsDefined(), "addressIndex must be defined");
  int addressIndex = addressIndexInt->GetIntval();
  PRECONDITION(addressIndex >= 0, "addressIndex must not be negative");

  std::string query = queryText->GetValue();
  PregelContext::get().setFunction(query);
  PregelContext::get().setAddressIndex(addressIndex);

  ((CcBool *) result.addr)->Set(true, true);
  return 0;
 }

 OperatorSpec SetPregelFunctionWorker::operatorSpec(
  "map(stream(tuple), stream(tuple)) x int -> bool",
  "# (_)",
  "function x address attribute index -> success",
  "query setPregelFunctionWorker(Function, 1);"
 );

 Operator SetPregelFunctionWorker::setPregelFunctionWorker(
  "setPregelFunctionWorker",
  SetPregelFunctionWorker::operatorSpec.getStr(),
  SetPregelFunctionWorker::valueMapping,
  Operator::SimpleSelect,
  SetPregelFunctionWorker::typeMapping
 );
}