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

1.1 Implementation of Operator at

This operator reduces a moving gpoint to the times he is at a given gpoint.

March 2008 Simone Jandt

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

#include "OpTempNetAt.h"

/*
Typemap function of the operator

*/
ListExpr OpTempNetAt::AtMap(ListExpr in_xArgs)
{
  ListExpr arg1, arg2;
  if ( nl->ListLength(  in_xArgs ) == 2 )
  {
    arg1 = nl->First( in_xArgs );
    arg2 = nl->Second( in_xArgs );

    if ( nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
         nl->SymbolValue(arg1) == "mgpoint" ) {
      if (nl->IsAtom(arg2) && nl->AtomType(arg2) == SymbolType &&
         (nl->SymbolValue(arg2) == "gpoint" ||
          nl->SymbolValue(arg2) == "gline")){
        return (nl->SymbolAtom("mgpoint"));
      }
    }
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
Value mapping function of operator ~at~

*/
int OpTempNetAt::at_mgpgp(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  // Get (empty) return value
  MGPoint* pResult = (MGPoint*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pResult);
  pResult->Clear();
  // Get input values
  MGPoint* pMGPoint = (MGPoint*)args[0].addr;
  if(pMGPoint == NULL || pMGPoint->GetNoComponents() < 1 ||
      !pMGPoint->IsDefined()) {
    cout << "MGPoint not Defined" << endl;
    pResult->SetDefined(false);
    return 0;
  }
  GPoint* pGPoint = (GPoint*)args[1].addr;
  if(pGPoint == NULL || !pGPoint->IsDefined()) {
    cout << "GPoint not Defined" << endl;
    pResult->SetDefined(false);
    return 0;
  }
  Instant tPos;
  double factor;
  const UGPoint *pCurrentUnit;
  pMGPoint->Get(0, pCurrentUnit);
  if (pGPoint->GetNetworkId() != pCurrentUnit->p0.GetNetworkId()) {
    pResult->SetDefined(false);
    return 0;
  }
  pResult->StartBulkLoad();
  for (int i = 0; i < pMGPoint->GetNoComponents(); i++)
  {
    pMGPoint->Get(i, pCurrentUnit);
    if (pCurrentUnit->p0.GetRouteId() == pGPoint->GetRouteId() &&
        (pCurrentUnit->p0.GetSide() == pGPoint->GetSide() ||
        pCurrentUnit->p0.GetSide() ==2 || pGPoint->GetSide()== 2)){
    // p is between p0 and p1
      if((pCurrentUnit->p0.GetPosition() < pGPoint->GetPosition() &&
        pGPoint->GetPosition() < pCurrentUnit->p1.GetPosition()) ||
        (pCurrentUnit->p1.GetPosition() < pGPoint->GetPosition() &&
        pGPoint->GetPosition() < pCurrentUnit->p0.GetPosition()))
      {
        factor = fabs(pCurrentUnit->p1.GetPosition() -
               pCurrentUnit->p0.GetPosition()) /
               fabs(pGPoint->GetPosition() - pCurrentUnit->p0.GetPosition());
        Instant tPos = (pCurrentUnit->timeInterval.end -
                pCurrentUnit->timeInterval.start) * factor +
                pCurrentUnit->timeInterval.start;
        pResult->Add(UGPoint(Interval<Instant>(tPos, tPos, true, true),
                     pGPoint->GetNetworkId(), pGPoint->GetRouteId(),
                     pCurrentUnit->p0.GetSide(), pGPoint->GetPosition(),
                     pGPoint->GetPosition()));
      } else {
        if (pGPoint->GetPosition() == pCurrentUnit->p0.GetPosition() &&
           pCurrentUnit->timeInterval.lc){
          pResult->Add(UGPoint(Interval<Instant>(
                            pCurrentUnit->timeInterval.start,
                            pCurrentUnit->timeInterval.start,
                            true, true),
                     pGPoint->GetNetworkId(), pGPoint->GetRouteId(),
                     pCurrentUnit->p0.GetSide(), pGPoint->GetPosition(),
                     pGPoint->GetPosition()));
        } else {
          if (pGPoint->GetPosition() == pCurrentUnit->p1.GetPosition() &&
              pCurrentUnit->timeInterval.rc){
            pResult->Add(UGPoint(Interval<Instant>(
                         pCurrentUnit->timeInterval.end,
                         pCurrentUnit->timeInterval.end,
                         true, true),
                     pGPoint->GetNetworkId(), pGPoint->GetRouteId(),
                     pCurrentUnit->p0.GetSide(), pGPoint->GetPosition(),
                     pGPoint->GetPosition()));
          }
        }
      }
    }
  }
  pResult->EndBulkLoad();
  pResult->SetDefined(true);
  return 0;
};



int OpTempNetAt::at_mgpgl(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  // Get (empty) return value
  MGPoint* pResult = (MGPoint*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pResult);
  pResult->Clear();
  // Get input values
  MGPoint* pMGPoint = (MGPoint*)args[0].addr;
  if(pMGPoint == NULL || pMGPoint->GetNoComponents() < 1 ||
      !pMGPoint->IsDefined()) {
    cerr << "MGPoint does not exist." << endl;
    pResult->SetDefined(false);
    return 0;
  }
  GLine* pGLine = (GLine*)args[1].addr;
  if(pGLine == NULL || !pGLine->IsDefined()) {
    cerr << "GLine does not exist." << endl;
    pResult->SetDefined(false);
    return 0;
  }
  const UGPoint *pCurrentUnit;
  pMGPoint->Get(0, pCurrentUnit);
  if (pGLine->GetNetworkId() != pCurrentUnit->p0.GetNetworkId()) {
    pResult->SetDefined(false);
    return 0;
  }
  int iNetworkId = pGLine->GetNetworkId();
  double mgStart, mgEnd, lStart, lEnd, factor, interStart, interEnd;
  const RouteInterval *pCurrRInter;
  Instant tInterStart, tInterEnd;
  bool bInterStart, bInterEnd, swapped;
  int iRouteMgp;
  pResult->StartBulkLoad();
  for (int i = 0; i < pMGPoint->GetNoComponents(); i++)
  {
    pMGPoint->Get(i, pCurrentUnit);
    int j = 0;
    iRouteMgp = pCurrentUnit->p0.GetRouteId();
    swapped = false;
    while (j < pGLine->NoOfComponents()) {
      pGLine->Get(j,pCurrRInter);
      if (iRouteMgp == pCurrRInter->m_iRouteId){
        mgStart = pCurrentUnit->p0.GetPosition();
        mgEnd = pCurrentUnit->p1.GetPosition();
        if (mgEnd < mgStart) {
          mgStart = pCurrentUnit->p1.GetPosition();
          mgEnd = pCurrentUnit->p0.GetPosition();
          swapped = true;
        }
        lStart = pCurrRInter->m_dStart;
        lEnd = pCurrRInter->m_dEnd;
        if (lStart > lEnd) {
          lStart = pCurrRInter->m_dEnd;
          lEnd = pCurrRInter->m_dStart;
        }
        if (!(mgEnd < lStart || mgStart > lEnd)){
          //intersection exists compute intersecting part and timevalues for
          //resulting unit
          if (!swapped) {
            if (lStart <= mgStart) {
              interStart = pCurrentUnit->p0.GetPosition();
              tInterStart = pCurrentUnit->timeInterval.start;
              bInterStart = pCurrentUnit->timeInterval.lc;
            } else {
              interStart = lStart;
              bInterStart = true;
              factor =  fabs(lStart - pCurrentUnit->p0.GetPosition())/
                        fabs(pCurrentUnit->p1.GetPosition() -
                        pCurrentUnit->p0.GetPosition());
              tInterStart = (pCurrentUnit->timeInterval.end -
                            pCurrentUnit->timeInterval.start) * factor +
                            pCurrentUnit->timeInterval.start;
            }
            if (lEnd >= mgEnd) {
              interEnd = pCurrentUnit->p1.GetPosition();
              tInterEnd = pCurrentUnit->timeInterval.end;
              bInterEnd = pCurrentUnit->timeInterval.rc;
            } else {
              interEnd = lEnd;
              bInterEnd = true;
              factor = fabs(lEnd - pCurrentUnit->p0.GetPosition())/
                        fabs(pCurrentUnit->p1.GetPosition() -
                        pCurrentUnit->p0.GetPosition());
              tInterEnd = (pCurrentUnit->timeInterval.end -
                            pCurrentUnit->timeInterval.start) * factor +
                            pCurrentUnit->timeInterval.start;
            }
            if (interStart == interEnd && (!bInterStart || !bInterEnd)) {
              continue;
            } else {
              pResult->Add(UGPoint(Interval<Instant> (tInterStart, tInterEnd,
                                bInterStart, bInterEnd), iNetworkId, iRouteMgp,
                                pCurrentUnit->p0.GetSide(), interStart,
                                interEnd));
            }
          } else {
            mgStart = pCurrentUnit->p0.GetPosition();
            mgEnd = pCurrentUnit->p1.GetPosition();
            if (lEnd >= mgStart) {
              interStart = pCurrentUnit->p0.GetPosition();
              tInterStart = pCurrentUnit->timeInterval.start;
              bInterStart = pCurrentUnit->timeInterval.lc;
            } else {
              interStart = lEnd;
              bInterStart = true;
              factor =fabs(lEnd - pCurrentUnit->p0.GetPosition())/
                        fabs(pCurrentUnit->p1.GetPosition() -
                        pCurrentUnit->p0.GetPosition());
              tInterStart = (pCurrentUnit->timeInterval.end -
                            pCurrentUnit->timeInterval.start) * factor +
                            pCurrentUnit->timeInterval.start;
            }
            if (lStart <= mgEnd) {
              interEnd = pCurrentUnit->p1.GetPosition();
              tInterEnd = pCurrentUnit->timeInterval.end;
              bInterEnd = pCurrentUnit->timeInterval.rc;
            } else {
              interEnd = lStart;
              bInterEnd = true;
              factor =fabs(lStart - pCurrentUnit->p0.GetPosition())/
                        fabs(pCurrentUnit->p1.GetPosition() -
                        pCurrentUnit->p0.GetPosition());
              tInterEnd = (pCurrentUnit->timeInterval.end -
                            pCurrentUnit->timeInterval.start) * factor +
                            pCurrentUnit->timeInterval.start;
            }
            if (interStart == interEnd && (!bInterStart || !bInterEnd)) {
              continue;
            } else {
              pResult->Add(UGPoint(Interval<Instant> (tInterStart, tInterEnd,
                                bInterStart, bInterEnd), iNetworkId, iRouteMgp,
                                pCurrentUnit->p0.GetSide(), interStart,
                                interEnd));
            }
          }
        }
      }
      j++;
    }
  }
  pResult->EndBulkLoad();
  pResult->SetDefined(true);
  return 0;
};

/*
Value mapping function of operator ~at~

*/
ValueMapping OpTempNetAt::atmap[] = {
  at_mgpgp,
  at_mgpgl
};

int
OpTempNetAt::SelectAt( ListExpr args )
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
const string OpTempNetAt::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint x A -> mgpoint if A is gpoint or gline" "</text--->"
  "<text> _ at _ </text--->"
  "<text>Restricts the moving gpoint to the times he was at"
    " the gpoint or gline.</text--->"
  "<text> X_MGPOINT at X_GPOINT</text--->"
  ") )";
