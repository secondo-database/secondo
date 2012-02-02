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

February 2008 - Simone Jandt

Defines, includes, and constants

*/

#include "NestedList.h"
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "TupleIdentifier.h"
#include "../../Tools/Flob/DbArray.h"
#include "../../Tools/Flob/Flob.h"
#include "StandardTypes.h"
#include "Network2Algebra.h"
#include "TemporalAlgebra.h"
#include "TemporalNet2Algebra.h"
#include "Network2Manager.h"
#include "ListUtils.h"
#include "Symbols.h"
#include <iostream>
#include <sstream>
#include <string>
#include "QueryProcessor.h"
#include "Algebra.h"
#include "DateTime.h"

extern NestedList* nl;
extern QueryProcessor* qp;
map<int,string> *netList;


using namespace temporalnet2;

/*

1.1 Additional Methods

Returns true if the ~RouteInterval~ ~pRi~ is found in the set of ~rint~.
false elsewhere.

*/
bool searchUnit(DbArray<RouteInterval> &rint, int low, int high,
                 RouteInterval pRi){
  RouteInterval rI;
  if (low <= high) {
    int mid = (high + low) / 2;
    int n = 0;
    if (mid < n || mid >= rint.Size()) {
      return false;
    }else {
      rint.Get(mid, rI);
      if (rI.GetRouteId() < pRi.GetRouteId()) {
        return searchUnit(rint, mid+1, high, pRi);
      } else {
        if (rI.GetRouteId() > pRi.GetRouteId()){
          if (mid == n) return false;
          else return searchUnit(rint, low, mid-1, pRi);
        } else {
          if (rI.GetStartPos() > pRi.GetEndPos()) {
            if (mid == n) return false;
            else return searchUnit(rint, low, mid-1, pRi);
          } else {
            if (rI.GetEndPos() < pRi.GetStartPos()){
              return searchUnit(rint, mid+1, high, pRi);
            } else {
              return true;
            }
          }
        }
      }
    }
  } else return false;
  return false;
}

/*
Returns true if the two given sets of ~RouteIntervals~ at least intersect once.

*/
bool RIsIntersects(DbArray<RouteInterval> &riint1,
               DbArray<RouteInterval> &riint2,
               bool sortedri1,
               bool sortedri2){
  RouteInterval pRi1, pRi2;
  if (!sortedri1) {
    for (int i = 0; i < riint1.Size(); i++){
      riint1.Get(i, pRi1);
      if (sortedri2){
        if (searchUnit(riint2, 0, riint2.Size()-1, pRi1)){
          return true;
        };
      } else {
        for (int j = 0 ; j < riint2.Size(); j++){
          riint2.Get(j,pRi2);
          if (pRi1.GetRouteId() == pRi2.GetRouteId() &&
              (!(pRi1.GetEndPos() < pRi2.GetStartPos() ||
              pRi2.GetStartPos() > pRi1.GetEndPos()))){
            return true;
          }
        }
      }
    }
  } else {
    if (sortedri2) {
      int i = 0;
      int j = 0;
      while (i<riint1.Size() && j < riint2.Size()) {
        riint1.Get(i,pRi1);
        riint2.Get(j,pRi2);
        if (pRi1.GetRouteId() < pRi2.GetRouteId()) i++;
        else
          if (pRi1.GetRouteId() > pRi2.GetRouteId()) j++;
          else
            if (pRi1.GetStartPos() > pRi2.GetEndPos()) j++;
            else
              if (pRi1.GetEndPos() < pRi2.GetStartPos()) i++;
              else return true;
      }
    } else {
        for (int i = 0; i < riint2.Size(); i++){
          riint2.Get(i, pRi2);
          if (searchUnit(riint1, 0, riint1.Size()-1, pRi2)) return true;
        }
    }
  }
  return false;
}

/*
Returns true if a ~RouteInterval~ is found that contains the ~gpoint~

*/
bool searchRouteInterval(GPoint *pGPoint, DbArray<RouteInterval> &tra,
                          int low, int high) {
  RouteInterval rI;
  if (low <= high) {
    int mid = (high + low) / 2;
    if ((mid < 0) || (mid >= tra.Size())) {
      return false;
    }else {
      tra.Get(mid, rI);
      if (rI.GetRouteId() < pGPoint->GetRouteId()) {
        return searchRouteInterval(pGPoint, tra, mid+1, high);
      } else {
        if (rI.GetRouteId() > pGPoint->GetRouteId()){
          return searchRouteInterval(pGPoint, tra, low, mid-1);
        } else {
          if (fabs(pGPoint->GetPosition() - rI.GetStartPos()) < 0.01 ||
              fabs(pGPoint->GetPosition() - rI.GetEndPos()) < 0.01) {
            return true;
          } else {
            if (rI.GetStartPos() > pGPoint->GetPosition()) {
              return searchRouteInterval(pGPoint, tra, low, mid-1);
            } else {
              if (rI.GetEndPos() < pGPoint->GetPosition()){
                return searchRouteInterval(pGPoint, tra, mid+1, high);
              } else {
                return true;
              }
            }
          }
        }
      }
    }
  }
  return false;
}


/*
Returns true if a ~RouteInterval~ is found that contains the ~gpoint~

*/
bool Includes(DbArray<RouteInterval> &tra, GPoint *gp){
  if (tra.Size() < 1) return false;
  return (searchRouteInterval(gp, tra, 0, tra.Size()-1));
}

/*
Sets parameter movingUp and side for the given Unit. Used by ~mpoint2mgpoint~.

*/

