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

1.1 Declaration of Operator Inside

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

#include "OpInside.h"
#include "Messages.h"

/*
TypeMap Function of the operator ~inside~

*/

ListExpr OpInside::TypeMap(ListExpr args){
  if (nl->ListLength(args) != 2) {
    return (nl->SymbolAtom("typeerror"));
  }
  ListExpr gpoint = nl->First(args);
  ListExpr gline = nl->Second(args);
  if (!nl->IsAtom(gpoint) || nl->AtomType(gpoint) != SymbolType ||
       nl->SymbolValue(gpoint) != "gpoint" || !nl->IsAtom(gline) ||
      nl->AtomType(gline) != SymbolType || nl->SymbolValue(gline) != "gline"){
    return (nl->SymbolAtom("typeerror"));
  }
  return nl->SymbolAtom("bool");
}

/*
ValueMapping function of the operator ~inside~

*/

int OpInside::ValueMapping (Word* args, Word& result, int message,
      Word& local, Supplier in_pSupplier){
  GPoint* pGPoint = (GPoint*) args[0].addr;
  GLine* pGLine = (GLine*) args[1].addr;
  result = qp->ResultStorage(in_pSupplier);
  CcBool* pResult = (CcBool*) result.addr;
  if (!(pGLine->IsDefined()) || !(pGPoint->IsDefined()) ||
        pGLine->NoOfComponents() < 1) {
    pResult->Set(true, false);
    return 0;
  };
  if (pGPoint->GetNetworkId() != pGLine->GetNetworkId()) {
    pResult->Set(true, false);
    return 1;
  }
  const RouteInterval *pCurrRInter;
  int i = 0;
  while (i < pGLine->NoOfComponents()) {
    pGLine->Get(i, pCurrRInter);
    if (pCurrRInter->m_iRouteId == pGPoint->GetRouteId()){
      if ((pCurrRInter->m_dStart <= pGPoint->GetPosition() &&
          pCurrRInter->m_dEnd >= pGPoint->GetPosition()) ||
         (pCurrRInter->m_dEnd <= pGPoint->GetPosition() &&
          pCurrRInter->m_dStart >= pGPoint->GetPosition())) {
        pResult->Set(true, true);
        return 0;
      }
    }
    i++;
  }
  pResult->Set(true, false);
  return 1;
}
/*
Specification of operator ~inside~:

*/

const string OpInside::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gpoint x gline -> bool" "</text--->"
  "<text> _ inside _</text--->"
  "<text>Returns true if gpoint is inside gline false elsewhere.</text--->"
  "<text>GPOINT inside GLINE</text--->"
  ") )";

