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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

1 Implementation of Algebra TemporalNet

May 2007 - October 2007 Martin Scheppokat

February 2008 - October 2008 Simone Jandt

Defines, includes, and constants

*/

#include "NestedList.h"
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "TupleIdentifier.h"
#include "DBArray.h"
#include "StandardTypes.h"
#include "NetworkAlgebra.h"
#include "TemporalAlgebra.h"
#include "TemporalNetAlgebra.h"
#include "NetworkManager.h"

#include <iostream>
#include <sstream>
#include <string>
#include "QueryProcessor.h"
#include "Algebra.h"
#include "DateTime.h"

extern NestedList* nl;
extern QueryProcessor* qp;


/*

1.1 Additional Methods

1.1.1 ~setMoveAndSide~

Sets parameter movingUp and side for the given Unit. Used by ~mpoint2mgpoint~.

*/

void setMoveAndSide(double startPos, double endPos, bool &MovingUp, bool dual,
                Side &side){
  if (MovingUp && startPos <= endPos) { MovingUp = true;}
  else {
    if (MovingUp && startPos > endPos) { MovingUp = false;}
    else {
      if (!MovingUp && startPos < endPos) { MovingUp = true;}
      else {
        if (!MovingUp && startPos >= endPos) { MovingUp = false;}
      }
    }
  }
  if (dual && MovingUp) { side = Up;}
  else {
    if (dual && !MovingUp) { side = Down;}
    else { side = None;}
  }
}

/*
1.1.2 ~getUnitValues~

Gets the parameter Values of the given Unit. Used by

*/

void getUnitValues(const UPoint *curUnit, Point &endPoint, Point &startPoint,
                   Instant &startTime, Instant &endTime,
                   double &lStartTime, double &lEndTime, double &duration)
{
  startPoint = curUnit->p0;
  endPoint = curUnit->p1;
  startTime = curUnit->timeInterval.start;
  lStartTime = startTime.ToDouble();
  endTime = curUnit->timeInterval.end;
  lEndTime = endTime.ToDouble();
  duration = lEndTime - lStartTime;

}

/*
1.1.3 ~checkPoint~

Checks sline value for the point and computes the position from the point on
the sline including difference value if not exactly on the sline. The problem
is caused by the gps value simulation which is mostly not exactly on the sline.
So that AtPosition and AtPoint could not find the point on the sline.
Returns true if the ~point~ is on the sline false elsewhere.
Used by ~mpoint2mgpoint~.

*/

bool checkPoint (SimpleLine *route, Point point, bool startSmaller,
                   double &pos, double &difference){
  bool result = false;
  const HalfSegment *hs;
  double k1, k2;
  Point left, right;
  for (int i = 0; i < route->Size()-1; i++) {
    route->Get(i, hs);
    left = hs->GetLeftPoint();
    right = hs->GetRightPoint();
    Coord xl = left.GetX(),
          yl = left.GetY(),
          xr = right.GetX(),
          yr = right.GetY(),
          x = point.GetX(),
          y = point.GetY();
    if ((fabs(x-xr) < 0.01 && fabs (y-yr) < 0.01) ||
       (fabs(x-xl) < 0.01 && fabs (y-yl) < 0.01)){
      difference = 0.0;
      result = true;
    } else {
      if (xl != xr && xl != x) {
        k1 = (y - yl) / (x - xl);
        k2 = (yr - yl) / (xr - xl);
        if ((fabs(k1-k2) < 0.01) &&
           ((xl < xr && (x > xl || fabs(x-xl) < 0.01) &&
           (x < xr || fabs(x-xr) < 0.01)) || (xl > xr &&  (x < xl ||
           fabs(x-xl)<0.01)  && ( x > xr || fabs(x-xr)))) && (((yl < yr ||
           fabs(yl-yr)<0.01) && (y > yl || fabs(y-yl)<0.01 )&& (y < yr ||
           fabs(y-yr)<0.01)) || (yl > yr && (y < yl || fabs(y-yl) <0.01) &&
           (y > yr || fabs(y-yr)<0.01)))) {
              difference = fabs(k1-k2);
              result = true;
        } else {result = false;}
      } else {
        if (( fabs(xl - xr) < 0.01 && fabs(xl -x) < 0.01) &&
           (((yl < yr|| fabs(yl-yr)<0.01) && (yl < y || fabs(yl-y) <0.01)&&
           (y < yr ||fabs(y-yr)<0.01))|| (yl > yr && (yl > y ||
           fabs(yl-y)<0.01)&& (y > yr ||fabs(y-yr)<0.01)))) {
              difference = 0.0;
              result = true;
        } else {result = false;}
      }
    }
    if (result) {
       const LRS *lrs;
        route->Get( hs->attr.edgeno, lrs );
        route->Get( lrs->hsPos, hs );
        pos = lrs->lrsPos + point.Distance( hs->GetDomPoint() );
        if( startSmaller != route->GetStartSmaller())
          pos = route->Length() - pos;
        if( fabs(pos-0.0) < 0.01)
          pos = 0.0;
        else if (fabs(pos-route->Length())<0.01)
              pos = route->Length();
        return result;

    }
  }
  return result;
}


/*
1.1.4 ~checkPoint03~

Almost Equal to ~checkPoint~ but allows bigger differences. Used by
~mpoint2mgpoint~.

*/

bool checkPoint03 (SimpleLine *route, Point point, bool startSmaller,
                   double &pos, double &difference){
  bool result = false;
  const HalfSegment *hs;
  double k1, k2;
  Point left, right;
  for (int i = 0; i < route->Size()-1; i++) {
    route->Get(i, hs);
    left = hs->GetLeftPoint();
    right = hs->GetRightPoint();
    Coord xl = left.GetX(),
          yl = left.GetY(),
          xr = right.GetX(),
          yr = right.GetY(),
          x = point.GetX(),
          y = point.GetY();
    if ((fabs(x-xl) < 0.01 && fabs (y-yl) < 0.01) ||
        (fabs(x-xr) < 0.01 && fabs (y-yr) < 0.01)) {
      difference = 0.0;
      result = true;
    } else {
      if (xl != xr && xl != x) {
        k1 = (y - yl) / (x - xl);
        k2 = (yr - yl) / (xr - xl);
        if ((fabs(k1-k2) < 1.2) &&
           ((xl < xr && (x > xl || fabs(x-xl) < 0.01) &&
           (x < xr || fabs(x-xr) < 0.01)) || (xl > xr &&  (x < xl ||
           fabs(x-xl)<0.01)  && ( x > xr || fabs(x-xr)))) && (((yl < yr ||
           fabs(yl-yr)<0.01) && (y > yl || fabs(y-yl)<0.01 )&& (y < yr ||
           fabs(y-yr)<0.01)) || (yl > yr && (y < yl || fabs(y-yl) <0.01) &&
           (y > yr || fabs(y-yr)<0.01)))) {
              difference = fabs(k1-k2);
              result = true;
        } else {result = false;}
      } else {
        if (( fabs(xl - xr) < 0.01 && fabs(xl -x) < 0.01) &&
           (((yl < yr|| fabs(yl-yr)<0.01) && (yl < y || fabs(yl-y) <0.01)&&
           (y < yr ||fabs(y-yr)<0.01))|| (yl > yr && (yl > y ||
           fabs(yl-y)<0.01)&& (y > yr ||fabs(y-yr)<0.01)))) {
              difference = 0.0;
              result = true;
        } else {result = false;}
      }
    }
    if (result) {
       const LRS *lrs;
        route->Get( hs->attr.edgeno, lrs );
        route->Get( lrs->hsPos, hs );
        pos = lrs->lrsPos + point.Distance( hs->GetDomPoint() );
        if( startSmaller != route->GetStartSmaller())
          pos = route->Length() - pos;
        if( fabs(pos-0.0) < 0.01)
          pos = 0.0;
        else if (fabs(pos-route->Length())<0.01)
              pos = route->Length();
        return result;
    }
  }
  return result;
}

bool lastcheckPoint03 (SimpleLine *route, Point point, bool startSmaller,
                   double &pos, double &difference){
  bool result = false;
  const HalfSegment *hs;
  double k1, k2;
  Point left, right;
  for (int i = 0; i < route->Size()-1; i++) {
    route->Get(i, hs);
    left = hs->GetLeftPoint();
    right = hs->GetRightPoint();
    Coord xl = left.GetX(),
          yl = left.GetY(),
          xr = right.GetX(),
          yr = right.GetY(),
          x = point.GetX(),
          y = point.GetY();
    if ((fabs(x-xl) < 0.01 && fabs (y-yl) < 0.01) ||
        (fabs(x-xr) < 0.01 && fabs (y-yr) < 0.01)) {
      difference = 0.0;
      result = true;
    } else {
      if (xl != xr && xl != x) {
        k1 = (y - yl) / (x - xl);
        k2 = (yr - yl) / (xr - xl);
        if (((xl < xr && (x > xl || fabs(x-xl) < 0.1) &&
           (x < xr || fabs(x-xr) < 0.01)) || (xl > xr &&  (x < xl ||
           fabs(x-xl)<0.01)  && ( x > xr || fabs(x-xr)))) && (((yl < yr ||
           fabs(yl-yr)<0.01) && (y > yl || fabs(y-yl)<0.01 )&& (y < yr ||
           fabs(y-yr)<0.01)) || (yl > yr && (y < yl || fabs(y-yl) <0.01) &&
           (y > yr || fabs(y-yr)<0.01)))) {
              difference = fabs(k1-k2);
              result = true;
        } else {result = false;}
      } else {
        if (( fabs(xl - xr) < 0.01 && fabs(xl -x) < 0.01) &&
           (((yl < yr|| fabs(yl-yr)<0.01) && (yl < y || fabs(yl-y) <0.01)&&
           (y < yr ||fabs(y-yr)<0.01))|| (yl > yr && (yl > y ||
           fabs(yl-y)<0.01)&& (y > yr ||fabs(y-yr)<0.01)))) {
              difference = 0.0;
              result = true;
        } else {result = false;}
      }
    }
    if (result) {
       const LRS *lrs;
        route->Get( hs->attr.edgeno, lrs );
        route->Get( lrs->hsPos, hs );
        pos = lrs->lrsPos + point.Distance( hs->GetDomPoint() );
        if( startSmaller != route->GetStartSmaller())
          pos = route->Length() - pos;
        if( pos < 0.0 || fabs(pos - 0.0) < 0.01)
          pos = 0.0;
        else if (pos > route->Length() || fabs(pos - route->Length()) < 0.01)
              pos = route->Length();
        return result;
    }
  }
  return result;
}

/*
1.1.5 ~searchUnit~

Returns true if there is a ~RouteInterval~ in the sorted ~GLine~ which
intersects the given ~RouteInterval~ false elsewhere. Used by operator
~passes~.

*/

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
          if (mid == 0) return false;
          else return searchUnit(pGLine, low, mid-1, unitRouteId, dMGPStart,
                            dMGPEnd);
        } else {
          if (rI->m_dStart > dMGPEnd) {
            if (mid == 0) return false;
            else return searchUnit(pGLine, low, mid-1, unitRouteId, dMGPStart,
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
1.1.6 ~checkEndOfUGPoint~

Used by operator ~inside~ to check the rest of the ~UGPoint~ if there is a
further intersection.

Two different parameter lists are possible.

*/
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

void checkEndOfUGPoint(double startPos, double endPos, Instant startTime,
                       bool bstart, Instant endTime, bool bend,
                       size_t actRoutInt, vector<RouteInterval> &vRI,
                       MBool* &pResult, int iRouteId){
  RouteInterval *pCurrInt;
  UBool interbool(true);
  double help, factor;
  bool swapped = false;
  if (endPos < startPos) {
    help = startPos;
    startPos = endPos;
    endPos = help;
    swapped = true;
  }
  bool found = false;
  Instant tInterStart, tInterEnd;
  size_t k = actRoutInt + 1;
  while ( k < vRI.size()) {
    pCurrInt = &vRI[k];
    if (pCurrInt->m_iRouteId == iRouteId) {
      if (!(endPos < pCurrInt->m_dStart || startPos > pCurrInt->m_dEnd ||
            startPos == endPos || pCurrInt->m_dStart == pCurrInt->m_dEnd)){
          //intersection exists compute intersecting part and timevalues for
          //resulting unit
        if (!swapped) {
          if (pCurrInt->m_dStart <= startPos) {
            found = true;
            interbool.timeInterval.start = startTime;
            interbool.timeInterval.lc = bstart;
            interbool.constValue.Set(true, true);
          } else {
            found = true;
            // compute and write unit befor mgpoint inside gline
            interbool.timeInterval.start = startTime;
            interbool.timeInterval.lc = bstart;
            factor = fabs(pCurrInt->m_dStart - startPos) /
                     fabs(endPos - startPos);
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
          if (pCurrInt->m_dEnd >= endPos) {
            interbool.timeInterval.end = endTime;
            interbool.timeInterval.rc = bend;
            pResult->MergeAdd(interbool);
          } else {
            // compute end of mgpoint at end of gline.and add result unit
            interbool.timeInterval.rc = true;
            factor = fabs(pCurrInt->m_dEnd - startPos) /
                     fabs(endPos -startPos);
            tInterEnd = (endTime -startTime) * factor + startTime;
            interbool.timeInterval.end = tInterEnd;
            pResult->MergeAdd(interbool);
            // the rest of the current unit is not in the current
            // routeinterval.
            checkEndOfUGPoint(pCurrInt->m_dEnd, endPos, tInterEnd, false,
                              endTime, bend, k, vRI, pResult, iRouteId);
          }
        } else {
          help = startPos;
          startPos = endPos;
          endPos = help;
          if (pCurrInt->m_dEnd >= startPos) {
            found = true;
            interbool.timeInterval.start = startTime;
            interbool.timeInterval.lc = bstart;
            interbool.constValue.Set(true, true);
          } else {
            found = true;
            // compute and write unit befor mgpoint inside gline
            interbool.timeInterval.start = startTime;
            interbool.timeInterval.lc = bstart;
            factor =  fabs(pCurrInt->m_dEnd - startPos) /
                      fabs(endPos - startPos);
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
          if (pCurrInt->m_dStart <= endPos) {
              interbool.timeInterval.end = endTime;
              interbool.timeInterval.rc = bend;
              pResult->MergeAdd(interbool);
          } else {
              // compute end of mgpoint at end of gline.and add result unit
              interbool.timeInterval.rc = true;
              factor = fabs(pCurrInt->m_dStart - startPos) /
                       fabs(endPos - startPos);
              tInterEnd = (endTime - startTime) * factor + startTime;
              interbool.timeInterval.end = tInterEnd;
              pResult->MergeAdd(interbool);
              // the rest of the current unit is not in the current
              // routeinterval.
              checkEndOfUGPoint(pCurrInt->m_dEnd, endPos, tInterEnd, false,
                                endTime, bend, k, vRI, pResult, iRouteId);
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
          if (pCurrInt->m_dStart == pCurrInt->m_dEnd) {
            found = true;
            if ((pCurrInt->m_dStart > startPos && pCurrInt->m_dStart < endPos)||
              (pCurrInt->m_dStart < startPos && pCurrInt->m_dStart > endPos)) {
              // compute and write unit befor mgpoint inside gline
              interbool.timeInterval.start = startTime;
              interbool.timeInterval.lc = bstart;
              factor = fabs(pCurrInt->m_dStart - startPos) /
                       fabs(endPos -startPos);
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
              checkEndOfUGPoint(pCurrInt->m_dEnd, endPos, tInterEnd, false,
                                endTime, bend, k, vRI, pResult, iRouteId);
            } else {
              if (pCurrInt->m_dStart == startPos && bstart) {
                found = true;
                interbool.timeInterval.start = startTime;
                interbool.timeInterval.lc = bstart;
                interbool.timeInterval.end = startTime;
                interbool.timeInterval.rc = true;
                interbool.constValue.Set(true,true);
                pResult->MergeAdd(interbool);
                checkEndOfUGPoint(pCurrInt->m_dEnd, endPos, tInterEnd, false,
                                  endTime, bend, k, vRI, pResult, iRouteId);
              } else {
                if (pCurrInt->m_dStart == endPos && bend) {
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
            if ((startPos == pCurrInt->m_dStart && endPos == pCurrInt->m_dEnd)||
               (startPos == pCurrInt->m_dEnd && endPos == pCurrInt->m_dStart)){
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
1.1.7 ~sendMessage~

Sending a message via the message-center

*/

void sendMessages(string in_strMessage)
{
  // Get message-center and initialize message-list
  static MessageCenter* xMessageCenter= MessageCenter::GetInstance();
  NList xMessage;
  xMessage.append(NList("error"));

  xMessage.append(NList().textAtom(in_strMessage));
  xMessageCenter->Send(xMessage);
}


/*
1.1.8 ~getRouteIntervals~

Returns the ~RouteIntervals~ of a route between mgpstart and mgpend.

Used by operators ~at~ and ~inside~.

*/

void getRouteIntervals(GLine *pGLine, int iRouteId, double mgpstart,
                       double mgpend, int low, int high,
                       vector<RouteInterval> &vRI){
  vRI.clear();
  const RouteInterval *aktRI;
  bool found;
  int mid;
  if (low <= high) {
    mid = (high + low) / 2;
    if  (!(mid < 0 || mid >= pGLine->NoOfComponents())) {
      pGLine->Get(mid, aktRI);
      if (aktRI->m_iRouteId < iRouteId) {
        getRouteIntervals(pGLine, iRouteId, mgpstart, mgpend, mid+1, high, vRI);
      } else {
        if (aktRI->m_iRouteId > iRouteId) {
          getRouteIntervals(pGLine, iRouteId, mgpstart, mgpend, low, mid-1,
                            vRI);
        } else {
          if (aktRI->m_dStart > mgpend) {
            getRouteIntervals(pGLine, iRouteId, mgpstart, mgpend, low, mid-1,
                              vRI);
          } else {
            if (aktRI->m_dEnd < mgpstart) {
              getRouteIntervals(pGLine, iRouteId, mgpstart, mgpend, mid+1, high,
                                vRI);
            } else {
              if (mid > 0) {
                mid--;
                found = false;
                while (mid >= 0 && !found) {
                  pGLine->Get(mid, aktRI);
                  if (aktRI->m_iRouteId == iRouteId &&
                      aktRI->m_dEnd >= mgpstart) {
                    mid--;
                  } else {
                    found = true;
                    mid++;
                    pGLine->Get(mid, aktRI);
                  }
                }
              } else {
                pGLine->Get(0, aktRI);
              }
             vRI.push_back(*aktRI);
              mid++;
              found = false;
              while (!found && mid < pGLine->NoOfComponents() ){
                pGLine->Get(mid, aktRI);
                if (aktRI->m_iRouteId == iRouteId &&
                    aktRI->m_dStart <= mgpend) {
                  vRI.push_back(*aktRI);
                  mid++;
                } else {
                  found = true;
                }
              }
            }
          }
        }
      }
    }
  }
}

/*
1.1.9 ~intervalCheck~

Compares two time intervals and returns  a case number describing their
intersection.

Used by ~refinementMovingGPoint~.

*/

int intervalCheck (Interval<Instant> i1, Interval<Instant> i2) {
  if (i1.start > i2.end) return 1;
  if (i1.end < i2.start ) return 16;
  if (i1.start > i2.start && i1.start == i2.end && i1.start < i1.end) return 2;
  if (i1.start > i2.start && i1.start < i2.end && i1.end < i2.end &&
      i1.start < i1.end) return 3;
  if (i1.start > i2.start && i1.end == i2.end && i1.start < i1.end) return 4;
  if (i1.start > i2.start && i1.end > i2.end && i2.end > i1.start &&
     i1.start < i1.end) return 5;
  if (i1.start == i2.start && i1.start == i2.end && i1.start < i1.end) return 9;
  if (i1.start == i2.start && i1.end > i2.end && i2.start < i2.end &&
     i1.start < i1.end) return 6;
  if (i1.start == i2.start && i1.end == i2.end && i1.start < i1.end) return 7;
  if (i1.start == i2.start && i1.end < i2.end && i1.start < i1.end) return 8;
  if (i1.start < i2.start && i1.end > i2.end && i2.start == i2.end &&
     i1.start < i1.end) return 13;
  if (i1.start < i2.start && i1.end > i2.end && i2.start < i2.end &&
     i1.start < i1.end) return 10;
  if (i1.start < i2.start && i1.end == i2.end && i2.start < i2.end &&
     i1.start < i1.end) return 11;
  if (i1.start < i2.start && i1.end < i2.end && i1.end > i2.start &&
     i1.start < i1.end) return 12;
  if (i1.end == i2.start && i2.start == i2.end && i1.start < i1.end) return 14;
  if (i1.end == i2.start && i1.end < i2.end && i2.start < i2.end &&
     i1.start < i1.end) return 15;
  if (i1.start == i1.end && i2.start == i2.end &&
      i1.start == i2.start) return 19;
  if (i1.start == i1.end && i1.start == i2.start &&
      i2.start < i2.end) return 17;
  if (i1.start == i1.end && i1.start == i2.end && i2.start < i2.end) return 18;
  if (i1.start == i1.end && i1.start > i2.start && i1.start < i2.end) return 20;
  return -1; //should never be reached
};

/*
1.1.10 ~refinementMovingGPoint~

Translates two given ~MGPoint~ to two ~MGPoint~ which have a identical number
of units with identical time intervals. Used by operator ~intersection~ and
later on ~distance~ and ~netdistance~

*/

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
                                             ua->p0.GetPosition())/*,
                                             false*/);
          resB->Add(UGPoint(Interval<Instant>(ub->timeInterval.end,
                                             ub->timeInterval.end,
                                             true, true),
                                             ub->p1.GetNetworkId(),
                                             ub->p1.GetRouteId(),
                                             ub->p1.GetSide(),
                                             ub->p1.GetPosition(),
                                             ub->p1.GetPosition())/*,
                                             false*/);
        }
        j++;
        if (j < b->GetNoComponents()) b->Get(j, ub);
        break;
      case 3:
        ub->TemporalFunction(ua->timeInterval.start, pos1, false);
        ub->TemporalFunction(ua->timeInterval.end, pos2, false);
        resA->Add(UGPoint(ua->timeInterval,
                                             ua->p0.GetNetworkId(),
                                             ua->p0.GetRouteId(),
                                             ua->p0.GetSide(),
                                             ua->p0.GetPosition(),
                                             ua->p1.GetPosition())/*,
                                             false*/);
        resB->Add(UGPoint(ua->timeInterval,
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             pos1.GetPosition(),
                                             pos2.GetPosition())/*,
                                             false*/);
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
                                             ua->p1.GetPosition())/*,
                                             false*/);
        resB->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                           ua->timeInterval.end,
                                           ua->timeInterval.lc,
                                           ua->timeInterval.rc &&
                                           ub->timeInterval.rc),
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             pos2.GetPosition(),
                                             ub->p1.GetPosition())/*,
                                            false*/);
        if (ua->timeInterval.rc && ub->timeInterval.rc ||
            (!ua->timeInterval.rc && !ub->timeInterval.rc)) {
          i++;
          if (i < a->GetNoComponents()) a->Get(i, ua);
          j++;
          if (j < b->GetNoComponents()) b->Get(j, ub);
        } else {
          if (ua->timeInterval.rc && !ub->timeInterval.rc) {
            j++;
            if (j < b->GetNoComponents()) b->Get(j, ub);
          }
          if (ub->timeInterval.rc && !ua->timeInterval.rc) {
            i++;
            if (i < a->GetNoComponents()) a->Get(i, ua);
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
                                            pos1.GetPosition())/*,
                                            false*/);
        resB->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                             ub->timeInterval.end,
                                             ua->timeInterval.lc,
                                             ub->timeInterval.rc),
                                            ub->p0.GetNetworkId(),
                                            ub->p0.GetRouteId(),
                                            ub->p0.GetSide(),
                                            pos2.GetPosition(),
                                            ub->p1.GetPosition())/*,
                                            false*/);
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
                                             pos1.GetPosition())/*,
                                            false*/);
        resB->Add(UGPoint(Interval<Instant>(ub->timeInterval.start,
                                           ub->timeInterval.end,
                                           ub->timeInterval.lc &&
                                           ua->timeInterval.lc,
                                           ub->timeInterval.rc),
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             ub->p0.GetPosition(),
                                             ub->p1.GetPosition())/*,
                                          false*/);
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
                                             ua->p1.GetPosition())/*,
                                            false*/);
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
                                             ub->p1.GetPosition())/*,
                                            false*/);
        if (ua->timeInterval.rc && ub->timeInterval.rc ||
            (!ua->timeInterval.rc && !ub->timeInterval.rc)) {
          i++;
          if (i < a->GetNoComponents()) a->Get(i, ua);
          j++;
          if (j < b->GetNoComponents()) b->Get(j, ub);
        } else {
          if (ua->timeInterval.rc && !ub->timeInterval.rc) {
            j++;
            if (j < b->GetNoComponents()) b->Get(j, ub);
          }
          if (ub->timeInterval.rc && !ua->timeInterval.rc) {
            i++;
            if (i < a->GetNoComponents()) a->Get(i, ua);
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
                                             ua->p1.GetPosition())/*,
                                            false*/);
        resB->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                           ua->timeInterval.end,
                                           ua->timeInterval.lc &&
                                           ub->timeInterval.lc,
                                           ua->timeInterval.rc),
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             ub->p0.GetPosition(),
                                             pos2.GetPosition())/*,
                                            false*/);
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
                                             ua->p0.GetPosition())/*,
                                            false*/);
          resB->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                             ua->timeInterval.start,
                                             true, true),
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             ub->p0.GetPosition(),
                                             ub->p0.GetPosition())/*,
                                             false*/);
        }
        j++;
        if (j < b->GetNoComponents()) b->Get(j, ub);
        break;
      case 10:
        ua->TemporalFunction(ub->timeInterval.start, pos1, false);
        ua->TemporalFunction(ub->timeInterval.end, pos2, false);
        resA->Add(UGPoint(ub->timeInterval,
                                            ua->p0.GetNetworkId(),
                                            ua->p0.GetRouteId(),
                                            ua->p0.GetSide(),
                                            pos1.GetPosition(),
                                            pos2.GetPosition())/*, false*/);
        resB->Add(UGPoint(ub->timeInterval,
                                            ub->p0.GetNetworkId(),
                                            ub->p0.GetRouteId(),
                                            ub->p0.GetSide(),
                                            ub->p0.GetPosition(),
                                            ub->p1.GetPosition())/*,
                                            false*/);
        j++;
        if (j < b->GetNoComponents()) b->Get(j, ub);
        break;
      case 11:
        ua->TemporalFunction(ub->timeInterval.start, pos1, false);
        resA->Add(UGPoint(ub->timeInterval,
                                             ua->p0.GetNetworkId(),
                                             ua->p0.GetRouteId(),
                                             ua->p0.GetSide(),
                                             pos1.GetPosition(),
                                             ua->p1.GetPosition())/*,
                                             false*/);
        resB->Add(UGPoint(ub->timeInterval,
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             ub->p0.GetPosition(),
                                             ub->p1.GetPosition())/*,
                                             false*/);
        if (ua->timeInterval.rc && ub->timeInterval.rc ||
            (!ua->timeInterval.rc && !ub->timeInterval.rc)) {
          i++;
          if (i < a->GetNoComponents()) a->Get(i, ua);
          j++;
          if (j < b->GetNoComponents()) b->Get(j, ub);
        } else {
          if (ua->timeInterval.rc && !ub->timeInterval.rc) {
            j++;
            if (j < b->GetNoComponents()) b->Get(j, ub);
          }
          if (ub->timeInterval.rc && !ua->timeInterval.rc) {
            i++;
            if (i < a->GetNoComponents()) a->Get(i, ua);
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
                                             ua->p1.GetPosition())/*,
                                            false*/);
        resB->Add(UGPoint(Interval<Instant>(ub->timeInterval.start,
                                           ua->timeInterval.end,
                                           ub->timeInterval.lc,
                                           ua->timeInterval.rc),
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             ub->p0.GetPosition(),
                                             pos2.GetPosition())/*,
                                            false*/);
        i++;
        if (i < a->GetNoComponents()) a->Get(i, ua);
        break;
      case 13:
        if (ub->timeInterval.lc && ub->timeInterval.rc) {
          ua->TemporalFunction(ub->timeInterval.start, pos1, false);
          resA->Add(UGPoint(ub->timeInterval,
                                             ua->p0.GetNetworkId(),
                                             ua->p0.GetRouteId(),
                                             ua->p0.GetSide(),
                                             pos1.GetPosition(),
                                             pos1.GetPosition())/*, false*/);
          resB->Add(UGPoint(ub->timeInterval,
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             ub->p0.GetPosition(),
                                             ub->p0.GetPosition())/*,
                                              false*/);
        }
        j++;
        if (j < b->GetNoComponents()) b->Get(j, ub);
        break;
      case 14:
        if (ua->timeInterval.rc && ub->timeInterval.lc && ub->timeInterval.rc) {
          resA->Add(UGPoint(ub->timeInterval, ua->p1, ua->p1)/*, false*/);
          resB->Add(UGPoint(ub->timeInterval, ub->p0, ub->p0)/*, false*/);
        }
        if (ua->timeInterval.rc && ub->timeInterval.rc ||
            (!ua->timeInterval.rc && !ub->timeInterval.rc)) {
          i++;
          if (i < a->GetNoComponents()) a->Get(i, ua);
          j++;
          if (j < b->GetNoComponents()) b->Get(j, ub);
        } else {
          if (ua->timeInterval.rc && !ub->timeInterval.rc) {
            j++;
            if (j < b->GetNoComponents()) b->Get(j, ub);
          }
          if (ub->timeInterval.rc && !ua->timeInterval.rc) {
            i++;
            if (i < a->GetNoComponents()) a->Get(i, ua);
          }
        }
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
                                             ua->p1.GetPosition())/*,
                                              false*/);
          resB->Add(UGPoint(Interval<Instant>(ua->timeInterval.end,
                                              ua->timeInterval.end,
                                              true, true),
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             ub->p0.GetPosition(),
                                             ub->p0.GetPosition())/*,
                                            false*/);
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
                                             ua->p0.GetPosition())/*,
                                            false*/);
          resB->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                             ua->timeInterval.end,
                                             true, true),
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             ub->p0.GetPosition(),
                                             ub->p0.GetPosition())/*,
                                            false*/);
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
                                             ua->p0.GetPosition())/*,
                                            false*/);
          resB->Add(UGPoint(Interval<Instant>(ua->timeInterval.start,
                                             ua->timeInterval.end,
                                             true, true),
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             ub->p1.GetPosition(),
                                             ub->p1.GetPosition())/*,
                                            false*/);
        }
        if (ua->timeInterval.rc && ub->timeInterval.rc ||
            (!ua->timeInterval.rc && !ub->timeInterval.rc)) {
          i++;
          if (i < a->GetNoComponents()) a->Get(i, ua);
          j++;
          if (j < b->GetNoComponents()) b->Get(j, ub);
        } else {
          if (ua->timeInterval.rc && !ub->timeInterval.rc) {
            j++;
            if (j < b->GetNoComponents()) b->Get(j, ub);
          }
          if (ub->timeInterval.rc && !ua->timeInterval.rc) {
            i++;
            if (i < a->GetNoComponents()) a->Get(i, ua);
          }
        }
        break;
      case 19:
        if (ua->timeInterval.lc && ua->timeInterval.rc &&
           ub->timeInterval.lc && ub->timeInterval.rc) {
          resA->Add(UGPoint(Interval<Instant> (ua->timeInterval.start,
                                              ua->timeInterval.start,
                                              true,true),
                                              ua->p0.GetNetworkId(),
                                              ua->p0.GetRouteId(),
                                              ua->p0.GetSide(),
                                              ua->p0.GetPosition(),
                                              ua->p0.GetPosition())/*,
                                              false*/);
          resB->Add(UGPoint(Interval<Instant> (ub->timeInterval.end,
                                              ub->timeInterval.end,
                                              true,true),
                                              ub->p0.GetNetworkId(),
                                              ub->p0.GetRouteId(),
                                              ub->p0.GetSide(),
                                              ub->p0.GetPosition(),
                                              ub->p0.GetPosition())/*,
                                              false*/);
        }
        if (ua->timeInterval.rc && ub->timeInterval.rc ||
            (!ua->timeInterval.rc && !ub->timeInterval.rc)) {
          i++;
          if (i < a->GetNoComponents()) a->Get(i, ua);
          j++;
          if (j < b->GetNoComponents()) b->Get(j, ub);
        } else {
          if (ua->timeInterval.rc && !ub->timeInterval.rc) {
            j++;
            if (j < b->GetNoComponents()) b->Get(j, ub);
          }
          if (ub->timeInterval.rc && !ua->timeInterval.rc) {
            i++;
            if (i < a->GetNoComponents()) a->Get(i, ua);
          }
        }
        break;
      case 20:
        if (ua->timeInterval.lc && ua->timeInterval.rc) {
          ub->TemporalFunction(ua->timeInterval.start, pos2, false);
          resA->Add(UGPoint(ua->timeInterval,
                                             ua->p0.GetNetworkId(),
                                             ua->p0.GetRouteId(),
                                             ua->p0.GetSide(),
                                             ua->p0.GetPosition(),
                                             ua->p0.GetPosition())/*,
                                            false*/);
          resB->Add(UGPoint(ua->timeInterval,
                                             ub->p0.GetNetworkId(),
                                             ub->p0.GetRouteId(),
                                             ub->p0.GetSide(),
                                             pos2.GetPosition(),
                                             pos2.GetPosition())/*,
                                            false*/);
        }
        i++;
        if (i < a->GetNoComponents()) a->Get(i, ua);
        break;
      default: //should never happen
        cerr << "an error occured while checking the time interval." << endl;
        resA->EndBulkLoad(false/*, false*/);
        resB->EndBulkLoad(false/*, false*/);
        resA->SetDefined(false);
        resB->SetDefined(false);
        return;
    } // end switch
  }//end while
  resA->EndBulkLoad(true/*, false, false*/);
  resB->EndBulkLoad(true/*, false, false*/);
  resA->SetDefined(true);
  resB->SetDefined(true);
  return;
}

