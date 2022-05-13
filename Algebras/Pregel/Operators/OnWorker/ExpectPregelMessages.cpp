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

This file contains definitions of the members of class ExpectPregelMessages

*/

#include <ListUtils.h>
#include "../Relation-C++/RelationAlgebra.h"
#include "../FText/FTextAlgebra.h"
#include <StandardTypes.h>
#include "ExpectPregelMessages.h"
#include "MessageBroker/MessageBroker.h"
#include "Stream.h"

namespace pregel {

 ListExpr ExpectPregelMessages::typeMapping(ListExpr args) {
  if (!nl->IsEmpty(args)) {
   return listutils::typeError("You must provide no arguments.");
  }

  return nl->SymbolAtom(CcBool::BasicType());
 }

 int
 ExpectPregelMessages::valueMapping(Word *, Word &result, int,
                                       Word &, Supplier s) {
  result = qp->ResultStorage(s);
  MessageBroker &broker = MessageBroker::get();

  broker.expectInitMessages();

  ((CcBool *) result.addr)->Set(true, true);
  return 0;
 }

 OperatorSpec ExpectPregelMessages::operatorSpec(
  "() -> bool",
  "# ()",
  "This operator notifies a worker to expect messages being sent "
  "to its message servers."
  "Returns 'true' if successful.",
  "query expectPregelMessages();",
  "This operator belongs to the Pregel API."
  "It may require knowledge of the system to effectively understand and "
  "use all the operators that are provided."
  "CAUTION: This operator is used internally by the Pregel system. "
  "Hence you must not use it in queries yourself."
  "Doing so may lead to inconsistent states of the Pregel system."
 );

 Operator ExpectPregelMessages::expectPregelMessages(
  "expectPregelMessages",
  ExpectPregelMessages::operatorSpec.getStr(),
  ExpectPregelMessages::valueMapping,
  Operator::SimpleSelect,
  ExpectPregelMessages::typeMapping
 );
}
