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

1.1 Implementation of Operator distance

The operator returns a moving real representing the distance between the
two given moving gpoint.

April 2008 Simone Jandt

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

#include "OpTempNetDistance.h"

int intervalCheck (Interval<Instant> i1, Interval<Instant> i2) {
  if (i1.start > i2.end) return 1;
  if (i1.end < i2.start ) return 16;
  if (i1.start == i1.end && i1.start < i2.start) return 19;
  if (i1.start == i1.end && i1.start > i2.end) return 21;
  if (i1.start > i2.start && i1.start == i2.end) return 2;
  if (i1.start > i2.start && i1.start < i2.end && i1.end < i2.end) return 3;
  if (i1.start > i2.start && i1.end == i2.end) return 4;
  if (i1.start > i2.start && i1.end > i2.end && i2.end > i2.start) return 5;
  if (i1.start == i2.start && i1.end > i2.end) return 6;
  if (i1.start == i2.start && i1.end == i2.end) return 7;
  if (i1.start == i2.start && i1.end < i2.end) return 8;
  if (i1.start == i2.start && i1.start == i2.end) return 9;
  if (i1.start < i2.start && i1.end > i2.end) return 10;
  if (i1.start < i2.start && i1.end == i2.end) return 11;
  if (i1.start < i2.start && i1.end < i2.end && i1.end > i2.start) return 12;
  if (i1.start < i2.start && i1.end > i2.end && i2.start == i2.end) return 13;
  if (i1.end == i2.start && i2.start == i2.end) return 14;
  if (i1.end == i2.start && i1.end < i2.end) return 15;
  if (i1.start == i1.end && i1.start == i2.start) return 17;
  if (i1.start == i1.end && i1.start == i2.end) return 18;
  if (i1.start == i1.end && i1.start > i2.start && i1.start < i2.end) return 20;
  return -1; //should never be reached
};