void setMoveAndSide(double &startPos, double &endPos, bool &MovingUp,
                    bool &dual, Side &side){
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
Gets the parameter Values of the given ~upoint~.Used by

*/

void getUnitValues(const UPoint *&curUnit, Point &endPoint, Point &startPoint,
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
Checks sline value for the point and computes the position from the point on
the sline including difference value if not exactly on the sline. The problem
is caused by the gps value simulation which is mostly not exactly on the sline.
So that AtPosition and AtPoint could not find the point on the sline.
Returns true if the ~point~ is on the sline false elsewhere.
Used by ~mpoint2mgpoint~.

*/

bool checkPoint (SimpleLine *&route, Point point, bool startSmaller,
                   double &pos, double &difference){
  bool result = false;
  HalfSegment hs;
  double k1, k2;
  Point left, right;
  for (int i = 0; i < route->Size()-1; i++) {
    route->Get(i, hs);
    left = hs.GetLeftPoint();
    right = hs.GetRightPoint();
    Coord xl = left.GetX(),
          yl = left.GetY(),
          xr = right.GetX(),
          yr = right.GetY(),
          x = point.GetX(),
          y = point.GetY();
    if ((fabs(x-xr) < 0.01 && fabs (y-yr) < 0.01) ||
        (fabs(x-xl) < 0.01 && fabs (y-yl) < 0.01))
    {
      difference = 0.0;
      result = true;
    } else {
      if (xl != xr && xl != x) {
        k1 = (y - yl) / (x - xl);
        k2 = (yr - yl) / (xr - xl);
        if ((fabs(k1-k2) < 0.004) &&
            ((xl < xr &&
              (x > xl || fabs(x-xl) < 0.01) &&
              (x < xr || fabs(x-xr) < 0.01)) ||
            (xl > xr &&
              (x < xl || fabs(x-xl)<0.01) &&
              (x > xr || fabs(x-xr) < 0.01))) &&
            (((yl < yr || fabs(yl-yr)<0.01) &&
              (y > yl || fabs(y-yl)<0.01 ) &&
              (y < yr || fabs(y-yr)<0.01)) ||
            (yl > yr &&
              (y < yl || fabs(y-yl) <0.01) &&
              (y > yr || fabs(y-yr)<0.01))))
        {
              difference = fabs(k1-k2);
              result = true;
        }
        else {result = false;}
      }
      else
      {
        if (( fabs(xl - xr) < 0.01 && fabs(xl -x) < 0.01) &&
            (((yl < yr|| fabs(yl-yr)<0.01) &&
                (yl < y || fabs(yl-y) <0.01)&&
                (y < yr ||fabs(y-yr)<0.01))||
              (yl > yr &&
                (yl > y || fabs(yl-y)<0.01)&&
                (y > yr ||fabs(y-yr)<0.01))))
        {
              difference = 0.0;
              result = true;
        }
        else {result = false;}
      }
    }
    if (result) {
       LRS lrs;
        route->Get( hs.attr.edgeno, lrs );
        route->Get( lrs.hsPos, hs );
        pos = lrs.lrsPos + point.Distance( hs.GetDomPoint() );
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

bool checkPointN (SimpleLine route, Point point, bool startSmaller,
                 double &pos)
{
  bool result = false;
  HalfSegment hs;
  double k1, k2;
  Point left, right;
  for (int i = 0; i < route.Size()-1; i++) {
    route.Get(i, hs);
    left = hs.GetLeftPoint();
    right = hs.GetRightPoint();
    Coord xl = left.GetX(),
    yl = left.GetY(),
    xr = right.GetX(),
    yr = right.GetY(),
    x = point.GetX(),
    y = point.GetY();
    if ((fabs(x-xr) < 0.01 && fabs (y-yr) < 0.01) ||
          (fabs(x-xl) < 0.01 && fabs (y-yl) < 0.01))
      result = true;
    else
    {
      if (xl != xr && xl != x)
      {
        k1 = (y - yl) / (x - xl);
        k2 = (yr - yl) / (xr - xl);
        if ((fabs(k1-k2) < 0.004) &&
              ((xl < xr &&
                (x > xl || fabs(x-xl) < 0.01) &&
                (x < xr || fabs(x-xr) < 0.01)) ||
              (xl > xr &&
                (x < xl || fabs(x-xl) < 0.01) &&
                (x > xr || fabs(x-xr) < 0.01))) &&
              (((yl < yr || fabs(yl-yr) < 0.01) &&
                (y > yl || fabs(y-yl) < 0.01) &&
                (y < yr || fabs(y-yr) < 0.01)) ||
              (yl > yr &&
                (y < yl || fabs(y-yl) <0.01) &&
                (y > yr || fabs(y-yr)<0.01))))
          result = true;
        else result = false;
      }
      else
      {
        if (( fabs(xl - xr) < 0.01 && fabs(xl -x) < 0.01) &&
              (((yl < yr|| fabs(yl-yr)<0.01) &&
                  (yl < y || fabs(yl-y) <0.01)&&
                  (y < yr ||fabs(y-yr)<0.01))||
                (yl > yr &&
                  (yl > y ||fabs(yl-y)<0.01)&&
                  (y > yr ||fabs(y-yr)<0.01))))
          result = true;
        else result = false;
      }
    }
    if (result) {
      LRS lrs;
      route.Get( hs.attr.edgeno, lrs );
      route.Get( lrs.hsPos, hs );
      pos = lrs.lrsPos + point.Distance( hs.GetDomPoint() );
      if( startSmaller != route.GetStartSmaller())
        pos = route.Length() - pos;
      if( fabs(pos-0.0) < 0.01)
        pos = 0.0;
      else if (fabs(pos-route.Length())<0.01)
        pos = route.Length();
      return result;
    }
  }
  return result;
}

/*
Almost Equal to ~checkPoint~ but allows bigger differences. Used by
~mpoint2mgpoint~.

*/

bool checkPoint03 (SimpleLine *&route, Point point, bool startSmaller,
                   double &pos, double &difference){
  bool result = false;
  HalfSegment hs;
  double k1, k2;
  Point left, right;
  for (int i = 0; i < route->Size()-1; i++) {
    route->Get(i, hs);
    left = hs.GetLeftPoint();
    right = hs.GetRightPoint();
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
            ((xl < xr &&
                (x > xl || fabs(x-xl) < 0.01) &&
                (x < xr || fabs(x-xr) < 0.01)) ||
             (xl > xr &&
                (x < xl || fabs(x-xl) < 0.01) &&
                ( x > xr || fabs(x-xr) < 0.01))) &&
            (((yl < yr || fabs(yl-yr) < 0.01) &&
                (y > yl || fabs(y-yl) < 0.01 ) &&
                (y < yr || fabs(y-yr)<0.01)) ||
            (yl > yr &&
                (y < yl || fabs(y-yl) <0.01) &&
                (y > yr || fabs(y-yr)<0.01))))
        {
              difference = fabs(k1-k2);
              result = true;
        }
        else {result = false;}
      }
      else
      {
        if (( fabs(xl - xr) < 0.01 && fabs(xl -x) < 0.01) &&
            (((yl < yr|| fabs(yl-yr)<0.01) &&
                  (yl < y || fabs(yl-y) <0.01)&&
                  (y < yr ||fabs(y-yr)<0.01))||
              (yl > yr &&
                  (yl > y || fabs(yl-y)<0.01)&&
                  (y > yr ||fabs(y-yr)<0.01))))
        {
              difference = 0.0;
              result = true;
        }
        else {result = false;}
      }
    }
    if (result)
    {
       LRS lrs;
      route->Get( hs.attr.edgeno, lrs );
      route->Get( lrs.hsPos, hs );
      pos = lrs.lrsPos + point.Distance( hs.GetDomPoint() );
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

bool lastcheckPoint03 (SimpleLine *&route, Point point, bool startSmaller,
                   double &pos, double &difference){
  bool result = false;
  HalfSegment hs;
  double k1, k2;
  Point left, right;
  for (int i = 0; i < route->Size()-1; i++) {
    route->Get(i, hs);
    left = hs.GetLeftPoint();
    right = hs.GetRightPoint();
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
        if (((xl < xr &&
                (x > xl || fabs(x-xl) < 0.1) &&
                (x < xr || fabs(x-xr) < 0.01)) ||
            (xl > xr &&
                ( x < xl || fabs(x-xl)<0.01)  &&
                ( x > xr || fabs(x-xr)<0.01))) &&
            (((yl < yr || fabs(yl-yr)<0.01) &&
                (y > yl || fabs(y-yl)<0.01 )&&
                (y < yr || fabs(y-yr)<0.01)) ||
            (yl > yr &&
                (y < yl || fabs(y-yl) <0.01) &&
                (y > yr || fabs(y-yr)<0.01))))
        {
              difference = fabs(k1-k2);
              result = true;
        }
        else {result = false;}
      }
      else
      {
        if (( fabs(xl - xr) < 0.1 && fabs(xl -x) < 0.1) &&
            (((yl < yr|| fabs(yl-yr)<0.1) &&
                  (yl < y || fabs(yl-y) <0.1)&&
                  (y < yr || fabs(y-yr)<0.1))||
            (yl > yr &&
                  (yl > y || fabs(yl-y)<0.1)&&
                  (y > yr ||fabs(y-yr)<0.1))))
        {
              difference = 0.0;
              result = true;
        }
        else {result = false;}
      }
    }
    if (result) {
       LRS lrs;
        route->Get( hs.attr.edgeno, lrs );
        route->Get( lrs.hsPos, hs );
        pos = lrs.lrsPos + point.Distance( hs.GetDomPoint() );
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
Used by operator ~inside~ to check the rest of the ~UGPoint~ if there is a
further intersection.

Two different parameter lists are possible.

*/
void checkEndOfUGPoint(double startPos, double endPos, Instant startTime,
                       bool bstart, Instant endTime, bool bend, int actRoutInt,
                       GLine* &pGLine, MBool* &pResult,
                       int iRouteId){
  RouteInterval pCurrInt;
  UBool interbool(true);
  bool swapped = false;
  bool found = false;
  double lStart, lEnd, factor, help;
  Instant tInterStart, tInterEnd;
  int k = actRoutInt + 1;
  while ( k < pGLine->NoOfComponents()) {
    pGLine->Get(k, pCurrInt);
    if (pCurrInt.GetRouteId() == iRouteId) {
      if (endPos < startPos) {
          help = startPos;
          startPos = endPos;
          endPos = help;
          swapped = true;
      }
      lStart = pCurrInt.GetStartPos();
      lEnd = pCurrInt.GetEndPos();
      if (lStart > lEnd) {
        lStart = pCurrInt.GetEndPos();
        lEnd = pCurrInt.GetStartPos();
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
  RouteInterval pCurrInt;
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
    pCurrInt = vRI[k];
    if (pCurrInt.GetRouteId() == iRouteId) {
      if (!(endPos < pCurrInt.GetStartPos() ||
            startPos > pCurrInt.GetEndPos() ||
            startPos == endPos ||
            pCurrInt.GetStartPos() == pCurrInt.GetEndPos())){
          //intersection exists compute intersecting part and timevalues for
          //resulting unit
        if (!swapped) {
          if (pCurrInt.GetStartPos() <= startPos) {
            found = true;
            interbool.timeInterval.start = startTime;
            interbool.timeInterval.lc = bstart;
            interbool.constValue.Set(true, true);
          } else {
            found = true;
            // compute and write unit befor mgpoint inside gline
            interbool.timeInterval.start = startTime;
            interbool.timeInterval.lc = bstart;
            factor = fabs(pCurrInt.GetStartPos() - startPos) /
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
          if (pCurrInt.GetEndPos() >= endPos) {
            interbool.timeInterval.end = endTime;
            interbool.timeInterval.rc = bend;
            pResult->MergeAdd(interbool);
          } else {
            // compute end of mgpoint at end of gline.and add result unit
            interbool.timeInterval.rc = true;
            factor = fabs(pCurrInt.GetEndPos() - startPos) /
                     fabs(endPos -startPos);
            tInterEnd = (endTime -startTime) * factor + startTime;
            interbool.timeInterval.end = tInterEnd;
            pResult->MergeAdd(interbool);
            // the rest of the current unit is not in the current
            // routeinterval.
            checkEndOfUGPoint(pCurrInt.GetEndPos(), endPos, tInterEnd, false,
                              endTime, bend, k, vRI, pResult, iRouteId);
          }
        } else {
          help = startPos;
          startPos = endPos;
          endPos = help;
          if (pCurrInt.GetEndPos() >= startPos) {
            found = true;
            interbool.timeInterval.start = startTime;
            interbool.timeInterval.lc = bstart;
            interbool.constValue.Set(true, true);
          } else {
            found = true;
            // compute and write unit befor mgpoint inside gline
            interbool.timeInterval.start = startTime;
            interbool.timeInterval.lc = bstart;
            factor =  fabs(pCurrInt.GetEndPos() - startPos) /
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
          if (pCurrInt.GetStartPos() <= endPos) {
              interbool.timeInterval.end = endTime;
              interbool.timeInterval.rc = bend;
              pResult->MergeAdd(interbool);
          } else {
              // compute end of mgpoint at end of gline.and add result unit
              interbool.timeInterval.rc = true;
              factor = fabs(pCurrInt.GetStartPos() - startPos) /
                       fabs(endPos - startPos);
              tInterEnd = (endTime - startTime) * factor + startTime;
              interbool.timeInterval.end = tInterEnd;
              pResult->MergeAdd(interbool);
              // the rest of the current unit is not in the current
              // routeinterval.
              checkEndOfUGPoint(pCurrInt.GetEndPos(), endPos, tInterEnd, false,
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
          if (pCurrInt.GetStartPos() == pCurrInt.GetEndPos()) {
            found = true;
            if ((pCurrInt.GetStartPos() > startPos &&
                 pCurrInt.GetStartPos() < endPos)||
              (pCurrInt.GetStartPos() < startPos &&
                 pCurrInt.GetStartPos() > endPos)) {
              // compute and write unit befor mgpoint inside gline
              interbool.timeInterval.start = startTime;
              interbool.timeInterval.lc = bstart;
              factor = fabs(pCurrInt.GetStartPos() - startPos) /
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
              checkEndOfUGPoint(pCurrInt.GetEndPos(), endPos, tInterEnd, false,
                                endTime, bend, k, vRI, pResult, iRouteId);
            } else {
              if (pCurrInt.GetStartPos() == startPos && bstart) {
                found = true;
                interbool.timeInterval.start = startTime;
                interbool.timeInterval.lc = bstart;
                interbool.timeInterval.end = startTime;
                interbool.timeInterval.rc = true;
                interbool.constValue.Set(true,true);
                pResult->MergeAdd(interbool);
                checkEndOfUGPoint(pCurrInt.GetEndPos(), endPos, tInterEnd,
                                  false, endTime, bend, k, vRI, pResult,
                                  iRouteId);
              } else {
                if (pCurrInt.GetStartPos() == endPos && bend) {
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
            if ((startPos == pCurrInt.GetStartPos() &&
                 endPos == pCurrInt.GetEndPos())||
               (startPos == pCurrInt.GetEndPos() &&
                 endPos == pCurrInt.GetStartPos())){
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
Returns the ~RouteIntervals~ of a route between mgpstart and mgpend.

Used by operators ~at~ and ~inside~.

*/

void getRouteIntervals(GLine *&pGLine, int iRouteId, double mgpstart,
                       double mgpend, int low, int high,
                       vector<RouteInterval> &vRI){
  vRI.clear();
  RouteInterval aktRI;
  bool found;
  int mid;
  if (low <= high) {
    mid = (high + low) / 2;
    if  (!(mid < 0 || mid >= pGLine->NoOfComponents())) {
      pGLine->Get(mid, aktRI);
      if (aktRI.GetRouteId() < iRouteId) {
        getRouteIntervals(pGLine, iRouteId, mgpstart, mgpend, mid+1, high, vRI);
      } else {
        if (aktRI.GetRouteId() > iRouteId) {
          getRouteIntervals(pGLine, iRouteId, mgpstart, mgpend, low, mid-1,
                            vRI);
        } else {
          if (aktRI.GetStartPos() > mgpend) {
            getRouteIntervals(pGLine, iRouteId, mgpstart, mgpend, low, mid-1,
                              vRI);
          } else {
            if (aktRI.GetEndPos() < mgpstart) {
              getRouteIntervals(pGLine, iRouteId, mgpstart, mgpend, mid+1, high,
                                vRI);
            } else {
              if (mid > 0) {
                mid--;
                found = false;
                while (mid >= 0 && !found) {
                  pGLine->Get(mid, aktRI);
                  if (aktRI.GetRouteId() == iRouteId &&
                      aktRI.GetEndPos() >= mgpstart) {
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
             vRI.push_back(aktRI);
              mid++;
              found = false;
              while (!found && mid < pGLine->NoOfComponents() ){
                pGLine->Get(mid, aktRI);
                if (aktRI.GetRouteId() == iRouteId &&
                    aktRI.GetStartPos() <= mgpend) {
                  vRI.push_back(aktRI);
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
Translates two given ~MGPoint~ to two ~MGPoint~ which have a identical number
of units with identical time intervals. Used by operator ~intersection~ and
later on ~distance~ and ~netdistance~

*/

void refinementMovingGPoint (MGPoint *a, MGPoint *b,
                             MGPoint *&resA, MGPoint *&resB){
  UGPoint ua, ub;
  GPoint pos1, pos2;
  int i = 0;
  int j = 0;
  a->Get(i,ua);
  b->Get(j,ub);
  resA->StartBulkLoad();
  resB->StartBulkLoad();
  while (i < a->GetNoComponents() && j < b->GetNoComponents()) {
    switch (intervalCheck(ua.timeInterval, ub.timeInterval)) {
      case 1: //no units to add
        j++;
        if (j < b->GetNoComponents()) b->Get(j, ub);
        break;
      case 2:
        if (ua.timeInterval.lc && ub.timeInterval.rc) {
          resA->Add(UGPoint(Interval<Instant>(ua.timeInterval.start,
                                             ua.timeInterval.start,
                                             true, true),
                                             ua.p0.GetNetworkId(),
                                             ua.p0.GetRouteId(),
                                             ua.p0.GetSide(),
                                             ua.p0.GetPosition(),
                                             ua.p0.GetPosition()));
          resB->Add(UGPoint(Interval<Instant>(ub.timeInterval.end,
                                             ub.timeInterval.end,
                                             true, true),
                                             ub.p1.GetNetworkId(),
                                             ub.p1.GetRouteId(),
                                             ub.p1.GetSide(),
                                             ub.p1.GetPosition(),
                                             ub.p1.GetPosition()));
        }
        j++;
        if (j < b->GetNoComponents()) b->Get(j, ub);
        break;
      case 3:
        ub.TemporalFunction(ua.timeInterval.start, pos1, false);
        ub.TemporalFunction(ua.timeInterval.end, pos2, false);
        resA->Add(UGPoint(ua.timeInterval,
                                             ua.p0.GetNetworkId(),
                                             ua.p0.GetRouteId(),
                                             ua.p0.GetSide(),
                                             ua.p0.GetPosition(),
                                             ua.p1.GetPosition()));
        resB->Add(UGPoint(ua.timeInterval,
                                             ub.p0.GetNetworkId(),
                                             ub.p0.GetRouteId(),
                                             ub.p0.GetSide(),
                                             pos1.GetPosition(),
                                             pos2.GetPosition()));
        i++;
        if (i < a->GetNoComponents()) a->Get(i, ua);
        break;
      case 4:
        ub.TemporalFunction(ua.timeInterval.start, pos2, false);
        resA->Add(UGPoint(Interval<Instant>(ua.timeInterval.start,
                                           ua.timeInterval.end,
                                           ua.timeInterval.lc,
                                           ua.timeInterval.rc &&
                                           ub.timeInterval.rc),
                                             ua.p0.GetNetworkId(),
                                             ua.p0.GetRouteId(),
                                             ua.p0.GetSide(),
                                             ua.p0.GetPosition(),
                                             ua.p1.GetPosition()));
        resB->Add(UGPoint(Interval<Instant>(ua.timeInterval.start,
                                           ua.timeInterval.end,
                                           ua.timeInterval.lc,
                                           ua.timeInterval.rc &&
                                           ub.timeInterval.rc),
                                             ub.p0.GetNetworkId(),
                                             ub.p0.GetRouteId(),
                                             ub.p0.GetSide(),
                                             pos2.GetPosition(),
                                             ub.p1.GetPosition()));
        if ((ua.timeInterval.rc && ub.timeInterval.rc) ||
            (!ua.timeInterval.rc && !ub.timeInterval.rc)) {
          i++;
          if (i < a->GetNoComponents()) a->Get(i, ua);
          j++;
          if (j < b->GetNoComponents()) b->Get(j, ub);
        } else {
          if (ua.timeInterval.rc && !ub.timeInterval.rc) {
            j++;
            if (j < b->GetNoComponents()) b->Get(j, ub);
          }
          if (ub.timeInterval.rc && !ua.timeInterval.rc) {
            i++;
            if (i < a->GetNoComponents()) a->Get(i, ua);
          }
        }
        break;
      case 5:
        ua.TemporalFunction(ub.timeInterval.end, pos1, false);
        ub.TemporalFunction(ua.timeInterval.start, pos2, false);
        resA->Add(UGPoint(Interval<Instant>(ua.timeInterval.start,
                                             ub.timeInterval.end,
                                             ua.timeInterval.lc,
                                             ub.timeInterval.rc),
                                            ua.p0.GetNetworkId(),
                                            ua.p0.GetRouteId(),
                                            ua.p0.GetSide(),
                                            ua.p0.GetPosition(),
                                            pos1.GetPosition()));
        resB->Add(UGPoint(Interval<Instant>(ua.timeInterval.start,
                                             ub.timeInterval.end,
                                             ua.timeInterval.lc,
                                             ub.timeInterval.rc),
                                            ub.p0.GetNetworkId(),
                                            ub.p0.GetRouteId(),
                                            ub.p0.GetSide(),
                                            pos2.GetPosition(),
                                            ub.p1.GetPosition()));
        j++;
        if (j < b->GetNoComponents()) b->Get(j, ub);
        break;
      case 6:
        ua.TemporalFunction(ub.timeInterval.end, pos1, false);
        resA->Add(UGPoint(Interval<Instant>(ub.timeInterval.start,
                                           ub.timeInterval.end,
                                           ub.timeInterval.lc &&
                                           ua.timeInterval.lc,
                                           ub.timeInterval.rc),
                                             ua.p0.GetNetworkId(),
                                             ua.p0.GetRouteId(),
                                             ua.p0.GetSide(),
                                             ua.p0.GetPosition(),
                                             pos1.GetPosition()));
        resB->Add(UGPoint(Interval<Instant>(ub.timeInterval.start,
                                           ub.timeInterval.end,
                                           ub.timeInterval.lc &&
                                           ua.timeInterval.lc,
                                           ub.timeInterval.rc),
                                             ub.p0.GetNetworkId(),
                                             ub.p0.GetRouteId(),
                                             ub.p0.GetSide(),
                                             ub.p0.GetPosition(),
                                             ub.p1.GetPosition()));
        j++;
        if (j < b->GetNoComponents()) b->Get(j, ub);
        break;
      case 7:
        resA->Add(UGPoint(Interval<Instant>(ua.timeInterval.start,
                                           ua.timeInterval.end,
                                           ua.timeInterval.lc &&
                                           ub.timeInterval.lc,
                                           ua.timeInterval.rc &&
                                           ub.timeInterval.rc),
                                             ua.p0.GetNetworkId(),
                                             ua.p0.GetRouteId(),
                                             ua.p0.GetSide(),
                                             ua.p0.GetPosition(),
                                             ua.p1.GetPosition()));
        resB->Add(UGPoint(Interval<Instant>(ua.timeInterval.start,
                                           ua.timeInterval.end,
                                           ua.timeInterval.lc &&
                                           ub.timeInterval.lc,
                                           ua.timeInterval.rc &&
                                           ub.timeInterval.rc),
                                             ub.p0.GetNetworkId(),
                                             ub.p0.GetRouteId(),
                                             ub.p0.GetSide(),
                                             ub.p0.GetPosition(),
                                             ub.p1.GetPosition()));
        if ((ua.timeInterval.rc && ub.timeInterval.rc) ||
            (!ua.timeInterval.rc && !ub.timeInterval.rc)) {
          i++;
          if (i < a->GetNoComponents()) a->Get(i, ua);
          j++;
          if (j < b->GetNoComponents()) b->Get(j, ub);
        } else {
          if (ua.timeInterval.rc && !ub.timeInterval.rc) {
            j++;
            if (j < b->GetNoComponents()) b->Get(j, ub);
          }
          if (ub.timeInterval.rc && !ua.timeInterval.rc) {
            i++;
            if (i < a->GetNoComponents()) a->Get(i, ua);
          }
        }
        break;
      case 8:
        ub.TemporalFunction(ua.timeInterval.end, pos2, false);
        resA->Add(UGPoint(Interval<Instant>(ua.timeInterval.start,
                                           ua.timeInterval.end,
                                           ua.timeInterval.lc &&
                                           ub.timeInterval.lc,
                                           ua.timeInterval.rc),
                                             ua.p0.GetNetworkId(),
                                             ua.p0.GetRouteId(),
                                             ua.p0.GetSide(),
                                             ua.p0.GetPosition(),
                                             ua.p1.GetPosition()));
        resB->Add(UGPoint(Interval<Instant>(ua.timeInterval.start,
                                           ua.timeInterval.end,
                                           ua.timeInterval.lc &&
                                           ub.timeInterval.lc,
                                           ua.timeInterval.rc),
                                             ub.p0.GetNetworkId(),
                                             ub.p0.GetRouteId(),
                                             ub.p0.GetSide(),
                                             ub.p0.GetPosition(),
                                             pos2.GetPosition()));
        i++;
        if (i < a->GetNoComponents()) a->Get(i, ua);
        break;
      case 9:
        if (ua.timeInterval.lc && ub.timeInterval.lc && ub.timeInterval.rc){
          resA->Add(UGPoint(Interval<Instant>(ua.timeInterval.start,
                                           ua.timeInterval.start,
                                           true, true),
                                             ua.p0.GetNetworkId(),
                                             ua.p0.GetRouteId(),
                                             ua.p0.GetSide(),
                                             ua.p0.GetPosition(),
                                             ua.p0.GetPosition()));
          resB->Add(UGPoint(Interval<Instant>(ua.timeInterval.start,
                                             ua.timeInterval.start,
                                             true, true),
                                             ub.p0.GetNetworkId(),
                                             ub.p0.GetRouteId(),
                                             ub.p0.GetSide(),
                                             ub.p0.GetPosition(),
                                             ub.p0.GetPosition()));
        }
        j++;
        if (j < b->GetNoComponents()) b->Get(j, ub);
        break;
      case 10:
        ua.TemporalFunction(ub.timeInterval.start, pos1, false);
        ua.TemporalFunction(ub.timeInterval.end, pos2, false);
        resA->Add(UGPoint(ub.timeInterval,
                                            ua.p0.GetNetworkId(),
                                            ua.p0.GetRouteId(),
                                            ua.p0.GetSide(),
                                            pos1.GetPosition(),
                                            pos2.GetPosition()));
        resB->Add(UGPoint(ub.timeInterval,
                                            ub.p0.GetNetworkId(),
                                            ub.p0.GetRouteId(),
                                            ub.p0.GetSide(),
                                            ub.p0.GetPosition(),
                                            ub.p1.GetPosition()));
        j++;
        if (j < b->GetNoComponents()) b->Get(j, ub);
        break;
      case 11:
        ua.TemporalFunction(ub.timeInterval.start, pos1, false);
        resA->Add(UGPoint(ub.timeInterval,
                                             ua.p0.GetNetworkId(),
                                             ua.p0.GetRouteId(),
                                             ua.p0.GetSide(),
                                             pos1.GetPosition(),
                                             ua.p1.GetPosition()));
        resB->Add(UGPoint(ub.timeInterval,
                                             ub.p0.GetNetworkId(),
                                             ub.p0.GetRouteId(),
                                             ub.p0.GetSide(),
                                             ub.p0.GetPosition(),
                                             ub.p1.GetPosition()));
        if ((ua.timeInterval.rc && ub.timeInterval.rc) ||
            (!ua.timeInterval.rc && !ub.timeInterval.rc)) {
          i++;
          if (i < a->GetNoComponents()) a->Get(i, ua);
          j++;
          if (j < b->GetNoComponents()) b->Get(j, ub);
        } else {
          if (ua.timeInterval.rc && !ub.timeInterval.rc) {
            j++;
            if (j < b->GetNoComponents()) b->Get(j, ub);
          }
          if (ub.timeInterval.rc && !ua.timeInterval.rc) {
            i++;
            if (i < a->GetNoComponents()) a->Get(i, ua);
          }
        }
        break;
      case 12:
        ua.TemporalFunction(ub.timeInterval.start, pos1, false);
        ub.TemporalFunction(ua.timeInterval.end, pos2, false);
        resA->Add(UGPoint(Interval<Instant>(ub.timeInterval.start,
                                           ua.timeInterval.end,
                                           ub.timeInterval.lc,
                                           ua.timeInterval.rc),
                                             ua.p0.GetNetworkId(),
                                             ua.p0.GetRouteId(),
                                             ua.p0.GetSide(),
                                             pos1.GetPosition(),
                                             ua.p1.GetPosition()));
        resB->Add(UGPoint(Interval<Instant>(ub.timeInterval.start,
                                           ua.timeInterval.end,
                                           ub.timeInterval.lc,
                                           ua.timeInterval.rc),
                                             ub.p0.GetNetworkId(),
                                             ub.p0.GetRouteId(),
                                             ub.p0.GetSide(),
                                             ub.p0.GetPosition(),
                                             pos2.GetPosition()));
        i++;
        if (i < a->GetNoComponents()) a->Get(i, ua);
        break;
      case 13:
        if (ub.timeInterval.lc && ub.timeInterval.rc) {
          ua.TemporalFunction(ub.timeInterval.start, pos1, false);
          resA->Add(UGPoint(ub.timeInterval,
                                             ua.p0.GetNetworkId(),
                                             ua.p0.GetRouteId(),
                                             ua.p0.GetSide(),
                                             pos1.GetPosition(),
                                             pos1.GetPosition()));
          resB->Add(UGPoint(ub.timeInterval,
                                             ub.p0.GetNetworkId(),
                                             ub.p0.GetRouteId(),
                                             ub.p0.GetSide(),
                                             ub.p0.GetPosition(),
                                             ub.p0.GetPosition()));
        }
        j++;
        if (j < b->GetNoComponents()) b->Get(j, ub);
        break;
      case 14:
        if (ua.timeInterval.rc && ub.timeInterval.lc && ub.timeInterval.rc) {
          resA->Add(UGPoint(ub.timeInterval, ua.p1, ua.p1));
          resB->Add(UGPoint(ub.timeInterval, ub.p0, ub.p0));
        }
        if ((ua.timeInterval.rc && ub.timeInterval.rc) ||
            (!ua.timeInterval.rc && !ub.timeInterval.rc)) {
          i++;
          if (i < a->GetNoComponents()) a->Get(i, ua);
          j++;
          if (j < b->GetNoComponents()) b->Get(j, ub);
        } else {
          if (ua.timeInterval.rc && !ub.timeInterval.rc) {
            j++;
            if (j < b->GetNoComponents()) b->Get(j, ub);
          }
          if (ub.timeInterval.rc && !ua.timeInterval.rc) {
            i++;
            if (i < a->GetNoComponents()) a->Get(i, ua);
          }
        }
        break;
      case 15:
        if (ua.timeInterval.rc && ub.timeInterval.lc) {
          resA->Add(UGPoint(Interval<Instant>(ua.timeInterval.end,
                                              ua.timeInterval.end,
                                              true, true),
                                             ua.p0.GetNetworkId(),
                                             ua.p0.GetRouteId(),
                                             ua.p0.GetSide(),
                                             ua.p1.GetPosition(),
                                             ua.p1.GetPosition()));
          resB->Add(UGPoint(Interval<Instant>(ua.timeInterval.end,
                                              ua.timeInterval.end,
                                              true, true),
                                             ub.p0.GetNetworkId(),
                                             ub.p0.GetRouteId(),
                                             ub.p0.GetSide(),
                                             ub.p0.GetPosition(),
                                             ub.p0.GetPosition()));
        }
        i++;
        if (i < a->GetNoComponents()) a->Get(i, ua);
        break;
      case 16: //nothing to add
        i++;
        if (i < a->GetNoComponents()) a->Get(i, ua);
        break;
      case 17:
        if (ua.timeInterval.lc && ua.timeInterval.rc && ub.timeInterval.lc){
          resA->Add(UGPoint(Interval<Instant>(ua.timeInterval.start,
                                             ua.timeInterval.end,
                                             true, true),
                                             ua.p0.GetNetworkId(),
                                             ua.p0.GetRouteId(),
                                             ua.p0.GetSide(),
                                             ua.p0.GetPosition(),
                                             ua.p0.GetPosition()));
          resB->Add(UGPoint(Interval<Instant>(ua.timeInterval.start,
                                             ua.timeInterval.end,
                                             true, true),
                                             ub.p0.GetNetworkId(),
                                             ub.p0.GetRouteId(),
                                             ub.p0.GetSide(),
                                             ub.p0.GetPosition(),
                                             ub.p0.GetPosition()));
        }
        i++;
        if (i < a->GetNoComponents()) a->Get(i, ua);
        break;
      case 18:
        if (ua.timeInterval.lc && ua.timeInterval.rc && ub.timeInterval.rc){
          resA->Add(UGPoint(Interval<Instant>(ua.timeInterval.start,
                                             ua.timeInterval.end,
                                             true, true),
                                             ua.p0.GetNetworkId(),
                                             ua.p0.GetRouteId(),
                                             ua.p0.GetSide(),
                                             ua.p0.GetPosition(),
                                             ua.p0.GetPosition()));
          resB->Add(UGPoint(Interval<Instant>(ua.timeInterval.start,
                                             ua.timeInterval.end,
                                             true, true),
                                             ub.p0.GetNetworkId(),
                                             ub.p0.GetRouteId(),
                                             ub.p0.GetSide(),
                                             ub.p1.GetPosition(),
                                             ub.p1.GetPosition()));
        }
        if ((ua.timeInterval.rc && ub.timeInterval.rc) ||
            (!ua.timeInterval.rc && !ub.timeInterval.rc)) {
          i++;
          if (i < a->GetNoComponents()) a->Get(i, ua);
          j++;
          if (j < b->GetNoComponents()) b->Get(j, ub);
        } else {
          if (ua.timeInterval.rc && !ub.timeInterval.rc) {
            j++;
            if (j < b->GetNoComponents()) b->Get(j, ub);
          }
          if (ub.timeInterval.rc && !ua.timeInterval.rc) {
            i++;
            if (i < a->GetNoComponents()) a->Get(i, ua);
          }
        }
        break;
      case 19:
        if (ua.timeInterval.lc && ua.timeInterval.rc &&
           ub.timeInterval.lc && ub.timeInterval.rc) {
          resA->Add(UGPoint(Interval<Instant> (ua.timeInterval.start,
                                              ua.timeInterval.start,
                                              true,true),
                                              ua.p0.GetNetworkId(),
                                              ua.p0.GetRouteId(),
                                              ua.p0.GetSide(),
                                              ua.p0.GetPosition(),
                                              ua.p0.GetPosition()));
          resB->Add(UGPoint(Interval<Instant> (ub.timeInterval.end,
                                              ub.timeInterval.end,
                                              true,true),
                                              ub.p0.GetNetworkId(),
                                              ub.p0.GetRouteId(),
                                              ub.p0.GetSide(),
                                              ub.p0.GetPosition(),
                                              ub.p0.GetPosition()));
        }
        if ((ua.timeInterval.rc && ub.timeInterval.rc) ||
            (!ua.timeInterval.rc && !ub.timeInterval.rc)) {
          i++;
          if (i < a->GetNoComponents()) a->Get(i, ua);
          j++;
          if (j < b->GetNoComponents()) b->Get(j, ub);
        } else {
          if (ua.timeInterval.rc && !ub.timeInterval.rc) {
            j++;
            if (j < b->GetNoComponents()) b->Get(j, ub);
          }
          if (ub.timeInterval.rc && !ua.timeInterval.rc) {
            i++;
            if (i < a->GetNoComponents()) a->Get(i, ua);
          }
        }
        break;
      case 20:
        if (ua.timeInterval.lc && ua.timeInterval.rc) {
          ub.TemporalFunction(ua.timeInterval.start, pos2, false);
          resA->Add(UGPoint(ua.timeInterval,
                                             ua.p0.GetNetworkId(),
                                             ua.p0.GetRouteId(),
                                             ua.p0.GetSide(),
                                             ua.p0.GetPosition(),
                                             ua.p0.GetPosition()));
          resB->Add(UGPoint(ua.timeInterval,
                                             ub.p0.GetNetworkId(),
                                             ub.p0.GetRouteId(),
                                             ub.p0.GetSide(),
                                             pos2.GetPosition(),
                                             pos2.GetPosition()));
        }
        i++;
        if (i < a->GetNoComponents()) a->Get(i, ua);
        break;
      default: //should never happen
        cerr << "an error occured while checking the time interval." << endl;
        resA->EndBulkLoad(false);
        resB->EndBulkLoad(false);
        resA->SetDefined(false);
        resB->SetDefined(false);
        resA->SetTrajectoryDefined(false);
        resB->SetTrajectoryDefined(false);
        resA->SetBoundingBoxDefined(false);
        resB->SetBoundingBoxDefined(false);
        return;
    } // end switch
  }//end while
  resA->EndBulkLoad(true);
  resB->EndBulkLoad(true);
  resA->SetDefined(true);
  resB->SetDefined(true);
  resA->SetTrajectoryDefined(false);
  resB->SetTrajectoryDefined(false);
  resA->m_trajectory.TrimToSize();
  resB->m_trajectory.TrimToSize();
  resA->SetBoundingBoxDefined(false);
  resB->SetBoundingBoxDefined(false);
  return;
}

/*
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
2 class ~MGPoint~

Constructors

*/

MGPoint::MGPoint( const int n ):Mapping< UGPoint, GPoint >(n), m_trajectory (0)
{
  m_length = 0.0;
  m_bbox = Rectangle<3>(false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 );
  m_traj_Defined = false;
}

/*
Functions for integration in SECONDO

*/
void MGPoint::Clear()
{
  //if (IsDefined()){
    Mapping<UGPoint, GPoint>::Clear();
    m_length = 0.0;
    m_trajectory.clean();
    m_traj_Defined = false;
    m_bbox = Rectangle<3>(false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 );
  //}
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
Getter and Setter Methods

*/

int MGPoint::GetNetworkId() const{
  if (GetNoComponents()>0)
  {
    UGPoint u;
    Get(0,u);
    return u.p0.GetNetworkId();
  }
   else return numeric_limits<int>::min();
}

Network* MGPoint::GetNetwork() const
{
  return NetworkManager::GetNetworkNew(GetNetworkId(),netList);
}

double MGPoint::GetLength() const{
  if (IsDefined()) return m_length;
  else return 0.0;
}

double MGPoint::Length(){
  if (IsDefined()) return m_length;
  else return 0.0;
}

void MGPoint::SetBoundingBox(Rectangle<3> mbr){
  m_bbox = mbr;
}

void MGPoint::SetTrajectory(const DbArray<RouteInterval>& tra){
  if (tra.Size() > 0){
    m_traj_Defined = true;
    /*m_trajectory.resize(tra.Size());
    RouteInterval ri;
    for (int i = 0; i < tra.Size(); i++) {
      tra.Get(i,ri);
      m_trajectory.Put(i,ri);
    }*/
    m_trajectory.copyFrom(tra);
  } else m_traj_Defined = false;
}

void MGPoint::SetTrajectory(GLine src){
  if (src.IsDefined() && src.NoOfComponents() > 0) {
    m_trajectory.copyFrom(*(src.GetRouteIntervals()));
    /*m_trajectory.resize(src.NoOfComponents());
    RouteInterval ri;
    for (int i = 0; i < src.NoOfComponents(); i++) {
      src.Get(i,ri);
      m_trajectory.Put(i,ri);
    }*/
    m_traj_Defined = true;
  }else m_traj_Defined = false;
}

void MGPoint::Trajectory(GLine* res) {
  if (m_traj_Defined) {
    if (m_trajectory.Size() > 0) {
      res->SetNetworkId(GetNetworkId());
      RouteInterval ri;
      for (int i = 0; i < m_trajectory.Size(); i++){
        m_trajectory.Get(i,ri);
        res->AddRouteInterval(ri.GetRouteId(), ri.GetStartPos(),
                              ri.GetEndPos());
      }
      res->SetSorted(true);
      res->SetDefined(true);
    } else res->SetDefined(false);
  } else {
    if (GetNoComponents() > 0) {
      UGPoint pCurrentUnit;
      Get(0, pCurrentUnit);
      res->SetNetworkId(GetNetworkId());
      res->SetDefined(true);
      int aktRouteId = pCurrentUnit.p0.GetRouteId();
      double aktStartPos = pCurrentUnit.p0.GetPosition();
      double aktEndPos = pCurrentUnit.p1.GetPosition();
      chkStartEnd(aktStartPos, aktEndPos);
      RITree *tree;
      tree = new RITree(aktRouteId, aktStartPos, aktEndPos,0,0);
      int curRoute;
      double curStartPos, curEndPos;
      for (int i = 1; i < GetNoComponents(); i++)
      {
        // Get start and end of current unit
        Get(i, pCurrentUnit);
        curRoute = pCurrentUnit.p0.GetRouteId();
        curStartPos = pCurrentUnit.p0.GetPosition();
        curEndPos = pCurrentUnit.p1.GetPosition();
        chkStartEnd(curStartPos, curEndPos);
        if (curRoute != aktRouteId) {
          tree->Insert(aktRouteId, aktStartPos, aktEndPos);
          aktRouteId = curRoute;
          aktStartPos = curStartPos;
          aktEndPos = curEndPos;
        } else { // curRoute == aktRouteId concat pieces if possible
          if (curEndPos >= aktStartPos && curStartPos < aktStartPos)
            aktStartPos = curStartPos;
          else {
            if (curStartPos <= aktEndPos && aktEndPos < curEndPos)
              aktEndPos = curEndPos;
            else { //concat impossible start new routeInterval for gline.
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
      tree->TreeToDbArray(&(this->m_trajectory));
      tree->RemoveTree();
      res->SetSorted(true);
      res->SetDefined(true);
      SetTrajectoryDefined(true);
    } else res->SetDefined(false);
  }
  res->TrimToSize();
}

void MGPoint::Deftime(Periods &res){
    res.Clear();
    Periods per(0);
    UGPoint unit;
    per.StartBulkLoad();
    for( int i = 0; i < GetNoComponents(); i++ ) {
      Get( i, unit );
      if (unit.IsDefined())
        per.MergeAdd(unit.timeInterval);
    }
    per.EndBulkLoad(false);
}


/*
Operations with ~mgpoint~

Euclidean Distance computing

*/

void MGPoint::Distance(MGPoint *&mgp, MReal *&result){
  if (IsDefined()&&mgp->IsDefined()){
    MPoint *p1 = new MPoint(0);
    MPoint *p2 = new MPoint(0);
    Mgpoint2mpoint(p1);
    mgp->Mgpoint2mpoint(p2);
    UReal uReal(true);

    RefinementPartition<MPoint, MPoint, UPoint, UPoint> rp(*p1, *p2);

    result->Clear();
    result->Resize(rp.Size());
    result->StartBulkLoad();
    for( unsigned int i = 0; i < rp.Size(); i++ )
    {
      Interval<Instant> iv;
      int u1Pos, u2Pos;
      UPoint u1;
      UPoint u2;

      rp.Get(i, iv, u1Pos, u2Pos);
      if (u1Pos == -1 || u2Pos == -1)
        continue;
      else {
        p1->Get(u1Pos, u1);
        p2->Get(u2Pos, u2);
      }
      if(u1.IsDefined() && u2.IsDefined())
      { // do not need to test for overlapping deftimes anymore...
        u1.Distance( u2, uReal );
        result->MergeAdd( uReal );
      }
    }
    result->EndBulkLoad();
    p1->DeleteIfAllowed();
    p2->DeleteIfAllowed();
  }
}

void MGPoint::DistanceE(MGPoint* mgp, MReal* result){
  //Network* pNetwork = NetworkManager::GetNetwork(mgp->GetNetworkId());
  Network* pNetwork = GetNetwork();
  UReal uReal(true);
  result->StartBulkLoad();
  UGPoint ugp1;
  UGPoint ugp2;
  UGPoint* ug1 = new UGPoint(true);
  UGPoint* ug2 = new UGPoint(true);
  Instant start;
  Instant end;
  Get(0,ugp1);
  Get(0,ugp2);
  if(ugp1.timeInterval.start < ugp2.timeInterval.start)
    start = ugp2.timeInterval.start;
  else
    start = ugp1.timeInterval.start;
  Get(GetNoComponents() - 1,ugp1);
  mgp->Get(mgp->GetNoComponents() -1 ,ugp2);
  if(ugp1.timeInterval.end < ugp2.timeInterval.end)
    end = ugp1.timeInterval.end;
  else
    end = ugp2.timeInterval.end;
  int pos1 = 0,pos2 = 0;
  uReal.timeInterval.start = start;
  Get(pos1,ugp1);
  mgp->Get(pos2,ugp2);
  *ug1 = ugp1;
  *ug2 = ugp2;
  while(1){
    assert(ug1->timeInterval.start >= uReal.timeInterval.start);
    assert(ug2->timeInterval.start >= uReal.timeInterval.start);
    if(ug2->timeInterval.end < ug1->timeInterval.start){
      pos2++;
      mgp->Get(pos2,ugp2);
      *ug2 = ugp2;
      continue;
    }
    if(ug2->timeInterval.start > ug1->timeInterval.end){
      pos1++;
      Get(pos1,ugp1);
      *ug1 = ugp1;
      continue;
    }
    //starttime
    if(ug1->timeInterval.start < uReal.timeInterval.start){ //split
      GPoint gp0;
      ug1->TemporalFunction(uReal.timeInterval.start,gp0,true);
      assert(gp0.IsDefined());
      ug1->timeInterval.start = uReal.timeInterval.start;
      ug1->p0 = gp0;
    }
    if(ug2->timeInterval.start < uReal.timeInterval.start){ //split
      GPoint gp0;
      ug2->TemporalFunction(uReal.timeInterval.start,gp0,true);
      assert(gp0.IsDefined());
      ug2->timeInterval.start = uReal.timeInterval.start;
      ug2->p0 = gp0;
    }
    //endtime
    if(ug1->timeInterval.end < ug2->timeInterval.end){
      uReal.timeInterval.end = ug1->timeInterval.end;
      GPoint gp1;
      ug2->TemporalFunction(uReal.timeInterval.end,gp1,true);
      assert(gp1.IsDefined());
      ug2->timeInterval.end = uReal.timeInterval.end;
      ug2->p1 = gp1;
      Point* p1_0 = new Point();
      p1_0->SetDefined(true);
      pNetwork->GetPointOnRoute(&ug1->p0,p1_0);
      Point* p1_1 = new Point();
      p1_1->SetDefined(true);
      pNetwork->GetPointOnRoute(&ug1->p1,p1_1);
      Point* p2_0 = new Point();
      p2_0->SetDefined(true);
      pNetwork->GetPointOnRoute(&ug2->p0,p2_0);
      Point* p2_1 = new Point();
      p2_1->SetDefined(true);
      pNetwork->GetPointOnRoute(&ug2->p0,p2_1);
     double dt = (uReal.timeInterval.end-uReal.timeInterval.start).ToDouble();
      if(AlmostEqual(dt,0)){
        uReal.a = 0.0;
        uReal.b = 0.0;
        uReal.c =  pow(fabs(p1_0->GetX()-p2_0->GetX()),2) +
                 pow(fabs(p1_0->GetY()-p2_0->GetY()),2);
        uReal.r = false;
        result->MergeAdd(uReal);
      }else{
        double v1_x = (p1_1->GetX() - p1_0->GetX()) / dt;
        double v1_y = (p1_1->GetY() - p1_0->GetY()) / dt;
        double v2_x = (p2_1->GetX() - p2_0->GetX()) / dt;
        double v2_y = (p2_1->GetY() - p2_0->GetY()) / dt;
        double a = pow(fabs(v1_x-v2_x),2)+pow(fabs(v1_y-v2_y),2);
        double b = 2*((p1_0->GetX()-p2_0->GetX())*(v1_x-v2_x) +
                      (p1_0->GetY()-p2_0->GetY())*(v1_y-v2_y));
        double c = pow(fabs(p1_0->GetX()-p2_0->GetX()),2) +
                 pow(fabs(p1_0->GetY()-p2_0->GetY()),2);
        uReal.a = a;
        uReal.b = b;
        uReal.c = c;
        uReal.r = true;
        result->MergeAdd(uReal);
      }
      uReal.r = false;
      uReal.timeInterval.start = uReal.timeInterval.end;
      pos1++;
      if(pos1 == GetNoComponents()){
        p1_0->DeleteIfAllowed();
        p1_1->DeleteIfAllowed();
        p2_0->DeleteIfAllowed();
        p2_1->DeleteIfAllowed();
        break;
      }
      Get(pos1,ugp1);
      *ug1 = ugp1;
      mgp->Get(pos2,ugp2);
      *ug2 = ugp2;
      GPoint gp0;
      ug2->TemporalFunction(uReal.timeInterval.start,gp0,true);
      assert(gp0.IsDefined());
      ug2->timeInterval.start = uReal.timeInterval.start;
      ug2->p0 = gp0;
      p1_0->DeleteIfAllowed();
      p1_1->DeleteIfAllowed();
      p2_0->DeleteIfAllowed();
      p2_1->DeleteIfAllowed();
    }else{ //ugp1->end > ugp2.end
      uReal.timeInterval.end = ug2->timeInterval.end;
      GPoint gp1;
      ug1->TemporalFunction(uReal.timeInterval.end,gp1,true);
      assert(gp1.IsDefined());
      ug1->timeInterval.end = uReal.timeInterval.end;
      ug1->p1 = gp1;
      Point* p1_0 = new Point();
      p1_0->SetDefined(true);
      pNetwork->GetPointOnRoute(&ug1->p0,p1_0);
      Point* p1_1 = new Point();
      p1_1->SetDefined(true);
      pNetwork->GetPointOnRoute(&ug1->p1,p1_1);
      Point* p2_0 = new Point();
      p2_0->SetDefined(true);
      pNetwork->GetPointOnRoute(&ug2->p0,p2_0);
      Point* p2_1 = new Point();
      p2_1->SetDefined(true);
      pNetwork->GetPointOnRoute(&ug2->p0,p2_1);
      double dt = (uReal.timeInterval.end-uReal.timeInterval.start).ToDouble();
      if(AlmostEqual(dt,0)){
        uReal.a = 0.0;
        uReal.b = 0.0;
        uReal.c =  pow(fabs(p1_0->GetX()-p2_0->GetX()),2) +
                 pow(fabs(p1_0->GetY()-p2_0->GetY()),2);
        uReal.r = false;
       result->MergeAdd(uReal);
      }else{
        double v1_x = (p1_1->GetX() - p1_0->GetX()) / dt;
        double v1_y = (p1_1->GetY() - p1_0->GetY()) / dt;
        double v2_x = (p2_1->GetX() - p2_0->GetX()) / dt;
        double v2_y = (p2_1->GetY() - p2_0->GetY()) / dt;
        double a = pow(fabs(v1_x-v2_x),2)+pow(fabs(v1_y-v2_y),2);
        double b = 2*((p1_0->GetX()-p2_0->GetX())*(v1_x-v2_x) +
                      (p1_0->GetY()-p2_0->GetY())*(v1_y-v2_y));
        double c = pow(fabs(p1_0->GetX()-p2_0->GetX()),2) +
                 pow(fabs(p1_0->GetY()-p2_0->GetY()),2);
        uReal.a = a;
        uReal.b = b;
        uReal.c = c;
        uReal.r = true;
        result->MergeAdd(uReal);
      }
      uReal.r = false;
      uReal.timeInterval.start = uReal.timeInterval.end;
      pos2++;
      if(pos2 == mgp->GetNoComponents()){
        p1_0->DeleteIfAllowed();
        p1_1->DeleteIfAllowed();
        p2_0->DeleteIfAllowed();
        p2_1->DeleteIfAllowed();
        break;
      }
      mgp->Get(pos2,ugp2);
      *ug2 = ugp2;
      Get(pos1,ugp1);
      *ug1 = ugp1;
      GPoint gp0;
      ug1->TemporalFunction(uReal.timeInterval.start,gp0,true);
      assert(gp0.IsDefined());
      ug1->timeInterval.start = uReal.timeInterval.start;
      ug1->p0 = gp0;
      p1_0->DeleteIfAllowed();
      p1_1->DeleteIfAllowed();
      p2_0->DeleteIfAllowed();
      p2_1->DeleteIfAllowed();
    }
  }
  ug1->DeleteIfAllowed();
  ug2->DeleteIfAllowed();
  NetworkManager::CloseNetwork(pNetwork);
  result->EndBulkLoad();
}

void MGPoint::DistanceFunction(UGPoint* ug1,UGPoint* ug2,Network* pNetwork,
vector<UReal>& dist)
{
  assert(ug1->timeInterval.start == ug2->timeInterval.start);
  assert(ug1->timeInterval.end == ug2->timeInterval.end);
  if(*ug1 == *ug2){
    UReal* ureal = new UReal(true);
    ureal->timeInterval = ug1->timeInterval;
    ureal->a = 0.0;
    ureal->b = 0.0;
    ureal->c = 0.0;
    ureal->r = true;
    dist.push_back(*ureal);
    ureal->DeleteIfAllowed();
    return;
  }
  GPoint* gp1 = new GPoint(true,GetNetworkId(),ug1->p0.GetRouteId(),
          (ug1->p0.GetPosition()+ug1->p1.GetPosition())/2,ug1->p0.GetSide());
  Tuple* sec1 = pNetwork->GetSectionOnRoute(gp1);
  GPoint* gp2 = new GPoint(true,GetNetworkId(),ug2->p0.GetRouteId(),
          (ug2->p0.GetPosition()+ug2->p1.GetPosition())/2,ug2->p0.GetSide());
  Tuple* sec2 = pNetwork->GetSectionOnRoute(gp2);
  gp1->DeleteIfAllowed();
  gp2->DeleteIfAllowed();
  double m1 = ((CcReal*)sec1->GetAttribute(SECTION_MEAS1))->GetRealval();
  double m2 = ((CcReal*)sec1->GetAttribute(SECTION_MEAS2))->GetRealval();
  int rid1 = ug1->p0.GetRouteId();
  GPoint* ep1_0 = new GPoint(true,GetNetworkId(),rid1,m1,ug1->p0.GetSide());
  GPoint* ep1_1 = new GPoint(true,GetNetworkId(),rid1,m2,ug1->p0.GetSide());
  double meas1 = fabs(ug1->p0.GetPosition() - ep1_0->GetPosition());
  double meas2 = fabs(ug1->p0.GetPosition() - ep1_1->GetPosition());
  m1 = ((CcReal*)sec2->GetAttribute(SECTION_MEAS1))->GetRealval();
  m2 = ((CcReal*)sec2->GetAttribute(SECTION_MEAS2))->GetRealval();
  int rid2 = ug2->p0.GetRouteId();
  GPoint* ep2_0 = new GPoint(true,GetNetworkId(),rid2,m1,ug2->p0.GetSide());
  GPoint* ep2_1 = new GPoint(true,GetNetworkId(),rid2,m2,ug2->p0.GetSide());
  double meas3 = fabs(ug2->p0.GetPosition() - ep2_0->GetPosition());
  double meas4 = fabs(ug2->p0.GetPosition() - ep2_1->GetPosition());
  //get the junction id
  vector<JunctionSortEntry> juns;
  CcInt* routeid1 = new CcInt(true,rid1);
  pNetwork->GetJunctionsOnRoute(routeid1,juns);
  TupleId j1 = 0;
  TupleId j2 = 0;
  for(unsigned int i = 0;i < juns.size();i++){
    Tuple* t_jun = juns[i].m_pJunction;
    if(((CcInt*)t_jun->GetAttribute(JUNCTION_ROUTE1_ID))->GetIntval() == rid1){
      if(fabs(((CcReal*)t_jun->GetAttribute(JUNCTION_ROUTE1_MEAS))->GetRealval()
          -ep1_0->GetPosition()) < 0.1)
        j1 = t_jun->GetTupleId();
      if(fabs(((CcReal*)t_jun->GetAttribute(JUNCTION_ROUTE1_MEAS))->GetRealval()
          -ep1_1->GetPosition()) < 0.1)
        j2 = t_jun->GetTupleId();
    }
    if(((CcInt*)t_jun->GetAttribute(JUNCTION_ROUTE2_ID))->GetIntval() == rid1){
      if(fabs(((CcReal*)t_jun->GetAttribute(JUNCTION_ROUTE2_MEAS))->GetRealval()
          -ep1_0->GetPosition()) < 0.1)
        j1 = t_jun->GetTupleId();
      if(fabs(((CcReal*)t_jun->GetAttribute(JUNCTION_ROUTE2_MEAS))->GetRealval()
          -ep1_1->GetPosition()) < 0.1)
        j2 = t_jun->GetTupleId();
    }
  }
  routeid1->DeleteIfAllowed();
  juns.clear();
  CcInt* routeid2 = new CcInt(true,rid2);
  pNetwork->GetJunctionsOnRoute(routeid2,juns);
  TupleId j3 = 0;
  TupleId j4 = 0;
  for(unsigned int i = 0;i < juns.size();i++){
    Tuple* t_jun = juns[i].m_pJunction;
    if(((CcInt*)t_jun->GetAttribute(JUNCTION_ROUTE1_ID))->GetIntval() == rid2){
      if(fabs(((CcReal*)t_jun->GetAttribute(JUNCTION_ROUTE1_MEAS))->GetRealval()
          -ep2_0->GetPosition()) < 0.1)
        j3 = t_jun->GetTupleId();
      if(fabs(((CcReal*)t_jun->GetAttribute(JUNCTION_ROUTE1_MEAS))->GetRealval()
          -ep2_1->GetPosition()) < 0.1)
        j4 = t_jun->GetTupleId();
    }
    if(((CcInt*)t_jun->GetAttribute(JUNCTION_ROUTE2_ID))->GetIntval() == rid2){
      if(fabs(((CcReal*)t_jun->GetAttribute(JUNCTION_ROUTE2_MEAS))->GetRealval()
          -ep2_0->GetPosition()) < 0.1)
        j3 = t_jun->GetTupleId();
      if(fabs(((CcReal*)t_jun->GetAttribute(JUNCTION_ROUTE2_MEAS))->GetRealval()
          -ep2_1->GetPosition()) < 0.1)
        j4 = t_jun->GetTupleId();
    }
  }
  routeid2->DeleteIfAllowed();
  //cout<<j1<<" "<<j2<<" "<<j3<<" "<<j4<<endl;
  //find network distance from storage
  double l1,l2,l3,l4;
  GLine* gline1 = new GLine(0);
  GLine* gline2 = new GLine(0);
  GLine* gline3 = new GLine(0);
  GLine* gline4 = new GLine(0);
  l1 = l2 = l3 = l4 = 0;
  if(j1 == 0){  // not junction point, the start or end point of one route
    assert(j2 != 0);
    if(j3 == 0){
      assert(j4 != 0);
      pNetwork->FindSP(j2,j4,l4,gline4);
      l3 = l4;
      l3 += fabs(ep2_1->GetPosition()-ep2_0->GetPosition());
      *gline3 = *gline4;
      gline3->AddRouteInterval(rid2,ep2_1->GetPosition(),ep2_0->GetPosition());
    }else{
      pNetwork->FindSP(j2,j3,l3,gline3);
      if(j4 == 0){
        l4 = l3;
        l4 += fabs(ep2_0->GetPosition()-ep2_1->GetPosition());
        *gline4 = *gline3;
       gline4->AddRouteInterval(rid2,ep2_0->GetPosition(),ep2_1->GetPosition());
      }else
        pNetwork->FindSP(j2,j4,l4,gline4);
    }
      l1 = l3;
      l1 += fabs(ep1_0->GetPosition()-ep1_1->GetPosition());
      *gline1 = *gline3;
      gline1->AddRouteInterval(rid1,ep1_0->GetPosition(),ep1_1->GetPosition());
    l2 = l4;
      l2 += fabs(ep1_0->GetPosition()-ep1_1->GetPosition());
      *gline2 = *gline4;
      gline2->AddRouteInterval(rid1,ep1_0->GetPosition(),ep1_1->GetPosition());
  }else{
    if(j2 == 0){ //j2 not junction point
      if(j3 == 0){
        assert(j4 != 0);
        pNetwork->FindSP(j1,j4,l2,gline2);
        l1 = l2;
        l1 += fabs(ep2_1->GetPosition()-ep2_0->GetPosition());
        *gline1 = *gline2;
      gline1->AddRouteInterval(rid2,ep2_1->GetPosition(),ep2_0->GetPosition());
      }else{
        pNetwork->FindSP(j1,j3,l1,gline1);
        if(j4 == 0){
          l2 = l1;
          l2 += fabs(ep2_0->GetPosition()-ep2_1->GetPosition());
          *gline2 = *gline1;
       gline2->AddRouteInterval(rid2,ep2_0->GetPosition(),ep2_1->GetPosition());
        }else //j3 !=0 j4 != 0
          pNetwork->FindSP(j1,j4,l2,gline2);
      }
    l3 = l1;
     l3 += fabs(ep1_1->GetPosition()-ep1_0->GetPosition());
     *gline3 = *gline1;
     gline3->AddRouteInterval(rid1,ep1_1->GetPosition(),ep1_0->GetPosition());
      l4 = l2;
      l4 += fabs(ep1_1->GetPosition()-ep1_0->GetPosition());
      *gline4 = *gline2;
      gline4->AddRouteInterval(rid1,ep1_1->GetPosition(),ep1_0->GetPosition());
    }else{
      if(j3 == 0){
        assert(j4 != 0);
        pNetwork->FindSP(j1,j4,l2,gline2);
        pNetwork->FindSP(j2,j4,l4,gline4);
        if(j1 == j3)
          gline1->SetNetworkId(gline2->GetNetworkId());
        else{
          l1 = l2;
          l1 += fabs(ep2_1->GetPosition()-ep2_0->GetPosition());
          *gline1 = *gline2;
      gline1->AddRouteInterval(rid2,ep2_1->GetPosition(),ep2_0->GetPosition());
        }
        l3 = l4;
          l3 += fabs(ep2_1->GetPosition()-ep2_0->GetPosition());
          *gline3 = *gline4;
       gline3->AddRouteInterval(rid2,ep2_1->GetPosition(),ep2_0->GetPosition());
      }else{
        pNetwork->FindSP(j1,j3,l1,gline1);
        pNetwork->FindSP(j2,j3,l3,gline3);
        if(j4 == 0){
          if(j1 == j4)
            gline2->SetNetworkId(gline1->GetNetworkId());
          else{
          l2 = l1;
          l2 += fabs(ep2_0->GetPosition()-ep2_1->GetPosition());
          *gline2 = *gline1;
       gline2->AddRouteInterval(rid2,ep2_0->GetPosition(),ep2_1->GetPosition());
          }
          l4 = l3;
            l4 += fabs(ep2_0->GetPosition()-ep2_1->GetPosition());
            *gline4 = *gline3;
       gline4->AddRouteInterval(rid2,ep2_0->GetPosition(),ep2_1->GetPosition());
        }else{
          pNetwork->FindSP(j1,j4,l2,gline2);
          pNetwork->FindSP(j2,j4,l4,gline4);
        }
      }
    }
  }
  cout<<l1<<" "<<l2<<" "<<l3<<" "<<l4<<endl;
  GLine* gl1 = new GLine(0);
  GLine* gl2 = new GLine(0);
  GLine* gl3 = new GLine(0);
  GLine* gl4 = new GLine(0);
  double dist1 = ep1_0->NewNetdistance(ep2_0,gl1);//ignore side info
  double dist2 = ep1_0->NewNetdistance(ep2_1,gl2);
  double dist3 = ep1_1->NewNetdistance(ep2_0,gl3);
  double dist4 = ep1_1->NewNetdistance(ep2_1,gl4);
  ep1_0->Print(cout);
  ep1_1->Print(cout);
  ep2_0->Print(cout);
  ep2_1->Print(cout);
  gl1->Print(cout);
  gl2->Print(cout);
  gl3->Print(cout);
  gl4->Print(cout);
  cout<<dist1<<" "<<dist2<<" "<<dist3<<" "<<dist4<<endl;
  double v1 = ug1->p0.Netdistance(&ug1->p1)/
        (ug1->timeInterval.end-ug1->timeInterval.start).ToDouble();
  double v2 = ug2->p0.Netdistance(&ug2->p1)/
        (ug2->timeInterval.end-ug2->timeInterval.start).ToDouble();
  vector<double> c;
  vector<double> b;
  if(sec1->GetTupleId() == sec2->GetTupleId()){ // in the same section
    double dist = fabs(ug1->p0.GetPosition()-ug2->p0.GetPosition());
    c.push_back(dist);
    if(ug1->p0.GetSide() == 0){
      if(ug2->p0.GetSide() == 0){ //ug1 down ug2 down
        if(ug1->p0.GetPosition() < ug2->p0.GetPosition())
          b.push_back(v1-v2);
        else
          b.push_back(v2-v1);
      }else{//ug1 down ug2 up
        if(ug1->p0.GetPosition() < ug2->p0.GetPosition())
          b.push_back(v1+v2);
        else
          b.push_back(-(v1+v2));
      }
    }else{
       if(ug2->p0.GetSide() == 0){ //ug1 up ug2 down
        if(ug1->p0.GetPosition() < ug2->p0.GetPosition())
          b.push_back(-(v1+v2));
        else
          b.push_back(v1+v2);
      }else{ //ug1 up ug2 up
        if(ug1->p0.GetPosition() < ug2->p0.GetPosition())
          b.push_back(v2-v1);
        else
          b.push_back(v1-v2);
      }
    }
  }else{ //different section
  if(ug1->p0.GetSide() == 0){ //ug1 Down
    if(ug2->p0.GetSide() == 0){ //ug1 Down ug2 Down
      assert(ep2_1->Inside(gl1) == ep2_1->Inside(gline1));
      if(ep2_1->Inside(gl1)){ //gl1
        if(ep1_1->Inside(gl1)){
          c.push_back(dist1-meas1-meas3);
          b.push_back(v1+v2);
        }
        else{
          c.push_back(meas1+dist1-meas3);
          b.push_back(v2-v1);
        }
        assert(ep2_0->Inside(gl2) == false);
        assert(ep2_0->Inside(gl2) == ep2_0->Inside(gline2));
      }else{
        assert(ep1_1->Inside(gl1) == ep1_1->Inside(gline1));
        if(ep1_1->Inside(gl1)){
          c.push_back(dist1-meas1+meas3);
          b.push_back(v1-v2);
        }
        else{
          c.push_back(meas1+dist1+meas3);
          b.push_back(-(v2+v1));
        }
      }
      assert(ep2_0->Inside(gl2) == ep2_0->Inside(gline2));
      if(ep2_0->Inside(gl2)){//gl2
        if(ep1_1->Inside(gl1)){
          c.push_back(dist2-meas1-meas4);
          b.push_back(v1-v2);
        }else{
          c.push_back(dist2+meas1-meas4);
          b.push_back(-(v2+v1));
        }
      }else{
          if(ep1_1->Inside(gl1)){
            c.push_back(dist2-meas1+meas4);
            b.push_back(v1+v2);
          }
          else{
            c.push_back(meas1+dist2+meas4);
            b.push_back(v2-v1);
          }
        }
      assert(ep2_1->Inside(gl3) == ep2_1->Inside(gline3));
      if(ep2_1->Inside(gl3)){//gl3
        if(ep1_0->Inside(gl3)){
          c.push_back(dist3-meas2-meas3);
          b.push_back(v2-v1);
        }
        else{
          c.push_back(meas2+dist3-meas3);
          b.push_back(v1+v2);
        }
      }else{
        assert(ep1_0->Inside(gl3) == ep1_0->Inside(gline3));
        if(ep1_0->Inside(gl3)){
          c.push_back(dist3-meas2+meas3);
          b.push_back(-(v1+v2));
        }
        else{
          c.push_back(meas2+dist3+meas3);
          b.push_back(v1-v2);
        }
      }
      assert(ep2_0->Inside(gl4) == ep2_0->Inside(gline4));
      if(ep2_0->Inside(gl4)){ //gl4
        if(ep1_0->Inside(gl4)){
          c.push_back(dist4-meas2-meas4);
          b.push_back(-(v2+v1));
        }else{
          c.push_back(dist4+meas2-meas4);
          b.push_back(v1-v2);
        }
      }else{
        if(ep1_0->Inside(gl4)){
            c.push_back(dist4-meas2+meas4);
            b.push_back(v2-v1);
        }
        else{
            c.push_back(meas2+dist4+meas4);
            b.push_back(v1+v2);
        }
      }
    }else{ //case2 ug1 down ug2 Up
      assert(ep2_1->Inside(gl1) == ep2_1->Inside(gline1));
      if(ep2_1->Inside(gl1)){ //gl1
        if(ep1_1->Inside(gl1)){
          c.push_back(dist1-meas1-meas3);
          b.push_back(v1-v2);
        }
        else{
          c.push_back(meas1+dist1-meas3);
          b.push_back(-(v1+v2));
        }
        assert(ep2_0->Inside(gl2) == false);
        assert(ep2_0->Inside(gl2) == ep2_0->Inside(gline2));
      }else{
        assert(ep1_1->Inside(gl1) == ep1_1->Inside(gline1));
        if(ep1_1->Inside(gl1)){
          c.push_back(dist1-meas1+meas3);
          b.push_back(v1+v2);
        }
        else{
          c.push_back(meas1+dist1+meas3);
          b.push_back(v2-v1);
        }
      }
      assert(ep2_0->Inside(gl2) == ep2_0->Inside(gline2));
      if(ep2_0->Inside(gl2)){ //gl2
        if(ep1_1->Inside(gl1)){
          c.push_back(dist2-meas1-meas4);
          b.push_back(v1+v2);
        }else{
          c.push_back(dist2+meas1-meas4);
          b.push_back(v2-v1);
        }
      }else{
          if(ep1_1->Inside(gl1)){
            c.push_back(dist2-meas1+meas4);
            b.push_back(v1-v2);
          }
          else{
            c.push_back(meas1+dist2+meas4);
            b.push_back(-(v1+v2));
          }
        }
      assert(ep2_1->Inside(gl3) == ep2_1->Inside(gline3));
      if(ep2_1->Inside(gl3)){ //gl3
        if(ep1_0->Inside(gl3)){
          c.push_back(dist3-meas2-meas3);
          b.push_back(-(v1+v2));
        }
        else{
          c.push_back(meas2+dist3-meas3);
          b.push_back(v1-v2);
        }
      }else{
        assert(ep1_0->Inside(gl3) == ep1_0->Inside(gline3));
        if(ep1_0->Inside(gl3)){
          c.push_back(dist3-meas2+meas3);
          b.push_back(v1-v2);
        }
        else{
          c.push_back(meas2+dist3+meas3);
          b.push_back(v2+v1);
        }
      }
      assert(ep2_0->Inside(gl4) == ep2_0->Inside(gline4));
      if(ep2_0->Inside(gl4)){ //gl4
        if(ep1_0->Inside(gl4)){
          c.push_back(dist4-meas2-meas4);
          b.push_back(v2-v1);
        }else{
          c.push_back(dist4+meas2-meas4);
          b.push_back(v1+v2);
        }
      }else{
        if(ep1_0->Inside(gl4)){
            c.push_back(dist4-meas2+meas4);
            b.push_back(-(v1+v2));
        }
        else{
            c.push_back(meas2+dist4+meas4);
            b.push_back(v1-v2);
        }
      }
    }
  }else{ //ug1 Up
    if(ug2->p0.GetSide() == 0){ //case3 ug1 Up ug2 Down
      assert(ep2_1->Inside(gl1) == ep2_1->Inside(gline1));
      if(ep2_1->Inside(gl1)){ //gl1
        if(ep1_1->Inside(gl1)){
          c.push_back(dist1-meas1-meas3);
          b.push_back(v2-v1);
        }
        else{
          c.push_back(meas1+dist1-meas3);
          b.push_back(v1+v2);
        }
        assert(ep2_0->Inside(gl2) == false);
        assert(ep2_0->Inside(gl2) == ep2_0->Inside(gline2));
      }else{
        assert(ep1_1->Inside(gl1) == ep1_1->Inside(gline1));
        if(ep1_1->Inside(gl1)){
          c.push_back(dist1-meas1+meas3);
          b.push_back(-(v2+v1));
        }
        else{
          c.push_back(meas1+dist1+meas3);
          b.push_back(v1-v2);
        }
      }
      assert(ep2_0->Inside(gl2) == ep2_0->Inside(gline2));
      if(ep2_0->Inside(gl2)){ //gl2
        if(ep1_1->Inside(gl2)){
          c.push_back(dist2-meas1-meas4);
          b.push_back(-(v2+v1));
        }else{
          c.push_back(dist2+meas1-meas4);
          b.push_back(v1-v2);
        }
      }else{
          if(ep1_1->Inside(gl2)){
            c.push_back(dist2-meas1+meas4);
            b.push_back(v2-v1);
          }
          else{
            c.push_back(meas1+dist2+meas4);
            b.push_back(v2+v1);
          }
        }
      assert(ep2_1->Inside(gl3) == ep2_1->Inside(gline3));
      if(ep2_1->Inside(gl3)){ //gl3
        if(ep1_0->Inside(gl3)){
          c.push_back(dist3-meas2-meas3);
          b.push_back(v1+v2);
        }
        else{
          c.push_back(meas2+dist3-meas3);
          b.push_back(v2-v1);
        }
      }else{
        assert(ep1_0->Inside(gl3) == ep1_0->Inside(gline3));
        if(ep1_0->Inside(gl3)){//gl3
          c.push_back(dist3-meas2+meas3);
          b.push_back(v1-v2);
        }
        else{
          c.push_back(meas2+dist3+meas3);
          b.push_back(-(v2+v1));
        }
      }
      assert(ep2_0->Inside(gl4) == ep2_0->Inside(gline4));
      if(ep2_0->Inside(gl4)){//gl4
        if(ep1_0->Inside(gl4)){
          c.push_back(dist4-meas2-meas4);
          b.push_back(v1-v2);
        }else{
          c.push_back(dist4+meas2-meas4);
          b.push_back(-(v2+v1));
        }
      }else{
        if(ep1_0->Inside(gl4)){
          c.push_back(dist4-meas2+meas4);
          b.push_back(v1+v2);
        }
        else{
          c.push_back(meas2+dist4+meas4);
          b.push_back(v2-v1);
        }
      }
    }else{//case4 ug1 Up ug2 Up
      assert(ep2_1->Inside(gl1) == ep2_1->Inside(gline1));
      if(ep2_1->Inside(gl1)){//gl1
        if(ep1_1->Inside(gl1)){
          c.push_back(dist1-meas1-meas3);
          b.push_back(-(v1+v2));
        }
        else{
          c.push_back(meas1+dist1-meas3);
          b.push_back(v1-v2);
        }
        assert(ep2_0->Inside(gl2) == false);
        assert(ep2_0->Inside(gl2) == ep2_0->Inside(gline2));
      }else{
        assert(ep1_1->Inside(gl1) == ep1_1->Inside(gline1));
        if(ep1_1->Inside(gl1)){
          c.push_back(dist1-meas1+meas3);
          b.push_back(v2-v1);
        }
        else{
          c.push_back(meas1+dist1+meas3);
          b.push_back(v1+v2);
        }
      }
      assert(ep2_0->Inside(gl2) == ep2_0->Inside(gline2));
      if(ep2_0->Inside(gl2)){ //gl2
        if(ep1_1->Inside(gl2)){
          c.push_back(dist2-meas1-meas4);
          b.push_back(v2-v1);
        }else{
          c.push_back(dist2+meas1-meas4);
          b.push_back(v1+v2);
        }
      }else{
          if(ep1_1->Inside(gl2)){
            c.push_back(dist2-meas1+meas4);
            b.push_back(-(v1+v2));
          }
          else{
            c.push_back(meas1+dist2+meas4);
            b.push_back(v1-v2);
          }
        }
      assert(ep2_1->Inside(gl3) == ep2_1->Inside(gline3));
      if(ep2_1->Inside(gl3)){ //gl3
        if(ep1_0->Inside(gl3)){
          c.push_back(dist3-meas2-meas3);
          b.push_back(v1-v2);
        }
        else{
          c.push_back(meas2+dist3-meas3);
          b.push_back(-(v1+v2));
        }
      }else{
        assert(ep1_0->Inside(gl3) == ep1_0->Inside(gline3));
        if(ep1_0->Inside(gl3)){//gl3
          c.push_back(dist3-meas2+meas3);
          b.push_back(v1+v2);
        }
        else{
          c.push_back(meas2+dist3+meas3);
          b.push_back(v2-v1);
        }
      }
      assert(ep2_0->Inside(gl4) == ep2_0->Inside(gline4));
      if(ep2_0->Inside(gl4)){//gl4
        if(ep1_0->Inside(gl4)){
          c.push_back(dist4-meas2-meas4);
          b.push_back(v1+v2);
        }else{
          c.push_back(dist4+meas2-meas4);
          b.push_back(v2-v1);
        }
      }else{
        if(ep1_0->Inside(gl4)){
          c.push_back(dist4-meas2+meas4);
          b.push_back(v1-v2);
        }
        else{
          c.push_back(meas2+dist4+meas4);
          b.push_back(-(v1+v2));
        }
      }
     }
    }
  }
  //find the split points
  vector<double> split_time;
  for(unsigned int i = 0;i < c.size();i++){
    for(unsigned int j = i + 1;j< c.size();j++){
      double cc = c[j] - c[i];
      double bb = b[i] - b[j];
      if(!AlmostEqual(bb,0)){
        if(!AlmostEqual(cc/bb,0) && cc/bb > 0.0){
          Instant time(instanttype);
          time.ReadFrom(cc/bb);
          if(time > ug1->timeInterval.start && time < ug1->timeInterval.end )
            split_time.push_back(cc/bb);
        }
      }
    }
  }
  sort(split_time.begin(),split_time.end());
  vector<double>::iterator end = unique(split_time.begin(),split_time.end());
  vector<double>::iterator begin = split_time.begin();
  Instant time(instanttype);

  double d = c[0]; //store distance
  vector<double> result_time;
  vector<double> result_dist;
  for(unsigned int i = 1;i < c.size();i ++){
    if(c[i] < d)
      d = c[i];
  }
  result_time.push_back(0); //start time;
  result_dist.push_back(d); //start distance

  for(;begin != end;begin++){
    time.ReadFrom(*begin);
    double dt = (time - ug1->timeInterval.start).ToDouble();
    d = b[0]*dt+c[0];
    for(unsigned int i = 1;i < c.size();i++){
      if(b[i]*dt+c[i] < d)
        d = b[i]*dt+c[i];
    }
    result_time.push_back(dt);
    result_dist.push_back(d);
  }
  double dt = (ug1->timeInterval.end-ug1->timeInterval.start).ToDouble();
  d = b[0]*dt+c[0];//dist
  for(unsigned int i = 1;i < c.size();i ++){
    if(b[i]*dt+c[i] < d)
      d = b[i]*dt+c[i];
  }
  result_time.push_back((ug1->timeInterval.end-
                        ug1->timeInterval.start).ToDouble());//end time;
  result_dist.push_back(d);//end distance
  for(unsigned int i = 0;i < result_time.size() - 1;i++){
    UReal* ureal = new UReal(true);
    time.ReadFrom(result_time[i]+ug1->timeInterval.start.ToDouble());
    ureal->timeInterval.start = time;
    time.ReadFrom(result_time[i+1]+ug1->timeInterval.start.ToDouble());
    ureal->timeInterval.end = time;
    double b = (result_dist[i+1]-result_dist[i])/
                (result_time[i+1]-result_time[i]);
    double c = result_dist[i]-b*result_time[i];
    ureal->a = pow(b,2);
    ureal->b = 2*b*c;
    ureal->c = pow(c,2);
    ureal->r = true;
    dist.push_back(*ureal);
    ureal->DeleteIfAllowed();
  }
  gl1->DeleteIfAllowed();
  gl2->DeleteIfAllowed();
  gl3->DeleteIfAllowed();
  gl4->DeleteIfAllowed();
  ep1_0->DeleteIfAllowed();
  ep1_1->DeleteIfAllowed();
  ep2_0->DeleteIfAllowed();
  ep2_1->DeleteIfAllowed();
  gline1->DeleteIfAllowed();
  gline2->DeleteIfAllowed();
  gline3->DeleteIfAllowed();
  gline4->DeleteIfAllowed();
  sec1->DeleteIfAllowed();
  sec2->DeleteIfAllowed();
}


/*

let p0 and p1 of the UGPoint in the same section

*/
void MGPoint::DivideUGPoint(Network* pNetwork)
{
  MGPoint* mgp = new MGPoint(0);
  mgp->StartBulkLoad();
  UGPoint ug1;
  UGPoint* ug;
  for(int i = 0;i < GetNoComponents();i++){
    Get(i,ug1);
    ug = &ug1;
    Tuple* tuple1 = pNetwork->GetSectionOnRoute(&ug->p0);
    Tuple* tuple2 = pNetwork->GetSectionOnRoute(&ug->p1);
    assert(tuple1 != NULL);
    assert(tuple2 != NULL);
    if(tuple1->GetTupleId() == tuple2->GetTupleId()){
      mgp->Add(*ug);
      continue;
    }
    /*
    assume that there is at most one junction point between p0 and p1
    this is very important because more junction points make it much complex
    to split the UGPoint
    */
    double dt = (ug->timeInterval.end-ug->timeInterval.start).ToDouble();

    if(AlmostEqual(dt,0)){
      continue;
    }
    double length = ug->p0.Netdistance(&ug->p1);
    GPoint* endp1;
    assert(ug->p0.GetSide() != None);  //None is not allowed
    double pos1 = 0.0;
    if(ug->p0.GetSide() == Down)
      pos1 = ((CcReal*)tuple1->GetAttribute(SECTION_MEAS1))->GetRealval();
    if(ug->p0.GetSide() == Up)
      pos1 = ((CcReal*)tuple1->GetAttribute(SECTION_MEAS2))->GetRealval();
    endp1 = new GPoint(true,GetNetworkId(),ug->p0.GetRouteId(),
                      pos1,ug->p0.GetSide());


    double dist1 = fabs(pos1-ug->p0.GetPosition());
    UGPoint* u1 = new UGPoint(ug1);
    Instant middle( (u1->timeInterval.start.ToDouble() +
                  (ug->timeInterval.end.ToDouble() -
                  ug->timeInterval.start.ToDouble())*dist1/length));
    GPoint gp;
    u1->TemporalFunction(middle,gp,true);
    assert(gp.IsDefined());
    u1->timeInterval.end = middle;
    u1->timeInterval.rc = false;
    u1->p1 = gp;
    mgp->Add(*u1);
    u1->DeleteIfAllowed();

    GPoint* endp2;
    double pos2 = 0.0;
    if(ug->p1.GetSide() == Down)
      pos2 = ((CcReal*)tuple2->GetAttribute(SECTION_MEAS2))->GetRealval();
    if(ug->p1.GetSide() == Up)
      pos2 = ((CcReal*)tuple2->GetAttribute(SECTION_MEAS1))->GetRealval();
    endp2 = new GPoint(true,GetNetworkId(),ug->p1.GetRouteId(),
                       pos2,ug->p0.GetSide());

    //double dist2 = fabs(pos2-ug->p1.GetPosition());
    UGPoint* u2 = new UGPoint(ug1);
    u2->TemporalFunction(middle,gp,true);
    assert(gp.IsDefined());
    u2->timeInterval.start = middle;
    u2->p0 = gp;
    mgp->Add(*u2);

    u2->DeleteIfAllowed();
    endp1->DeleteIfAllowed();
    endp2->DeleteIfAllowed();
  }
  mgp->EndBulkLoad(true);
//  *this = *mgp;
  this->CopyFrom(mgp);
  mgp->DeleteIfAllowed();
}


void MGPoint::DistanceN(MGPoint* mgp, MReal* result){

  //Network* pNetwork = NetworkManager::GetNetwork(mgp->GetNetworkId());
  Network* pNetwork = GetNetwork();
  DivideUGPoint(pNetwork);  //partition ugpoints
  mgp->DivideUGPoint(pNetwork);

  UReal uReal(true);
  result->StartBulkLoad();
  UGPoint ugp1;
  UGPoint ugp2;
  UGPoint* ug1 = new UGPoint(true);
  UGPoint* ug2 = new UGPoint(true);
  Instant start;
  Instant end;
  Get(0,ugp1);
  mgp->Get(0,ugp2);

  if(ugp1.timeInterval.start < ugp2.timeInterval.start)
    start = ugp2.timeInterval.start;
  else
    start = ugp1.timeInterval.start;
  Get(GetNoComponents() - 1,ugp1);
  mgp->Get(mgp->GetNoComponents() - 1 ,ugp2);
  if(ugp1.timeInterval.end < ugp2.timeInterval.end)
    end = ugp1.timeInterval.end;
  else
    end = ugp2.timeInterval.end;
  int pos1 = 0,pos2 = 0;
  uReal.timeInterval.start = start;
  Get(pos1,ugp1);
  mgp->Get(pos2,ugp2);
  *ug1 = ugp1;
  *ug2 = ugp2;

  while(1){

    if(ug2->timeInterval.end < ug1->timeInterval.start){
      pos2++;
      mgp->Get(pos2,ugp2);
      *ug2 = ugp2;
      continue;
    }
    if(ug2->timeInterval.start > ug1->timeInterval.end){
      pos1++;
      Get(pos1,ugp1);
      *ug1 = ugp1;
      continue;
    }

    //starttime
    if(ug1->timeInterval.start < uReal.timeInterval.start){ //time split
      GPoint gp0;
      ug1->TemporalFunction(uReal.timeInterval.start,gp0,true);
      assert(gp0.IsDefined());
      ug1->timeInterval.start = uReal.timeInterval.start;
      ug1->p0 = gp0;
    }
    if(ug2->timeInterval.start < uReal.timeInterval.start){ //time split
      GPoint gp0;
      ug2->TemporalFunction(uReal.timeInterval.start,gp0,true);
      assert(gp0.IsDefined());
      ug2->timeInterval.start = uReal.timeInterval.start;
      ug2->p0 = gp0;
    }
    //endtime
    if(ug1->timeInterval.end < ug2->timeInterval.end){ //ug1.end < ug2.end
      uReal.timeInterval.end = ug1->timeInterval.end;
      GPoint gp1;
      ug2->TemporalFunction(uReal.timeInterval.end,gp1,true);
      assert(gp1.IsDefined());
      ug2->timeInterval.end = uReal.timeInterval.end;
      ug2->p1 = gp1;

      double dt = (uReal.timeInterval.end-uReal.timeInterval.start).ToDouble();
      if(AlmostEqual(dt,0)){
        uReal.a = 0.0;
        uReal.b = 0.0;
        uReal.c = 0;
        uReal.r = false;
//        result->MergeAdd(uReal);
      }else{
        vector<UReal> df;//distance function
        DistanceFunction(ug1,ug2,pNetwork,df);
        for(unsigned int i = 0;i < df.size();i++){
          uReal.timeInterval = df[i].timeInterval;
          if(uReal.timeInterval.end == end)
            uReal.timeInterval.rc = true;
          else
            uReal.timeInterval.rc = false;
          uReal.a = df[i].a;
          uReal.b = df[i].b;
          uReal.c = df[i].c;
          uReal.r = true;

          result->MergeAdd(uReal);
        }
      }
      uReal.r = false;
      uReal.timeInterval.start = uReal.timeInterval.end;
      pos1++;
      if(pos1 == GetNoComponents()){
        break;
      }
      Get(pos1,ugp1);
      *ug1 = ugp1;
      mgp->Get(pos2,ugp2);
      *ug2 = ugp2;
      GPoint gp0;
      ug2->TemporalFunction(uReal.timeInterval.start,gp0,true);
      assert(gp0.IsDefined());
      ug2->timeInterval.start = uReal.timeInterval.start;
      ug2->p0 = gp0;

    }else{ //ugp1.end > ugp2.end
      uReal.timeInterval.end = ug2->timeInterval.end;
      GPoint gp1;
      ug1->TemporalFunction(uReal.timeInterval.end,gp1,true);
      assert(gp1.IsDefined());
      ug1->timeInterval.end = uReal.timeInterval.end;
      ug1->p1 = gp1;

      double dt = (uReal.timeInterval.end-uReal.timeInterval.start).ToDouble();
      if(AlmostEqual(dt,0)){
        uReal.a = 0.0;
        uReal.b = 0.0;
        uReal.c = 0.0;
        uReal.r = false;
//        result->MergeAdd(uReal);
      }else{
        //double v1 = ug1->p0.Distance(&ug1->p1)/dt;
        //double v2 = ug2->p0.Distance(&ug2->p1)/dt;
        vector<UReal> df;//distance function
        DistanceFunction(ug1,ug2,pNetwork,df);
        for(unsigned int i = 0;i < df.size();i++){
          uReal.timeInterval = df[i].timeInterval;
          if(uReal.timeInterval.end == end)
            uReal.timeInterval.rc = true;
          else
            uReal.timeInterval.rc = false;

          uReal.a = df[i].a;
          uReal.b = df[i].b;
          uReal.c = df[i].c;
          uReal.r = true;
          result->MergeAdd(uReal);
        }
      }
      uReal.r = false;
      uReal.timeInterval.start = uReal.timeInterval.end;
      pos2++;
      if(pos2 == mgp->GetNoComponents()){
        break;
      }
      mgp->Get(pos2,ugp2);
      *ug2 = ugp2;
      Get(pos1,ugp1);
      *ug1 = ugp1;
      GPoint gp0;
      ug1->TemporalFunction(uReal.timeInterval.start,gp0,true);
      assert(gp0.IsDefined());
      ug1->timeInterval.start = uReal.timeInterval.start;
      ug1->p0 = gp0;
    }
  }
  ug1->DeleteIfAllowed();
  ug2->DeleteIfAllowed();
  NetworkManager::CloseNetwork(pNetwork);
  result->EndBulkLoad();
}

/*
Translation from network ~mgpoint~ to spatial ~mpoint~

*/

void MGPoint::Mgpoint2mpoint(MPoint *&mp) {
  if (IsDefined() && !IsEmpty()){
  //Network* pNetwork = NetworkManager::GetNetwork(GetNetworkId());
  Network* pNetwork = GetNetwork();
  UGPoint pCurrUnit;
  UGPoint CurrUnit;
  int iAktRouteId = 1;
  int lrsposakt = 0;
  int lrsposnext = 0;
  Tuple *pRoute = pNetwork->GetRoute(iAktRouteId);
  SimpleLine *pRouteCurve = (SimpleLine*) pRoute->GetAttribute(ROUTE_CURVE);
  Instant correcture(0,1,durationtype);
  bool bendcorrecture = false;
  Instant tStart, tEnd, tInter1, tInter2, tEndNew;
  Point pStart(false);
  Point pEnd(false);
  LRS lrsAkt, lrsNext;
  HalfSegment hs;
  mp->Clear();
  mp->StartBulkLoad();
  for (int i = 0 ; i < GetNoComponents(); i++) {
    Get(i, pCurrUnit);
    CurrUnit = pCurrUnit;
    tStart = pCurrUnit.timeInterval.start;
    tEnd = pCurrUnit.timeInterval.end;
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
    if (pCurrUnit.p0.GetRouteId() != iAktRouteId || i == 0) {
      pRoute->DeleteIfAllowed();
      iAktRouteId = pCurrUnit.p0.GetRouteId();
      pRoute = pNetwork->GetRoute(iAktRouteId);
      pRouteCurve = (SimpleLine*) pRoute->GetAttribute(ROUTE_CURVE);
      LRS lrs(pCurrUnit.p0.GetPosition(),0);
      if (!pRouteCurve->Get(lrs, lrsposakt)) {
        break;
      }
      pRouteCurve->Get( lrsposakt, lrsAkt );
      pRouteCurve->Get( lrsAkt.hsPos, hs );
      pStart = hs.AtPosition(pCurrUnit.p0.GetPosition() - lrsAkt.lrsPos);
    }
    if (pCurrUnit.p0.GetPosition() == pCurrUnit.p1.GetPosition()){
       mp->Add(UPoint(Interval<Instant> (tStart, tEnd,
              pCurrUnit.timeInterval.lc, pCurrUnit.timeInterval.rc),
              pStart, pStart));
    } else {
      if (pCurrUnit.p0.GetPosition() < pCurrUnit.p1.GetPosition()){
        tInter1 = tStart;
        lrsposnext = lrsposakt + 1;
        if (lrsposnext < (pRouteCurve->Size()/2)){
          pRouteCurve->Get(lrsposnext, lrsNext);
          if (lrsNext.lrsPos >= pCurrUnit.p1.GetPosition()) {
            pEnd = hs.AtPosition(pCurrUnit.p1.GetPosition() - lrsAkt.lrsPos);
            mp->Add(UPoint(Interval<Instant> (tInter1, tEnd,
                    pCurrUnit.timeInterval.lc, pCurrUnit.timeInterval.rc),
                    pStart, pEnd));
            pStart = pEnd;
          } else {
            while (lrsNext.lrsPos <= pCurrUnit.p1.GetPosition() &&
                   lrsposnext < (pRouteCurve->Size()/2)){
              tInter2 = CurrUnit.TimeAtPos(lrsNext.lrsPos);
              pRouteCurve->Get(lrsNext.hsPos, hs);
              pEnd = hs.AtPosition(0);
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
              if (lrsposnext < (pRouteCurve->Size()/2))
                pRouteCurve->Get(lrsposnext, lrsNext);
            }
            pEnd = hs.AtPosition(pCurrUnit.p1.GetPosition() - lrsAkt.lrsPos);
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
          pEnd = hs.AtPosition(pCurrUnit.p1.GetPosition() - lrsAkt.lrsPos);
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
        if (lrsAkt.lrsPos <= pCurrUnit.p1.GetPosition()) {
            pEnd = hs.AtPosition(pCurrUnit.p1.GetPosition() - lrsAkt.lrsPos);
            mp->Add(UPoint(Interval<Instant> (tStart, tEnd,
                    pCurrUnit.timeInterval.lc, pCurrUnit.timeInterval.rc),
                    pStart, pEnd));
            pStart = pEnd;
        } else {
          tInter1 = tStart;
          while (lrsAkt.lrsPos > pCurrUnit.p1.GetPosition() &&
                lrsposakt >= 0){
            pEnd = hs.AtPosition(0);
            tInter2 = CurrUnit.TimeAtPos(lrsAkt.lrsPos);
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
              pRouteCurve->Get(lrsAkt.hsPos, hs);
            }
          }
          pEnd = hs.AtPosition(pCurrUnit.p1.GetPosition() - lrsAkt.lrsPos);
          if (tInter1 >= tEnd) {
            tEnd = tInter1 + correcture;
            bendcorrecture = true;
            tEndNew = tEnd;
          }
          mp->Add(UPoint(Interval<Instant> (tInter1, tEnd,
                    pCurrUnit.timeInterval.lc, pCurrUnit.timeInterval.rc),
                    pStart, pEnd));
          pStart = pEnd;
        }
      }
    }
  }
  mp->EndBulkLoad();
  pRoute->DeleteIfAllowed();
  NetworkManager::CloseNetwork(pNetwork);
  }
  else mp->SetDefined(false);
}

/*
Checks if ~mgpoint~ is present in the given ~periods~

*/

bool MGPoint::Present(Periods *&per) {
  if (!IsDefined() || !per->IsDefined() || IsEmpty() || per->IsEmpty())
    return false;
  Interval<Instant> intper;
  UGPoint pCurrUnit;
  int j = 0;
  int mid, first, last;
  while (j < per->GetNoComponents()) {
    per->Get( j, intper );
    Get(0, pCurrUnit);
    if(!intper.Before(pCurrUnit.timeInterval)){
      Get(GetNoComponents()-1, pCurrUnit);
      if (!pCurrUnit.timeInterval.Before(intper)){
        first = 0;
        last = GetNoComponents()-1;
        while (first <= last) {
          mid = (first + last) /2;
          if (mid < 0 || mid >= GetNoComponents()) break;
          Get( mid, pCurrUnit );
          if (pCurrUnit.timeInterval.Before(intper)) first = mid + 1;
          else {
            if (intper.Before(pCurrUnit.timeInterval)) last = mid - 1;
            else return true;
          }
        }
      }
    }
    j++;
  }
  return false;
}

/*
Searches binary the unit id of the mpgoint unit which includes the given time
stamp. Returns -1 if the time stamp is not found.

*/
int MGPoint::Position(const Instant &ins, bool atinst /*=true*/) {
  if(GetNoComponents() <= 0) return -1;
  int mid = -1;
  int first = 0;
  int last = GetNoComponents() - 1;
  UGPoint pCurrUnit;
  Get(first, pCurrUnit);
  if (pCurrUnit.timeInterval.start > ins) return -1;
  Get(last, pCurrUnit);
  if (pCurrUnit.timeInterval.end < ins) return -1;
  while (first <= last) {
    mid = (first+last)/2;
    if (mid<0 || mid >= GetNoComponents()) return -1;
    Get(mid, pCurrUnit);
    if (pCurrUnit.timeInterval.end < ins) first = mid + 1;
    else
      if (pCurrUnit.timeInterval.start > ins ) last = mid - 1;
      else return mid;
  }
/*
If we try to find a start position of a period inside a mgpoint it must not
be the exact position it reaches to find the position nearest before the start
of the periods start to catch the units inside the periods.

*/
  if (atinst) return -1;
  else return mid-1;
}


bool MGPoint::Present(Instant *&per) {
  if (!IsDefined() || IsEmpty()) return false;
  int pos = Position(*per);
  if (pos == -1) return false;
  else return true;
}

/*
Computes the intersection of two ~mgpoint~

*/
void MGPoint::Intersection(MGPoint *&mgp, MGPoint *&res){
//   DbArray<RouteInterval> tra1 = GetTrajectory();
//   DbArray<RouteInterval> tra2 = mgp->GetTrajectory();
//   if (Intersects(tra1,tra2,true, true)){
    UGPoint pCurr1, pCurr2;
    Get(0, pCurr1);
    mgp->Get(0, pCurr2);
    //Network *pNetwork = NetworkManager::GetNetwork(pCurr1.p0.GetNetworkId());
    Network* pNetwork = GetNetwork();
    if (!pNetwork->IsDefined() || pNetwork == NULL) {
      cerr << "Network does not exist."<< endl;
      res->SetDefined(false);
      NetworkManager::CloseNetwork(pNetwork);
    } else {
      if (pCurr1.p0.GetNetworkId() != pCurr2.p0.GetNetworkId()) {
        cerr <<"mgpoints belong to different networks." << endl;
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
            res->SetDefined(false);
            NetworkManager::CloseNetwork(pNetwork);
          } else {
            res->StartBulkLoad();
            double interPosition;
            Instant tinter, tinter2, tlast;
            for (int i = 0; i < resA->GetNoComponents() ; i++) {
              resA->Get(i,pCurr1);
              resB->Get(i,pCurr2);
              if (pCurr1.p0.GetRouteId() == pCurr2.p0.GetRouteId()) {
                if (pCurr1.p0.GetPosition() == pCurr2.p0.GetPosition() &&
                    pCurr1.p1.GetPosition() == pCurr2.p1.GetPosition()) {
                  res->Add(pCurr1/*, pNetwork*/);
                } else {
                  tinter =
                      (pCurr1.timeInterval.end - pCurr1.timeInterval.start)
                      * ((pCurr2.p0.GetPosition() - pCurr1.p0.GetPosition())/
                          (pCurr1.p1.GetPosition() - pCurr1.p0.GetPosition()-
                          pCurr2.p1.GetPosition() + pCurr2.p0.GetPosition()))+
                          pCurr1.timeInterval.start;
                  if (pCurr1.timeInterval.start <= tinter &&
                      tinter <= pCurr1.timeInterval.end)
                  {
                    interPosition = pCurr1.p0.GetPosition() +
                        (((pCurr1.p1.GetPosition() - pCurr1.p0.GetPosition())*
                      (tinter.ToDouble()-pCurr1.timeInterval.start.ToDouble())
                              / (pCurr1.timeInterval.end.ToDouble() -
                              pCurr1.timeInterval.start.ToDouble())));
                    bool ok = true;
                    if (pCurr1.p0.GetPosition()!= pCurr1.p1.GetPosition())
                    {
                      if (fabs(interPosition - pCurr1.p0.GetPosition()) < 0.01)
                        ok = ok && pCurr1.timeInterval.lc;
                      if (fabs(interPosition - pCurr1.p1.GetPosition()) < 0.01)
                        ok = ok && pCurr1.timeInterval.rc;
                    }
                    else
                      if (fabs(interPosition - pCurr1.p0.GetPosition()) < 0.01)
                        ok = true;
                      else ok = false;
                    if(pCurr2.p0.GetPosition() != pCurr2.p1.GetPosition())
                    {
                      if (fabs(interPosition - pCurr2.p0.GetPosition()) < 0.01)
                        ok = ok && pCurr2.timeInterval.lc;
                      if (fabs(interPosition - pCurr2.p1.GetPosition()) < 0.01)
                        ok = ok && pCurr2.timeInterval.rc;
                    }
                    else
                      if (fabs(interPosition-pCurr2.p0.GetPosition()) < 0.01)
                        ok = true;
                      else ok = false;
                    if (ok)
                    {
                      res->Add(UGPoint(Interval<Instant> (tinter, tinter,
                                true, true),
                                pCurr1.p0.GetNetworkId(),
                                pCurr1.p0.GetRouteId(),
                                pCurr1.p0.GetSide(),
                                interPosition,
                                interPosition));
                    }
                  }
                }
              }
              else
              {
                CcInt *pRid1 = new CcInt(true,pCurr1.p0.GetRouteId());
                CcInt *pRid2 = new CcInt(true,pCurr2.p0.GetRouteId());
                double r1meas = numeric_limits<double>::max();
                double r2meas = numeric_limits<double>::max();
                pNetwork->GetJunctionMeasForRoutes(pRid1,pRid2,r1meas,r2meas);
                if ((r1meas != numeric_limits<double>::max() &&
                   r2meas != numeric_limits<double>::max())&&
                    (((pCurr1.p0.GetPosition() <= r1meas &&
                    r1meas <= pCurr1.p1.GetPosition()) ||
                      (pCurr1.p0.GetPosition() >= r1meas &&
                       r1meas >= pCurr1.p1.GetPosition())) &&
                        ((pCurr2.p0.GetPosition() <= r2meas &&
                        r2meas <= pCurr2.p1.GetPosition()) ||
                        (pCurr2.p0.GetPosition() >= r2meas &&
                        r2meas >= pCurr2.p1.GetPosition()))))
                {
                  UGPoint pCurr = pCurr1;
                  tinter = pCurr.TimeAtPos(r1meas);
                  pCurr = pCurr2;
                  tinter2 = pCurr.TimeAtPos(r2meas);
                  if (tinter == tinter2) {
                    bool ok = true;
                    if (fabs(r1meas - pCurr1.p0.GetPosition()) < 0.01)
                      ok = ok && pCurr1.timeInterval.lc;
                    if (fabs(r1meas - pCurr1.p1.GetPosition()) < 0.01)
                      ok = ok && pCurr1.timeInterval.rc;
                    if (fabs(r2meas - pCurr2.p0.GetPosition()) < 0.01)
                      ok = ok && pCurr2.timeInterval.lc;
                    if (fabs(r2meas - pCurr2.p1.GetPosition()) < 0.01)
                      ok = ok && pCurr2.timeInterval.rc;
                    if (ok)
                      res->Add(UGPoint(Interval<Instant> (tinter, tinter,
                               true, true),
                               pCurr1.p0.GetNetworkId(),
                               pCurr1.p0.GetRouteId(),
                               pCurr1.p0.GetSide(),
                               r1meas,
                               r1meas));
                  }
                }
              }// end if else same route
            }// end for
            res->EndBulkLoad(true);
            if (res->GetNoComponents() > 0) res->SetDefined(true);
            else res->SetDefined(false);
            NetworkManager::CloseNetwork(pNetwork);
          }
        }
        resA->DeleteIfAllowed();
        resB->DeleteIfAllowed();
      }
    }
//   } else {
//     res->Clear();
//     res->SetDefined(true);
//   }
  res->SetTrajectoryDefined(false);
  res->m_trajectory.TrimToSize();
  res->SetBoundingBoxDefined(false);
}

/*
Returns true if the two mgpoint met at any point of their trips.

*/

bool MGPoint::Intersects(MGPoint *mgp)
{
  UGPoint pCurr1, pCurr2;
  Get(0, pCurr1);
  mgp->Get(0, pCurr2);
  //Network *pNetwork = NetworkManager::GetNetwork(pCurr1.p0.GetNetworkId());
  Network* pNetwork = GetNetwork();
  if (!pNetwork->IsDefined() || pNetwork == NULL)
  {
    cerr << "Network does not exist."<< endl;
    NetworkManager::CloseNetwork(pNetwork);
    return false;
  }
  else
  {
    if (pCurr1.p0.GetNetworkId() != pCurr2.p0.GetNetworkId())
    {
      cerr <<"mgpoints belong to different networks." << endl;
      NetworkManager::CloseNetwork(pNetwork);
      return false;
    }
    else
    {
      MGPoint *resA = new MGPoint(0);
      MGPoint *resB = new MGPoint(0);
      refinementMovingGPoint (this, mgp, resA, resB);
      if (resA == NULL || !resA->IsDefined() ||
          resB == NULL || !resB->IsDefined() ||
          resA->GetNoComponents() != resB->GetNoComponents ())
      {
        NetworkManager::CloseNetwork(pNetwork);
        resA->DeleteIfAllowed();
        resB->DeleteIfAllowed();
        return false;
      }
      else
      {
        if (resA->GetNoComponents() < 1)
        {
          NetworkManager::CloseNetwork(pNetwork);
          resA->DeleteIfAllowed();
          resB->DeleteIfAllowed();
          return false;
        }
        else
        {
          double interPosition;
          Instant tinter, tinter2, tlast;
          for (int i = 0; i < resA->GetNoComponents() ; i++)
          {
            resA->Get(i,pCurr1);
            resB->Get(i,pCurr2);
            if (pCurr1.p0.GetRouteId() == pCurr2.p0.GetRouteId())
            {
              if (pCurr1.p0.GetPosition() == pCurr2.p0.GetPosition() &&
                  pCurr1.p1.GetPosition() == pCurr2.p1.GetPosition())
              {
                NetworkManager::CloseNetwork(pNetwork);
                resA->DeleteIfAllowed();
                resB->DeleteIfAllowed();
                return true;
              }
              else
              {
                tinter =
                  (pCurr1.timeInterval.end - pCurr1.timeInterval.start)
                    * ((pCurr2.p0.GetPosition() - pCurr1.p0.GetPosition())/
                  (pCurr1.p1.GetPosition() - pCurr1.p0.GetPosition()-
                     pCurr2.p1.GetPosition() + pCurr2.p0.GetPosition()))+
                       pCurr1.timeInterval.start;
                if (pCurr1.timeInterval.start <= tinter &&
                    tinter <= pCurr1.timeInterval.end)
                {
                  interPosition = pCurr1.p0.GetPosition() +
                      (((pCurr1.p1.GetPosition() - pCurr1.p0.GetPosition())*
                      (tinter.ToDouble()-pCurr1.timeInterval.start.ToDouble())
                              / (pCurr1.timeInterval.end.ToDouble() -
                              pCurr1.timeInterval.start.ToDouble())));
                  bool ok = true;
                  if (pCurr1.p0.GetPosition()!= pCurr1.p1.GetPosition())
                  {
                    if (fabs(interPosition - pCurr1.p0.GetPosition()) < 0.01)
                      ok = ok && pCurr1.timeInterval.lc;
                    if (fabs(interPosition - pCurr1.p1.GetPosition()) < 0.01)
                      ok = ok && pCurr1.timeInterval.rc;
                  }
                  else
                    if (fabs(interPosition - pCurr1.p0.GetPosition()) < 0.01)
                      ok = true;
                    else ok = false;
                    if(pCurr2.p0.GetPosition() != pCurr2.p1.GetPosition())
                    {
                      if (fabs(interPosition - pCurr2.p0.GetPosition()) < 0.01)
                        ok = ok && pCurr2.timeInterval.lc;
                      if (fabs(interPosition - pCurr2.p1.GetPosition()) < 0.01)
                        ok = ok && pCurr2.timeInterval.rc;
                    }
                    else
                      if (fabs(interPosition-pCurr2.p0.GetPosition()) < 0.01)
                        ok = true;
                      else ok = false;
                    if (ok)
                    {
                      NetworkManager::CloseNetwork(pNetwork);
                      resA->DeleteIfAllowed();
                      resB->DeleteIfAllowed();
                      return true;
                    }
                  }
                }
              }
              else
              {
                CcInt *pRid1 = new CcInt(true,pCurr1.p0.GetRouteId());
                CcInt *pRid2 = new CcInt(true,pCurr2.p0.GetRouteId());
                double r1meas = numeric_limits<double>::max();
                double r2meas = numeric_limits<double>::max();
                pNetwork->GetJunctionMeasForRoutes(pRid1,pRid2,r1meas,r2meas);
                if ((r1meas != numeric_limits<double>::max() &&
                   r2meas != numeric_limits<double>::max())&&
                    (((pCurr1.p0.GetPosition() <= r1meas &&
                    r1meas <= pCurr1.p1.GetPosition()) ||
                      (pCurr1.p0.GetPosition() >= r1meas &&
                       r1meas >= pCurr1.p1.GetPosition())) &&
                        ((pCurr2.p0.GetPosition() <= r2meas &&
                        r2meas <= pCurr2.p1.GetPosition()) ||
                        (pCurr2.p0.GetPosition() >= r2meas &&
                        r2meas >= pCurr2.p1.GetPosition()))))
                {
                  UGPoint pCurr = pCurr1;
                  tinter = pCurr.TimeAtPos(r1meas);
                  pCurr = pCurr2;
                  tinter2 = pCurr.TimeAtPos(r2meas);
                  if (tinter == tinter2)
                  {
                    bool ok = true;
                    if (fabs(r1meas - pCurr1.p0.GetPosition()) < 0.01)
                      ok = ok && pCurr1.timeInterval.lc;
                    if (fabs(r1meas - pCurr1.p1.GetPosition()) < 0.01)
                      ok = ok && pCurr1.timeInterval.rc;
                    if (fabs(r2meas - pCurr2.p0.GetPosition()) < 0.01)
                      ok = ok && pCurr2.timeInterval.lc;
                    if (fabs(r2meas - pCurr2.p1.GetPosition()) < 0.01)
                      ok = ok && pCurr2.timeInterval.rc;
                    if (ok)
                    {
                      NetworkManager::CloseNetwork(pNetwork);
                      resA->DeleteIfAllowed();
                      resB->DeleteIfAllowed();
                      return true;
                    }
                  }
                }
              }// end if else same route
            }// end for
          }
        }
        resA->DeleteIfAllowed();
        resB->DeleteIfAllowed();
      }
      NetworkManager::CloseNetwork(pNetwork);
    }
  return false;
}

/*
Checks if ~mgpoint~ is inside a ~gline~. Returns a ~mbool~ which is true for the
times the ~mgpoint~ is inside the ~gline~ false elsewhere.

*/

void MGPoint::Inside(GLine *&gl, MBool *&res){
//   DbArray<RouteInterval> riarr = GetTrajectory();
//   DbArray<RouteInterval>* riglarr = gl->GetRouteIntervals();
//   if (Intersects(riarr, *riglarr, true, gl->IsSorted())){
    UGPoint pCurrentUnit;
    Get(0, pCurrentUnit);
    if (gl->GetNetworkId() != pCurrentUnit.p0.GetNetworkId()) {
      res->SetDefined(false);
    } else {
      double mgStart, mgEnd, lStart, lEnd;
      RouteInterval pCurrRInter;
      bool swapped, found, bInterStart, bInterEnd;
      Instant tInterStart, tInterEnd;
      UBool interbool(true);
      int iRouteMgp;
      double interStart, interEnd;
      res->StartBulkLoad();
      for (int i = 0; i < GetNoComponents(); i++){
        Get(i, pCurrentUnit);
        UGPoint CurrUnit = pCurrentUnit;
        int j = 0;
        iRouteMgp = pCurrentUnit.p0.GetRouteId();
        swapped = false;
        mgStart = pCurrentUnit.p0.GetPosition();
        mgEnd = pCurrentUnit.p1.GetPosition();
        if (mgEnd < mgStart) {
          mgStart = pCurrentUnit.p1.GetPosition();
          mgEnd = pCurrentUnit.p0.GetPosition();
          swapped = true;
        }
        if (gl->IsSorted()){
          vector<RouteInterval> vRI;
          RouteInterval currRInter;
          vRI.clear();
          getRouteIntervals(gl, iRouteMgp, mgStart, mgEnd, 0,
                            gl->NoOfComponents(), vRI);
          if (vRI.size() > 0) {
            size_t k = 0;
            while (k < vRI.size()) {
              currRInter = vRI[k];
              if (pCurrentUnit.p0.GetPosition() <
                  pCurrentUnit.p1.GetPosition())
              {
                if (pCurrentUnit.p0.GetPosition() >=
                    currRInter.GetStartPos()) {
                  interbool.timeInterval.start =
                      pCurrentUnit.timeInterval.start;
                  interbool.timeInterval.lc = pCurrentUnit.timeInterval.lc;
                  interbool.constValue.Set(true, true);
                } else {
                  interbool.timeInterval.start =
                      pCurrentUnit.timeInterval.start;
                  interbool.timeInterval.lc = pCurrentUnit.timeInterval.lc;
                  tInterStart = CurrUnit.TimeAtPos(currRInter.GetStartPos());
                  interbool.timeInterval.end = tInterStart;
                  interbool.timeInterval.rc = false;
                  interbool.constValue.Set(true,false);
                  res->MergeAdd(interbool);
                  //compute start of unit inside
                  interbool.timeInterval.start = tInterStart;
                  interbool.timeInterval.lc = true;
                  interbool.constValue.Set(true, true);
                }
                if (pCurrentUnit.p1.GetPosition() <= currRInter.GetEndPos()) {
                  interbool.timeInterval.end = pCurrentUnit.timeInterval.end;
                  interbool.timeInterval.rc = pCurrentUnit.timeInterval.rc;
                  res->MergeAdd(interbool);
                } else {
                  // compute end of mgpoint at end of gline.and add result unit
                  interbool.timeInterval.rc = true;
                  tInterEnd = CurrUnit.TimeAtPos(currRInter.GetEndPos());
                  interbool.timeInterval.end = tInterEnd;
                  res->MergeAdd(interbool);
                  // the rest of the current unit is not in the current
                  // routeinterval.
                  checkEndOfUGPoint(currRInter.GetEndPos(),
                                    pCurrentUnit.p1.GetPosition(), tInterEnd,
                                    false, pCurrentUnit.timeInterval.end,
                                    pCurrentUnit.timeInterval.rc, k, vRI,
                                    res, iRouteMgp);
                }
              } else {
                if (currRInter.GetEndPos() >= pCurrentUnit.p0.GetPosition()) {
                  interStart = pCurrentUnit.p0.GetPosition();
                  tInterStart = pCurrentUnit.timeInterval.start;
                  bInterStart = pCurrentUnit.timeInterval.lc;
                } else {
                  interStart = currRInter.GetEndPos();
                  bInterStart = true;
                  tInterStart = CurrUnit.TimeAtPos(interStart);
                }
                if (currRInter.GetStartPos() <=
                    pCurrentUnit.p1.GetPosition()) {
                  interEnd = pCurrentUnit.p1.GetPosition();
                  tInterEnd = pCurrentUnit.timeInterval.end;
                  bInterEnd = pCurrentUnit.timeInterval.rc;
                } else {
                  interEnd = currRInter.GetStartPos();
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
            if (iRouteMgp == pCurrRInter.GetRouteId()){
              mgStart = pCurrentUnit.p0.GetPosition();
              mgEnd = pCurrentUnit.p1.GetPosition();
              if (mgEnd < mgStart) {
                mgStart = pCurrentUnit.p1.GetPosition();
                mgEnd = pCurrentUnit.p0.GetPosition();
                swapped = true;
              }
              lStart = pCurrRInter.GetStartPos();
              lEnd = pCurrRInter.GetEndPos();
              if (lStart > lEnd) {
                lStart = pCurrRInter.GetEndPos();
                lEnd = pCurrRInter.GetStartPos();
              }
              if (!(mgEnd < lStart || mgStart > lEnd || mgStart == mgEnd ||
                  lStart == lEnd)){
                //intersection exists compute intersecting part and timevalues
                //for resulting unit
                found = true;
                if (!swapped) {
                  if (lStart <= mgStart) {
                    interbool.timeInterval.start =
                        pCurrentUnit.timeInterval.start;
                    interbool.timeInterval.lc = pCurrentUnit.timeInterval.lc;
                    interbool.constValue.Set(true, true);
                  } else {
                    // compute and write unit befor mgpoint inside gline
                    interbool.timeInterval.start =
                        pCurrentUnit.timeInterval.start;
                    interbool.timeInterval.lc = pCurrentUnit.timeInterval.lc;
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
                    interbool.timeInterval.end = pCurrentUnit.timeInterval.end;
                    interbool.timeInterval.rc = pCurrentUnit.timeInterval.rc;
                    res->MergeAdd(interbool);
                  } else {
                    // compute end of mgpoint at end of gline.and add resultunit
                    interbool.timeInterval.rc = true;
                    tInterEnd = CurrUnit.TimeAtPos(lEnd);
                    interbool.timeInterval.end = tInterEnd;
                    res->MergeAdd(interbool);
                    // the rest of the current unit is not in the current
                    // routeinterval.
                    checkEndOfUGPoint(lEnd, pCurrentUnit.p1.GetPosition(),
                                      tInterEnd, false,
                                      pCurrentUnit.timeInterval.end,
                                    pCurrentUnit.timeInterval.rc, j, gl,
                                    res, iRouteMgp);
                  }
                } else {
                  mgStart = pCurrentUnit.p0.GetPosition();
                  mgEnd = pCurrentUnit.p1.GetPosition();
                  if (lEnd >= mgStart) {
                    interbool.timeInterval.start =
                        pCurrentUnit.timeInterval.start;
                    interbool.timeInterval.lc = pCurrentUnit.timeInterval.lc;
                    interbool.constValue.Set(true, true);
                  } else {
                    // compute and write unit befor mgpoint inside gline
                    interbool.timeInterval.start =
                        pCurrentUnit.timeInterval.start;
                    interbool.timeInterval.lc = pCurrentUnit.timeInterval.lc;
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
                    interbool.timeInterval.end = pCurrentUnit.timeInterval.end;
                    interbool.timeInterval.rc = pCurrentUnit.timeInterval.rc;
                    res->MergeAdd(interbool);
                  } else {
                    // compute end of mgpoint at end of gline.and add resultunit
                    interbool.timeInterval.rc = true;
                    tInterEnd = CurrUnit.TimeAtPos(lStart);
                    interbool.timeInterval.end = tInterEnd;
                    res->MergeAdd(interbool);
                    // the rest of the current unit is not in the current
                    // routeinterval.
                    checkEndOfUGPoint(lEnd, pCurrentUnit.p1.GetPosition(),
                                      tInterEnd, false,
                                      pCurrentUnit.timeInterval.end,
                                    pCurrentUnit.timeInterval.rc, j, gl,
                                    res, iRouteMgp);
                  }
                }
              }else{
                if (mgStart == mgEnd && pCurrentUnit.timeInterval.lc &&
                  pCurrentUnit.timeInterval.rc){
                  found = true;
                  interbool.timeInterval.start =
                      pCurrentUnit.timeInterval.start;
                  interbool.timeInterval.lc = pCurrentUnit.timeInterval.lc;
                  interbool.timeInterval.end = pCurrentUnit.timeInterval.end;
                  interbool.timeInterval.rc = pCurrentUnit.timeInterval.rc;
                  interbool.constValue.Set(true,true);
                  res->MergeAdd(interbool);
                } else {
                  if (lStart == lEnd) {
                    if ((lStart > mgStart && lStart < mgEnd) ||
                        (lStart < mgStart && lStart > mgEnd)) {
                      found = true;
                      // compute and write unit befor mgpoint inside gline
                      interbool.timeInterval.start =
                          pCurrentUnit.timeInterval.start;
                      interbool.timeInterval.lc = pCurrentUnit.timeInterval.lc;
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
                      checkEndOfUGPoint(lEnd, pCurrentUnit.p1.GetPosition(),
                                        tInterEnd, false,
                                        pCurrentUnit.timeInterval.end,
                                        pCurrentUnit.timeInterval.rc, j, gl,
                                        res, iRouteMgp);
                    } else {
                      if (lStart == mgStart && pCurrentUnit.timeInterval.lc) {
                        found = true;
                        interbool.timeInterval.start =
                            pCurrentUnit.timeInterval.start;
                        interbool.timeInterval.lc =
                            pCurrentUnit.timeInterval.lc;
                        interbool.timeInterval.end =
                            pCurrentUnit.timeInterval.start;
                        interbool.timeInterval.rc = true;
                        interbool.constValue.Set(true,true);
                        res->MergeAdd(interbool);
                        checkEndOfUGPoint(lEnd, pCurrentUnit.p1.GetPosition(),
                                          tInterEnd, false,
                                          pCurrentUnit.timeInterval.end,
                                          pCurrentUnit.timeInterval.rc, j, gl,
                                          res, iRouteMgp);
                      } else {
                        if (lStart == mgEnd && pCurrentUnit.timeInterval.rc) {
                          found = true;
                          interbool.timeInterval.start =
                            pCurrentUnit.timeInterval.start;
                          interbool.timeInterval.lc =
                              pCurrentUnit.timeInterval.lc;
                          interbool.timeInterval.end =
                              pCurrentUnit.timeInterval.end;
                          interbool.timeInterval.rc = false;
                          interbool.constValue.Set(true,false);
                          res->MergeAdd(interbool);
                          interbool.timeInterval.start =
                              pCurrentUnit.timeInterval.end;
                          interbool.timeInterval.rc =
                              pCurrentUnit.timeInterval.rc;
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
                      interbool.timeInterval = pCurrentUnit.timeInterval;
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
            interbool.timeInterval = pCurrentUnit.timeInterval;
            interbool.constValue.Set(true,false);
            res->MergeAdd(interbool);
          }
        }
      } // end for
      res->EndBulkLoad();
    }
//   } else {
//     const UGPoint *pFirst, *pLast;
//     Get(0, pFirst);
//     Get(GetNoComponents()-1, pLast);
//     UBool interbool;
//     interbool.timeInterval.start = pFirst->timeInterval.start;
//     interbool.timeInterval.end = pLast->timeInterval.end;
//     interbool.timeInterval.lc = pFirst->timeInterval.lc;
//     interbool.timeInterval.rc = pLast->timeInterval.rc;
//     interbool.constValue.Set(true,false);
//     res->StartBulkLoad();
//     res->MergeAdd(interbool);
//     res->EndBulkLoad();
//   }
}

/*
Restricts the ~mgpoint~ to the given ~periods~

*/

void MGPoint::Atperiods(Periods *&per, MGPoint *&res){
  if(!IsDefined() || !per->IsDefined() || IsEmpty() || per->IsEmpty())
  {
    res->SetDefined(false);
  }
  else //both are defined and have at least one interval
  {
    Instant utstart, utend;
    GPoint uGPstart, uGPend;
    bool ulc = true;
    bool urc = true;
    UGPoint pCurrentUnit;
    int i = 0;
    Get(i, pCurrentUnit);
    int j = 0;
    Interval<Instant> interval;
    per->Get(j,interval);
    while (interval.Before(pCurrentUnit.timeInterval))
    {
      if (++j >= per->GetNoComponents()) break;
      else per->Get(j,interval);
    }
    if(pCurrentUnit.timeInterval.Before(interval))
    {
      i = Position(interval.start, false);
      if (i != -1) Get(i, pCurrentUnit);
      else i = 0;
    }
    res->StartBulkLoad();
    while( i < GetNoComponents() && j < per->GetNoComponents())
    {
      if (pCurrentUnit.timeInterval.end < interval.start)
      {
        if( ++i >= GetNoComponents()) break;
        else Get( i, pCurrentUnit );
      }
      else
      { // we have overlapping intervals, now
        if (pCurrentUnit.timeInterval.start > interval.start &&
            pCurrentUnit.timeInterval.end < interval.end)
        {
          res->Add(pCurrentUnit);
          if (++i >= GetNoComponents()) break;
          else Get(i,pCurrentUnit);
        }
        else
        {
          if (pCurrentUnit.timeInterval.start == interval.start)
          {
            utstart = interval.start;
            uGPstart = pCurrentUnit.p0;
            ulc = pCurrentUnit.timeInterval.lc && interval.lc;
          }
          else
          {
            if (pCurrentUnit.timeInterval.start > interval.start)
            {
              utstart = pCurrentUnit.timeInterval.start;
              uGPstart = pCurrentUnit.p0;
              ulc = pCurrentUnit.timeInterval.lc;
            }
            else
            {
              if (pCurrentUnit.timeInterval.start < interval.start)
              {
                utstart = interval.start;
                ulc = interval.lc;
                pCurrentUnit.TemporalFunction(utstart, uGPstart, false);
              }
            }
          }
          if (pCurrentUnit.timeInterval.end == interval.end)
          {
            utend = interval.end;
            uGPend = pCurrentUnit.p1;
            urc = pCurrentUnit.timeInterval.rc && interval.rc;
          }
          else
          {
            if (pCurrentUnit.timeInterval.end < interval.end)
            {
              utend = pCurrentUnit.timeInterval.end;
              urc = pCurrentUnit.timeInterval.rc;
              uGPend = pCurrentUnit.p1;
            }
            else
            {
              if (pCurrentUnit.timeInterval.end > interval.end)
              {
                utend = interval.end;
                pCurrentUnit.TemporalFunction(utend, uGPend , false);
                urc = interval.rc;
              }
            }
          }
          res->Add(UGPoint(Interval<Instant>(utstart, utend, ulc, urc),
                                  uGPstart.GetNetworkId(),
                                  uGPstart.GetRouteId(),
                                  uGPstart.GetSide(),
                                  uGPstart.GetPosition(),
                                  uGPend.GetPosition()));
          if( interval.end == pCurrentUnit.timeInterval.end )
          {
            // same ending instant
            if (interval.rc == pCurrentUnit.timeInterval.rc)
            {
              if( ++i >= GetNoComponents()) break;
              else Get( i, pCurrentUnit );
              if( ++j >= per->GetNoComponents()) break;
              else per->Get( j, interval );
            }
            else
            {
              if(interval.rc)
              {
                if( ++i >= GetNoComponents()) break;
                else Get( i, pCurrentUnit );
              }
              else
              { // !interval->rc
                if( ++j >= per->GetNoComponents() ) break;
                else per->Get( j, interval );
              }
            }
          }
          else
          {
            if (interval.end > pCurrentUnit.timeInterval.end)
            {
              if( ++i >= GetNoComponents() ) break;
              else Get( i, pCurrentUnit );
            }
            else
            {
              if( ++j >= per->GetNoComponents()) break;
              else per->Get( j, interval );
            }
          }
        }
      }
    }
    res->EndBulkLoad(true);
    res->SetDefined(true);
    res->SetTrajectoryDefined(false);
    res->m_trajectory.TrimToSize();
    res->SetBoundingBoxDefined(false);
  }
}

/*
Restricts the ~mgpoint~ to the given ~instant~

*/

void MGPoint::Atinstant(Instant *&per, Intime<GPoint> *&res){
  if (IsDefined() && !IsEmpty() && per->IsDefined())
  {
    UGPoint pCurrUnit;
    int pos = Position(*per);
    if (pos == -1) res->SetDefined(false);
    else
    {
      res->SetDefined(true);
      Get(pos,pCurrUnit);
      GPoint gp = GPoint(false);
      pCurrUnit.TemporalFunction(*per, gp);
      if (gp.IsDefined()) *res=Intime<GPoint>(*per,gp);
      else res->SetDefined(false);
    }
  }
  else res->SetDefined(false);
}

/*
Restricts the ~mgpoint~ to the times it was at the given places.

*/

void MGPoint::At(GPoint *&gp, MGPoint *&res){
//   DbArray<RouteInterval> tra = GetTrajectory();
//   if (Includes(tra, gp)){
  if (!IsDefined() || IsEmpty() || !gp->IsDefined()) res->SetDefined(false);
  else {
    Instant tPos;
    UGPoint pCurrentUnit, pCheckUnit;
    Get(0, pCurrentUnit);
    if (gp->GetNetworkId() != pCurrentUnit.p0.GetNetworkId()) {
      res->SetDefined(false);
    } else {
      res->StartBulkLoad();
      int i= 0;
      while (i < GetNoComponents()) {
        Get(i, pCurrentUnit);
        UGPoint CurrUnit = pCurrentUnit;
        if (pCurrentUnit.p0.GetRouteId() == gp->GetRouteId() &&
            (pCurrentUnit.p0.GetSide() == gp->GetSide() ||
            pCurrentUnit.p0.GetSide() ==2 || gp->GetSide()== 2)){
          if (fabs(gp->GetPosition()-pCurrentUnit.p0.GetPosition()) < 0.01){
            if (pCurrentUnit.p0.GetPosition() ==
                pCurrentUnit.p1.GetPosition()){
              res->Add(UGPoint(pCurrentUnit.timeInterval,
                        gp->GetNetworkId(), gp->GetRouteId(),
                        pCurrentUnit.p0.GetSide(), gp->GetPosition(),
                        gp->GetPosition()));
            } else {
              if(pCurrentUnit.timeInterval.lc) {
                res->Add(UGPoint(Interval<Instant>(
                                pCurrentUnit.timeInterval.start,
                                pCurrentUnit.timeInterval.start,
                                true, true),
                                gp->GetNetworkId(), gp->GetRouteId(),
                                pCurrentUnit.p0.GetSide(), gp->GetPosition(),
                                gp->GetPosition()));
              }
            }
          } else {
            if(fabs(gp->GetPosition()-pCurrentUnit.p1.GetPosition())<0.01) {
              if (pCurrentUnit.timeInterval.rc) {
                if (i < GetNoComponents()-1){
                  i++;
                  Get(i,pCheckUnit);
                  if (pCheckUnit.p0.GetRouteId() ==
                      pCurrentUnit.p1.GetRouteId() &&
                      pCheckUnit.p0.GetPosition() ==
                      pCurrentUnit.p1.GetPosition()&&
                      pCheckUnit.timeInterval.start ==
                      pCurrentUnit.timeInterval.end){
                    if (pCheckUnit.p0.GetPosition() !=
                        pCheckUnit.p1.GetPosition()){
                      res->Add(UGPoint(Interval<Instant>(
                                pCurrentUnit.timeInterval.end,
                                pCurrentUnit.timeInterval.end,
                                true, true),
                                gp->GetNetworkId(), gp->GetRouteId(),
                                pCurrentUnit.p0.GetSide(), gp->GetPosition(),
                                gp->GetPosition()));
                    } else {
                      res->Add(UGPoint(pCheckUnit.timeInterval,
                                  gp->GetNetworkId(), gp->GetRouteId(),
                                  pCheckUnit.p0.GetSide(), gp->GetPosition(),
                                  gp->GetPosition()));
                    }
                  }
                } else {
                  res->Add(UGPoint(Interval<Instant>(
                                pCurrentUnit.timeInterval.end,
                                pCurrentUnit.timeInterval.end,
                                true, true),
                                gp->GetNetworkId(), gp->GetRouteId(),
                                pCurrentUnit.p0.GetSide(), gp->GetPosition(),
                                gp->GetPosition()));
                }
              } else {
                if (i < GetNoComponents()-1) {
                  i++;
                  Get(i,pCheckUnit);
                  if (pCheckUnit.p0.GetRouteId() ==
                      pCurrentUnit.p1.GetRouteId()
                      && pCheckUnit.p0.GetPosition() ==
                      pCurrentUnit.p1.GetPosition()
                      && pCheckUnit.timeInterval.start ==
                      pCurrentUnit.timeInterval.end){
                    if (pCheckUnit.p0.GetPosition() !=
                        pCheckUnit.p1.GetPosition()){
                      res->Add(UGPoint(Interval<Instant>(
                              pCurrentUnit.timeInterval.end,
                              pCurrentUnit.timeInterval.end,
                              true, true),
                              gp->GetNetworkId(), gp->GetRouteId(),
                              pCurrentUnit.p0.GetSide(), gp->GetPosition(),
                              gp->GetPosition()));
                    } else {
                      res->Add(UGPoint(pCheckUnit.timeInterval,
                        gp->GetNetworkId(), gp->GetRouteId(),
                        pCheckUnit.p0.GetSide(), gp->GetPosition(),
                        gp->GetPosition()));
                    }
                  }
                }
              }
            } else {
              if((pCurrentUnit.p0.GetPosition() < gp->GetPosition() &&
                  gp->GetPosition() < pCurrentUnit.p1.GetPosition()) ||
                (pCurrentUnit.p1.GetPosition() < gp->GetPosition() &&
                  gp->GetPosition() < pCurrentUnit.p0.GetPosition())) {
                Instant tPos = CurrUnit.TimeAtPos(gp->GetPosition());
                res->Add(UGPoint(Interval<Instant>(tPos, tPos, true, true),
                        gp->GetNetworkId(), gp->GetRouteId(),
                        pCurrentUnit.p0.GetSide(), gp->GetPosition(),
                        gp->GetPosition()));
              }
            }
          }
        }
        i++;
      }
      res->EndBulkLoad(true);
      res->SetDefined(true);
    }
  }
//   }else{
//     res->SetDefined(true);
//   }
  res->SetTrajectoryDefined(false);
  res->m_trajectory.TrimToSize();
  res->SetBoundingBoxDefined(false);
}

void MGPoint::At(GLine *&gl, MGPoint *&res){
//   DbArray<RouteInterval> tra = GetTrajectory();
//   DbArray<RouteInterval>* gltra = gl->GetRouteIntervals();
//   if (Intersects(tra, *gltra, true, gl->IsSorted())){
    UGPoint pCurrentUnit;
    Get(0, pCurrentUnit);
    if (gl->GetNetworkId() != pCurrentUnit.p0.GetNetworkId()) {
      res->SetDefined(false);
    } else {
      int iNetworkId = gl->GetNetworkId();
      double mgStart, mgEnd, lStart, lEnd, interStart, interEnd;
      RouteInterval pCurrRInter;
      Instant tInterStart, tInterEnd;
      bool bInterStart, bInterEnd, swapped;
      int iRouteMgp;
      res->StartBulkLoad();
      for (int i = 0; i < GetNoComponents(); i++) {
        Get(i, pCurrentUnit);
        UGPoint CurrUnit = pCurrentUnit;
        iRouteMgp = pCurrentUnit.p0.GetRouteId();
        mgStart = pCurrentUnit.p0.GetPosition();
        mgEnd = pCurrentUnit.p1.GetPosition();
        swapped = false;
        if (mgEnd < mgStart) {
          mgStart = pCurrentUnit.p1.GetPosition();
          mgEnd = pCurrentUnit.p0.GetPosition();
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
              if (pCurrentUnit.p0.GetPosition() ==
                  pCurrentUnit.p1.GetPosition() &&
                  ((currRInter->GetStartPos() <=
                      pCurrentUnit.p0.GetPosition() &&
                    currRInter->GetEndPos() >= pCurrentUnit.p0.GetPosition())||
                    (currRInter->GetStartPos() >=
                      pCurrentUnit.p0.GetPosition() &&
                    currRInter->GetEndPos() <=
                      pCurrentUnit.p0.GetPosition()))) {
                res->Add(pCurrentUnit);
              } else {
                if(pCurrentUnit.p0.GetPosition() <
                  pCurrentUnit.p1.GetPosition())
                {
                  if (pCurrentUnit.p0.GetPosition() >=
                        currRInter->GetStartPos() &&
                    pCurrentUnit.p1.GetPosition() <= currRInter->GetEndPos()){
                    res->Add(pCurrentUnit);
                  }else{
                    if (pCurrentUnit.p0.GetPosition() >=
                          currRInter->GetStartPos()){
                      interStart = pCurrentUnit.p0.GetPosition();
                      tInterStart = pCurrentUnit.timeInterval.start;
                      bInterStart = pCurrentUnit.timeInterval.lc;
                    } else {
                      interStart = currRInter->GetStartPos();
                      bInterStart = true;
                      tInterStart = CurrUnit.TimeAtPos(interStart);
                    }
                    if (pCurrentUnit.p1.GetPosition() <=
                          currRInter->GetEndPos()) {
                      interEnd = pCurrentUnit.p1.GetPosition();
                      tInterEnd = pCurrentUnit.timeInterval.end;
                      bInterEnd = pCurrentUnit.timeInterval.rc;
                    } else {
                      interEnd = currRInter->GetEndPos();
                      bInterEnd = true;
                      tInterEnd = CurrUnit.TimeAtPos(interEnd);
                    }
                    if (interStart != interEnd || (bInterStart && bInterEnd)) {
                      res->Add(UGPoint(Interval<Instant> (tInterStart,tInterEnd,
                                    bInterStart, bInterEnd), iNetworkId,
                                    iRouteMgp, pCurrentUnit.p0.GetSide(),
                                    interStart, interEnd));
                    }
                  }
                } else {
                  if(pCurrentUnit.p0.GetPosition() >
                    pCurrentUnit.p1.GetPosition())
                  {
                    if(pCurrentUnit.p1.GetPosition() >=
                        currRInter->GetStartPos() &&
                    pCurrentUnit.p0.GetPosition()<=currRInter->GetEndPos()){
                      res->Add(pCurrentUnit);
                    }else{
                      if (currRInter->GetEndPos() >=
                            pCurrentUnit.p0.GetPosition()){
                        interStart = pCurrentUnit.p0.GetPosition();
                        tInterStart = pCurrentUnit.timeInterval.start;
                        bInterStart = pCurrentUnit.timeInterval.lc;
                      } else {
                        interStart = currRInter->GetEndPos();
                        bInterStart = true;
                        tInterStart = CurrUnit.TimeAtPos(interStart);
                      }
                      if(currRInter->GetStartPos() <=
                          pCurrentUnit.p1.GetPosition()){
                        interEnd = pCurrentUnit.p1.GetPosition();
                        tInterEnd = pCurrentUnit.timeInterval.end;
                        bInterEnd = pCurrentUnit.timeInterval.rc;
                      } else {
                        interEnd = currRInter->GetStartPos();
                        bInterEnd = true;
                        tInterEnd = CurrUnit.TimeAtPos(interEnd);
                      }
                      if(interStart != interEnd || (bInterStart && bInterEnd)){
                        res->Add(UGPoint(Interval<Instant> (tInterStart,
                                    tInterEnd, bInterStart, bInterEnd),
                                    iNetworkId, iRouteMgp,
                                    pCurrentUnit.p0.GetSide(),
                                    interStart, interEnd));
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
            if (iRouteMgp == pCurrRInter.GetRouteId()){

              lStart = pCurrRInter.GetStartPos();
              lEnd = pCurrRInter.GetEndPos();
              if (lStart > lEnd) {
                lStart = pCurrRInter.GetEndPos();
                lEnd = pCurrRInter.GetStartPos();
              }
              if (!(mgEnd < lStart || mgStart > lEnd)){
                //intersection exists compute intersecting part and timevalues
                //for resulting unit

                if (!swapped) {
                  if (lStart <= mgStart) {

                    interStart = pCurrentUnit.p0.GetPosition();
                    tInterStart = pCurrentUnit.timeInterval.start;
                    bInterStart = pCurrentUnit.timeInterval.lc;
                  } else {

                    interStart = lStart;
                    bInterStart = true;
                    tInterStart = CurrUnit.TimeAtPos(interStart);
                  }
                  if (lEnd >= mgEnd) {

                    interEnd = pCurrentUnit.p1.GetPosition();
                    tInterEnd = pCurrentUnit.timeInterval.end;
                    bInterEnd = pCurrentUnit.timeInterval.rc;
                  } else {

                    interEnd = lEnd;
                    bInterEnd = true;
                    tInterEnd = CurrUnit.TimeAtPos(interEnd);
                  }
                  if (!(interStart == interEnd &&
                        (!bInterStart || !bInterEnd))) {
                    if (tInterStart == pCurrentUnit.timeInterval.start &&
                        tInterEnd == pCurrentUnit.timeInterval.end &&
                        interStart == pCurrentUnit.p0.GetPosition()&&
                        interEnd == pCurrentUnit.p1.GetPosition()){
                      res->Add(pCurrentUnit);
                    }else{
                      res->Add(UGPoint(Interval<Instant> (tInterStart,
                                       tInterEnd,
                                    bInterStart, bInterEnd), iNetworkId,
                                    iRouteMgp, pCurrentUnit.p0.GetSide(),
                                    interStart, interEnd));
                    }
                  } else {
                    if (pCurrentUnit.p0.GetPosition() ==
                      pCurrentUnit.p1.GetPosition() && interStart == interEnd){
                      res->Add(pCurrentUnit);
                    }
                  }
                } else {
                  mgStart = pCurrentUnit.p0.GetPosition();
                  mgEnd = pCurrentUnit.p1.GetPosition();
                  if (lEnd >= mgStart) {
                    interStart = pCurrentUnit.p0.GetPosition();
                    tInterStart = pCurrentUnit.timeInterval.start;
                    bInterStart = pCurrentUnit.timeInterval.lc;
                  } else {
                    interStart = lEnd;
                    bInterStart = true;
                    tInterStart = CurrUnit.TimeAtPos(interStart);
                  }
                  if (lStart <= mgEnd) {
                    interEnd = pCurrentUnit.p1.GetPosition();
                    tInterEnd = pCurrentUnit.timeInterval.end;
                    bInterEnd = pCurrentUnit.timeInterval.rc;
                  } else {
                    interEnd = lStart;
                    bInterEnd = true;
                    tInterEnd = CurrUnit.TimeAtPos(interEnd);
                  }
                  if (!(interStart == interEnd &&
                        (!bInterStart || !bInterEnd))) {
                    if (tInterStart== pCurrentUnit.timeInterval.start &&
                      tInterEnd == pCurrentUnit.timeInterval.end &&
                      interStart == pCurrentUnit.p0.GetPosition() &&
                      interEnd == pCurrentUnit.p1.GetPosition()){
                      res->Add(pCurrentUnit);
                    }else{
                      res->Add(UGPoint(Interval<Instant> (tInterStart,
                                        tInterEnd,
                                        bInterStart, bInterEnd), iNetworkId,
                                        iRouteMgp, pCurrentUnit.p0.GetSide(),
                                        interStart, interEnd));
                    }
                  } else {
                    if (pCurrentUnit.p0.GetPosition() ==
                        pCurrentUnit.p1.GetPosition() &&
                        interStart == interEnd)
                    {
                      res->Add(pCurrentUnit);
                    }
                  }
                }
              }
            }
            j++;
          }
        }
      }
      res->EndBulkLoad(true);
      res->SetDefined(true);
    }
//   }else{
//     res->SetDefined(true);
//   }
  res->SetTrajectoryDefined(false);
  res->m_trajectory.TrimToSize();
  res->SetBoundingBoxDefined(false);
}

/*
Compresses the ~mgpoint~ by equalizing speed differences witch are smaller than
~d~

*/

void  MGPoint::Simplify(double d, MGPoint* res){
  res->StartBulkLoad();

  int iNetworkId;
  int iStartRouteId = 0;
  Side xStartSide = None;
  double dStartSpeed = 0.0;

  Instant xStartStartTime;
  double dStartStartPosition = 0.0;
  double dLastEndPosition = 0.0;
  Instant xLastEndTime;
  for (int i = 0; i < GetNoComponents(); i++)
  {
    //////////////////////////////
    //
    // Get values for current unit
    //
    //////////////////////////////
    UGPoint pCurrentUnit;
    Get(i, pCurrentUnit);

    // Duration
    Instant xCurrentStartTime = pCurrentUnit.timeInterval.start;
    int32_t lCurrentStartTime = xCurrentStartTime.GetAllMilliSeconds();
    Instant xCurrentEndTime = pCurrentUnit.timeInterval.end;
    int32_t lCurrentEndTime = xCurrentEndTime.GetAllMilliSeconds();
    int32_t lCurrentDuration = lCurrentEndTime - lCurrentStartTime;

    // Distance
    GPoint xCurrentStart = pCurrentUnit.p0;
    GPoint xCurrentEnd = pCurrentUnit.p1;
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
      dLastEndPosition = dCurrentEndPosition;
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
      if (xStartStartTime == pCurrentUnit.timeInterval.start &&
         xLastEndTime == pCurrentUnit.timeInterval.end &&
         iStartRouteId == pCurrentUnit.p0.GetRouteId()&&
         xStartSide == pCurrentUnit.p0.GetSide() &&
         dStartStartPosition == pCurrentUnit.p0.GetPosition() &&
         dLastEndPosition == pCurrentUnit.p1.GetPosition()) {
        res->Add(pCurrentUnit/*, false*/);
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
      dLastEndPosition = dCurrentEndPosition;
    }

    if( i == GetNoComponents() -1)
    {
      // Last loop - create last unit
      if (xStartStartTime == pCurrentUnit.timeInterval.start &&
         xLastEndTime == pCurrentUnit.timeInterval.end &&
         iStartRouteId == pCurrentUnit.p0.GetRouteId()&&
         xStartSide == pCurrentUnit.p0.GetSide() &&
         dStartStartPosition == pCurrentUnit.p0.GetPosition() &&
         dLastEndPosition == pCurrentUnit.p1.GetPosition()){
        res->Add(pCurrentUnit );
      } else {
        res->Add(UGPoint(Interval<Instant>(xStartStartTime,
                                                        xCurrentEndTime,
                                                        true,
                                                        false),
                              iNetworkId,
                              iStartRouteId,
                              xStartSide,
                              dStartStartPosition,
                              dCurrentEndPosition) );
      }
    }

    // Set Last-Values for next loop
    dLastEndPosition = dCurrentEndPosition;
    xLastEndTime = xCurrentEndTime;
  }
  // Units were added to the moving gpoint. They are sorted and
  // the bulk-load is ended:
  res->EndBulkLoad(true);
  if (m_traj_Defined) {
    res->SetTrajectory(m_trajectory);
  }
  if (m_bbox.IsDefined()) {
    res->SetBoundingBox(BoundingBox());
    res->SetBoundingBoxDefined(true);
  }
  res->m_trajectory.TrimToSize();
}

/*
Checks if the ~mgpoint~ passes the given ~gpoint~ respectively ~gline~.

*/

bool MGPoint::Passes(GPoint *&gp){
  if (!m_traj_Defined){
    GLine *help = new GLine(0);
    Trajectory(help);
    help->DeleteIfAllowed();
  }
  if (Includes(m_trajectory, gp)) return true;
    else return false;
}



bool MGPoint::Passes(GLine *&gl){
  if (!m_traj_Defined){
    GLine *help = new GLine(0);
    Trajectory(help);
    help->DeleteIfAllowed();
  }
  DbArray<RouteInterval>* gltra = gl->GetRouteIntervals();
  if (RIsIntersects(m_trajectory, *gltra, true, gl->IsSorted())) return true;
  else return false;
}


MGPoint* MGPoint::Clone() const {
  MGPoint *result = new MGPoint( GetNoComponents() );
  if(GetNoComponents()>0){
      result->units.resize(GetNoComponents());
  }
  result->StartBulkLoad();
  UGPoint unit;
  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, unit );
    result->Add( unit);
  }
  result->EndBulkLoad( false);
  if (m_traj_Defined)
    result->SetTrajectory(m_trajectory);
  result->SetTrajectoryDefined(m_traj_Defined);
  result->m_trajectory.TrimToSize();
  if (m_bbox.IsDefined())
    result->SetBoundingBox(m_bbox);
  result->SetBoundingBoxDefined(m_bbox.IsDefined());
  return result;
}

void MGPoint::Restrict( const vector< pair<int, int> >& intervals )
{
  units.Restrict( intervals, units ); // call super
  SetTrajectoryDefined(false);
  m_trajectory.TrimToSize();
  SetBoundingBoxDefined(false);
}

void MGPoint::Add(const UGPoint& u/*, bool setbbox =true*/){
   if (u.IsValid()) {
    units.Append(u);
    if (units.Size() == 1){;
        m_length = u.Length();
    } else {
        m_length += u.Length();
    }
  }
}

 Rectangle<3> MGPoint::BoundingBox() const{
   if (m_bbox.IsDefined()) return m_bbox;
   else {
    if(IsDefined() && !IsEmpty()){
      if (!m_traj_Defined) {
        if (GetNoComponents() > 0) {
          UGPoint pCurrentUnit;
          Get(0, pCurrentUnit);
          int aktRouteId = pCurrentUnit.p0.GetRouteId();
          double aktStartPos = pCurrentUnit.p0.GetPosition();
          double aktEndPos = pCurrentUnit.p1.GetPosition();
          chkStartEnd(aktStartPos, aktEndPos);
          RITree *tree = new RITree(aktRouteId, aktStartPos, aktEndPos,0,0);
          int curRoute;
          double curStartPos, curEndPos;
          for (int i = 1; i < GetNoComponents(); i++)
          {
            // Get start and end of current unit
            Get(i, pCurrentUnit);
            curRoute = pCurrentUnit.p0.GetRouteId();
            curStartPos = pCurrentUnit.p0.GetPosition();
            curEndPos = pCurrentUnit.p1.GetPosition();
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
          GLine *help = new GLine(0);
          tree->TreeToGLine(help);
          tree->RemoveTree();
          Rectangle<3> bbox, ribox3;
          Rectangle<2> ribox = Rectangle<2u>(false,0.0,0.0,0.0,0.0);
          UGPoint unit;
          Get(0,unit);
          /*Network *pNetwork =
              NetworkManager::GetNetwork(unit->p0.GetNetworkId());*/
          Network* pNetwork = GetNetwork();
          double x5 = unit.timeInterval.start.ToDouble();
          Get(GetNoComponents()-1,unit);
          double x6 = unit.timeInterval.end.ToDouble();
          RouteInterval ri;
          bool firstri = true;
          if (help->NoOfComponents()>0) {
            for (int i = 0; i < help->NoOfComponents(); i++) {
              help->Get(i,ri);
              ribox = ri.BoundingBox(pNetwork);
              if (firstri) {
                firstri = false;
                bbox = Rectangle<3> (true,
                                    ribox.MinD(0),
                                    ribox.MaxD(0),
                                    ribox.MinD(1),
                                    ribox.MaxD(1),
                                    x5, x6);
              } else {
                ribox3 = Rectangle<3> (true,
                                      ribox.MinD(0),
                                      ribox.MaxD(0),
                                      ribox.MinD(1),
                                      ribox.MaxD(1),
                                      x5, x6);
                bbox = bbox.Union(ribox3);
              }
            }
            NetworkManager::CloseNetwork(pNetwork);
            help->DeleteIfAllowed();
            return bbox;
          } else {
            NetworkManager::CloseNetwork(pNetwork);
            help->DeleteIfAllowed();
            return Rectangle<3> (false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
          }
        } else {
          return Rectangle<3>(false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        }
      } else {
        if (m_trajectory.Size() <= 0)
          return Rectangle<3>(false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        Rectangle<3> bbox, ribox3;
        Rectangle<2> ribox = Rectangle<2u>(false,0.0,0.0,0.0,0.0);
        UGPoint unit;
        Get(0,unit);
        /*Network *pNetwork =
          NetworkManager::GetNetwork(unit->p0.GetNetworkId());*/
        Network* pNetwork = GetNetwork();
        double x5 = unit.timeInterval.start.ToDouble();
        Get(GetNoComponents()-1,unit);
        double x6 = unit.timeInterval.end.ToDouble();
        RouteInterval ri;
        bool firstri = true;
        for (int i = 0; i < m_trajectory.Size(); i++) {
          m_trajectory.Get(i,ri);
          ribox = ri.BoundingBox(pNetwork);
          if (firstri) {
            firstri = false;
            bbox = Rectangle<3> (true,
                                ribox.MinD(0),
                                ribox.MaxD(0),
                                ribox.MinD(1),
                                ribox.MaxD(1),
                                x5, x6);
          } else {
            ribox3 = Rectangle<3> (true,
                                  ribox.MinD(0),
                                  ribox.MaxD(0),
                                  ribox.MinD(1),
                                  ribox.MaxD(1),
                                  x5, x6);
            bbox = bbox.Union(ribox3);
          }
        }
        NetworkManager::CloseNetwork(pNetwork);
        return bbox;
      }
    } else return Rectangle<3>(false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  }
}

void MGPoint::GetMGPSecUnits(vector<MGPSecUnit> &res,
                             double maxSectLength,
                            Network *pNet) const
{
  res.clear();
  if (IsDefined() && 0 < GetNoComponents())
  {
    UGPoint unit;
    MGPSecUnit actUnit, nextUnit;
    vector<MGPSecUnit> intermediate;
    intermediate.clear();
    Get(0,unit);
    unit.GetMGPSecUnits(intermediate, maxSectLength, pNet);
    actUnit = intermediate[0];
    if (intermediate.size()>1)
    {
      for (size_t j = 1; j < intermediate.size() ; j++)
      {
        res.push_back(actUnit);
        actUnit = intermediate[j];
      }
    }
    for (int i = 1; i < GetNoComponents(); i++)
    {
      Get(i,unit);
      intermediate.clear();
      unit.GetMGPSecUnits(intermediate, maxSectLength, pNet);
      nextUnit = intermediate[0];
      if (actUnit.GetSecId() == nextUnit.GetSecId() &&
          actUnit.GetPart() == nextUnit.GetPart() &&
          (actUnit.GetDirect() == nextUnit.GetDirect() ||
           actUnit.GetDirect() == None || nextUnit.GetDirect() == None)&&
         actUnit.GetTimeInterval().end == nextUnit.GetTimeInterval().start)
      {
        if(actUnit.GetDirect() == None) actUnit.SetDirect(nextUnit.GetDirect());
        double speed = (actUnit.GetDurationInSeconds() * actUnit.GetSpeed() +
              nextUnit.GetDurationInSeconds() * nextUnit.GetSpeed()) /
              (actUnit.GetDurationInSeconds() +
              nextUnit.GetDurationInSeconds());
        actUnit.SetSpeed(speed);
        actUnit.SetTimeInterval(Interval<Instant> (
                                actUnit.GetTimeInterval().start,
                                nextUnit.GetTimeInterval().end,
                                actUnit.GetTimeInterval().lc,
                                nextUnit.GetTimeInterval().rc));
      }
      else
      {
        res.push_back(actUnit);
        actUnit = nextUnit;
      }
      if (intermediate.size() > 1)
      {
        for (size_t j = 1; j < intermediate.size() ; j++)
        {
          res.push_back(actUnit);
          actUnit = intermediate[j];
        }
      }
    }
    res.push_back(actUnit);
    intermediate.clear();
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
    UGPoint unit;
    Get( i , unit );
    os << "\n\t";
    unit.Print(os);
  }
  os << "\n" << endl;
  /*os <<", constains Trajectory " << m_trajectory.Size() << " intervals:";
  for (int i = 0; i < m_trajectory.Size(); i++) {
    const RouteInterval *ri;
    m_trajectory.Get(i, ri);
    os << "/n/t";
    os << "(" << ri->GetRouteId() << ", ";
    os << ri->GetStartPos() << ", " << ri->GetEndPos() << ")";
  }
  os << "\n)" << endl;*/
  return os;
}

Word InMGPoint(const ListExpr typeInfo, const ListExpr instance,
                const int errorPos, ListExpr& errorInfo, bool& correct)
{
  int numUnits = nl->ListLength(instance);
  MGPoint* m = new MGPoint( numUnits );
  correct = true;
  int unitcounter = 0;
  string errmsg;
  m->StartBulkLoad();
  ListExpr rest = instance;
  if (nl->AtomType( rest ) != NoAtom)
  { if(nl->IsEqual(rest,"undef")){
       m->EndBulkLoad(true);
       m->SetDefined(false);
       return SetWord( Address( m ) );
    } else {
      correct = false;
      m->DeleteIfAllowed();
      return SetWord( Address( 0 ) );
    }
  }
  else {
    double test1, test2;
    RITree *tree = 0;
    bool firstunit = true;
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
        unit->DeleteIfAllowed();
        m->DeleteIfAllowed();
        //if (!bfirst) NetworkManager::CloseNetwork(pNetwork);
        return SetWord( Address(0) );
      }
      if ( !correct )
      {
        errmsg = "InMapping(): Representation of Unit "
                  + int2string(unitcounter) + " is wrong.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        m->Destroy();
        m->DeleteIfAllowed();
        //if (!bfirst) NetworkManager::CloseNetwork(pNetwork);
        return SetWord( Address(0) );
      }
      m->Add( *unit);
      unitcounter++;
      test1 = unit->p0.GetPosition();
      test2 = unit->p1.GetPosition();
      chkStartEnd(test1, test2);
      if (firstunit) {
        firstunit = false;
        tree = new RITree(unit->p0.GetRouteId(), test1, test2);
      } else
        tree->Insert(unit->p0.GetRouteId(), test1, test2);
        unit->DeleteIfAllowed();
    }
    m->EndBulkLoad(true); // if this succeeds, all is OK
    tree->TreeToDbArray(&(m->m_trajectory));
    m->SetTrajectoryDefined(true);
    m->m_trajectory.TrimToSize();
    tree->RemoveTree();
    m->SetBoundingBoxDefined(false);
  }
  return SetWord( m );
}


bool OpenMGPoint(SmiRecord& valueRecord,
                     size_t& offset,
                     const ListExpr typeInfo,
                     Word& value)
{
  MGPoint *m =
      static_cast<MGPoint*>(Attribute::Open( valueRecord, offset, typeInfo ));
  value = SetWord( m );
  m->m_trajectory.TrimToSize();
  return true;
}

DbArray<RouteInterval>& MGPoint::GetTrajectory(){
  if (!m_traj_Defined) {
    GLine *help = new GLine(0);
    Trajectory(help);
    help->DeleteIfAllowed();
  }
  return m_trajectory;
}

void MGPoint::SetTrajectoryDefined(bool defined){
  m_traj_Defined=defined;
}

void MGPoint::SetBoundingBoxDefined(bool defined){
  m_bbox.SetDefined(defined);
  if (!defined) m_bbox = Rectangle<3> (false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
}

/*
Merges two MGPoint into one if the time intervals don't overlap.
Otherwise the union is undefined.

*/

void MGPoint::Union(MGPoint *mp, MGPoint *res)
{
  if (IsDefined() && mp->IsDefined() &&
      GetNoComponents() > 0 && mp->GetNoComponents() > 0)
  {
    int i = 0;
    int j = 0;
    UGPoint u1, u2;
    res->StartBulkLoad();
    while (i < GetNoComponents() && j < mp->GetNoComponents())
    {
      Get(i, u1);
      mp->Get(j,u2);
      if (u1.timeInterval.end.ToDouble() <=
            u2.timeInterval.start.ToDouble())
      {
        res->Add(UGPoint(Interval<Instant> (u1.timeInterval.start,
                        u1.timeInterval.end,
                        true, false),
                        u1.p0.GetNetworkId(),
                        u1.p0.GetRouteId(),
                        u1.p0.GetSide(),
                        u1.p0.GetPosition(),
                        u1.p1.GetPosition()));
        i++;
      }
      else
      {
        if (u2.timeInterval.end.ToDouble() <=
             u1.timeInterval.start.ToDouble())
        {
          res->Add(UGPoint(Interval<Instant> (u2.timeInterval.start,
                        u2.timeInterval.end,
                        true, false),
                        u2.p0.GetNetworkId(),
                        u2.p0.GetRouteId(),
                        u2.p0.GetSide(),
                        u2.p0.GetPosition(),
                        u2.p1.GetPosition()));
          j++;
        }
        else
        {
          if (u1 == u2)
          {
            res->Add(UGPoint(Interval<Instant> (u1.timeInterval.start,
                        u1.timeInterval.end,
                        true, false),
                        u1.p0.GetNetworkId(),
                        u1.p0.GetRouteId(),
                        u1.p0.GetSide(),
                        u1.p0.GetPosition(),
                        u1.p1.GetPosition()));
            i++;
            j++;
          }
          else
          {
            if (fabs(u1.timeInterval.end.ToDouble() -
                            u2.timeInterval.start.ToDouble())<= 0.00000002)
            {
              res->Add(UGPoint(Interval<Instant> (u1.timeInterval.start,
                               u2.timeInterval.start,true, false),
                               u1.p0.GetNetworkId(),
                               u1.p0.GetRouteId(),
                               u1.p0.GetSide(),
                               u1.p0.GetPosition(),
                               u1.p1.GetPosition()));
              res->Add(UGPoint(Interval<Instant> (u2.timeInterval.start,
                               u2.timeInterval.end,true, false),
                               u2.p0.GetNetworkId(),
                               u2.p0.GetRouteId(),
                               u2.p0.GetSide(),
                               u2.p0.GetPosition(),
                               u2.p1.GetPosition()));
              i++;
              j++;
            }
            else
            {
              if (fabs(u2.timeInterval.end.ToDouble() -
                       u1.timeInterval.start.ToDouble()) <= 0.00000002)
              {
                res->Add(UGPoint(Interval<Instant> (u2.timeInterval.start,
                               u1.timeInterval.start,true, false),
                               u2.p0.GetNetworkId(),
                               u2.p0.GetRouteId(),
                               u2.p0.GetSide(),
                               u2.p0.GetPosition(),
                               u2.p1.GetPosition()));
                res->Add(UGPoint(Interval<Instant> (u1.timeInterval.start,
                               u1.timeInterval.end,true, false),
                               u1.p0.GetNetworkId(),
                               u1.p0.GetRouteId(),
                               u1.p0.GetSide(),
                               u1.p0.GetPosition(),
                               u1.p1.GetPosition()));
                i++;
                j++;
              }
              else
              {
                res->SetDefined(false);
                i = GetNoComponents();
                j = mp->GetNoComponents();
              }
            }
          }
        }
      }
    }
    if (i < GetNoComponents())
    {
      Get(i,u1);
      res->Add(UGPoint(Interval<Instant> (u1.timeInterval.start,
                        u1.timeInterval.end,
                        true, false),
                        u1.p0.GetNetworkId(),
                        u1.p0.GetRouteId(),
                        u1.p0.GetSide(),
                        u1.p0.GetPosition(),
                        u1.p1.GetPosition()));
      i++;
      while (i < GetNoComponents())
      {
        Get(i,u1);
        res->Add(UGPoint(Interval<Instant> (u1.timeInterval.start,
                        u1.timeInterval.end,
                        true, false),
                        u1.p0.GetNetworkId(),
                        u1.p0.GetRouteId(),
                        u1.p0.GetSide(),
                        u1.p0.GetPosition(),
                        u1.p1.GetPosition()));
        i++;
      }
    }
    if (j < mp->GetNoComponents())
    {
      mp->Get(j,u2);
      res->Add(UGPoint(Interval<Instant> (u2.timeInterval.start,
                        u2.timeInterval.end,
                        true, false),
                        u2.p0.GetNetworkId(),
                        u2.p0.GetRouteId(),
                        u2.p0.GetSide(),
                        u2.p0.GetPosition(),
                        u2.p1.GetPosition()));
      j++;
      while (j < mp->GetNoComponents())
      {
        mp->Get(j,u2);
        res->Add(UGPoint(Interval<Instant> (u2.timeInterval.start,
                        u2.timeInterval.end,
                        true, false),
                        u2.p0.GetNetworkId(),
                        u2.p0.GetRouteId(),
                        u2.p0.GetSide(),
                        u2.p0.GetPosition(),
                        u2.p1.GetPosition()));
        j++;
      }
    }
    res->EndBulkLoad();
  }
  else
  {
    if (IsDefined() && !(mp->IsDefined()) && GetNoComponents() > 0)
    {
      int i = 0;
      UGPoint u1;
      res->StartBulkLoad();
      while (i < GetNoComponents())
      {
        Get(i,u1);
        res->Add(UGPoint(Interval<Instant> (u1.timeInterval.start,
                        u1.timeInterval.end,
                        true, false),
                        u1.p0.GetNetworkId(),
                        u1.p0.GetRouteId(),
                        u1.p0.GetSide(),
                        u1.p0.GetPosition(),
                        u1.p1.GetPosition()));
        i++;
      }
      res->EndBulkLoad();
    }
    else
    {
      if (mp->IsDefined() && !(IsDefined()) && mp->GetNoComponents() > 0)
      {
        int j = 0;
        UGPoint u2;
        res->StartBulkLoad();
        while (j < mp->GetNoComponents())
        {
          mp->Get(j,u2);
          res->Add(UGPoint(Interval<Instant> (u2.timeInterval.start,
                        u2.timeInterval.end,
                        true, false),
                        u2.p0.GetNetworkId(),
                        u2.p0.GetRouteId(),
                        u2.p0.GetSide(),
                        u2.p0.GetPosition(),
                        u2.p1.GetPosition()));
          j++;
        }
        res->EndBulkLoad();
      }
      else
      {
        res->SetDefined(false);
      }
    }
  }
  res->SetTrajectoryDefined(false);
  res->m_trajectory.TrimToSize();
  res->SetBoundingBoxDefined(false);
}

TypeConstructor movinggpoint(
        "mgpoint",                                  // Name
        MGPoint::Property,                          // Property function
        OutMapping<MGPoint, UGPoint, UGPoint::Out>, // Out and In functions
        InMapping<MGPoint, UGPoint, UGPoint::In>,
        /*InMGPoint,*/
        0,                                          // SaveToList and
        0,                                          // RestoreFromList
        CreateMapping<MGPoint>,                     // Object creation and
        DeleteMapping<MGPoint>,                     // deletion
        OpenAttribute<MGPoint>,                    /*OpenMGPoint,*/
        SaveAttribute<MGPoint>,                     // Object open and save
        CloseMapping<MGPoint>,                      // Object close and clone
        CloneMapping<MGPoint>,
        CastMapping<MGPoint>,                       // Cast function
        SizeOfMapping<MGPoint>,                     // Sizeof function
        MGPoint::Check);                            // Kind checking function



/*
3 Classe UGPoint

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
        result = GPoint(true, p0.GetNetworkId(), p0.GetRouteId(),
                       (((p1.GetPosition()-p0.GetPosition()) *
                       ((t-timeInterval.start) /
                       (timeInterval.end - timeInterval.start)))
                        + p0.GetPosition()),
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
        start->DeleteIfAllowed();
        return SetWord( Address(0) );
      }

      Instant *end = (Instant *)InInstant( nl->TheEmptyList(),
       nl->Second( first ),
       errorPos, errorInfo, correct ).addr;

      if( !correct )
      {
        errmsg = "InUGPoint(): Error in second instant.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        start->DeleteIfAllowed();
        end->DeleteIfAllowed();
        return SetWord( Address(0) );
      }

      Interval<Instant> tinterval( *start, *end,
                                   nl->BoolValue( nl->Third( first ) ),
                                   nl->BoolValue( nl->Fourth( first ) ) );
      start->DeleteIfAllowed();
      end->DeleteIfAllowed();

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
        ugpoint->DeleteIfAllowed();
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
  return (SetWord( new UGPoint(true) ));
}

void UGPoint::Delete(const ListExpr typeInfo,
                     Word& w)
{
  UGPoint *u = (UGPoint *)w.addr;
  if(u->DeleteIfAllowed()) w.addr = 0;
}

void UGPoint::Close(const ListExpr typeInfo,
                    Word& w)
{
  UGPoint *u = (UGPoint *)w.addr;
  if(u->DeleteIfAllowed()) w.addr = 0;
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


void UGPoint::Deftime(Periods &per){
  per.Clear();
  if (IsDefined()) {
    per.StartBulkLoad();
    per.Add(timeInterval);
    per.EndBulkLoad();
    per.SetDefined(true);
  } else per.SetDefined(false);
}

Instant UGPoint::TimeAtPos(double pos) const{
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
  //Network* pNetwork = NetworkManager::GetNetwork(rgp10.GetNetworkId());
  Network* pNetwork = NetworkManager::GetNetworkNew(rgp10.GetNetworkId(),
                                                    netList);
  Point *rp10;
  pNetwork->GetPointOnRoute(&rgp10, rp10);
  Point *rp11;
  pNetwork->GetPointOnRoute(&rgp11, rp11);
  Point *rp20;
  pNetwork->GetPointOnRoute(&rgp20, rp20);
  Point *rp21;
  pNetwork->GetPointOnRoute(&rgp21, rp21);
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
  rp10->DeleteIfAllowed();
  rp11->DeleteIfAllowed();
  rp20->DeleteIfAllowed();
  rp21->DeleteIfAllowed();
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

  const Rectangle<3> UGPoint::BoundingBox(const Geoid* g )const {
    assert (g == 0);
    if (IsDefined()) {
      RouteInterval *ri = new RouteInterval(p0.GetRouteId(), p0.GetPosition(),
                                         p1.GetPosition());
      //Network *pNetwork = NetworkManager::GetNetwork(p0.GetNetworkId());
      Network* pNetwork = NetworkManager::GetNetworkNew(p0.GetNetworkId(),
                                                        netList);
      Rectangle<2> rect = ri->BoundingBox(pNetwork);
      NetworkManager::CloseNetwork(pNetwork);
      delete ri;
      return Rectangle<3> (true,
                          rect.MinD(0), rect.MaxD(0),
                          rect.MinD(1), rect.MaxD(1),
                          timeInterval.start.ToDouble(),
                          timeInterval.end.ToDouble());
    } else return Rectangle<3>(false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  }

 double UGPoint::Distance(const Rectangle<3>& rect, const Geoid* g) const{
   assert( g == 0);
   cerr << "Distance function not implemented yet";
   if(!IsDefined() || !rect.IsDefined()){
     return -1;
   } else {
     return BoundingBox().Distance(rect);
   }
 }

 void UGPoint::GetPassedSections(Network* pNet, vector<TupleId>& pS) const
 {
   const RouteInterval *ri = new RouteInterval(p0.GetRouteId(),p0.GetPosition(),
                                               p1.GetPosition());
   pNet->GetSectionsOfRoutInterval(ri,pS);
   delete ri;
   if (pS.size() > 1 && MovingDirection() == Down)
   {
     vector<TupleId> help;
     help.clear();
     for (size_t a = 0; a < pS.size() ; a++)
     {
       help.push_back(pS[a]);
     }
     pS.clear();
     size_t b = help.size()-1;
     while (b > 0)
     {
       pS.push_back(help[b]);
       b--;
     }
     pS.push_back(help[b]);
     help.clear();
   }
 }


 void PartedSection(UGPoint unit, double sectMeas1, double sectMeas2,
                    double &startPos, int &partNo, double maxSectLength,
                    int secId, Instant &tStart, Instant &tEnd,
                    vector<MGPSecUnit> &res)
{
  if (unit.MovingDirection() == Up)
  {
    double actEndPos = sectMeas1 + partNo * maxSectLength;
    while (unit.p1.GetPosition() > actEndPos && !(actEndPos > sectMeas2))
    {
      tEnd = unit.TimeAtPos(actEndPos);
      double speed = (actEndPos - startPos)/
            ((tEnd - tStart).ToDouble()/0.00001157);
      if (tStart != tEnd)
        res.push_back(MGPSecUnit(true, secId, partNo, Up, speed,
                    Interval<Instant> (tStart, tEnd, true, false)));
      tStart = tEnd;
      startPos = actEndPos;
      partNo++;
      actEndPos = sectMeas1 + partNo * maxSectLength;
    }
    if (unit.p1.GetPosition() > sectMeas2)
    {
      tEnd = unit.TimeAtPos(sectMeas2);
      double speed = (sectMeas2 - startPos)/
          ((tEnd-tStart).ToDouble()/0.00001157);
      if (tStart != tEnd)
        res.push_back(MGPSecUnit(true, secId, partNo, Up, speed,
                    Interval<Instant> (tStart, tEnd, true, false)));
      tStart = tEnd;
      startPos = sectMeas2;
    }
    else
    {
      double speed = (unit.p1.GetPosition()-startPos)/
             ((unit.timeInterval.end - tStart).ToDouble()/0.00001157);
      if (tStart != unit.timeInterval.end)
        res.push_back(MGPSecUnit(true, secId, partNo, Up, speed,
                    Interval<Instant> (tStart,
                                       unit.timeInterval.end,
                                       true, false)));
      startPos = unit.p1.GetPosition();
      tStart = unit.timeInterval.end;
    }
  }
  else //MovingDirection == Down
  {
    double actEndPos = sectMeas1 + (partNo-1) * maxSectLength;
    while (unit.p1.GetPosition() < actEndPos && !(actEndPos < sectMeas1) &&
          partNo > 0)
    {
      tEnd = unit.TimeAtPos(actEndPos);
      double speed = (startPos-actEndPos)/
            ((tEnd - tStart).ToDouble()/0.00001157);
      if (tStart != tEnd)
        res.push_back(MGPSecUnit(true, secId, partNo, Down, speed,
                    Interval<Instant> (tStart, tEnd, true, false)));
      tStart = tEnd;
      startPos = actEndPos;
      partNo--;
      actEndPos = sectMeas1 + (partNo-1) * maxSectLength;
    }
    if (partNo <= 0) partNo = 1;
    if (unit.p1.GetPosition() < sectMeas1)
    {
      tEnd = unit.TimeAtPos(sectMeas1);
      if (tStart != tEnd)
      {
        double speed = (startPos - sectMeas1)/
            ((tEnd-tStart).ToDouble()/0.00001157);
        res.push_back(MGPSecUnit(true, secId, partNo, Down, speed,
                      Interval<Instant> (tStart, tEnd, true, false)));
      }
      tStart = tEnd;
      startPos = sectMeas1;
    }
    else
    {
      double speed = (startPos - unit.p1.GetPosition())/
            ((unit.timeInterval.end - tStart).ToDouble()/0.00001157);
      if(tStart != unit.timeInterval.end)
        res.push_back(MGPSecUnit(true, secId, partNo, Down, speed,
                      Interval<Instant> (tStart,
                                        unit.timeInterval.end,
                                        true, false)));
      startPos = unit.p1.GetPosition();
      tStart = unit.timeInterval.end;
    }
  }
}

void NotPartedSection(UGPoint unit, double sectMeas1, double sectMeas2,
                   double &startPos, int &partNo, double maxSectLength,
                   int secId, Instant &tStart, Instant &tEnd,
                   vector<MGPSecUnit> &res)
{
  if(unit.MovingDirection() == Up)
  {
    if (unit.p1.GetPosition() > sectMeas2)
    {
      tEnd = unit.TimeAtPos(sectMeas2);
      double speed = (sectMeas2 - startPos)/
            ((tEnd - tStart).ToDouble()/0.00001157);
      if (tStart != tEnd)
       res.push_back(MGPSecUnit(true, secId, partNo, Up, speed,
                    Interval<Instant> (tStart, tEnd, true, false)));
      tStart = tEnd;
      startPos = sectMeas2;
    }
    else
    {
      double speed = (unit.p1.GetPosition()-startPos)/
                      ((unit.timeInterval.end - tStart).ToDouble()/0.00001157);
      if (tStart != unit.timeInterval.end)
        res.push_back(MGPSecUnit(true, secId, partNo, Up, speed,
                              Interval<Instant> (tStart,
                                                unit.timeInterval.end,
                                                true, false)));
      tStart = unit.timeInterval.end;
      startPos = unit.p1.GetPosition();
    }
  }
  else //Moving Down
  {
    if(unit.p1.GetPosition() < sectMeas1)
    {
      tEnd = unit.TimeAtPos(sectMeas1);
      double speed = (startPos - sectMeas1)/
            ((tEnd - tStart).ToDouble()/0.00001157);
      if (tStart != tEnd)
        res.push_back(MGPSecUnit(true, secId, partNo, Down, speed,
                  Interval<Instant> (tStart, tEnd, true, false)));
      tStart = tEnd;
      startPos = sectMeas1;
    }
    else
    {
      double speed = (startPos - unit.p1.GetPosition())/
            ((unit.timeInterval.end - tStart).ToDouble()/0.00001157);
      if (tStart != unit.timeInterval.end)
        res.push_back(MGPSecUnit(true, secId, partNo, Down, speed,
                    Interval<Instant> (tStart,
                                       unit.timeInterval.end,
                                       true, false)));
      tStart = unit.timeInterval.end;
      startPos = unit.p1.GetPosition();
    }
  }
}

 void UGPoint::GetMGPSecUnits(vector<MGPSecUnit>& res,
                              double maxSectLength,
                             Network *pNet) const
 {
   res.clear();
   vector<TupleId> passedSections;
   passedSections.clear();
   GetPassedSections(pNet, passedSections);
   bool moreThanOneSection = false;
   size_t j = 0;
   if (passedSections.size() > 1) moreThanOneSection = true;
   TupleId actSectTid = passedSections[j++];
   Tuple *pActSect = pNet->GetSection(actSectTid);
   int actSectId = ((CcInt*) pActSect->GetAttribute(SECTION_SID))->GetIntval();
   double sectMeas1 =
         ((CcReal*)pActSect->GetAttribute(SECTION_MEAS1))->GetRealval();
   double sectMeas2 =
         ((CcReal*)pActSect->GetAttribute(SECTION_MEAS2))->GetRealval();
   double sectLength = sectMeas2 - sectMeas1;
   bool sectionParted = false;
   pActSect->DeleteIfAllowed();
   if (sectLength > maxSectLength) sectionParted = true;
   int partNo = 1;
   if (sectionParted) //find section partition of start gpoint
   {
     while (p0.GetPosition() > sectMeas1 + partNo * maxSectLength )
     {
       partNo++;
     }
   }
   if ((!sectionParted && !moreThanOneSection) ||
       (sectionParted && !moreThanOneSection &&
         (p1.GetPosition() >= sectMeas1 + (partNo-1) * maxSectLength &&
          p1.GetPosition() <= sectMeas1 + partNo * maxSectLength)))
   { //whole unit one mgpsecunit
     res.push_back(MGPSecUnit(true,actSectId, partNo, MovingDirection(),
                              Speed(), GetUnitTimeInterval()));
   }
   else
   {
     Instant tStart = timeInterval.start;
     Instant tEnd = tStart;
     double startPos = p0.GetPosition();
     if (!sectionParted) NotPartedSection(*this, sectMeas1, sectMeas2, startPos,
                                         partNo, maxSectLength, actSectId,
                                         tStart, tEnd, res);
     else PartedSection(*this, sectMeas1, sectMeas2, startPos,
                        partNo, maxSectLength, actSectId, tStart,
                        tEnd, res);
     while (moreThanOneSection && j < passedSections.size())
     {
       actSectTid = passedSections[j++];
       pActSect = pNet->GetSection(actSectTid);
       actSectId =
           ((CcInt*) pActSect->GetAttribute(SECTION_SID))->GetIntval();
       sectMeas1 =
             ((CcReal*)pActSect->GetAttribute(SECTION_MEAS1))->GetRealval();
       sectMeas2 =
             ((CcReal*)pActSect->GetAttribute(SECTION_MEAS2))->GetRealval();
       sectLength = sectMeas2 - sectMeas1;
       pActSect->DeleteIfAllowed();
       sectionParted = false;
       if (sectLength > maxSectLength) sectionParted = true;
       partNo = 1;
       if (sectionParted) //find section partition of start gpoint
       {
         while (startPos > sectMeas1 + partNo * maxSectLength )
         {
           partNo++;
         }
         PartedSection(*this, sectMeas1, sectMeas2, startPos, partNo,
                        maxSectLength, actSectId, tStart, tEnd, res);
       }
       else
       {
         NotPartedSection(*this, sectMeas1, sectMeas2, startPos, partNo,
                            maxSectLength, actSectId, tStart, tEnd, res);
       }
     }
  }
}




  Rectangle<3> UGPoint::BoundingBox(Network*& pNetwork)const{
    if(IsDefined()){
    RouteInterval *ri = new RouteInterval(p0.GetRouteId(), p0.GetPosition(),
                                         p1.GetPosition());
    Rectangle<2> rect = ri->BoundingBox(pNetwork);
    delete ri;
    return Rectangle<3>  (true,
                          rect.MinD(0), rect.MaxD(0),
                          rect.MinD(1), rect.MaxD(1),
                          timeInterval.start.ToDouble(),
                          timeInterval.end.ToDouble());
    }else return Rectangle<3>(false,0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
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
4 ~igpoint~

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
2. Implementation of Class ~MGPSecUnit~

*/
MGPSecUnit::MGPSecUnit():Attribute()
{
}

MGPSecUnit::MGPSecUnit(bool defined, int secId, int part, Side direct,
                       double sp, Interval<Instant> timeInterval):
    Attribute(defined),
    m_secId(secId),
    m_part(part),
    m_direct(direct),
    m_speed(sp),
    m_time(timeInterval)
{
  SetDefined(defined);
}

MGPSecUnit::MGPSecUnit( const MGPSecUnit& in_xOther):
    Attribute(in_xOther.IsDefined())
{
  SetDefined(in_xOther.IsDefined());
  if (IsDefined())
  {
    m_secId = in_xOther.GetSecId();
    m_part = in_xOther.GetPart();
    m_direct = in_xOther.GetDirect();
    m_speed = in_xOther.GetSpeed();
    m_time = in_xOther.GetTimeInterval();
  }
}

MGPSecUnit::~MGPSecUnit() {}

int MGPSecUnit::GetSecId() const
{
  return m_secId;
}

int MGPSecUnit::GetPart() const
{
  return m_part;
}

Side MGPSecUnit::GetDirect() const
{
  return m_direct;
}

double MGPSecUnit::GetSpeed() const
{
  return m_speed;
}

Interval<Instant> MGPSecUnit::GetTimeInterval() const
{
  return m_time;
}

 double MGPSecUnit::GetDurationInSeconds() const
{
  return (m_time.end - m_time.start).ToDouble()/0.00001157;
}


void MGPSecUnit::SetSecId(int secId)
{
  m_secId = secId;
}

void MGPSecUnit::SetPart(int p)
{
  m_part = p;
}

void MGPSecUnit::SetDirect(Side dir)
{
  m_direct = dir;
}

void MGPSecUnit::SetSpeed(double x)
{
  m_speed = x;
}

void MGPSecUnit::SetTimeInterval(Interval<Instant> time)
{
  m_time = time;
}

 MGPSecUnit& MGPSecUnit::operator=( const MGPSecUnit& in_xOther )
{
  m_secId = in_xOther.GetSecId();
  m_part = in_xOther.GetPart();
  m_direct = in_xOther.GetDirect();
  m_speed = in_xOther.GetSpeed();
  m_time = in_xOther.GetTimeInterval();
  SetDefined(in_xOther.IsDefined());
  return *this;
}

size_t MGPSecUnit::Sizeof() const
{
  return sizeof(MGPSecUnit);
}

size_t MGPSecUnit::HashValue() const
{
  size_t hash = m_secId + m_part + (int) m_direct + (int) m_speed +
      (int) m_time.start.ToDouble() +
      (int) m_time.end.ToDouble();
  return hash;
}

void MGPSecUnit::CopyFrom( const Attribute* right )
{
  const MGPSecUnit* gp = (const MGPSecUnit*)right;
  *this = *gp;
}

int MGPSecUnit::Compare( const Attribute* arg ) const
{
  const MGPSecUnit *p = (const MGPSecUnit*) arg;
  if (!IsDefined() && !p->IsDefined()) return 0;
  if (!IsDefined() && p->IsDefined()) return -1;
  if (IsDefined() && !p->IsDefined()) return 1;
  if (m_secId < p->GetSecId()) return -1;
  else
    if (m_secId > p->GetSecId()) return 1;
  else
    if (m_part < p->GetPart()) return -1;
  else
    if (m_part > p->GetPart()) return 1;
  else
    if (m_direct < p->GetDirect()) return -1;
  else
    if (m_direct > p->GetDirect()) return 1;
  else
    if (m_time.start < p->GetTimeInterval().start) return -1;
  else
    if (m_time.start > p->GetTimeInterval().start) return 1;
  else
    if (m_time.end < p->GetTimeInterval().end) return -1;
  else
    if (m_time.end > p->GetTimeInterval().end) return 1;
  else
    if (m_speed < p->GetSpeed()) return -1;
  else
    if (m_speed > p->GetSpeed()) return 1;
  else return 0;
}

bool MGPSecUnit::operator<(const MGPSecUnit arg) const
{
  return Compare(&arg) < 0;
}

bool MGPSecUnit::operator>(const MGPSecUnit arg) const
{
  return Compare(&arg) > 0;
}

bool MGPSecUnit::operator==(const MGPSecUnit arg)const{
  return Compare(&arg)==0;
}

bool MGPSecUnit::operator!=(const MGPSecUnit arg)const{
  return Compare(&arg)!=0;
}

bool MGPSecUnit::operator<=(const MGPSecUnit arg)const{
  return Compare(&arg)<=0;
}

bool MGPSecUnit::operator>=(const MGPSecUnit arg)const{
  return Compare(&arg)>=0;
}

bool MGPSecUnit::Adjacent( const Attribute *arg ) const
{
  return false;
}

MGPSecUnit* MGPSecUnit::Clone() const
{
  return new MGPSecUnit( *this );
}

ostream& MGPSecUnit::Print( ostream& os ) const
{
  os << "MGPSecUnit: " << m_secId
      << ", Part: " << m_part
      << ", Side: " << m_direct
      << ", Speed: " << m_speed
      << ", Timeinterval: " ;
  m_time.Print(os);
  os << endl;
  return os;
}


ListExpr MGPSecUnit::Out(ListExpr typeInfo, Word value)
{
  MGPSecUnit* msec = static_cast<MGPSecUnit*> (value.addr);
  if (msec->IsDefined())
    return nl->FiveElemList(nl->IntAtom(msec->GetSecId()),
                            nl->IntAtom(msec->GetPart()),
                            nl->IntAtom(msec->GetDirect()),
                            nl->RealAtom(msec->GetSpeed()),
                            nl->FourElemList(OutDateTime(nl->TheEmptyList(),
                                SetWord(&msec->m_time.start)),
                                OutDateTime(nl->TheEmptyList(),
                                SetWord(&msec->m_time.end)),
                                nl->BoolAtom(msec->m_time.lc),
                                nl->BoolAtom(msec->m_time.rc)));
  else return nl->SymbolAtom("undef");
}

Word MGPSecUnit::In(const ListExpr typeInfo, const ListExpr instance,
                    const int errorPos, ListExpr& errorInfo, bool& correct)
{
  NList  list(instance);
  if (list.length() == 5)
  {
    NList seclist = list.first();
    NList partlist = list.second();
    NList dirlist = list.third();
    NList speedlist = list.fourth();
    if (seclist.isInt() && dirlist.isInt()
        && partlist.isInt() && speedlist.isReal())
    {
      NList timelist = list.fifth();
      if (timelist.length() == 4)
      {
        NList stinst = timelist.first();
        NList einst = timelist.second();
        NList lclist = timelist.third();
        NList rclist = timelist.fourth();
        if (lclist.isBool() && rclist.isBool())
        {
          correct = true;
          Instant *start = (Instant*)InInstant(nl->TheEmptyList(),
                            stinst.listExpr(),
                                            errorPos,
                                            errorInfo,
                                            correct).addr;
          if(correct)
          {
            Instant *end = (Instant*)InInstant(nl->TheEmptyList(),
                            einst.listExpr(),
                                           errorPos,
                                           errorInfo,
                                           correct).addr;
            if (correct)
            {
              Word w = new MGPSecUnit(true,
                                      seclist.intval(),
                                      partlist.intval(),
                                      (Side) dirlist.intval(),
                                       nl->RealValue(speedlist.listExpr()),
                                       Interval<Instant> (*start, *end,
                                                          lclist.boolval(),
                                                          rclist.boolval()));
              return w;
            }
          }
        }
      }
    }
  }
  errorInfo = nl->Append(errorInfo, nl->StringAtom(
                         "Expected <int><int><int><real><timeinterval>."));
  correct = false;
  return SetWord(Address(0));
}

bool MGPSecUnit::CheckKind( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "mgpsecunit" ));
}

int MGPSecUnit::NumOfFLOBs()const
{
  return 0;
}

Flob* MGPSecUnit::GetFLOB(const int i)
{
  return 0;
}

void* MGPSecUnit::Cast(void* addr)
{
  return new (addr) MGPSecUnit;
}

/*
3 Type Constructor for ~mgpsecunit~

*/
struct mgpsecFunctions:ConstructorFunctions<MGPSecUnit>
{
  mgpsecFunctions()
  {
    in = MGPSecUnit::In;
    out = MGPSecUnit::Out;
    kindCheck = MGPSecUnit::CheckKind;
    cast = MGPSecUnit::Cast;
  }
};

struct mgpsecInfo:ConstructorInfo
{
  mgpsecInfo()
  {
    name = "mgpsecunit";
    signature = "-> DATA";
    typeExample = "mgpsecunit";
    listRep = "(<secId><part><dir><speed>(<timeinterval>))";
    valueExample = "(15 1 1 3.5 <timeinterval>)";
    remarks = "direction:down=0,up=1,none=2. Speed: m/s";
  }
};

mgpsecInfo mgpinfo;
mgpsecFunctions mgpfunct;
TypeConstructor mgpsecunitTC(mgpinfo, mgpfunct);

/*
5 Operators

5.0 ~mgp2mgpsecunit~

The operation ~mgp2mgpsecunit~ gets a network and a maximum section
length and a stream of ~mgpoint~. With this values it computes the

TypeMapping:

*/

ListExpr OpMgp2mgpsecunitsTypeMap(ListExpr in_xArgs)
{
  NList type(in_xArgs);
  if (type.length() == 4)
  {
    ListExpr rel = type.first().listExpr();
    ListExpr attr = type.second().listExpr();
    NList net = type.third();
    NList length = type.fourth();
    if (net.isEqual("network") && length.isEqual(CcReal::BasicType())
        && (!(nl->IsAtom(attr) && nl->AtomType(attr) != SymbolType))
        && IsRelDescription(rel))
    {
      string attrname = nl->SymbolValue(attr);
      ListExpr attrtype;
      ListExpr tupleDescr = nl->Second(rel);
      int j=listutils::findAttribute(nl->Second(tupleDescr),attrname, attrtype);
      if (j!=0 && nl->IsEqual(attrtype, "mgpoint"))
      {
        return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                                 nl->OneElemList(nl->IntAtom(j)),
                                     nl->TwoElemList(
                                         nl->SymbolAtom(Symbol::STREAM()),
                                         nl->SymbolAtom("mgpsecunit")));
      }
    }
  }
  return NList::typeError(
        "Expected <rel(tuple(..ai xi..)><ai><network><real> with xi=mgpoint.");
}

/*
Auxilliary Functions

*/

struct OpMgp2mgpsecLocalInfo
{
  OpMgp2mgpsecLocalInfo()
  {
    vmgpsecunit.clear();
    pos = 0;
    pNetwork = 0;
    iterRel = 0;
    attrIndex = 0;
    maxSectLength = numeric_limits<double>::max();
  }

  vector<MGPSecUnit> vmgpsecunit; // vector mit mgpsecunits
  size_t pos; //position im Vector
  Network *pNetwork; //networkobject
  GenericRelationIterator *iterRel; //pointer to actual tuple of rel
  int attrIndex; //attribute index of mgpoint attribut in rel
  double maxSectLength; //maximum section part length
};

/*
Value Mapping

*/

int OpMgp2mgpsecunitsValueMap(Word* args, Word& result, int message,
                              Word& local, Supplier s)
{
  OpMgp2mgpsecLocalInfo* li = 0;
  switch( message )
  {
    case OPEN:
    {
      li = new OpMgp2mgpsecLocalInfo();
      GenericRelation *rel = (GenericRelation*) args[0].addr;
      li->pNetwork = (Network*) args[2].addr;
      li->maxSectLength = ((CcReal*) args[3].addr)->GetRealval();
      li->attrIndex = ((CcInt*)args[4].addr)->GetIntval()-1;
      li->iterRel = rel->MakeScan();
      local.addr = li;
      return 0;
    }

    case REQUEST:
    {
      result = qp->ResultStorage(s);
      if (local.addr)
        li = (OpMgp2mgpsecLocalInfo*) local.addr;
      else return CANCEL;
      if (!li->vmgpsecunit.empty() && li->pos < li->vmgpsecunit.size())
      {
        result = SetWord(new MGPSecUnit(li->vmgpsecunit[li->pos++]));
        return YIELD;
      }
      else
      {
        li->vmgpsecunit.clear();
        Tuple *actTuple = li->iterRel->GetNextTuple() ;
        while (actTuple != 0)
        {
          MGPoint *m = (MGPoint*) actTuple->GetAttribute(li->attrIndex);
          if (m != 0)
          {
            m->GetMGPSecUnits(li->vmgpsecunit, li->maxSectLength, li->pNetwork);
            if (li->vmgpsecunit.size() > 0)
            {
              li->pos = 0;
              result = SetWord(new MGPSecUnit(li->vmgpsecunit[li->pos++]));
              actTuple->DeleteIfAllowed();
              return YIELD;
            }
          }
          actTuple->DeleteIfAllowed();
          actTuple = li->iterRel->GetNextTuple();
        }
        return CANCEL;
      }
    }

    case CLOSE:
    {
      if (local.addr)
      {
        li = (OpMgp2mgpsecLocalInfo*) local.addr;
        li->pNetwork = 0;
        if (li->iterRel)
          delete li->iterRel;
        li->iterRel = 0;
        li->vmgpsecunit.clear();
        delete li;
        li = 0;
        local.addr = 0;
      }
      return 0;
    }
    default:
    {
      // should not happen
      return -1;
    }
  }
}


struct mgp2mgpsecunitsInfo : OperatorInfo {

  mgp2mgpsecunitsInfo()
  {
    name      = "mgp2mgpsecunits";
    signature = "rel x attr x net x real->stream(mgpsecunit)";
    syntax    = "_ mgp2mgpsecunits[_,_,_]";
    meaning   = "Builds a stream of mgpsecunits from mgpoint.";
  }
};

/*
5.0.1 ~mgp2mgpsecunit2~

The operation ~mgp2mgpsecunit2~ gets a maximum section
length and a ~mgpoint~. With this values it computes the mgpsecunits of
the mgpoint.

TypeMapping:

*/

ListExpr OpMgp2mgpsecunits2TypeMap(ListExpr in_xArgs)
{
  NList type(in_xArgs);
  if (type.length() == 2)
  {
    NList mgp = type.first();
    NList length = type.second();
    if (mgp.isEqual("mgpoint") && length.isEqual(CcReal::BasicType()))
    {
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                             nl->SymbolAtom("mgpsecunit"));
    }
  }
  return NList::typeError( "Expected <mgpoint>, <real>.");
}

/*
Auxilliary Functions

*/

struct OpMgp2mgpsec2LocalInfo
{
  OpMgp2mgpsec2LocalInfo()
  {
    vmgpsecunit.clear();
    pos = 0;
  }

  vector<MGPSecUnit> vmgpsecunit; // vector mit mgpsecunits
  size_t pos; //position im Vector
};

/*
Value Mapping

*/

int OpMgp2mgpsecunits2ValueMap(Word* args, Word& result, int message,
                              Word& local, Supplier s)
{
  OpMgp2mgpsec2LocalInfo* li = 0;
  switch( message )
  {
    case OPEN:
    {
      li = new OpMgp2mgpsec2LocalInfo();
      MGPoint *m = (MGPoint*) args[0].addr;
      Network *pNetwork = m->GetNetwork();
      double maxSectLength = ((CcReal*) args[1].addr)->GetRealval();
      li->vmgpsecunit.clear();
      m->GetMGPSecUnits(li->vmgpsecunit, maxSectLength, pNetwork);
      li->pos = 0;
      NetworkManager::CloseNetwork(pNetwork);
      local.addr = li;
      return 0;
    }

    case REQUEST:
    {
      result = qp->ResultStorage(s);
      if (local.addr)
        li = (OpMgp2mgpsec2LocalInfo*) local.addr;
      else return CANCEL;
      if (!li->vmgpsecunit.empty() && li->pos < li->vmgpsecunit.size())
      {
        result = SetWord(new MGPSecUnit(li->vmgpsecunit[li->pos++]));
        return YIELD;
      }
      else return CANCEL;
    }

    case CLOSE:
    {
      if (local.addr)
      {
        li = (OpMgp2mgpsec2LocalInfo*) local.addr;
        li->vmgpsecunit.clear();
        delete li;
        li = 0;
        local.addr = 0;
      }
      return 0;
    }
    default:
    {
      // should not happen
      return -1;
    }
  }
}


struct mgp2mgpsecunits2Info : OperatorInfo {

  mgp2mgpsecunits2Info()
  {
    name      = "mgp2mgpsecunits2";
    signature = "mgpoint x real -> stream(mgpsecunit)";
    syntax    = "mgp2mgpsecunits2 (_ , _ )";
    meaning   = "Builds a stream of mgpsecunits from mgpoint.";
  }
};

/*
5.0.2 ~mgp2mgpsecunit3~

The operation ~mgp2mgpsecunit3~ gets a maximum section
length and a ~stream~ of ~mgpoint~. With this values it computes a
~stream~ of ~mgpsecunit~s for this mgpoints.

TypeMapping:

*/

ListExpr OpMgp2mgpsecunits3TypeMap(ListExpr in_xArgs)
{
  NList type(in_xArgs);
  if (type.length() == 2)
  {
    NList stream = type.first();
    NList mgp("mgpoint");
    NList partlength = type.second();
    if (stream.length() == 2 && stream.checkStream(mgp) &&
        partlength.isEqual(CcReal::BasicType()))
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                             nl->SymbolAtom("mgpsecunit"));
  }
  return NList::typeError( "Expected ((stream mgpoint) real).");
}

/*
Auxilliary Functions

*/

struct OpMgp2mgpsec3LocalInfo
{
  OpMgp2mgpsec3LocalInfo()
  {
    vmgpsecunit.clear();
    pos = 0;
    maxLength = 0.0;
    pNetwork = 0;
  }

  vector<MGPSecUnit> vmgpsecunit; // vector mit mgpsecunits
  size_t pos; //position im Vector
  double maxLength; //maxLength of section part
  Network* pNetwork; //network the mgpoint belong to
};

/*
Value Mapping

*/

int OpMgp2mgpsecunits3ValueMap(Word* args, Word& result, int message,
                               Word& local, Supplier s)
{
  OpMgp2mgpsec3LocalInfo* li = 0;
  switch( message )
  {
    case OPEN:
    {
      li = new OpMgp2mgpsec3LocalInfo();
      li->maxLength = ((CcReal*) args[1].addr)->GetRealval();
      Word curAddr;
      qp->Open(args[0].addr);
      qp->Request(args[0].addr,curAddr);
      if (qp->Received(args[0].addr))
      {
        MGPoint *m = (MGPoint*) curAddr.addr;
        li->pNetwork = m->GetNetwork();
        li->vmgpsecunit.clear();
        m->GetMGPSecUnits(li->vmgpsecunit, li->maxLength, li->pNetwork);
        li->pos = 0;
        local.addr = li;
        m->DeleteIfAllowed();
        return 0;
      }
      else
      {
        qp->Close(args[0].addr);
        delete li;
        li = 0;
        return CANCEL;
      }
    }

    case REQUEST:
    {
      result = qp->ResultStorage(s);
      if (local.addr)
        li = (OpMgp2mgpsec3LocalInfo*) local.addr;
      else return CANCEL;
      if (!li->vmgpsecunit.empty() && li->pos < li->vmgpsecunit.size())
      {
        result = SetWord(new MGPSecUnit(li->vmgpsecunit[li->pos++]));
        return YIELD;
      }
      else
      {
        Word curAddr;
        qp->Request(args[0].addr, curAddr);
        if (qp->Received(args[0].addr))
        {
          MGPoint *m = (MGPoint*) curAddr.addr;
          li->vmgpsecunit.clear();
          m->GetMGPSecUnits(li->vmgpsecunit, li->maxLength, li->pNetwork);
          li->pos = 0;
          m->DeleteIfAllowed();
          result = SetWord (new MGPSecUnit(li->vmgpsecunit[li->pos++]));
          return YIELD;
        }
        else
        {
          qp->Close(args[0].addr);
          NetworkManager::CloseNetwork(li->pNetwork);
          li->pNetwork = 0;
          li->vmgpsecunit.clear();
          delete li;
          li = 0;
          local.addr = 0;
          return CANCEL;
        }
      }
    }

    case CLOSE:
    {
      qp->Close(args[0].addr);
      if (local.addr)
      {
        li = (OpMgp2mgpsec3LocalInfo*) local.addr;
        NetworkManager::CloseNetwork(li->pNetwork);
        li->pNetwork = 0;
        li->vmgpsecunit.clear();
        delete li;
        li = 0;
        local.addr = 0;
      }
      return 0;
    }
    default:
    {
      // should not happen
      return -1;
    }
  }
}


struct mgp2mgpsecunits3Info : OperatorInfo {

  mgp2mgpsecunits3Info()
  {
    name      = "mgp2mgpsecunits3";
    signature = "stream(mgpoint) x real -> stream(mgpsecunit)";
    syntax    = "_ mgp2mgpsecunits3 ( _ ) ";
    meaning   = "Builds a stream of mgpsecunits from a stream of mgpoint.";
  }
};
/*


5.1 Operator ~mpoint2mgpoint~

Translates a spatial ~MPoint~ into a network based ~MGPoint~.

*/


ListExpr OpMPoint2MGPointTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 2 ){
    sendMessages("Expects a list of length 2.");
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }
  ListExpr xNetworkIdDesc = nl->First(in_xArgs);
  ListExpr xMPointDesc = nl->Second(in_xArgs);

  if( (!nl->IsAtom(xNetworkIdDesc)) ||
      !nl->IsEqual(xNetworkIdDesc, "network"))
  {
    sendMessages("Expected network as first argument.");
    return (nl->SymbolAtom(Symbol::TYPEERROR()));
  }

  if( (!nl->IsAtom( xMPointDesc )) ||
      nl->AtomType( xMPointDesc ) != SymbolType ||
      nl->SymbolValue( xMPointDesc ) != MPoint::BasicType() )
  {
    sendMessages("Expected mpoint as second argument.");
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
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
int OpMPoint2MGPointValueMappingNeu(Word* args,
                                 Word& result,
                                 int message,
                                 Word& local,
                                 Supplier in_xSupplier)
{
  /*
  Initialize Result. Load and Check Arguments.

  */
  result = qp->ResultStorage(in_xSupplier);
  MGPoint* res = static_cast<MGPoint*>(result.addr);
  res->Clear();
  Network *pNetwork = (Network*) args[0].addr;
  if (pNetwork == 0 || !pNetwork->IsDefined())
  {
    res->SetDefined(false);
    cout << "Network not defined!" << endl;
    return 0;
  }
  MPoint *pMPoint = (MPoint*)args[1].addr;
  if (pMPoint == 0 || !pMPoint->IsDefined())
  {
    res->SetDefined(false);
    cout << "MPoint is not defined!" << endl;
    return 0;
  }
  if (pMPoint->GetNoComponents() == 0)
  {
    cout << "MPoint is empty!" << endl;
    res->SetDefined(false);
    return 0;
  }
  /*
  Use Startunit to initialize values

  */
  int iNetworkId = pNetwork->GetId();
  UPoint pUPoint;
  int i = 0;
  pMPoint->Get(i,pUPoint);
  RouteInterval *ri = pNetwork->FindInterval(pUPoint.p0, pUPoint.p1);
  if (ri == 0 || ri->GetRouteId() == numeric_limits<int>::max())
  {
    cout << "First Interval not found!"<< endl;
    res->SetDefined(false);
    return 0;
  }
  res->SetDefined(true);
  res->StartBulkLoad();
  SimpleLine pActRouteCurve = pNetwork->GetRouteCurve(ri->GetRouteId());
  bool bDual = pNetwork->GetDual(ri->GetRouteId());
  bool bMovingUp = true;
  if (ri->GetStartPos() > ri->GetEndPos()) bMovingUp = false;
  Side side = None;
  if (bDual && bMovingUp) side = Up;
  else
    if (bDual && !bMovingUp) side = Down;
    else side = None;
  UGPoint aktUGPoint = UGPoint(Interval<Instant> (
                               pUPoint.timeInterval.start,
                               pUPoint.timeInterval.end,
                               pUPoint.timeInterval.lc,
                               pUPoint.timeInterval.rc),
                               iNetworkId,
                               ri->GetRouteId(),
                               side,
                               ri->GetStartPos(),
                               ri->GetEndPos());
  RITree *riTree = 0;
  if (ri->GetStartPos() < ri->GetEndPos())
    riTree = new RITree(ri->GetRouteId(), ri->GetStartPos(), ri->GetEndPos());
  else
    riTree = new RITree(ri->GetRouteId(), ri->GetEndPos(), ri->GetStartPos());
  delete ri;
  ri = 0;
  /*
  Continue with translation of all other units.

  */
  while (++i < pMPoint->GetNoComponents())
  {
    pMPoint->Get(i,pUPoint);
    double dNewEndPos;
    if (checkPointN(pActRouteCurve, pUPoint.p1, true, dNewEndPos))
    {
      /*
      End Found on same route like last ~ugpoint~

      */
      if (((bMovingUp && aktUGPoint.GetUnitEndPos() <= dNewEndPos) ||
            (!bMovingUp && aktUGPoint.GetUnitEndPos() >= dNewEndPos)) &&
          (fabs(aktUGPoint.Speed() -
            ((fabs(aktUGPoint.GetUnitEndPos() - dNewEndPos))/
             ((pUPoint.timeInterval.end -
            pUPoint.timeInterval.start).ToDouble()/0.00001157))) < 0.01))
      {
        /*
        0.00001157 =  miliseconds to seconds. Compare meter per second.

        Unit moves same direction and speed like previous units.
        Extend akt ~ugpoint~ to include unit values.

        */
        aktUGPoint.SetUnitEndPos(dNewEndPos);
        aktUGPoint.SetUnitEndTime(pUPoint.timeInterval.end);
      }
      else
      {
        /*
        Speed changed save akt ~ugpoint~ and start new ~ugpoint~
        for actual ~upoint~ values

        */
        res->Add(aktUGPoint);
        riTree->InsertUnit(aktUGPoint.GetUnitRid(),
                          aktUGPoint.GetUnitStartPos(),
                          aktUGPoint.GetUnitEndPos());
        aktUGPoint.SetUnitStartTime(pUPoint.timeInterval.start);
        aktUGPoint.SetUnitEndTime(pUPoint.timeInterval.end);
        aktUGPoint.SetUnitStartPos(aktUGPoint.GetUnitEndPos());
        aktUGPoint.SetUnitEndPos(dNewEndPos);
        if (aktUGPoint.GetUnitStartPos() > aktUGPoint.GetUnitEndPos())
          bMovingUp = false;
        else bMovingUp = true;
        if (bDual && bMovingUp) side = Up;
        else
          if(bDual && !bMovingUp) side = Down;
          else side = None;
        aktUGPoint.SetUnitSide(side);
      }
    }
    else
    {
      /*
      Route must have been changed. Save akt ~ugpoint~ and compute new ~ugpoint~
      for actual ~upoint~ values

      */
      res->Add(aktUGPoint);
      riTree->InsertUnit(aktUGPoint.GetUnitRid(),
                         aktUGPoint.GetUnitStartPos(),
                         aktUGPoint.GetUnitEndPos());
      //TODO:Remove simple FindInterval against adjacent section version
      ri = pNetwork->FindInterval(pUPoint.p0, pUPoint.p1);
      if (ri == 0 || ri->GetRouteId() == numeric_limits<int>::max())
      {
        /*
        MPoint lost Network!

        */
        Instant tstart = pUPoint.timeInterval.start;
        GPoint start = aktUGPoint.p1;
        while (ri == 0 && ++i < pMPoint->GetNoComponents())
        {
          /*
          Find first unit of Mpoint back on Network.

          */
          pMPoint->Get(i,pUPoint);
          ri = pNetwork->FindInterval(pUPoint.p0, pUPoint.p1);

          if (ri != 0)
          {
            /*
            Calculate shortest path between last known network position and
            new network position. Fill in ugpoint units for the shortest path
            route intervals. For the time interval between network lost and
            network found again.

            */
            Instant tend = pUPoint.timeInterval.start;
            Side s = None;
            if (ri->GetStartPos() > ri->GetEndPos()) bMovingUp = false;
            else bMovingUp = true;
            if (bDual && bMovingUp) s = Up;
            else
              if (bDual && !bMovingUp) s = Down;
              else s = None;
            pActRouteCurve = pNetwork->GetRouteCurve(ri->GetRouteId());
            bDual = pNetwork->GetDual(ri->GetRouteId());
            GPoint end = GPoint(true, iNetworkId, ri->GetRouteId(),
                                ri->GetStartPos(), s);
            GLine *gl = new GLine(0);
            if (!start.ShortestPath(&end,gl))
            {
              cout << "One unit couldn't be mapped to the network." << endl;
              delete ri;
              ri = 0;
            }
            else
            {
              for (int k = 0; k < gl->NoOfComponents(); k++)
              {
                RouteInterval gri;
                gl->Get(k,gri);
                Instant tpos =(tend - tstart) *
                                (fabs(gri.GetEndPos()-gri.GetStartPos())/
                                  gl->GetLength()) +
                              tstart;
                if (gri.GetRouteId() == end.GetRouteId() &&
                    gri.GetEndPos() == end.GetPosition()) tpos = tend;
                Side s = None;
                if (ri->GetStartPos() > ri->GetEndPos()) bMovingUp = false;
                else bMovingUp = true;
                if (bDual && gri.GetStartPos() <= gri.GetEndPos()) s = Up;
                else
                  if (bDual && gri.GetStartPos() > gri.GetEndPos()) s = Down;
                  else s = None;
                res->Add(UGPoint(Interval<Instant> (tstart, tpos, true, false),
                                iNetworkId,
                                gri.GetRouteId(),
                                s,
                                gri.GetStartPos(),
                                gri.GetEndPos()));
                riTree->InsertUnit(gri.GetRouteId(),
                                  gri.GetStartPos(),
                                  gri.GetEndPos());
                tstart = tpos;
              }
            }
            gl->DeleteIfAllowed();
          }
        }
        if (ri == 0)
        {
          cout << "MPoint lost Network! Translation stopped!" << endl;
          res->EndBulkLoad(true);
          riTree->TreeToDbArray(&(res->m_trajectory));
          res->SetTrajectoryDefined(true);
          res->m_trajectory.TrimToSize();
          res->SetBoundingBox(pMPoint->BoundingBox());
          riTree->RemoveTree();
          return 0;
        }
      }
      aktUGPoint.SetUnitStartTime(pUPoint.timeInterval.start);
      aktUGPoint.SetUnitEndTime(pUPoint.timeInterval.end);
      pActRouteCurve = pNetwork->GetRouteCurve(ri->GetRouteId());
      bDual = pNetwork->GetDual(ri->GetRouteId());
      aktUGPoint.SetUnitRid(ri->GetRouteId());
      aktUGPoint.SetUnitStartPos(ri->GetStartPos());
      aktUGPoint.SetUnitEndPos(ri->GetEndPos());
      if (ri->GetStartPos() > ri->GetEndPos()) bMovingUp = false;
      else bMovingUp = true;
      if (bDual && bMovingUp) side = Up;
      else
        if (bDual && !bMovingUp) side = Down;
        else side = None;
      aktUGPoint.SetUnitSide(side);
      delete ri;
      ri = 0;
    }
  }
  /*
  Finish mgpoint computation

  */
  res->Add(aktUGPoint);
  riTree->InsertUnit(aktUGPoint.GetUnitRid(), aktUGPoint.GetUnitStartPos(),
                     aktUGPoint.GetUnitEndPos());
  res->EndBulkLoad(true);
  riTree->TreeToDbArray(&(res->m_trajectory));
  res->SetTrajectoryDefined(true);
  res->m_trajectory.TrimToSize();
  res->SetBoundingBox(pMPoint->BoundingBox());
  /*
  Clean Memory.

  */
  riTree->RemoveTree();
  return 0;
}

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
          OpMPoint2MGPointValueMappingNeu,  // value mapping
          Operator::SimpleSelect,          // trivial selection function
          OpMPoint2MGPointTypeMap        // type mapping
);

/*
5.2 Operator ~passes~

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
        return (nl->SymbolAtom(CcBool::BasicType()));
      }
    }
  }
  return (nl->SymbolAtom( Symbol::TYPEERROR() ));
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
    cerr << "MGPoint not Defined" << endl;
    pPasses->Set(false, false);
    return 0;
  }
  GPoint* pGPoint = (GPoint*)args[1].addr;
  if(pGPoint == NULL || !pGPoint->IsDefined()) {
    cerr << "GPoint not Defined" << endl;
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
5.3 Operator ~simplify~

Reduces units of a ~MGPoint~ by concatenation if the speed difference is
smaller than a given value.

*/

ListExpr OpSimplifyTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 2 )
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));

  ListExpr xMPointDesc = nl->First(in_xArgs);
  ListExpr xEpsilonDesc = nl->Second(in_xArgs);

  if( (!nl->IsAtom( xMPointDesc )) ||
      nl->AtomType( xMPointDesc ) != SymbolType ||
      nl->SymbolValue( xMPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }

  if( (!nl->IsAtom( xEpsilonDesc)) ||
      nl->AtomType( xEpsilonDesc ) != SymbolType ||
      nl->SymbolValue( xEpsilonDesc ) != CcReal::BasicType() )
  {
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
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
    pMGPoint->SetDefined(false);
    return 0;
  }
  pMGPoint->Simplify(dEpsilon, pMGPointSimplified);
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
5.4 Operator ~at~

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
  return (nl->SymbolAtom( Symbol::TYPEERROR() ));
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
    cerr << "MGPoint not Defined" << endl;
    pResult->SetDefined(false);
    return 0;
  }
  GPoint* pGPoint = (GPoint*)args[1].addr;
  if(pGPoint == NULL || !pGPoint->IsDefined()) {
    cerr << "GPoint not Defined" << endl;
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
5.5 Operator ~atinstant~

Restricts the ~MGPoint~ to a given time instant.

*/

ListExpr OpAtinstantTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 2 )
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);
  ListExpr xInstant = nl->Second(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }

  if (!nl->IsEqual( xInstant, Instant::BasicType() ))
  {
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }

  return nl->SymbolAtom("igpoint");
}

int OpAtinstantValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  Intime<GPoint>* pIGPres =
      (Intime<GPoint>*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pIGPres );
  MGPoint* pMGP = (MGPoint*)args[0].addr;
  if(pMGP == NULL ||!pMGP->IsDefined()) {
    cerr << "MGPoint does not exist." << endl;
    pIGPres->SetDefined(false);
    return 0;
  }

  Instant* per = (Instant*)args[1].addr;
  if(per == NULL || !per->IsDefined()) {
    cerr << "Periods are not defined." << endl;
    pIGPres->SetDefined(false);
    return 0;
  }
  if (pMGP->GetNoComponents() < 1) {
    pIGPres->SetDefined(true);
    return 0;
  }
  pMGP->Atinstant(per, pIGPres);
  return 0;
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
                OpAtinstantValueMapping,
                Operator::SimpleSelect,
                OpAtinstantTypeMap );


/*
5.6 Operator ~atperiods~

Restricts a ~MGPoint~ to the given periods.

*/

ListExpr OpAtperiodsTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 2 )
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);
  ListExpr xPeriods = nl->Second(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }

  if (!nl->IsEqual( xPeriods, Periods::BasicType() ))
  {
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
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
5.7 Operator ~deftime~

Returns the deftime of a ~MGPoint~ respectively a ~ugpoint~ as ~periods~ value.

*/

ListExpr OpDeftimeTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (nl->IsAtom( xMGPointDesc )) &&
      nl->AtomType( xMGPointDesc ) == SymbolType &&
      (nl->SymbolValue( xMGPointDesc ) == "mgpoint" ||
      nl->SymbolValue( xMGPointDesc ) == "ugpoint"))
  {
    return (nl->SymbolAtom( Periods::BasicType() ));
  }

  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

int OpDeftime_ugp(Word* args, Word& result, int message,
                  Word& local, Supplier in_xSupplier) {
  Periods* pResult = (Periods*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pResult );
  UGPoint* pUGP = (UGPoint*) args[0].addr;
  pResult->Clear();
  if (!pUGP->IsDefined()) {
    pResult->SetDefined(false);
    return 0;
  }
  pUGP->Deftime(*pResult);
  return 0;
}

int OpDeftime_mgp(Word* args, Word& result, int message, Word& local,
                  Supplier in_xSupplier) {
  Periods* res = (Periods*) qp->ResultStorage(in_xSupplier).addr;
  result = SetWord(res);
  MGPoint* pMGP = (MGPoint*) args[0].addr;
  if (!pMGP->IsDefined() || pMGP->GetNoComponents() == 0) {
    res->Clear();
    res->SetDefined(false);
    return 0;
  } else {
    pMGP->Deftime(*res);
    return 0;
  }
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
5.8 Operator ~final~

Returns the final time and position of the ~MGPoint~ as ~IGPoint~

TypeMapping see operator ~final~.

*/

ListExpr OpFinalInitialTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
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
5.9 Operator ~initial~

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
5.10 Operator ~inside~

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
        return (nl->SymbolAtom(MBool::BasicType()));
    }
  }
  return (nl->SymbolAtom( Symbol::TYPEERROR() ));
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
5.11 Operator ~inst~

Returns the time instant of the ~IGPoint~

*/

ListExpr OpInstTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));

  ListExpr xIGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xIGPointDesc )) ||
      nl->AtomType( xIGPointDesc ) != SymbolType ||
      nl->SymbolValue( xIGPointDesc ) != "igpoint" )
  {
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }

  return nl->SymbolAtom( Instant::BasicType() );
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
5.12 Operator ~intersection~

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
  return (nl->SymbolAtom( Symbol::TYPEERROR() ));
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
  if(pMGPoint1 == NULL || !pMGPoint1->IsDefined() ||
     pMGPoint1->GetNoComponents() < 1) {
    cerr << "First mgpoint does not exist." << endl;
    pResult->SetDefined(false);
    return 0;
  }
  MGPoint* pMGPoint2 = (MGPoint*)args[1].addr;
  if(pMGPoint2 == NULL || !pMGPoint2->IsDefined() ||
     pMGPoint2->GetNoComponents() < 1 ) {
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
5.12 Operator ~intersects~

Returns true if a intersection of the two ~MGPoint~ exists.

*/

ListExpr OpIntersectsTypeMapping(ListExpr in_xArgs)
{
  ListExpr arg1, arg2;
  if( nl->ListLength(in_xArgs) == 2 ){
    arg1 = nl->First(in_xArgs);
    arg2 = nl->Second(in_xArgs);
    if (nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
        nl->SymbolValue(arg1) == "mgpoint" && nl->IsAtom(arg2) &&
        nl->AtomType(arg2) == SymbolType &&
        nl->SymbolValue(arg2) == "mgpoint"){
        return (nl->SymbolAtom(CcBool::BasicType()));
    }
  }
  return (nl->SymbolAtom( Symbol::TYPEERROR() ));
}

int OpIntersectsValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  // Get (empty) return value
  CcBool* pResult = (CcBool*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pResult);
  // Get input values
  MGPoint* pMGPoint1 = (MGPoint*)args[0].addr;
  if(pMGPoint1 == NULL || !pMGPoint1->IsDefined() ||
     pMGPoint1->GetNoComponents() < 1) {
    cerr << "First mgpoint does not exist." << endl;
    pResult->SetDefined(false);
    return 0;
  }
  MGPoint* pMGPoint2 = (MGPoint*)args[1].addr;
  if(pMGPoint2 == NULL || !pMGPoint2->IsDefined() ||
     pMGPoint2->GetNoComponents() < 1 ) {
    sendMessages("Second mgpoint does not exist.");
    pResult->SetDefined(false);
    return 0;
  }
  pResult->Set(true,pMGPoint1->Intersects(pMGPoint2));
  return 0;
}

const string OpIntersectsSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint x mgpoint -> bool" "</text--->"
  "<text>mgpoint intersects mgpoint</text--->"
  "<text>Returns true if the mgpoint meet at any point, false"
  " otherwise.</text--->"
  "<text>mgpoint intersects mgpoint</text--->"
  ") )";

Operator tempnetintersects("intersects",
                OpIntersectsSpec,
                OpIntersectsValueMapping,
                Operator::SimpleSelect,
                OpIntersectsTypeMapping );

/*
5.13 Operator ~isempty~

Returns true if the ~MGPoint~ has no units.

*/

ListExpr OpIsEmptyTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }

  return nl->SymbolAtom(CcBool::BasicType());
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
5.14 Operator ~length~

Returns the length of the trip of the ~MGPoint~.

*/

ListExpr OpLengthTypeMapping(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||(
      nl->SymbolValue( xMGPointDesc ) != "mgpoint" &&
      nl->SymbolValue (xMGPointDesc) != "ugpoint"))
  {
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }

  return nl->SymbolAtom( CcReal::BasicType() );
}

int OpLength_mgp(Word* args,
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

int OpLength_ugp(Word* args,
                 Word& result,
                 int message,
                 Word& local,
                 Supplier in_xSupplier)
{
  result = qp->ResultStorage(in_xSupplier);
  CcReal* pResult = (CcReal*) result.addr;
  // Get input value
  UGPoint* pUGPoint = (UGPoint*)args[0].addr;
  if(pUGPoint == NULL || !pUGPoint->IsDefined()) {
    cmsg.inFunError("UGPoint does not exist.");
    pResult->Set(false, 0.0);
    return 0;
     }
     pResult-> Set(true, pUGPoint->Length());
     return 1;
}

const string OpTLengthSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint -> real, ugpoint -> real" "</text--->"
  "<text>length(xgpoint)</text--->"
  "<text>Returns the length of the path passed by a m(u)gpoint.</text--->"
  "<text>length(xgpoint)</text--->"
  ") )";

int OpLengthSelect(ListExpr args){
  ListExpr arg1 = nl->First( args );

  if ( nl->SymbolValue(arg1) == "mgpoint")
    return 0;
  if ( nl->SymbolValue(arg1) == "ugpoint")
    return 1;
  return -1; // This point should never be reached
};

ValueMapping OpLengthValueMap[] = {
  OpLength_mgp,
  OpLength_ugp
};

Operator tempnetlength("length",
                    OpTLengthSpec,
                    2,
                    OpLengthValueMap,
                    OpLengthSelect,
                    OpLengthTypeMapping );


/*
5.15 Operator ~no\_components~

Returns the number of units of a ~MGPoint~.

*/

ListExpr OpNoCompTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }

  return nl->SymbolAtom(CcInt::BasicType());
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
5.16 Operator ~present~

Returns true if the ~MGPoint~ has at least almost one unit with the time instant
respectively one of the periods.

*/

ListExpr OpPresentTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) == 2 ) {
    ListExpr arg1 = nl->First(in_xArgs);
    ListExpr arg2 = nl->Second(in_xArgs);
    if( nl->IsEqual(arg1,"mgpoint") && (
      nl->IsEqual(arg2,Instant::BasicType()) ||
      nl->IsEqual(arg2, Periods::BasicType())))
      return (nl->SymbolAtom( CcBool::BasicType() ));
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
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
       nl->SymbolValue( arg2) == Periods::BasicType() )
    return 0;
  if ( nl->SymbolValue(arg1) == "mgpoint" &&
       nl->SymbolValue( arg2) == Instant::BasicType())
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
5.17 Operator ~val~

Returns the ~GPoint~ value of a ~IGPoint~

*/

ListExpr OpValTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));

  ListExpr xIGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xIGPointDesc )) ||
      nl->AtomType( xIGPointDesc ) != SymbolType ||
      nl->SymbolValue( xIGPointDesc ) != "igpoint" )
  {
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
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
5.18 Operator ~trajectory~

Returns the sorted ~GLine~ passed by a ~MGPoint~.

*/

ListExpr OpTrajectoryTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
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
  qp->ChangeResultStorage(in_xSupplier, result);
  pGLine->DeleteIfAllowed();
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
5.19 Operator ~units~

Returns the stream of ~UGPoint~ from the given ~MGPoint~.

*/

ListExpr OpUnitsTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));

  ListExpr xMPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMPointDesc )) ||
      nl->AtomType( xMPointDesc ) != SymbolType ||
      nl->SymbolValue( xMPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }

  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
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
5.20 Operator ~unitendpos~

Returns the end position of the ~UGPoint~ as ~real~.

TypeMapping for ~unitendpos~, ~unitstartpos~, ~unitendtime~, ~unitstarttime~
and ~unitrid~.

*/

ListExpr OpUnitPosTimeTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "ugpoint" )
  {
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }
  return nl->SymbolAtom(CcReal::BasicType());
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
5.21 Operator ~unitstartpos~

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
5.22 Operator ~unitendtime~

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
    pNumber->Set(true, (double) pUGP->GetDoubleUnitEndTime());
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
5.23 Operator ~unitstarttime~

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
    pNumber->Set(true, (double) pUGP->GetDoubleUnitStartTime());
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
5.24 Operator ~unitrid~

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
5.25 Operator ~unitbox~

Returns the bounding box of the ~ugpoint~ as rectangle of dimension 3. with
rid, rid, min(p0.pos. p1.pos), max(p0.pos, p1.pos), timeInterval.start,
timeInterval.end.

*/

ListExpr OpUnitBoxTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "ugpoint" )
  {
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }
  return nl->SymbolAtom(Rectangle<3>::BasicType());
}

int OpUnitBoxValueMapping( Word* args, Word& result, int message,
                             Word& local, Supplier s ){
  result = qp->ResultStorage( s );
  Rectangle<3>* box = static_cast<Rectangle<3>* >(result.addr);
  UGPoint* arg = static_cast<UGPoint*>(args[0].addr);
  if(!arg->IsDefined()) box->SetDefined(false);
  else (*box) = arg->NetBoundingBox3d();
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
5.26 Operator ~unitbox~

Returns the bounding box of the ~ugpoint~ as rectangle of dimension 2. with
rid, rid, min(p0.pos. p1.pos), max(p0.pos, p1.pos).

*/

ListExpr OpUnitBox2TypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "ugpoint" )
  {
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }
  return nl->SymbolAtom(Rectangle<2>::BasicType());
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
5.27 Operator ~unitboundingbox~

Returns the spatialtemporal bounding box of the ~ugpoint~.

*/

ListExpr OpUnitBoundingBoxTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "ugpoint" )
  {
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }
  return nl->SymbolAtom(Rectangle<3>::BasicType());
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
5.28 Operator ~mgpointboundingbox~

Returns the spatialtemporal bounding box of the ~ugpoint~.

*/

ListExpr OpMGPointBoundingBoxTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }
  return nl->SymbolAtom(Rectangle<3>::BasicType());
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
5.29 Operator ~mgpoint2mpoint~

Returns the ~mpoint~ value of the given ~MGPoint~.

*/

ListExpr OpMGPoint2MPointTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 ){
    sendMessages("Expects a list of length 1.");
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }

  ListExpr xsource = nl->First(in_xArgs);

  if( (!nl->IsAtom(xsource)) ||
      !nl->IsEqual(xsource, "mgpoint"))
  {
    sendMessages("Element must be of type mgpoint.");
    return (nl->SymbolAtom(Symbol::TYPEERROR()));
  }
  return nl->SymbolAtom( MPoint::BasicType() );
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
5.30 Operator ~distance~

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
      return (nl->SymbolAtom(MReal::BasicType()));
    }
  }
  return (nl->SymbolAtom( Symbol::TYPEERROR() ));
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
//    pMGPoint1->DistanceE(pMGPoint2, pResult);
//  pMGPoint1->DistanceN(pMGPoint2, pResult);
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
5.31 Operator ~union~

Returns a ~MGPoint~ which is the ~union~ of the two given MGPoints if possible,
undefined elsewhere.

*/


ListExpr OpUnionTypeMap(ListExpr in_xArgs)
{
  ListExpr arg1, arg2;
  if ( nl->ListLength(  in_xArgs ) == 2 )
  {
    arg1 = nl->First( in_xArgs );
    arg2 = nl->Second( in_xArgs );

    if ( nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
         nl->SymbolValue(arg1) == "mgpoint" && nl->IsAtom(arg2) &&
         nl->AtomType(arg2) == SymbolType &&
         nl->SymbolValue(arg2) == "mgpoint"){
        return (nl->SymbolAtom("mgpoint"));
    }
  }
  return (nl->SymbolAtom( Symbol::TYPEERROR() ));
}

int OpUnionValueMapping(Word* args,
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
  MGPoint *pMGP1 = (MGPoint*)args[0].addr;
  if(pMGP1 == NULL) {
    cerr << "MGPoint1 not Defined" << endl;
    pResult->SetDefined(false);
    return 0;
  }
  MGPoint *pMGP2 = (MGPoint*)args[1].addr;
  if(pMGP2 == NULL) {
    cerr << "MGPoint2 not Defined" << endl;
    pResult->SetDefined(false);
    return 0;
  }
  pMGP1->Union(pMGP2, pResult);
  return 0;
};

const string OpUnionSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint x mgpoint -> mgpoint" "</text--->"
  "<text> _ union _ </text--->"
  "<text>Returns the union of the two given mgpoint if possible,"
  "  undef elsewhere.</text--->"
  "<text> X_MGPOINTA union X_MGPOINTB</text--->"
  ") )";

Operator tempnetunion("union",
                OpUnionSpec,
                OpUnionValueMapping,
                Operator::SimpleSelect,
                OpUnionTypeMap );

/*
5.32 Operator ~endunitinst~

Returns the end time instant of a ~UGPoint~ as ~Instant~

*/

ListExpr OpEndStartunitinstTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
        nl->AtomType( xMGPointDesc ) != SymbolType ||
        nl->SymbolValue( xMGPointDesc ) != "ugpoint")
  {
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }
  return nl->SymbolAtom(Instant::BasicType());
}

int OpEndunitinstValueMapping(Word* args,
                        Word& result,
                        int message,
                        Word& local,
                        Supplier in_xSupplier)
{
  // Get (empty) return value
  Instant* pResult = (Instant*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pResult);
  // Get input values
  UGPoint *pUGP = (UGPoint*)args[0].addr;
  if(pUGP == NULL || !pUGP->IsDefined()) {
    cerr << "UGPoint not Defined" << endl;
    pResult->SetDefined(false);
    return 0;
  }
  *pResult = pUGP->timeInterval.end;
  return 0;
};

const string OpEndunitinstSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>ugpoint -> instant" "</text--->"
    "<text>endunitinst (_)</text--->"
    "<text>Computes the final time instant of a ugpoint.</text--->"
    "<text>endunitinst(UGPOINT)</text--->"
    ") )";

Operator tempnetendunitinst("endunitinst",
                      OpEndunitinstSpec,
                      OpEndunitinstValueMapping,
                      Operator::SimpleSelect,
                      OpEndStartunitinstTypeMap);


/*
5.33 Operator ~startunitinst~

Returns the start time of the ~UGPoint~ as ~Instant~.

TypeMapping see 5.32 ~OpEndStartunitinstTypeMap~

*/

int OpStartunitinstValueMapping(Word* args,
                              Word& result,
                              int message,
                              Word& local,
                              Supplier in_xSupplier)
{
  // Get (empty) return value
  Instant* pResult = (Instant*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pResult);
  // Get input values
  UGPoint *pUGP = (UGPoint*)args[0].addr;
  if(pUGP == NULL || !pUGP->IsDefined()) {
    cerr << "UGPoint not Defined" << endl;
    pResult->SetDefined(false);
    return 0;
  }
  *pResult = pUGP->timeInterval.start;
  return 0;
};

const string OpStartunitinstSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>ugpoint -> instant" "</text--->"
    "<text> startunitinst (_)</text--->"
    "<text>Computes the start time instant of a ugpoint.</text--->"
    "<text>startunitinst (UGPOINT)</text--->"
    ") )";

Operator tempnetstartunitinst("startunitinst",
                        OpStartunitinstSpec,
                        OpStartunitinstValueMapping,
                        Operator::SimpleSelect,
                        OpEndStartunitinstTypeMap);

/*
5.34 Operator ~ugpoint2mgpoint~

Builds a ~MGPoint~ from a single ~UGPoint~

*/

ListExpr OpUgpoint2mgpointTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
        nl->AtomType( xMGPointDesc ) != SymbolType ||
        nl->SymbolValue( xMGPointDesc ) != "ugpoint")
  {
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }
  return nl->SymbolAtom("mgpoint");
}

int OpUgpoint2mgpointValueMapping(Word* args,
                              Word& result,
                              int message,
                              Word& local,
                              Supplier in_xSupplier)
{
  // Get (empty) return value
  MGPoint* pResult = (MGPoint*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pResult);
  // Get input values
  UGPoint *pUGP = (UGPoint*)args[0].addr;
  if(pUGP == NULL || !pUGP->IsDefined()) {
    cerr << "UGPoint not Defined" << endl;
    pResult->SetDefined(false);
    return 0;
  }
  pResult->Clear();
  pResult->StartBulkLoad();
  pResult->Add(*pUGP);
  pResult->EndBulkLoad();
  pResult->SetTrajectoryDefined(false);
  pResult->m_trajectory.TrimToSize();
  pResult->SetBoundingBoxDefined(false);
  return 0;
};

const string OpUgpoint2mgpointSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>ugpoint -> mgpoint" "</text--->"
    "<text>upgoint2mgpoint (_)</text--->"
    "<text>Constructs a mgpoint consisting of the single ugpoint.</text--->"
    "<text>ugpoint2mgpoint(UGPOINT)</text--->"
    ") )";

Operator tempnetugpoint2mgpoint("ugpoint2mgpoint",
                            OpUgpoint2mgpointSpec,
                            OpUgpoint2mgpointValueMapping,
                            Operator::SimpleSelect,
                            OpUgpoint2mgpointTypeMap);

/*
5.35 ~mgpsu2tuple~

The operation ~mgpsu2tuple~ gets a ~stream~ of ~mgpsecunits~ and translates
them into a stream of tuples with the mgpsecunit values as attributes.

TypeMapping:

*/
static string mgpSecTypeInfo =
    "(stream (tuple ((Secid int) (Part int) (Dir int) (Speed real)"
                    "(Starttime instant)(Endtime instant)"
                    "(Leftclosed bool)(Rightclosed bool))))";


ListExpr OpMgpsu2tupleTypeMap(ListExpr in_xArgs)
{
  NList type(in_xArgs);
  if (type.length() == 1)
  {
    NList stream = type.first();
    NList mgp("mgpsecunit");
    if (stream.length() == 2 && stream.checkStream(mgp))
    {
      ListExpr retList;
      nl->ReadFromString(mgpSecTypeInfo, retList);
      return retList;
    }
  }
  return NList::typeError( "Expected a stream of mgpsecunit.");
}

/*
Value Mapping

*/

int OpMgpsu2tupleValueMap(Word* args, Word& result, int message,
                               Word& local, Supplier s)
{
  TupleType *resultTupleType;
  ListExpr resultType;
  Word curAddr;
  switch( message )
  {
    case OPEN:
    {
      qp->Open(args[0].addr);
      resultType = GetTupleResultType( s );
      resultTupleType = new TupleType( nl->Second( resultType ) );
      local.setAddr( resultTupleType );
      return 0;
    }

    case REQUEST:
    {
      resultTupleType = (TupleType *)local.addr;
      qp->Request(args[0].addr, curAddr);
      if (qp->Received(args[0].addr))
      {
        MGPSecUnit *m = (MGPSecUnit*) curAddr.addr;
        Tuple *newTuple = new Tuple( resultTupleType );
        newTuple->PutAttribute(0, new CcInt(true, m->GetSecId()));
        newTuple->PutAttribute(1, new CcInt(true, m->GetPart()));
        newTuple->PutAttribute(2, new CcInt(true, m->GetDirect()));
        newTuple->PutAttribute(3, new CcReal(true, m->GetSpeed()));
        newTuple->PutAttribute(4, new Instant(m->GetTimeInterval().start));
        newTuple->PutAttribute(5, new Instant(m->GetTimeInterval().end));
        newTuple->PutAttribute(6, new CcBool(true,m->GetTimeInterval().lc));
        newTuple->PutAttribute(7, new CcBool(true,m->GetTimeInterval().rc));
        result.setAddr(newTuple);
        m->DeleteIfAllowed();
        return YIELD;
      }
      else return CANCEL;
    }

    case CLOSE:
    {
      qp->Close(args[0].addr);
      if (local.addr) ((TupleType*) local.addr)->DeleteIfAllowed();
      local.setAddr(0);
      return 0;
    }
    default:
    {
      // should not happen
      return -1;
    }
  }
}


struct mgpsu2tupleInfo : OperatorInfo {

  mgpsu2tupleInfo()
  {
    name      = "mgpsu2tuple";
    signature = "stream(mgpsecunit)-> stream(tuple((secid)..(flow))";
    syntax    = "_ mgp2mgpsecunits3 ( _ ) ";
    meaning   = "Builds a stream of mgpsecunits from a stream of mgpoint.";
  }
};

/*
6 Creating the Algebra

*/

class TemporalNetAlgebra : public Algebra
{
  public:

  TemporalNetAlgebra() : Algebra()
  {
    AddTypeConstructor( &unitgpoint );
    AddTypeConstructor( &movinggpoint );
    AddTypeConstructor( &intimegpoint);
    AddTypeConstructor( &mgpsecunitTC);

    movinggpoint.AssociateKind( Kind::TEMPORAL() );
    movinggpoint.AssociateKind( Kind::DATA() );
    unitgpoint.AssociateKind( Kind::TEMPORAL() );
    unitgpoint.AssociateKind( Kind::DATA() );
    intimegpoint.AssociateKind(Kind::TEMPORAL());
    intimegpoint.AssociateKind(Kind::DATA());
    mgpsecunitTC.AssociateKind( Kind::DATA() );

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
    AddOperator(&tempnetunion);
    AddOperator(&tempnetstartunitinst);
    AddOperator(&tempnetendunitinst);
    AddOperator(&tempnetugpoint2mgpoint);
    AddOperator(&tempnetintersects);
    AddOperator(mgp2mgpsecunitsInfo(), OpMgp2mgpsecunitsValueMap,
                OpMgp2mgpsecunitsTypeMap);
    AddOperator(mgp2mgpsecunits2Info(), OpMgp2mgpsecunits2ValueMap,
                OpMgp2mgpsecunits2TypeMap);
    AddOperator(mgp2mgpsecunits3Info(), OpMgp2mgpsecunits3ValueMap,
                OpMgp2mgpsecunits3TypeMap);
    AddOperator(mgpsu2tupleInfo(), OpMgpsu2tupleValueMap, OpMgpsu2tupleTypeMap);
  }


  ~TemporalNetAlgebra() {delete netList;};
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
  netList = new map<int,string>();
  return (new TemporalNetAlgebra());
}