/*
1.1.12 ~chkStarEnd~

Checks if StartPos <= EndPos if not changes StartPos and EndPos. Used by
operator ~trajectory~.

*/

void chkStartEnd(double &StartPos, double &EndPos){
  double help;
  if (StartPos > EndPos) {
    help = StartPos;
    StartPos = EndPos;
    EndPos = help;
  }
};

/*
1.2 Class definitions

1.2.1 class ~MGPoint~

Constructors
The simple constructor should not be used.

*/

MGPoint::MGPoint( const int n ) : Mapping< UGPoint, GPoint >( n )
{
  /*m_bbox = Rectangle<3>(false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  m_length = 0.0;
  m_trajectory.SetDefined(false);*/
}

/*
Functions for integration in SECONDO

*/
void MGPoint::Clear()
{
  Mapping<UGPoint, GPoint>::Clear();
  /*m_bbox.SetDefined(false);
  m_length = 0.0;
  m_trajectory.Clear();*/
}

ListExpr MGPoint::Property()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> MAPPING"),
                             nl->StringAtom("(mgpoint) "),
                             nl->StringAtom("( u1 ... un ) "),
                             nl->StringAtom("(((i1 i2 TRUE FALSE) "
                                            "(1 1 0 0.1 2.4)) ...)"))));
}

bool MGPoint::Check(ListExpr type,
                    ListExpr& errorInfo)
{
  return (nl->IsEqual( type, "mgpoint" ));
}

/*
Get Methods

*/

int MGPoint::GetNetworkId() {
  const UGPoint *u;
  Get(0,u);
  return u->p0.GetNetworkId();
}


double MGPoint::GetLength() const{
  double res = 0.0;
  const UGPoint *pCurrUnit;
  for (int i = 0 ; i < GetNoComponents(); i++){
    Get(i,pCurrUnit);
    res += fabs(pCurrUnit->p1.GetPosition() - pCurrUnit->p0.GetPosition());
  }
  return res;
}

double MGPoint::Length(){
  double res = 0.0;
  const UGPoint *pCurrUnit;
  for (int i = 0 ; i < GetNoComponents(); i++){
    Get(i,pCurrUnit);
    res += fabs(pCurrUnit->p1.GetPosition() - pCurrUnit->p0.GetPosition());
  }
  return res;
}

void MGPoint::Trajectory(GLine *res){
  /*(*res) = m_trajectory;
  cout << "MGPTraj: " << *res << endl;*/
  const UGPoint *pCurrentUnit;
  Get(0, pCurrentUnit);
  int iNetworkId = pCurrentUnit->p0.GetNetworkId();
  res->SetNetworkId(iNetworkId);
  res->SetDefined(true);
  int aktRouteId = pCurrentUnit->p0.GetRouteId();
  double aktStartPos = pCurrentUnit->p0.GetPosition();
  double aktEndPos = pCurrentUnit->p1.GetPosition();
  chkStartEnd(aktStartPos, aktEndPos);
  RITree *tree;
  tree = new RITree(aktRouteId, aktStartPos, aktEndPos,0,0);
  int curRoute;
  double curStartPos, curEndPos;
  for (int i = 1; i < GetNoComponents(); i++)
  {
    // Get start and end of current unit
    Get(i, pCurrentUnit);
    curRoute = pCurrentUnit->p0.GetRouteId();
    curStartPos = pCurrentUnit->p0.GetPosition();
    curEndPos = pCurrentUnit->p1.GetPosition();
    chkStartEnd(curStartPos, curEndPos);
    if (curRoute != aktRouteId) {
      tree->Insert(aktRouteId, aktStartPos, aktEndPos);
      aktRouteId = curRoute;
      aktStartPos = curStartPos;
      aktEndPos = curEndPos;
    } else { // curRoute == aktRouteId concat pieces if possible
      if (AlmostEqual(aktStartPos,curEndPos)) {
        aktStartPos = curStartPos;
      } else {
        if (AlmostEqual(aktEndPos,curStartPos)) {
          aktEndPos = curEndPos;
        } else { //concat impossible start new routeInterval for gline.
          tree->Insert(aktRouteId, aktStartPos, aktEndPos);
          aktRouteId = curRoute;
          aktStartPos = curStartPos;
          aktEndPos = curEndPos;
        }
      }
    }
  }
  tree->Insert(aktRouteId, aktStartPos, aktEndPos);
  tree->TreeToGLine(res);
  tree->RemoveTree();
  res->SetSorted(true);
}

void MGPoint::Deftime(Periods *res){
  Periods per(GetNoComponents());
  const UGPoint *unit;
  per.StartBulkLoad();
  for( int i = 0; i < GetNoComponents(); i++ ) {
    Get( i, unit );
    if (unit->IsDefined())
      per.Add(unit->timeInterval);
  }
  per.EndBulkLoad(false);
  per.Merge(*res);
}


/*
Operations with ~mgpoint~

Euclidean Distance computing

*/

void MGPoint::Distance(MGPoint *mgp, MReal *result){
  UReal uReal(true);
  RefinementPartition<MGPoint, MGPoint, UGPoint, UGPoint>
      rp(*this, *mgp);
  result->StartBulkLoad();
  for( unsigned int i = 0; i < rp.Size(); i++ )
  {
    Interval<Instant>* iv;
    int u1Pos, u2Pos;
    const UGPoint *u1;
    const UGPoint *u2;

    rp.Get(i, iv, u1Pos, u2Pos);
    if (u1Pos == -1 || u2Pos == -1)
      continue;
    else {
      Get(u1Pos, u1);
      mgp->Get(u2Pos, u2);
    }
    if(u1->IsDefined() && u2->IsDefined()) {
       // do not need to test for overlapping deftimes anymore...
      if (u1->p0.GetRouteId() == u2->p0.GetRouteId()) {
          //there might be intersecting units.
          GPoint u1gstart, u1gend, u2gstart, u2gend;
          Interval<Instant> interv;
          u1->timeInterval.Intersection(u2->timeInterval, interv);
          u1->TemporalFunction(interv.start, u1gstart, true);
          u1->TemporalFunction(interv.end, u1gend, true);
          u2->TemporalFunction(interv.start, u2gstart, true);
          u2->TemporalFunction(interv.end, u2gend, true);
          if ((u1gstart.GetPosition() < u2gstart.GetPosition() &&
              u1gend.GetPosition() > u2gstart.GetPosition()) ||
              (u2gstart.GetPosition() < u1gstart.GetPosition() &&
              u2gend.GetPosition() > u1gstart.GetPosition()) ||
              (u2gstart.GetPosition() < u1gend.GetPosition() &&
              u2gend.GetPosition() > u1gend.GetPosition()) ||
              (u1gstart.GetPosition() < u2gend.GetPosition() &&
              u1gend.GetPosition() > u2gend.GetPosition()) ||
              (u1gstart.GetPosition() > u2gstart.GetPosition() &&
              u1gend.GetPosition() < u2gstart.GetPosition()) ||
              (u2gstart.GetPosition() > u1gstart.GetPosition() &&
              u2gend.GetPosition() < u1gstart.GetPosition()) ||
              (u2gstart.GetPosition() > u1gend.GetPosition() &&
              u2gend.GetPosition() < u1gend.GetPosition()) ||
              (u1gstart.GetPosition() > u2gend.GetPosition() &&
              u1gend.GetPosition() < u2gend.GetPosition())){
            //intersecting units. Intersectionpoint must be
            //computed and units be devided up at the time of
            //intersection.
            Instant tinter = (interv.end - interv.start) *
                  ((u2gstart.GetPosition() - u1gstart.GetPosition()) /
                    (u1gend.GetPosition() - u1gstart.GetPosition() -
                    u2gend.GetPosition() + u2gstart.GetPosition())) +
                  interv.start;
            if (interv.start <= tinter && tinter <= interv.end) {
              double interPosition = u1gstart.GetPosition() +
                      ((u1gend.GetPosition() - u1gstart.GetPosition()) *
                      ((tinter.ToDouble() - interv.start.ToDouble())
                      / (interv.end.ToDouble() - interv.start.ToDouble())));
              if (((interPosition >= u1gstart.GetPosition() &&
                  interPosition <= u1gend.GetPosition()) ||
                  (interPosition >= u1gend.GetPosition() &&
                   interPosition <= u1gend.GetPosition())) &&
                  ((interPosition >= u2gstart.GetPosition() &&
                  interPosition <= u2gend.GetPosition()) ||
                  (interPosition >= u2gend.GetPosition() &&
                   interPosition <= u2gend.GetPosition()))) {
                UGPoint u1a = UGPoint(Interval<Instant> (interv.start,
                                                      tinter,
                                                      true, false),
                                                  u1gstart.GetNetworkId(),
                                                  u1gstart.GetRouteId(),
                                                  u1gstart.GetSide(),
                                                  u1gstart.GetPosition(),
                                                  interPosition);
                UGPoint u1b = UGPoint(Interval<Instant> (tinter,
                                                  interv.end,
                                                      true, false),
                                                  u1gstart.GetNetworkId(),
                                                  u1gstart.GetRouteId(),
                                                  u1gstart.GetSide(),
                                                  interPosition,
                                                  u1gend.GetPosition());
                UGPoint u2a = UGPoint(Interval<Instant> (interv.start,
                                                      tinter,
                                                      true, false),
                                                  u2gstart.GetNetworkId(),
                                                  u2gstart.GetRouteId(),
                                                  u2gstart.GetSide(),
                                                  u2gstart.GetPosition(),
                                                  interPosition);
                UGPoint u2b = UGPoint(Interval<Instant> (tinter,
                                                       interv.end,
                                                       true, false),
                                                  u2gstart.GetNetworkId(),
                                                  u2gstart.GetRouteId(),
                                                  u2gstart.GetSide(),
                                                  interPosition,
                                                  u2gend.GetPosition());
                u1a.Distance(u2a, uReal);
                result->MergeAdd(uReal);
                u1b.Distance(u2b, uReal);
                result->MergeAdd(uReal);
              } else {
                u1->Distance( *u2, uReal );
                result->MergeAdd( uReal );
              }
            } else {
              u1->Distance( *u2, uReal );
              result->MergeAdd( uReal );
            }
          } else {
            u1->Distance( *u2, uReal );
            result->MergeAdd( uReal );
          }
      } else {
        u1->Distance( *u2, uReal );
        result->MergeAdd( uReal );
      }
    }
  }
  result->EndBulkLoad();
}

/*
Translation from network ~mgpoint~ to spatial ~mpoint~

*/

void MGPoint::Mgpoint2mpoint(MPoint *mp) {
  if (IsDefined() && !IsEmpty()){
  Network* pNetwork = NetworkManager::GetNetwork(GetNetworkId());
  const UGPoint *pCurrUnit;
  UGPoint CurrUnit;
  int iAktRouteId = 1;
  int lrsposakt, lrsposnext;
  Tuple *pRoute = pNetwork->GetRoute(1);
  SimpleLine *pRouteCurve = (SimpleLine*) pRoute->GetAttribute(ROUTE_CURVE);
  Instant correcture(0,1,durationtype);
  bool bendcorrecture = false;
  Instant tStart, tEnd, tInter1, tInter2, tEndNew;
  Point pStart = new Point(false);
  Point pEnd = new Point(false);
  const LRS *lrsAkt, *lrsNext;
  const HalfSegment *hs;
  mp->Clear();
  mp->StartBulkLoad();
  for (int i = 0 ; i < GetNoComponents(); i++) {
    Get(i, pCurrUnit);
    CurrUnit = *pCurrUnit;
    tStart = pCurrUnit->timeInterval.start;
    tEnd = pCurrUnit->timeInterval.end;
    if (bendcorrecture) {
      bendcorrecture = false;
      if (tStart <= tEndNew) {
        tStart = tEndNew;
        if (tStart >= tEnd ) {
          tEnd = tStart + correcture;
          bendcorrecture = true;
          tEndNew = tEnd;
        }
      }
    }
    if (pCurrUnit->p0.GetRouteId() != iAktRouteId || i == 0) {
      pRoute->DeleteIfAllowed();
      iAktRouteId = pCurrUnit->p0.GetRouteId();
      pRoute = pNetwork->GetRoute(iAktRouteId);
      pRouteCurve = (SimpleLine*) pRoute->GetAttribute(ROUTE_CURVE);
      LRS lrs(pCurrUnit->p0.GetPosition(),0);
      if (!pRouteCurve->Get(lrs, lrsposakt)) {
        cerr << "not found on route" << endl;
        break;
      }
      pRouteCurve->Get( lrsposakt, lrsAkt );
      pRouteCurve->Get( lrsAkt->hsPos, hs );
      pStart = hs->AtPosition(pCurrUnit->p0.GetPosition() - lrsAkt->lrsPos);
     }
    if (pCurrUnit->p0.GetPosition() == pCurrUnit->p1.GetPosition()){
       mp->Add(UPoint(Interval<Instant> (tStart, tEnd,
              pCurrUnit->timeInterval.lc, pCurrUnit->timeInterval.rc),
              pStart, pStart));
    } else {
      if (pCurrUnit->p0.GetPosition() < pCurrUnit->p1.GetPosition()){
        tInter1 = tStart;
        lrsposnext = lrsposakt + 1;
        if (lrsposnext <= (pRouteCurve->Size()/2)-1){
          pRouteCurve->Get(lrsposnext, lrsNext);
          if (lrsNext->lrsPos >= pCurrUnit->p1.GetPosition()) {
            pEnd = hs->AtPosition(pCurrUnit->p1.GetPosition() - lrsAkt->lrsPos);
            mp->Add(UPoint(Interval<Instant> (tInter1, tEnd,
                    pCurrUnit->timeInterval.lc, pCurrUnit->timeInterval.rc),
                    pStart, pEnd));
            pStart = pEnd;
          } else {
            while (lrsNext->lrsPos <= pCurrUnit->p1.GetPosition() &&
                   lrsposnext < (pRouteCurve->Size()/2)-1){
              tInter2 = CurrUnit.TimeAtPos(lrsNext->lrsPos);
              pRouteCurve->Get(lrsNext->hsPos, hs);
              pEnd = hs->AtPosition(0);
              if (tInter1 >= tInter2) {
                tInter2 = tInter1 + correcture;
                bendcorrecture = true;
                tEndNew = tInter2;
              }
              mp->Add(UPoint(Interval<Instant> (tInter1, tInter2,
                                                true, false),
                                                pStart, pEnd));
              tInter1 = tInter2;
              pStart = pEnd;
              lrsposakt = lrsposnext;
              pRouteCurve->Get(lrsposakt, lrsAkt);
              lrsposnext = lrsposakt +1;
              if (lrsposnext <= (pRouteCurve->Size()/2) - 1)
                pRouteCurve->Get(lrsposnext, lrsNext);
            }
            pEnd = hs->AtPosition(pCurrUnit->p1.GetPosition() - lrsAkt->lrsPos);
            if (tInter1 >= tEnd) {
              tEnd = tInter1 + correcture;
              bendcorrecture = true;
              tEndNew = tEnd;
            }
            mp->Add(UPoint(Interval<Instant>(tInter1, tEnd, true, false),
                          pStart, pEnd));
            pStart = pEnd;
          }
        } else {
          pEnd = hs->AtPosition(pCurrUnit->p1.GetPosition() - lrsAkt->lrsPos);
          if (tInter1 >= tEnd) {
            tEnd = tInter1 + correcture;
            bendcorrecture = true;
            tEndNew = tEnd;
          }
          mp->Add(UPoint(Interval<Instant>(tInter1, tEnd, true, false),
                          pStart, pEnd));
          pStart = pEnd;
        }
      } else {
        if (lrsAkt->lrsPos <= pCurrUnit->p1.GetPosition()) {
            pEnd = hs->AtPosition(pCurrUnit->p1.GetPosition() - lrsAkt->lrsPos);
            mp->Add(UPoint(Interval<Instant> (tStart, tEnd,
                    pCurrUnit->timeInterval.lc, pCurrUnit->timeInterval.rc),
                    pStart, pEnd));
            pStart = pEnd;
        } else {
          tInter1 = tStart;
          while (lrsAkt->lrsPos > pCurrUnit->p1.GetPosition() &&
                lrsposakt >= 0){
            pEnd = hs->AtPosition(0);
            tInter2 = CurrUnit.TimeAtPos(lrsAkt->lrsPos);
            if (tInter1 >= tInter2) {
              tInter2 = tInter1 + correcture;
              bendcorrecture = true;
              tEndNew = tInter2;
            }
            mp->Add(UPoint(Interval<Instant> (tInter1,tInter2,true, false),
                pStart, pEnd));
            tInter1 = tInter2;
            pStart = pEnd;
            lrsposakt = lrsposakt - 1;
            if (lrsposakt >= 0) {
              pRouteCurve->Get(lrsposakt, lrsAkt);
              pRouteCurve->Get(lrsAkt->hsPos, hs);
            }
          }
          pEnd = hs->AtPosition(pCurrUnit->p1.GetPosition() - lrsAkt->lrsPos);
          if (tInter1 >= tEnd) {
            tEnd = tInter1 + correcture;
            bendcorrecture = true;
            tEndNew = tEnd;
          }
          mp->Add(UPoint(Interval<Instant> (tInter1, tEnd,
                    pCurrUnit->timeInterval.lc, pCurrUnit->timeInterval.rc),
                    pStart, pEnd));
          pStart = pEnd;
        }
      }
    }
  }
  mp->EndBulkLoad();
  pRoute->DeleteIfAllowed();
  NetworkManager::CloseNetwork(pNetwork);
  } else mp->SetDefined(false);
}

/*
Checks if ~mgpoint~ is present in the given ~periods~

*/

bool MGPoint::Present(Periods *per) {
  const Interval<Instant> *intper;
  const UGPoint *pCurrUnit;
  int j = 0;
  int mid, first, last;
  while (j < per->GetNoComponents()) {
    per->Get( j, intper );
    first = 0;
    last = GetNoComponents() -1;
    Get(first, pCurrUnit);
    if(!(intper->Before(pCurrUnit->timeInterval))){
      Get(last, pCurrUnit);
      if (!pCurrUnit->timeInterval.Before(*intper)){
        while (first <= last) {
          mid = (first + last) /2;
          if (mid < 0 || mid >= GetNoComponents()){
            break;
          }
          Get( mid, pCurrUnit );
          if (pCurrUnit->timeInterval.Before(*intper)) first = mid + 1;
          else {
            if (intper->Before(pCurrUnit->timeInterval)) last = mid - 1;
            else {
              return true;
            }
          }
        }
      }
    }
    j++;
  }
  return false;
}

