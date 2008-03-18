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

1.1 Implementation of Operator inside

This operator returns a moving bool which is true for the times the moving
gpoint moves at a given gline.

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

#include "OpTempNetInside.h"


void checkEndOfUGPoint(double startPos, double endPos, Instant startTime,
                       bool bstart, Instant endTime, bool bend, int actRoutInt,
                       GLine* pGLine, MBool* &pResult,
                       int iRouteId){
  const RouteInterval *pCurrInt;
  UBool interbool(true);
  bool swapped = false;
  bool found = false;
  double lStart, lEnd, factor, help;
  Instant tInterStart, tInterEnd;
  int k = actRoutInt + 1;
  while ( k < pGLine->NoOfComponents()) {
    pGLine->Get(k, pCurrInt);
    if (pCurrInt->m_iRouteId == iRouteId) {
      if (endPos < startPos) {
          help = startPos;
          startPos = endPos;
          endPos = help;
          swapped = true;
      }
      lStart = pCurrInt->m_dStart;
      lEnd = pCurrInt->m_dEnd;
      if (lStart > lEnd) {
        lStart = pCurrInt->m_dEnd;
        lEnd = pCurrInt->m_dStart;
      }
      if (!(endPos < lStart || startPos > lEnd || startPos == endPos ||
             lStart == lEnd)){
          //intersection exists compute intersecting part and timevalues for
          //resulting unit
        if (!swapped) {
          if (lStart <= startPos) {
            found = true;
            interbool.timeInterval.start = startTime;
            interbool.timeInterval.lc = bstart;
            interbool.constValue.Set(true, true);
          } else {
            found = true;
            // compute and write unit befor mgpoint inside gline
            interbool.timeInterval.start = startTime;
            interbool.timeInterval.lc = bstart;
            factor =  fabs(lStart - startPos) / fabs(endPos - startPos);
            tInterStart = (endTime - startTime) * factor + startTime;
            interbool.timeInterval.end = tInterStart;
            interbool.timeInterval.rc = false;
            interbool.constValue.Set(true,false);
            pResult->MergeAdd(interbool);
            //compute start of unit inside
            interbool.timeInterval.start = tInterStart;
            interbool.timeInterval.lc = true;
            interbool.constValue.Set(true, true);
          }
          if (lEnd >= endPos) {
            interbool.timeInterval.end = endTime;
            interbool.timeInterval.rc = bend;
            pResult->MergeAdd(interbool);
          } else {
            // compute end of mgpoint at end of gline.and add result unit
            interbool.timeInterval.rc = true;
            factor = fabs(lEnd - startPos) / fabs(endPos -startPos);
            tInterEnd = (endTime -startTime) * factor + startTime;
            interbool.timeInterval.end = tInterEnd;
            pResult->MergeAdd(interbool);
            // the rest of the current unit is not in the current
            // routeinterval.
            checkEndOfUGPoint(lEnd, endPos, tInterEnd, false, endTime, bend,
                                k, pGLine, pResult, iRouteId);
          }
        } else {
          help = startPos;
          startPos = endPos;
          endPos = help;
          if (lEnd >= startPos) {
            found = true;
            interbool.timeInterval.start = startTime;
            interbool.timeInterval.lc = bstart;
            interbool.constValue.Set(true, true);
          } else {
            found = true;
            // compute and write unit befor mgpoint inside gline
            interbool.timeInterval.start = startTime;
            interbool.timeInterval.lc = bstart;
            factor =  fabs(lEnd - startPos) / fabs(endPos - startPos);
            tInterStart = (endTime - startTime) * factor + startTime;
            interbool.timeInterval.end = tInterStart;
            interbool.timeInterval.rc = false;
            interbool.constValue.Set(true,false);
            pResult->MergeAdd(interbool);
            //compute start of unit inside
            interbool.timeInterval.start = tInterStart;
            interbool.timeInterval.lc = true;
            interbool.constValue.Set(true, true);
          }
          if (lStart <= endPos) {
              interbool.timeInterval.end = endTime;
              interbool.timeInterval.rc = bend;
              pResult->MergeAdd(interbool);
          } else {
              // compute end of mgpoint at end of gline.and add result unit
              interbool.timeInterval.rc = true;
              factor = fabs(lStart - startPos) / fabs(endPos - startPos);
              tInterEnd = (endTime - startTime) * factor + startTime;
              interbool.timeInterval.end = tInterEnd;
              pResult->MergeAdd(interbool);
              // the rest of the current unit is not in the current
              // routeinterval.
              checkEndOfUGPoint(lEnd, endPos, tInterEnd, false, endTime, bend,
                                k, pGLine, pResult, iRouteId);
          }
        }
      }else{
        if (startPos == endPos && bstart && bend){
          found = true;
          interbool.timeInterval.start = startTime;
          interbool.timeInterval.lc = bstart;
          interbool.timeInterval.end = endTime;
          interbool.timeInterval.rc = bend;
          interbool.constValue.Set(true,true);
          pResult->MergeAdd(interbool);
        } else {
          if (lStart == lEnd) {
            found = true;
            if ((lStart > startPos && lStart < endPos) ||
                  (lStart < startPos && lStart > endPos)) {
              // compute and write unit befor mgpoint inside gline
              interbool.timeInterval.start = startTime;
              interbool.timeInterval.lc = bstart;
              factor =  fabs(lStart - startPos) / fabs(endPos -startPos);
              tInterStart = (endTime - startTime) * factor + startTime;
              interbool.timeInterval.end = tInterStart;
              interbool.timeInterval.rc = false;
              interbool.constValue.Set(true,false);
              pResult->MergeAdd(interbool);
              interbool.timeInterval.start = tInterStart;
              interbool.timeInterval.rc = true;
              interbool.timeInterval.lc = true;
              interbool.constValue.Set(true, true);
              pResult->MergeAdd(interbool);
              checkEndOfUGPoint(lEnd, endPos, tInterEnd, false, endTime, bend,
                                 k, pGLine, pResult, iRouteId);
            } else {
              if (lStart == startPos && bstart) {
                found = true;
                interbool.timeInterval.start = startTime;
                interbool.timeInterval.lc = bstart;
                interbool.timeInterval.end = startTime;
                interbool.timeInterval.rc = true;
                interbool.constValue.Set(true,true);
                pResult->MergeAdd(interbool);
                checkEndOfUGPoint(lEnd, endPos, tInterEnd, false, endTime, bend,
                                  k, pGLine, pResult, iRouteId);
              } else {
                if (lStart == endPos && bend) {
                  found = true;
                  interbool.timeInterval.start = startTime;
                  interbool.timeInterval.lc = bstart;
                  interbool.timeInterval.end = endTime;
                  interbool.timeInterval.rc = false;
                  interbool.constValue.Set(true,false);
                  pResult->MergeAdd(interbool);
                  interbool.timeInterval.start = endTime;
                  interbool.timeInterval.rc = bend;
                  interbool.timeInterval.lc = true;
                  interbool.constValue.Set(true, true);
                  pResult->MergeAdd(interbool);
                }
              }
            }
          } else {
            if ((startPos == lStart && endPos == lEnd)||
               (startPos == lEnd && endPos == lStart)){
              found = true;
              interbool.timeInterval.start = startTime;
              interbool.timeInterval.end = endTime;
              interbool.timeInterval.lc = bstart;
              interbool.timeInterval.rc = bend;
              interbool.constValue.Set(true, true);
              pResult->MergeAdd(interbool);
            }
          }
        }
      }
    } // end if routeid==
    k++;
  } // end while
  if (!found) {
      interbool.timeInterval.start = startTime;
      interbool.timeInterval.lc = bstart;
      interbool.timeInterval.end = endTime;
      interbool.timeInterval.rc = bend;
      interbool.constValue.Set(true,false);
      pResult->MergeAdd(interbool);
  }
}

