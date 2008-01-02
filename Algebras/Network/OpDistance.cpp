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

1.1 Declaration of Operator Distance

January 2008 Simone Jandt

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

#include "OpLength.h"
#include "OpDistance.h"
#include "Messages.h"

/*
TypeMap Function of the operator distance

*/

ListExpr OpDistance::TypeMap(ListExpr args){
  if (nl->ListLength(args) != 2) {
    return (nl->SymbolAtom("typeerror"));
  }
  ListExpr param1 = nl->First(args);
  ListExpr param2 = nl->Second(args);

  if (!nl->IsAtom(param1) || nl->AtomType(param1) != SymbolType ||
       nl->SymbolValue(param1) != "gpoint" || !nl->IsAtom(param2) ||
       nl->AtomType(param2) != SymbolType ||
       nl->SymbolValue(param2) != "gpoint" ){
    return (nl->SymbolAtom("typeerror"));
  }
  return nl->SymbolAtom("real");
}

/*
ValueMapping function of the operator distance

*/

int OpDistance::ValueMapping (Word* args, Word& result, int message,
      Word& local, Supplier in_pSupplier){
  GPoint* pFromGPoint = (GPoint*) args[0].addr;
  GPoint* pToGPoint = (GPoint*) args[1].addr;
  result = qp->ResultStorage(in_pSupplier);
  CcReal* pResult = (CcReal*) result.addr;
  if (!(pFromGPoint->IsDefined()) || !(pToGPoint->IsDefined())) {
    cmsg.inFunError("Both gpoint must be defined!");
    return 0;
  };
  pResult-> Set(true, pFromGPoint->distance(pToGPoint));
  return 1;
}
/*
Specification of operator distance:

*/

const string OpDistance::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gpoint x gpoint -> real" "</text--->"
  "<text>distance(_,_)</text--->"
  "<text>Calculates the network distance of two gpoint.</text--->"
  "<text>let d = distance(p1,p2)</text--->"
  ") )";