bool MGPoint::Present(Instant *per) {
  //Periods *deft = new Periods(0);
  //Deftime(deft);
  int mid;
  int first = 0;
  int last = GetNoComponents() - 1;
  const UGPoint *pCurrUnit;
  Get(first, pCurrUnit);
  if (pCurrUnit->timeInterval.start.CompareTo(per) >0) return false;
  Get(last, pCurrUnit);
  if (pCurrUnit->timeInterval.end.CompareTo(per) < 0) return false;
  while (first <= last) {
    mid = (first+last)/2;
    if (mid<0 || mid >= GetNoComponents()) return false;
    Get(mid, pCurrUnit);
    if (pCurrUnit->timeInterval.end.CompareTo(per) < 0) first = mid + 1;
    else
      if (pCurrUnit->timeInterval.start.CompareTo(per) > 0 ) last = mid - 1;
      else return true;
  }
  return false;
}

/*
Computes the intersection of two ~mgpoint~

*/
void MGPoint::Intersection(MGPoint *mgp, MGPoint *res){
  /*if ((mgp->m_trajectory).Intersects(&m_trajectory)){*/
  const UGPoint *pCurr1, *pCurr2;
  Get(0, pCurr1);
  mgp->Get(0, pCurr2);
  Network *pNetwork = NetworkManager::GetNetwork(pCurr1->p0.GetNetworkId());
  if (!pNetwork->isDefined() || pNetwork == NULL) {
    sendMessages("Network does not exist.");
    res->SetDefined(false);
    NetworkManager::CloseNetwork(pNetwork);
  } else {
    if (pCurr1->p0.GetNetworkId() != pCurr2->p0.GetNetworkId()) {
      sendMessages("mgpoints belong to different networks.");
      res->SetDefined(false);
      NetworkManager::CloseNetwork(pNetwork);
    } else {
      MGPoint *resA = new MGPoint(0);
      MGPoint *resB = new MGPoint(0);
      refinementMovingGPoint (this, mgp, resA, resB);
      if (resA == NULL || !resA->IsDefined() ||
          resB == NULL || !resB->IsDefined() ||
          resA->GetNoComponents() != resB->GetNoComponents ()){
        res->SetDefined(false);
        NetworkManager::CloseNetwork(pNetwork);
      } else {
        if (resA->GetNoComponents() < 1) {
          res->SetDefined(true);
          NetworkManager::CloseNetwork(pNetwork);
        } else {
          res->StartBulkLoad();
          double interPosition;
          Instant tinter, tinter2;
          for (int i = 0; i < resA->GetNoComponents() ; i++) {
            resA->Get(i,pCurr1);
            resB->Get(i,pCurr2);
            if (pCurr1->p0.GetNetworkId() == pCurr2->p0.GetNetworkId() &&
                pCurr1->p0.GetRouteId() == pCurr2->p0.GetRouteId() &&
                pCurr1->p0.GetSide() == pCurr2->p0.GetSide()) {;
              if (pCurr1->p0.GetPosition() == pCurr2->p0.GetPosition() &&
                  pCurr1->p1.GetPosition() == pCurr2->p1.GetPosition()) {
                res->Add(*pCurr1/*, pNetwork*/);
              } else {
                tinter =
                    (pCurr1->timeInterval.end - pCurr1->timeInterval.start)
                    * ((pCurr2->p0.GetPosition() - pCurr1->p0.GetPosition()) /
                        (pCurr1->p1.GetPosition() - pCurr1->p0.GetPosition() -
                        pCurr2->p1.GetPosition() + pCurr2->p0.GetPosition())) +
                        pCurr1->timeInterval.start;
                if (pCurr1->timeInterval.start <= tinter &&
                    tinter <= pCurr1->timeInterval.end) {
                  interPosition = pCurr1->p0.GetPosition() +
                      (((pCurr1->p1.GetPosition() - pCurr1->p0.GetPosition())*
                    (tinter.ToDouble() - pCurr1->timeInterval.start.ToDouble())
                            / (pCurr1->timeInterval.end.ToDouble() -
                              pCurr1->timeInterval.start.ToDouble())));
                  if (!(fabs(interPosition - pCurr1->p0.GetPosition())<0.01||
                      fabs(interPosition - pCurr1->p1.GetPosition()) < 0.01 ||
                      fabs(interPosition - pCurr2->p0.GetPosition()) < 0.01 ||
                      fabs(interPosition - pCurr2->p1.GetPosition()) < 0.01)){
                    res->Add(UGPoint(Interval<Instant> (tinter, tinter,
                                                    true, true),
                                                  pCurr1->p0.GetNetworkId(),
                                                    pCurr1->p0.GetRouteId(),
                                                    pCurr1->p0.GetSide(),
                                                    interPosition,
                                                    interPosition)/*,
                                                pNetwork*/);
                  } else {
                    if (pCurr1->timeInterval.lc == true &&
                      fabs(interPosition - pCurr1->p0.GetPosition()) < 0.01){
                      res->Add(UGPoint(Interval<Instant> (tinter, tinter,
                                                    true, true),
                                                  pCurr1->p0.GetNetworkId(),
                                                    pCurr1->p0.GetRouteId(),
                                                    pCurr1->p0.GetSide(),
                                                    interPosition,
                                                    interPosition)/*,
                                                pNetwork*/);
                    } else {
                      if (pCurr1->timeInterval.rc == true &&
                        fabs(interPosition - pCurr1->p1.GetPosition()) <0.01){
                      res->Add(UGPoint(Interval<Instant> (tinter, tinter,
                                                      true, true),
                                                    pCurr1->p0.GetNetworkId(),
                                                      pCurr1->p0.GetRouteId(),
                                                      pCurr1->p0.GetSide(),
                                                      interPosition,
                                                      interPosition)/*,
                                                  pNetwork*/);
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
                if ((pCurr1->p0.GetPosition() <= pCurrJunct.GetRouteMeas() &&
                    pCurrJunct.GetRouteMeas() <= pCurr1->p1.GetPosition()) ||
                    (pCurr1->p1.GetPosition() <= pCurrJunct.GetRouteMeas() &&
                    pCurrJunct.GetRouteMeas() <= pCurr1->p0.GetPosition()) &&
                    pCurrJunct.GetOtherRouteId() == pCurr2->p0.GetRouteId()) {
                  found = true;
                  interPosition = pCurrJunct.GetRouteMeas();
                  // interPosition = pCurrJunct.getOtherRouteMeas();
                  if ((pCurr2->p0.GetPosition() <= interPosition &&
                    interPosition <= pCurr2->p1.GetPosition()) ||
                    (pCurr2->p1.GetPosition() <= interPosition &&
                    interPosition <= pCurr2->p0.GetPosition())) {
                    UGPoint pCurr = *pCurr1;
                    tinter = pCurr.TimeAtPos(pCurrJunct.GetRouteMeas());
                    tinter2 = pCurr.TimeAtPos(interPosition);
                    if (tinter == tinter2) {
                      double posJunc = pCurrJunct.GetRouteMeas();
                      res->Add(UGPoint(Interval<Instant> (tinter, tinter,
                                                    true, true),
                                                    pCurr1->p0.GetNetworkId(),
                                                    pCurr1->p0.GetRouteId(),
                                                    pCurr1->p0.GetSide(),
                                                    posJunc,
                                                    posJunc)/*,
                                                    pNetwork*/);
                    }
                  }
                };
                k++;
              }
            }// end if else same route and side
          }// end for
          res->EndBulkLoad(true/*, false*/);
          res->SetDefined(true);
          NetworkManager::CloseNetwork(pNetwork);
        }
      }
    }
  }
  /*}else {
    res->Clear();
    res->SetDefined(true);
  }*/
}

/*
Checks if ~mgpoint~ is inside a ~gline~

*/

void MGPoint::Inside(GLine *gl, MBool *res){
  /*if (m_trajectory.Intersects(gl)){*/
  const UGPoint *pCurrentUnit;
  Get(0, pCurrentUnit);
  if (gl->GetNetworkId() != pCurrentUnit->p0.GetNetworkId()) {
    res->SetDefined(false);
  } else {
    double mgStart, mgEnd, lStart, lEnd;
    const RouteInterval *pCurrRInter = new RouteInterval (0, 0.0, 0.0);
    bool swapped, found, bInterStart, bInterEnd;
    Instant tInterStart, tInterEnd;
    UBool interbool(true);
    int iRouteMgp;
    double interStart, interEnd;
    res->StartBulkLoad();
    for (int i = 0; i < GetNoComponents(); i++){
      Get(i, pCurrentUnit);
      UGPoint CurrUnit = *pCurrentUnit;
      int j = 0;
      iRouteMgp = pCurrentUnit->p0.GetRouteId();
      swapped = false;
      mgStart = pCurrentUnit->p0.GetPosition();
      mgEnd = pCurrentUnit->p1.GetPosition();
      if (mgEnd < mgStart) {
        mgStart = pCurrentUnit->p1.GetPosition();
        mgEnd = pCurrentUnit->p0.GetPosition();
        swapped = true;
      }
      if (gl->IsSorted()){
        vector<RouteInterval> vRI;
        const RouteInterval *currRInter;
        vRI.clear();
        getRouteIntervals(gl, iRouteMgp, mgStart, mgEnd, 0,
                          gl->NoOfComponents(), vRI);
        if (vRI.size() > 0) {
          size_t k = 0;
          while (k < vRI.size()) {
            currRInter = &vRI[k];
            if (pCurrentUnit->p0.GetPosition() <
                pCurrentUnit->p1.GetPosition())
            {
              if (pCurrentUnit->p0.GetPosition() >= currRInter->m_dStart) {
                interbool.timeInterval.start =
                    pCurrentUnit->timeInterval.start;
                interbool.timeInterval.lc = pCurrentUnit->timeInterval.lc;
                interbool.constValue.Set(true, true);
              } else {
                interbool.timeInterval.start =
                    pCurrentUnit->timeInterval.start;
                interbool.timeInterval.lc = pCurrentUnit->timeInterval.lc;
                tInterStart = CurrUnit.TimeAtPos(currRInter->m_dStart);
                interbool.timeInterval.end = tInterStart;
                interbool.timeInterval.rc = false;
                interbool.constValue.Set(true,false);
                res->MergeAdd(interbool);
                //compute start of unit inside
                interbool.timeInterval.start = tInterStart;
                interbool.timeInterval.lc = true;
                interbool.constValue.Set(true, true);
              }
              if (pCurrentUnit->p1.GetPosition() <= currRInter->m_dEnd) {
                interbool.timeInterval.end = pCurrentUnit->timeInterval.end;
                interbool.timeInterval.rc = pCurrentUnit->timeInterval.rc;
                res->MergeAdd(interbool);
              } else {
                // compute end of mgpoint at end of gline.and add result unit
                interbool.timeInterval.rc = true;
                tInterEnd = CurrUnit.TimeAtPos(currRInter->m_dEnd);
                interbool.timeInterval.end = tInterEnd;
                res->MergeAdd(interbool);
                // the rest of the current unit is not in the current
                // routeinterval.
                checkEndOfUGPoint(currRInter->m_dEnd,
                                  pCurrentUnit->p1.GetPosition(), tInterEnd,
                                  false, pCurrentUnit->timeInterval.end,
                                  pCurrentUnit->timeInterval.rc, k, vRI,
                                  res, iRouteMgp);
              }
              if (!(interStart == interEnd && (!bInterStart || !bInterEnd))) {
                res->MergeAdd(interbool);
              }
            } else {
              if (currRInter->m_dEnd >= pCurrentUnit->p0.GetPosition()) {
                interStart = pCurrentUnit->p0.GetPosition();
                tInterStart = pCurrentUnit->timeInterval.start;
                bInterStart = pCurrentUnit->timeInterval.lc;
              } else {
                interStart = currRInter->m_dEnd;
                bInterStart = true;
                tInterStart = CurrUnit.TimeAtPos(interStart);
              }
              if (currRInter->m_dStart <= pCurrentUnit->p1.GetPosition()) {
                interEnd = pCurrentUnit->p1.GetPosition();
                tInterEnd = pCurrentUnit->timeInterval.end;
                bInterEnd = pCurrentUnit->timeInterval.rc;
              } else {
                interEnd = currRInter->m_dStart;
                bInterEnd = true;
                tInterEnd = CurrUnit.TimeAtPos(interEnd);
              }
              if (!(interStart == interEnd && (!bInterStart || !bInterEnd))) {
                res->MergeAdd(interbool);
              }
            }
            k++;
          }
        }
      } else {
        found = false;
        while (j < gl->NoOfComponents()) {
          gl->Get(j, pCurrRInter);
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
              //intersection exists compute intersecting part and timevalues
              //for resulting unit
              found = true;
              if (!swapped) {
                if (lStart <= mgStart) {
                  interbool.timeInterval.start =
                      pCurrentUnit->timeInterval.start;
                  interbool.timeInterval.lc = pCurrentUnit->timeInterval.lc;
                  interbool.constValue.Set(true, true);
                } else {
                  // compute and write unit befor mgpoint inside gline
                  interbool.timeInterval.start =
                      pCurrentUnit->timeInterval.start;
                  interbool.timeInterval.lc = pCurrentUnit->timeInterval.lc;
                  tInterStart = CurrUnit.TimeAtPos(lStart);
                  interbool.timeInterval.end = tInterStart;
                  interbool.timeInterval.rc = false;
                  interbool.constValue.Set(true,false);
                  res->MergeAdd(interbool);
                  //compute start of unit inside
                  interbool.timeInterval.start = tInterStart;
                  interbool.timeInterval.lc = true;
                  interbool.constValue.Set(true, true);
                }
                if (lEnd >= mgEnd) {
                  interbool.timeInterval.end = pCurrentUnit->timeInterval.end;
                  interbool.timeInterval.rc = pCurrentUnit->timeInterval.rc;
                  res->MergeAdd(interbool);
                } else {
                  // compute end of mgpoint at end of gline.and add resultunit
                  interbool.timeInterval.rc = true;
                  tInterEnd = CurrUnit.TimeAtPos(lEnd);
                  interbool.timeInterval.end = tInterEnd;
                  res->MergeAdd(interbool);
                  // the rest of the current unit is not in the current
                  // routeinterval.
                  checkEndOfUGPoint(lEnd, pCurrentUnit->p1.GetPosition(),
                                    tInterEnd, false,
                                    pCurrentUnit->timeInterval.end,
                                  pCurrentUnit->timeInterval.rc, j, gl,
                                  res, iRouteMgp);
                }
              } else {
                mgStart = pCurrentUnit->p0.GetPosition();
                mgEnd = pCurrentUnit->p1.GetPosition();
                if (lEnd >= mgStart) {
                  interbool.timeInterval.start =
                      pCurrentUnit->timeInterval.start;
                  interbool.timeInterval.lc = pCurrentUnit->timeInterval.lc;
                  interbool.constValue.Set(true, true);
                } else {
                  // compute and write unit befor mgpoint inside gline
                  interbool.timeInterval.start =
                      pCurrentUnit->timeInterval.start;
                  interbool.timeInterval.lc = pCurrentUnit->timeInterval.lc;
                  tInterStart = CurrUnit.TimeAtPos(lEnd);
                  interbool.timeInterval.end = tInterStart;
                  interbool.timeInterval.rc = false;
                  interbool.constValue.Set(true,false);
                  res->MergeAdd(interbool);
                  //compute start of unit inside
                  interbool.timeInterval.start = tInterStart;
                  interbool.timeInterval.lc = true;
                  interbool.constValue.Set(true, true);
                }
                if (lStart <= mgEnd) {
                  interbool.timeInterval.end = pCurrentUnit->timeInterval.end;
                  interbool.timeInterval.rc = pCurrentUnit->timeInterval.rc;
                  res->MergeAdd(interbool);
                } else {
                  // compute end of mgpoint at end of gline.and add resultunit
                  interbool.timeInterval.rc = true;
                  tInterEnd = CurrUnit.TimeAtPos(lStart);
                  interbool.timeInterval.end = tInterEnd;
                  res->MergeAdd(interbool);
                  // the rest of the current unit is not in the current
                  // routeinterval.
                  checkEndOfUGPoint(lEnd, pCurrentUnit->p1.GetPosition(),
                                    tInterEnd, false,
                                    pCurrentUnit->timeInterval.end,
                                  pCurrentUnit->timeInterval.rc, j, gl,
                                  res, iRouteMgp);
                }
              }
            }else{
              if (mgStart == mgEnd && pCurrentUnit->timeInterval.lc &&
                pCurrentUnit->timeInterval.rc){
                found = true;
                interbool.timeInterval.start =
                    pCurrentUnit->timeInterval.start;
                interbool.timeInterval.lc = pCurrentUnit->timeInterval.lc;
                interbool.timeInterval.end = pCurrentUnit->timeInterval.end;
                interbool.timeInterval.rc = pCurrentUnit->timeInterval.rc;
                interbool.constValue.Set(true,true);
                res->MergeAdd(interbool);
              } else {
                if (lStart == lEnd) {
                  if ((lStart > mgStart && lStart < mgEnd) ||
                      (lStart < mgStart && lStart > mgEnd)) {
                    found = true;
                    // compute and write unit befor mgpoint inside gline
                    interbool.timeInterval.start =
                        pCurrentUnit->timeInterval.start;
                    interbool.timeInterval.lc = pCurrentUnit->timeInterval.lc;
                    tInterStart = CurrUnit.TimeAtPos(lStart);
                    interbool.timeInterval.end = tInterStart;
                    interbool.timeInterval.rc = false;
                    interbool.constValue.Set(true,false);
                    res->MergeAdd(interbool);
                    interbool.timeInterval.start = tInterStart;
                    interbool.timeInterval.rc = true;
                    interbool.timeInterval.lc = true;
                    interbool.constValue.Set(true, true);
                    res->MergeAdd(interbool);
                    checkEndOfUGPoint(lEnd, pCurrentUnit->p1.GetPosition(),
                                      tInterEnd, false,
                                      pCurrentUnit->timeInterval.end,
                                      pCurrentUnit->timeInterval.rc, j, gl,
                                      res, iRouteMgp);
                  } else {
                    if (lStart == mgStart && pCurrentUnit->timeInterval.lc) {
                      found = true;
                      interbool.timeInterval.start =
                          pCurrentUnit->timeInterval.start;
                      interbool.timeInterval.lc =
                          pCurrentUnit->timeInterval.lc;
                      interbool.timeInterval.end =
                          pCurrentUnit->timeInterval.start;
                      interbool.timeInterval.rc = true;
                      interbool.constValue.Set(true,true);
                      res->MergeAdd(interbool);
                      checkEndOfUGPoint(lEnd, pCurrentUnit->p1.GetPosition(),
                                        tInterEnd, false,
                                        pCurrentUnit->timeInterval.end,
                                        pCurrentUnit->timeInterval.rc, j, gl,
                                        res, iRouteMgp);
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
                        res->MergeAdd(interbool);
                        interbool.timeInterval.start =
                            pCurrentUnit->timeInterval.end;
                        interbool.timeInterval.rc =
                            pCurrentUnit->timeInterval.rc;
                        interbool.timeInterval.lc = true;
                        interbool.constValue.Set(true, true);
                        res->MergeAdd(interbool);
                      }
                    }
                  }
                } else {
                  if ((mgStart == lStart && mgEnd == lEnd) ||
                    (mgStart == lEnd && mgEnd == lStart)) {
                    found = true;
                    interbool.timeInterval = pCurrentUnit->timeInterval;
                    interbool.constValue.Set(true, true);
                    res->MergeAdd(interbool);
                  }
                }
              }
            }
          }
          j++;
        } // end while
        if (!found) {
            //no intersection found mgpoint not inside gline
          interbool.timeInterval = pCurrentUnit->timeInterval;
          interbool.constValue.Set(true,false);
          res->MergeAdd(interbool);
        }
      }
    } // end for
    res->EndBulkLoad();
  }
  /*}else{
    const UGPoint *pFirst, *pLast;
    Get(0, pFirst);
    Get(GetNoComponents()-1, pLast);
    UBool interbool;
    interbool.timeInterval.start = pFirst->timeInterval.start;
    interbool.timeInterval.end = pLast->timeInterval.end;
    interbool.timeInterval.lc = pFirst->timeInterval.lc;
    interbool.timeInterval.rc = pLast->timeInterval.rc;
    interbool.constValue.Set(true,false);
    res->StartBulkLoad();
    res->MergeAdd(interbool);
    res->EndBulkLoad();
  }*/
}

/*
Restricts the ~mgpoint~ to the given ~periods~

*/

void MGPoint::Atperiods(Periods *per, MGPoint *res){
  Instant utstart, utend;
  GPoint uGPstart, uGPend;
  bool ulc, urc;
  const UGPoint *pFirstUnit, *pLastUnit, *pCurrentUnit;
  if(!IsDefined() || !per->IsDefined() || IsEmpty() || per->IsEmpty())
    res->SetDefined(false);
  else {
   //both are defined and have at least one interval
    Get(0, pFirstUnit);
    Get(GetNoComponents()-1, pLastUnit);
    const Interval<Instant> *interval;
    int i = 0, j = 0;
    Get( i, pCurrentUnit );
    per->Get( j, interval );
    res->StartBulkLoad();
    while( i < GetNoComponents() && j < per->GetNoComponents()) {
      if (pLastUnit->timeInterval.Before(*interval)) break;
      if (pCurrentUnit->timeInterval.Before( *interval)){
        if( ++i >= GetNoComponents()) break;
        Get( i, pCurrentUnit );
      } else {
        if( interval->Before( pCurrentUnit->timeInterval )) {
          if( ++j >= per->GetNoComponents()) break;
          per->Get( j, interval );
        } else { // we have overlapping intervals, now
          if (pCurrentUnit->timeInterval.start >= interval->start &&
             pCurrentUnit->timeInterval.end <= interval->end) {
            res->Add(*pCurrentUnit);
            if (++i >= GetNoComponents()) break;
              Get(i,pCurrentUnit);
          }else{
            if (pCurrentUnit->timeInterval.start == interval->start) {
              utstart = interval->start;
              uGPstart = pCurrentUnit->p0;
              ulc = pCurrentUnit->timeInterval.lc && interval->lc;
            } else {
              if (pCurrentUnit->timeInterval.start > interval->start) {
                utstart = pCurrentUnit->timeInterval.start;
                uGPstart = pCurrentUnit->p0;
                ulc = pCurrentUnit->timeInterval.lc;
              } else {
                if (pCurrentUnit->timeInterval.start < interval->start) {
                  utstart = interval->start;
                  ulc = interval->lc;
                  pCurrentUnit->TemporalFunction(utstart, uGPstart, true);
                }
              }
            }
            if (pCurrentUnit->timeInterval.end == interval->end) {
              utend = interval->end;
              uGPend = pCurrentUnit->p1;
              urc = pCurrentUnit->timeInterval.rc && interval->rc;
            } else {
              if (pCurrentUnit->timeInterval.end < interval->end) {
                utend = pCurrentUnit->timeInterval.end;
                urc = pCurrentUnit->timeInterval.rc;
                uGPend = pCurrentUnit->p1;
              } else {
                if (pCurrentUnit->timeInterval.end > interval->end) {
                  utend = interval->end;
                  pCurrentUnit->TemporalFunction(utend, uGPend , true);
                  urc = interval->rc;
                }
              }
            }
            res->Add(UGPoint(Interval<Instant>(utstart, utend, ulc, urc),
                                uGPstart.GetNetworkId(),
                                uGPstart.GetRouteId(),
                                uGPstart.GetSide(),
                                uGPstart.GetPosition(),
                                uGPend.GetPosition())/*, pNetwork*/);
            if( interval->end == pCurrentUnit->timeInterval.end ){
              // same ending instant
              if( interval->rc == pCurrentUnit->timeInterval.rc ) {
                // same ending instant and rightclosedness: Advance both
                if( ++i >= GetNoComponents()) break;
                Get( i, pCurrentUnit );
                if( ++j >= per->GetNoComponents()) break;
                per->Get( j, interval );
              } else {
                if( interval->rc == true ) { // Advance in mapping
                  if( ++i >= GetNoComponents() ) break;
                  Get( i, pCurrentUnit );
                } else { // Advance in periods
                  assert( pCurrentUnit->timeInterval.rc == true );
                  if( ++j >= per->GetNoComponents() ) break;
                  per->Get( j, interval );
                }
              }
            } else {
              if( interval->end > pCurrentUnit->timeInterval.end ) {
                // Advance in mpoint
                if( ++i >= GetNoComponents() ) break;
                Get( i, pCurrentUnit );
              } else { // Advance in periods
                assert( interval->end < pCurrentUnit->timeInterval.end );
                if( ++j >= per->GetNoComponents() ) break;
                per->Get( j, interval );
              }
            }
          }
        }
      }
    }
    res->EndBulkLoad(true/*, false*/);
    res->SetDefined(true);
  }
}

/*
Restricts the ~mgpoint~ to the times it was at the given places.

*/

void MGPoint::At(GPoint *gp, MGPoint *res){
  /*if (gp->Inside(&m_trajectory)){*/
  Instant tPos;
  const UGPoint *pCurrentUnit, *pCheckUnit;
  Get(0, pCurrentUnit);
  if (gp->GetNetworkId() != pCurrentUnit->p0.GetNetworkId()) {
    res->SetDefined(false);
  } else {
    res->StartBulkLoad();
    int i= 0;
    while (i < GetNoComponents()) {
      Get(i, pCurrentUnit);
      UGPoint CurrUnit = *pCurrentUnit;
      if (pCurrentUnit->p0.GetRouteId() == gp->GetRouteId() &&
          (pCurrentUnit->p0.GetSide() == gp->GetSide() ||
          pCurrentUnit->p0.GetSide() ==2 || gp->GetSide()== 2)){
        if (fabs(gp->GetPosition()-pCurrentUnit->p0.GetPosition()) < 0.01){
          if (pCurrentUnit->p0.GetPosition() ==
              pCurrentUnit->p1.GetPosition()){
            res->Add(UGPoint(pCurrentUnit->timeInterval,
                      gp->GetNetworkId(), gp->GetRouteId(),
                      pCurrentUnit->p0.GetSide(), gp->GetPosition(),
                      gp->GetPosition())/*,pNetwork*/);
          } else {
            if(pCurrentUnit->timeInterval.lc) {
              res->Add(UGPoint(Interval<Instant>(
                              pCurrentUnit->timeInterval.start,
                              pCurrentUnit->timeInterval.start,
                              true, true),
                              gp->GetNetworkId(), gp->GetRouteId(),
                              pCurrentUnit->p0.GetSide(), gp->GetPosition(),
                              gp->GetPosition())/*,pNetwork*/);
            }
          }
        } else {
          if(fabs(gp->GetPosition()-pCurrentUnit->p1.GetPosition())<0.01) {
            if (pCurrentUnit->timeInterval.rc) {
              if (i < GetNoComponents()-1){
                i++;
                Get(i,pCheckUnit);
                if (pCheckUnit->p0.GetRouteId() ==
                    pCurrentUnit->p1.GetRouteId() &&
                    pCheckUnit->p0.GetPosition() ==
                    pCurrentUnit->p1.GetPosition()&&
                    pCheckUnit->timeInterval.start ==
                    pCurrentUnit->timeInterval.end){
                  if (pCheckUnit->p0.GetPosition() !=
                      pCheckUnit->p1.GetPosition()){
                    res->Add(UGPoint(Interval<Instant>(
                              pCurrentUnit->timeInterval.end,
                              pCurrentUnit->timeInterval.end,
                              true, true),
                              gp->GetNetworkId(), gp->GetRouteId(),
                              pCurrentUnit->p0.GetSide(), gp->GetPosition(),
                              gp->GetPosition())/*,pNetwork*/);
                  } else {
                    res->Add(UGPoint(pCheckUnit->timeInterval,
                                gp->GetNetworkId(), gp->GetRouteId(),
                                pCheckUnit->p0.GetSide(), gp->GetPosition(),
                                gp->GetPosition())/*,pNetwork*/);
                  }
                }
              } else {
                res->Add(UGPoint(Interval<Instant>(
                              pCurrentUnit->timeInterval.end,
                              pCurrentUnit->timeInterval.end,
                              true, true),
                              gp->GetNetworkId(), gp->GetRouteId(),
                              pCurrentUnit->p0.GetSide(), gp->GetPosition(),
                              gp->GetPosition())/*,pNetwork*/);
              }
            } else {
              if (i < GetNoComponents()-1) {
                i++;
                Get(i,pCheckUnit);
                if (pCheckUnit->p0.GetRouteId() ==
                    pCurrentUnit->p1.GetRouteId()
                    && pCheckUnit->p0.GetPosition() ==
                    pCurrentUnit->p1.GetPosition()
                    && pCheckUnit->timeInterval.start ==
                    pCurrentUnit->timeInterval.end){
                  if (pCheckUnit->p0.GetPosition() !=
                      pCheckUnit->p1.GetPosition()){
                    res->Add(UGPoint(Interval<Instant>(
                            pCurrentUnit->timeInterval.end,
                            pCurrentUnit->timeInterval.end,
                            true, true),
                            gp->GetNetworkId(), gp->GetRouteId(),
                            pCurrentUnit->p0.GetSide(), gp->GetPosition(),
                            gp->GetPosition())/*,pNetwork*/);
                  } else {
                    res->Add(UGPoint(pCheckUnit->timeInterval,
                      gp->GetNetworkId(), gp->GetRouteId(),
                      pCheckUnit->p0.GetSide(), gp->GetPosition(),
                      gp->GetPosition()));
                  }
                }
              }
            }
          } else {
            if((pCurrentUnit->p0.GetPosition() < gp->GetPosition() &&
                gp->GetPosition() < pCurrentUnit->p1.GetPosition()) ||
              (pCurrentUnit->p1.GetPosition() < gp->GetPosition() &&
                gp->GetPosition() < pCurrentUnit->p0.GetPosition())) {
              Instant tPos = CurrUnit.TimeAtPos(gp->GetPosition());
              res->Add(UGPoint(Interval<Instant>(tPos, tPos, true, true),
                      gp->GetNetworkId(), gp->GetRouteId(),
                      pCurrentUnit->p0.GetSide(), gp->GetPosition(),
                      gp->GetPosition()));
            }
          }
        }
      }
      i++;
    }
    res->EndBulkLoad(true/*, false*/);
    res->SetDefined(true);
  }
  /*}else{
    res->SetDefined(true);
  }*/
}

void MGPoint::At(GLine *gl, MGPoint *res){
  /*if (m_trajectory.Intersects(gl)){*/
  const UGPoint *pCurrentUnit;
  Get(0, pCurrentUnit);
  if (gl->GetNetworkId() != pCurrentUnit->p0.GetNetworkId()) {
    res->SetDefined(false);
  } else {
    int iNetworkId = gl->GetNetworkId();
    //Network *pNetwork = NetworkManager::GetNetwork(iNetworkId);
    double mgStart, mgEnd, lStart, lEnd, interStart, interEnd;
    const RouteInterval *pCurrRInter;
    Instant tInterStart, tInterEnd;
    bool bInterStart, bInterEnd, swapped;
    int iRouteMgp;
    res->StartBulkLoad();
    for (int i = 0; i < GetNoComponents(); i++) {
      Get(i, pCurrentUnit);
      UGPoint CurrUnit = *pCurrentUnit;
      iRouteMgp = pCurrentUnit->p0.GetRouteId();
      mgStart = pCurrentUnit->p0.GetPosition();
      mgEnd = pCurrentUnit->p1.GetPosition();
      swapped = false;
      if (mgEnd < mgStart) {
        mgStart = pCurrentUnit->p1.GetPosition();
        mgEnd = pCurrentUnit->p0.GetPosition();
        swapped = true;
      }
      if (gl->IsSorted()){

        vector<RouteInterval> vRI;
        const RouteInterval *currRInter;
        vRI.clear();
        getRouteIntervals(gl, iRouteMgp, mgStart, mgEnd, 0,
                          gl->NoOfComponents(), vRI);
        if (vRI.size() > 0) {
          size_t k = 0;
          while (k < vRI.size()) {
            currRInter = &vRI[k];
            if (pCurrentUnit->p0.GetPosition() ==
                pCurrentUnit->p1.GetPosition() &&
                ((currRInter->m_dStart <= pCurrentUnit->p0.GetPosition() &&
                  currRInter->m_dEnd >= pCurrentUnit->p0.GetPosition())||
                  (currRInter->m_dStart >= pCurrentUnit->p0.GetPosition() &&
                  currRInter->m_dEnd <= pCurrentUnit->p0.GetPosition()))) {
              res->Add(*pCurrentUnit/*,pNetwork*/);
            } else {
              if(pCurrentUnit->p0.GetPosition() <
                pCurrentUnit->p1.GetPosition())
              {
                if (pCurrentUnit->p0.GetPosition()>=currRInter->m_dStart &&
                   pCurrentUnit->p1.GetPosition()<=currRInter->m_dEnd){
                  res->Add(*pCurrentUnit/*,pNetwork*/);
                }else{
                  if (pCurrentUnit->p0.GetPosition() >= currRInter->m_dStart) {
                    interStart = pCurrentUnit->p0.GetPosition();
                    tInterStart = pCurrentUnit->timeInterval.start;
                    bInterStart = pCurrentUnit->timeInterval.lc;
                  } else {
                    interStart = currRInter->m_dStart;
                    bInterStart = true;
                    tInterStart = CurrUnit.TimeAtPos(interStart);
                  }
                  if (pCurrentUnit->p1.GetPosition() <= currRInter->m_dEnd) {
                    interEnd = pCurrentUnit->p1.GetPosition();
                    tInterEnd = pCurrentUnit->timeInterval.end;
                    bInterEnd = pCurrentUnit->timeInterval.rc;
                  } else {
                    interEnd = currRInter->m_dEnd;
                    bInterEnd = true;
                    tInterEnd = CurrUnit.TimeAtPos(interEnd);
                  }
                  if (interStart != interEnd || (bInterStart && bInterEnd)) {
                    res->Add(UGPoint(Interval<Instant> (tInterStart, tInterEnd,
                                  bInterStart, bInterEnd), iNetworkId,
                                  iRouteMgp, pCurrentUnit->p0.GetSide(),
                                  interStart, interEnd)/*,pNetwork*/);
                  }
                }
              } else {
                if(pCurrentUnit->p0.GetPosition() >
                  pCurrentUnit->p1.GetPosition())
                {
                  if(pCurrentUnit->p1.GetPosition()>=currRInter->m_dStart &&
                   pCurrentUnit->p0.GetPosition()<=currRInter->m_dEnd){
                    res->Add(*pCurrentUnit/*,pNetwork*/);
                  }else{
                    if (currRInter->m_dEnd >= pCurrentUnit->p0.GetPosition()) {
                      interStart = pCurrentUnit->p0.GetPosition();
                      tInterStart = pCurrentUnit->timeInterval.start;
                      bInterStart = pCurrentUnit->timeInterval.lc;
                    } else {
                      interStart = currRInter->m_dEnd;
                      bInterStart = true;
                      tInterStart = CurrUnit.TimeAtPos(interStart);
                    }
                    if (currRInter->m_dStart <= pCurrentUnit->p1.GetPosition()){
                      interEnd = pCurrentUnit->p1.GetPosition();
                      tInterEnd = pCurrentUnit->timeInterval.end;
                      bInterEnd = pCurrentUnit->timeInterval.rc;
                    } else {
                      interEnd = currRInter->m_dStart;
                      bInterEnd = true;
                      tInterEnd = CurrUnit.TimeAtPos(interEnd);
                    }
                    if (interStart != interEnd || (bInterStart && bInterEnd)) {
                      res->Add(UGPoint(Interval<Instant> (tInterStart,
                                  tInterEnd, bInterStart, bInterEnd),
                                  iNetworkId, iRouteMgp,
                                  pCurrentUnit->p0.GetSide(),
                                  interStart, interEnd)/*,pNetwork*/);
                    }
                  }
                }
              }
            }
            k++;
          }
        }
      } else {

        int j = 0;
        while (j < gl->NoOfComponents()) {

          gl->Get(j,pCurrRInter);
          if (iRouteMgp == pCurrRInter->m_iRouteId){

            lStart = pCurrRInter->m_dStart;
            lEnd = pCurrRInter->m_dEnd;
            if (lStart > lEnd) {
              lStart = pCurrRInter->m_dEnd;
              lEnd = pCurrRInter->m_dStart;
            }
            if (!(mgEnd < lStart || mgStart > lEnd)){
              //intersection exists compute intersecting part and timevalues
              //for resulting unit

              if (!swapped) {
                if (lStart <= mgStart) {

                  interStart = pCurrentUnit->p0.GetPosition();
                  tInterStart = pCurrentUnit->timeInterval.start;
                  bInterStart = pCurrentUnit->timeInterval.lc;
                } else {

                  interStart = lStart;
                  bInterStart = true;
                  tInterStart = CurrUnit.TimeAtPos(interStart);
                }
                if (lEnd >= mgEnd) {

                  interEnd = pCurrentUnit->p1.GetPosition();
                  tInterEnd = pCurrentUnit->timeInterval.end;
                  bInterEnd = pCurrentUnit->timeInterval.rc;
                } else {

                  interEnd = lEnd;
                  bInterEnd = true;
                  tInterEnd = CurrUnit.TimeAtPos(interEnd);
                }
                if (!(interStart == interEnd &&
                      (!bInterStart || !bInterEnd))) {
                  if (tInterStart == pCurrentUnit->timeInterval.start &&
                      tInterEnd == pCurrentUnit->timeInterval.end &&
                      interStart == pCurrentUnit->p0.GetPosition()&&
                      interEnd == pCurrentUnit->p1.GetPosition()){
                    res->Add(*pCurrentUnit/*,pNetwork*/);
                  }else{
                    res->Add(UGPoint(Interval<Instant> (tInterStart, tInterEnd,
                                  bInterStart, bInterEnd), iNetworkId,
                                  iRouteMgp, pCurrentUnit->p0.GetSide(),
                                  interStart, interEnd)/*,pNetwork*/);
                  }
                } else {
                  if (pCurrentUnit->p0.GetPosition() ==
                    pCurrentUnit->p1.GetPosition() && interStart == interEnd){
                    res->Add(*pCurrentUnit/*,pNetwork*/);
                  }
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
                  tInterStart = CurrUnit.TimeAtPos(interStart);
                }
                if (lStart <= mgEnd) {
                  interEnd = pCurrentUnit->p1.GetPosition();
                  tInterEnd = pCurrentUnit->timeInterval.end;
                  bInterEnd = pCurrentUnit->timeInterval.rc;
                } else {
                  interEnd = lStart;
                  bInterEnd = true;
                  tInterEnd = CurrUnit.TimeAtPos(interEnd);
                }
                if (!(interStart == interEnd &&
                      (!bInterStart || !bInterEnd))) {
                  if (tInterStart== pCurrentUnit->timeInterval.start &&
                     tInterEnd == pCurrentUnit->timeInterval.end &&
                     interStart == pCurrentUnit->p0.GetPosition() &&
                     interEnd == pCurrentUnit->p1.GetPosition()){
                    res->Add(*pCurrentUnit/*,pNetwork*/);
                  }else{
                    res->Add(UGPoint(Interval<Instant> (tInterStart, tInterEnd,
                                      bInterStart, bInterEnd), iNetworkId,
                                      iRouteMgp, pCurrentUnit->p0.GetSide(),
                                      interStart, interEnd)/*,pNetwork*/);
                  }
                } else {
                  if (pCurrentUnit->p0.GetPosition() ==
                      pCurrentUnit->p1.GetPosition() &&
                      interStart == interEnd)
                  {
                    res->Add(*pCurrentUnit/*,pNetwork*/);
                  }
                }
              }
            }
          }
          j++;
        }
      }
    }
    res->EndBulkLoad(true/*, false*/);
    res->SetDefined(true);
    //NetworkManager::CloseNetwork(pNetwork);
  }
  /*}else{
    res->SetDefined(true);
  }*/
}

/*
Compresses the ~mgpoint~ by equalizing speed differences witch are smaller than
~d~

*/

MGPoint* MGPoint::Simplify(double d){
  MGPoint *res=new MGPoint(0);
  /*res->SetBBox(this->BoundingBox());
  res->SetLength(m_length);
  res->m_trajectory = m_trajectory;*/
  res->StartBulkLoad();

  int iNetworkId;
  int iStartRouteId = 0;
  Side xStartSide;
  double dStartSpeed;

  Instant xStartStartTime;
  double dStartStartPosition;
  double dLastEndPosition;
  Instant xLastEndTime;
  for (int i = 0; i < GetNoComponents(); i++)
  {
    //////////////////////////////
    //
    // Get values for current unit
    //
    //////////////////////////////
    const UGPoint *pCurrentUnit;
    Get(i, pCurrentUnit);

    // Duration
    Instant xCurrentStartTime = pCurrentUnit->timeInterval.start;
    LONGTYPE lCurrentStartTime = xCurrentStartTime.GetAllMilliSeconds();
    Instant xCurrentEndTime = pCurrentUnit->timeInterval.end;
    LONGTYPE lCurrentEndTime = xCurrentEndTime.GetAllMilliSeconds();
    LONGTYPE lCurrentDuration = lCurrentEndTime - lCurrentStartTime;

    // Distance
    GPoint xCurrentStart = pCurrentUnit->p0;
    GPoint xCurrentEnd = pCurrentUnit->p1;
    iNetworkId = xCurrentStart.GetNetworkId();
    int iCurrentRouteId = xCurrentStart.GetRouteId();
    Side xCurrentSide = xCurrentStart.GetSide();
    double dCurrentStartPosition = xCurrentStart.GetPosition();
    double dCurrentEndPosition = xCurrentEnd.GetPosition();
    double dCurrentDistance = dCurrentEndPosition - dCurrentStartPosition;

    // Speed
    double dCurrentSpeed = dCurrentDistance / lCurrentDuration;
    //////////////////////////////
    //
    // Set start values if this
    // is the first unit-start
    //
    //////////////////////////////
    if(iStartRouteId == 0)
    {
      iStartRouteId = iCurrentRouteId;
      xStartSide = xCurrentSide;
      dStartSpeed = dCurrentSpeed;
      xStartStartTime = xCurrentStartTime;
      dStartStartPosition = dCurrentStartPosition;
    }

    //////////////////////////////
    //
    // Check if this units differs
    // from the ones before.
    // If so create unit for all
    // units before this.
    //
    //////////////////////////////
    double dSpeedDifference = dCurrentSpeed > dStartSpeed ?
                              dCurrentSpeed - dStartSpeed :
                              dStartSpeed - dCurrentSpeed;

    if( iCurrentRouteId != iStartRouteId ||
        xCurrentSide != xStartSide ||
        dSpeedDifference > d)
    {
      // Create new unit
      if (xStartStartTime == pCurrentUnit->timeInterval.start &&
         xLastEndTime == pCurrentUnit->timeInterval.end &&
         iStartRouteId == pCurrentUnit->p0.GetRouteId()&&
         xStartSide == pCurrentUnit->p0.GetSide() &&
         dStartStartPosition == pCurrentUnit->p0.GetPosition() &&
         dLastEndPosition == pCurrentUnit->p1.GetPosition()) {
        res->Add(*pCurrentUnit/*, false*/);
      } else {
        res->Add(UGPoint(Interval<Instant>(xStartStartTime,
                                         xLastEndTime,
                                         true,
                                         false),
                              iNetworkId,
                              iStartRouteId,
                              xStartSide,
                              dStartStartPosition,
                              dLastEndPosition)/*, false*/);
      }
      // Set new Start-Values
      iStartRouteId = iCurrentRouteId;
      xStartSide = xCurrentSide;
      dStartSpeed = dCurrentSpeed;
      xStartStartTime = xCurrentStartTime;
      dStartStartPosition = dCurrentStartPosition;
    }

    if( i == GetNoComponents() -1)
    {
      // Last loop - create last unit
      if (xStartStartTime == pCurrentUnit->timeInterval.start &&
         xLastEndTime == pCurrentUnit->timeInterval.end &&
         iStartRouteId == pCurrentUnit->p0.GetRouteId()&&
         xStartSide == pCurrentUnit->p0.GetSide() &&
         dStartStartPosition == pCurrentUnit->p0.GetPosition() &&
         dLastEndPosition == pCurrentUnit->p1.GetPosition()){
        res->Add(*pCurrentUnit/*,false*/);
      } else {
        res->Add(UGPoint(Interval<Instant>(xStartStartTime,
                                                        xCurrentEndTime,
                                                        true,
                                                        false),
                              iNetworkId,
                              iStartRouteId,
                              xStartSide,
                              dStartStartPosition,
                              dCurrentEndPosition)/*,false*/);
      }
    }

    // Set Last-Values for next loop
    dLastEndPosition = dCurrentEndPosition;
    xLastEndTime = xCurrentEndTime;
  }


  // Units were added to the moving gpoint. They are sorted and
  // the bulk-load is ended:
  res->EndBulkLoad(true/*, false, false*/);
  return res;
}

bool MGPoint::Passes(GPoint *gp){
  /*if (gp->Inside(&m_trajectory)) return true;
  else return false;*/

  const UGPoint *pCurrentUnit;
  Get(0, pCurrentUnit);
  if (gp->GetNetworkId() != pCurrentUnit->p0.GetNetworkId()) return false;
  for (int i = 0; i < GetNoComponents(); i++) {
    Get(i, pCurrentUnit);
    if (pCurrentUnit->p0.GetRouteId() == gp->GetRouteId()){
    // check if p is between p0 and p1
      if((pCurrentUnit->p0.GetPosition() < gp->GetPosition() &&
        gp->GetPosition() < pCurrentUnit->p1.GetPosition()) ||
        (pCurrentUnit->p1.GetPosition() < gp->GetPosition() &&
        gp->GetPosition() < pCurrentUnit->p0.GetPosition())) return true;
      // If the edge of the interval is included we need to check the exakt
      // Position too.
      if((pCurrentUnit->timeInterval.lc &&
          AlmostEqual(pCurrentUnit->p0.GetPosition(), gp->GetPosition()))||
          (pCurrentUnit->timeInterval.rc &&
          AlmostEqual(pCurrentUnit->p1.GetPosition(),gp->GetPosition())))
        return true;
    }
  }
  return false;

}

/*
Checks if the ~mgpoint~ passes the given places.

*/

bool MGPoint::Passes(GLine *gl){
  /*if (m_trajectory.Intersects(gl)) return true;
  else return false;*/
  double dMGPStart, dMGPEnd;
  const UGPoint *pCurrentUnit;
  Get(0, pCurrentUnit);
  if (gl->GetNetworkId() != pCurrentUnit->p0.GetNetworkId()) return false;
  const RouteInterval *pCurrRInter;
  for (int i = 0; i < GetNoComponents(); i++)
  {
    Get(i, pCurrentUnit);
    if (pCurrentUnit->p0.GetPosition() <= pCurrentUnit->p1.GetPosition()) {
      dMGPStart = pCurrentUnit->p0.GetPosition();
      dMGPEnd = pCurrentUnit->p1.GetPosition();
    } else {
      dMGPStart = pCurrentUnit->p1.GetPosition();
      dMGPEnd = pCurrentUnit->p0.GetPosition();
    }
    if (gl->IsSorted()){
      if (searchUnit(gl, 0, gl->NoOfComponents()-1,
                     pCurrentUnit->p0.GetRouteId(), dMGPStart, dMGPEnd)){
        return true;
      }
    } else {
      for (int j = 0 ; j < gl->NoOfComponents(); j ++){
        gl->Get(j,pCurrRInter);
        if (pCurrRInter->m_iRouteId == pCurrentUnit->p0.GetRouteId() &&
           (!(pCurrRInter->m_dEnd < dMGPStart ||
              pCurrRInter->m_dStart > dMGPEnd))) return true;
      }
    }
  }
  return false;
}

MGPoint* MGPoint::Clone() const {
  MGPoint *result = new MGPoint( GetNoComponents() );
  if(GetNoComponents()>0){
      result->units.Resize(GetNoComponents());
  }
  /*result->SetBBox(BoundingBox());
  result->SetLength(GetLength());
  result->m_trajectory = m_trajectory;*/
  result->StartBulkLoad();
  const UGPoint *unit;
  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, unit );
    result->Add( *unit/*, false */);
  }
    result->EndBulkLoad( false/*, false */);
    return result;
}

