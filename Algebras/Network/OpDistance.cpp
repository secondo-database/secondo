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
TypeMap Function of the operator ~distance~

*/

ListExpr OpDistance::DistanceTypeMap(ListExpr args){
  if (nl->ListLength(args) != 2) {
    return (nl->SymbolAtom("typeerror"));
  }
  ListExpr param1 = nl->First(args);
  ListExpr param2 = nl->Second(args);

  if ((nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&
     nl->IsAtom(param2) && nl->AtomType(param2) == SymbolType) &&
     ((nl->SymbolValue(param1) == "gpoint" &&
     nl->SymbolValue(param2) == "gpoint") ||
     (nl->SymbolValue(param1) == "gline" &&
     nl->SymbolValue(param2) == "gline"))) {
    return nl->SymbolAtom("real");
  }
  return nl->SymbolAtom("typeerror");
}

/*
ValueMapping function of the operator ~distance~

*/

int OpDistance::distance_gpgp (Word* args, Word& result, int message,
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
};

int OpDistance::distance_glgl (Word* args, Word& result, int message,
      Word& local, Supplier in_pSupplier){
  GLine* pGLine1 = (GLine*) args[0].addr;
  GLine* pGLine2 = (GLine*) args[1].addr;
  result = qp->ResultStorage(in_pSupplier);
  CcReal* pResult = (CcReal*) result.addr;
  if (!(pGLine1->IsDefined()) || !(pGLine2->IsDefined())) {
    cmsg.inFunError("Both gpoint must be defined!");
    return 0;
  };
  pResult-> Set(true, pGLine1->distance(pGLine2));
  return 1;
};

ValueMapping OpDistance::distancemap[] = {
  distance_gpgp,
  distance_glgl
};

int OpDistance::selectDistance (ListExpr args){
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if ( nl->SymbolValue(arg1) == "gpoint" &&
       nl->SymbolValue(arg2) == "gpoint")
    return 0;
  if ( nl->SymbolValue(arg1) == "gline" &&
       nl->SymbolValue(arg2) == "gline")
    return 1;
  return -1; // This point should never be reached
};
/*
Specification of operator distance:

*/

const string OpDistance::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>A x A -> real with A = gpoint or gline" "</text--->"
  "<text>distance(_,_)</text--->"
  "<text>Calculates the network distance of two gpoints resp. glines.</text--->"
  "<text>query distance(p1,p2)</text--->"
  ") )";

