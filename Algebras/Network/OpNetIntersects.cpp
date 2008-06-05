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
//[TOC] [\tableofcontents]

1.1 Implementation of Operator interssects

The operator returns true if two given glines intersect false elsewhere.

June 2008 Simone Jandt

Defines, includes, and constants

*/
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "DBArray.h"
#include "TupleIdentifier.h"

#include "StandardTypes.h"

#include "TemporalAlgebra.h"
#include "NetworkAlgebra.h"
#include "GPoint.h"
#include "GLine.h"
#include "UGPoint.h"
#include "MGPoint.h"
#include "TemporalNetAlgebra.h"
#include "Network.h"
#include "NetworkManager.h"

#include "OpNetIntersects.h"

bool searchUnit(GLine *pGLine, size_t low, size_t high,
                const RouteInterval *pRi) {
  const RouteInterval *rI;
  if (low <= high) {
    size_t mid = (high + low) / 2;
    int imid = mid;
    if ((mid < 0) || (imid >= pGLine->NoOfComponents())) {
      return false;
    }else {
      pGLine->Get(mid, rI);
      if (rI->m_iRouteId < pRi->m_iRouteId) {
        return searchUnit(pGLine, mid+1, high, pRi);
      } else {
        if (rI->m_iRouteId > pRi->m_iRouteId){
          return searchUnit(pGLine, low, mid-1, pRi);
        } else {
          if (rI->m_dStart > pRi->m_dEnd) {
            return searchUnit(pGLine, low, mid-1, pRi);
          } else {
            if (rI->m_dEnd < pRi->m_dStart){
              return searchUnit(pGLine, mid+1, high, pRi);
            } else {
              return true;
            }
          }
        }
      }
    }
  } else {
    return false;
  }
  return false;
}


/*
Typemap function of the operator ~intersects~

*/
ListExpr OpNetIntersects::TypeMap(ListExpr in_xArgs)
{
  ListExpr arg1, arg2;
  if( nl->ListLength(in_xArgs) == 2 ){
    arg1 = nl->First(in_xArgs);
    arg2 = nl->Second(in_xArgs);
    if (nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
        nl->SymbolValue(arg1) == "gline" && nl->IsAtom(arg2) &&
        nl->AtomType(arg2) == SymbolType &&
        nl->SymbolValue(arg2) == "gline"){
        return (nl->SymbolAtom("bool"));
    }
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
Value mapping function of operator ~intersects~

*/
int OpNetIntersects::ValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  // Get (empty) return value
  CcBool* pResult = (CcBool*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pResult);
  pResult->Set(false, false);
  // Get input values
  GLine* pGLine1 = (GLine*)args[0].addr;
  if(pGLine1 == NULL || !pGLine1->IsDefined()) {
    cerr << "First gline does not exist." << endl;
    pResult->Set(false, false);
    return 0;
  }
  GLine* pGLine2 = (GLine*)args[1].addr;
  if(pGLine2 == NULL || !pGLine2->IsDefined()) {
    cerr << "Second gline does not exist." << endl;
    pResult->Set(false, false);
    return 0;
  }
  const RouteInterval *pRi1, *pRi2;
  if (pGLine1->GetNetworkId() != pGLine2->GetNetworkId()) {
    cerr << "glines belong to different networks." << endl;
    pResult->Set(true, false);
    return 0;
  }
  for (int i = 0; i < pGLine1->NoOfComponents(); i++)
  {
    pGLine1->Get(i, pRi1);
    if (pGLine2->IsSorted()){
      if (searchUnit(pGLine2, 0, pGLine2->NoOfComponents()-1, pRi1)){
        pResult->Set(true, true);
        return 0;
      };
    } else {
      for (int j = 0 ; j < pGLine2->NoOfComponents(); j ++){
        pGLine2->Get(j,pRi2);
        if (pRi1->m_iRouteId == pRi2->m_iRouteId &&
           (!(pRi1->m_dEnd < pRi2->m_dStart ||
              pRi2->m_dStart > pRi1->m_dEnd))){
          pResult->Set(true, true);
          return 0;
        }
      }
    }
  }
  pResult->Set(true, false);
  return 0;
}

/*
Specification of operator ~intersects~

*/
const string OpNetIntersects::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>gline x gline -> bool" "</text--->"
  "<text>intersects(gline, gline)</text--->"
  "<text>Returns true if the both glines intersects false elsewhere.</text--->"
  "<text>intersects(gline, gline)</text--->"
  ") )";