void MGPoint::Restrict( const vector< pair<int, int> >& intervals )
{
  units.Restrict( intervals ); // call super
  /*m_bbox.SetDefined(false);      // invalidate bbox
  m_length = 0.0;
  m_trajectory.Clear();
  RestoreBoundingBox(true); */       // recalculate it#
}

void MGPoint::Add(const UGPoint& u/*, bool setbbox =true*/){
  if (u.IsValid()) {
    units.Append(u);
    /*if (setbbox) {
      if (units.Size() == 1){
        m_bbox.SetDefined(true);
        m_bbox = u.BoundingBox();
        m_length = fabs(u.p1.GetPosition()-u.p0.GetPosition());
      } else {
        m_bbox = m_bbox.Union(u.BoundingBox());
        m_length += fabs(u.p1.GetPosition()-u.p0.GetPosition());
      }
    }*/
  }
}

    /*
void MGPoint::Add(const UGPoint& u, Network *pNetwork, bool setbbox =true){
  if (u.IsValid()){
    units.Append(u);
    if (setbbox) {
      if (units.Size() == 1){
        m_bbox.SetDefined(true);
        m_bbox = u.BoundingBox(pNetwork);
        m_length = fabs(u.p1.GetPosition()-u.p0.GetPosition());

      } else {
        m_bbox = m_bbox.Union(u.BoundingBox(pNetwork));
        m_length += fabs(u.p1.GetPosition()-u.p0.GetPosition());
      }
    }
  }
}*/

 Rectangle<3> MGPoint::BoundingBox() const{
   if(IsDefined() && !IsEmpty()){
     const UGPoint *pCurrUnit;
     Get(0,pCurrUnit);
     Network *pNetwork =
         NetworkManager::GetNetwork(pCurrUnit->p0.GetNetworkId());
     Rectangle<3> res = pCurrUnit->BoundingBox(pNetwork);
     for (int i = 1; i < GetNoComponents(); i++){
       Get(i,pCurrUnit);
       res = res.Union(pCurrUnit->BoundingBox());
     }
     NetworkManager::CloseNetwork(pNetwork);
     return res;
   }else{
     return Rectangle<3>(false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
   }
 }

 ostream& MGPoint::Print( ostream &os ) const
{
  if( !IsDefined() )
  {
    return os << "(MGPoint: undefined)";
  }
  os << "(MGPoint: defined ";
  os << ", contains " << GetNoComponents() << " units: ";
  for(int i=0; i<GetNoComponents(); i++)
  {
    const UGPoint *unit;
    Get( i , unit );
    os << "\n\t";
    unit->Print(os);
  }
  os << "\n)" << endl;
  return os;
}

void MGPoint::EndBulkLoad(const bool sort /*=true,
                           bool setbbox =false,
                          bool setTraj true*/)
{
  Mapping<UGPoint, GPoint>::EndBulkLoad( sort );
  /*if ((setbbox || !m_bbox.IsDefined()) & setTraj) RestoreAll();
  else {
    RestoreBoundingBox(setbbox);
    if (setTraj) RestoreTrajectory();
  }*/
}
    /*
  void MGPoint::RestoreAll(){
  if(!IsDefined() || GetNoComponents() == 0)
  { // invalidate m_bbox and length value
    m_bbox.SetDefined(false);
    m_length = 0.0;
    m_trajectory.SetDefined(false);
  }
  else
  {
    const UGPoint *unit;
    GLine *res = new GLine(0);
    res->SetNetworkId(GetNetworkId());
    res->SetDefined(true);
    int aktRouteId;
    double aktStartPos, aktEndPos;
    RITree *tree;
    int size = GetNoComponents();
    for( int i = 0; i < size; i++ )
    {
      Get( i, unit );
      if (i == 0)
      {
        m_length = fabs(unit->p1.GetPosition()-unit->p0.GetPosition());
        m_bbox = unit->BoundingBox();
        aktRouteId = unit->p0.GetRouteId();
        aktStartPos = unit->p0.GetPosition();
        aktEndPos = unit->p1.GetPosition();
        chkStartEnd(aktStartPos, aktEndPos);
        tree = new RITree(aktRouteId, aktStartPos, aktEndPos,0,0);
      }
      else
      {
        m_bbox = m_bbox.Union(unit->BoundingBox());
        m_length += fabs(unit->p1.GetPosition()-unit->p0.GetPosition());
        int curRoute = unit->p0.GetRouteId();
        double curStartPos = unit->p0.GetPosition();
        double curEndPos = unit->p1.GetPosition();
        chkStartEnd(curStartPos, curEndPos);
        if (curRoute != aktRouteId) {
          tree->Insert(aktRouteId, aktStartPos, aktEndPos);
          aktRouteId = curRoute;
          aktStartPos = curStartPos;
          aktEndPos = curEndPos;
        } else { // curRoute == aktRouteId concat pieces if possible
          if (AlmostEqual(aktStartPos,curEndPos)) {
            aktStartPos = curStartPos;
          } else {
            if (AlmostEqual(aktEndPos,curStartPos)) {
              aktEndPos = curEndPos;
            } else { //concat impossible start new routeInterval for gline.
              tree->Insert(aktRouteId, aktStartPos, aktEndPos);
              aktRouteId = curRoute;
              aktStartPos = curStartPos;
              aktEndPos = curEndPos;
            }
          }
        }
      }
    }
    tree->Insert(aktRouteId, aktStartPos, aktEndPos);
    tree->TreeToGLine(res);
    tree->RemoveTree();
    res->SetSorted(true);
    res->SetDefined(true);
    m_trajectory = *res;
    delete res;
  }
}

void MGPoint::RestoreTrajectory() {
  GLine *res = new GLine(0);
  const UGPoint *pCurrentUnit;
  Get(0, pCurrentUnit);
  int iNetworkId = pCurrentUnit->p0.GetNetworkId();
  res->SetNetworkId(iNetworkId);
  res->SetDefined(true);
  int aktRouteId = pCurrentUnit->p0.GetRouteId();
  double aktStartPos = pCurrentUnit->p0.GetPosition();
  double aktEndPos = pCurrentUnit->p1.GetPosition();
  chkStartEnd(aktStartPos, aktEndPos);
  RITree *tree;
  tree = new RITree(aktRouteId, aktStartPos, aktEndPos,0,0);
  for (int i = 1; i < GetNoComponents(); i++)
  {
    // Get start and end of current unit
    Get(i, pCurrentUnit);
    int curRoute = pCurrentUnit->p0.GetRouteId();
    double curStartPos = pCurrentUnit->p0.GetPosition();
    double curEndPos = pCurrentUnit->p1.GetPosition();
    chkStartEnd(curStartPos, curEndPos);
    if (curRoute != aktRouteId) {
      tree->Insert(aktRouteId, aktStartPos, aktEndPos);
      aktRouteId = curRoute;
      aktStartPos = curStartPos;
      aktEndPos = curEndPos;
    } else { // curRoute == aktRouteId concat pieces if possible
      if (AlmostEqual(aktStartPos,curEndPos)) {
        aktStartPos = curStartPos;
      } else {
        if (AlmostEqual(aktEndPos,curStartPos)) {
          aktEndPos = curEndPos;
        } else { //concat impossible start new routeInterval for gline.
          tree->Insert(aktRouteId, aktStartPos, aktEndPos);
          aktRouteId = curRoute;
          aktStartPos = curStartPos;
          aktEndPos = curEndPos;
        }
      }
    }
  }
  tree->Insert(aktRouteId, aktStartPos, aktEndPos);
  tree->TreeToGLine(res);
  tree->RemoveTree();
  res->SetSorted(true);
  res->SetDefined(true);
  m_trajectory = *res;
  delete res;
}
void MGPoint::RestoreBoundingBox(const bool force =false)
{
  if(!IsDefined() || GetNoComponents() == 0)
  { // invalidate m_bbox and length value
    m_bbox.SetDefined(false);
    m_length = 0.0;
  }
  else if(force || !m_bbox.IsDefined())
  { // construct m_bbox and m_length value
    const UGPoint *unit;
    int size = GetNoComponents();
    for( int i = 0; i < size; i++ )
    {
      Get( i, unit );
      if (i == 0)
      {
        m_length = fabs(unit->p1.GetPosition()-unit->p0.GetPosition());
        m_bbox = unit->BoundingBox();
      }
      else
      {
        m_bbox = m_bbox.Union(unit->BoundingBox());
        m_length += fabs(unit->p1.GetPosition()-unit->p0.GetPosition());
      }
    }
  } // else: bbox unchanged and still correct
}

void MGPoint::SetBBox(Rectangle<3> r){
  m_bbox = r;
}*/

Word InMGPoint(const ListExpr typeInfo, const ListExpr instance,
                const int errorPos, ListExpr& errorInfo, bool& correct)
{
  int numUnits = nl->ListLength(instance);
  MGPoint* m = new MGPoint( numUnits );
  correct = true;
  int unitcounter = 0;
  string errmsg;
  //Network *pNetwork;
  m->StartBulkLoad();

  ListExpr rest = instance;
  if (nl->AtomType( rest ) != NoAtom)
  { if(nl->IsEqual(rest,"undef")){
       m->EndBulkLoad(true/*, false*/);
       m->SetDefined(false);
       return SetWord( Address( m ) );
    } else {
      correct = false;
      delete m;
      return SetWord( Address( 0 ) );
    }
  }
  else {
   // bool bfirst = true;
    while( !nl->IsEmpty( rest ) )
    {
      ListExpr first = nl->First( rest );
      rest = nl->Rest( rest );

      UGPoint *unit = (UGPoint*) UGPoint::In( nl->TheEmptyList(), first,
                                  errorPos, errorInfo, correct ).addr;

      if( correct && (!unit->IsDefined() || !unit->IsValid() ) )
      {
        errmsg = "InMapping(): Unit " + int2string(unitcounter) + " is undef.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        correct = false;
        delete unit;
        delete m;
        //if (!bfirst) NetworkManager::CloseNetwork(pNetwork);
        return SetWord( Address(0) );
      }
      if ( !correct )
      {
        errmsg = "InMapping(): Representation of Unit "
                  + int2string(unitcounter) + " is wrong.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        m->Destroy();
        delete m;
        //if (!bfirst) NetworkManager::CloseNetwork(pNetwork);
        return SetWord( Address(0) );
      }
      /*if (bfirst) {
        pNetwork = NetworkManager::GetNetwork(unit->p0.GetNetworkId());
        bfirst = false;
      }*/
      m->Add( *unit/*, pNetwork */);
      unitcounter++;
      delete unit;
    }

    m->EndBulkLoad(true/*, false*/); // if this succeeds, all is OK
    //if (!bfirst) NetworkManager::CloseNetwork(pNetwork);
    return SetWord( m );
  }
}


bool OpenMGPoint(SmiRecord& valueRecord,
                     size_t& offset,
                     const ListExpr typeInfo,
                     Word& value)
{
  MGPoint *m =
      static_cast<MGPoint*>(Attribute::Open( valueRecord, offset, typeInfo ));
  value = SetWord( m );
  //m->RestoreAll();
  //m->RestoreBoundingBox();
  return true;
}

TypeConstructor movinggpoint(
        "mgpoint",                                  // Name
        MGPoint::Property,                          // Property function
        OutMapping<MGPoint, UGPoint, UGPoint::Out>, // Out and In functions
        InMGPoint,            /*InMapping<MGPoint, UGPoint, UGPoint::In>,*/
        0,                                          // SaveToList and
        0,                                          // RestoreFromList
        CreateMapping<MGPoint>,                     // Object creation and
        DeleteMapping<MGPoint>,                     // deletion
        OpenMGPoint,            /*OpenAttribute<MGPoint>, */
        SaveAttribute<MGPoint>,                     // Object open and save
        CloseMapping<MGPoint>,                      // Object close and clone
        CloneMapping<MGPoint>,
        CastMapping<MGPoint>,                       // Cast function
        SizeOfMapping<MGPoint>,                     // Sizeof function
        MGPoint::Check);                            // Kind checking function



/*
1.2.2 Classe UGPoint

Temporal Function

*/

void UGPoint::TemporalFunction( const Instant& t,
                                GPoint& result,
                                bool ignoreLimits ) const
{
  if( !IsDefined() ||
      !t.IsDefined() ||
      (!timeInterval.Contains( t ) && !ignoreLimits) ){
      result.SetDefined(false);
  } else {
    if( t == timeInterval.start ){
      result = p0;
      result.SetDefined(true);
    } else {
      if( t == timeInterval.end ) {
        result = p1;
        result.SetDefined(true);
      }  else {
        double tStart = timeInterval.start.ToDouble();
        double tEnd = timeInterval.end.ToDouble();
        double tInst = t.ToDouble();
        double posStart = p0.GetPosition();
        double posEnd = p1.GetPosition();
        double posInst = (posEnd-posStart) * (tInst-tStart) /
                        (tEnd - tStart) + posStart;
        result = GPoint(true, p0.GetNetworkId(), p0.GetRouteId(), posInst,
                        p0.GetSide());
        result.SetDefined(true);
      }
    }
  }
  return;
}


/*
Checks wether a unit passes a fixed point in the network

*/

bool UGPoint::Passes( const GPoint& p ) const
{
  assert( IsDefined() );
  assert( p.IsDefined() );
  // Check if this unit is on the same route as the GPoint
  if(p0.GetRouteId()!= p.GetRouteId())
  {
    return false;
  }

  // p is between p0 and p1
  if((p0.GetPosition() < p.GetPosition() &&
     p.GetPosition() < p1.GetPosition()) ||
     (p1.GetPosition() < p.GetPosition() &&
     p.GetPosition() < p0.GetPosition()))
  {
    return true;
  }

  // If the edge of the interval is included we need to check the exakt
  // Position too.
  if((timeInterval.lc &&
     AlmostEqual(p0.GetPosition(), p.GetPosition())) ||
     (timeInterval.rc &&
     AlmostEqual(p1.GetPosition(),p.GetPosition())))
  {
    return true;
  }
  return false;
}

/*
Restricts the ~UGPoint~ to the times he was at a given ~GPoint~.

*/

bool UGPoint::At( const GPoint& p, TemporalUnit<GPoint>& result ) const
{
  if (!IsDefined() || !p.IsDefined()) {
    cerr << "mgpoint and gpoint must be defined." << endl;
    return false;
  }
  assert (IsDefined());
  assert (p.IsDefined());
  UGPoint *pResult = (UGPoint*) &result;
  if (p0.GetNetworkId() != p.GetNetworkId()) {
    return false;
  } else {
    if (p0.GetRouteId() != p.GetRouteId()){
      return false;
    } else {
      if (p.GetSide() != p0.GetSide() &&
          !(p.GetSide() == 2 || p0.GetSide() == 2)) {
            return false;
      } else {
        double start = p0.GetPosition();
        double end = p1.GetPosition();
        double pos = p.GetPosition();
        if (AlmostEqual(start,pos) && timeInterval.lc) {
          Interval<Instant> interval(timeInterval.start,
                                     timeInterval.start, true, true);
          UGPoint aktunit(interval, p,p);
          *pResult = aktunit;
          return true;
        } else {
          if (AlmostEqual(end,pos) && timeInterval.rc) {
            Interval<Instant> interval(timeInterval.end,
                                       timeInterval.end, true, true);
            UGPoint aktunit(interval, p,p);
            *pResult = aktunit;
            return true;
          } else {
            if ((start < pos && pos < end) || (end < pos && pos < start)) {
              double factor = fabs(pos-start) / fabs(end-start);
              Instant tpos = (timeInterval.end - timeInterval.start) * factor +
                              timeInterval.start;
              Interval<Instant> interval(tpos, tpos, true, true);
              UGPoint aktunit(interval, p, p);
              *pResult = aktunit;
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}

/*
SECONDO Integration of ~ugpoint~

*/
ListExpr UGPoint::Property()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> UNIT"),
                             nl->StringAtom("(ugpoint) "),
nl->TextAtom("( timeInterval (<nid> <rid> <side> <pos1> <pos2> ) ) "),
nl->StringAtom("((i1 i2 TRUE FALSE) (1 1 0 0.0 0.3))"))));
}

bool UGPoint::Check(ListExpr type, ListExpr& errorInfo)
{
  return (nl->IsEqual( type, "ugpoint" ));
}

ListExpr UGPoint::Out(ListExpr typeInfo,
                      Word value)
{
  UGPoint* ugpoint = (UGPoint*)(value.addr);

  if( !(((UGPoint*)value.addr)->IsDefined()) )
  {
    return (nl->SymbolAtom("undef"));
  }
  else
  {
    ListExpr timeintervalList =
        nl->FourElemList(OutDateTime( nl->TheEmptyList(),
                                      SetWord(&ugpoint->timeInterval.start) ),
                         OutDateTime( nl->TheEmptyList(),
                                      SetWord(&ugpoint->timeInterval.end) ),
                         nl->BoolAtom( ugpoint->timeInterval.lc ),
                         nl->BoolAtom( ugpoint->timeInterval.rc));

    ListExpr pointsList =
         nl->FiveElemList(nl->IntAtom( ugpoint->p0.GetNetworkId() ),
                          nl->IntAtom( ugpoint->p0.GetRouteId() ),
                          nl->IntAtom( ugpoint->p0.GetSide() ),
                          nl->RealAtom( ugpoint->p0.GetPosition()),
                          nl->RealAtom( ugpoint->p1.GetPosition()));
      return nl->TwoElemList( timeintervalList, pointsList );
  }
}

Word UGPoint::In(const ListExpr typeInfo,
                 const ListExpr instance,
                 const int errorPos,
                 ListExpr& errorInfo,
                 bool& correct)
{
  string errmsg;
  if ( nl->ListLength( instance ) == 2 )
  {
    ListExpr first = nl->First( instance );

    if( nl->ListLength( first ) == 4 &&
        nl->IsAtom( nl->Third( first ) ) &&
        nl->AtomType( nl->Third( first ) ) == BoolType &&
        nl->IsAtom( nl->Fourth( first ) ) &&
        nl->AtomType( nl->Fourth( first ) ) == BoolType )
    {
      correct = true;
      Instant *start = (Instant *)InInstant( nl->TheEmptyList(),
       nl->First( first ),
        errorPos, errorInfo, correct ).addr;

      if( !correct )
      {
        errmsg = "InUGPoint(): Error in first instant.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        return SetWord( Address(0) );
      }

      Instant *end = (Instant *)InInstant( nl->TheEmptyList(),
       nl->Second( first ),
       errorPos, errorInfo, correct ).addr;

      if( !correct )
      {
        errmsg = "InUGPoint(): Error in second instant.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        delete end;
        return SetWord( Address(0) );
      }

      Interval<Instant> tinterval( *start, *end,
                                   nl->BoolValue( nl->Third( first ) ),
                                   nl->BoolValue( nl->Fourth( first ) ) );
      delete start;
      delete end;

      correct = tinterval.IsValid();
      if (!correct)
        {
          errmsg = "InGUPoint(): Non valid time interval.";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          return SetWord( Address(0) );
        }

      ListExpr second = nl->Second( instance );
      if( nl->ListLength( second ) == 5 &&
          nl->IsAtom( nl->First( second ) ) &&
          nl->AtomType( nl->First( second ) ) == IntType &&
          nl->IsAtom( nl->Second( second ) ) &&
          nl->AtomType( nl->Second( second ) ) == IntType &&
          nl->IsAtom( nl->Third( second ) ) &&
          nl->AtomType( nl->Third( second ) ) == IntType &&
          nl->IsAtom( nl->Fourth( second ) ) &&
          nl->AtomType( nl->Fourth( second ) ) == RealType &&
          nl->IsAtom( nl->Fifth( second ) ) &&
          nl->AtomType( nl->Fifth( second ) ) == RealType )
      {
        UGPoint *ugpoint = new UGPoint(tinterval,
                                     nl->IntValue( nl->First( second ) ),
                                     nl->IntValue( nl->Second( second ) ),
                                     (Side)nl->IntValue( nl->Third( second ) ),
                                     nl->RealValue( nl->Fourth( second ) ),
                                     nl->RealValue( nl->Fifth( second ) ));

        correct = ugpoint->IsValid();
        if( correct )
          return SetWord( ugpoint );

        errmsg = "InUGPoint(): Error in start/end point.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete ugpoint;
      }
    }
  }
  else if ( nl->IsAtom( instance ) && nl->AtomType( instance ) == SymbolType
            && nl->SymbolValue( instance ) == "undef" )
    {
      UGPoint *ugpoint = new UGPoint(true);
      ugpoint->SetDefined(false);
      ugpoint->timeInterval=
        Interval<DateTime>(DateTime(instanttype),
                           DateTime(instanttype),true,true);
      correct = ugpoint->timeInterval.IsValid();
      if ( correct )
        return (SetWord( ugpoint ));
    }
  errmsg = "InUGPoint(): Error in representation.";
  errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
  correct = false;
  return SetWord( Address(0) );
}

Word UGPoint::Create(const ListExpr typeInfo)
{
  return (SetWord( new UGPoint() ));
}

void UGPoint::Delete(const ListExpr typeInfo,
                     Word& w)
{
  delete (UGPoint *)w.addr;
  w.addr = 0;
}

void UGPoint::Close(const ListExpr typeInfo,
                    Word& w)
{
  delete (UGPoint *)w.addr;
  w.addr = 0;
}

Word UGPoint::Clone(const ListExpr typeInfo,
                    const Word& w )
{
  UGPoint *ugpoint = (UGPoint *)w.addr;
  return SetWord( new UGPoint( *ugpoint ) );
}

int UGPoint::SizeOf()
{
  return sizeof(UGPoint);
}

void* UGPoint::Cast(void* addr)
{
  return new (addr) UGPoint;
}

/*
Get methods of ~ugpoint~

*/

int UGPoint::GetUnitRid(){
  return p0.GetRouteId();
}

double UGPoint::GetUnitStartPos(){
  return p0.GetPosition();
}

double UGPoint::GetUnitEndPos(){
  return p1.GetPosition();
}

double UGPoint::GetUnitStartTime(){
  return timeInterval.start.ToDouble();
}

double UGPoint::GetUnitEndTime(){
  return timeInterval.end.ToDouble();
}

void UGPoint::Deftime(Periods &per){
  per.Clear();
  if (IsDefined()) {
    per.StartBulkLoad();
    per.Add(timeInterval);
    per.EndBulkLoad();
    per.SetDefined(true);
  } else per.SetDefined(false);
}

Instant UGPoint::TimeAtPos(double pos){
  double factor = fabs(pos - p0.GetPosition())/
                  fabs(p1.GetPosition() - p0.GetPosition());
  return (timeInterval.end - timeInterval.start) * factor + timeInterval.start;
}


/*
Computes the distance between two ~ugpoint~

*/

void UGPoint::Distance (const UGPoint &ugp, UReal &ur) const {
  assert( IsDefined() && ugp.IsDefined() );
  assert( timeInterval.Intersects(ugp.timeInterval) );
  Interval<Instant>iv;
  DateTime DT(durationtype);
  GPoint rgp10, rgp11, rgp20, rgp21;
  double
    x10, x11, x20, x21,
    y10, y11, y20, y21,
    dx1, dy1,
    dx2, dy2,
    dx12, dy12,
    dt;

  timeInterval.Intersection(ugp.timeInterval, iv);
  ur.timeInterval = iv;
  // ignore closedness for TemporalFunction:
  TemporalFunction(   iv.start, rgp10, true);
  TemporalFunction(   iv.end,   rgp11, true);
  ugp.TemporalFunction(iv.start, rgp20, true);
  ugp.TemporalFunction(iv.end,   rgp21, true);
  if (rgp10.GetRouteId() == rgp20.GetRouteId() &&
     fabs(rgp10.GetPosition()-rgp20.GetPosition()) < 0.01 &&
     fabs(rgp11.GetPosition()-rgp21.GetPosition()) < 0.01)
  { // identical points -> zero distance!
    ur.a = 0.0;
    ur.b = 0.0;
    ur.c = 0.0;
    ur.r = false;
    return;
  }
  DT = iv.end - iv.start;
  dt = DT.ToDouble();
  Network* pNetwork = NetworkManager::GetNetwork(rgp10.GetNetworkId());
  Point *rp10 = pNetwork->GetPointOnRoute(&rgp10);
  Point *rp11 = pNetwork->GetPointOnRoute(&rgp11);
  Point *rp20 = pNetwork->GetPointOnRoute(&rgp20);
  Point *rp21 = pNetwork->GetPointOnRoute(&rgp21);
  x10 = rp10->GetX(); y10 = rp10->GetY();
  x11 = rp11->GetX(); y11 = rp11->GetY();
  x20 = rp20->GetX(); y20 = rp20->GetY();
  x21 = rp21->GetX(); y21 = rp21->GetY();
  dx1 = x11 - x10;   // x-difference final-initial for u1
  dy1 = y11 - y10;   // y-difference final-initial for u1
  dx2 = x21 - x20;   // x-difference final-initial for u2
  dy2 = y21 - y20;   // y-difference final-initial for u2
  dx12 = x10 - x20;  // x-distance at initial instant
  dy12 = y10 - y20;  // y-distance at initial instant
  delete rp10;
  delete rp11;
  delete rp20;
  delete rp21;
  if ( AlmostEqual(dt, 0) )
  { // almost equal start and end time -> constant distance
    ur.a = 0.0;
    ur.b = 0.0;
    ur.c =   pow( ( (x11-x10) - (x21-x20) ) / 2, 2)
        + pow( ( (y11-y10) - (y21-y20) ) / 2, 2);
    ur.r = true;
    return;
  }

  double a1 = (pow((dx1-dx2),2)+pow(dy1-dy2,2))/pow(dt,2);
  double b1 = dx12 * (dx1-dx2);
  double b2 = dy12 * (dy1-dy2);

  ur.a = a1;
  ur.b = 2*(b1+b2)/dt;
  ur.c = pow(dx12,2) + pow(dy12,2);
  ur.r = true;
  NetworkManager::CloseNetwork(pNetwork);
  return;
}

  const Rectangle<3> UGPoint::BoundingBox()const {
   if (IsDefined()) {
      Point *pt0 = p0.ToPoint();
      Point *pt1 = p1.ToPoint();
      Rectangle<3> *res = new Rectangle<3>(true,
                                   min(pt0->GetX(), pt1->GetX()),
                                   max(pt0->GetX(), pt1->GetX()),
                                   min(pt0->GetY(), pt1->GetY()),
                                   max(pt0->GetY(), pt1->GetY()),
                                   timeInterval.start.ToDouble(),
                                   timeInterval.end.ToDouble());
      delete pt0;
      delete pt1;
      return *res;
    } else return Rectangle<3>(false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  }
 
 double UGPoint::Distance(const Rectangle<3>& rect) const{
   cerr << "Distance function not implemented yet";
   if(!IsDefined() || !rect.IsDefined()){
     return -1;
   } else {
     return BoundingBox().Distance(rect);
   }

 }

  Rectangle<3> UGPoint::BoundingBox(Network* pNetwork)const{
 if (IsDefined()) {
      Point *pt0 = p0.ToPoint(pNetwork);
      Point *pt1 = p1.ToPoint(pNetwork);
      Rectangle<3> *res = new Rectangle<3>(true,
                                   min(pt0->GetX(), pt1->GetX()),
                                   max(pt0->GetX(), pt1->GetX()),
                                   min(pt0->GetY(), pt1->GetY()),
                                   max(pt0->GetY(), pt1->GetY()),
                                   timeInterval.start.ToDouble(),
                                   timeInterval.end.ToDouble());
      delete pt0;
      delete pt1;
      return *res;
    } else return Rectangle<3>(false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  }


TypeConstructor unitgpoint(
        "ugpoint",                              // Name
        UGPoint::Property,                      // Property function
        UGPoint::Out, UGPoint::In,              // Out and In functions
        0,             0,                       // Save to and restore
                                                // from list functions
        UGPoint::Create,                        // Object creation
        UGPoint::Delete,                        // and deletion
        OpenAttribute<UGPoint>,SaveAttribute<UGPoint>, //Object open and save
        UGPoint::Close, UGPoint::Clone,         // Object close and clone
        UGPoint::Cast,                          // Cast function
        UGPoint::SizeOf,                        // Sizeof function
        UGPoint::Check);                        // Kind checking function

/*
1.2.3 ~igpoint~

*/

ListExpr IntimeGPointProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> TEMPORAL"),
                             nl->StringAtom("(igpoint) "),
                             nl->StringAtom("(instant gpoint-value)"),
                             nl->StringAtom("((instant) (1 1 1.0 2))"))));
}


bool CheckIntimeGPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "igpoint" ));
}


