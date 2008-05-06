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

1.1 Implementation of Operator Passes

This operator checks whether a moving point passes a fixed point or not.

Mai-Oktober 2007 Martin Scheppokat

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

#include "OpPasses.h"

bool searchUnit(GLine *pGLine, size_t low, size_t high,
                int unitRouteId, double dMGPStart,
                double dMGPEnd) {
  const RouteInterval *rI;
  if (low <= high) {
    size_t mid = (high + low) / 2;
    int imid = mid;
    if ((mid < 0) || (imid >= pGLine->NoOfComponents())) {
      return false;
    }else {
      pGLine->Get(mid, rI);
      if (rI->m_iRouteId < unitRouteId) {
        return searchUnit(pGLine, mid+1, high, unitRouteId, dMGPStart, dMGPEnd);
      } else {
        if (rI->m_iRouteId > unitRouteId){
          return searchUnit(pGLine, low, mid-1, unitRouteId, dMGPStart,
                            dMGPEnd);
        } else {
          if (rI->m_dStart > dMGPEnd) {
            return searchUnit(pGLine, low, mid-1, unitRouteId, dMGPStart,
                              dMGPEnd);
          } else {
            if (rI->m_dEnd < dMGPStart){
              return searchUnit(pGLine, mid+1, high, unitRouteId, dMGPStart,
                                dMGPEnd);
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
Typemap function of the operator

*/
ListExpr
OpPasses:: PassesMap( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if ( nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
         nl->SymbolValue(arg1) == "mgpoint" ) {
      if (nl->IsAtom(arg2) && nl->AtomType(arg2) == SymbolType &&
         (nl->SymbolValue(arg2) == "gpoint" ||
          nl->SymbolValue(arg2) == "gline")){
        return (nl->SymbolAtom("bool"));
      }
    }
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
Value mapping function of operator ~passes~

*/
int OpPasses::passes_mgpgp(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  // Get (empty) return value
  CcBool* pPasses = (CcBool*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pPasses);
  // Get input values
  MGPoint* pMGPoint = (MGPoint*)args[0].addr;
  if(pMGPoint == NULL || pMGPoint->GetNoComponents() < 1 ||
      !pMGPoint->IsDefined()) {
    cout << "MGPoint not Defined" << endl;
    pPasses->Set(false, false);
    return 0;
  }
  GPoint* pGPoint = (GPoint*)args[1].addr;
  if(pGPoint == NULL || !pGPoint->IsDefined()) {
    cout << "GPoint not Defined" << endl;
    pPasses->Set(false, false);
    return 0;
  }
  const UGPoint *pCurrentUnit;
  pMGPoint->Get(0, pCurrentUnit);
  if (pGPoint->GetNetworkId() != pCurrentUnit->p0.GetNetworkId()) {
    pPasses->Set(true, false);
    return 0;
  }
  for (int i = 0; i < pMGPoint->GetNoComponents(); i++) {
    pMGPoint->Get(i, pCurrentUnit);
    if (pCurrentUnit->p0.GetRouteId() == pGPoint->GetRouteId()){
    // check if p is between p0 and p1
      if((pCurrentUnit->p0.GetPosition() < pGPoint->GetPosition() &&
        pGPoint->GetPosition() < pCurrentUnit->p1.GetPosition()) ||
        (pCurrentUnit->p1.GetPosition() < pGPoint->GetPosition() &&
        pGPoint->GetPosition() < pCurrentUnit->p0.GetPosition())) {
        pPasses->Set(true, true);
        return 0;
      }

      // If the edge of the interval is included we need to check the exakt
      // Position too.
      if((pCurrentUnit->timeInterval.lc &&
          AlmostEqual(pCurrentUnit->p0.GetPosition(), pGPoint->GetPosition()))||
          (pCurrentUnit->timeInterval.rc &&
          AlmostEqual(pCurrentUnit->p1.GetPosition(),pGPoint->GetPosition()))) {
        pPasses->Set(true, true);
        return 0;
      }
    }
  }
  pPasses->Set(true, false);
  return 0;
};

int OpPasses::passes_mgpgl(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  // Get (empty) return value
  CcBool* pPasses = (CcBool*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pPasses);
  double dMGPStart, dMGPEnd;
  // Get input values
  MGPoint* pMGPoint = (MGPoint*)args[0].addr;
  if(pMGPoint == NULL || pMGPoint->GetNoComponents() < 1 ||
      !pMGPoint->IsDefined()) {
    cerr << "MGPoint does not exist." << endl;
    pPasses->Set(false, false);
    return 0;
  }
  GLine* pGLine = (GLine*)args[1].addr;
  if(pGLine == NULL || !pGLine->IsDefined()) {
    cerr << "GLine does not exist." << endl;
    pPasses->Set(false, false);
    return 0;
  }
  const UGPoint *pCurrentUnit;
  pMGPoint->Get(0, pCurrentUnit);
  if (pGLine->GetNetworkId() != pCurrentUnit->p0.GetNetworkId()) {
    pPasses->Set(true, false);
    return 0;
  }
  const RouteInterval *pCurrRInter;
  for (int i = 0; i < pMGPoint->GetNoComponents(); i++)
  {
    pMGPoint->Get(i, pCurrentUnit);
    if (pCurrentUnit->p0.GetPosition() <= pCurrentUnit->p1.GetPosition()) {
      dMGPStart = pCurrentUnit->p0.GetPosition();
      dMGPEnd = pCurrentUnit->p1.GetPosition();
    } else {
      dMGPStart = pCurrentUnit->p1.GetPosition();
      dMGPEnd = pCurrentUnit->p0.GetPosition();
    }
    if (pGLine->IsSorted()){
      if (searchUnit(pGLine, 0, pGLine->NoOfComponents()-1,
                     pCurrentUnit->p0.GetRouteId(), dMGPStart, dMGPEnd)){
        pPasses->Set(true, true);
        return 0;
      };
    } else {
      for (int j = 0 ; j < pGLine->NoOfComponents(); j ++){
        pGLine->Get(j,pCurrRInter);
        if (pCurrRInter->m_iRouteId == pCurrentUnit->p0.GetRouteId() &&
           (!(pCurrRInter->m_dEnd < dMGPStart ||
              pCurrRInter->m_dStart > dMGPEnd))){
          pPasses->Set(true, true);
          return 0;
        }
      }
    }
  }
  pPasses->Set(true, false);
  return 0;
};

/*
Value mapping function of operator ~passes~

*/
ValueMapping OpPasses::passesmap[] = {
  passes_mgpgp,
  passes_mgpgl
};

int
OpPasses::SelectPasses( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( nl->SymbolValue(arg1) == "mgpoint" &&
       nl->SymbolValue(arg2) == "gpoint")
    return 0;
  if ( nl->SymbolValue(arg1) == "mgpoint" &&
       nl->SymbolValue(arg2) == "gline")
    return 1;
  return -1; // This point should never be reached
}







/*
Specification of the operator

*/
const string OpPasses::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint x A -> bool for A gpoint or gline " "</text--->"
  "<text>_ passes _</text--->"
  "<text>Checks whether a moving point passes a gpoint or a gline.</text--->"
  "<text>X_MGPOINT passes X_GPOINT </text--->"
  ") )";