void refinementMovingGPoint (MGPoint *a, MGPoint *b,
                             MGPoint *&resA, MGPoint *&resB){
  const UGPoint *ua, *ub;
  GPoint pos1, pos2;
  int i = 0;
  int j = 0;
  a->Get(i,ua);
  b->Get(j,ub);
  resA->StartBulkLoad();
  resB->StartBulkLoad();
  while (i < a->GetNoComponents() && j < b->GetNoComponents()) {
    switch (intervalCheck(ua->timeInterval, ub->timeInterval)) {
      case 1: //no units to add
        j++;
        if (j < b->GetNoComponents()) b->Get(j, ub);
        break;
      case 2:
        if (ua->timeInterval.lc && ub->timeInterval.rc) {
          resA->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                             ua->timeInterval.start,
                                             true, true),
                                             ua->p0.GetNetworkId(),
                                             ua->p0.GetRouteId(),
                                             ua->p0.GetSide(),
                                             ua->p0.GetPosition(),
                                             ua->p0.GetPosition()));
          resB->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                             ua->timeInterval.start,
                                             true, true),
                                             ub->p1.GetNetworkId(),
                                             ub->p1.GetRouteId(),
                                             ub->p1.GetSide(),
                                             ub->p1.GetPosition(),
                                             ub->p1.GetPosition()));
        }
        j++;
        if (j < b->GetNoComponents()) b->Get(j, ub);
        break;
      case 3:
        ub->TemporalFunction(ua->timeInterval.start, pos1, false);
        ub->TemporalFunction(ua->timeInterval.end, pos2, false);
        resA->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                           ua->timeInterval.end,
                                           ua->timeInterval.lc,
                                           ua->timeInterval.rc),
                                             ua->p0.GetNetworkId(),
                                             ua->p0.GetRouteId(),
                                             ua->p0.GetSide(),
                                             ua->p0.GetPosition(),
                                             ua->p1.GetPosition()));
        resB->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                            ua->timeInterval.end,
                                            ua->timeInterval.lc,
                                            ua->timeInterval.rc),
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             pos1.GetPosition(),
                                             pos2.GetPosition()));
        i++;
        if (i < a->GetNoComponents()) a->Get(i, ua);
        break;
      case 4:
        ub->TemporalFunction(ua->timeInterval.start, pos2, false);
        resA->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                           ua->timeInterval.end,
                                           ua->timeInterval.lc,
                                           ua->timeInterval.rc &&
                                           ub->timeInterval.rc),
                                             ua->p0.GetNetworkId(),
                                             ua->p0.GetRouteId(),
                                             ua->p0.GetSide(),
                                             ua->p0.GetPosition(),
                                             ua->p1.GetPosition()));
        resB->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                           ua->timeInterval.end,
                                           ua->timeInterval.lc,
                                           ua->timeInterval.rc &&
                                           ub->timeInterval.rc),
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             pos2.GetPosition(),
                                             ub->p1.GetPosition()));
        if (ua->timeInterval.rc && ub->timeInterval.rc) {
          i++;
          if (i < a->GetNoComponents()) a->Get(i, ua);
          j++;
          if (j < b->GetNoComponents()) b->Get(j, ub);
        } else {
          if (!ua->timeInterval.rc) {
            i++;
            if (i < a->GetNoComponents()) a->Get(i, ua);
          }
          if (!ub->timeInterval.rc) {
            j++;
            if (j < b->GetNoComponents()) b->Get(j, ub);
          }
        }
        break;
      case 5:
        ua->TemporalFunction(ub->timeInterval.end, pos1, false);
        ub->TemporalFunction(ua->timeInterval.start, pos2, false);
        resA->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                             ub->timeInterval.end,
                                             ua->timeInterval.lc,
                                             ub->timeInterval.rc),
                                            ua->p0.GetNetworkId(),
                                            ua->p0.GetRouteId(),
                                            ua->p0.GetSide(),
                                            ua->p0.GetPosition(),
                                            pos1.GetPosition()));
        resB->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                             ub->timeInterval.end,
                                             ua->timeInterval.lc,
                                             ub->timeInterval.rc),
                                            ub->p0.GetNetworkId(),
                                            ub->p0.GetRouteId(),
                                            ub->p0.GetSide(),
                                            pos2.GetPosition(),
                                            ub->p1.GetPosition()));
        j++;
        if (j < b->GetNoComponents()) b->Get(j, ub);
        break;
      case 6:
        ua->TemporalFunction(ub->timeInterval.end, pos1, false);
        resA->Add(UGPoint(Interval<Instant>(ub->timeInterval.start,
                                           ub->timeInterval.end,
                                           ub->timeInterval.lc &&
                                           ua->timeInterval.lc,
                                           ub->timeInterval.rc),
                                             ua->p0.GetNetworkId(),
                                             ua->p0.GetRouteId(),
                                             ua->p0.GetSide(),
                                             ua->p0.GetPosition(),
                                             pos1.GetPosition()));
        resB->Add(UGPoint(Interval<Instant>(ub->timeInterval.start,
                                           ub->timeInterval.end,
                                           ub->timeInterval.lc &&
                                           ua->timeInterval.lc,
                                           ub->timeInterval.rc),
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             ub->p0.GetPosition(),
                                             ub->p1.GetPosition()));
        j++;
        if (j < b->GetNoComponents()) b->Get(j, ub);
        break;
      case 7:
        resA->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                           ua->timeInterval.end,
                                           ua->timeInterval.lc &&
                                           ub->timeInterval.lc,
                                           ua->timeInterval.rc &&
                                           ub->timeInterval.rc),
                                             ua->p0.GetNetworkId(),
                                             ua->p0.GetRouteId(),
                                             ua->p0.GetSide(),
                                             ua->p0.GetPosition(),
                                             ua->p1.GetPosition()));
        resB->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                           ua->timeInterval.end,
                                           ua->timeInterval.lc &&
                                           ub->timeInterval.lc,
                                           ua->timeInterval.rc &&
                                           ub->timeInterval.rc),
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             ub->p0.GetPosition(),
                                             ub->p1.GetPosition()));
        if (ua->timeInterval.rc && ub->timeInterval.rc) {
          i++;
          if (i < a->GetNoComponents()) a->Get(i, ua);
          j++;
          if (j < b->GetNoComponents()) b->Get(j, ub);
        } else {
          if (!ua->timeInterval.rc) {
            i++;
            if (i < a->GetNoComponents()) a->Get(i, ua);
          }
          if (!ub->timeInterval.rc) {
            j++;
            if (j < b->GetNoComponents()) b->Get(j, ub);
          }
        }
        break;
      case 8:
        ub->TemporalFunction(ua->timeInterval.end, pos2, false);
        resA->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                           ua->timeInterval.end,
                                           ua->timeInterval.lc &&
                                           ub->timeInterval.lc,
                                           ua->timeInterval.rc),
                                             ua->p0.GetNetworkId(),
                                             ua->p0.GetRouteId(),
                                             ua->p0.GetSide(),
                                             ua->p0.GetPosition(),
                                             ua->p1.GetPosition()));
        resB->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                           ua->timeInterval.end,
                                           ua->timeInterval.lc &&
                                           ub->timeInterval.lc,
                                           ua->timeInterval.rc),
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             ub->p0.GetPosition(),
                                             pos2.GetPosition()));
        i++;
        if (i < a->GetNoComponents()) a->Get(i, ua);
        break;
      case 9:
        if (ua->timeInterval.lc && ub->timeInterval.lc && ub->timeInterval.rc) {
          resA->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                           ua->timeInterval.start,
                                           true, true),
                                             ua->p0.GetNetworkId(),
                                             ua->p0.GetRouteId(),
                                             ua->p0.GetSide(),
                                             ua->p0.GetPosition(),
                                             ua->p1.GetPosition()));
          resB->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                             ua->timeInterval.start,
                                             true, true),
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             ub->p0.GetPosition(),
                                             ub->p0.GetPosition()));
        }
        j++;
        if (j < b->GetNoComponents()) b->Get(j, ub);
        break;
      case 10:
        ua->TemporalFunction(ub->timeInterval.start, pos1, false);
        ua->TemporalFunction(ub->timeInterval.end, pos2, false);
        resA->Add(UGPoint(Interval<Instant>(ub->timeInterval.start,
                                             ub->timeInterval.end,
                                             ub->timeInterval.lc,
                                             ub->timeInterval.rc),
                                            ua->p0.GetNetworkId(),
                                            ua->p0.GetRouteId(),
                                            ua->p0.GetSide(),
                                            pos1.GetPosition(),
                                            pos2.GetPosition()));
        resB->Add(UGPoint(Interval<Instant>(ub->timeInterval.start,
                                             ub->timeInterval.end,
                                             ub->timeInterval.lc,
                                             ub->timeInterval.rc),
                                            ub->p0.GetNetworkId(),
                                            ub->p0.GetRouteId(),
                                            ub->p0.GetSide(),
                                            ub->p0.GetPosition(),
                                            ub->p1.GetPosition()));
        j++;
        if (j < b->GetNoComponents()) b->Get(j, ub);
        break;
      case 11:
        ua->TemporalFunction(ub->timeInterval.start, pos1, false);
        resA->Add(UGPoint(Interval<Instant>(ub->timeInterval.start,
                                           ub->timeInterval.end,
                                           ub->timeInterval.lc,
                                           ua->timeInterval.rc &&
                                           ub->timeInterval.rc),
                                             ua->p0.GetNetworkId(),
                                             ua->p0.GetRouteId(),
                                             ua->p0.GetSide(),
                                             pos1.GetPosition(),
                                             ua->p1.GetPosition()));
        resB->Add(UGPoint(Interval<Instant>(ub->timeInterval.start,
                                           ub->timeInterval.end,
                                           ub->timeInterval.lc,
                                           ua->timeInterval.rc &&
                                           ub->timeInterval.rc),
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             ub->p0.GetPosition(),
                                             ub->p1.GetPosition()));
        if (ua->timeInterval.rc && ub->timeInterval.rc) {
          i++;
          if (i < a->GetNoComponents()) a->Get(i, ua);
          j++;
          if (j < b->GetNoComponents()) b->Get(j, ub);
        } else {
          if (!ua->timeInterval.rc) {
            i++;
            if (i < a->GetNoComponents()) a->Get(i, ua);
          }
          if (!ub->timeInterval.rc) {
            j++;
            if (j < b->GetNoComponents()) b->Get(j, ub);
          }
        }
        break;
      case 12:
        ua->TemporalFunction(ub->timeInterval.start, pos1, false);
        ub->TemporalFunction(ua->timeInterval.end, pos2, false);
        resA->Add(UGPoint(Interval<Instant>(ub->timeInterval.start,
                                           ua->timeInterval.end,
                                           ub->timeInterval.lc,
                                           ua->timeInterval.rc),
                                             ua->p0.GetNetworkId(),
                                             ua->p0.GetRouteId(),
                                             ua->p0.GetSide(),
                                             pos1.GetPosition(),
                                             ua->p1.GetPosition()));
        resB->Add(UGPoint(Interval<Instant>(ub->timeInterval.start,
                                           ua->timeInterval.end,
                                           ub->timeInterval.lc,
                                           ua->timeInterval.rc),
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             ub->p0.GetPosition(),
                                             pos2.GetPosition()));
        i++;
        if (i < a->GetNoComponents()) a->Get(i, ua);
        break;
      case 13:
        if (ub->timeInterval.lc && ub->timeInterval.rc) {
          ua->TemporalFunction(ub->timeInterval.start, pos1, false);
          resA->Add(UGPoint(Interval<Instant>(ub->timeInterval.start,
                                             ub->timeInterval.end,
                                             ub->timeInterval.lc,
                                             ub->timeInterval.rc),
                                             ua->p0.GetNetworkId(),
                                             ua->p0.GetRouteId(),
                                             ua->p0.GetSide(),
                                             pos1.GetPosition(),
                                             pos1.GetPosition()));
          resB->Add(UGPoint(Interval<Instant>(ub->timeInterval.start,
                                             ub->timeInterval.end,
                                             ub->timeInterval.lc,
                                             ub->timeInterval.rc),
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             ub->p0.GetPosition(),
                                             ub->p0.GetPosition()));
        }
        j++;
        if (j < b->GetNoComponents()) b->Get(j, ub);
        break;
      case 14:
        resA->Add(UGPoint(Interval<Instant>(ub->timeInterval.start,
                                             ub->timeInterval.end,
                                             ub->timeInterval.lc,
                                             ub->timeInterval.rc),
                                            ua->p0.GetNetworkId(),
                                            ua->p0.GetRouteId(),
                                            ua->p0.GetSide(),
                                            ua->p1.GetPosition(),
                                            ua->p1.GetPosition()));
        resB->Add(UGPoint(Interval<Instant>(ub->timeInterval.start,
                                             ub->timeInterval.end,
                                             ub->timeInterval.lc,
                                             ub->timeInterval.rc),
                                            ub->p0.GetNetworkId(),
                                            ub->p0.GetRouteId(),
                                            ub->p0.GetSide(),
                                            ub->p0.GetPosition(),
                                            ub->p0.GetPosition()));
        i++;
        if (i < a->GetNoComponents()) a->Get(i, ua);
        j++;
        if (j < b->GetNoComponents()) b->Get(j, ub);
        break;
      case 15:
        if (ua->timeInterval.rc && ub->timeInterval.lc) {
          resA->Add(UGPoint(Interval<Instant>(ua->timeInterval.end,
                                              ua->timeInterval.end,
                                              true, true),
                                             ua->p0.GetNetworkId(),
                                             ua->p0.GetRouteId(),
                                             ua->p0.GetSide(),
                                             ua->p1.GetPosition(),
                                             ua->p1.GetPosition()));
          resB->Add(UGPoint(Interval<Instant>(ua->timeInterval.end,
                                              ua->timeInterval.end,
                                              true, true),
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             ub->p0.GetPosition(),
                                             ub->p0.GetPosition()));
        }
        i++;
        if (i < a->GetNoComponents()) a->Get(i, ua);
        break;
      case 16: //nothing to add
        i++;
        if (i < a->GetNoComponents()) a->Get(i, ua);
        break;
      case 17:
        if (ua->timeInterval.lc && ua->timeInterval.rc && ub->timeInterval.lc){
          resA->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                             ua->timeInterval.end,
                                             true, true),
                                             ua->p0.GetNetworkId(),
                                             ua->p0.GetRouteId(),
                                             ua->p0.GetSide(),
                                             ua->p0.GetPosition(),
                                             ua->p0.GetPosition()));
          resB->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                             ua->timeInterval.end,
                                             true, true),
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             ub->p0.GetPosition(),
                                             ub->p0.GetPosition()));
        }
        i++;
        if (i < a->GetNoComponents()) a->Get(i, ua);
        break;
      case 18:
        if (ua->timeInterval.lc && ua->timeInterval.rc && ub->timeInterval.rc){
          resA->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                             ua->timeInterval.end,
                                             true, true),
                                             ua->p0.GetNetworkId(),
                                             ua->p0.GetRouteId(),
                                             ua->p0.GetSide(),
                                             ua->p0.GetPosition(),
                                             ua->p0.GetPosition()));
          resB->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                             ua->timeInterval.end,
                                             true, true),
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             ub->p1.GetPosition(),
                                             ub->p1.GetPosition()));
        }
        i++;
        if (i < a->GetNoComponents()) a->Get(i, ua);
        j++;
        if (j < b->GetNoComponents()) b->Get(j, ub);
        break;
      case 19:
        i++;
        if (i < a->GetNoComponents()) a->Get(i, ua);
        break;
      case 20:
        if (ua->timeInterval.lc && ua->timeInterval.rc) {
          ub->TemporalFunction(ua->timeInterval.start, pos2, false);
          resA->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                             ua->timeInterval.end,
                                             ua->timeInterval.lc,
                                             ua->timeInterval.rc),
                                             ua->p0.GetNetworkId(),
                                             ua->p0.GetRouteId(),
                                             ua->p0.GetSide(),
                                             ua->p0.GetPosition(),
                                             ua->p0.GetPosition()));
          resB->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                             ua->timeInterval.end,
                                             ua->timeInterval.lc,
                                             ua->timeInterval.rc),
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             pos2.GetPosition(),
                                             pos2.GetPosition()));
        }
        i++;
        if (i < a->GetNoComponents()) a->Get(i, ua);
        break;
      case 21:
        j++;
        if (j < b->GetNoComponents()) b->Get(j, ub);
        break;
      default: //should never happen
        cerr << "an error occured while checking the time interval." << endl;
        resA->EndBulkLoad();
        resB->EndBulkLoad();
        resA->SetDefined(false);
        resB->SetDefined(false);
        return;
    } // end switch
  }//end while
  resA->EndBulkLoad();
  resB->EndBulkLoad();
  resA->SetDefined(true);
  resB->SetDefined(true);
  return;
}