TypeConstructor intimegpoint(
        "igpoint",                    //name
        IntimeGPointProperty,  //property function describing signature
        OutIntime<GPoint, GPoint::OutGPoint>,
        InIntime<GPoint, GPoint::InGPoint>,         //Out and In functions
        0,
        0,       //SaveToList and RestoreFromList functions
        CreateIntime<GPoint>,
        DeleteIntime<GPoint>,              //object creation and deletion
        OpenAttribute<GPoint>,
        OpenAttribute<GPoint>,             // object open and save
        CloseIntime<GPoint>,
        CloneIntime<GPoint>,               //object close and clone
        CastIntime<GPoint>,                //cast function
        SizeOfIntime<GPoint>,              //sizeof function
        CheckIntimeGPoint );               //kind checking function

/*
1.3 Operators

1.3.1 Operator ~mpoint2mgpoint~

Translates a spatial ~MPoint~ into a network based ~MGPoint~.

*/


ListExpr OpMPoint2MGPointTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 2 ){
    sendMessages("Expects a list of length 2.");
    return (nl->SymbolAtom( "typeerror" ));
  }
  ListExpr xNetworkIdDesc = nl->First(in_xArgs);
  ListExpr xMPointDesc = nl->Second(in_xArgs);

  if( (!nl->IsAtom(xNetworkIdDesc)) ||
      !nl->IsEqual(xNetworkIdDesc, "network"))
  {
    sendMessages("Expected network as first argument.");
    return (nl->SymbolAtom("typeerror"));
  }

  if( (!nl->IsAtom( xMPointDesc )) ||
      nl->AtomType( xMPointDesc ) != SymbolType ||
      nl->SymbolValue( xMPointDesc ) != "mpoint" )
  {
    sendMessages("Expected mpoint as second argument.");
    return (nl->SymbolAtom( "typeerror" ));
  }

  return nl->SymbolAtom( "mgpoint" );
}
/*
Value mapping function of operator ~mpoint2mgpoint~

The method will first look for a segment at the start of the mpoint.

Preconditions:
- The moving point has to be exactly over the routes of the network.
- The trajectory of the mpoint must not be disjoint i.e. each unit starts
  where the one before ended.
- If the mpoint changes the route at the crossing a new unit has to be started
- A change of the direction on a route is only allowed at crossings (and only
  if the opposite lane can be reached at the crossing e.g. a u-turn is allowed).

*/