/*
Typemap function of the operator

*/
ListExpr OpTempNetInside::TypeMapping(ListExpr in_xArgs)
{
  ListExpr arg1, arg2;
  if ( nl->ListLength(  in_xArgs ) == 2 )
  {
    arg1 = nl->First( in_xArgs );
    arg2 = nl->Second( in_xArgs );

    if ( nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
         nl->SymbolValue(arg1) == "mgpoint" && nl->IsAtom(arg2) &&
         nl->AtomType(arg2) == SymbolType && nl->SymbolValue(arg2) == "gline"){
        return (nl->SymbolAtom("mbool"));
    }
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
Value mapping function of operator ~inside~

*/
int OpTempNetInside::ValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  // Get (empty) return value
  MBool* pResult = (MBool*)qp->ResultStorage(in_xSupplier).addr;
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
  double mgStart, mgEnd, lStart, lEnd, factor;
  const RouteInterval *pCurrRInter = new RouteInterval (0, 0.0, 0.0);
  bool swapped, found;
  Instant tInterStart, tInterEnd;
  UBool interbool(true);
  int iRouteMgp;
  pResult->StartBulkLoad();
  for (int i = 0; i < pMGPoint->GetNoComponents(); i++){
    pMGPoint->Get(i, pCurrentUnit);
    int j = 0;
    iRouteMgp = pCurrentUnit->p0.GetRouteId();
    swapped = false;
    found = false;
    while (j < pGLine->NoOfComponents()) {
      pGLine->Get(j, pCurrRInter);
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
        if (!(mgEnd < lStart || mgStart > lEnd || mgStart == mgEnd ||
             lStart == lEnd)){
          //intersection exists compute intersecting part and timevalues for
          //resulting unit
          found = true;
          if (!swapped) {
            if (lStart <= mgStart) {
              interbool.timeInterval.start = pCurrentUnit->timeInterval.start;
              interbool.timeInterval.lc = pCurrentUnit->timeInterval.lc;
              interbool.constValue.Set(true, true);
            } else {
              // compute and write unit befor mgpoint inside gline
              interbool.timeInterval.start = pCurrentUnit->timeInterval.start;
              interbool.timeInterval.lc = pCurrentUnit->timeInterval.lc;
              factor =  fabs(lStart - pCurrentUnit->p0.GetPosition())/
                        fabs(pCurrentUnit->p1.GetPosition() -
                        pCurrentUnit->p0.GetPosition());
              tInterStart = (pCurrentUnit->timeInterval.end -
                            pCurrentUnit->timeInterval.start) * factor +
                            pCurrentUnit->timeInterval.start;
              interbool.timeInterval.end = tInterStart;
              interbool.timeInterval.rc = false;
              interbool.constValue.Set(true,false);
              pResult->MergeAdd(interbool);
              //compute start of unit inside
              interbool.timeInterval.start = tInterStart;
              interbool.timeInterval.lc = true;
              interbool.constValue.Set(true, true);
            }
            if (lEnd >= mgEnd) {
              interbool.timeInterval.end = pCurrentUnit->timeInterval.end;
              interbool.timeInterval.rc = pCurrentUnit->timeInterval.rc;
              pResult->MergeAdd(interbool);
            } else {
              // compute end of mgpoint at end of gline.and add result unit
              interbool.timeInterval.rc = true;
              factor = fabs(lEnd - pCurrentUnit->p0.GetPosition())/
                        fabs(pCurrentUnit->p1.GetPosition() -
                        pCurrentUnit->p0.GetPosition());
              tInterEnd = (pCurrentUnit->timeInterval.end -
                           pCurrentUnit->timeInterval.start) * factor +
                           pCurrentUnit->timeInterval.start;
              interbool.timeInterval.end = tInterEnd;
              pResult->MergeAdd(interbool);
              // the rest of the current unit is not in the current
              // routeinterval.
              checkEndOfUGPoint(lEnd, pCurrentUnit->p1.GetPosition(), tInterEnd,
                               false, pCurrentUnit->timeInterval.end,
                               pCurrentUnit->timeInterval.rc, j, pGLine,
                               pResult, iRouteMgp);
            }
          } else {
            mgStart = pCurrentUnit->p0.GetPosition();
            mgEnd = pCurrentUnit->p1.GetPosition();
            if (lEnd >= mgStart) {
              interbool.timeInterval.start = pCurrentUnit->timeInterval.start;
              interbool.timeInterval.lc = pCurrentUnit->timeInterval.lc;
              interbool.constValue.Set(true, true);
            } else {
              // compute and write unit befor mgpoint inside gline
              interbool.timeInterval.start = pCurrentUnit->timeInterval.start;
              interbool.timeInterval.lc = pCurrentUnit->timeInterval.lc;
              factor =  fabs(lEnd - pCurrentUnit->p0.GetPosition())/
                        fabs(pCurrentUnit->p1.GetPosition() -
                        pCurrentUnit->p0.GetPosition());
              tInterStart = (pCurrentUnit->timeInterval.end -
                            pCurrentUnit->timeInterval.start) * factor +
                            pCurrentUnit->timeInterval.start;
              interbool.timeInterval.end = tInterStart;
              interbool.timeInterval.rc = false;
              interbool.constValue.Set(true,false);
              pResult->MergeAdd(interbool);
              //compute start of unit inside
              interbool.timeInterval.start = tInterStart;
              interbool.timeInterval.lc = true;
              interbool.constValue.Set(true, true);
            }
            if (lStart <= mgEnd) {
              interbool.timeInterval.end = pCurrentUnit->timeInterval.end;
              interbool.timeInterval.rc = pCurrentUnit->timeInterval.rc;
              pResult->MergeAdd(interbool);
            } else {
              // compute end of mgpoint at end of gline.and add result unit
              interbool.timeInterval.rc = true;
              factor = fabs(lStart - pCurrentUnit->p0.GetPosition())/
                        fabs(pCurrentUnit->p1.GetPosition() -
                        pCurrentUnit->p0.GetPosition());
              tInterEnd = (pCurrentUnit->timeInterval.end -
                           pCurrentUnit->timeInterval.start) * factor +
                           pCurrentUnit->timeInterval.start;
              interbool.timeInterval.end = tInterEnd;
              pResult->MergeAdd(interbool);
              // the rest of the current unit is not in the current
              // routeinterval.
              checkEndOfUGPoint(lEnd, pCurrentUnit->p1.GetPosition(), tInterEnd,
                               false, pCurrentUnit->timeInterval.end,
                               pCurrentUnit->timeInterval.rc, j, pGLine,
                               pResult, iRouteMgp);
            }
          }
        }else{
          if (mgStart == mgEnd && pCurrentUnit->timeInterval.lc &&
            pCurrentUnit->timeInterval.rc){
            found = true;
            interbool.timeInterval.start = pCurrentUnit->timeInterval.start;
            interbool.timeInterval.lc = pCurrentUnit->timeInterval.lc;
            interbool.timeInterval.end = pCurrentUnit->timeInterval.end;
            interbool.timeInterval.rc = pCurrentUnit->timeInterval.rc;
            interbool.constValue.Set(true,true);
            pResult->MergeAdd(interbool);
          } else {
            if (lStart == lEnd) {
              if ((lStart > mgStart && lStart < mgEnd) ||
                  (lStart < mgStart && lStart > mgEnd)) {
                found = true;
                // compute and write unit befor mgpoint inside gline
                interbool.timeInterval.start = pCurrentUnit->timeInterval.start;
                interbool.timeInterval.lc = pCurrentUnit->timeInterval.lc;
                factor =  fabs(lStart - pCurrentUnit->p0.GetPosition())/
                          fabs(pCurrentUnit->p1.GetPosition() -
                          pCurrentUnit->p0.GetPosition());
                tInterStart = (pCurrentUnit->timeInterval.end -
                            pCurrentUnit->timeInterval.start) * factor +
                            pCurrentUnit->timeInterval.start;
                interbool.timeInterval.end = tInterStart;
                interbool.timeInterval.rc = false;
                interbool.constValue.Set(true,false);
                pResult->MergeAdd(interbool);
                interbool.timeInterval.start = tInterStart;
                interbool.timeInterval.rc = true;
                interbool.timeInterval.lc = true;
                interbool.constValue.Set(true, true);
                pResult->MergeAdd(interbool);
                checkEndOfUGPoint(lEnd, pCurrentUnit->p1.GetPosition(),
                                  tInterEnd, false,
                                  pCurrentUnit->timeInterval.end,
                                  pCurrentUnit->timeInterval.rc, j, pGLine,
                                  pResult, iRouteMgp);
              } else {
                if (lStart == mgStart && pCurrentUnit->timeInterval.lc) {
                  found = true;
                  interbool.timeInterval.start =
                      pCurrentUnit->timeInterval.start;
                  interbool.timeInterval.lc = pCurrentUnit->timeInterval.lc;
                  interbool.timeInterval.end = pCurrentUnit->timeInterval.start;
                  interbool.timeInterval.rc = true;
                  interbool.constValue.Set(true,true);
                  pResult->MergeAdd(interbool);
                  checkEndOfUGPoint(lEnd, pCurrentUnit->p1.GetPosition(),
                                    tInterEnd, false,
                                    pCurrentUnit->timeInterval.end,
                                    pCurrentUnit->timeInterval.rc, j, pGLine,
                                    pResult, iRouteMgp);
                } else {
                  if (lStart == mgEnd && pCurrentUnit->timeInterval.rc) {
                    found = true;
                    interbool.timeInterval.start =
                      pCurrentUnit->timeInterval.start;
                    interbool.timeInterval.lc =
                        pCurrentUnit->timeInterval.lc;
                    interbool.timeInterval.end =
                        pCurrentUnit->timeInterval.end;
                    interbool.timeInterval.rc = false;
                    interbool.constValue.Set(true,false);
                    pResult->MergeAdd(interbool);
                    interbool.timeInterval.start =
                        pCurrentUnit->timeInterval.end;
                    interbool.timeInterval.rc = pCurrentUnit->timeInterval.rc;
                    interbool.timeInterval.lc = true;
                    interbool.constValue.Set(true, true);
                    pResult->MergeAdd(interbool);
                  }
                }
              }
            } else {
              if ((mgStart == lStart && mgEnd == lEnd) ||
                 (mgStart == lEnd && mgEnd == lStart)) {
                found = true;
                interbool.timeInterval.start = pCurrentUnit->timeInterval.start;
                interbool.timeInterval.end = pCurrentUnit->timeInterval.end;
                interbool.timeInterval.lc = pCurrentUnit->timeInterval.lc;
                interbool.timeInterval.rc = pCurrentUnit->timeInterval.rc;
                interbool.constValue.Set(true, true);
                pResult->MergeAdd(interbool);
              }
            }
          }
        }
      }
      j++;
    } // end while
    if (!found) {
        //no intersection found mgpoint not inside gline
      interbool.timeInterval.start = pCurrentUnit->timeInterval.start;
      interbool.timeInterval.lc = pCurrentUnit->timeInterval.lc;
      interbool.timeInterval.end = pCurrentUnit->timeInterval.end;
      interbool.timeInterval.rc = pCurrentUnit->timeInterval.rc;
      interbool.constValue.Set(true,false);
      pResult->MergeAdd(interbool);
    }
  } // end for
  pResult->EndBulkLoad();
  return 0;
};


/*
Specification of the operator

*/
const string OpTempNetInside::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint x gline -> mbool" "</text--->"
  "<text> _ at _ </text--->"
  "<text>Returns true for the times the mgpoint is at the gline."
    " False elsewhere.</text--->"
  "<text> X_MGPOINT inside GLINE</text--->"
  ") )";