/*
Typemap function of the operator ~distance~

*/
ListExpr OpTempNetDistance::TypeMapping(ListExpr in_xArgs)
{
  ListExpr arg1, arg2;
  if( nl->ListLength(in_xArgs) == 2 ){
    arg1 = nl->First(in_xArgs);
    arg2 = nl->Second(in_xArgs);
    if (nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
        nl->SymbolValue(arg1) == "mgpoint" && nl->IsAtom(arg2) &&
        nl->AtomType(arg2) == SymbolType &&
        nl->SymbolValue(arg2) == "mgpoint"){
        return (nl->SymbolAtom("mgpoint"));
    }
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
Value mapping function of operator ~distance~

*/
int OpTempNetDistance::ValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  // Get (empty) return value
  MGPoint* pResult = (MGPoint*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pResult);
  pResult->Clear();
  pResult->SetDefined(true);
  // Get input values
  MGPoint* pMGPoint1 = (MGPoint*)args[0].addr;
  if(pMGPoint1 == NULL || pMGPoint1->GetNoComponents() < 1 ||
      !pMGPoint1->IsDefined()) {
    cerr << "First mgpoint does not exist." << endl;
    pResult->SetDefined(false);
    return 0;
  }
  MGPoint* pMGPoint2 = (MGPoint*)args[1].addr;
  if(pMGPoint2 == NULL || pMGPoint2->GetNoComponents() < 1 ||
      !pMGPoint2->IsDefined()) {
    cerr << "Second mgpoint does not exist." << endl;
    pResult->SetDefined(false);
    return 0;
  }
  const UGPoint *pCurr1, *pCurr2;
  pMGPoint1->Get(0, pCurr1);
  pMGPoint2->Get(0, pCurr2);
  if (pCurr1->p0.GetNetworkId() != pCurr2->p0.GetNetworkId()) {
    cerr << "mgpoints belong to different networks." << endl;
    pResult->SetDefined(true);
    return 0;
  }
  Network *pNetwork = NetworkManager::GetNetwork(pCurr1->p0.GetNetworkId());
  MGPoint *resA = new MGPoint(0);
  MGPoint *resB = new MGPoint(0);
  resA->Clear();
  resB->Clear();
  refinementMovingGPoint (pMGPoint1, pMGPoint2, resA, resB);
  if (resA == NULL || !resA->IsDefined() ||
      resB == NULL || !resB->IsDefined() ||
      resA->GetNoComponents() != resB->GetNoComponents ()){
    pResult->SetDefined(false);
    return 0;
  }
  if (resA->GetNoComponents() < 1) {
    pResult->SetDefined(true);
    return 0;
  }
  pResult->StartBulkLoad();
  double interPosition, factor, factor2;
  Instant tinter, tinter2;
  for (int i = 0; i < resA->GetNoComponents() ; i++) {
    resA->Get(i,pCurr1);
    resB->Get(i,pCurr2);
    if (pCurr1->p0.GetNetworkId() == pCurr2->p0.GetNetworkId() &&
        pCurr1->p0.GetRouteId() == pCurr2->p0.GetRouteId() &&
        pCurr1->p0.GetSide() == pCurr2->p0.GetSide()) {;
      if (pCurr1->p0.GetPosition() == pCurr2->p0.GetPosition() &&
          pCurr1->p1.GetPosition() == pCurr2->p1.GetPosition()) {
        pResult->Add(UGPoint(Interval<Instant> (pCurr1->timeInterval.start,
                                               pCurr1->timeInterval.end,
                                              pCurr1->timeInterval.lc,
                                              pCurr1->timeInterval.rc),
                                              pCurr1->p0.GetNetworkId(),
                                              pCurr1->p0.GetRouteId(),
                                              pCurr1->p0.GetSide(),
                                              pCurr1->p0.GetPosition(),
                                              pCurr1->p1.GetPosition()));
      } else {
        tinter = (pCurr1->timeInterval.end - pCurr1->timeInterval.start) *
                 ((pCurr2->p0.GetPosition() - pCurr1->p0.GetPosition()) /
                  (pCurr1->p1.GetPosition() - pCurr1->p0.GetPosition() -
                  pCurr2->p1.GetPosition() + pCurr2->p0.GetPosition())) +
                 pCurr1->timeInterval.start;
        if (pCurr1->timeInterval.start <= tinter &&
            tinter <= pCurr1->timeInterval.end) {
          interPosition = pCurr1->p0.GetPosition() +
                    ((pCurr1->p1.GetPosition() - pCurr1->p0.GetPosition()) *
                    ((tinter.ToDouble() - pCurr1->timeInterval.start.ToDouble())
                    / (pCurr1->timeInterval.end.ToDouble() -
                       pCurr1->timeInterval.start.ToDouble())));
          if (!(fabs(interPosition - pCurr1->p0.GetPosition()) < 0.01||
              fabs(interPosition - pCurr1->p1.GetPosition()) < 0.01 ||
              fabs(interPosition - pCurr2->p0.GetPosition()) < 0.01 ||
              fabs(interPosition - pCurr2->p1.GetPosition()) < 0.01)) {
            pResult->Add(UGPoint(Interval<Instant> (tinter, tinter, true, true),
                                                pCurr1->p0.GetNetworkId(),
                                                pCurr1->p0.GetRouteId(),
                                                pCurr1->p0.GetSide(),
                                                interPosition,
                                                interPosition));
          } else {
            if (pCurr1->timeInterval.lc == true &&
                fabs(interPosition - pCurr1->p0.GetPosition()) < 0.01){
              pResult->Add(UGPoint(Interval<Instant> (tinter, tinter,
                                                true, true),
                                                pCurr1->p0.GetNetworkId(),
                                                pCurr1->p0.GetRouteId(),
                                                pCurr1->p0.GetSide(),
                                                interPosition,
                                                interPosition));
            } else {
              if (pCurr1->timeInterval.rc == true &&
                fabs(interPosition - pCurr1->p1.GetPosition()) < 0.01){
               pResult->Add(UGPoint(Interval<Instant> (tinter, tinter,
                                                true, true),
                                                pCurr1->p0.GetNetworkId(),
                                                pCurr1->p0.GetRouteId(),
                                                pCurr1->p0.GetSide(),
                                                interPosition,
                                                interPosition));
              }
            }
          }
        }
      }
    } else {
      vector<JunctionSortEntry> junctions;
      JunctionSortEntry pCurrJunct;
      pCurrJunct.m_pJunction = 0;
      junctions.clear();
      int rid = pCurr1->p0.GetRouteId();
      CcInt pRid = rid;
      pNetwork->GetJunctionsOnRoute(&pRid, junctions);
      size_t k = 0;
      bool found = false;
      while (k < junctions.size() && !found){
        pCurrJunct = junctions[k];
        if ((pCurr1->p0.GetPosition() <= pCurrJunct.getRouteMeas() &&
            pCurrJunct.getRouteMeas() <= pCurr1->p1.GetPosition()) ||
            (pCurr1->p1.GetPosition() <= pCurrJunct.getRouteMeas() &&
            pCurrJunct.getRouteMeas() <= pCurr1->p0.GetPosition()) &&
            pCurrJunct.getOtherRouteId() == pCurr2->p0.GetRouteId()) {
          found = true;
          interPosition = pCurrJunct.getOtherRouteMeas();
          if ((pCurr2->p0.GetPosition() <= interPosition &&
             interPosition <= pCurr2->p1.GetPosition()) ||
             (pCurr2->p1.GetPosition() <= interPosition &&
             interPosition <= pCurr2->p0.GetPosition())) {
            factor = fabs(pCurrJunct.getRouteMeas() -
                          pCurr1->p0.GetPosition()) /
                    fabs(pCurr1->p1.GetPosition() - pCurr1->p0.GetPosition());
            tinter = (pCurr1->timeInterval.end - pCurr1->timeInterval.start) *
                     factor + pCurr1->timeInterval.start;
            factor2 = fabs(interPosition - pCurr2->p0.GetPosition()) /
                    fabs(pCurr2->p1.GetPosition() - pCurr2->p0.GetPosition());
            tinter2 = (pCurr1->timeInterval.end - pCurr1->timeInterval.start) *
                     factor + pCurr1->timeInterval.start;
            if (tinter == tinter2) {
              pResult->Add(UGPoint(Interval<Instant> (tinter, tinter,
                                                  true, true),
                                                  pCurr1->p0.GetNetworkId(),
                                                  pCurr1->p0.GetRouteId(),
                                                  pCurr1->p0.GetSide(),
                                                  pCurrJunct.getRouteMeas(),
                                                  pCurrJunct.getRouteMeas()));
            }
          }
        };
        k++;
      }
    }// end if else same route and side
  }// end for
  pResult->EndBulkLoad();
  pResult->SetDefined(true);
  delete pNetwork;
  return 0;
}
/*
Specification of operator ~distance~

*/
const string OpTempNetDistance::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint x mgpoint -> mreal" "</text--->"
  "<text>distance(mgpoint, mgpoint)</text--->"
  "<text>Returns a moving real representing the distance of the two "
    " given moving gpoint.</text--->"
  "<text>distance(mgpoint, mgpoint)</text--->"
  ") )";