int OpMPoint2MGPointValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
/*
Initialize return value

*/
  MGPoint* pMGPoint = (MGPoint*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pMGPoint );
  MGPoint* res= new MGPoint(0);
  pMGPoint->SetDefined(true);
  res->SetDefined(true);
  // Get and check input values.
  Network *pNetwork = (Network*)args[0].addr;
  if (pNetwork->isDefined() < 1) {
    sendMessages("Network is not defined.");
    pMGPoint->SetDefined(false);
    return 0;
  }
  int iNetworkId = pNetwork->GetId();
  MPoint* pMPoint = (MPoint*)args[1].addr;
  if(pMPoint == NULL || pMPoint->GetNoComponents() < 1 ||
     !pMPoint->IsDefined()) {
    sendMessages("MPoint does not exist.");
    pMGPoint->SetDefined(false);
    return 0;
  }
  // Some initialisations
  const UPoint *pCurrentUnit;
  Point uStartPoint, uEndPoint, juncPoint, uFirstEndPoint;
  bool bStartFound, bEndFound, bMovingUp, bNewRouteFound, bDual;
  bool bMGPointCurrMovingUp, bSecondEndCheck, bSecCheckDual,bAdjSecCheck;
  bool bAdjAddCheck, bAdjSecCheckDual, blastSecEndCheck, blastAdjSecCheck;
  bool blastSecEndCheckDual, blastAdjSecCheckDual;
  CcBool *pDual;
  double dStartPosOld, difference, firstDifference, aktDifference, dFirstEndPos;
  double dStartPos, dEndPos, speedDifference, dAktUnitSpeed, dAktUnitDistance;
  double dMGPointCurrDistance, dMGPointCurrEndPos, dMGPointCurrStartPos;
  double dMGPointCurrSpeed, rid1meas, rid2meas, dSecondCheckEndPos, dSecDiff;
  double dAdjSecCheckEndPos, dFirstAdjSecCheckDiff, dAdjAddCheckEndPos;
  double dAdjSecCheckDiff, dlastSecCheckEndPos, dlastSecDiff;
  double dlastAdjSecCheckEndPos, dlastAdjSecCheckDiff;
  int iRouteId, iOldSectionTid, iCurrentSectionTid, iLastRouteId;
  int iCurrentSectionRid, iCurrentSectionRTid, iMGPointCurrRId;
  int iSecCheckRouteId, iAdjSecCheckRid, ilastSecEndCheckRouteId;
  int ilastAdjSecCheckRid;
  CcInt *pCurrentSectionRid, *pCurrentSectionRTid, *pRouteId;
  Tuple *pCurrentRoute, *pOldSectionTuple, *pCurrentSectionT, *pTestRoute;
  SimpleLine *pRouteCurve, *pLastRouteCurve, *pTestRouteCurve;
  Relation* pRoutes = pNetwork->GetRoutes();
  Relation* pSections = pNetwork->GetSectionsInternal();
  GPoint *pStartPos;
  Side side, sMGPointCurrSide;
  vector<DirectedSection> pAdjacentSections;
  Instant tMGPointCurrStartTime, tMGPointCurrEndTime, aktUnitStartTime;
  Instant aktUnitEndTime, tPassJunction;
  Instant correcture(0,1,durationtype);
  double lMGPointCurrStartTime, lMGPointCurrEndTime, lMGPointCurrDuration;
  double lAktUnitStartTime, lAktUnitEndTime, lAktUnitDuration;
  // Translate every Unit of MPoint to MGPoint Units.
  // Compute Position in Network for the (start) of the first Unit of MGPoint
  /*res->SetBBox(pMPoint->BoundingBox());
  res->SetLength(pMPoint->Length());*/
  res->StartBulkLoad();
  pMPoint->Get(0, pCurrentUnit);
  getUnitValues(pCurrentUnit, uEndPoint, uStartPoint, aktUnitStartTime,
                  aktUnitEndTime, lAktUnitStartTime, lAktUnitEndTime,
                  lAktUnitDuration);
  RouteInterval *ri = pNetwork->FindInterval(uStartPoint, uEndPoint);
  iRouteId = ri->m_iRouteId;
  pCurrentRoute = pNetwork->GetRoute(iRouteId);
  pTestRoute = pNetwork->GetRoute(iRouteId);
  dStartPos = ri->m_dStart;
  dEndPos = ri->m_dEnd;
  pDual = (CcBool*) pCurrentRoute->GetAttribute(ROUTE_DUAL);
  bDual = pDual->GetBoolval();
  if (dStartPos < dEndPos) bMovingUp = true;
  else  bMovingUp = false;
  if (bDual && bMovingUp)  side = Up;
  else {
    if (bDual && !bMovingUp)  side = Down;
    else  side = None;
  }

  /*
  GenericRelationIterator* pRoutesIt = pRoutes->MakeScan();
  while( (pCurrentRoute = pRoutesIt->GetNextTuple()) != 0 ) {
    iRouteTid = -1;
    pRouteId = (CcInt*) pCurrentRoute->GetAttribute(ROUTE_ID);
    iRouteId = pRouteId->GetIntval();
    pRouteCurve = (SimpleLine*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
    bStartFound = checkPoint(pRouteCurve, uStartPoint, true, dStartPos,
                             difference);
    if (bStartFound) {
      bEndFound = checkPoint(pRouteCurve, uEndPoint, true, dEndPos, difference);
      if (bEndFound) {
        iRouteTid = pCurrentRoute->GetTupleId();
        pDual = (CcBool*) pCurrentRoute->GetAttribute(ROUTE_DUAL);
        bDual = pDual->GetBoolval();
        if (dStartPos < dEndPos) { bMovingUp = true;}
        else { bMovingUp = false;}
        if (bDual && bMovingUp) { side = Up;}
        else {
            if (bDual && !bMovingUp) { side = Down;}
            else { side = None;}
        }
        pCurrentRoute->DeleteIfAllowed();
        if (fabs(uEndPoint.Distance(uStartPoint) -
              fabs(dEndPos-dStartPos)) > 10.0)
          continue;
        else break;
      }
    }
    pCurrentRoute->DeleteIfAllowed();
  }

  delete pRoutesIt;
  if (iRouteTid == -1 ) {
    sendMessages("First Unit of mpoint not found in network.");
    pMGPoint->EndBulkLoad();
    //delete pRoutes;
    return 0;
  }

  pCurrentRoute = pRoutes->GetTuple(iRouteTid);
  pTestRoute = pRoutes->GetTuple(iRouteTid);
  */
  pRouteCurve = (SimpleLine*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
  dAktUnitDistance = fabs (dEndPos - dStartPos);
  dAktUnitSpeed = dAktUnitDistance / lAktUnitDuration;
  iMGPointCurrRId = iRouteId;
  sMGPointCurrSide = side;
  dMGPointCurrSpeed = dAktUnitSpeed;
  tMGPointCurrStartTime = aktUnitStartTime;
  tMGPointCurrEndTime = aktUnitEndTime;
  dMGPointCurrStartPos = dStartPos;
  dMGPointCurrEndPos = dEndPos;
  bMGPointCurrMovingUp = bMovingUp;
  dMGPointCurrDistance = dAktUnitDistance;
  lMGPointCurrStartTime = lAktUnitStartTime;
  lMGPointCurrEndTime= lAktUnitEndTime;
  lMGPointCurrDuration = lAktUnitDuration;
  dStartPos = dEndPos;
  uStartPoint = uEndPoint;
  for (int i=1; i < pMPoint->GetNoComponents() ; i++){
    bSecondEndCheck = false;
    blastSecEndCheck = false;
    bAdjSecCheck = false;
    blastAdjSecCheck = false;
    bAdjAddCheck = false;
    bNewRouteFound = false;
    pMPoint->Get(i, pCurrentUnit);
    getUnitValues(pCurrentUnit, uEndPoint, uStartPoint, aktUnitStartTime,
                  aktUnitEndTime, lAktUnitStartTime, lAktUnitEndTime,
                  lAktUnitDuration);
    if (aktUnitStartTime < tMGPointCurrEndTime) {
       aktUnitStartTime = tMGPointCurrEndTime;
       lAktUnitStartTime = aktUnitStartTime.ToDouble();
       if (aktUnitEndTime < aktUnitStartTime) {
         aktUnitEndTime = aktUnitStartTime + correcture;
         lAktUnitEndTime = aktUnitEndTime.ToDouble();
       }
       lAktUnitDuration = lAktUnitEndTime - lAktUnitStartTime;
    }
    bEndFound = checkPoint(pRouteCurve, uEndPoint, true, dEndPos,
                           firstDifference);
    if (!bEndFound) {
      bSecondEndCheck = checkPoint03(pRouteCurve, uEndPoint, true,
                                   dSecondCheckEndPos, dSecDiff);
      if (bSecondEndCheck) {
        bSecCheckDual = bDual;
        iSecCheckRouteId = pCurrentRoute->GetTupleId();
      } else {
        blastSecEndCheck = lastcheckPoint03 (pRouteCurve, uEndPoint, true,
                                   dlastSecCheckEndPos, dlastSecDiff);
        if (blastSecEndCheck) {
          blastSecEndCheckDual = bDual;
          ilastSecEndCheckRouteId = pCurrentRoute->GetTupleId();
        }
      }
    }
    if (bEndFound &&
       (fabs(uEndPoint.Distance(uStartPoint) -
          fabs(dEndPos-dStartPos)) < 10.0)) {
/*
Moving point stays on the same route.
Check if the direction,speed and side are the same as before. If this is the
case expand the current unit of the mgpoint for the current unit of the mpoint.
In the other case close and write the open unit of the mgpoint and start a new
mgpoint unit with the values of the actual mpoint unit as start values.

*/
          bNewRouteFound = true;
          setMoveAndSide(dStartPos, dEndPos, bMovingUp, bDual, side);
          dAktUnitDistance = fabs (dEndPos - dStartPos);
          dAktUnitSpeed = dAktUnitDistance / lAktUnitDuration;
          speedDifference = fabs (dAktUnitSpeed - dMGPointCurrSpeed);
          if (bMGPointCurrMovingUp == bMovingUp &&
              sMGPointCurrSide == side &&
              speedDifference < 0.0000000001){
            tMGPointCurrEndTime = aktUnitEndTime;
            dMGPointCurrEndPos = dEndPos;
            dMGPointCurrDistance =
                fabs(dMGPointCurrEndPos - dMGPointCurrStartPos);
            lMGPointCurrEndTime= tMGPointCurrEndTime.ToDouble();
            lMGPointCurrDuration = lMGPointCurrEndTime - lMGPointCurrStartTime;
            dMGPointCurrSpeed = dMGPointCurrDistance / lMGPointCurrDuration;
            dStartPos = dEndPos;
            uStartPoint = uEndPoint;
          } else {
            res->Add(UGPoint(Interval<Instant>(tMGPointCurrStartTime,
                                               tMGPointCurrEndTime,
                                               true,
                                               false),
                             iNetworkId,
                             iMGPointCurrRId,
                             sMGPointCurrSide,
                             dMGPointCurrStartPos,
                             dMGPointCurrEndPos)/*, false*/);
            tMGPointCurrStartTime = aktUnitStartTime;
            tMGPointCurrEndTime = aktUnitEndTime;
            dMGPointCurrStartPos = dStartPos;
            dMGPointCurrEndPos = dEndPos;
            bMGPointCurrMovingUp = bMovingUp;
            dMGPointCurrDistance = dAktUnitDistance;
            lMGPointCurrStartTime= lAktUnitStartTime;
            lMGPointCurrEndTime= lAktUnitEndTime;
            lMGPointCurrDuration = lAktUnitDuration;
            iMGPointCurrRId = iRouteId;
            sMGPointCurrSide = side;
            dMGPointCurrSpeed = dAktUnitSpeed;
            dStartPos = dEndPos;
            uStartPoint = uEndPoint;
            continue;
          }
        continue;
    } else {
/*
End not on the  same Route!
The ~mpoint~ changed route at the start of the unit.
Write the open unit to the mgpoint and compute the start of the new mgpoint
unit. To find the new route use the adjacent segments of the actual segment. If
the new Route is found compute the new values for the next mgpoint unit.

*/

        res->Add(UGPoint(Interval<Instant>(tMGPointCurrStartTime,
                                                tMGPointCurrEndTime,
                                                true,
                                                false),
                                iNetworkId,
                                iMGPointCurrRId,
                                sMGPointCurrSide,
                                dMGPointCurrStartPos,
                                dMGPointCurrEndPos)/*, false*/);
        bNewRouteFound = false;
        if (bMovingUp && dStartPos > 0.1) {
          pStartPos = new GPoint(true, iNetworkId, iRouteId, dStartPos-0.1);}
        else {
          if (!bMovingUp && dStartPos < pRouteCurve->Length()- 0.1){
            pStartPos = new GPoint(true, iNetworkId, iRouteId, dStartPos+0.1);}
          else {
            if (!bMovingUp && fabs(dStartPos-pRouteCurve->Length())<0.1) {
              pStartPos = new GPoint(true, iNetworkId, iRouteId, dStartPos-0.1);
              if (dMGPointCurrSpeed == 0) bMovingUp = true;
            }
            else {
              if (bMovingUp && fabs(dStartPos - 0.0) < 0.1) {
                pStartPos =
                    new GPoint(true, iNetworkId, iRouteId, dStartPos+0.1);
                if (dMGPointCurrSpeed == 0) bMovingUp = false;
              }
              else {
              pStartPos = new GPoint(true, iNetworkId, iRouteId, dStartPos);}
            }
          }
        }
        pOldSectionTuple = pNetwork->GetSectionOnRoute(pStartPos);
        pStartPos->DeleteIfAllowed();
        if (pOldSectionTuple == NULL) {
            sendMessages("No last section for adjacent sections found.");
            res->EndBulkLoad(true/*, false*/);
            result = SetWord(res);
            pCurrentRoute->DeleteIfAllowed();
            pTestRoute->DeleteIfAllowed();
            //delete pRoutes;
            return 0;
        }
        iOldSectionTid = pOldSectionTuple->GetTupleId();
        pLastRouteCurve = pRouteCurve;
        iLastRouteId = iMGPointCurrRId;
        dStartPosOld = dStartPos;
        pOldSectionTuple->DeleteIfAllowed();
        pAdjacentSections.clear();
        pNetwork->GetAdjacentSections(iOldSectionTid,
                                      bMovingUp,
                                      pAdjacentSections);
        if (pAdjacentSections.size() == 0 && pRouteCurve->IsCycle()) {
          pAdjacentSections.clear();
          pStartPos = new GPoint(true, iNetworkId, iRouteId, 0.1);
          pOldSectionTuple = pNetwork->GetSectionOnRoute(pStartPos);
          pStartPos->DeleteIfAllowed();
          if (pOldSectionTuple == NULL) {
            sendMessages("No section on cylce found");
            res->EndBulkLoad(true/*, false*/);
            result = SetWord(res);
            pCurrentRoute->DeleteIfAllowed();
            pTestRoute->DeleteIfAllowed();
            //delete pRoutes;
            return 0;
          }
          iOldSectionTid = pOldSectionTuple->GetTupleId();
          pOldSectionTuple->DeleteIfAllowed();
          pNetwork->GetAdjacentSections(iOldSectionTid, false,
                                        pAdjacentSections);
        }
        for (size_t i = 0; i < pAdjacentSections.size(); i++) {
          DirectedSection pCurrentDirectedSection = pAdjacentSections[i];
          iCurrentSectionTid = pCurrentDirectedSection.GetSectionTid();
          pCurrentSectionT = pSections->GetTuple(iCurrentSectionTid);
          pCurrentSectionRid =
              (CcInt*) pCurrentSectionT->GetAttribute(SECTION_RID);
          iCurrentSectionRid = pCurrentSectionRid->GetIntval();
          pCurrentSectionRTid =
              (CcInt*) pCurrentSectionT->GetAttribute(SECTION_RRC);
          iCurrentSectionRTid = pCurrentSectionRTid->GetIntval();
          pCurrentSectionT->DeleteIfAllowed();

          if (iCurrentSectionRid != iRouteId) {
            pCurrentRoute->DeleteIfAllowed();
            pCurrentRoute = pRoutes->GetTuple(iCurrentSectionRTid);
            pRouteCurve =
                (SimpleLine*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
            bEndFound = checkPoint(pRouteCurve, uEndPoint, true, dEndPos,
                                  aktDifference);
            if (!bEndFound) {
              if (!bAdjSecCheck) {
                bAdjSecCheck = checkPoint03(pRouteCurve, uEndPoint, true,
                                            dAdjSecCheckEndPos,
                                            dFirstAdjSecCheckDiff);
                if (bAdjSecCheck) {
                  bAdjSecCheckDual = bDual;
                  iAdjSecCheckRid = pCurrentRoute->GetTupleId();
                } else {
                  blastAdjSecCheck = lastcheckPoint03(pRouteCurve, uEndPoint,
                                                      true,
                                                      dlastAdjSecCheckEndPos,
                                                      dlastAdjSecCheckDiff);
                  if (blastAdjSecCheck) {
                    blastAdjSecCheckDual = bDual;
                    ilastAdjSecCheckRid = pCurrentRoute->GetTupleId();
                  }
                }
              } else {
                bAdjAddCheck = checkPoint03(pRouteCurve, uEndPoint, true,
                                          dAdjAddCheckEndPos, dAdjSecCheckDiff);
                if (bAdjAddCheck && dFirstAdjSecCheckDiff > dAdjSecCheckDiff) {
                  dAdjSecCheckEndPos = dAdjAddCheckEndPos;
                  dFirstAdjSecCheckDiff = dAdjSecCheckDiff;
                  bAdjSecCheckDual = bDual;
                  iAdjSecCheckRid = pCurrentRoute->GetTupleId();
                }
              }
            }
            if (bEndFound) {
              bStartFound =
                  checkPoint(pRouteCurve, uStartPoint, true, dStartPos,
                             difference);
              if (bStartFound) {
                  bNewRouteFound = true;
                  pDual = (CcBool*) pCurrentRoute->GetAttribute(ROUTE_DUAL);
                  bDual = pDual->GetBoolval();
                  setMoveAndSide(dStartPos, dEndPos, bMovingUp, bDual, side);
                  iRouteId = iCurrentSectionRid;
                  pTestRoute->DeleteIfAllowed();
                  pTestRoute = pRoutes->GetTuple(iCurrentSectionRTid);
                  pTestRouteCurve =
                      (SimpleLine*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
                  firstDifference = aktDifference;
                  dAktUnitDistance = fabs (dEndPos - dStartPos);
                  dAktUnitSpeed = dAktUnitDistance / lAktUnitDuration;
                  iMGPointCurrRId = iRouteId;
                  sMGPointCurrSide = side;
                  dMGPointCurrSpeed = dAktUnitSpeed;
                  tMGPointCurrStartTime = aktUnitStartTime;
                  tMGPointCurrEndTime = aktUnitEndTime;
                  dMGPointCurrStartPos = dStartPos;
                  dMGPointCurrEndPos = dEndPos;
                  bMGPointCurrMovingUp = bMovingUp;
                  dMGPointCurrDistance = dAktUnitDistance;
                  lMGPointCurrStartTime = lAktUnitStartTime;
                  lMGPointCurrEndTime= lAktUnitEndTime;
                  lMGPointCurrDuration = lAktUnitDuration;
                  dFirstEndPos = dEndPos;
                  uFirstEndPoint = uEndPoint;
/*
If a route splits up into two routes it might be that in the beginning
the difference between the two new routes is very small, So it can happen
that the checkPoint-Function first computes the false new route. Because of this
we have to check the other adjacent routes if there is one with a lower
difference than the first found route. If a route with a lower difference value
is found. This route has to be taken instead of the first computed route.

*/
                  for (size_t j = i ; j < pAdjacentSections.size(); j++) {
                    DirectedSection pCurrentDirectedSection =
                        pAdjacentSections[j];
                    iCurrentSectionTid =
                        pCurrentDirectedSection.GetSectionTid();
                    pCurrentSectionT = pSections->GetTuple(iCurrentSectionTid);
                    pCurrentSectionRid =
                        (CcInt*) pCurrentSectionT->GetAttribute(SECTION_RID);
                    iCurrentSectionRid = pCurrentSectionRid->GetIntval();
                    pCurrentSectionRTid =
                        (CcInt*) pCurrentSectionT->GetAttribute(SECTION_RRC);
                    iCurrentSectionRTid = pCurrentSectionRTid->GetIntval();
                    pCurrentSectionT->DeleteIfAllowed();
                    if (iCurrentSectionRid != iMGPointCurrRId) {
                      pTestRoute->DeleteIfAllowed();
                      pTestRoute = pRoutes->GetTuple(iCurrentSectionRTid);
                      pTestRouteCurve =
                          (SimpleLine*) pTestRoute->GetAttribute(ROUTE_CURVE);
                      bEndFound = checkPoint(pTestRouteCurve, uEndPoint, true,
                                            dEndPos, aktDifference);
                      if (bEndFound) {
                        bStartFound = checkPoint(pTestRouteCurve, uStartPoint,
                                                true, dStartPos, difference);
                        if (bStartFound) {
                          if (firstDifference > aktDifference) {
                            pDual =
                              (CcBool*) pCurrentRoute->GetAttribute(ROUTE_DUAL);
                            bDual = pDual->GetBoolval();
                            setMoveAndSide(dStartPos, dEndPos, bMovingUp, bDual,
                                          side);
                            iRouteId = iCurrentSectionRid;
                            pCurrentRoute->DeleteIfAllowed();
                            pCurrentRoute =
                                pRoutes->GetTuple(iCurrentSectionRTid);
                            pRouteCurve =
                              (SimpleLine*) pCurrentRoute->
                                GetAttribute(ROUTE_CURVE);
                            firstDifference = aktDifference;
                            dAktUnitDistance = fabs (dEndPos - dStartPos);
                            dAktUnitSpeed = dAktUnitDistance / lAktUnitDuration;
                            iMGPointCurrRId = iRouteId;
                            sMGPointCurrSide = side;
                            dMGPointCurrSpeed = dAktUnitSpeed;
                            dMGPointCurrStartPos = dStartPos;
                            dMGPointCurrEndPos = dEndPos;
                            bMGPointCurrMovingUp = bMovingUp;
                            dMGPointCurrDistance = dAktUnitDistance;
                            dFirstEndPos = dEndPos;
                            uFirstEndPoint = uEndPoint;
                          }
                        }
                      }
                    }
                  }
                  dStartPos = dFirstEndPos;
                  uStartPoint = uFirstEndPoint;
                  break;
              } else {
/*
Specialcase; ~mpoint~ passes junction within unit changing the route.
Might happen if old Route and new Route have the same direction.
Compute additional Unit for MGPoint from end last Unit to junction and correct
values for next unit of MGPoint taking the time of passing the junction as
start time and the junction as startposition of the new current unit.

*/

                CcInt pRouteId1(true, iLastRouteId);
                CcInt pRouteId2(true, iCurrentSectionRid);
                pNetwork->GetJunctionMeasForRoutes(&pRouteId1,
                                                  &pRouteId2,
                                                  rid1meas,
                                                  rid2meas);
                double factor = fabs(rid1meas - dStartPosOld) /
                                (uEndPoint.Distance(uStartPoint));
                tPassJunction = (aktUnitEndTime - aktUnitStartTime) * factor +
                                aktUnitStartTime;
                if (tPassJunction == aktUnitStartTime)
                    tPassJunction = tPassJunction + correcture;
                else if (tPassJunction < aktUnitStartTime)
                        tPassJunction = aktUnitStartTime + correcture;
                res->Add(UGPoint(Interval<Instant>(aktUnitStartTime,
                                                tPassJunction,
                                                true,
                                                false),
                                iNetworkId,
                                iLastRouteId,
                                sMGPointCurrSide,
                                dMGPointCurrEndPos,
                                rid1meas)/*, false*/);
                bNewRouteFound = true;
                pDual = (CcBool*) pCurrentRoute->GetAttribute(ROUTE_DUAL);
                bDual = pDual->GetBoolval();
                setMoveAndSide(rid2meas, dEndPos, bMovingUp, bDual, side);
                iRouteId = iCurrentSectionRid;
                dAktUnitDistance = fabs (dEndPos - rid2meas);
                if (tPassJunction == aktUnitEndTime)
                    aktUnitEndTime = aktUnitEndTime + correcture;
                else if (tPassJunction > aktUnitEndTime)
                        aktUnitEndTime = tPassJunction + correcture;
                lAktUnitEndTime = aktUnitEndTime.ToDouble();
                lAktUnitDuration = lAktUnitEndTime -
                                            tPassJunction.ToDouble();
                dAktUnitSpeed = dAktUnitDistance / lAktUnitDuration;
                iMGPointCurrRId = iRouteId;
                sMGPointCurrSide = side;
                dMGPointCurrSpeed = dAktUnitSpeed;
                tMGPointCurrStartTime = tPassJunction;
                tMGPointCurrEndTime = aktUnitEndTime;
                dMGPointCurrStartPos = rid2meas;
                dMGPointCurrEndPos = dEndPos;
                bMGPointCurrMovingUp = bMovingUp;
                dMGPointCurrDistance = dAktUnitDistance;
                lMGPointCurrStartTime= tPassJunction.ToDouble();
                lMGPointCurrEndTime= lAktUnitEndTime;
                lMGPointCurrDuration = lAktUnitDuration;
                dStartPos = dEndPos;
                uStartPoint = uEndPoint;
                break;
              }//end ifelse bStartFound
            } // end if bEndFound
          } //end if CurrentRid != RouteId
        } //end for AdjacentSections
      } //end ifelse bEndFound
      if (!bNewRouteFound && bSecondEndCheck) {
          pCurrentRoute->DeleteIfAllowed();
          pCurrentRoute = pRoutes->GetTuple(iSecCheckRouteId);
          pRouteCurve = (SimpleLine*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
          pRouteId = (CcInt*) pCurrentRoute->GetAttribute(ROUTE_ID);
          iRouteId = pRouteId->GetIntval();
          pDual = (CcBool*) pCurrentRoute->GetAttribute(ROUTE_DUAL);
          bDual = pDual->GetBoolval();
          bNewRouteFound = true;
          dEndPos = dSecondCheckEndPos;
          setMoveAndSide(dStartPos, dEndPos, bMovingUp, bDual, side);
          dAktUnitDistance = fabs (dEndPos - dStartPos);
          dAktUnitSpeed = dAktUnitDistance / lAktUnitDuration;
          iMGPointCurrRId = iRouteId;
          sMGPointCurrSide = side;
          dMGPointCurrSpeed = dAktUnitSpeed;
          tMGPointCurrStartTime = aktUnitStartTime;
          tMGPointCurrEndTime = aktUnitEndTime;
          dMGPointCurrStartPos = dStartPos;
          dMGPointCurrEndPos = dEndPos;
          bMGPointCurrMovingUp = bMovingUp;
          dMGPointCurrDistance = dAktUnitDistance;
          lMGPointCurrStartTime = lAktUnitStartTime;
          lMGPointCurrEndTime= lAktUnitEndTime;
          lMGPointCurrDuration = lAktUnitDuration;
          dStartPos = dEndPos;
          uStartPoint = uEndPoint;
          continue;
        }
        if (!bNewRouteFound && bAdjSecCheck) {
          pCurrentRoute->DeleteIfAllowed();
          pCurrentRoute = pRoutes->GetTuple(iAdjSecCheckRid);
          pRouteCurve = (SimpleLine*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
          pRouteId = (CcInt*) pCurrentRoute->GetAttribute(ROUTE_ID);
          iRouteId = pRouteId->GetIntval();
          pDual = (CcBool*) pCurrentRoute->GetAttribute(ROUTE_DUAL);
          bDual = pDual->GetBoolval();
          bNewRouteFound = true;
          dEndPos = dAdjSecCheckEndPos;
          bStartFound = checkPoint(pRouteCurve, uStartPoint, true, dStartPos,
                                   difference);
          if (!bStartFound) {
            checkPoint03(pRouteCurve, uStartPoint, true, dStartPos, difference);
          }
          setMoveAndSide(dStartPos, dEndPos, bMovingUp, bDual, side);
          dAktUnitDistance = fabs (dEndPos - dStartPos);
          dAktUnitSpeed = dAktUnitDistance / lAktUnitDuration;
          iMGPointCurrRId = iRouteId;
          sMGPointCurrSide = side;
          dMGPointCurrSpeed = dAktUnitSpeed;
          tMGPointCurrStartTime = aktUnitStartTime;
          tMGPointCurrEndTime = aktUnitEndTime;
          dMGPointCurrStartPos = dStartPos;
          dMGPointCurrEndPos = dEndPos;
          bMGPointCurrMovingUp = bMovingUp;
          dMGPointCurrDistance = dAktUnitDistance;
          lMGPointCurrStartTime = lAktUnitStartTime;
          lMGPointCurrEndTime= lAktUnitEndTime;
          lMGPointCurrDuration = lAktUnitDuration;
          dStartPos = dEndPos;
          uStartPoint = uEndPoint;
          continue;
        }
      if (!bNewRouteFound) {
        pAdjacentSections.clear();
        pNetwork->GetAdjacentSections(iOldSectionTid, !bMovingUp,
                                        pAdjacentSections);
        for (size_t i = 0; i < pAdjacentSections.size(); i++) {
          DirectedSection pCurrentDirectedSection = pAdjacentSections[i];
          iCurrentSectionTid = pCurrentDirectedSection.GetSectionTid();
          pCurrentSectionT = pSections->GetTuple(iCurrentSectionTid);
          pCurrentSectionRid =
            (CcInt*) pCurrentSectionT->GetAttribute(SECTION_RID);
          iCurrentSectionRid = pCurrentSectionRid->GetIntval();
          pCurrentSectionRTid =
            (CcInt*) pCurrentSectionT->GetAttribute(SECTION_RRC);
          iCurrentSectionRTid = pCurrentSectionRTid->GetIntval();
          pCurrentSectionT->DeleteIfAllowed();
          if (iCurrentSectionRid != iRouteId) {
            pCurrentRoute->DeleteIfAllowed();
            pCurrentRoute = pRoutes->GetTuple(iCurrentSectionRTid);
            pRouteCurve =
                (SimpleLine*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
            bEndFound = checkPoint(pRouteCurve, uEndPoint, true, dEndPos,
                                 aktDifference);
            if (bEndFound) {
              bNewRouteFound = true;
              CcInt pRouteId1(true, iLastRouteId);
              CcInt pRouteId2(true, iCurrentSectionRid);
              pNetwork->GetJunctionMeasForRoutes(&pRouteId1,
                                                  &pRouteId2,
                                                  rid1meas,
                                                  rid2meas);
              double factor = fabs(rid1meas - dStartPosOld) /
                                (uEndPoint.Distance(uStartPoint));
              tPassJunction = (aktUnitEndTime - aktUnitStartTime) * factor +
                                aktUnitStartTime;
              if (tPassJunction <= aktUnitStartTime) {
                if (tPassJunction < aktUnitStartTime) {
                  tPassJunction = aktUnitStartTime + correcture;
                } else {
                  tPassJunction = tPassJunction + correcture;
                }
              }
              res->Add(UGPoint(Interval<Instant>(aktUnitStartTime,
                                                tPassJunction,
                                                true,
                                                false),
                                iNetworkId,
                                iLastRouteId,
                                sMGPointCurrSide,
                                dMGPointCurrEndPos,
                                rid1meas)/*, false*/);
              bNewRouteFound = true;
              pDual = (CcBool*) pCurrentRoute->GetAttribute(ROUTE_DUAL);
              bDual = pDual->GetBoolval();
              setMoveAndSide(rid2meas, dEndPos, bMovingUp, bDual, side);
              iRouteId = iCurrentSectionRid;
              dAktUnitDistance = fabs (dEndPos - rid2meas);
              if (tPassJunction >= aktUnitEndTime) {
                if (tPassJunction > aktUnitEndTime) {
                  aktUnitEndTime = tPassJunction + correcture;
                } else {
                    aktUnitEndTime = aktUnitEndTime + correcture;
                }
              }
              lAktUnitEndTime = aktUnitEndTime.ToDouble();
              lAktUnitDuration = lAktUnitEndTime -
                                          tPassJunction.ToDouble();
              dAktUnitSpeed = dAktUnitDistance / lAktUnitDuration;
              iMGPointCurrRId = iRouteId;
              sMGPointCurrSide = side;
              dMGPointCurrSpeed = dAktUnitSpeed;
              tMGPointCurrStartTime = tPassJunction;
              tMGPointCurrEndTime = aktUnitEndTime;
              dMGPointCurrStartPos = rid2meas;
              dMGPointCurrEndPos = dEndPos;
              bMGPointCurrMovingUp = bMovingUp;
              dMGPointCurrDistance = dAktUnitDistance;
              lMGPointCurrStartTime= tPassJunction.ToDouble();
              lMGPointCurrEndTime= lAktUnitEndTime;
              lMGPointCurrDuration = lAktUnitDuration;
              dStartPos = dEndPos;
              uStartPoint = uEndPoint;
              break;
            }
          }
        } //end for adjacentSections
      }
      if (!bNewRouteFound && blastSecEndCheck) {
          pCurrentRoute->DeleteIfAllowed();
          pCurrentRoute = pRoutes->GetTuple(ilastSecEndCheckRouteId);
          pRouteCurve = (SimpleLine*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
          pRouteId = (CcInt*) pCurrentRoute->GetAttribute(ROUTE_ID);
          iRouteId = pRouteId->GetIntval();
          pDual = (CcBool*) pCurrentRoute->GetAttribute(ROUTE_DUAL);
          bDual = pDual->GetBoolval();
          bNewRouteFound = true;
          dEndPos = dlastSecCheckEndPos;
          setMoveAndSide(dStartPos, dEndPos, bMovingUp, bDual, side);
          dAktUnitDistance = fabs (dEndPos - dStartPos);
          dAktUnitSpeed = dAktUnitDistance / lAktUnitDuration;
          iMGPointCurrRId = iRouteId;
          sMGPointCurrSide = side;
          dMGPointCurrSpeed = dAktUnitSpeed;
          tMGPointCurrStartTime = aktUnitStartTime;
          tMGPointCurrEndTime = aktUnitEndTime;
          dMGPointCurrStartPos = dStartPos;
          dMGPointCurrEndPos = dEndPos;
          bMGPointCurrMovingUp = bMovingUp;
          dMGPointCurrDistance = dAktUnitDistance;
          lMGPointCurrStartTime = lAktUnitStartTime;
          lMGPointCurrEndTime= lAktUnitEndTime;
          lMGPointCurrDuration = lAktUnitDuration;
          dStartPos = dEndPos;
          uStartPoint = uEndPoint;
          continue;
        }
        if (!bNewRouteFound && blastAdjSecCheck) {
          pCurrentRoute->DeleteIfAllowed();
          pCurrentRoute = pRoutes->GetTuple(ilastAdjSecCheckRid);
          pRouteCurve = (SimpleLine*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
          pRouteId = (CcInt*) pCurrentRoute->GetAttribute(ROUTE_ID);
          iRouteId = pRouteId->GetIntval();
          pDual = (CcBool*) pCurrentRoute->GetAttribute(ROUTE_DUAL);
          bDual = pDual->GetBoolval();
          bNewRouteFound = true;
          dEndPos = dlastAdjSecCheckEndPos;
          bStartFound = checkPoint(pRouteCurve, uStartPoint, true, dStartPos,
                                   difference);
          if (!bStartFound) {
            checkPoint03(pRouteCurve, uStartPoint, true, dStartPos, difference);
          }
          setMoveAndSide(dStartPos, dEndPos, bMovingUp, bDual, side);
          dAktUnitDistance = fabs (dEndPos - dStartPos);
          dAktUnitSpeed = dAktUnitDistance / lAktUnitDuration;
          iMGPointCurrRId = iRouteId;
          sMGPointCurrSide = side;
          dMGPointCurrSpeed = dAktUnitSpeed;
          tMGPointCurrStartTime = aktUnitStartTime;
          tMGPointCurrEndTime = aktUnitEndTime;
          dMGPointCurrStartPos = dStartPos;
          dMGPointCurrEndPos = dEndPos;
          bMGPointCurrMovingUp = bMovingUp;
          dMGPointCurrDistance = dAktUnitDistance;
          lMGPointCurrStartTime = lAktUnitStartTime;
          lMGPointCurrEndTime= lAktUnitEndTime;
          lMGPointCurrDuration = lAktUnitDuration;
          dStartPos = dEndPos;
          uStartPoint = uEndPoint;
          continue;
        }
      if (!bNewRouteFound) {
          //should not happen
          sendMessages("MPoint is not longer found on network.");
          res->EndBulkLoad(true/*, false*/);
          result = SetWord(res);
          pCurrentRoute->DeleteIfAllowed();
          pTestRoute->DeleteIfAllowed();
          //delete pRoutes;
          return 0;
      }
  } //end for units MPoint
  res->Add(UGPoint(Interval<Instant>(tMGPointCurrStartTime,
                                                tMGPointCurrEndTime,
                                                true,
                                                false),
                              iNetworkId,
                              iMGPointCurrRId,
                              sMGPointCurrSide,
                              dMGPointCurrStartPos,
                              dMGPointCurrEndPos)/*, false*/);
  res->EndBulkLoad(true/*, false*/);
  result = SetWord(res);
  pCurrentRoute->DeleteIfAllowed();
  pTestRoute->DeleteIfAllowed();
  //pRoutes->Delete();
  pAdjacentSections.clear();
  // delete &pAdjacentSections;
  return 0;
} //end ValueMapping


const string OpMPoint2MGPointSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>network x mpoint -> mgpoint" "</text--->"
  "<text>mpoint2mgpoint(Networkobject, mpoint)</text--->"
  "<text>Finds a path in a network for a moving point.</text--->"
  "<text>mpoint2mgpoint(B_NETWORK, x)</text--->"
  ") )";

Operator mpoint2mgpoint (
          "mpoint2mgpoint",                // name
          OpMPoint2MGPointSpec,          // specification
          OpMPoint2MGPointValueMapping,  // value mapping
          Operator::SimpleSelect,          // trivial selection function
          OpMPoint2MGPointTypeMap        // type mapping
);

/*
1.3.2 Operator ~passes~

Returns true if a ~MGPoint~ passes a given ~GPoint~ or ~GLine~.

*/

ListExpr OpPassesTypeMap( ListExpr args )
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

int OpPasses_mgpgp(Word* args,
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
  pPasses->Set(true, pMGPoint->Passes(pGPoint));
  return 0;
};

int OpPasses_mgpgl(Word* args,
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
  pPasses->Set(true, pMGPoint->Passes(pGLine));
  return 0;
};

ValueMapping OpPassesvaluemap[] = {
  OpPasses_mgpgp,
  OpPasses_mgpgl
};

int OpPassesSelect( ListExpr args )
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

const string OpPassesSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint x A -> bool for A gpoint or gline " "</text--->"
  "<text>_ passes _</text--->"
  "<text>Checks whether a moving point passes a gpoint or a gline.</text--->"
  "<text>X_MGPOINT passes X_GPOINT </text--->"
  ") )";

Operator tempnetpasses (
  "passes",
  OpPassesSpec,
  2,
  OpPassesvaluemap,
  OpPassesSelect,
  OpPassesTypeMap );


/*
1.3.3 Operator ~simplify~

Reduces units of a ~MGPoint~ by concatenation if the speed difference is
smaller than a given value.

*/

ListExpr OpSimplifyTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 2 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMPointDesc = nl->First(in_xArgs);
  ListExpr xEpsilonDesc = nl->Second(in_xArgs);

  if( (!nl->IsAtom( xMPointDesc )) ||
      nl->AtomType( xMPointDesc ) != SymbolType ||
      nl->SymbolValue( xMPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }

  if( (!nl->IsAtom( xEpsilonDesc)) ||
      nl->AtomType( xEpsilonDesc ) != SymbolType ||
      nl->SymbolValue( xEpsilonDesc ) != "real" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }

  return nl->SymbolAtom( "mgpoint" );
}

int OpSimplifyValueMapping(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier in_xSupplier)
{
  MGPoint* pMGPoint = (MGPoint*)args[0].addr;
  CcReal* pEpsilon = (CcReal*)args[1].addr;
  double dEpsilon = pEpsilon->GetRealval();

  // Get (empty) return value
  MGPoint* pMGPointSimplified = (MGPoint*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pMGPointSimplified );

  if(pMGPoint->GetNoComponents() == 0)
  {
    string strMessage = "MGPoint is Empty.";
    cerr << endl << strMessage << endl << endl;
    return 0;
  }
  (*pMGPointSimplified) = *(pMGPoint->Simplify(dEpsilon));
  return 0;
}

const string OpSimplifySpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint x real -> mgpoint" "</text--->"
  "<text>simplify(0.0001, mgpoint)</text--->"
  "<text>Removes unecessary units from a mgpoint.</text--->"
  "<text>simplify(0.0001, mgpoint)</text--->"
  ") )";

Operator simplify("simplify",
                  OpSimplifySpec,
                  OpSimplifyValueMapping,
                  Operator::SimpleSelect,
                  OpSimplifyTypeMap );


/*
1.3.4 Operator ~at~

Restricts the ~MGPoint~ to the times it was at a given ~GPoint~ or ~GLine~.

*/

ListExpr OpAtTypeMap(ListExpr in_xArgs)
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

int OpAt_mgpgp(Word* args,
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
  pMGPoint->At(pGPoint, pResult);
  return 0;
};



int OpAt_mgpgl(Word* args,
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
  if(pGLine == NULL || !pGLine->IsDefined() || pGLine->NoOfComponents() <= 0) {
    cerr << "GLine does not exist or is empty." << endl;
    pResult->SetDefined(false);
    return 0;
  }
  pMGPoint->At(pGLine, pResult);
  return 0;
};

ValueMapping OpAtValueMap[] = {
  OpAt_mgpgp,
  OpAt_mgpgl
};

int OpAtSelect( ListExpr args )
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

const string OpAtSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint x A -> mgpoint if A is gpoint or gline" "</text--->"
  "<text> _ at _ </text--->"
  "<text>Restricts the moving gpoint to the times he was at"
    " the gpoint or gline.</text--->"
  "<text> X_MGPOINT at X_GPOINT</text--->"
  ") )";

Operator tempnetat("at",
                OpAtSpec,
                2,
                OpAtValueMap,
                OpAtSelect,
                OpAtTypeMap );

/*
1.3.5 Operator ~atinstant~

Restricts the ~MGPoint~ to a given time instant.

*/

ListExpr OpAtinstantTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 2 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);
  ListExpr xInstant = nl->Second(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }

  if (!nl->IsEqual( xInstant, "instant" ))
  {
    return (nl->SymbolAtom( "typeerror" ));
  }

  return nl->SymbolAtom("igpoint");
}

const string OpAtinstantSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint x instant -> igpoint" "</text--->"
  "<text> _ atinstant _</text--->"
  "<text>Computes the position of a moving gpoint at a given instant.</text--->"
  "<text>X_MGPOINT atinstant TIME </text--->"
  ") )";

Operator tempnetatinstant("atinstant",
                OpAtinstantSpec,
                MappingAtInstant<MGPoint, GPoint>,
                Operator::SimpleSelect,
                OpAtinstantTypeMap );


/*
1.3.6 Operator ~atperiods~

Restricts a ~MGPoint~ to the given periods.

*/

ListExpr OpAtperiodsTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 2 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);
  ListExpr xPeriods = nl->Second(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }

  if (!nl->IsEqual( xPeriods, "periods" ))
  {
    return (nl->SymbolAtom( "typeerror" ));
  }

  return nl->SymbolAtom("mgpoint");
}

int OpAtperiodsValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  MGPoint* pMGPres = (MGPoint*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pMGPres );
  pMGPres->Clear();
  pMGPres->SetDefined(true);

  MGPoint* pMGP = (MGPoint*)args[0].addr;
  if(pMGP == NULL ||!pMGP->IsDefined()) {
    cerr << "MGPoint does not exist." << endl;
    pMGPres->SetDefined(false);
    return 0;
  }

  Periods* per = (Periods*)args[1].addr;
  if(per == NULL || !per->IsDefined()) {
    cerr << "Periods are not defined." << endl;
    pMGPres->SetDefined(false);
    return 0;
  }
  if (pMGP->GetNoComponents() < 1 || per->IsEmpty()) {
    pMGPres->SetDefined(true);
    return 0;
  }
  pMGP->Atperiods(per, pMGPres);
  return 0;
}

const string OpAtperiodsSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint x periods -> mgpoint" "</text--->"
  "<text> _ atperiods _</text--->"
  "<text>Restricts the moving gpoint to the given periods.</text--->"
  "<text>X_MGPOINT atperiods PERIODS </text--->"
  ") )";

Operator tempnetatperiods("atperiods",
                OpAtperiodsSpec,
                OpAtperiodsValueMapping,
                Operator::SimpleSelect,
                OpAtperiodsTypeMap );


/*
1.3.7 Operator ~deftime~

Returns the deftime of a ~MGPoint~ as ~periods~ value.

*/

ListExpr OpDeftimeTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (nl->IsAtom( xMGPointDesc )) &&
      nl->AtomType( xMGPointDesc ) == SymbolType &&
      (nl->SymbolValue( xMGPointDesc ) == "mgpoint" ||
      nl->SymbolValue( xMGPointDesc ) == "ugpoint"))
  {
    return (nl->SymbolAtom( "periods" ));
  }

  return nl->SymbolAtom( "typeerror" );
}

