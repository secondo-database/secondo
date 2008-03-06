/*
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]

1.1 Declaration of Operator ~eaual~

March 2008 Simone Jandt

Defines, includes, and constants

*/
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "DBArray.h"
#include "TupleIdentifier.h"

#include "SpatialAlgebra.h"
#include "StandardTypes.h"
#include "GLine.h"
#include "GPoint.h"
#include "Network.h"
#include "NetworkManager.h"

#include "OpNetEqual.h"
#include "Messages.h"

/*
TypeMap Function of the operator ~equal~

*/

ListExpr OpNetEqual::TypeMap(ListExpr args){
  if (nl->ListLength(args) != 2) {
    return (nl->SymbolAtom("typeerror"));
  }
  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args);
  if (!nl->IsAtom(first) || nl->AtomType(first) != SymbolType ||
       nl->SymbolValue(first) != "gpoint"){
    return (nl->SymbolAtom("typeerror"));
  }
  if (!nl->IsAtom(second) || nl->AtomType(second) != SymbolType ||
       nl->SymbolValue(second) != "gpoint"){
    return (nl->SymbolAtom("typeerror"));
  }
  return nl->SymbolAtom("bool");
}

/*
ValueMapping function of the operator ~equal~

*/

int OpNetEqual::ValueMapping (Word* args, Word& result, int message,
      Word& local, Supplier in_pSupplier){
  GPoint* p1 = (GPoint*) args[0].addr;
  GPoint* p2 = (GPoint*) args[1].addr;
  result = qp->ResultStorage(in_pSupplier);
  CcBool* pResult = (CcBool*) result.addr;
  if (!(p1->IsDefined()) || !p2->IsDefined()) {
    cmsg.inFunError("Both gpoints must be defined!");
    pResult->Set(false, false);
    return 0;
  };
  pResult-> Set(true, *p1 == *p2);
  return 1;
}
/*
Specification of operator ~equal~:

*/

const string OpNetEqual::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gpoint x gpoint -> bool" "</text--->"
  "<text> _ = _</text--->"
  "<text>Returns if two gpoints are equal.</text--->"
  "<text>query gpoint1 = gpoint2</text--->"
  ") )";