int OpDeftime_ugp(Word* args, Word& result, int message,
                  Word& local, Supplier in_xSupplier) {
  Periods* pResult = (Periods*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pResult );
  UGPoint* pUGP = (UGPoint*) args[0].addr;
  pResult->Clear();
  if (!pUGP->IsDefined()) {
    return 0;
  }
  pUGP->Deftime(*pResult);
  return 0;
}

int OpDeftime_mgp(Word* args, Word& result, int message, Word& local,
                  Supplier in_xSupplier) {
  result = qp->ResultStorage(in_xSupplier);
  MGPoint* pMGP = (MGPoint*) args[0].addr;
  if (!pMGP->IsDefined() || pMGP->GetNoComponents() < 1) {
    return 0;
  }
  pMGP->Deftime((Periods*) result.addr);
  return 0;
}

int OpDeftimeSelect(ListExpr args) {
  ListExpr arg = nl->First(args);
  if ( nl->SymbolValue(arg) == "mgpoint")
    return 0;
  if ( nl->SymbolValue(arg) == "ugpoint")
    return 1;
  return -1; // This point should never be reached
};

ValueMapping OpDeftimeValueMapping [] = {
  OpDeftime_mgp,
  OpDeftime_ugp
};

const string OpDeftimeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>A -> periods if A is mgpoint or ugpoint" "</text--->"
  "<text>deftime(_)</text--->"
  "<text>Returns the periods in which the mgpoint is defined.</text--->"
  "<text>deftime(mgpoint)</text--->"
  ") )";

Operator tempnetdeftime("deftime",
                OpDeftimeSpec,
                2,
                OpDeftimeValueMapping,
                OpDeftimeSelect,
                OpDeftimeTypeMap );

/*
1.3.8 Operator ~final~

Returns the final time and position of the ~MGPoint~ as ~IGPoint~

TypeMapping see operator ~final~.

*/

ListExpr OpFinalInitialTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }
  return nl->SymbolAtom("igpoint");
}

const string OpFinalSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint -> igpoint" "</text--->"
  "<text> final (_)</text--->"
  "<text>Computes the final instant of a moving gpoint.</text--->"
  "<text>final (C_MGPOINT)</text--->"
  ") )";

Operator tempnetfinal("final",
                OpFinalSpec,
                MappingFinal<MGPoint,UGPoint, GPoint>,
                Operator::SimpleSelect,
                OpFinalInitialTypeMap );


/*
1.3.9 Operator ~initial~

Returns the start point and time of the ~MGPoint~ as ~IGPoint~.

*/

const string OpInitialSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint -> igpoint" "</text--->"
  "<text> initial (_)</text--->"
  "<text>Computes the initial position of a moving gpoint.</text--->"
  "<text>initial (C_MGPOINT)</text--->"
  ") )";

Operator tempnetinitial("initial",
                OpInitialSpec,
                MappingInitial<MGPoint, UGPoint, GPoint>,
                Operator::SimpleSelect,
                OpFinalInitialTypeMap );

/*
1.3.10 Operator ~inside~

Returns a ~mbool~ which is true for the times the ~MGPoint~ is inside a ~GLine~
false elsewhere.

*/

ListExpr OpInsideTypeMapping(ListExpr in_xArgs)
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

int OpInsideValueMapping(Word* args,
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
  pMGPoint->Inside(pGLine, pResult);
  return 0;
};


const string OpTInsideSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint x gline -> mbool" "</text--->"
  "<text> _ inside _ </text--->"
  "<text>Returns true for the times the mgpoint is at the gline."
    " False elsewhere.</text--->"
  "<text> X_MGPOINT inside GLINE</text--->"
  ") )";

Operator tempnetinside("inside",
                OpTInsideSpec,
                OpInsideValueMapping,
                Operator::SimpleSelect,
                OpInsideTypeMapping );


/*
1.3.11 Operator ~inst~

Returns the time instant of the ~IGPoint~

*/

ListExpr OpInstTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xIGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xIGPointDesc )) ||
      nl->AtomType( xIGPointDesc ) != SymbolType ||
      nl->SymbolValue( xIGPointDesc ) != "igpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }

  return nl->SymbolAtom( "instant" );
}

const string OpInstSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>igpoint -> instant" "</text--->"
  "<text>inst(igpoint)</text--->"
  "<text>Returns the timeinstant value of the igpoint.</text--->"
  "<text>inst(igpoint)</text--->"
  ") )";

Operator tempnetinst("inst",
                OpInstSpec,
                IntimeInst<GPoint>,
                Operator::SimpleSelect,
                OpInstTypeMap );

/*
1.3.12 Operator ~intersection~

Computes a ~MGPoint~ representing the intersection of two ~MGPoint~.

*/

ListExpr OpIntersectionTypeMapping(ListExpr in_xArgs)
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

int OpIntersectionValueMapping(Word* args,
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
    sendMessages("Second mgpoint does not exist.");
    pResult->SetDefined(false);
    return 0;
  }
  pMGPoint1->Intersection(pMGPoint2, pResult);
  return 0;
}

const string OpIntersectionSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint x mgpoint -> mgpoint" "</text--->"
  "<text>intersection(mgpoint, mgpoint)</text--->"
  "<text>Returns a moving gpoint representing the intersection of the two "
    " given moving gpoint.</text--->"
  "<text>intersection(mgpoint, mgpoint)</text--->"
  ") )";

Operator tempnetintersection("intersection",
                OpIntersectionSpec,
                OpIntersectionValueMapping,
                Operator::SimpleSelect,
                OpIntersectionTypeMapping );


/*
1.3.13 Operator ~isempty~

Returns true if the ~MGPoint~ has no units.

*/

ListExpr OpIsEmptyTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }

  return nl->SymbolAtom("bool");
}

int OpIsEmptyValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  CcBool* pEmpty = (CcBool*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pEmpty );
  MGPoint* pMGP = (MGPoint*)args[0].addr;
  if (pMGP == NULL ||!pMGP->IsDefined()) {
    pEmpty->Set(false, false);
  } else {
    pEmpty->Set(true,pMGP->GetNoComponents() == 0);
  }
  return 0;
}

const string OpIsEmptySpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint -> bool" "</text--->"
  "<text> isemtpy (_) </text--->"
  "<text>Returns true if the moving gpoint is empty, false elsewhere.</text--->"
  "<text>isempty (MGPOINT) </text--->"
  ") )";

Operator tempnetisempty("isempty",
                OpIsEmptySpec,
                OpIsEmptyValueMapping,
                Operator::SimpleSelect,
                OpIsEmptyTypeMap );

/*
1.3.14 Operator ~length~

Returns the length of the trip of the ~MGPoint~.

*/

ListExpr OpLengthTypeMapping(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }

  return nl->SymbolAtom( "real" );
}

int OpLengthValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  result = qp->ResultStorage(in_xSupplier);
  CcReal* pResult = (CcReal*) result.addr;
  // Get input value
  MGPoint* pMGPoint = (MGPoint*)args[0].addr;
  if(pMGPoint == NULL || pMGPoint->GetNoComponents() < 1 ||
      !pMGPoint->IsDefined()) {
    cmsg.inFunError("MGPoint does not exist.");
    pResult->Set(false, 0.0);
    return 0;
  }
  pResult-> Set(true, pMGPoint->Length());
  return 1;
}

const string OpTLengthSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint -> real" "</text--->"
  "<text>length(mgpoint)</text--->"
  "<text>Calculates the length of the pass passed by a moving gpoint.</text--->"
  "<text>length(mgpoint)</text--->"
  ") )";

Operator tempnetlength("length",
                    OpTLengthSpec,
                    OpLengthValueMapping,
                    Operator::SimpleSelect,
                    OpLengthTypeMapping );


/*
1.3.15 Operator ~no\_components~

Returns the number of units of a ~MGPoint~.

*/

ListExpr OpNoCompTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }

  return nl->SymbolAtom("int");
}

int OpNoCompValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  CcInt* pNumber = (CcInt*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pNumber );
  MGPoint* pMGP = (MGPoint*)args[0].addr;
  pNumber->Set(false, 0);
  if(pMGP == NULL ||!pMGP->IsDefined()) {
    pNumber->Set(false,0);
    cerr << "MGPoint is not defined." << endl;
  } else {
    pNumber->Set(true, pMGP->GetNoComponents());
  }
  return 0;
}

const string OpNoCompSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint -> int" "</text--->"
  "<text> no_components (_) </text--->"
  "<text>Returns the number of components of the moving gpoint.</text--->"
  "<text>no_components(MGPOINT) </text--->"
  ") )";

Operator tempnetnocomp("no_components",
                OpNoCompSpec,
                OpNoCompValueMapping,
                Operator::SimpleSelect,
                OpNoCompTypeMap );

/*
1.3.16 Operator ~present~

Returns true if the ~MGPoint~ has at least almost one unit with the time instant
respectively one of the periods.

*/

ListExpr OpPresentTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) == 2 ) {
    ListExpr arg1 = nl->First(in_xArgs);
    ListExpr arg2 = nl->Second(in_xArgs);
    if( nl->IsEqual(arg1,"mgpoint") && (
      nl->IsEqual(arg2,"instant") || nl->IsEqual(arg2, "periods")))
      return (nl->SymbolAtom( "bool" ));
  }
  return nl->SymbolAtom("typeerror");
}

int OpPresent_mgpi(Word* args, Word& result, int message,
                                Word& local,Supplier in_xSupplier){
  CcBool* pPresent = (CcBool*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pPresent );
  MGPoint* pMGP = (MGPoint*)args[0].addr;
  if(pMGP == NULL ||!pMGP->IsDefined()) {
    cerr << "MGPoint does not exist." << endl;
    pPresent->Set(false, false);
    return 0;
  }
  Instant* ins = (Instant*)args[1].addr;
  if(ins == NULL || !ins->IsDefined()) {
    cerr << "Instant is not defined." << endl;
    pPresent->Set(false, false);
    return 0;
  }
  if (pMGP->GetNoComponents() < 1) {
    pPresent->Set(true,false);
    return 0;
  }
  pPresent->Set(true,pMGP->Present(ins));
  return 0;
};

int OpPresent_mgpp(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  CcBool* pPresent = (CcBool*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pPresent);
  // Get input values
  MGPoint* pMGPoint = (MGPoint*) args[0].addr;
  if(pMGPoint == NULL){
    cerr << "MGPoint == null." << endl;
    pPresent->Set(false, false);
    return 0;
  }
  if (!pMGPoint->IsDefined()) {
    cerr << "MGPoint not defined." << endl;
    pPresent->Set(false, false);
    return 0;
  }
  Periods* pPeriods = (Periods*)args[1].addr;
  if(pPeriods == NULL || !pPeriods->IsDefined()) {
    cerr << "Periods does not exist." << endl;
    pPresent->Set(false, false);
    return 0;
  }
  if (pMGPoint->GetNoComponents() < 1 || pPeriods->IsEmpty()) {
    pPresent->Set(true, false);
    return 0;
  }
  pPresent->Set(true, pMGPoint->Present(pPeriods));
  return 0;
}

int OpPresentSelect(ListExpr args){
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( nl->SymbolValue(arg1) == "mgpoint" &&
       nl->SymbolValue( arg2) == "periods" )
    return 0;
  if ( nl->SymbolValue(arg1) == "mgpoint" &&
       nl->SymbolValue( arg2) == "instant")
    return 1;
  return -1; // This point should never be reached
};

ValueMapping OpPresentValueMap[] = {
  OpPresent_mgpp,
  OpPresent_mgpi
};

const string OpPresentSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint x A -> bool with A = instant or periods" "</text--->"
  "<text> _ present _</text--->"
  "<text>Returns true if the moving gpoint at least once exists in the given "
    "instant or one of the periods.</text--->"
  "<text>X_MGPOINT present PERIODS </text--->"
  ") )";

Operator tempnetpresent(
                "present",
                OpPresentSpec,
                2,
                OpPresentValueMap,
                OpPresentSelect,
                OpPresentTypeMap );


/*
1.3.17 Operator ~val~

Returns the ~GPoint~ value of a ~IGPoint~

*/

ListExpr OpValTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xIGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xIGPointDesc )) ||
      nl->AtomType( xIGPointDesc ) != SymbolType ||
      nl->SymbolValue( xIGPointDesc ) != "igpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }

  return nl->SymbolAtom( "gpoint" );
}

const string OpValSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>igpoint -> gpoint" "</text--->"
  "<text>val(igpoint)</text--->"
  "<text>Returns the gpoint value of the igpoint.</text--->"
  "<text>val(igpoint)</text--->"
  ") )";

Operator tempnetval("val",
                OpValSpec,
                IntimeVal<GPoint>,
                Operator::SimpleSelect,
                OpValTypeMap );

/*
1.3.18 Operator ~trajectory~

Returns the sorted ~GLine~ passed by a ~MGPoint~.

*/

ListExpr OpTrajectoryTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }

  return nl->SymbolAtom( "gline" );
}

int OpTrajectoryValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  // Get (empty) return value
  GLine* pGLine = (GLine*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pGLine);
  // Get input values
  MGPoint* pMGPoint = (MGPoint*)args[0].addr;
  if(pMGPoint == NULL || pMGPoint->GetNoComponents() < 1 ||
      !pMGPoint->IsDefined()) {
    cerr << "MGPoint does not exist." << endl;
    pGLine->SetDefined(false);
    return 0;
  }
  GLine *res = new GLine(0);
  pMGPoint->Trajectory(res);
  result = SetWord(res);
  //(*pGLine) = *res;
  return 0;
}

const string OpTrajectorySpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint -> gline" "</text--->"
  "<text>trajectory(mgpoint)</text--->"
  "<text>Calculates the trajectory for a moving gpoint.</text--->"
  "<text>trajectory(mgpoint)</text--->"
  ") )";

Operator trajectory("trajectory",
                    OpTrajectorySpec,
                    OpTrajectoryValueMapping,
                    Operator::SimpleSelect,
                    OpTrajectoryTypeMap );

/*
1.3.19 Operator ~units~

Returns the stream of ~UGPoint~ from the given ~MGPoint~.

*/

ListExpr OpUnitsTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMPointDesc )) ||
      nl->AtomType( xMPointDesc ) != SymbolType ||
      nl->SymbolValue( xMPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }

  return nl->TwoElemList(nl->SymbolAtom("stream"),
                         nl->SymbolAtom("ugpoint"));
}

const string OpUnitsSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint -> (stream ugpoint)" "</text--->"
  "<text>units(mgpoint)</text--->"
  "<text>get the stream of units of the moving value.</text--->"
  "<text>units(mgpoint)</text--->"
  ") )";

Operator units("units",
               OpUnitsSpec,
               MappingUnits<MGPoint, UGPoint>,
               Operator::SimpleSelect,
               OpUnitsTypeMap );

/*
1.3.20 Operator ~unitendpos~

Returns the end position of the ~UGPoint~ as ~real~.

TypeMapping for ~unitendpos~, ~unitstartpos~, ~unitendtime~, ~unitstarttime~
and ~unitrid~.

*/

ListExpr OpUnitPosTimeTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "ugpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }
  return nl->SymbolAtom("real");
}

int OpUnitEndPosValueMapping( Word* args, Word& result, int message,
                             Word& local, Supplier s ){
  CcReal* pNumber = (CcReal*)qp->ResultStorage(s).addr;
  result = SetWord( pNumber );
  UGPoint* pUGP = (UGPoint*)args[0].addr;
  pNumber->Set(false, 0.0);
  if(pUGP == NULL ||!pUGP->IsDefined()) {
    pNumber->Set(false,0.0);
    cerr << "UGPoint is not defined." << endl;
  } else {
    pNumber->Set(true, pUGP->GetUnitEndPos());
  }
  return 0;
}

const string OpUnitEndPosSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>ugpoint -> double" "</text--->"
  "<text> unitendpos (_)</text--->"
  "<text>Returns the end position of a given ugpoint.</text--->"
  "<text>unitendpos(UGPOINT)</text--->"
  ") )";

Operator tempnetunitendpos("unitendpos",
                OpUnitEndPosSpec,
                OpUnitEndPosValueMapping,
                Operator::SimpleSelect,
                OpUnitPosTimeTypeMap );

/*
1.3.21 Operator ~unitstartpos~

Returns the start position of the ~UGPoint~ as ~real~.

TypeMapping see operator ~unitendpos~

*/

int OpUnitStartPosValueMapping( Word* args, Word& result, int message,
                             Word& local, Supplier s ){
  CcReal* pNumber = (CcReal*)qp->ResultStorage(s).addr;
  result = SetWord( pNumber );
  UGPoint* pUGP = (UGPoint*)args[0].addr;
  pNumber->Set(false, 0.0);
  if(pUGP == NULL ||!pUGP->IsDefined()) {
    pNumber->Set(false,0.0);
    cerr << "UGPoint is not defined." << endl;
  } else {
    pNumber->Set(true, pUGP->GetUnitStartPos());
  }
  return 0;
}

const string OpUnitStartPosSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>ugpoint -> double" "</text--->"
  "<text> unitstartpos (_)</text--->"
  "<text>Returns the start position of a given ugpoint.</text--->"
  "<text>unitstartpos(UGPOINT)</text--->"
  ") )";

Operator tempnetunitstartpos("unitstartpos",
                OpUnitStartPosSpec,
                OpUnitStartPosValueMapping,
                Operator::SimpleSelect,
                OpUnitPosTimeTypeMap );



/*
1.3.22 Operator ~unitendtime~

Returns the last time instant of the ~UGPoint~ as ~real~.

TypeMapping see operator ~unitendpos~

*/
int OpUnitEndTimeValueMapping( Word* args, Word& result, int message,
                             Word& local, Supplier s ){
  CcReal* pNumber = (CcReal*)qp->ResultStorage(s).addr;
  result = SetWord( pNumber );
  UGPoint* pUGP = (UGPoint*)args[0].addr;
  pNumber->Set(false, 0.0);
  if(pUGP == NULL ||!pUGP->IsDefined()) {
    pNumber->Set(false,0.0);
    cerr << "UGPoint is not defined." << endl;
  } else {
    pNumber->Set(true, (double) pUGP->GetUnitEndTime());
  }
  return 0;
}

const string OpUnitEndTimeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>ugpoint -> real" "</text--->"
  "<text> unitendtime (_)</text--->"
  "<text>Returns the double value representing the end time instant of the"
  " given ugpoint.</text--->"
  "<text>unitendtime(UGPOINT)</text--->"
  ") )";

Operator tempnetunitendtime("unitendtime",
                OpUnitEndTimeSpec,
                OpUnitEndTimeValueMapping,
                Operator::SimpleSelect,
                OpUnitPosTimeTypeMap );


/*
1.3.23 Operator ~unitstarttime~

Returns the starting time instant of the ~UGPoint~ as ~real~.

TypeMapping see operator ~unitendpos~

*/

int OpUnitStartTimeValueMapping( Word* args, Word& result, int message,
                             Word& local, Supplier s ){
  CcReal* pNumber = (CcReal*)qp->ResultStorage(s).addr;
  result = SetWord( pNumber );
  UGPoint* pUGP = (UGPoint*)args[0].addr;
  pNumber->Set(false, 0.0);
  if(pUGP == NULL ||!pUGP->IsDefined()) {
    pNumber->Set(false,0.0);
    cerr << "UGPoint is not defined." << endl;
  } else {
    pNumber->Set(true, (double) pUGP->GetUnitStartTime());
  }
  return 0;
}

const string OpUnitStartTimeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>ugpoint -> real" "</text--->"
  "<text> unitstarttime (_)</text--->"
  "<text>Returns the double value representing the start time instant of the"
  " given ugpoint.</text--->"
  "<text>unitstarttime(UGPOINT)</text--->"
  ") )";

Operator tempnetunitstarttime("unitstarttime",
                OpUnitStartTimeSpec,
                OpUnitStartTimeValueMapping,
                Operator::SimpleSelect,
                OpUnitPosTimeTypeMap );

/*
1.3.24 Operator ~unitrid~

Returns the route id of the ~UGPoint~ as ~real~.

TypeMapping see operator ~unitendpos~

*/

int OpUnitRidValueMapping( Word* args, Word& result, int message,
                             Word& local, Supplier s ){
  CcReal* pNumber = (CcReal*)qp->ResultStorage(s).addr;
  result = SetWord( pNumber );
  UGPoint* pUGP = (UGPoint*)args[0].addr;
  pNumber->Set(false, 0.0);
  if(pUGP == NULL ||!pUGP->IsDefined()) {
    pNumber->Set(false,0.0);
    cerr << "UGPoint is not defined." << endl;
  } else {
    pNumber->Set(true, (double) pUGP->GetUnitRid());
  }
  return 0;
}

const string OpUnitRidSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>ugpoint -> real" "</text--->"
  "<text> unitrid (_)</text--->"
  "<text>Returns the route id of a given ugpoint.</text--->"
  "<text>unitrid(UGPOINT)</text--->"
  ") )";

Operator tempnetunitrid("unitrid",
                OpUnitRidSpec,
                OpUnitRidValueMapping,
                Operator::SimpleSelect,
                OpUnitPosTimeTypeMap );


/*
1.3.25 Operator ~unitbox~

Returns the bounding box of the ~ugpoint~ as rectangle of dimension 3. with
rid, rid, min(p0.pos. p1.pos), max(p0.pos, p1.pos), timeInterval.start,
timeInterval.end.

*/

ListExpr OpUnitBoxTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "ugpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }
  return nl->SymbolAtom("rect3");
}

int OpUnitBoxValueMapping( Word* args, Word& result, int message,
                             Word& local, Supplier s ){
  result = qp->ResultStorage( s );
  Rectangle<3>* box = static_cast<Rectangle<3>* >(result.addr);
  UGPoint* arg = static_cast<UGPoint*>(args[0].addr);
  if(!arg->IsDefined()){
    box->SetDefined(false);
  } else {
    (*box) = arg->NetBoundingBox3d();
  }
  return 0;
}

const string OpUnitBoxSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>ugpoint -> rect3" "</text--->"
  "<text> unitbox (_)</text--->"
  "<text>Returns the bounding box rectangle of the ugpoint in form: rid,"
  " rid, min(startpos, endpos), max(startpos, endpos), timeInterval.start,"
  " timeInterval.end. </text--->"
  "<text>unitbox(UGPOINT)</text--->"
  ") )";

Operator tempnetunitbox("unitbox",
                OpUnitBoxSpec,
                OpUnitBoxValueMapping,
                Operator::SimpleSelect,
                OpUnitBoxTypeMap );

/*
1.3.26 Operator ~unitbox~

Returns the bounding box of the ~ugpoint~ as rectangle of dimension 2. with
rid, rid, min(p0.pos. p1.pos), max(p0.pos, p1.pos).

*/

ListExpr OpUnitBox2TypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "ugpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }
  return nl->SymbolAtom("rect");
}

int OpUnitBox2ValueMapping( Word* args, Word& result, int message,
                             Word& local, Supplier s ){

  result = qp->ResultStorage( s );
  Rectangle<2>* box = static_cast<Rectangle<2>* >(result.addr);
  UGPoint* arg = static_cast<UGPoint*>(args[0].addr);
  if(!arg->IsDefined()){
    box->SetDefined(false);
  } else {
    (*box) = arg->NetBoundingBox2d();
  }
  return 0;
}

const string OpUnitBox2Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>ugpoint -> rect2" "</text--->"
  "<text> unitbox2 (_)</text--->"
  "<text>Returns the bounding box rectangle of the ugpoint in form: rid,"
  " rid, min(startpos, endpos), max(startpos, endpos). </text--->"
  "<text>unitbox2(UGPOINT)</text--->"
  ") )";

Operator tempnetunitbox2("unitbox2",
                OpUnitBox2Spec,
                OpUnitBox2ValueMapping,
                Operator::SimpleSelect,
                OpUnitBox2TypeMap );


/*
1.3.26 Operator ~unitboundingbox~

Returns the spatialtemporal bounding box of the ~ugpoint~.

*/

ListExpr OpUnitBoundingBoxTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "ugpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }
  return nl->SymbolAtom("rect3");
}

int OpUnitBoundingBoxValueMapping( Word* args, Word& result, int message,
                             Word& local, Supplier s ){

  result = qp->ResultStorage( s );
  Rectangle<3>* box = static_cast<Rectangle<3>* >(result.addr);
  UGPoint* arg = static_cast<UGPoint*>(args[0].addr);
  if(!arg->IsDefined()){
    box->SetDefined(false);
  } else {
    (*box) = arg->BoundingBox();
  }
  return 0;
}

const string OpUnitBoundingBoxSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>ugpoint -> rect3" "</text--->"
  "<text> unitboundingbox (_)</text--->"
  "<text>Returns the spatialtemporal bounding box of the ugpoint. </text--->"
  "<text>unitboundingbox(UGPOINT)</text--->"
  ") )";

Operator tempnetunitboundingbox("unitboundingbox",
                OpUnitBoundingBoxSpec,
                OpUnitBoundingBoxValueMapping,
                Operator::SimpleSelect,
                OpUnitBoundingBoxTypeMap );

/*
1.3.26 Operator ~unitboundingbox~

Returns the spatialtemporal bounding box of the ~ugpoint~.

*/

ListExpr OpMGPointBoundingBoxTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }
  return nl->SymbolAtom("rect3");
}

int OpMGPointBoundingBoxValueMapping( Word* args, Word& result, int message,
                             Word& local, Supplier s ){

  result = qp->ResultStorage( s );
  Rectangle<3>* box = static_cast<Rectangle<3>* >(result.addr);
  MGPoint* arg = static_cast<MGPoint*>(args[0].addr);
  if(!arg->IsDefined() || arg->GetNoComponents()<1){
    box->SetDefined(false);
  } else {
    *box = arg->BoundingBox();
  }
  return 0;
}

const string OpMGPointBoundingBoxSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint -> rect3" "</text--->"
  "<text> mgpbbox (_)</text--->"
  "<text>Returns the spatialtemporal bounding box of the mgpoint. </text--->"
  "<text>mgpbbox(MGPOINT)</text--->"
  ") )";

Operator tempnetmgpointboundingbox("mgpbbox",
                OpMGPointBoundingBoxSpec,
                OpMGPointBoundingBoxValueMapping,
                Operator::SimpleSelect,
                OpMGPointBoundingBoxTypeMap );


/*
1.3.25 Operator ~mgpoint2mpoint~

Returns the ~mpoint~ value of the given ~MGPoint~.

*/

ListExpr OpMGPoint2MPointTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 ){
    sendMessages("Expects a list of length 1.");
    return (nl->SymbolAtom( "typeerror" ));
  }

  ListExpr xsource = nl->First(in_xArgs);

  if( (!nl->IsAtom(xsource)) ||
      !nl->IsEqual(xsource, "mgpoint"))
  {
    sendMessages("Element must be of type mgpoint.");
    return (nl->SymbolAtom("typeerror"));
  }
  return nl->SymbolAtom( "mpoint" );
}

int OpMGPoint2MPointValueMapping(Word* args,
                                   Word& result,
                                  int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  MPoint* pMPoint = (MPoint*) qp->ResultStorage(in_xSupplier).addr;
  result = SetWord(pMPoint);
  pMPoint->SetDefined(true);
  MGPoint* pMGPoint = (MGPoint*)args[0].addr;
  if (pMGPoint == NULL || !pMGPoint->IsDefined()) {
    sendMessages("MGPoint must be defined!");
    pMPoint->SetDefined(false);
    return 0;
  }
  pMGPoint->Mgpoint2mpoint(pMPoint);
  return 0;
}

const string OpMGPoint2MPointSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>mgpoint -> mpoint" "</text--->"
  "<text>mgpoint2mpoint(MGPOINT)</text--->"
  "<text>Returns the mpoint value of the mgpoint.</text--->"
  "<text> mgpoint2mpoint(mgpoint) </text--->"
  ") )";

Operator tempnetmgpoint2mpoint (
           "mgpoint2mpoint",               // name
           OpMGPoint2MPointSpec,          // specification
           OpMGPoint2MPointValueMapping,  // value mapping
           Operator::SimpleSelect,        // selection function
           OpMGPoint2MPointTypeMap        // type mapping
);



/*
1.3.26 Operator ~distance~

Computes a mreal representing the Euclidean Distance between the two ~mgpoint~.

*/

ListExpr OpDistanceTypeMapping(ListExpr in_xArgs)
{
  ListExpr arg1, arg2;
  if( nl->ListLength(in_xArgs) == 2 ){
    arg1 = nl->First(in_xArgs);
    arg2 = nl->Second(in_xArgs);
    if (nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
        nl->SymbolValue(arg1) == "mgpoint" && nl->IsAtom(arg2) &&
        nl->AtomType(arg2) == SymbolType &&
        nl->SymbolValue(arg2) == "mgpoint")
    {
      return (nl->SymbolAtom("mreal"));
    }
  }
  return (nl->SymbolAtom( "typeerror" ));
}

int OpDistanceValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  // Get (empty) return value
  MReal* pResult = (MReal*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pResult);
//   pResult->Clear();
//   pResult->SetDefined(true);
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
    sendMessages("Second mgpoint does not exist.");
    pResult->SetDefined(false);
    return 0;
  }
  if(pMGPoint1->GetNetworkId() != pMGPoint2->GetNetworkId()) {
    sendMessages("MGPoints must belong to the same network.");
    pResult->SetDefined(false);
    return 0;
  }
  pMGPoint1->Distance(pMGPoint2, pResult);
  return 0;
}

const string OpDistanceSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint x mgpoint -> mreal" "</text--->"
  "<text>distance(mgpoint, mgpoint)</text--->"
  "<text>Returns a moving real representing the Euclidean Distance of the two "
    " given moving gpoint.</text--->"
  "<text>query distance(mgpoint, mgpoint)</text--->"
  ") )";

Operator tempnetdistance("distance",
                OpDistanceSpec,
                OpDistanceValueMapping,
                Operator::SimpleSelect,
                OpDistanceTypeMapping );


/*
1.4 Creating the Algebra

*/

class TemporalNetAlgebra : public Algebra
{
  public:

  TemporalNetAlgebra() : Algebra()
  {
    AddTypeConstructor( &unitgpoint );
    AddTypeConstructor( &movinggpoint );
    AddTypeConstructor( &intimegpoint);

    movinggpoint.AssociateKind( "TEMPORAL" );
    movinggpoint.AssociateKind( "DATA" );
    unitgpoint.AssociateKind( "TEMPORAL" );
    unitgpoint.AssociateKind( "DATA" );
    intimegpoint.AssociateKind("TEMPORAL");
    intimegpoint.AssociateKind("DATA");

    AddOperator(&mpoint2mgpoint);
    AddOperator(&units);
    AddOperator(&simplify);
    AddOperator(&tempnetpasses);
    AddOperator(&trajectory);
    AddOperator(&tempnetlength);
    AddOperator(&tempnetatinstant);
    AddOperator(&tempnetinitial);
    AddOperator(&tempnetfinal);
    AddOperator(&tempnetat);
    AddOperator(&tempnetval);
    AddOperator(&tempnetinst);
    AddOperator(&tempnetatperiods);
    AddOperator(&tempnetpresent);
    AddOperator(&tempnetisempty);
    AddOperator(&tempnetnocomp);
    AddOperator(&tempnetinside);
    AddOperator(&tempnetintersection);
    AddOperator(&tempnetdeftime);
    AddOperator(&tempnetunitrid);
    AddOperator(&tempnetunitstartpos);
    AddOperator(&tempnetunitendpos);
    AddOperator(&tempnetunitstarttime);
    AddOperator(&tempnetunitendtime);
    AddOperator(&tempnetunitbox);
    AddOperator(&tempnetunitbox2);
    AddOperator(&tempnetunitboundingbox);
    AddOperator(&tempnetmgpointboundingbox);
    AddOperator(&tempnetmgpoint2mpoint);
    AddOperator(&tempnetdistance);
  }


  ~TemporalNetAlgebra() {};
};

/*
Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C" Algebra* InitializeTemporalNetAlgebra( NestedList* in_pNL,
                                                  QueryProcessor* in_pQP )
{
  nl = in_pNL;
  qp = in_pQP;
  return (new TemporalNetAlgebra());
}
