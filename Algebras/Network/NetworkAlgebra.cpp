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

1 Implementation of Algebra Network


March 2004 Victor Almeida

Mai-Oktober 2007 Martin Scheppokat

February 2008 -  Simone Jandt

October 2008 - Jianqiu Xu

1.1 Defines, includes, and constants

*/

#include <sstream>
#include <time.h>

#include "TupleIdentifier.h"
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "RTreeAlgebra.h"
#include "DBArray.h"
#include "SpatialAlgebra.h"
#include "NetworkAlgebra.h"
#include "RectangleAlgebra.h"
#include "StandardTypes.h"
#include "Algebra.h"
#include "Messages.h"
#include "NetworkManager.h"

extern NestedList* nl;
extern QueryProcessor* qp;

/*
1.2 Helping structs, methods and classes

1.2.1 Sending a message via the message-center

*/
void sendMessage(string in_strMessage)
{
  // Get message-center and initialize message-list
  static MessageCenter* xMessageCenter= MessageCenter::GetInstance();
  NList xMessage;
  xMessage.append(NList("error"));
  xMessage.append(NList().textAtom(in_strMessage));
  xMessageCenter->Send(xMessage);
}

Rectangle<2> RouteInterval::BoundingBox(Network* pNetwork) const{
  if (AlmostEqual(m_dStart , m_dEnd)) {
    Point *p = (GPoint(true, pNetwork->GetId(), m_iRouteId,
                m_dStart)).ToPoint(pNetwork);
    Rectangle<2> bbox = Rectangle<2>(true,
                           p->GetX(), p->GetX(),
                           p->GetY(), p->GetY());
    delete p;
    return bbox;
  } else {
    SimpleLine *line = new SimpleLine(0);
    pNetwork->GetLineValueOfRouteInterval(this, line);
    if (!line->IsEmpty()) {
      Rectangle<2> res = line->BoundingBox();
      delete line;
      return res;
    } else {
      delete line;
      Point *p1 = (GPoint(true, pNetwork->GetId(), m_iRouteId,
            m_dStart)).ToPoint(pNetwork);
      Point *p2 = (GPoint(true, pNetwork->GetId(), m_iRouteId,
            m_dEnd)).ToPoint(pNetwork);
      Rectangle<2> bbox = Rectangle<2>(true,
                           min(p1->GetX(), p2->GetX()),
                           max(p1->GetX(), p2->GetX()),
                           min(p1->GetY(), p2->GetY()),
                           max(p1->GetY(), p2->GetY()));
      delete p1;
      delete p2;
      return bbox;
    }
  }
}

/*
1.2.2 ~searchRouteInterval~

Method for binary search after a route interval in a sorted ~GLine~.
Used for example by operator ~inside~.

*/

bool searchRouteInterval(GPoint *pGPoint, GLine *&pGLine, size_t low,
                         size_t high) {
  const RouteInterval *rI;
  if (low <= high) {
    size_t mid = (high + low) / 2;
    int imid = mid;
    if ((imid < 0) || (imid >= pGLine->NoOfComponents())) {
      return false;
    }else {
      pGLine->Get(mid, rI);
      if (rI->m_iRouteId < pGPoint->GetRouteId()) {
        return searchRouteInterval(pGPoint, pGLine, mid+1, high);
      } else {
        if (rI->m_iRouteId > pGPoint->GetRouteId()){
          return searchRouteInterval(pGPoint, pGLine, low, mid-1);
        } else {
          if (fabs(pGPoint->GetPosition() - rI->m_dStart) < 0.01 ||
              fabs(pGPoint->GetPosition() - rI->m_dEnd) < 0.01) {
            return true;
          } else {
            if (rI->m_dStart > pGPoint->GetPosition()) {
              return searchRouteInterval(pGPoint, pGLine, low, mid-1);
            } else {
              if (rI->m_dEnd < pGPoint->GetPosition()){
                return searchRouteInterval(pGPoint, pGLine, mid+1, high);
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

struct SectTree {

  SectTree(){};

  SectTree(SectTreeEntry *nEntry, SectTree *l = 0, SectTree *r = 0) {
    value = *nEntry;
    left = l;
    right = r;
  };

  ~SectTree(){};

  void Insert(SectTreeEntry *nEntry){
    if (nEntry->secttid < value.secttid) {
      if (right != 0) right->Insert(nEntry);
      else right = new SectTree(nEntry,0,0);
    } else {
      if (nEntry->secttid > value.secttid) {
        if(left != 0) left->Insert(nEntry);
        else left = new SectTree(nEntry,0,0);
      }
    }
  };

  void Find (int n, SectTree *result, bool &found){
    if (n < value.secttid) {
      if (right != 0) right->Find(n, result, found);
      else {
        found = false;
        result = 0;
      }
    } else {
      if (n > value.secttid) {
        if (left != 0) left->Find(n, result, found);
        else {
          found = false;
          result = 0;
        }
      } else {
        found = true;
        result = this;
      }
    }
  };

  void CheckSection(Network *pNetwork, SectTreeEntry n, GPoints &result){
    vector<DirectedSection> sectList;
    sectList.clear();
    SectTree *pSectTree = 0;
    if (n.startbool || n.endbool) {
      if (n.startbool){
        pNetwork->GetAdjacentSections(n.secttid, false, sectList);
        size_t j = 0;
        bool found = true;
        while (j < sectList.size() && found) {
          DirectedSection actSection = sectList[j];
          Find(actSection.GetSectionTid(), pSectTree, found);
        }
        if (!found)
          result += GPoint(true, pNetwork->GetId(), n.rid, n.start, None);
      }
      sectList.clear();
      if (n.endbool){
        pNetwork->GetAdjacentSections(n.secttid, true, sectList);
        size_t j = 0;
        bool found = true;
        while (j < sectList.size() && found) {
          DirectedSection actSection = sectList[j];
          Find(actSection.GetSectionTid(), pSectTree, found);
        }
        if(!found) result+= GPoint(true, pNetwork->GetId(), n.rid, n.end, None);
      }
      sectList.clear();
    } else result += GPoint(true, pNetwork->GetId(), n.rid, n.end, None);
  }

  void WriteResult(Network* pNetwork, GPoints &result, SectTree &secTr){
    if (left != 0) left->WriteResult(pNetwork, result, secTr);
    if (right != 0) right ->WriteResult(pNetwork, result, secTr);
    secTr.CheckSection(pNetwork, value, result);
    //TODO: Remove duplicate GPoint from Result, which are caused by junction
    //problematic
  };

  void Remove(){
    if (left != 0) left->Remove();
    if (right != 0) right->Remove();
    delete this;
  };

  SectTreeEntry value;
  SectTree *left, *right;
};

/*
1.2.6 ~chkPoint~

Almost similar to operator ~checkPoint~ but additional returning a difference
value if the point is not exactly on the ~sline~.

Used by operator ~point2gpoint~

*/

bool chkPoint (SimpleLine *&route, Point point, bool startSmaller, double &pos,
                 double &difference){
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
1.2.7 ~chkPoint~

Almost similar to operator ~chkPoint~ but allowing a greater difference if the
point is not exactly on the ~sline~.

Used by operator ~point2gpoint~

*/

bool chkPoint03 (SimpleLine *&route, Point point, bool startSmaller,
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

bool lastchkPoint03 (SimpleLine *&route, Point point, bool startSmaller,
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
1.2.4 ~checkPoint~

Returns true if a ~point~ is part of a ~sline~, false elsewhere. If the point
is part of the sline his distance from the start is computed also. Used by
operator ~line2gline~.

*/
bool checkPoint (SimpleLine *&route, Point point, bool startSmaller,
                 double &pos){
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
              result = true;
        } else {result = false;}
      } else {
        if (( fabs(xl - xr) < 0.01 && fabs(xl -x) < 0.01) &&
           (((yl < yr|| fabs(yl-yr)<0.01) && (yl < y || fabs(yl-y) <0.01)&&
           (y < yr ||fabs(y-yr)<0.01))|| (yl > yr && (yl > y ||
           fabs(yl-y)<0.01)&& (y > yr ||fabs(y-yr)<0.01)))) {
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
};

/*
1.2.5 ~searchUnit~

Precondition: ~GLine~ is sorted.

Returns true if there is a ~RouteInterval~ in the sorted ~GLine~ which
intersects with the given ~RouteInterval~ false elsewhere.

Used by operator ~intersects~

*/

bool searchUnit(GLine *pGLine, size_t low, size_t high,
                const RouteInterval *pRi) {
  assert(pGLine->IsSorted());
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
1.2.8 class ~GPointList~

Used by the operator ~polygpoint~. Computes and stores the resulting ~GPoints~
for the resulting ~stream~ of ~GPoint~.

*/

class GPointList{
public:
/*
The constructor creates a GPointList from a given gpoint.

*/
   GPointList(GPoint *gp, Network *pNetwork):
    aliasGP(0) {
    GPoint *test;
    lastPos = 0;
    aliasGP.Clear();
    aliasGP.Append(*gp);
    vector<JunctionSortEntry> xJunctions;
    xJunctions.clear();
    if (pNetwork != 0) {
      CcInt iRouteId(true, gp->GetRouteId());
      pNetwork->GetJunctionsOnRoute(&iRouteId, xJunctions);
      bool found = false;
      JunctionSortEntry pCurrJunction;
      size_t i = 0;
      while (!found && i < xJunctions.size()){
        pCurrJunction = xJunctions[i];
        if (fabs (pCurrJunction.GetRouteMeas() - gp->GetPosition()) < 0.01) {
          found = true;
          test = new GPoint(true, gp->GetNetworkId(),
                            pCurrJunction.GetOtherRouteId(),
                            pCurrJunction.GetOtherRouteMeas(),
                            None);
          aliasGP.Append(*test);
        }
        i++;
      }
      while (found && i < xJunctions.size()) {
        pCurrJunction = xJunctions[i];
        if (fabs(pCurrJunction.GetRouteMeas() - gp->GetPosition()) <0.01) {
          test = new GPoint(true, gp->GetNetworkId(),
                            pCurrJunction.GetOtherRouteId(),
                            pCurrJunction.GetOtherRouteMeas(),
                            None);
          aliasGP.Append(*test);
        } else {
          found = false;
        }
        i++;
      }
      xJunctions.clear();
    }
  }

   ~GPointList(){}

/*
~NextGPoint~

This function returns the next GPoint from the GPointList.
If there is no more GPoint in the  List the result will be
0. This function creates a new GPoint instance via the new
operator. The caller of this function has to ensure the
deletion of this object.

*/
   const GPoint* NextGPoint(){
      if(lastPos >= aliasGP.Size() || lastPos < 0){
         return 0;
      } else {
        const GPoint *pAktGPoint;
        aliasGP.Get(lastPos, pAktGPoint);
        lastPos++;
        return pAktGPoint;
      }
    }

private:

   DBArray<GPoint> aliasGP;
   int lastPos;

};

/*
1.2.9 class ~RectangleList~

Almost similar to ~GPointList~. But storing rectangles representing route
intervals of a ~gline~. Used by operator ~routeintervals~ to create a ~stream~
of ~rectangles~.

*/

class RectangleList{

public:

/*
~Constructor~

Creates a RectangleList from a given gline.

*/

   RectangleList(GLine *gl):
    aliasRectangleList(0) {
    Rectangle<2> *elem;
    const RouteInterval *ri;
    lastPos = 0;
    aliasRectangleList.Clear();
    for (int i = 0 ; i < gl->NoOfComponents(); i++) {
      gl->Get(i, ri);
      elem = new Rectangle<2>(true,
                              (double) ri->m_iRouteId,
                              (double) ri->m_iRouteId,
                              min(ri->m_dStart,ri->m_dEnd),
                              max(ri->m_dStart, ri->m_dEnd));
      aliasRectangleList.Append(*elem);
    }
  }

   ~RectangleList(){}

/*
~NextRectangle~

This function returns the next rectangle from the RectangleList.
If there is no more route interval in the  List the result will be
0. This function creates a new Rectangle instance via the new
operator. The caller of this function has to ensure the
deletion of this object.

*/
   const Rectangle<2>* NextRectangle(){
      if(lastPos >= aliasRectangleList.Size() || lastPos < 0){
         return 0;
      } else {
        const Rectangle<2> *pAktRectangle;
        aliasRectangleList.Get(lastPos, pAktRectangle);
        lastPos++;
        return pAktRectangle;
      }
    }

private:

   DBArray<Rectangle<2> > aliasRectangleList;
   int lastPos;

};

/*
~struct RIStack~

Used to build compressed shortestpath ~gline~ values.

*/

struct RIStack {

  RIStack(){};

  RIStack( int ri,double pos1, double pos2, RIStack* next = 0){
    m_iRouteId = ri;
    m_dStart = pos1;
    m_dEnd = pos2;
    m_next = next;
  };

  ~RIStack(){};

  void Push (int rid, double pos1, double pos2, RIStack *&first) {
    RIStack *actElem = new RIStack(rid, pos1, pos2, this);
    first = actElem;
  };

  void StackToGLine (GLine *gline) {
    int actRId = m_iRouteId;
    double actStartPos = m_dStart;
    double actEndPos = m_dEnd;
    RIStack *actElem = this->m_next;
    while (actElem != 0){
      if (actRId == actElem->m_iRouteId &&
          AlmostEqual(actEndPos,actElem->m_dStart)){
        actEndPos = actElem->m_dEnd;
      } else {
        gline->AddRouteInterval(actRId, actStartPos, actEndPos);
        actRId = actElem->m_iRouteId;
        actStartPos = actElem->m_dStart;
        actEndPos = actElem->m_dEnd;
      }
      actElem = actElem->m_next;
    }
    gline->AddRouteInterval(actRId, actStartPos, actEndPos);
  };

  void RemoveStack(){
    if (m_next != 0) m_next->RemoveStack();
    delete this;
  };

  int m_iRouteId;
  double m_dStart, m_dEnd;
  RIStack *m_next;
};

/*
Class PQEntry used for priority Queue in Dijkstras Algorithm for shortest path
computing between two gpoint.

*/

class PQEntry {
  public:
  PQEntry() {}

  PQEntry(TupleId aktID, double distance, bool upDown,
          TupleId beforeID) {
    sectID = aktID;
    distFromStart = distance;
    upDownFlag = upDown;
    beforeSectID = beforeID;
  }

  PQEntry(PQEntry &e) {
    sectID = e.sectID;
    distFromStart = e.distFromStart;
    upDownFlag = e.upDownFlag;
    beforeSectID = e.beforeSectID;
  }
  PQEntry(const PQEntry &e) {
    sectID = e.sectID;
    distFromStart = e.distFromStart;
    upDownFlag = e.upDownFlag;
    beforeSectID = e.beforeSectID;
  }

  ~PQEntry(){}

  TupleId sectID;
  double distFromStart;
  bool upDownFlag;
  TupleId beforeSectID;
};

/*
struct sectIDTree stores the PQEntrys by section ID with identification flag
for the PrioQueue-Array.
An arrayIndex from max integer means not longer in PrioQ.

*/

struct SectIDTree {
  SectIDTree(){};

  SectIDTree(TupleId sectIdent, TupleId beforeSectIdent, bool upDown,
             int arrayIndex,
             SectIDTree *l = 0,SectIDTree *r = 0) {
    sectID = sectIdent;
    beforeSectId = beforeSectIdent;
    upDownFlag = upDown;
    index = arrayIndex;
    left = l;
    right = r;

  };

  ~SectIDTree() {};

  SectIDTree* Find(TupleId sectIdent){
    if (sectID > sectIdent) {
      if (left != 0) return left->Find(sectIdent);
      else {
        return this;
      }
    } else {
      if (sectID < sectIdent) {
        if (right != 0) return right->Find(sectIdent);
        else {
          return this;
        }
      } else {
        return this;
      }
    }
  };

  void Remove(){
    if (left != 0) left->Remove();
    if (right != 0) right->Remove();
    delete this;
  };

  bool Insert(TupleId sectIdent, TupleId beforeSectIdent, bool upDownFlag,
              int arrayIndex,SectIDTree *&pointer){
    pointer = Find(sectIdent);
    if (pointer->sectID != sectIdent) {
      if (pointer->sectID > sectIdent) {
        pointer->left = new SectIDTree(sectIdent, beforeSectIdent, upDownFlag,
                                       arrayIndex);
        pointer = pointer->left;
        return true;
      } else {
        if (pointer->sectID < sectIdent) {
          pointer->right = new SectIDTree(sectIdent, beforeSectIdent,
                                          upDownFlag, arrayIndex);
          pointer = pointer->right;
          return true;
        } else {
          //should never be reached
          return false;
        }
      }
    } else {
      return false;
    }
  };

  void SetIndex(TupleId sectIdent, int arrayIndex){
    Find(sectIdent)->index = arrayIndex;
  };

  void SetIndex(int arrayIndex) {
    index = arrayIndex;
  };

  int GetIndex(TupleId sectIdent){
    return Find(sectIdent)->index;
  };

  void SetBeforeSectId (TupleId sectIdent, TupleId before) {
    Find(sectIdent)->beforeSectId = before;
  };

  void SetBeforeSectId (TupleId before) {
    beforeSectId = before;
  };

  TupleId sectID;
  TupleId beforeSectId;
  bool upDownFlag;
  int index;
  SectIDTree *left, *right;
};

/*
struct Priority Queue for Dijkstras Algorithm of shortest path computing between
two gpoint.

*/

struct PrioQueue {

  PrioQueue() {};

  PrioQueue(int n): prioQ(0) {firstFree = 0;};

  ~PrioQueue(){};

  void CorrectPosition(int checkX, const PQEntry nElem, SectIDTree *pSection,
                       SectIDTree *sectTree){
    int act = checkX;
    const PQEntry* test;
    bool found = false;
    while (checkX >= 0 && !found) {
      if ((act % 2) == 0) checkX = (act-2) / 2;
      else checkX = (act -1) / 2;
      if (checkX >= 0) {
        prioQ.Get(checkX, test);
        if (test->distFromStart > nElem.distFromStart) {
            PQEntry help = *(const_cast<PQEntry*> (test));
            prioQ.Put(checkX, nElem);
            pSection->SetIndex(checkX);
            prioQ.Put(act, help);
            SectIDTree *thelp = sectTree->Find(help.sectID);
            thelp->SetIndex(act);
            act = checkX;
        } else {
          found = true;
        }
      } else {
        found = true;
      }
    }
  };

  void Insert(PQEntry nElem, SectIDTree *sectTree){
    SectIDTree *pSection = sectTree->Find(nElem.sectID);
    const PQEntry *old;
    if (pSection->sectID == nElem.sectID) {
      if (pSection->index != numeric_limits<int>::max()) {
        prioQ.Get(pSection->index, old);
        if (nElem.distFromStart < old->distFromStart) {
          prioQ.Put(pSection->index, nElem);
          pSection->SetBeforeSectId(nElem.beforeSectID);
          CorrectPosition(pSection->index, nElem, pSection, sectTree);
        }
      }
    } else {
      prioQ.Put(firstFree, nElem);
      sectTree->Insert(nElem.sectID, nElem.beforeSectID, nElem.upDownFlag,
                                     firstFree, pSection);
      CorrectPosition(firstFree, nElem, pSection ,sectTree);
      firstFree++;
    }
  }

  PQEntry* GetAndDeleteMin (SectIDTree *sectTree){
    if (firstFree > 0) {
      const PQEntry *result, *last, *test1, *test2;
      prioQ.Get(0,result);
      PQEntry *retValue = new PQEntry(result->sectID, result->distFromStart,
                                      result->upDownFlag, result->beforeSectID);
      SectIDTree *tRet = sectTree->Find(result->sectID);
      tRet->SetIndex(numeric_limits<int>::max());
      prioQ.Get(firstFree-1, last);
      prioQ.Put(0, *last);
      firstFree = firstFree--;
      SectIDTree *pSection = sectTree->Find(last->sectID);
      pSection->SetIndex(0);
      int act = 0;
      int checkX = 0;
      bool found = false;
      while (checkX < firstFree && !found){
        checkX = 2*act + 1;
        if (checkX < firstFree-1) {
          prioQ.Get(checkX, test1);
          prioQ.Get(checkX+1, test2);
          if (test1->distFromStart < last->distFromStart ||
             test2->distFromStart < last->distFromStart){
            if (test1->distFromStart <= test2->distFromStart) {
              PQEntry help = *(const_cast<PQEntry*> (test1));
              prioQ.Put(checkX, *last);
              pSection->SetIndex(checkX);
              prioQ.Put(act, help);
              SectIDTree *thelp = sectTree->Find(help.sectID);
              thelp->SetIndex(act);
              act = checkX;
            } else {
              PQEntry help = *(const_cast<PQEntry*> (test2));
              prioQ.Put(checkX+1, *last);
              pSection->SetIndex(checkX+1);
              prioQ.Put(act, help);
              SectIDTree *thelp = sectTree->Find(help.sectID);
              thelp->SetIndex(act);
              act = checkX+1;
            }
          } else {
            found = true;
          }
        } else {
          if (checkX != 0 && checkX == firstFree-1){
            prioQ.Get(checkX, test1);
            if (test1->distFromStart < last->distFromStart) {
              PQEntry help = *(const_cast<PQEntry*> (test1));
              prioQ.Put(checkX, *last);
              pSection->SetIndex(checkX);
              prioQ.Put(act, help);
              SectIDTree *thelp = sectTree->Find(help.sectID);
              thelp->SetIndex(act);
              act = checkX;
            } else {
              found = true;
            }
          }
        }
      }
      return retValue;
    } else {
      return 0;
    }
  }

  void Clear(){
    prioQ.Clear();
    firstFree = 0;
  }

  bool IsEmpty() {
    if (firstFree == 0) return true;
    else return false;
  }

  void Destroy(){
    prioQ.Destroy();
  }

  DBArray<PQEntry> prioQ;
  int firstFree;

};
    /*
Deprecated
1.2.10 struct ~DijkstraStruct~

Used for Dijkstras-Algorithm to compute the shortest path between two ~GPoint~.



struct DijkstraStruct
{
  DijkstraStruct()
  {
  }

  DijkstraStruct(int in_iSectionTid,
                 bool in_bUpDownFlag,
                 int in_iRouteId,
                 double in_dMeas1,
                 double in_dMeas2,
                 double in_dD,
                 double in_dHeuristicDistanceToEnd,
                 int in_iPiSectionTid,
                 bool in_bPiUpDownFlag)
  {
    m_iSectionTid = in_iSectionTid;
    m_bUpDownFlag = in_bUpDownFlag;
    m_iRouteId = in_iRouteId;
    m_dMeas1 = in_dMeas1;
    m_dMeas2 = in_dMeas2;
    m_dD = in_dD;
    m_dHeuristicDistanceToEnd = in_dHeuristicDistanceToEnd;
    m_iPiSectionTid = in_iPiSectionTid;
    m_bPiUpDownFlag = in_bPiUpDownFlag;
  }

  DijkstraStruct( const DijkstraStruct& in_xDikstraStruct):
    m_iSectionTid(in_xDikstraStruct.m_iSectionTid),
    m_bUpDownFlag(in_xDikstraStruct.m_bUpDownFlag),
    m_iRouteId(in_xDikstraStruct.m_iRouteId),
    m_dMeas1(in_xDikstraStruct.m_dMeas1),
    m_dMeas2(in_xDikstraStruct.m_dMeas2),
    m_dD(in_xDikstraStruct.m_dD),
    m_dHeuristicDistanceToEnd(in_xDikstraStruct.m_dHeuristicDistanceToEnd),
    m_iPiSectionTid(in_xDikstraStruct.m_iPiSectionTid),
    m_bPiUpDownFlag(in_xDikstraStruct.m_bPiUpDownFlag)
  {
  }

  int m_iSectionTid;

  bool m_bUpDownFlag;

  int m_iRouteId;

  double m_dMeas1;

  double m_dMeas2;

  double m_dD;

  double m_dHeuristicDistanceToEnd;

  int m_iPiSectionTid;

  bool m_bPiUpDownFlag;

  double Length()
  {
    return m_dMeas2 - m_dMeas1;
  }

  double DistanceEstimation()
  {
    return m_dD + m_dHeuristicDistanceToEnd;
  }


  bool operator== (const DijkstraStruct& in_xOther)
  {
    bool bReturn = (m_iSectionTid == in_xOther.m_iSectionTid &&
                    m_bUpDownFlag == in_xOther.m_bUpDownFlag);
    return bReturn;
  }
};


1.2.11 class ~PriorityQueue~

Used for shortest path computing in Dijkstras Algorithm.



class PriorityQueue
{
  public:

  PriorityQueue()
  {
  }

  void push(DijkstraStruct* in_pStruct)
  {
    m_xQueue.push_back(in_pStruct);
    size_t iIndex = 2 * (in_pStruct->m_iSectionTid - 1);
    if(in_pStruct->m_bUpDownFlag)
    {
      iIndex++;
    }
    if(m_xElements.size() < iIndex + 1)
    {
      m_xElements.resize(iIndex + 1);
    }
    m_xElements[iIndex] = in_pStruct;

  }

  DijkstraStruct* pop()
  {

    // ----------------------------------------------------------
    // For testing only !
    // This implementation needs O(n)
    // To be replaced
    // ----------------------------------------------------------
    DijkstraStruct* pMinDijkstraStruct;

    for(size_t i = 0; i < m_xQueue.size(); ++i)
    {
      DijkstraStruct* pCurrentStruct = m_xQueue[i];
      if(i == 0 ||
         pCurrentStruct->DistanceEstimation() <
         pMinDijkstraStruct->DistanceEstimation())
      {
        pMinDijkstraStruct = pCurrentStruct;
      }
    }

    std::vector<DijkstraStruct*>::iterator xIt = m_xQueue.begin();
    while(xIt != m_xQueue.end())
    {
      DijkstraStruct* pCurrent = *xIt;
      if(pCurrent->m_iSectionTid == pMinDijkstraStruct->m_iSectionTid &&
         pCurrent->m_bUpDownFlag == pMinDijkstraStruct->m_bUpDownFlag)
      {
        m_xQueue.erase (xIt);
        m_xS.push_back(pCurrent);

        break;
      }
      xIt++;
    }
    return pMinDijkstraStruct;
  }

  bool isEmtpy()
  {
    return m_xQueue.empty();
  }

  DijkstraStruct* get(int in_iSectionTid,
                      bool in_bUpDownFlag)
  {
    size_t iIndex = 2 * (in_iSectionTid - 1);
    if(in_bUpDownFlag)
    {
      iIndex++;
    }
    return m_xElements[iIndex];
  }

  int getSSize()
  {
    return m_xS.size();
  }

  DijkstraStruct* getS(int in_iIndex)
  {
    return m_xS[in_iIndex];
  }

  private:

  vector<DijkstraStruct*> m_xElements;

  // TODO: Zweite Datenstruktur mit allen Elementen in sortiert
  vector<DijkstraStruct*> m_xQueue;

  vector<DijkstraStruct*> m_xS;

};


1.2.12 ~Dijkstra~

Modified version of dikstras algorithm calculating shortest path in graphs.

Whereas sedgewick's version of disktra operates on the nodes of the graph this
version will process an array of edges. This change is necessary to take into
account that not all transitions between connecting edges are possible. The
preceding edge has to be known when looking for the next one.


void Dijkstra(Network* in_pNetwork,
                              int in_iStartSegmentId,
                              GPoint* in_pFromGPoint,
                              int in_iEndSegmentId,
                              GPoint* in_pToGPoint,
                              Point* in_pToPoint,
                              GLine* in_pGLine)
{
 // clock_t xEnterTime = clock();
  // Specialized data structure with all edges of the graph. This
  // structure will not only support the access to the remaining edges
  // with the fewest weight but also to all edges by their index.
  PriorityQueue xQ;

  /////////////////////////////////////////
  //
  // InitializeSingleSource
  //
  // Get all Sections and fill them into Q. All weights but
  // the one for the starting segment are set to infinity
  Relation* pSections = in_pNetwork->GetSectionsInternal();
  GenericRelationIterator* pSectionsIt = pSections->MakeScan();
  Tuple* pSection;
  while( (pSection = pSectionsIt->GetNextTuple()) != 0 )
  {
    // Get values
    int iSegmentId = pSectionsIt->GetTupleId();
    CcInt* xRouteId = (CcInt*)pSection->GetAttribute(SECTION_RID);
    int iRouteId = xRouteId->GetIntval();
    CcReal* xMeas1 = (CcReal*)pSection->GetAttribute(SECTION_MEAS1);
    double dMeas1 = xMeas1->GetRealval();
    CcReal* xMeas2 = (CcReal*)pSection->GetAttribute(SECTION_MEAS2);
    double dMeas2 = xMeas2->GetRealval();
    SimpleLine* pLine = (SimpleLine*)pSection->GetAttribute(SECTION_CURVE);
    Point* pPoint = new Point(false);
    pLine->AtPosition(0, true, *pPoint);

    double dx = pPoint->GetX() - in_pToPoint->GetX();
    double dy = pPoint->GetY() - in_pToPoint->GetY();
    delete pPoint;
    double dHeuristicDistanceToEnd = sqrt( pow( dx, 2 ) + pow( dy, 2 ) );

//    // Use Dijkstra instead of A*
//    fHeuristicDistanceToEnd = 0;

    // Put one struct for each direction in the map (up and down)
    bool bStartSegment = iSegmentId == in_iStartSegmentId &&
                         (in_pFromGPoint->GetSide() == None ||
                          in_pFromGPoint->GetSide() == Up);
    xQ.push(new DijkstraStruct(iSegmentId,
                               true,
                               iRouteId,
                               dMeas1,
                               dMeas2,
                               bStartSegment ? 0 : double(1e29),
                               dHeuristicDistanceToEnd,
                               -1,
                               true));

    bStartSegment = iSegmentId == in_iStartSegmentId &&
                         (in_pFromGPoint->GetSide() == None ||
                         in_pFromGPoint->GetSide() == Down);
    xQ.push(new DijkstraStruct(iSegmentId,
                               false,
                               iRouteId,
                               dMeas1,
                               dMeas2,
                               bStartSegment ? 0 : double(1e29),
                               dHeuristicDistanceToEnd,
                               -1,
                               true));

    pSection->DeleteIfAllowed();
  }
  delete pSectionsIt;
//       << "InitializeSingleSource Time: "
//       << (clock() - xEnterTime) << " / " << CLOCKS_PER_SEC << " Sekunden."
//       << endl;
  // End InitializeSingleSource

  /////////////////////////////////
  //
  // Now dikstras algorithm will handle each node in
  // the queue recalculating their weights.
  //
  while(! xQ.isEmtpy())
  {
    // Extract-Min
    DijkstraStruct* pCurrent = xQ.pop();

    // Abbruchbedingung pr�fen
    if(pCurrent->m_iSectionTid == in_iEndSegmentId &&
       (
         in_pToGPoint->GetSide() == None ||
         (
           (
             in_pToGPoint->GetSide() == Up &&
             pCurrent->m_bUpDownFlag
           ) ||
           (
             in_pToGPoint->GetSide() == Down &&
             !pCurrent->m_bUpDownFlag
           )
         )
       )
      )
    {
      break;
    }

    // Get values
    int iCurrentSectionTid = pCurrent->m_iSectionTid;
    bool bCurrentUpDownFlag = pCurrent->m_bUpDownFlag;

    // Get all adjacent sections
    vector<DirectedSection> xAdjacentSections;
    xAdjacentSections.clear();
    in_pNetwork->GetAdjacentSections(iCurrentSectionTid,
                                     bCurrentUpDownFlag,
                                     xAdjacentSections);

    // Iterate over adjacent sections
    for(size_t i = 0;  i < xAdjacentSections.size(); i++)
    {
      // Load the structure belonging to the adjacent section
      DirectedSection xAdjacentSection = xAdjacentSections[i];
      int iAdjacentSectionTid = xAdjacentSection.GetSectionTid();
      bool bAdjacentUpDownFlag = xAdjacentSection.GetUpDownFlag();
      DijkstraStruct* pAdjacent = xQ.get(iAdjacentSectionTid,
                                         bAdjacentUpDownFlag);

      // Relax the weight if a shorter path has been found
      if(pAdjacent->m_dD > pCurrent->m_dD + pCurrent->Length())
      {
        // Calculate new distance
        pAdjacent->m_dD = pCurrent->m_dD + pCurrent->Length();
        // Set current as predecessor of adjacent section
        pAdjacent->m_iPiSectionTid = pCurrent->m_iSectionTid;
        pAdjacent->m_bPiUpDownFlag = pCurrent->m_bUpDownFlag;
      }
    }
  }
  // Now all weights and all predecessors have been found.

  // Find the route starting at the end looking at the pi-entries pointing
  // at a predecessor.
  DijkstraStruct* pStruct;
  if(in_pToGPoint->GetSide() == Up)
  {
    pStruct = xQ.get(in_iEndSegmentId,
                     true);
  }
  else if(in_pToGPoint->GetSide() == Down)
  {
    pStruct = xQ.get(in_iEndSegmentId,
                     false);
  }
  else
  {
    // GPoint lies on an undirected section. We first get the distance
    // for both sides
    double dUpDistance = xQ.get(in_iEndSegmentId, true)->m_dD;
    double dDownDistance = xQ.get(in_iEndSegmentId, false)->m_dD;
    // Now we get the directed section for the shorter Distance
    pStruct = xQ.get(in_iEndSegmentId,
                     dUpDistance < dDownDistance);
  }

  if(pStruct->m_iPiSectionTid == -1)
  {
    // End not reachable from start. Either the graph consists of two
    // parts or the ConnectivityCode prevents driving on all path from
    // start to end.
    // Return empty GLine
    string strMessage = "Destination is not reachable from the start. Either "
                        "the points are located in disjoint parts of the "
                        "network or the ConnectityCode prevents reaching the "
                        "destination.";
    sendMessage(strMessage);
    return;
  }

  // Add last interval of the route
  double dPos1 = pStruct->m_dMeas1;
  double dPos2 = pStruct->m_dMeas2;
  if(pStruct->m_bUpDownFlag)
  {
    dPos2 = in_pToGPoint->GetPosition();
  }
  else
  {
    dPos1 = in_pToGPoint->GetPosition();
  }
  in_pGLine->AddRouteInterval(pStruct->m_iRouteId,
                              dPos1,
                              dPos2);
  // Add all predecessors until the start is reached.
  while(pStruct->m_iSectionTid != in_iStartSegmentId)
  {
    pStruct = xQ.get(pStruct->m_iPiSectionTid,
                     pStruct->m_bPiUpDownFlag);
    double dPos1 = pStruct->m_dMeas1;
    double dPos2 = pStruct->m_dMeas2;

    if(pStruct->m_iSectionTid == in_iStartSegmentId)
    {
      // First segment reached. We only need a part of this segment
      if(pStruct->m_bUpDownFlag)
      {
        dPos1 = in_pFromGPoint->GetPosition();
      }
      else
      {
        dPos2 = in_pFromGPoint->GetPosition();
      }
    }

    in_pGLine->AddRouteInterval(pStruct->m_iRouteId,
                                dPos1,
                                dPos2);
  }

//       << "DijkstraTime: "
//       << (clock() - xEnterTime) << " / " << CLOCKS_PER_SEC << " Sekunden."
//       << endl;
}
    */
/*
1.3 Class Definitions

1.3.1 class ~Network~

1.3.1.1 Network relations

*/
string Network::routesTypeInfo =
      "(rel (tuple ((id int) (length real) (curve sline) "
      "(dual bool) (startsSmaller bool))))";
string Network::routesBTreeTypeInfo =
      "(btree (tuple ((id int) (length real) (curve sline) "
      "(dual bool) (startsSmaller bool))) int)";

string Network::routesRTreeTypeInfo =
      "(rtree (tuple((id int)(length real)(curve sline)(dual bool)"
      "(startsSmaller bool))) sline FALSE)";

string Network::junctionsTypeInfo =
      "(rel (tuple ((r1id int) (meas1 real) (r2id int) "
      "(meas2 real) (cc int))))";
string Network::junctionsInternalTypeInfo =
      "(rel (tuple ((r1id int) (meas1 real) (r2id int) "
      "(meas2 real) (cc int) (pos point) (r1rc tid) (r2rc tid) "
      "(sauprc tid) (sadownrc tid)(sbuprc tid) (sbdownrc tid))))";
string Network::junctionsBTreeTypeInfo =
      "(btree (tuple ((r1id int) (meas1 real) (r2id int) "
      "(meas2 real) (cc int) (pos point) (r1rc tid) (r2rc tid) "
      "(sauprc tid) (sadownrc tid)(sbuprc tid) (sbdownrc tid))) int)";
string Network::sectionsInternalTypeInfo =
      "(rel (tuple ((rid int) (meas1 real) (meas2 real) (dual bool)"
      "(curve sline)(curveStartsSmaller bool) (rrc tid) (sid int))))";
string Network::sectionsBTreeTypeInfo =
      "(btree (tuple ((rid int) (meas1 real) (meas2 real) (dual bool)"
      "(curve sline)(curveStartsSmaller bool) (rrc tid) (sid int))) int)";
string Network::distancestorageTypeInfo =
      "(rel (tuple((j1 tid)(j2 tid)(dist real)(sp gline))))";

/*
1.3.1.2 Constructors and destructors class ~Network~

*/

Network::Network():
m_iId(0),
m_bDefined(false),
m_pRoutes(0),
m_pJunctions(0),
m_pSections(0),
m_pBTreeRoutes(0),
m_pRTreeRoutes(0),
m_pBTreeJunctionsByRoute1(0),
m_pBTreeJunctionsByRoute2(0),
m_xAdjacencyList(0),
m_xSubAdjacencyList(0),
m_pBTreeSectionsByRoute(0)
/*alldistance(0)*/
{
}

Network::Network(SmiRecord& in_xValueRecord,
                 size_t& inout_iOffset,
                 const ListExpr in_xTypeInfo):
m_xAdjacencyList(0),
m_xSubAdjacencyList(0)
{

  // Read network id
  in_xValueRecord.Read( &m_iId, sizeof( int ), inout_iOffset );
  inout_iOffset += sizeof( int );

  // Open routes
  ListExpr xType;
  nl->ReadFromString(routesTypeInfo, xType);
  ListExpr xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  m_pRoutes = Relation::Open(in_xValueRecord,
                             inout_iOffset,
                             xNumericType);
  if(!m_pRoutes)
  {
    return;
  }

  // Open junctions
  nl->ReadFromString(junctionsInternalTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  m_pJunctions = Relation::Open(in_xValueRecord,
                                inout_iOffset,
                                xNumericType);
  if(!m_pJunctions)
  {
    m_pRoutes->Delete();
    return;
  }

  // Open sections
  nl->ReadFromString(sectionsInternalTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  m_pSections = Relation::Open(in_xValueRecord,
                               inout_iOffset,
                               xNumericType);
  if(!m_pSections)
  {
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    return;
  }

  // Open btree for routes
  nl->ReadFromString(routesBTreeTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  m_pBTreeRoutes = BTree::Open(in_xValueRecord,
                               inout_iOffset,
                               xNumericType);

  if(!m_pBTreeRoutes)
  {
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    m_pSections->Delete();
    return;
  }
  //Open  rtree for routes
  Word xValue;

  if (!(m_pRTreeRoutes->Open(in_xValueRecord,
                            inout_iOffset,
                            routesRTreeTypeInfo,
                            xValue)))
  {
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    m_pSections->Delete();
    delete m_pBTreeRoutes;
    return;
  }

  m_pRTreeRoutes = (R_Tree<2,TupleId>*) xValue.addr;

  // Open first btree for junctions
  nl->ReadFromString(junctionsBTreeTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  m_pBTreeJunctionsByRoute1 = BTree::Open(in_xValueRecord,
                                          inout_iOffset,
                                          xNumericType);
  if(!m_pBTreeJunctionsByRoute1)
  {
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    m_pSections->Delete();
    delete m_pBTreeRoutes;
    delete m_pRTreeRoutes;
    return;
  }

  // Open second btree for junctions
  nl->ReadFromString(junctionsBTreeTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  m_pBTreeJunctionsByRoute2 = BTree::Open(in_xValueRecord,
                                          inout_iOffset,
                                          xNumericType);
  if(!m_pBTreeJunctionsByRoute2)
  {
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    m_pSections->Delete();
    delete m_pBTreeRoutes;
    delete m_pRTreeRoutes;
    delete m_pBTreeJunctionsByRoute1;
    return;
  }

  m_xAdjacencyList.OpenFromRecord(in_xValueRecord, inout_iOffset);
  m_xSubAdjacencyList.OpenFromRecord(in_xValueRecord, inout_iOffset);

  // Open btree for sections
  nl->ReadFromString(sectionsBTreeTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  m_pBTreeSectionsByRoute = BTree::Open(in_xValueRecord,
                                          inout_iOffset,
                                          xNumericType);
  if(!m_pBTreeSectionsByRoute)
  {
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    m_pSections->Delete();
    delete m_pBTreeRoutes;
    delete m_pRTreeRoutes;
    delete m_pBTreeJunctionsByRoute1;
    delete m_pBTreeJunctionsByRoute2;
    return;
  }

  //Open distance storage
  /*
  nl->ReadFromString(distancestorageTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  alldistance = Relation::Open(in_xValueRecord,inout_iOffset,xNumericType);

  if(!alldistance){
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    m_pSections->Delete();
    delete m_pBTreeRoutes;
    delete m_pRTreeRoutes;
    delete m_pBTreeJunctionsByRoute1;
    delete m_pBTreeJunctionsByRoute2;
    return;
  }
  */
  m_bDefined = true;
}

Network::Network(ListExpr in_xValue,
                 int in_iErrorPos,
                 ListExpr& inout_xErrorInfo,
                 bool& inout_bCorrect):
m_iId(0),
m_bDefined(false),
m_pRoutes(0),
m_pJunctions(0),
m_pSections(0),
m_pBTreeRoutes(0),
m_pRTreeRoutes(0),
m_pBTreeJunctionsByRoute1(0),
m_pBTreeJunctionsByRoute2(0),
m_xAdjacencyList(0),
m_xSubAdjacencyList(0),
m_pBTreeSectionsByRoute(0)
/*alldistance(0)*/
{

  // Check the list
  if(!(nl->ListLength(in_xValue) == 3))
  {
    string strErrorMessage = "Network(): List length must be 3.";
    inout_xErrorInfo =
    nl->Append(inout_xErrorInfo, nl->StringAtom(strErrorMessage));
    inout_bCorrect = false;
    return;
  }

  // Get type-info for temporary table
  ListExpr xType;
  nl->ReadFromString(routesTypeInfo, xType);
  ListExpr xRoutesNumType = SecondoSystem::GetCatalog()->NumericType(xType);
  nl->ReadFromString(junctionsTypeInfo, xType);
  ListExpr xJunctionsNumType = SecondoSystem::GetCatalog()->NumericType(xType);

  // Split into the three parts
  ListExpr xIdList = nl->First(in_xValue);
  ListExpr xRouteList = nl->Second(in_xValue);
  ListExpr xJunctionList = nl->Third(in_xValue);
  // Sections will be calculated in the load-method

  // Read Id
  if(!nl->IsAtom(xIdList) ||
     nl->AtomType(xIdList) != IntType)
  {
    string strErrorMessage = "Network(): Id is missing.";
    inout_xErrorInfo = nl->Append(inout_xErrorInfo,
                                  nl->StringAtom(strErrorMessage));
    inout_bCorrect = false;
    return;
  }
  m_iId = nl->IntValue(xIdList);

  // Create new temporary relations.
  Relation* pRoutes = new Relation(xRoutesNumType, true);
  Relation* pJunctions = new Relation(xJunctionsNumType, true);

  // Iterate over all routes
  while(!nl->IsEmpty(xRouteList))
  {
    ListExpr xCurrentRoute = nl->First(xRouteList);
    xRouteList = nl->Rest(xRouteList);

    // Create tuple for internal table
    Tuple* pNewRoute = new Tuple(nl->Second(xRoutesNumType));

    // Check this part of the list
    if(nl->ListLength(xCurrentRoute) != 5 ||
      (!nl->IsAtom(nl->First(xCurrentRoute))) ||
      nl->AtomType(nl->First(xCurrentRoute)) != IntType ||
      (!nl->IsAtom(nl->Second(xCurrentRoute))) ||
      nl->AtomType(nl->Second(xCurrentRoute)) != RealType ||
      (nl->IsAtom(nl->Third(xCurrentRoute))) ||
      (!nl->IsAtom(nl->Fourth(xCurrentRoute))) ||
      nl->AtomType(nl->Fourth(xCurrentRoute)) != BoolType ||
      (!nl->IsAtom(nl->Fifth(xCurrentRoute))) ||
      nl->AtomType(nl->Fifth(xCurrentRoute)) != BoolType)
    {
      delete pRoutes;
      delete pRoutes;

      string strErrorMessage = "Network(): Error while reading out routes.";
      inout_xErrorInfo = nl->Append(inout_xErrorInfo,
                                    nl->StringAtom(strErrorMessage));
      inout_bCorrect = false;
      return;
    }

    // Read attributes from list
    // Read values from table
    int iRouteId = nl->IntValue(nl->First(xCurrentRoute));
    double dLength  = nl->RealValue(nl->Second(xCurrentRoute));
    Word xLineWord = InSimpleLine(nl->TheEmptyList(),
                            nl->Third(xCurrentRoute),
                            in_iErrorPos,
                            inout_xErrorInfo,
                            inout_bCorrect);
    SimpleLine* pLine = (SimpleLine*)(xLineWord.addr);
    bool bDual= nl->BoolValue(nl->Fourth(xCurrentRoute));
    bool bStartsSmaller  = nl->BoolValue(nl->Fifth(xCurrentRoute));

    // Set all necessary attributes
    pNewRoute->PutAttribute(ROUTE_ID, new CcInt(true, iRouteId));
    pNewRoute->PutAttribute(ROUTE_LENGTH, new CcReal(true, dLength));
    pNewRoute->PutAttribute(ROUTE_CURVE, pLine);
    pNewRoute->PutAttribute(ROUTE_DUAL, new CcBool(true, bDual));
    pNewRoute->PutAttribute(ROUTE_STARTSSMALLER, new CcBool(true,
                                                              bStartsSmaller));

    // Append new junction
    pRoutes->AppendTuple(pNewRoute);
  }

   // Iterate over all junctions
  while(!nl->IsEmpty(xJunctionList))
  {
    ListExpr xCurrentJunction = nl->First(xJunctionList);
    xJunctionList = nl->Rest(xJunctionList);

    // Create tuple for internal table
    Tuple* pNewJunction = new Tuple(nl->Second(xJunctionsNumType));

    // Check this part of the list
    if(nl->ListLength(xCurrentJunction) != 6 ||
      (!nl->IsAtom(nl->First(xCurrentJunction))) ||
      nl->AtomType(nl->First(xCurrentJunction)) != IntType ||
      (!nl->IsAtom(nl->Second(xCurrentJunction))) ||
      nl->AtomType(nl->Second(xCurrentJunction)) != RealType ||
      (!nl->IsAtom(nl->Third(xCurrentJunction))) ||
      nl->AtomType(nl->Third(xCurrentJunction)) != IntType ||
      (!nl->IsAtom(nl->Fourth(xCurrentJunction))) ||
      nl->AtomType(nl->Fourth(xCurrentJunction)) != RealType ||
      (!nl->IsAtom(nl->Fifth(xCurrentJunction))) ||
      nl->AtomType(nl->Fifth(xCurrentJunction)) != IntType)
    {
      delete pRoutes;
      delete pJunctions;

      string strErrorMessage = "Network(): Error while reading out junctions.";
      inout_xErrorInfo = nl->Append(inout_xErrorInfo,
                         nl->StringAtom(strErrorMessage));
      inout_bCorrect = false;
      return;
    }

    // Read attributes from list
    int iRoute1Id = nl->IntValue(nl->First(xCurrentJunction));
    double dMeas1 = nl->RealValue(nl->Second(xCurrentJunction));
    int iRoute2Id = nl->IntValue(nl->Third(xCurrentJunction));
    double dMeas2 = nl->RealValue(nl->Fourth(xCurrentJunction));
    int iConnectivityCode= nl->IntValue(nl->Fifth(xCurrentJunction));
    // The location of the junction "Point" is calculated in the load-method

    // Set all necessary attributes
    pNewJunction->PutAttribute(JUNCTION_ROUTE1_ID,
                                new CcInt(true, iRoute1Id));
    pNewJunction->PutAttribute(JUNCTION_ROUTE1_MEAS,
                                new CcReal(true, dMeas1));
    pNewJunction->PutAttribute(JUNCTION_ROUTE2_ID,
                                new CcInt(true, iRoute2Id));
    pNewJunction->PutAttribute(JUNCTION_ROUTE2_MEAS,
                                new CcReal(true, dMeas2));
    pNewJunction->PutAttribute(JUNCTION_CC,
                                new CcInt(true, iConnectivityCode));

    // Append new junction
    pJunctions->AppendTuple(pNewJunction);
  }

  Load(m_iId,
       pRoutes,
       pJunctions);

  delete pRoutes;
  delete pJunctions;


  m_bDefined = true;
}

/*
Destructor

*/
Network::~Network()
{
  delete m_pRoutes;

  delete m_pJunctions;

  delete m_pSections;

  delete m_pBTreeRoutes;

  delete m_pRTreeRoutes;

  delete m_pBTreeJunctionsByRoute1;

  delete m_pBTreeJunctionsByRoute2;

  delete m_pBTreeSectionsByRoute;

//  delete alldistance;
}

/*
1.3.1.3 Methods class ~network~

Destroy -- Removing a network from the database

*/
void Network::Destroy()
{
  assert(m_pRoutes != 0);
  m_pRoutes->Delete(); m_pRoutes = 0;

  assert(m_pJunctions != 0);
  m_pJunctions->Delete(); m_pJunctions = 0;

  assert(m_pSections != 0);
  m_pSections->Delete(); m_pSections = 0;

  assert(m_pBTreeRoutes != 0);
  m_pBTreeRoutes->DeleteFile();
  delete m_pBTreeRoutes; m_pBTreeRoutes = 0;

  assert(m_pRTreeRoutes != 0);
  //m_pRTreeRoutes->DeleteFile();
  delete m_pRTreeRoutes; m_pRTreeRoutes = 0;

  m_pBTreeJunctionsByRoute1->DeleteFile();
  delete m_pBTreeJunctionsByRoute1; m_pBTreeJunctionsByRoute1 = 0;

  m_pBTreeJunctionsByRoute2->DeleteFile();
  delete m_pBTreeJunctionsByRoute2; m_pBTreeJunctionsByRoute2 = 0;

//  m_xAdjacencyList.Destroy();
//  m_xSubAdjacencyList.Destroy();
  assert(m_pBTreeSectionsByRoute != 0);
  m_pBTreeSectionsByRoute->DeleteFile();
  delete m_pBTreeSectionsByRoute;
  m_pBTreeSectionsByRoute = 0;
  /*
  assert(alldistance != 0);
  delete alldistance;
  */
}

/*
Load -- Create a network from two external relations

*/

void Network::Load(int in_iId,
                   const Relation* in_pRoutes,
                   const Relation* in_pJunctions)
{
  m_iId = in_iId;
  FillRoutes(in_pRoutes);
  FillJunctions(in_pJunctions);
  FillSections();
  FillAdjacencyLists();
//FillDistanceStorage();//store distance
  m_bDefined = true;
}

void Network::FillRoutes(const Relation *routes)
{
  ostringstream xRoutesPtrStream;
  xRoutesPtrStream << (long)routes;

  string strQuery = "(consume (sort (feed (" + routesTypeInfo +
                       " (ptr " + xRoutesPtrStream.str() + ")))))";

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  m_pRoutes = (Relation *)xResult.addr;
  // Create B-Tree for the routes
  ostringstream xThisRoutesPtrStream;
  xThisRoutesPtrStream << (long)m_pRoutes;
  strQuery = "(createbtree (" + routesTypeInfo +
                " (ptr " + xThisRoutesPtrStream.str() + "))" + " id)";

  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted); // no query with side effects, please!
  m_pBTreeRoutes = (BTree*)xResult.addr;
  //Create R-Tree for the routes
  ostringstream xNetRoutes;
  xNetRoutes << (long) m_pRoutes;

  strQuery = "(creatertree (" + routesTypeInfo + "(ptr "
              + xNetRoutes.str() +")) curve)";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  m_pRTreeRoutes = (R_Tree<2,TupleId>*)xResult.addr;

}


void Network::FillJunctions(const Relation *in_pJunctions)
{
  /////////////////////////////////////////////////////////////////////
  //
  // Create new table for the junctions
  //
  ListExpr xTypeInfo;
  nl->ReadFromString(junctionsInternalTypeInfo, xTypeInfo);
  ListExpr xNumType = SecondoSystem::GetCatalog()->NumericType(xTypeInfo);
  Relation *pIntJunctions = new Relation(xNumType, true);

  /////////////////////////////////////////////////////////////////////
  //
  // Iterator for the input-table with junctions
  //
  GenericRelationIterator* pJunctionsIter = in_pJunctions->MakeScan();
  Tuple* pCurrentJunction;
  while((pCurrentJunction = pJunctionsIter->GetNextTuple()) != 0)
  {
    /////////////////////////////////////////////////////////////////////
    //
    // Create tuple for internal table and copy all attributes from input
    //
    Tuple* pNewJunction = new Tuple(nl->Second(xNumType));
    for(int i = 0; i < pCurrentJunction->GetNoAttributes(); i++)
    {
      pNewJunction->CopyAttribute(i, pCurrentJunction, i);
    }


    /////////////////////////////////////////////////////////////////////
    //
    // Fill other fields of the table
    //

    // Store Pointer to the first route in the new relation.
    CcInt* pR1Id = (CcInt*)pCurrentJunction->GetAttribute(JUNCTION_ROUTE1_ID);
    BTreeIterator* pRoutesIter = m_pBTreeRoutes->ExactMatch(pR1Id);
    int NextIter = pRoutesIter->Next();
    assert(NextIter);
    TupleIdentifier *pR1RC = new TupleIdentifier(true, pRoutesIter->GetId());
    pNewJunction->PutAttribute(JUNCTION_ROUTE1_RC, pR1RC);

    // Calculate and store the exakt location of the junction.
    Tuple* pRoute = m_pRoutes->GetTuple(pRoutesIter->GetId());
    assert(pRoute != 0);
    SimpleLine* pLine = (SimpleLine*)pRoute->GetAttribute(ROUTE_CURVE);
    assert(pLine != 0);
    CcReal* pMeas = (CcReal*)pNewJunction->GetAttribute(JUNCTION_ROUTE1_MEAS);
    Point* pPoint = new Point(false);
    pLine->AtPosition(pMeas->GetRealval(), true, *pPoint);
    pNewJunction->PutAttribute(JUNCTION_POS, pPoint);

    pRoute->DeleteIfAllowed();
    delete pRoutesIter;

    // Store Pointer to the second route in the new relation.
    CcInt* pR2Id = (CcInt*)pCurrentJunction->GetAttribute(JUNCTION_ROUTE2_ID);
    pRoutesIter = m_pBTreeRoutes->ExactMatch(pR2Id);
    // TODO: Fehlerbehandlung verbessern
    NextIter = pRoutesIter->Next();
    assert(NextIter); // no query with side effects, please!
    TupleIdentifier *pR2RC = new TupleIdentifier(true, pRoutesIter->GetId());
    pNewJunction->PutAttribute(JUNCTION_ROUTE2_RC, pR2RC);
    delete pRoutesIter;

    // Pointers to sections are filled in FillSections
    pNewJunction->PutAttribute(JUNCTION_SECTION_AUP_RC,
                                   new TupleIdentifier(false));
    pNewJunction->PutAttribute(JUNCTION_SECTION_ADOWN_RC,
                                   new TupleIdentifier(false));
    pNewJunction->PutAttribute(JUNCTION_SECTION_BUP_RC,
                                   new TupleIdentifier(false));
    pNewJunction->PutAttribute(JUNCTION_SECTION_BDOWN_RC,
                                   new TupleIdentifier(false));

    /////////////////////////////////////////////////////////////////////
    //
    // Append new junction
    //
    pIntJunctions->AppendTuple(pNewJunction);

    pCurrentJunction->DeleteIfAllowed();
    pNewJunction->DeleteIfAllowed();
  }
  delete pJunctionsIter;

  /////////////////////////////////////////////////////////////////////
  //
  // Sort the table which is now containing all junctions
  //
  ostringstream xJunctionsStream;
  xJunctionsStream << (long)pIntJunctions;
  string strQuery = "(consume (sortby (feed (" + junctionsInternalTypeInfo +
                    " (ptr " + xJunctionsStream.str() +
                    "))) ((r1id asc)(meas1 asc))))";


  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  m_pJunctions = (Relation *)xResult.addr;

  // Delete internal table
  pIntJunctions->Delete();

  /////////////////////////////////////////////////////////////////////
  //
  // Create two b-trees for the junctions sorted by first and second id
  //
  ostringstream xThisJunctionsPtrStream;
  xThisJunctionsPtrStream << (long)m_pJunctions;
  strQuery = "(createbtree (" + junctionsInternalTypeInfo +
                " (ptr " + xThisJunctionsPtrStream.str() + "))" + " r1id)";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted); // no query with side effects, please!
  m_pBTreeJunctionsByRoute1 = (BTree*)xResult.addr;

  ostringstream xThisJunctionsPtrStream2;
  xThisJunctionsPtrStream2 << (long)m_pJunctions;
  strQuery = "(createbtree (" + junctionsInternalTypeInfo +
                " (ptr " + xThisJunctionsPtrStream2.str() + "))" + " r2id)";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted); // no query with side effects, please!
  m_pBTreeJunctionsByRoute2 = (BTree*)xResult.addr;
}

void Network::FillSections()
{
  // The method will iterate over routes
  GenericRelationIterator* pRoutesIt = m_pRoutes->MakeScan();

  // Create relation for sections
  ListExpr xType;
  nl->ReadFromString(sectionsInternalTypeInfo, xType);
  ListExpr xNumType = SecondoSystem::GetCatalog()->NumericType(xType);
  m_pSections = new Relation(xNumType);
  /////////////////////////////////////////////////////////////////////
  //
  // Iterate over all Routes
  //
  Tuple* pRoute;
  TupleId iSectionTid = 0;
  while((pRoute = pRoutesIt->GetNextTuple()) != 0)
  {
    // Current position on route - starting at the beginning of the route
    double dCurrentPosOnRoute = 0;
    SimpleLine* pRouteCurve = (SimpleLine*)pRoute->GetAttribute(ROUTE_CURVE);
    TupleId iTupleId = pRoute->GetTupleId();
    CcInt* xRouteId = (CcInt*)pRoute->GetAttribute(ROUTE_ID);
    int iRouteId = xRouteId->GetIntval();
    bool bDual = ((CcBool*)pRoute->GetAttribute(ROUTE_DUAL))->GetBoolval();
    /////////////////////////////////////////////////////////////////////
    //
    // We need to find all junctions belonging to this route
    //
    vector<JunctionSortEntry> xJunctions;
    GetJunctionsOnRoute(xRouteId,
                        xJunctions);

    /////////////////////////////////////////////////////////////////////
    //
    // Now that we found all relevant junctions we can iterate over them.
    //
    JunctionSortEntry xCurrentEntry;
    xCurrentEntry.m_pJunction = 0;
    xCurrentEntry.m_bFirstRoute = false;
    for(size_t i = 0; i < xJunctions.size(); i++)
    {
      // Get next junction
      xCurrentEntry = xJunctions[i];

      // Find values for the new section
      double dStartPos = dCurrentPosOnRoute;
      double dEndPos = xCurrentEntry.GetRouteMeas();

      // If the first junction is at the very start of the route, no
      // section will be added
      if(xCurrentEntry.GetRouteMeas() == 0)
      {
        vector<int> xIndices;
        vector<Attribute*> xAttrs;
        if(xCurrentEntry.m_bFirstRoute)
        {
          xIndices.push_back(JUNCTION_SECTION_ADOWN_RC);
          xAttrs.push_back(new TupleIdentifier(true, 0));
        }
        else
        {
          xIndices.push_back(JUNCTION_SECTION_BDOWN_RC);
          xAttrs.push_back(new TupleIdentifier(true, 0));
        }
        m_pJunctions->UpdateTuple(xCurrentEntry.m_pJunction,
                                  xIndices,
                                  xAttrs);
        continue;
      }

      /////////////////////////////////////////////////////////////////////
      //
      // Create a new section
      //
      // Section will only be created if the length is > 0. Otherwise the
      // one before remains valid.
      if(dEndPos - dStartPos > 0.01)
      {
        // A sline for the section
        SimpleLine* pLine = new SimpleLine(0);

        // Take start from the route
        bool bStartSmaller = ((CcBool*)pRoute->GetAttribute(
                                  ROUTE_STARTSSMALLER))->GetBoolval();

        pRouteCurve->SubLine(dStartPos,
                             dEndPos,
                             bStartSmaller,
                             *pLine);

        // Find out, if the orientation of the subline differs from the position
        // of the line. If so, the direction has to be changed.
        bool bLineStartsSmaller;
        Point* pStartPoint = new Point(false);
        pRouteCurve->AtPosition(dStartPos, bStartSmaller, *pStartPoint);
        Point* pEndPoint = new Point(false);
        pRouteCurve->AtPosition(dEndPos, bStartSmaller, *pEndPoint);
        if(pStartPoint->GetX() < pEndPoint->GetX() ||
           (
             pStartPoint->GetX() == pEndPoint->GetX() &&
             pStartPoint->GetY() < pEndPoint->GetY()
           )
          )
        {
          // Normal orientation
          bLineStartsSmaller = true;
        }
        else
        {
          // Opposite orientation
          bLineStartsSmaller = false;
        }

        // The new section
        Tuple* pNewSection = new Tuple(nl->Second(xNumType));
        pNewSection->PutAttribute(SECTION_RID, new CcInt(true, iRouteId));
        pNewSection->PutAttribute(SECTION_DUAL, new CcBool(true, bDual));
        pNewSection->PutAttribute(SECTION_MEAS1, new CcReal(true, dStartPos));
        pNewSection->PutAttribute(SECTION_MEAS2, new CcReal(true, dEndPos));
        pNewSection->PutAttribute(SECTION_RRC, new TupleIdentifier(true,
                                  iTupleId));
        pNewSection->PutAttribute(SECTION_CURVE, pLine);
        pNewSection->PutAttribute(SECTION_CURVE_STARTS_SMALLER,
                                  new CcBool(true, bLineStartsSmaller));
        pNewSection->PutAttribute(SECTION_SID,
                                new CcInt(true, m_pSections->GetNoTuples()+1));
        m_pSections->AppendTuple(pNewSection);
        iSectionTid++;
        pNewSection->DeleteIfAllowed();
        // Update position for next loop
        dCurrentPosOnRoute = dEndPos;
      }

      /////////////////////////////////////////////////////////////////////
      //
      // Store ID of new section in junction behind that section.
      //
      vector<int> xIndices;
      vector<Attribute*> xAttrs;
      if(xCurrentEntry.m_bFirstRoute)
      {
        xIndices.push_back(JUNCTION_SECTION_ADOWN_RC);
        xAttrs.push_back(new TupleIdentifier(true, iSectionTid));
      }
      else
      {
        xIndices.push_back(JUNCTION_SECTION_BDOWN_RC);
        xAttrs.push_back(new TupleIdentifier(true, iSectionTid));
      }
      m_pJunctions->UpdateTuple(xCurrentEntry.m_pJunction,
                                xIndices,
                                xAttrs);
      if (pRouteCurve->Length() - xCurrentEntry.GetRouteMeas() < 0.01) {
        vector<int> xIndices;
        vector<Attribute*> xAttrs;
        if(xCurrentEntry.m_bFirstRoute)
        {
          xIndices.push_back(JUNCTION_SECTION_AUP_RC);
          xAttrs.push_back(new TupleIdentifier(true, 0));
        }
        else
        {
          xIndices.push_back(JUNCTION_SECTION_BUP_RC);
          xAttrs.push_back(new TupleIdentifier(true, 0));
        }
        m_pJunctions->UpdateTuple(xCurrentEntry.m_pJunction,
                                  xIndices,
                                  xAttrs);
      }

    } // End junctions-loop
    /////////////////////////////////////////////////////////////////////
    //
    // The last section of the route is still missing, if the last
    // junction is not at the end of the route.
    //
    if(pRouteCurve->Length() - dCurrentPosOnRoute > 0.01 ||
       dCurrentPosOnRoute == 0.0)
    {
      // Find values for the new section
      int iRouteId = ((CcInt*)pRoute->GetAttribute(ROUTE_ID))->GetIntval();
      bool bDual = ((CcBool*)pRoute->GetAttribute(ROUTE_DUAL))->GetBoolval();
      double dStartPos = dCurrentPosOnRoute;
      double dEndPos = pRouteCurve->Length();
      TupleId iTupleId = pRoute->GetTupleId();

      // Calculate line
      SimpleLine* pLine = new SimpleLine(0);
      bool bStartSmaller = ((CcBool*)pRoute->GetAttribute(
                                ROUTE_STARTSSMALLER))->GetBoolval();
      pRouteCurve->SubLine(dStartPos,
                           dEndPos,
                           bStartSmaller,
                           *pLine);

      // Find out, if the orientation of the subline differs from the position
      // of the sline. If so, the direction has to be changed.
      bool bLineStartsSmaller;
      Point* pStartPoint = new Point(false);
      pRouteCurve->AtPosition(dStartPos, bStartSmaller, *pStartPoint);
      Point* pEndPoint = new Point(false);
      pRouteCurve->AtPosition(dEndPos, bStartSmaller, *pEndPoint);
      if(pStartPoint->GetX() < pEndPoint->GetX() ||
         (
           pStartPoint->GetX() == pEndPoint->GetX() &&
           pStartPoint->GetY() < pEndPoint->GetY()
         )
        )
      {
        // Normal orientation
        bLineStartsSmaller = true;
      }
      else
      {
        // Opposite orientation
        bLineStartsSmaller = false;
      }

      // Create a new Section
      Tuple* pNewSection = new Tuple(nl->Second(xNumType));
      pNewSection->PutAttribute(SECTION_RID, new CcInt(true, iRouteId));
      pNewSection->PutAttribute(SECTION_DUAL, new CcBool(true, bDual));
      pNewSection->PutAttribute(SECTION_MEAS1, new CcReal(true, dStartPos));
      pNewSection->PutAttribute(SECTION_MEAS2, new CcReal(true, dEndPos));
      pNewSection->PutAttribute(SECTION_RRC,
                                new TupleIdentifier(true, iTupleId));
      pNewSection->PutAttribute(SECTION_CURVE, pLine);
      pNewSection->PutAttribute(SECTION_CURVE_STARTS_SMALLER,
                                new CcBool(true, bLineStartsSmaller));
      pNewSection->PutAttribute(SECTION_SID,
                                new CcInt(true, m_pSections->GetNoTuples()+1));
      m_pSections->AppendTuple(pNewSection);
      iSectionTid++;
      pNewSection->DeleteIfAllowed();
      // Store ID of new section in Junction
      if(xCurrentEntry.m_pJunction != 0)
      {
        vector<int> xIndicesLast;
        vector<Attribute*> xAttrsLast;
        if(xCurrentEntry.m_bFirstRoute)
        {
          xIndicesLast.push_back(JUNCTION_SECTION_AUP_RC);
          xAttrsLast.push_back(new TupleIdentifier(true,
                                                   iSectionTid));
        }
        else
        {
          xIndicesLast.push_back(JUNCTION_SECTION_BUP_RC);
          xAttrsLast.push_back(new TupleIdentifier(true,
                                                   iSectionTid));
        }
        m_pJunctions->UpdateTuple(xCurrentEntry.m_pJunction,
                                  xIndicesLast,
                                  xAttrsLast);
      }
    } // end if
    ////////////////////////////////////////////////////////////////////
    //
    // Fill Up-Pointers of all sections but the last
    //
    vector<JunctionSortEntry> yJunctions;
    GetJunctionsOnRoute(xRouteId,
                        yJunctions);
    if (yJunctions.size() > 2) {
      for(int i = yJunctions.size()-2; i >= 0; i--)
      {
        // Get next junction
        JunctionSortEntry xEntry = yJunctions[i];
        JunctionSortEntry xEntryBehind = yJunctions[i + 1];

        vector<int> xIndices;
        if(xEntry.m_bFirstRoute)
        {
          xIndices.push_back(JUNCTION_SECTION_AUP_RC);
        }
        else
        {
          xIndices.push_back(JUNCTION_SECTION_BUP_RC);
        }
        vector<Attribute*> xAttrs;
        if(xEntryBehind.GetRouteMeas() - xEntry.GetRouteMeas() < 0.01 )
        {
          // Two junctions at the same place. In this case they do have
          // the same up-pointers
          if(xEntryBehind.m_bFirstRoute)
          {
            TupleId iTid = xEntryBehind.GetUpSectionId();
            xAttrs.push_back(new TupleIdentifier(true, iTid));
          }
          else
          {
            TupleId iTid = xEntryBehind.GetUpSectionId();
            xAttrs.push_back(new TupleIdentifier(true, iTid));
          }
        }
        else
        {
          // Junctions not on the same place. The down-pointer of the second is
          // the up-pointer of the first.
          if(xEntryBehind.m_bFirstRoute)
          {
            TupleId iTid = xEntryBehind.GetDownSectionId();
            xAttrs.push_back(new TupleIdentifier(true, iTid));
          }
          else
          {
            TupleId iTid = xEntryBehind.GetDownSectionId();
            xAttrs.push_back(new TupleIdentifier(true, iTid));
          }
        }
        m_pJunctions->UpdateTuple(xEntry.m_pJunction,
                                  xIndices,
                                  xAttrs);
      }
    } else {
      if (yJunctions.size() == 2) {
        JunctionSortEntry xEntry = yJunctions[0];
        JunctionSortEntry xEntryBehind = yJunctions[1];
        vector<int> xIndices;
        if(xEntry.m_bFirstRoute)
        {
          xIndices.push_back(JUNCTION_SECTION_AUP_RC);
        }
        else
        {
          xIndices.push_back(JUNCTION_SECTION_BUP_RC);
        }
        vector<Attribute*> xAttrs;
        if (fabs(xEntry.GetRouteMeas() - xEntryBehind.GetRouteMeas()) < 0.01) {
          if(xEntryBehind.m_bFirstRoute) {
            TupleId iTid = xEntryBehind.GetUpSectionId();
            xAttrs.push_back(new TupleIdentifier(true, iTid));
          } else {
            TupleId iTid = xEntryBehind.GetUpSectionId();
            xAttrs.push_back(new TupleIdentifier(true, iTid));
          }
          m_pJunctions->UpdateTuple(xEntry.m_pJunction,
                                      xIndices,
                                      xAttrs);
        } else {
          if(xEntryBehind.m_bFirstRoute)
            {
              TupleId iTid = xEntryBehind.GetDownSectionId();
              xAttrs.push_back(new TupleIdentifier(true, iTid));
            }
            else
            {
              TupleId iTid = xEntryBehind.GetDownSectionId();
              xAttrs.push_back(new TupleIdentifier(true, iTid));
            }
            m_pJunctions->UpdateTuple(xEntry.m_pJunction,
                                      xIndices,
                                      xAttrs);
        }
      }
    }
    pRoute->DeleteIfAllowed();
  } // End while Routes
  delete pRoutesIt;
  // Create B-Tree for the sections
  Word xResult;
  ostringstream xThisSectionsPtrStream;
  xThisSectionsPtrStream << (long)m_pSections;
  string strQuery = "(createbtree (" + sectionsInternalTypeInfo +
                " (ptr " + xThisSectionsPtrStream.str() + "))" + " rid)";
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted); // no query with side effects, please!
  m_pBTreeSectionsByRoute = (BTree*)xResult.addr;
}

void Network::FillAdjacencyLists()
{
  // Adjust the adjacenzy list to the correct size. From each
  // section four directions are possible - including u-turns
  m_xAdjacencyList.Resize(m_pSections->GetNoTuples() * 2);
  for(int i = 0; i < m_pSections->GetNoTuples() * 2; i++)
  {
    m_xAdjacencyList.Put(i, AdjacencyListEntry(-1, -1));
  }

  GenericRelationIterator* pJunctionsIt = m_pJunctions->MakeScan();
  Tuple* pCurrentJunction;

  /////////////////////////////////////////////////////////////////////////
  //
  // In a first step all pairs of adjacent sections will be collected
  //
  vector<DirectedSectionPair> xList;
  while((pCurrentJunction = pJunctionsIt->GetNextTuple()) != 0)
  {
    //////////////////////////////////
    //
    // Retrieve the connectivity code
    //
    int iCc =
        ((CcInt*)pCurrentJunction->GetAttribute(JUNCTION_CC))->GetIntval();
    ConnectivityCode xCc(iCc);

    //////////////////////////////////
    //
    // Retrieve the four sections - if they exist
    //
    // (This should also be possible without loading the Section itself)
    //
    /*TupleIdentifier* pTid;
    Tuple* pAUp = 0;
    Tuple* pADown = 0;
    Tuple* pBUp = 0;
    Tuple* pBDown = 0;*/
    TupleId tidpAUp, tidpADown, tidpBUp, tidpBDown;

    tidpAUp = ((TupleIdentifier*)
      pCurrentJunction->GetAttribute(JUNCTION_SECTION_AUP_RC))->GetTid();
    tidpADown = ((TupleIdentifier*)
        pCurrentJunction->GetAttribute(JUNCTION_SECTION_ADOWN_RC))->GetTid();
    tidpBUp = ((TupleIdentifier*)
        pCurrentJunction->GetAttribute(JUNCTION_SECTION_BUP_RC))->GetTid();
    tidpBDown = ((TupleIdentifier*)
        pCurrentJunction->GetAttribute(JUNCTION_SECTION_BDOWN_RC))->GetTid();
    
    /*
    // First section
    Attribute* pAttr = pCurrentJunction->GetAttribute(JUNCTION_SECTION_AUP_RC);
    pTid = (TupleIdentifier*)pAttr;
    if(pTid->GetTid() > 0)
    {
      pAUp = m_pSections->GetTuple(pTid->GetTid());
    }

    // Second section
    pAttr = pCurrentJunction->GetAttribute(JUNCTION_SECTION_ADOWN_RC);
    pTid = (TupleIdentifier*)pAttr;
    if(pTid->GetTid() > 0)
    {
      pADown = m_pSections->GetTuple(pTid->GetTid());
    }

    // Third section
    pAttr = pCurrentJunction->GetAttribute(JUNCTION_SECTION_BUP_RC);
    pTid = (TupleIdentifier*)pAttr;
    if(pTid->GetTid() > 0)
    {
      pBUp = m_pSections->GetTuple(pTid->GetTid());
    }

    // Fourth section
    pAttr = pCurrentJunction->GetAttribute(JUNCTION_SECTION_BDOWN_RC);
    pTid = (TupleIdentifier*)pAttr;
    if(pTid->GetTid() > 0)
    {
      pBDown = m_pSections->GetTuple(pTid->GetTid());
    }
   */
    //////////////////////////////////
    //
    // If a section is existing and the transition is possible
    // it will be added to the list.
    //
    FillAdjacencyPair(tidpAUp, false, tidpAUp, true, xCc, AUP_AUP, xList);
    FillAdjacencyPair(tidpAUp, false, tidpADown, false, xCc, AUP_ADOWN, xList);
    FillAdjacencyPair(tidpAUp, false, tidpBUp, true, xCc, AUP_BUP, xList);
    FillAdjacencyPair(tidpAUp, false, tidpBDown, false, xCc, AUP_BDOWN, xList);

    FillAdjacencyPair(tidpADown, true, tidpAUp, true, xCc, ADOWN_AUP, xList);
    FillAdjacencyPair(tidpADown, true, tidpADown, false, xCc,ADOWN_ADOWN,xList);
    FillAdjacencyPair(tidpADown, true, tidpBUp, true, xCc, ADOWN_BUP, xList);
    FillAdjacencyPair(tidpADown, true, tidpBDown, false, xCc,ADOWN_BDOWN,xList);

    FillAdjacencyPair(tidpBUp, false, tidpAUp, true, xCc, BUP_AUP, xList);
    FillAdjacencyPair(tidpBUp, false, tidpADown, false, xCc, BUP_ADOWN, xList);
    FillAdjacencyPair(tidpBUp, false, tidpBUp, true, xCc, BUP_BUP, xList);
    FillAdjacencyPair(tidpBUp, false, tidpBDown, false, xCc, BUP_BDOWN, xList);

    FillAdjacencyPair(tidpBDown, true, tidpAUp, true, xCc, BDOWN_AUP, xList);
    FillAdjacencyPair(tidpBDown, true, tidpADown, false, xCc,BDOWN_ADOWN,xList);
    FillAdjacencyPair(tidpBDown, true, tidpBUp, true, xCc,BDOWN_BUP, xList);
    FillAdjacencyPair(tidpBDown, true, tidpBDown, false, xCc,BDOWN_BDOWN,xList);
    /*
    // First section
    FillAdjacencyPair(pAUp, false, pAUp, true, xCc, AUP_AUP, xList);
    FillAdjacencyPair(pAUp, false, pADown, false, xCc, AUP_ADOWN, xList);
    FillAdjacencyPair(pAUp, false, pBUp, true, xCc, AUP_BUP, xList);
    FillAdjacencyPair(pAUp, false, pBDown, false, xCc, AUP_BDOWN, xList);

    // Second section
    FillAdjacencyPair(pADown, true, pAUp, true, xCc, ADOWN_AUP, xList);
    FillAdjacencyPair(pADown, true, pADown, false, xCc, ADOWN_ADOWN, xList);
    FillAdjacencyPair(pADown, true, pBUp, true, xCc, ADOWN_BUP, xList);
    FillAdjacencyPair(pADown, true, pBDown, false, xCc, ADOWN_BDOWN, xList);

    // Third section
    FillAdjacencyPair(pBUp, false, pAUp, true, xCc, BUP_AUP, xList);
    FillAdjacencyPair(pBUp, false, pADown, false, xCc, BUP_ADOWN, xList);
    FillAdjacencyPair(pBUp, false, pBUp, true, xCc, BUP_BUP, xList);
    FillAdjacencyPair(pBUp, false, pBDown, false, xCc, BUP_BDOWN, xList);

    // Fourth section
    FillAdjacencyPair(pBDown, true, pAUp, true, xCc, BDOWN_AUP, xList);
    FillAdjacencyPair(pBDown, true, pADown, false, xCc, BDOWN_ADOWN, xList);
    FillAdjacencyPair(pBDown, true, pBUp, true, xCc, BDOWN_BUP, xList);
    FillAdjacencyPair(pBDown, true, pBDown, false, xCc, BDOWN_BDOWN, xList);
    */
    pCurrentJunction->DeleteIfAllowed();
  }
  delete pJunctionsIt;


  /////////////////////////////////////////////////////////////////////////
  //
  // Now - as the second step the adjacency lists are filled.
  //
  // Sort the list by the first directed section
  sort(xList.begin(),
       xList.end());

  DirectedSectionPair xLastPair;
  int iLow = 0;
  for(size_t i = 0; i < xList.size(); i++)
  {
    // Get next
    DirectedSectionPair xPair = xList[i];
    if (i == 0){
      // Append new entry to sub-list
      m_xSubAdjacencyList.Append(DirectedSection(xPair.m_iSecondSectionTid,
                                                 xPair.m_bSecondUpDown));
      xLastPair = xPair;
    }
    // Entry in adjacency list if all sections adjacent to one section have
    // been found. This is the case every time the first section changes. Never
    // at the first entry and always at the last.
    if(i == xList.size() -1 ||
       (
        i != 0 &&
        (
         xLastPair.m_iFirstSectionTid != xPair.m_iFirstSectionTid ||
         xLastPair.m_bFirstUpDown != xPair.m_bFirstUpDown
        )
       )
      )
    {
      int iHigh = m_xSubAdjacencyList.Size()-1;
      Tuple *pSect = m_pSections->GetTuple(xLastPair.m_iFirstSectionTid);
      int iSectId = ((CcInt*) pSect->GetAttribute(SECTION_SID))->GetIntval();
      pSect->DeleteIfAllowed();
      int iIndex = 2 * (iSectId-1);
      iIndex += xLastPair.m_bFirstUpDown ? 1 : 0;
      m_xAdjacencyList.Put(iIndex, AdjacencyListEntry(iLow, iHigh));
      iLow = iHigh + 1;
    }

    // Check if entry allready exists in list. As the list is sorted it
    // has to be the entry before.
    if(i == 0 ||
       xLastPair.m_iFirstSectionTid != xPair.m_iFirstSectionTid ||
       xLastPair.m_bFirstUpDown != xPair.m_bFirstUpDown ||
       xLastPair.m_iSecondSectionTid != xPair.m_iSecondSectionTid ||
       xLastPair.m_bSecondUpDown != xPair.m_bSecondUpDown)
    {
      // Append new entry to sub-list
      m_xSubAdjacencyList.Append(DirectedSection(xPair.m_iSecondSectionTid,
                                                 xPair.m_bSecondUpDown));
    }
    xLastPair = xPair;
  }
}

/*void Network::FillAdjacencyPair(Tuple* in_pFirstSection,
                                    bool in_bFirstUp,
                                    Tuple* in_pSecondSection,
                                    bool in_bSecondUp,
                                    ConnectivityCode in_xCc,
                                    Transition in_xTransition,
                                    vector<DirectedSectionPair> &inout_xPairs)*/
  void Network::FillAdjacencyPair(TupleId in_pFirstSection,
                                  bool in_bFirstUp,
                                  TupleId in_pSecondSection,
                                  bool in_bSecondUp,
                                  ConnectivityCode in_xCc,
                                  Transition in_xTransition,
                                  vector<DirectedSectionPair> &inout_xPairs)
{
  if(in_pFirstSection != 0 &&
     in_pSecondSection != 0 &&
     in_xCc.IsPossible(in_xTransition))
    {
      inout_xPairs.push_back(DirectedSectionPair(in_pFirstSection,
                                                in_bFirstUp,
                                                in_pSecondSection,
                                                in_bSecondUp));
    }
}

bool Network::InShortestPath(GPoint*start,GPoint *to, GLine *result)
{
  GPoint* end = new GPoint(*to);//copy the gpoint
  result->Clear();
  if (start == 0 || end == 0 || !start->IsDefined() ||
      !end->IsDefined()) {
     sendMessage("Both gpoints must exist and be defined.");
     result->SetDefined(false);

     return false;
  }
  // Check wether both points belong to the same network
  if(start->GetNetworkId() != end->GetNetworkId())
  {
    sendMessage("Both gpoints belong to different networks.");
    result->SetDefined(false);

    return false;
  }

  result->SetNetworkId(GetId());

  // Get sections where the path should start or end
  Tuple* startSection = GetSectionOnRoute(start);
  if (startSection == 0) {
    sendMessage("Starting GPoint not found in network.");
    result->SetDefined(false);

    return false;
  }
  Tuple* endSection = GetSectionOnRoute(end);
  if (endSection == 0) {
    sendMessage("End GPoint not found in network.");
    startSection->DeleteIfAllowed();
    result->SetDefined(false);

    return false;
  }
////////////////////////////////////////////////////
  bool junctionpoint = false;
  Point* endp = new Point();
  GetPointOnRoute(end,endp); //end point
  vector<JunctionSortEntry> juns;
  CcInt* routeid = new CcInt(true,end->GetRouteId());
  GetJunctionsOnRoute(routeid,juns);
  for(unsigned int i = 0;i < juns.size();i++){
    Tuple* t = juns[i].m_pJunction;
    if(((CcInt*)t->GetAttribute(JUNCTION_ROUTE1_ID))->GetIntval() ==
        end->GetRouteId() &&
      fabs(((CcReal*)t->GetAttribute(JUNCTION_ROUTE1_MEAS))->GetRealval()-
      end->GetPosition()) < 0.1)
      junctionpoint = true;
    if(((CcInt*)t->GetAttribute(JUNCTION_ROUTE2_ID))->GetIntval() ==
        end->GetRouteId() &&
      fabs(((CcReal*)t->GetAttribute(JUNCTION_ROUTE2_MEAS))->GetRealval()-
      end->GetPosition()) < 0.1)
      junctionpoint = true;
  }
  vector<TupleId> secjunid;
  if(junctionpoint){//it is a junction point
    vector<DirectedSection> sectionlist;
    if(fabs(end->GetPosition()-
      ((CcReal*)endSection->GetAttribute(SECTION_MEAS1))->GetRealval())< 0.1)
      GetAdjacentSections(endSection->GetTupleId(),false,sectionlist);
    else
      GetAdjacentSections(endSection->GetTupleId(),true,sectionlist);
    for(unsigned int i = 0;i < sectionlist.size();i++){
      if(sectionlist[i].GetSectionTid() != endSection->GetTupleId())
        secjunid.push_back(sectionlist[i].GetSectionTid());
    }
  }
  delete endp;
  delete routeid;
/////////////////////////////////////////////////////
// Calculate the shortest path using dijkstras algorithm.



  int startSectTID = startSection->GetTupleId();
  int lastSectTID = endSection->GetTupleId();

  if (startSectTID == lastSectTID) {
    result->AddRouteInterval(start->GetRouteId(), start->GetPosition(),
                            end->GetPosition());
  } else {

//Initialize PriorityQueue

    PrioQueue *prioQ = new PrioQueue (0);
    SectIDTree *visitedSect = 0;
    double sectMeas1 =
        ((CcReal*) startSection->GetAttribute(SECTION_MEAS1))->GetRealval();
    double sectMeas2 =
        ((CcReal*) startSection->GetAttribute(SECTION_MEAS2))->GetRealval();
    double dist = 0.0;
    vector<DirectedSection> adjSectionList;
    adjSectionList.clear();
    if (start->GetSide() == 0) {
      dist = start->GetPosition() - sectMeas1;
      GetAdjacentSections(startSectTID, false, adjSectionList);
      SectIDTree *startTree = new SectIDTree(startSectTID,
                                        (TupleId) numeric_limits<long>::max(),
                                        false,
                                        numeric_limits<int>::max());
      visitedSect = startTree;
      for(size_t i = 0;  i < adjSectionList.size(); i++) {
        DirectedSection actNextSect = adjSectionList[i];
        if (actNextSect.GetSectionTid() != startSectTID) {
          PQEntry *actEntry = new PQEntry(actNextSect.GetSectionTid(), dist,
                               actNextSect.GetUpDownFlag(),
                               startSectTID);
          prioQ->Insert(*actEntry, visitedSect) ;
          delete actEntry;
        }
      }
      adjSectionList.clear();
    } else {
      if (start->GetSide() == 1) {
        dist = sectMeas2 - start->GetPosition();
        SectIDTree *startTree = new SectIDTree(startSectTID,
                                          (TupleId) numeric_limits<long>::max(),
                                          true,
                                             numeric_limits<int>::max());
        visitedSect = startTree;
        GetAdjacentSections(startSectTID, true, adjSectionList);
        for(size_t i = 0;  i < adjSectionList.size(); i++) {
          DirectedSection actNextSect = adjSectionList[i];
          if (actNextSect.GetSectionTid() != startSectTID) {
            PQEntry *actEntry = new PQEntry(actNextSect.GetSectionTid(), dist,
                                  actNextSect.GetUpDownFlag(),
                                  startSectTID);
            prioQ->Insert(*actEntry, visitedSect);
            delete actEntry;
          }
        }
        adjSectionList.clear();
      } else {
        dist = start->GetPosition() - sectMeas1;
        GetAdjacentSections(startSectTID, false, adjSectionList);
        bool first = true;
        for(size_t i = 0;  i < adjSectionList.size(); i++) {
          DirectedSection actNextSect = adjSectionList[i];
          if (actNextSect.GetSectionTid() != startSectTID) {
            if (first) {
              first = false;
              SectIDTree *startTree = new SectIDTree(startSectTID,
                                          (TupleId) numeric_limits<long>::max(),
                                          false,
                                             numeric_limits<int>::max());
              visitedSect = startTree;
            }
            PQEntry *actEntry = new PQEntry(actNextSect.GetSectionTid(), dist,
                                 actNextSect.GetUpDownFlag(),
                                 startSectTID);
            prioQ->Insert(*actEntry, visitedSect);
            delete actEntry;
          }
        }
        adjSectionList.clear();
        dist = sectMeas2 -start->GetPosition();
        GetAdjacentSections(startSectTID, true, adjSectionList);
        for(size_t i = 0;  i < adjSectionList.size(); i++) {
          DirectedSection actNextSect = adjSectionList[i];
          if (actNextSect.GetSectionTid() != startSectTID) {
            if (first) {
              first = false;
              SectIDTree *startTree = new SectIDTree(startSectTID,
                                        (TupleId) numeric_limits<long>::max(),
                                        true,
                                             numeric_limits<int>::max());
              visitedSect = startTree;
            }
            PQEntry *actEntry = new PQEntry(actNextSect.GetSectionTid(), dist,
                                        actNextSect.GetUpDownFlag(),
                                        startSectTID);
            prioQ->Insert(*actEntry, visitedSect);
            delete actEntry;
          }
        }
        adjSectionList.clear();
      }
    }
// Use priorityQueue to find shortestPath.

    PQEntry *actPQEntry;
    bool found = false;
    while (!prioQ->IsEmpty() && !found){
      actPQEntry = prioQ->GetAndDeleteMin(visitedSect);
      Tuple *actSection = GetSection(actPQEntry->sectID);
      sectMeas1 =
        ((CcReal*) actSection->GetAttribute(SECTION_MEAS1))->GetRealval();
      sectMeas2 =
        ((CcReal*) actSection->GetAttribute(SECTION_MEAS2))->GetRealval();
      dist = actPQEntry->distFromStart + fabs(sectMeas2 - sectMeas1);

//////////////////////////////////////
      if(junctionpoint){ //end point is a junction point
        for(unsigned int i = 0;i < secjunid.size();i++){
          if(secjunid[i] == actPQEntry->sectID){
            lastSectTID = actPQEntry->sectID;
            Tuple* sect = GetSection(lastSectTID);
            double m1 =
                  ((CcReal*)sect->GetAttribute(SECTION_MEAS1))->GetRealval();
            double m2 =
                  ((CcReal*)sect->GetAttribute(SECTION_MEAS2))->GetRealval();
            if(actPQEntry->upDownFlag){
              GPoint* temp = new GPoint(true,end->GetNetworkId(),
                        end->GetRouteId(),m2,None);
              *end = *temp;
              delete temp;
            }else{
              GPoint* temp = new GPoint(true,end->GetNetworkId(),
                        end->GetRouteId(),m1,None);
              *end = *temp;
              delete temp;
            }
            sect->DeleteIfAllowed();
            break;
          }
        }
      }
////////////////////////////////////

      if (actPQEntry->sectID != lastSectTID) {
//Search further in the network unitl reached last section.
//Get adjacent sections and insert into priority Queue.

        adjSectionList.clear();
        GetAdjacentSections(actPQEntry->sectID,
                                      actPQEntry->upDownFlag,
                                      adjSectionList);
        for (size_t i = 0; i <adjSectionList.size();i++){
          DirectedSection actNextSect = adjSectionList[i];
          if (actNextSect.GetSectionTid() != actPQEntry->sectID) {
            PQEntry *actEntry = new PQEntry(actNextSect.GetSectionTid(),
                                        dist,
                                        actNextSect.GetUpDownFlag(),
                                        actPQEntry->sectID);
            prioQ->Insert(*actEntry, visitedSect);
            delete actEntry;
          }
        }
        delete actPQEntry;
        actSection->DeleteIfAllowed();
      } else {

// Shortest Path found.
// Compute last route interval and resulting gline.

        found = true;
        double startRI, endRI;
        int actRouteId =
            ((CcInt*)actSection->GetAttribute(SECTION_RID))->GetIntval();
        if (actPQEntry->upDownFlag == true) {
          startRI = sectMeas1;
          endRI = end->GetPosition();
        } else {
          startRI = sectMeas2;
          endRI = end->GetPosition();
        }

        actSection->DeleteIfAllowed();

//Get the sections used for shortest path and write them in right
//order (from start to end ) in the resulting gline. Because dijkstra gives
//the sections from end to start we first have to put the result sections on a
//stack to turn in right order.

        RIStack *riStack = new RIStack(actRouteId, startRI, endRI);
        int lastSectId = actPQEntry->sectID;
        SectIDTree *pElem = visitedSect->Find(actPQEntry->beforeSectID);
        bool end = false;
        bool upDown;
     //   if (startRI >= endRI) upDown = false;
        if(startRI > endRI || fabs(startRI-endRI) < 0.1) upDown = false;
        else upDown = true;
        while (!end) {
          //GetSection
          actSection = GetSection(pElem->sectID);
          actRouteId =
              ((CcInt*)actSection->GetAttribute(SECTION_RID))->GetIntval();
          sectMeas1 =
            ((CcReal*) actSection->GetAttribute(SECTION_MEAS1))->GetRealval();
          sectMeas2 =
            ((CcReal*) actSection->GetAttribute(SECTION_MEAS2))->GetRealval();
          upDown = pElem->upDownFlag;
          if (pElem->sectID != startSectTID){
            if (upDown)
              riStack->Push(actRouteId, sectMeas1, sectMeas2, riStack);
            else
              riStack->Push(actRouteId, sectMeas2, sectMeas1, riStack);
            lastSectId = pElem->sectID;
            pElem = visitedSect->Find(pElem->beforeSectId);
          } else {
            end = true;
            GetAdjacentSections(startSectTID, true, adjSectionList);
            size_t i = 0;
            bool stsectfound = false;
            while (i < adjSectionList.size() && !stsectfound) {
              DirectedSection adjSection = adjSectionList[i];
              if (adjSection.GetSectionTid() == lastSectId){
                  if(fabs(start->GetPosition()-sectMeas2) > 0.1){
                    stsectfound = true;
                    riStack->Push(actRouteId, start->GetPosition(), sectMeas2,
                              riStack);
                    end = true;
                  }
              }
              i++;
            }
            adjSectionList.clear();
            if (!stsectfound) {
              GetAdjacentSections(startSectTID, false,
                                            adjSectionList);
              i = 0;
              while (i < adjSectionList.size() && !stsectfound) {
                DirectedSection adjSection = adjSectionList[i];
                if (adjSection.GetSectionTid() == lastSectId){
                    if(fabs(start->GetPosition() - sectMeas1) > 0.1){
                      stsectfound = true;
                      riStack->Push(actRouteId, start->GetPosition(), sectMeas1,
                                riStack);
                      end = true;
                    }
                }
                i++;
              }
              adjSectionList.clear();
            }
          }
        }
        // Cleanup and return result
        riStack->StackToGLine(result);
        riStack->RemoveStack();
        delete actPQEntry;
      }
    }
    visitedSect->Remove();
    prioQ->Destroy();
    delete prioQ;
  }
  startSection->DeleteIfAllowed();
  endSection->DeleteIfAllowed();
  result->SetSorted(false);
  result->SetDefined(true);
  delete end;
  return true;
};

void Network::FindSP(TupleId j1,TupleId j2,double& length,GLine* res)
{
  res->SetNetworkId(GetId());
  for(int i = 1; i <= alldistance->GetNoTuples();i++){
    Tuple* tuple = alldistance->GetTuple(i);
    TupleId jun1 = ((CcInt*)tuple->GetAttribute(0))->GetIntval();
    TupleId jun2 = ((CcInt*)tuple->GetAttribute(1))->GetIntval();
    if((jun1 == j1 && jun2 == j2) ||
       (jun1 ==j2 && jun2 == j1)){
      length = ((CcReal*)tuple->GetAttribute(2))->GetRealval();
      GLine* gline = (GLine*)tuple->GetAttribute(3);
      *res = *gline;
      tuple->DeleteIfAllowed();
      break;
    }
    tuple->DeleteIfAllowed();
  }
}

void Network::FillDistanceStorage()
{
  ListExpr xType;
  nl->ReadFromString(distancestorageTypeInfo,xType);
  ListExpr xNumType = SecondoSystem::GetCatalog()->NumericType(xType);
  alldistance = new Relation(xNumType);
  //store the distance between each two junction points

  for(int i = 1;i <= m_pJunctions->GetNoTuples();i++){
    for(int j = i+1; j <= m_pJunctions->GetNoTuples();j++){
      Tuple* jun1 = m_pJunctions->GetTuple(i);
      Tuple* jun2 = m_pJunctions->GetTuple(j);
      int rid1 = ((CcInt*)jun1->GetAttribute(JUNCTION_ROUTE1_ID))->GetIntval();
      int rid2 = ((CcInt*)jun2->GetAttribute(JUNCTION_ROUTE1_ID))->GetIntval();
      float pos1 =
              ((CcReal*)jun1->GetAttribute(JUNCTION_ROUTE1_MEAS))->GetRealval();
      float pos2 =
              ((CcReal*)jun2->GetAttribute(JUNCTION_ROUTE1_MEAS))->GetRealval();
      Side side = None;
      Point* p1 = (Point*)jun1->GetAttribute(JUNCTION_POS);
      Point* p2 = (Point*)jun2->GetAttribute(JUNCTION_POS);
      if(fabs(p1->GetX()-p2->GetX()) < 0.1 &&
         fabs(p1->GetY()-p2->GetY()) < 0.1) //different junction point
        continue;
      GPoint* gp1 = new GPoint(true,GetId(),rid1,pos1,side);
      GPoint* gp2 = new GPoint(true,GetId(),rid2,pos2,side);
      Tuple* tuple = new Tuple(nl->Second(xNumType));
      tuple->PutAttribute(0,new TupleIdentifier(true,i));
      tuple->PutAttribute(1,new TupleIdentifier(true,j));
      GLine* gline = new GLine(0);
      assert(InShortestPath(gp1,gp2,gline));
      tuple->PutAttribute(2,new CcReal(true,gline->GetLength()));
      GLine* temp = new GLine(0);
      temp->SetNetworkId(gline->GetNetworkId());
      const RouteInterval* ri;
      gline->Get(0,ri);
      temp->AddRouteInterval(const_cast<RouteInterval*>(ri));//head
      gline->Get(gline->Size()-1,ri);
      temp->AddRouteInterval(const_cast<RouteInterval*>(ri));//tail
      tuple->PutAttribute(3,new GLine(temp));
      delete temp;
      delete gline;
      alldistance->AppendTuple(tuple);
      tuple->DeleteIfAllowed();
      delete gp1;
      delete gp2;
    }
  }
}

int Network::GetId()
{
  return m_iId;
}

Relation *Network::GetRoutes()
{
  return m_pRoutes;
}


Relation *Network::GetJunctions()
{
  ostringstream strJunctionsPtr;
  strJunctionsPtr << (long)m_pJunctions;

  string querystring = "(consume (feed (" + junctionsInternalTypeInfo +
                       " (ptr " + strJunctionsPtr.str() + "))))";

  Word resultWord;
  int QueryExecuted = QueryProcessor::ExecuteQuery(querystring, resultWord);
  assert(QueryExecuted); // no ASSERT with side effects, please
  return (Relation *)resultWord.addr;
}


void Network::GetJunctionsOnRoute(CcInt* in_pRouteId,
                                  vector<JunctionSortEntry> &inout_xJunctions)
{
  BTreeIterator* pJunctionsIt;
  pJunctionsIt = m_pBTreeJunctionsByRoute1->ExactMatch(in_pRouteId);
  while(pJunctionsIt->Next())
  {
    // Get next junction
    Tuple* pCurrentJunction = m_pJunctions->GetTuple(pJunctionsIt->GetId());
    inout_xJunctions.push_back(JunctionSortEntry(true, pCurrentJunction));
  }
  delete pJunctionsIt;

  // Now we look up the second b-tree
  pJunctionsIt = m_pBTreeJunctionsByRoute2->ExactMatch(in_pRouteId);
  while(pJunctionsIt->Next())
  {
    Tuple* pCurrentJunction = m_pJunctions->GetTuple(pJunctionsIt->GetId());
    inout_xJunctions.push_back(JunctionSortEntry(false, pCurrentJunction));
  }
  delete pJunctionsIt;

  // The junctions will be sorted by their mesure on the relevant route.
  sort(inout_xJunctions.begin(),
       inout_xJunctions.end());
}

Tuple* Network::GetSection(TupleId n) {
  return m_pSections->GetTuple(n);
}

TupleId Network::GetTupleIdSectionOnRoute(GPoint* in_xGPoint) {
  CcInt *ciRouteId = new CcInt(true, in_xGPoint->GetRouteId());
  BTreeIterator* pSectionIter =
      m_pBTreeSectionsByRoute->ExactMatch(ciRouteId);
  delete ciRouteId;
  Tuple *actSect = 0;
  TupleId result;
  while (pSectionIter->Next()){
    result = pSectionIter->GetId();
    actSect =
        m_pSections->GetTuple(pSectionIter->GetId());
    if(actSect != 0){
      double start =
            ((CcReal*)actSect->GetAttribute(SECTION_MEAS1))->GetRealval();
      double end =
            ((CcReal*) actSect->GetAttribute(SECTION_MEAS2))->GetRealval();
      if (in_xGPoint->GetPosition() >= start&&in_xGPoint->GetPosition() <= end)
      {
        delete pSectionIter;
        actSect->DeleteIfAllowed();
        return result;
      }
      else {
        if (fabs(in_xGPoint->GetPosition() - start) <= 0.01) {
          delete pSectionIter;
          actSect->DeleteIfAllowed();
          return result;
        }
        else {
          if (fabs(in_xGPoint->GetPosition() - end) <= 0.01) {
            Tuple *pRoute = GetRoute(((TupleIdentifier*)
                actSect->GetAttribute(SECTION_RRC))->GetTid());
            if (fabs(((CcReal*)
                pRoute->GetAttribute(ROUTE_LENGTH))->GetRealval()
                - end) <= 0.01){
              pRoute->DeleteIfAllowed();
              delete pSectionIter;
              actSect->DeleteIfAllowed();
              return result;
                } else {
                  pRoute->DeleteIfAllowed();
                }
          }
        }
      }
      actSect->DeleteIfAllowed();
    }
  }
  delete pSectionIter;
  return -1;
}

Tuple* Network::GetSectionOnRoute(GPoint* in_xGPoint)
{
 /*
New implementation using sectionsBTree

*/

  CcInt *ciRouteId = new CcInt(true, in_xGPoint->GetRouteId());
  BTreeIterator* pSectionIter =
      m_pBTreeSectionsByRoute->ExactMatch(ciRouteId);
  delete ciRouteId;
  Tuple *actSect = 0;
  while (pSectionIter->Next()){
    actSect =
      m_pSections->GetTuple(pSectionIter->GetId());
    if(actSect != 0){
      double start =
        ((CcReal*)actSect->GetAttribute(SECTION_MEAS1))->GetRealval();
      double end =
        ((CcReal*) actSect->GetAttribute(SECTION_MEAS2))->GetRealval();
      if (in_xGPoint->GetPosition() >= start&&in_xGPoint->GetPosition() <= end)
      {
        delete pSectionIter;
        return actSect;
      }
      else {
        if (fabs(in_xGPoint->GetPosition() - start) <= 0.01) {
          delete pSectionIter;
          return actSect;
        }
        else {
          if (fabs(in_xGPoint->GetPosition() - end) <= 0.01) {
            Tuple *pRoute = GetRoute(((TupleIdentifier*)
                actSect->GetAttribute(SECTION_RRC))->GetTid());
            if (fabs(((CcReal*)
                pRoute->GetAttribute(ROUTE_LENGTH))->GetRealval()
                - end) <= 0.01){
              pRoute->DeleteIfAllowed();
              delete pSectionIter;
              return actSect;
            } else {
              pRoute->DeleteIfAllowed();
            }
          }
        }
      }
      actSect->DeleteIfAllowed();
    }
  }
  delete pSectionIter;
  return 0;


  /*
  vector<JunctionSortEntry> xJunctions;
  CcInt xRouteId(true, in_xGPoint->GetRouteId());
  GetJunctionsOnRoute(&xRouteId,
                      xJunctions);

  // Now that we found all relevant junctions we can iterate over them.
  int iSectionId = 0;
  double juncpos;
  for(size_t i = 0; i < xJunctions.size(); i++)
  {
    // Get next junction
    JunctionSortEntry xCurrentEntry = xJunctions[i];
    iSectionId = xCurrentEntry.GetDownSectionId();
    juncpos = xCurrentEntry.GetRouteMeas();
    if(juncpos > in_xGPoint->GetPosition())
    {
      break;
    }
    if (juncpos != 0 && fabs(juncpos - in_xGPoint->GetPosition()) < 0.01){
      break;
    }
    iSectionId = xCurrentEntry.GetUpSectionId();
  }
  for(size_t i = 0; i < xJunctions.size(); i++)
  {
    // Get next junction
    JunctionSortEntry xCurrentEntry = xJunctions[i];
    xCurrentEntry.m_pJunction->DeleteIfAllowed();

  }

  if(iSectionId == 0) return 0;
  else return m_pSections->GetTuple(iSectionId);
    */
}

Tuple* Network::GetRoute(int in_RouteId){
  CcInt* pRouteId = new CcInt(true, in_RouteId);
  BTreeIterator *pRoutesIter = m_pBTreeRoutes->ExactMatch(pRouteId);
  delete pRouteId;
  Tuple *pRoute = 0;
  if (pRoutesIter->Next())
    pRoute = m_pRoutes->GetTuple(pRoutesIter->GetId());
  assert(pRoute != 0);
  delete pRoutesIter;
  return pRoute;

}

void Network::GetSectionsOfRouteInterval(const RouteInterval *ri,
                                DBArray<SectTreeEntry> *io_SectionIds){
  double ristart = min(ri->m_dStart, ri->m_dEnd);
  double riend = max(ri->m_dStart, ri->m_dEnd);
  CcInt* ciRouteId = new CcInt(true, ri->m_iRouteId);
  BTreeIterator* pSectionIter =
      m_pBTreeSectionsByRoute->ExactMatch(ciRouteId);
  delete ciRouteId;
  Tuple *actSect;
  TupleId actSectTID;
  bool bsectstart = true;
  bool bsectend = true;
  while (pSectionIter->Next()) {
    actSectTID = pSectionIter->GetId();
    actSect = m_pSections->GetTuple(actSectTID);
    assert(actSect != 0);
    double start =
      ((CcReal*) actSect->GetAttribute(SECTION_MEAS1))->GetRealval();
    double end =
      ((CcReal*) actSect->GetAttribute(SECTION_MEAS2))->GetRealval();
    if ((ristart <= start && riend >= end) ||
        (start <= ristart && end >= ristart) ||
        (start <= riend && end >= riend)){
      if (start <= ristart) {
        start = ristart;
        bsectstart = false;
      }
      if (riend <= end) {
        end = riend;
        bsectend = false;
      }
      SectTreeEntry *sect =
          new SectTreeEntry(actSect->GetTupleId(), ri->m_iRouteId, start, end,
                            bsectstart, bsectend);
      io_SectionIds->Append(*sect);
      delete sect;
      if (riend <= end) {
        actSect->DeleteIfAllowed();
        break;
      }
    }
    actSect->DeleteIfAllowed();
  }
  delete pSectionIter;
};

void Network::GetPointOnRoute(const GPoint* in_pGPoint, Point*& res){
  /*Point *res = new Point(false);*/
  CcInt* pRouteId = new CcInt(true, in_pGPoint->GetRouteId());
  BTreeIterator* pRoutesIter = m_pBTreeRoutes->ExactMatch(pRouteId);
  delete pRouteId;
  Tuple *pRoute = 0;
  if (pRoutesIter->Next())
    pRoute = m_pRoutes->GetTuple(pRoutesIter->GetId());
  assert(pRoute != 0);
  SimpleLine* pLine = (SimpleLine*)pRoute->GetAttribute(ROUTE_CURVE);
  assert(pLine != 0);
  pLine->AtPosition(in_pGPoint->GetPosition(),true, *res);
  pRoute->DeleteIfAllowed();
  delete pRoutesIter;
  /*return res;*/
}

void Network::GetGPointsOnInterval(int iRouteId, double start, double end,
                            DBArray<GPointPoint> &gpp_list){
  const RouteInterval *ri = new RouteInterval(iRouteId, start, end);
  SimpleLine *sline = new SimpleLine(0);
  GetLineValueOfRouteInterval (ri, sline);
  delete ri;
  CcInt* pRouteId = new CcInt(true, iRouteId);
  BTreeIterator* pRoutesIter = m_pBTreeRoutes->ExactMatch(pRouteId);
  delete pRouteId;
  Tuple *pRoute = 0;
  if (pRoutesIter->Next())
    pRoute = m_pRoutes->GetTuple(pRoutesIter->GetId());
  assert(pRoute != 0);
  SimpleLine* routeCurve = (SimpleLine*)pRoute->GetAttribute(ROUTE_CURVE);
  assert(routeCurve != 0);
  const HalfSegment *hs;
  sline->Get(0,hs);
  Point p1 = hs->GetLeftPoint();
  Point p2 = hs->GetRightPoint();
  double pos1, pos2;
  bool ok = checkPoint(routeCurve, p1, true, pos1);
  GPoint gp1 = GPoint(true, GetId(), iRouteId, pos1);
  GPointPointTree *gpptree = new GPointPointTree(gp1, p1, 0,0);
  ok = checkPoint(routeCurve,p2, true, pos2);
  GPoint gp2 = GPoint(true, GetId(), iRouteId, pos2);
  gpptree->Insert(gp2, p2);
  for (int i=1; i < sline->Size() ; i++){
    sline->Get(i,hs);
    p1 = hs->GetLeftPoint();
    p2 = hs->GetRightPoint();
    ok = checkPoint(routeCurve, p1, true, pos1);
    gp1 = GPoint(true, GetId(), iRouteId, pos1);
    gpptree->Insert(gp1, p1);
    ok = checkPoint(routeCurve,p2, true, pos2);
    gp2 = GPoint(true, this->GetId(), iRouteId, pos2);
    gpptree->Insert(gp2, p2);
  }
  gpptree->TreeToVector(&gpp_list);
  gpptree->RemoveTree();
  delete pRoute;
  delete pRoutesIter;
  delete sline;

}

Relation* Network::GetSectionsInternal()
{
  return m_pSections;
}

Relation* Network::GetSections()
{
  ostringstream strSectionsPtr;
  strSectionsPtr << (long)m_pSections;

  string querystring = "(consume (feed (" + sectionsInternalTypeInfo +
                       " (ptr " + strSectionsPtr.str() + "))))";

  Word resultWord;
  int QueryExecuted = QueryProcessor::ExecuteQuery(querystring, resultWord);
  assert(QueryExecuted); // No ASSERT with side effect, please!
  return (Relation *)resultWord.addr;
}

void Network::GetAdjacentSections(TupleId in_iSectionTId,
                                  bool in_bUpDown,
                                  vector<DirectedSection> &inout_xSections)
{
  Tuple *pSect = GetSection(in_iSectionTId);
  int iSectionId = ((CcInt*)pSect->GetAttribute(SECTION_SID))->GetIntval();
  pSect->DeleteIfAllowed();
  int iIndex = 2 * (iSectionId-1)  + (in_bUpDown ? 1 : 0);
  const AdjacencyListEntry* xEntry = 0;
  m_xAdjacencyList.Get(iIndex, xEntry);
  if(xEntry->m_iHigh != -1)
  {
    int iLow = xEntry->m_iLow;
    int iHigh = xEntry->m_iHigh;

    for (int i = iLow; i <= iHigh; i++)
    {
      const DirectedSection* xSection;
      m_xSubAdjacencyList.Get(i, xSection);

      bool bUpDownFlag = ((DirectedSection*)xSection)->GetUpDownFlag();
      TupleId iSectionTid = ((DirectedSection*)xSection)->GetSectionTid();
      inout_xSections.push_back(DirectedSection(iSectionTid,
      bUpDownFlag));

    }
  }
}

/*
Returns the route Interval between the two points

*/

void chkStartEndA(double &StartPos, double &EndPos){
  double help;
  if (StartPos > EndPos) {
    help = StartPos;
    StartPos = EndPos;
    EndPos = help;
  }
};

bool Network::ShorterConnection(Tuple *route, double &start,
                   double &end, double &dpos, double &dpos2, int &rid,
                   int &ridt, Point p1, Point p2 ){
  if (AlmostEqual(p1.Distance(p2), fabs(end-start))) return false;
  double difference;
  GPoint *gp = new GPoint(true, GetId(), route->GetTupleId(), end - 0.01 );
  Tuple *pSection1 = GetSectionOnRoute(gp);
  delete gp;
  vector<DirectedSection> pAdjSect1;
  vector<DirectedSection> pAdjSect2;
  pAdjSect1.clear();
  pAdjSect2.clear();
  GetAdjacentSections(pSection1->GetTupleId(),true, pAdjSect1);
  GetAdjacentSections(pSection1->GetTupleId(),false, pAdjSect2);
  pSection1->DeleteIfAllowed();
  if (pAdjSect1.size() == 0 || pAdjSect2.size() == 0) {
    pAdjSect1.clear();
    pAdjSect2.clear();
    return false;
  } else {
    size_t j = 0;
    while (j < pAdjSect1.size()) {
      DirectedSection actSection = pAdjSect1[j];
      Tuple *pCurrSect = m_pSections->GetTuple(actSection.GetSectionTid());
      ridt = ((CcInt*)pCurrSect->GetAttribute(SECTION_RID))->GetIntval();
      pCurrSect->DeleteIfAllowed();
      if (ridt != rid) {
        Tuple *pRoute = GetRoute(ridt);
        SimpleLine *pCurve =
          (SimpleLine*) pRoute->GetAttribute(ROUTE_CURVE);
        if ((chkPoint(pCurve, p1, true, dpos, difference)) &&
            (chkPoint(pCurve, p2, true, dpos2, difference))){
          pAdjSect1.clear();
          pAdjSect2.clear();
          chkStartEndA(dpos, dpos2);
          pRoute->DeleteIfAllowed();
          if (fabs(dpos2 - dpos) < fabs(end - start)) return  true;
          else return false;
        }
        pRoute->DeleteIfAllowed();
      }
      j++;
    }
    pAdjSect1.clear();
    j = 0;
    while (j < pAdjSect2.size()) {
      DirectedSection actSection = pAdjSect2[j];
      Tuple *pCurrSect = m_pSections->GetTuple(actSection.GetSectionTid());
      ridt = ((CcInt*)pCurrSect->GetAttribute(SECTION_RID))->GetIntval();
      pCurrSect->DeleteIfAllowed();
      if (ridt != rid) {
        Tuple *pRoute = GetRoute(ridt);
        SimpleLine *pCurve =
          (SimpleLine*) pRoute->GetAttribute(ROUTE_CURVE);
        if ((chkPoint(pCurve, p1, true, dpos, difference)) &&
            (chkPoint(pCurve, p2, true, dpos2, difference))){
          pAdjSect2.clear();
          chkStartEndA(dpos, dpos2);
          pRoute->DeleteIfAllowed();
          if (fabs(dpos2-dpos) < fabs(end - start)) return true;
          else return false;
        }
        pRoute->DeleteIfAllowed();
      }
      j++;
    }
    return false;
  }
}

bool Network::ShorterConnection2(Tuple *route, double &start,
                   double &end, double &dpos, double &dpos2, int &rid,
                   int &ridt, Point p1, Point p2 ){
  if (AlmostEqual(p1.Distance(p2), fabs(end-start))) return false;
  double difference = 0.0;
  if (start < end && end > 0.01) difference = end - 0.01;
  else
    if (start < end && end <= 0.01) difference = 0.01;
    else
      if (start > end) difference = end + 0.01;
      else difference = end; //start == end
  GPoint *gp = new GPoint(true, GetId(), route->GetTupleId(), difference);
  Tuple *pSection1 = GetSectionOnRoute(gp);
  delete gp;
  vector<DirectedSection> pAdjSect1;
  vector<DirectedSection> pAdjSect2;
  pAdjSect1.clear();
  pAdjSect2.clear();
  GetAdjacentSections(pSection1->GetTupleId(),true, pAdjSect1);
  GetAdjacentSections(pSection1->GetTupleId(),false, pAdjSect2);
  pSection1->DeleteIfAllowed();
  if (pAdjSect1.size() == 0 || pAdjSect2.size() == 0) {
    pAdjSect1.clear();
    pAdjSect2.clear();
    return false;
  } else {
    size_t j = 0;
    while (j < pAdjSect1.size()) {
      DirectedSection actSection = pAdjSect1[j];
      Tuple *pCurrSect = m_pSections->GetTuple(actSection.GetSectionTid());
      ridt = ((CcInt*)pCurrSect->GetAttribute(SECTION_RID))->GetIntval();
      pCurrSect->DeleteIfAllowed();
      if (ridt != rid) {
        Tuple *pRoute = GetRoute(ridt);
        SimpleLine *pCurve =
          (SimpleLine*) pRoute->GetAttribute(ROUTE_CURVE);
        if ((chkPoint(pCurve, p1, true, dpos, difference)) &&
            (chkPoint(pCurve, p2, true, dpos2, difference))){
          pAdjSect1.clear();
          pAdjSect2.clear();
          pRoute->DeleteIfAllowed();
          if (fabs(dpos2 - dpos) < fabs(end - start)) return  true;
          else return false;
        }
        pRoute->DeleteIfAllowed();
      }
      j++;
    }
    pAdjSect1.clear();
    j = 0;
    while (j < pAdjSect2.size()) {
      DirectedSection actSection = pAdjSect2[j];
      Tuple *pCurrSect = m_pSections->GetTuple(actSection.GetSectionTid());
      ridt = ((CcInt*)pCurrSect->GetAttribute(SECTION_RID))->GetIntval();
      pCurrSect->DeleteIfAllowed();
      if (ridt != rid) {
        Tuple *pRoute = GetRoute(ridt);
        SimpleLine *pCurve =
          (SimpleLine*) pRoute->GetAttribute(ROUTE_CURVE);
        if ((chkPoint(pCurve, p1, true, dpos, difference)) &&
            (chkPoint(pCurve, p2, true, dpos2, difference))){
          pAdjSect2.clear();
          pRoute->DeleteIfAllowed();
          if (fabs(dpos2-dpos) < fabs(end - start)) return true;
          else return false;
        }
        pRoute->DeleteIfAllowed();
      }
      j++;
    }
    return false;
  }
}


RouteInterval* Network::Find(Point p1, Point p2){
  GPoint *gpp1 = GetNetworkPosOfPoint(p1);
  GPoint *gpp2 = GetNetworkPosOfPoint(p2);
  assert (gpp1->IsDefined() && gpp2->IsDefined());
  int rid, ridt;
  double start, end, dpos, dpos2, difference;
  if (gpp1->GetRouteId() == gpp2->GetRouteId()){
    rid = gpp1->GetRouteId();
    start = gpp1->GetPosition();
    end = gpp2->GetPosition();
    chkStartEndA(start,end);
    Tuple *pRoute = GetRoute(rid);
    if (ShorterConnection(pRoute, start, end, dpos, dpos2, rid, ridt, p1, p2)) {
      delete gpp1;
      delete gpp2;
      pRoute->DeleteIfAllowed();
      return new RouteInterval(ridt, dpos, dpos2);
    } else {
      delete gpp1;
      delete gpp2;
      pRoute->DeleteIfAllowed();
      return new RouteInterval(rid, start, end);
    }
  } else { // different RouteIds
    Tuple *pRoute = GetRoute(gpp1->GetRouteId());
    SimpleLine *pCurve =
        (SimpleLine*) pRoute->GetAttribute(ROUTE_CURVE);
    if(chkPoint(pCurve, p2, true, dpos, difference)){
      rid =
        ((CcInt*)pRoute->GetAttribute(ROUTE_ID))->GetIntval();
      start = gpp1->GetPosition();
      end = dpos;
      chkStartEndA(start, end);
      if(ShorterConnection(pRoute, start, end, dpos, dpos2, rid, ridt, p1, p2)){
        delete gpp1;
        delete gpp2;
        pRoute->DeleteIfAllowed();
        return new RouteInterval(ridt, dpos, dpos2);
      } else {
        delete gpp1;
        delete gpp2;
        pRoute->DeleteIfAllowed();
        return new RouteInterval(rid, start, end);
      }
    }
    pRoute->DeleteIfAllowed();
    pRoute = GetRoute(gpp2->GetRouteId());
    pCurve = (SimpleLine*) pRoute->GetAttribute(ROUTE_CURVE);
    if(chkPoint(pCurve, p1, true, dpos, difference)){
      rid =
        ((CcInt*)pRoute->GetAttribute(ROUTE_ID))->GetIntval();
      start = gpp2->GetPosition();
      end = dpos;
      chkStartEndA(start, end);
      if(ShorterConnection(pRoute, start, end, dpos, dpos2, rid, ridt, p1, p2)){
        delete gpp1;
        delete gpp2;
        pRoute->DeleteIfAllowed();
        return new RouteInterval(ridt, dpos, dpos2);
      } else {
        delete gpp1;
        delete gpp2;
        pRoute->DeleteIfAllowed();
        return new RouteInterval(rid, start, end);
      }
    }
    pRoute->DeleteIfAllowed();
    Tuple *pSection = GetSectionOnRoute(gpp1);
    vector<DirectedSection> pAdjSect1;
    vector<DirectedSection> pAdjSect2;
    pAdjSect1.clear();
    pAdjSect2.clear();
    GetAdjacentSections(pSection->GetTupleId(),true, pAdjSect1);
    GetAdjacentSections(pSection->GetTupleId(),false, pAdjSect2);
    pSection->DeleteIfAllowed();
    size_t j = 0;
    Tuple *pCurrSect;
    while (j < pAdjSect1.size()) {
      DirectedSection actSection = pAdjSect1[j];
      pCurrSect = m_pSections->GetTuple(actSection.GetSectionTid());
      rid = ((CcInt*)pCurrSect->GetAttribute(SECTION_RID))->GetIntval();
      pCurrSect->DeleteIfAllowed();
      pRoute = GetRoute(rid);
      pCurve = (SimpleLine*) pRoute->GetAttribute(ROUTE_CURVE);
      if ((chkPoint(pCurve, p1, true, dpos, difference)) &&
          (chkPoint(pCurve, p2, true, dpos2, difference))){
        start = dpos;
        end = dpos2;
        chkStartEndA(start, end);
        pAdjSect1.clear();
        pAdjSect2.clear();
        if(ShorterConnection(pRoute, start, end, dpos, dpos2,
                             rid, ridt, p1, p2)) {
          delete gpp1;
          delete gpp2;
          pRoute->DeleteIfAllowed();
          return new RouteInterval(ridt, dpos, dpos2);
        } else {
          delete gpp1;
          delete gpp2;
          pRoute->DeleteIfAllowed();
          return new RouteInterval(rid, start, end);
        }
      }
      j++;
      pRoute->DeleteIfAllowed();
    }
    j = 0;
    pAdjSect1.clear();
    while (j < pAdjSect2.size()) {
      DirectedSection actSection = pAdjSect2[j];
      pCurrSect = m_pSections->GetTuple(actSection.GetSectionTid());
      rid = ((CcInt*)pCurrSect->GetAttribute(SECTION_RID))->GetIntval();
      pCurrSect->DeleteIfAllowed();
      pRoute = GetRoute(rid);
      pCurve = (SimpleLine*) pRoute->GetAttribute(ROUTE_CURVE);
      if ((chkPoint(pCurve, p1, true, dpos, difference)) &&
          (chkPoint(pCurve, p2, true, dpos2, difference))){
        start = dpos;
        end = dpos2;
        chkStartEndA(start, end);
        pAdjSect2.clear();
        if(ShorterConnection(pRoute, start, end, dpos, dpos2,
                             rid, ridt, p1, p2)) {
          delete gpp1;
          delete gpp2;
          pRoute->DeleteIfAllowed();
          return new RouteInterval(ridt, dpos, dpos2);
        } else {
          delete gpp1;
          delete gpp2;
          pRoute->DeleteIfAllowed();
          return new RouteInterval(rid, start, end);
        }
      }
      j++;
      pRoute->DeleteIfAllowed();
    }//should never be reached
    pAdjSect2.clear();
  }//should never be reached
  delete gpp1;
  delete gpp2;
  return 0;
}


/*
Returns the route interval for the connection from p1 to p2

*/

RouteInterval* Network::FindInterval(Point p1, Point p2){
  GPoint *gpp1 = GetNetworkPosOfPoint(p1);
  GPoint *gpp2 = GetNetworkPosOfPoint(p2);
  assert (gpp1->IsDefined() && gpp2->IsDefined());
  int rid, ridt;
  double start, end, dpos, dpos2, difference;
  if (gpp1->GetRouteId() == gpp2->GetRouteId()){
    rid = gpp1->GetRouteId();
    start = gpp1->GetPosition();
    end = gpp2->GetPosition();
    Tuple *pRoute = GetRoute(rid);
    if(ShorterConnection2(pRoute, start, end, dpos, dpos2, rid, ridt, p1, p2)){
      delete gpp1;
      delete gpp2;
      pRoute->DeleteIfAllowed();
      return new RouteInterval(ridt, dpos, dpos2);
    } else {
      delete gpp1;
      delete gpp2;
      pRoute->DeleteIfAllowed();
      return new RouteInterval(rid, start, end);
    }
  } else {
    Tuple *pRoute = GetRoute(gpp1->GetRouteId());
    SimpleLine *pCurve =
        (SimpleLine*) pRoute->GetAttribute(ROUTE_CURVE);
    if(chkPoint(pCurve, p2, true, dpos, difference)){
      rid =
        ((CcInt*)pRoute->GetAttribute(ROUTE_ID))->GetIntval();
      start = gpp1->GetPosition();
      end = dpos;
      if (ShorterConnection2(pRoute, start, end, dpos, dpos2,
                             rid, ridt, p1, p2)) {
        delete gpp1;
        delete gpp2;
        pRoute->DeleteIfAllowed();
        return new RouteInterval(ridt, dpos, dpos2);
      } else {
        delete gpp1;
        delete gpp2;
        pRoute->DeleteIfAllowed();
        return new RouteInterval(rid, start, end);
      }
    }
    pRoute->DeleteIfAllowed();
    pRoute = GetRoute(gpp2->GetRouteId());
    pCurve = (SimpleLine*) pRoute->GetAttribute(ROUTE_CURVE);
    if(chkPoint(pCurve, p1, true, dpos, difference)){
      rid =
        ((CcInt*)pRoute->GetAttribute(ROUTE_ID))->GetIntval();
      start = dpos;
      end = gpp2->GetPosition();
      if (ShorterConnection2(pRoute, start, end, dpos, dpos2,
                             rid, ridt, p1, p2)) {
        delete gpp1;
        delete gpp2;
        pRoute->DeleteIfAllowed();
        return new RouteInterval(ridt, dpos, dpos2);
      } else {
        delete gpp1;
        delete gpp2;
        pRoute->DeleteIfAllowed();
        return new RouteInterval(rid, start, end);
      }
    }
    pRoute->DeleteIfAllowed();
    Tuple *pSection = GetSectionOnRoute(gpp1);
    vector<DirectedSection> pAdjSect1;
    vector<DirectedSection> pAdjSect2;
    pAdjSect1.clear();
    pAdjSect2.clear();
    GetAdjacentSections(pSection->GetTupleId(),true, pAdjSect1);
    GetAdjacentSections(pSection->GetTupleId(),false, pAdjSect2);
    pSection->DeleteIfAllowed();
    size_t j = 0;
    Tuple *pCurrSect;
    while (j < pAdjSect1.size()) {
      DirectedSection actSection = pAdjSect1[j];
      pCurrSect = m_pSections->GetTuple(actSection.GetSectionTid());
      rid = ((CcInt*)pCurrSect->GetAttribute(SECTION_RID))->GetIntval();
      pCurrSect->DeleteIfAllowed();
      pRoute = GetRoute(rid);
      pCurve = (SimpleLine*) pRoute->GetAttribute(ROUTE_CURVE);
      if ((chkPoint(pCurve, p1, true, dpos, difference)) &&
          (chkPoint(pCurve, p2, true, dpos2, difference))){
        start = dpos;
        end = dpos2;
        pAdjSect1.clear();
        pAdjSect2.clear();
        if (ShorterConnection2(pRoute, start, end, dpos, dpos2,
                               rid, ridt, p1, p2)) {
          delete gpp1;
          delete gpp2;
          pRoute->DeleteIfAllowed();
          return new RouteInterval(ridt, dpos, dpos2);
        } else {
          delete gpp1;
          delete gpp2;
          pRoute->DeleteIfAllowed();
          return new RouteInterval(rid, start, end);
        }
      }
      j++;
      pRoute->DeleteIfAllowed();
    }
    j = 0;
    pAdjSect1.clear();
    while (j < pAdjSect2.size()) {
      DirectedSection actSection = pAdjSect2[j];
      pCurrSect = m_pSections->GetTuple( actSection.GetSectionTid());
      rid = ((CcInt*)pCurrSect->GetAttribute(SECTION_RID))->GetIntval();
      pCurrSect->DeleteIfAllowed();
      pRoute = GetRoute(rid);
      pCurve = (SimpleLine*) pRoute->GetAttribute(ROUTE_CURVE);
      if ((chkPoint(pCurve, p1, true, dpos, difference)) &&
          (chkPoint(pCurve, p2, true, dpos2, difference))){
        start = dpos;
        end = dpos2;
        pAdjSect2.clear();
        if (ShorterConnection2(pRoute, start, end, dpos, dpos2,
                               rid, ridt, p1, p2)) {
          delete gpp1;
          delete gpp2;
          pRoute->DeleteIfAllowed();
          return new RouteInterval(ridt, dpos, dpos2);
        } else {
          delete gpp1;
          delete gpp2;
          pRoute->DeleteIfAllowed();
          return new RouteInterval(rid, start, end);
        }
      }
      j++;
      pRoute->DeleteIfAllowed();
    }//should never be reached
    pAdjSect2.clear();
  }//should never be reached
  delete gpp1;
  delete gpp2;
  return 0;
}

/*
~Out~-function of type constructor ~network~

*/
ListExpr Network::Out(ListExpr typeInfo)
{

  ///////////////////////
  // Output of all routes
  GenericRelationIterator *pRoutesIter = m_pRoutes->MakeScan();
  Tuple *pCurrentRoute = 0;
  ListExpr xLast = nl->TheEmptyList();
  ListExpr xNext = nl->TheEmptyList();
  ListExpr xRoutes = nl->TheEmptyList();
  bool bFirst = true;

  while((pCurrentRoute = pRoutesIter->GetNextTuple()) != 0)
  {
    // Read values from table
    CcInt* pRouteId = (CcInt*)pCurrentRoute->GetAttribute(ROUTE_ID);
    int iRouteId = pRouteId->GetIntval();
    CcReal* pLength = (CcReal*)pCurrentRoute->GetAttribute(ROUTE_LENGTH);
    double dLength  = pLength->GetRealval();
    SimpleLine *pCurve = (SimpleLine*)pCurrentRoute->GetAttribute(ROUTE_CURVE);
    // The list for the curve contains all segments of the curve.
    ListExpr xCurve = OutSimpleLine(nl->TheEmptyList(), SetWord(pCurve));
    CcBool* pDual = (CcBool*)pCurrentRoute->GetAttribute(ROUTE_DUAL);
    bool bDual= pDual->GetBoolval();
    CcBool* pStartsSmaller;
    pStartsSmaller = (CcBool*)pCurrentRoute->GetAttribute(ROUTE_STARTSSMALLER);
    bool bStartsSmaller = pStartsSmaller->GetBoolval();

    // Build list
    xNext = nl->FiveElemList(nl->IntAtom(iRouteId),
                             nl->RealAtom(dLength),
                             xCurve,
                             nl->BoolAtom(bDual),
                             nl->BoolAtom(bStartsSmaller));

    // Create new list or append element to existing list
    if(bFirst)
    {
      xRoutes = nl->OneElemList(xNext);
      xLast = xRoutes;
      bFirst = false;
    }
    else
    {
      xLast = nl->Append(xLast, xNext);
    }
    pCurrentRoute->DeleteIfAllowed();
  }
  delete pRoutesIter;

  ///////////////////////
  // Output of all junctions
  GenericRelationIterator *pJunctionsIter = m_pJunctions->MakeScan();
  Tuple *pCurrentJunction;
  ListExpr xJunctions = nl->TheEmptyList();
  bFirst = true;

  while((pCurrentJunction = pJunctionsIter->GetNextTuple()) != 0)
  {
    // Read values from table
    CcInt* pRoute1Id;
    pRoute1Id = (CcInt*)pCurrentJunction->GetAttribute(JUNCTION_ROUTE1_ID);
    int iRoute1Id = pRoute1Id->GetIntval();
    CcReal* pMeas1;
    pMeas1 = (CcReal*)pCurrentJunction->GetAttribute(JUNCTION_ROUTE1_MEAS);
    double dMeas1 = pMeas1->GetRealval();
    CcInt* pRoute2Id;
    pRoute2Id = (CcInt*)pCurrentJunction->GetAttribute(JUNCTION_ROUTE2_ID);
    int iRoute2Id = pRoute2Id->GetIntval();
    CcReal* pMeas2;
    pMeas2 = (CcReal*)pCurrentJunction->GetAttribute(JUNCTION_ROUTE2_MEAS);
    double dMeas2 = pMeas2->GetRealval();
    CcInt* pConnectivityCode;
    pConnectivityCode = (CcInt*)pCurrentJunction->GetAttribute(JUNCTION_CC);
    int iConnectivityCode= pConnectivityCode->GetIntval();
    Point* pPoint = (Point*)pCurrentJunction->GetAttribute(JUNCTION_POS);
    ListExpr xPoint = OutPoint(nl->TheEmptyList(), SetWord(pPoint));

    // Build list
    xNext = nl->SixElemList(nl->IntAtom(iRoute1Id),
                            nl->RealAtom(dMeas1),
                            nl->IntAtom(iRoute2Id),
                            nl->RealAtom(dMeas2),
                            nl->IntAtom(iConnectivityCode),
                            xPoint);

    // Create new list or append element to existing list
    if(bFirst)
    {
      xJunctions= nl->OneElemList(xNext);
      xLast = xJunctions;
      bFirst = false;
    }
    else
    {
      xLast = nl->Append(xLast, xNext);
    }
    pCurrentJunction->DeleteIfAllowed();
  }

  delete pJunctionsIter;

  return nl->ThreeElemList(nl->IntAtom(m_iId),
                           xRoutes,
                           xJunctions);
}



/*
~Save~-function of type constructor ~network~

*/
ListExpr Network::Save(SmiRecord& in_xValueRecord,
                       size_t& inout_iOffset,
                       const ListExpr in_xTypeInfo)
{
  // Save id of the network
  int iId = m_iId;
  in_xValueRecord.Write(&iId,
                        sizeof(int),
                        inout_iOffset);
  inout_iOffset += sizeof(int);

  // Save routes
  ListExpr xType;
  nl->ReadFromString(routesTypeInfo, xType);
  ListExpr xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!m_pRoutes->Save(in_xValueRecord,
                      inout_iOffset,
                      xNumericType))
  {
    return false;
  }
  // Save junctions
  nl->ReadFromString(junctionsInternalTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!m_pJunctions->Save(in_xValueRecord,
                         inout_iOffset,
                         xNumericType))
  {
    return false;
  }
  // Save sections
  nl->ReadFromString(sectionsInternalTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!m_pSections->Save(in_xValueRecord,
                        inout_iOffset,
                        xNumericType))
  {
    return false;
  }
  // Save btree for routes
  nl->ReadFromString(routesBTreeTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!m_pBTreeRoutes->Save(in_xValueRecord,
                           inout_iOffset,
                           xNumericType))
  {
    return false;
  }
  // Save rtree for routes

  if (!m_pRTreeRoutes->Save(in_xValueRecord,
                            inout_iOffset))
  {
    return false;
  }
  // Save first btree for junctions
  nl->ReadFromString(junctionsBTreeTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!m_pBTreeJunctionsByRoute1->Save(in_xValueRecord,
                                      inout_iOffset,
                                      xNumericType))
  {
    return false;
  }
  // Save second btree for junctions
  nl->ReadFromString(junctionsBTreeTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!m_pBTreeJunctionsByRoute2->Save(in_xValueRecord,
                                      inout_iOffset,
                                      xNumericType))
  {
    return false;
  }
  SmiFileId fileId = 0;
  m_xAdjacencyList.SaveToRecord(in_xValueRecord, inout_iOffset, fileId);
  m_xSubAdjacencyList.SaveToRecord(in_xValueRecord, inout_iOffset, fileId);
  // Save btree for sections
  nl->ReadFromString(sectionsBTreeTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!m_pBTreeSectionsByRoute->Save(in_xValueRecord,
                                      inout_iOffset,
                                      xNumericType))
  {
    return false;
  }

  //save distance storage
  /*
  nl->ReadFromString(distancestorageTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!alldistance->Save(in_xValueRecord,
                                      inout_iOffset,
                                      xNumericType))
  {
    return false;
  }
  */

  return true;
}


/*
~Open~-function of type constructor ~network~

*/
Network *Network::Open(SmiRecord& in_xValueRecord,
                       size_t& inout_iOffset,
                       const ListExpr in_xTypeInfo)
{
  // Create network
  return new Network(in_xValueRecord,
                     inout_iOffset,
                     in_xTypeInfo);
}


ListExpr Network::NetworkProp()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,
                 "thenetwork(<id>,<routes-relation>, <junctions-relation>)");

  return (nl->TwoElemList(
            nl->TwoElemList(nl->StringAtom("Creation"),
                            nl->StringAtom("Example Creation")),
            nl->TwoElemList(examplelist,
                            nl->StringAtom("(let n = thenetwork(id, r, j))"))));
}

ListExpr Network::OutNetwork(ListExpr typeInfo, Word value)
{
  Network *n = (Network*)value.addr;
  return n->Out(typeInfo);
}

Word Network::InNetwork(ListExpr in_xTypeInfo,
                        ListExpr in_xValue,
                        int in_iErrorPos,
                        ListExpr& inout_xErrorInfo,
                        bool& inout_bCorrect)
{
  Network* pNetwork = new Network(in_xValue,
                                  in_iErrorPos,
                                  inout_xErrorInfo,
                                  inout_bCorrect);

  if(inout_bCorrect)
  {
    return SetWord(pNetwork);
  }
  else
  {
    delete pNetwork;
    return SetWord(Address(0));
  }
}

Word Network::CreateNetwork(const ListExpr typeInfo)
{
  return SetWord(new Network());
}

void Network::CloseNetwork(const ListExpr typeInfo, Word& w)
{
   delete static_cast<Network*> (w.addr);
   w.addr = 0;

}

/*
~Clone~-function of type constructor ~network~

Not implemented yet.

*/
Word Network::CloneNetwork(const ListExpr typeInfo, const Word& w)
{
  return SetWord(Address(0));
}

void Network::DeleteNetwork(const ListExpr typeInfo, Word& w)
{
  Network* n = (Network*)w.addr;
  //n->Destroy();
  delete n;
}

bool Network::CheckNetwork(ListExpr type, ListExpr& errorInfo)
{
  return (nl->IsEqual(type, "network"));
}

void* Network::CastNetwork(void* addr)
{
  return (0);
}

bool Network::SaveNetwork(SmiRecord& valueRecord,
                          size_t& offset,
                          const ListExpr typeInfo,
                          Word& value)
{
  Network *n = (Network*)value.addr;
  return n->Save(valueRecord, offset, typeInfo);
}

bool Network::OpenNetwork(SmiRecord& valueRecord,
                           size_t& offset,
                           const ListExpr typeInfo,
                           Word& value)
{
  value.addr = Network::Open(valueRecord, offset, typeInfo);
  return value.addr != 0;
}

int Network::SizeOfNetwork()
{
  return 0;
}

int Network::isDefined() {
  return m_bDefined;
}

GPoint* Network::GetNetworkPosOfPoint(Point p){
  const Rectangle<2> orig = p.BoundingBox();
  const Rectangle<2> bbox = Rectangle<2>(true,
                                         orig.MinD(0) - 1.0,
                                         orig.MaxD(0) + 1.0,
                                         orig.MinD(1) - 1.0,
                                         orig.MaxD(1) + 1.0);
  R_TreeLeafEntry<2,TupleId> res;
  Tuple *pCurrRoute = 0;
  if (m_pRTreeRoutes->First(bbox, res)){
    pCurrRoute = m_pRoutes->GetTuple(res.info);
    // pCurrRoute->PutAttribute(0, new TupleIdentifier(true, res.info));
  } else {
    GPoint *result = new GPoint(false);
    pCurrRoute->DeleteIfAllowed();
    return result;
  }
  double dpos, difference;
  SimpleLine* pRouteCurve = (SimpleLine*) pCurrRoute->GetAttribute(ROUTE_CURVE);
  if (chkPoint(pRouteCurve, p, true, dpos, difference)){
    int rid = ((CcInt*) pCurrRoute->GetAttribute(ROUTE_ID))->GetIntval();
    GPoint *result = new GPoint(true, GetId(), rid, dpos, None);
    pCurrRoute->DeleteIfAllowed();
    return result;
  } else {
    pCurrRoute->DeleteIfAllowed();
    while (m_pRTreeRoutes->Next(res)){
      pCurrRoute = m_pRoutes->GetTuple(res.info);
      pRouteCurve = (SimpleLine*) pCurrRoute->GetAttribute(ROUTE_CURVE);
      if (chkPoint(pRouteCurve, p, true, dpos, difference)){
        int rid = ((CcInt*) pCurrRoute->GetAttribute(ROUTE_ID))->GetIntval();
        GPoint *result = new GPoint(true, GetId(),
                                rid,
                                dpos, None);
        pCurrRoute->DeleteIfAllowed();
        return result;
      }
      pCurrRoute->DeleteIfAllowed();
    }
/*
If the point exact hits a route the route should be found here. If the point
value is not exact on the route curve we try to map it in the next step with
bigger tolerance for the hit of the route curve.

*/
    if (m_pRTreeRoutes->First(bbox, res))
      pCurrRoute = m_pRoutes->GetTuple(res.info);
    pRouteCurve = (SimpleLine*) pCurrRoute->GetAttribute(ROUTE_CURVE);
    if (chkPoint03(pRouteCurve, p, true, dpos, difference)){
      int rid = ((CcInt*) pCurrRoute->GetAttribute(ROUTE_ID))->GetIntval();
      GPoint *result = new GPoint(true, GetId(),
                                rid,
                                dpos, None);
      pCurrRoute->DeleteIfAllowed();
      return result;
    } else {
      pCurrRoute->DeleteIfAllowed();
      while (m_pRTreeRoutes->Next(res)){
        pCurrRoute = m_pRoutes->GetTuple(res.info);
        pRouteCurve = (SimpleLine*) pCurrRoute->GetAttribute(ROUTE_CURVE);
        if (chkPoint03(pRouteCurve, p, true, dpos, difference)){
          int rid = ((CcInt*) pCurrRoute->GetAttribute(ROUTE_ID))->GetIntval();
          GPoint *result = new GPoint(true, GetId(),
                                rid,
                                dpos, None);
          pCurrRoute->DeleteIfAllowed();
          return result;
        }
        pCurrRoute->DeleteIfAllowed();
      }
    }

    if (m_pRTreeRoutes->First(bbox, res))
      pCurrRoute = m_pRoutes->GetTuple(res.info);
    pRouteCurve = (SimpleLine*) pCurrRoute->GetAttribute(ROUTE_CURVE);
    if (lastchkPoint03(pRouteCurve, p, true, dpos, difference)){
      int rid = ((CcInt*) pCurrRoute->GetAttribute(ROUTE_ID))->GetIntval();
      GPoint *result = new GPoint(true, GetId(),
                                rid,
                                dpos, None);
      pCurrRoute->DeleteIfAllowed();
      return result;
    } else {
      pCurrRoute->DeleteIfAllowed();
      while (m_pRTreeRoutes->Next(res)){
        pCurrRoute = m_pRoutes->GetTuple(res.info);
        pRouteCurve = (SimpleLine*) pCurrRoute->GetAttribute(ROUTE_CURVE);
        if (lastchkPoint03(pRouteCurve, p, true, dpos, difference)){
          int rid = ((CcInt*) pCurrRoute->GetAttribute(ROUTE_ID))->GetIntval();
          GPoint *result = new GPoint(true, GetId(),
                                rid,
                                dpos, None);
          pCurrRoute->DeleteIfAllowed();
          return result;
        }
        pCurrRoute->DeleteIfAllowed();
      }
    } // should not be reached
    GPoint *result = new GPoint(false);
    pCurrRoute->DeleteIfAllowed();
    return result;
  }
}


/*
~GetJunctionsMeasForRoutes~

Returns the position of a junction on both routes building the junction.

*/

void Network::GetJunctionMeasForRoutes(CcInt *pRoute1Id, CcInt *pRoute2Id,
                              double &rid1meas, double &rid2meas){
  CcInt *pCurrJuncR2id, *pCurrJuncR1id;
  int iCurrJuncTupleR2id, iCurrJuncR1id, iRoute1Id, iRoute2Id;
  CcReal *pRid1Meas, *pRid2Meas;
  bool r1smallerr2, found;
  BTreeIterator *pJunctionsIt;
  if (pRoute1Id->GetIntval() <= pRoute2Id->GetIntval()) {
    pJunctionsIt = m_pBTreeJunctionsByRoute1->ExactMatch(pRoute1Id);
    iRoute1Id = pRoute1Id->GetIntval();
    iRoute2Id = pRoute2Id->GetIntval();
    r1smallerr2 = true;
  } else {
    pJunctionsIt = m_pBTreeJunctionsByRoute1->ExactMatch(pRoute2Id);
    iRoute1Id = pRoute2Id->GetIntval();
    iRoute2Id = pRoute1Id->GetIntval();
    r1smallerr2 = false;
  }
  found = false;
  while (!found && pJunctionsIt->Next()){
    Tuple *pCurrJuncTuple = m_pJunctions->GetTuple(pJunctionsIt->GetId());
    pCurrJuncR2id = (CcInt*) pCurrJuncTuple->GetAttribute(JUNCTION_ROUTE2_ID);
    iCurrJuncTupleR2id = pCurrJuncR2id->GetIntval();
    pCurrJuncR1id = (CcInt*) pCurrJuncTuple->GetAttribute(JUNCTION_ROUTE1_ID);
    iCurrJuncR1id = pCurrJuncR1id->GetIntval();
    if (iCurrJuncTupleR2id == iRoute2Id && iCurrJuncR1id == iRoute1Id) {
      found = true;
      if (r1smallerr2) {
        pRid1Meas = (CcReal*)pCurrJuncTuple->GetAttribute(JUNCTION_ROUTE1_MEAS);
        rid1meas = pRid1Meas->GetRealval();
        pRid2Meas = (CcReal*)pCurrJuncTuple->GetAttribute(JUNCTION_ROUTE2_MEAS);
        rid2meas = pRid2Meas->GetRealval();
      }else{
        pRid1Meas = (CcReal*)pCurrJuncTuple->GetAttribute(JUNCTION_ROUTE2_MEAS);
        rid1meas = pRid1Meas->GetRealval();
        pRid2Meas = (CcReal*)pCurrJuncTuple->GetAttribute(JUNCTION_ROUTE1_MEAS);
        rid2meas = pRid2Meas->GetRealval();
      }
    }
    pCurrJuncTuple->DeleteIfAllowed();
  }
  delete pJunctionsIt;
  if (!found) {
    rid1meas = numeric_limits<double>::max();
    rid2meas = numeric_limits<double>::max();
  }
}

/*
Return sLine Value from RouteId

*/

void Network::GetLineValueOfRouteInterval (const RouteInterval *in_ri,
                                           SimpleLine *out_Line){
  CcInt* pRouteId = new CcInt(true, in_ri->m_iRouteId);
  BTreeIterator *pRoutesIter = m_pBTreeRoutes->ExactMatch(pRouteId);
  delete pRouteId;
  Tuple *pRoute = 0;
  if(pRoutesIter->Next()) pRoute = m_pRoutes->GetTuple(pRoutesIter->GetId());
  assert(pRoute != 0);
  SimpleLine* pLine = (SimpleLine*)pRoute->GetAttribute(ROUTE_CURVE);
  assert(pLine != 0);
  CcBool* pStSm = (CcBool*) pRoute->GetAttribute(ROUTE_STARTSSMALLER);
  bool startSmaller = pStSm->GetBoolval();
  pLine->SubLine(min(in_ri->m_dStart, in_ri->m_dEnd),
                 max(in_ri->m_dStart, in_ri->m_dEnd),
                 startSmaller, *out_Line);
  pRoute->DeleteIfAllowed();
  delete pRoutesIter;
}

/*
1.3.1.4 Secondo TypeConstructor for class ~Network~

*/

TypeConstructor network( "network",          Network::NetworkProp,
                         Network::OutNetwork,           Network::InNetwork,
                         0,                    0,
                         Network::CreateNetwork,        Network::DeleteNetwork,
                         Network::OpenNetwork,          Network::SaveNetwork,
                         Network::CloseNetwork,         Network::CloneNetwork,
                         Network::CastNetwork,          Network::SizeOfNetwork,
                         Network::CheckNetwork );


/*
1.3.2 class ~GLine~

*/

/*
1.3.2.1 Constructors

The simple constructor. Should not be used.

*/
  GLine::GLine()
  {
    del.refs=1;
    del.isDelete=true;
  }

  GLine::GLine(int in_iSize):
    m_xRouteIntervals(in_iSize)
  {
    m_bDefined = false;
    m_bSorted = false;
    m_dLength = 0.0;
    del.refs=1;
    del.isDelete=true;
  }

  GLine::GLine(const GLine* in_xOther):
  m_xRouteIntervals(0)
  {
    m_bDefined = in_xOther->m_bDefined;
    m_bSorted = in_xOther->m_bSorted;
    m_iNetworkId = in_xOther->m_iNetworkId;
    m_dLength = 0.0;
    // Iterate over all RouteIntervalls
    for (int i = 0; i < in_xOther->m_xRouteIntervals.Size(); i++)
    {
      // Get next Interval
      const RouteInterval* pCurrentInterval;
      in_xOther->m_xRouteIntervals.Get(i, pCurrentInterval);

      int iRouteId = pCurrentInterval->m_iRouteId;
      double dStart = pCurrentInterval->m_dStart;
      double dEnd = pCurrentInterval->m_dEnd;
      AddRouteInterval(iRouteId,
                       dStart,
                       dEnd);
    }
    del.refs=1;
    del.isDelete=true;
  }

  GLine::GLine( ListExpr in_xValue,
                int in_iErrorPos,
                ListExpr& inout_xErrorInfo,
                bool& inout_bCorrect)
  {
  // Check the list
  if(!(nl->ListLength( in_xValue ) == 2))
  {
    string strErrorMessage = "GLine(): List length must be 2.";
    inout_xErrorInfo = nl->Append(inout_xErrorInfo,
                                  nl->StringAtom(strErrorMessage));
    inout_bCorrect = false;
    m_bDefined = false;
    m_bSorted = false;
    return;
  }

  // Split into the two parts
  ListExpr xNetworkIdList = nl->First(in_xValue);
  ListExpr xRouteIntervalList = nl->Second(in_xValue);

  // Check the parts
  if(!nl->IsAtom(xNetworkIdList) ||
      nl->AtomType(xNetworkIdList) != IntType)
  {
    string strErrorMessage = "GLine(): Error while reading network-id.";
        inout_xErrorInfo = nl->Append(inout_xErrorInfo,
                                      nl->StringAtom(strErrorMessage));
    m_bDefined = false;
    m_bSorted = false;
    inout_bCorrect = false;
    return;
  }

  m_iNetworkId = nl->IntValue(xNetworkIdList);
  m_dLength = 0.0;
  if (!nl->IsEmpty(xRouteIntervalList)){
    // Iterate over all routes

    while( !nl->IsEmpty( xRouteIntervalList) )
    {
      ListExpr xCurrentRouteInterval = nl->First( xRouteIntervalList );
      xRouteIntervalList = nl->Rest( xRouteIntervalList );

      if( nl->ListLength( xCurrentRouteInterval ) != 3 ||
        (!nl->IsAtom( nl->First(xCurrentRouteInterval))) ||
        nl->AtomType( nl->First(xCurrentRouteInterval)) != IntType ||
        (!nl->IsAtom( nl->Second(xCurrentRouteInterval))) ||
        nl->AtomType( nl->Second(xCurrentRouteInterval)) != RealType ||
        (!nl->IsAtom( nl->Third(xCurrentRouteInterval))) ||
        nl->AtomType( nl->Third(xCurrentRouteInterval)) != RealType)
      {
        string strErrorMessage = "GLine(): Error while reading route-interval.";
            inout_xErrorInfo = nl->Append(inout_xErrorInfo,
                                          nl->StringAtom(strErrorMessage));
        inout_bCorrect = false;
        m_bDefined = false;
        m_bSorted = false;
        return;
      }

      // Read attributes from list
      // Read values from table
      int iRouteId = nl->IntValue( nl->First(xCurrentRouteInterval) );
      double dStart = nl->RealValue( nl->Second(xCurrentRouteInterval) );
      double dEnd  = nl->RealValue( nl->Third(xCurrentRouteInterval) );

      AddRouteInterval(iRouteId,
                      dStart,
                      dEnd);

    }
    inout_bCorrect = true;
    m_bDefined = true;
    m_bSorted = false;
  } else {
    m_bDefined = false;
    m_bSorted = false;
    inout_bCorrect = true;
  }
  del.refs=1;
  del.isDelete=true;
  return;
}

/*
1.3.2.2 Methods of class ~GLine~

*/
void GLine::SetNetworkId(int in_iNetworkId)
{
  m_iNetworkId = in_iNetworkId;
  m_bDefined = true;
}

void GLine::AddRouteInterval(RouteInterval *ri) {
  m_xRouteIntervals.Append(*ri);
  m_dLength = m_dLength + fabs (ri->m_dEnd - ri->m_dStart);
}

void GLine::AddRouteInterval(int in_iRouteId,
                             double in_dStart,
                             double in_dEnd)
{
  RouteInterval *ri = new RouteInterval(in_iRouteId,
                                         in_dStart,
                                         in_dEnd);
  AddRouteInterval(ri);
  delete ri;
}

bool GLine::IsDefined() const
{
  return m_bDefined;
}

void GLine::SetDefined( bool in_bDefined )
{
  m_bDefined = in_bDefined;
}

bool GLine::IsSorted() {
  return m_bSorted;
}

void GLine::SetSorted(bool in_bSorted) {
  m_bSorted = in_bSorted;
}

Word GLine::In(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct)
{
  GLine* pGline = new GLine(0);
  if (nl->ListLength(instance) == 0) {
    correct = true;
    pGline->SetDefined(false);
    return SetWord(pGline);
  }
  if (nl->ListLength(instance) != 2) {
    correct = false;
    delete pGline;
    cmsg.inFunError("Expecting (networkid (list of routeintervals))");
    return SetWord(Address(0));
  }
  ListExpr FirstElem = nl->First(instance);
  ListExpr SecondElem = nl->Second(instance);
  if (!nl->IsAtom(FirstElem) || !nl->AtomType(FirstElem)== IntType) {
    correct = false;
    delete pGline;
    cmsg.inFunError("Networkadress is not evaluable");
    return SetWord(Address(0));
  }
  pGline->SetNetworkId(nl->IntValue(FirstElem));
  if (nl->IsEmpty(SecondElem)) {
    correct = false;
    delete pGline;
    return SetWord(Address(0));
  }
  while (!nl->IsEmpty(SecondElem)) {
    ListExpr start = nl->First(SecondElem);
    SecondElem = nl->Rest(SecondElem);
    if (nl->ListLength(start) != 3) {
      correct = false;
      delete pGline;
      cmsg.inFunError("Routeinterval incorrect.Expected list of 3 Elements.");
      return SetWord(Address(0));
    }
    ListExpr lrid = nl->First(start);
    ListExpr lpos1 = nl->Second(start);
    ListExpr lpos2 = nl->Third(start);
    if (!nl->IsAtom(lrid) || !nl->AtomType(lrid) == IntType ||
        !nl->IsAtom(lpos1) || !nl->AtomType(lpos1) == RealType ||
        !nl->IsAtom(lpos2) || !nl->AtomType(lpos2) == RealType) {
      correct = false;
      delete pGline;
      cmsg.inFunError("Routeinterval should be list int, real, real.");
      return SetWord(Address(0));
    }
    pGline->AddRouteInterval(nl->IntValue(lrid),
                            nl->RealValue(lpos1),
                            nl->RealValue(lpos2));
  }
  correct = true;
  return SetWord(pGline);
}

ListExpr GLine::Out(ListExpr in_xTypeInfo,
                    Word in_xValue)
{
  GLine *pGline = (GLine*)in_xValue.addr;

  if(pGline == 0 || !pGline->IsDefined())
  {
    return nl->SymbolAtom( "undef" );
  }

  ListExpr xLast = nl->TheEmptyList();
  ListExpr xNext = nl->TheEmptyList();
  bool bFirst = true;
  ListExpr xNetworkId = nl->IntAtom(pGline->m_iNetworkId);
  ListExpr xRouteIntervals = nl->TheEmptyList();
  // Iterate over all RouteIntervalls
  for (int i = 0; i < pGline->m_xRouteIntervals.Size(); ++i)
  {
    // Get next Interval
    const RouteInterval* pCurrentInterval;
    pGline->m_xRouteIntervals.Get(i, pCurrentInterval);

    int iRouteId = pCurrentInterval->m_iRouteId;
    double dStart = pCurrentInterval->m_dStart;
    double dEnd = pCurrentInterval->m_dEnd;
    // Build list
    xNext = nl->ThreeElemList(nl->IntAtom(iRouteId),
                              nl->RealAtom(dStart),
                              nl->RealAtom(dEnd));

    // Create new list or append element to existing list
    if(bFirst)
    {
      xRouteIntervals = nl->OneElemList(xNext);
      xLast = xRouteIntervals;
      bFirst = false;
    }
    else
    {
      xLast = nl->Append(xLast, xNext);
    }
  }
  if(pGline->m_xRouteIntervals.Size() == 0)
  {
    xRouteIntervals = nl->TheEmptyList();
  }
  return nl->TwoElemList(xNetworkId,
                         xRouteIntervals);
}

Word GLine::Create(const ListExpr typeInfo)
{
  return SetWord( new GLine(0) );
}

void GLine::Clear() {
  m_xRouteIntervals.Clear();
  SetSorted(false);
  m_dLength = 0.0;
}

void GLine::Delete(const ListExpr typeInfo,
                   Word& w )
{
  GLine *l = (GLine*)w.addr;
  //if (l->del.refs == 1) l->m_xRouteIntervals.Destroy();
  l->DeleteIfAllowed();
  w.addr = 0;
}

void GLine::Close(const ListExpr typeInfo,
                  Word& w )
{
    ((GLine*)w.addr)->DeleteIfAllowed();
    w.addr = 0;
}

Word GLine::CloneGLine(const ListExpr typeInfo,
                  const Word& w )
{
  return SetWord(((GLine*)w.addr)->Clone());
}

GLine* GLine::Clone() const{
  GLine *xOther = new GLine(Size());
  xOther->SetDefined(m_bDefined);
  xOther->SetSorted(m_bSorted);
  xOther->SetNetworkId(m_iNetworkId);
  const RouteInterval *ri;
  for (int i = 0; i < Size(); i++){
    Get(i, ri);
    int rid = ri->m_iRouteId;
    double start = ri->m_dStart;
    double end = ri->m_dEnd;
    xOther->AddRouteInterval(rid, start, end);
  }
  return xOther;
}

void* GLine::Cast(void* addr)
{
  return new (addr) GLine;
}

int GLine::Size() const
{
  return m_xRouteIntervals.Size();
}

int GLine::SizeOf()
{
  return sizeof(GLine);
}

size_t GLine::Sizeof() const
{
  return sizeof(*this);
}

ostream& GLine::Print( ostream& os ) const
{
    os << "GLine: NetworkId: " << m_iNetworkId << endl;
    for (int i = 0; i < m_xRouteIntervals.Size() ; i++) {
      const RouteInterval *ri;
      Get(i, ri);
      os << "RouteInterval: " << i << " rid: " << ri->m_iRouteId;
      os << " from: " << ri->m_dStart << " to: " << ri->m_dEnd << endl;
    }
    os << " end gline";
    return os;
};

bool GLine::Adjacent( const Attribute* arg ) const
{
  return false;
}

/*
Compare

*/
int GLine::Compare( const Attribute* arg ) const
{
 GLine *gl2 = (GLine*) arg;
  if (IsDefined() && !gl2->IsDefined()) return 1;
  else
    if (!IsDefined() && gl2->IsDefined()) return -1;
    else
      if (!IsDefined() && !gl2->IsDefined()) return 0;
      else
        if (m_dLength < gl2->m_dLength) return -1;
        else
          if (m_dLength > gl2->m_dLength) return 1;
          else
            if (m_xRouteIntervals.Size() < gl2->m_xRouteIntervals.Size())
              return -1;
            else
              if (m_xRouteIntervals.Size() > gl2->m_xRouteIntervals.Size())
                return 1;
              else
                if (*this == *gl2) return 0;
                else{
                  const RouteInterval *ri1, *ri2;
                  int i = 0;
                  while(i < m_xRouteIntervals.Size()) {
                    Get(i,ri1);
                    gl2->Get(i,ri2);
                    if (ri1->m_iRouteId < ri2->m_iRouteId) return -1;
                    else
                      if (ri1->m_iRouteId > ri2->m_iRouteId) return 1;
                      else
                        if (ri1->m_dStart < ri2->m_dStart) return -1;
                        else
                          if (ri1->m_dStart > ri2->m_dStart) return 1;
                          else
                            if (ri1->m_dEnd < ri2->m_dEnd) return -1;
                            else
                              if (ri1->m_dEnd > ri2->m_dEnd) return 1;
                    i++;
                  }
                }
 return 0;
}

GLine& GLine::operator=( const GLine& l )
{
  m_xRouteIntervals.Clear();
  if( l.m_xRouteIntervals.Size() > 0 ){
    m_xRouteIntervals.Resize( l.m_xRouteIntervals.Size() );
    const RouteInterval *ri;
    for( int i = 0; i < l.m_xRouteIntervals.Size(); i++ ) {
      l.m_xRouteIntervals.Get( i, ri );
      int rid = ri->m_iRouteId;
      double start = ri->m_dStart;
      double end = ri->m_dEnd;
      AddRouteInterval(rid, start, end);
    }
  }
  m_bSorted = l.m_bSorted;
  m_bDefined = l.m_bDefined;
  m_iNetworkId = l.m_iNetworkId;
  return *this;
}

bool GLine::operator== (const GLine& l) const{
  if (!m_bDefined || !l.m_bDefined) {
    return false;
  } else {
    const RouteInterval *rIt, *rIl;
    if (m_xRouteIntervals.Size() == l.m_xRouteIntervals.Size() &&
        AlmostEqual(m_dLength, l.m_dLength)) {
      if (m_bSorted && l.m_bSorted) {
        for (int i=0; i < m_xRouteIntervals.Size(); i++) {
          Get(i,rIt);
          l.Get(i,rIl);
          if (!(rIt->m_iRouteId == rIl->m_iRouteId &&
                rIt->m_dStart == rIl->m_dStart &&
                rIt->m_dEnd == rIl->m_dEnd)) return false;
        }
        return true;
      } else {
        for (int i=0; i < m_xRouteIntervals.Size(); i++) {
          Get(i,rIt);
          for (int j = 0; j < m_xRouteIntervals.Size(); j++) {
            l.Get(i,rIl);
            if (!(rIt->m_iRouteId == rIl->m_iRouteId &&
                  rIt->m_dStart == rIl->m_dStart &&
                  rIt->m_dEnd == rIl->m_dEnd)) return false;
          }
        }
        return true;
      }
    } else return false;
  }
}

size_t GLine::HashValue() const
{
  size_t xHash = m_iNetworkId;

  // Iterate over all RouteIntervalls
  for (int i = 0; i < m_xRouteIntervals.Size(); ++i)
  {
    // Get next Interval
    const RouteInterval* pCurrentInterval;
    m_xRouteIntervals.Get(i, pCurrentInterval);

    // Add something for each entry
    int iRouteId = pCurrentInterval->m_iRouteId;
    double dStart = pCurrentInterval->m_dStart;
    double dEnd = pCurrentInterval->m_dEnd;
    xHash += iRouteId + (size_t)dStart + (size_t)dEnd;
  }
  return xHash;
}

int GLine::NumOfFLOBs() const
{
  return 1;
}

FLOB* GLine::GetFLOB(const int i)
{
  if (i == 0) return &m_xRouteIntervals;
  return 0;
}

DBArray<RouteInterval>* GLine::GetRouteIntervals(){
  if (IsDefined()) return &m_xRouteIntervals;
  else {
    DBArray<RouteInterval> *n = new DBArray<RouteInterval>(0);
    return n;
  }
};

void GLine::CopyFrom(const StandardAttribute* right)
{
  *this = *((const GLine*) right);
  //Clear();
  /*
  GLine *src = (GLine*) right;
  SetDefined(src->IsDefined());
  SetSorted(src->IsSorted());
  const RouteInterval* ri;
  for (int i = 0; i < src->Size(); i++) {
    src->Get(i,ri);
    int rid = ri->m_iRouteId;
    double start = ri->m_dStart;
    double end = ri->m_dEnd;
    AddRouteInterval(rid, start, end);
  }
  */
}

double GLine::GetLength(){
  return m_dLength;
}

 int GLine::GetNetworkId() {
  return  m_iNetworkId;
 };

/*
~Get~ returns the route interval at position i in the route intervals ~DBArray~.

*/

void GLine::Get(const int i, const RouteInterval* &ri) const{
  m_xRouteIntervals.Get(i, ri);
};

int GLine::NoOfComponents(){
  return m_xRouteIntervals.Size();
};


ListExpr GLine::Property()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("gline"),
                    nl->StringAtom("(<nid> ((<rid> <startpos> <endpos>)...))"),
                             nl->StringAtom("(1 ((1 1.5 2.5)(2 1.5 2.0)))"))));
}

bool GLine::Check( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "gline" ));
}

/*
Netdistance method computes the network distance between two glines. Uses
network distance method of ~GPoint~.

*/

double GLine::Netdistance (GLine* pgl2){
  //GLine* pgl1 = (GLine*) this;
  double minDist = numeric_limits<double>::max();
  double aktDist = numeric_limits<double>::max();
  if (GetNetworkId() != pgl2->GetNetworkId()) {
    cmsg.inFunError("Both glines must belong to the network.");
    return minDist;
  }
  GPoints *bGPgl1 = GetBGP();
  GPoints *bGPgl2 = pgl2->GetBGP();
  const GPoint *gp1, *gp2;
  for (int i = 0; i < bGPgl1->Size(); i++) {
    bGPgl1->Get(i,gp1);
    if (const_cast<GPoint*>(gp1)->Inside(pgl2)) {
      delete bGPgl1;
      delete bGPgl2;
      return 0.0;
    }
    for (int j = 0; j < bGPgl2->Size(); j++) {
      bGPgl2->Get(j, gp2);
      if (const_cast<GPoint*>(gp2)->Inside(this)) {
        delete bGPgl1;
        delete bGPgl2;
        return 0.0;
      }
      aktDist = const_cast<GPoint*>(gp1)->Netdistance(const_cast<GPoint*>(gp2));
      if (aktDist < minDist) minDist = aktDist;
      if (minDist <= 0.0) {
        delete bGPgl1;
        delete bGPgl2;
        return 0.0;
      }
    }
  }
  delete bGPgl1;
  delete bGPgl2;
  return minDist;
}

/*
Distance method computes the Euclidean Distance between two glines. Uses
distance method of ~GPoint~.

*/

double GLine::Distance (GLine* pgl2){
  Line *l1 = new Line(0);
  Line *l2 = new Line(0);
  Gline2line(l1);
  pgl2->Gline2line(l2);
  if (l1->IsDefined() && l2->IsDefined()) {
    double res = l1->Distance(*l2);
    delete l1;
    delete l2;
    return res;
  } else return numeric_limits<double>::max();
}


void GLine::Uniongl(GLine *pgl2, GLine *res){
  const RouteInterval *pRi1, *pRi2;
  if (!IsDefined() || NoOfComponents() == 0) {
    if (pgl2->IsDefined() && pgl2->NoOfComponents() > 0){
      if (pgl2->IsSorted()) {
        for (int j = 0; j < pgl2->NoOfComponents(); j++) {
          pgl2->Get(j,pRi2);
          res->AddRouteInterval(pRi2->m_iRouteId,
                            pRi2->m_dStart,
                            pRi2->m_dEnd);
        }
      } else {
        pgl2->Get(0,pRi2);
        RITree *ritree = new RITree(pRi2->m_iRouteId,
                          pRi2->m_dStart, pRi2->m_dEnd,0,0);
        for (int j = 1; j < pgl2->NoOfComponents(); j++) {
          pgl2->Get(j,pRi2);
          ritree->Insert(pRi2->m_iRouteId, pRi2->m_dStart, pRi2->m_dEnd);
        }
        ritree->TreeToGLine(res);
        ritree->RemoveTree();
      }
      res->SetDefined(true);
      res->SetSorted(true);
      res->SetNetworkId(pgl2->GetNetworkId());
    } else {
      res->SetDefined(false);
      res->SetSorted(false);
    }
  } else {
    if (!pgl2->IsDefined() || pgl2->NoOfComponents() == 0) {
      if (IsDefined() && NoOfComponents() >0) {
        if (IsSorted()) {
          for (int i = 0; i < NoOfComponents(); i++) {
            Get(i,pRi1);
            res->AddRouteInterval(pRi1->m_iRouteId,
                                pRi1->m_dStart,
                                pRi1->m_dEnd);
          }
        } else {
          Get(0,pRi1);
          RITree *ritree = new RITree(pRi1->m_iRouteId,
                            pRi1->m_dStart, pRi1->m_dEnd,0,0);
          for (int i = 1; i < NoOfComponents(); i++) {
            Get(i,pRi1);
            ritree->Insert(pRi1->m_iRouteId, pRi1->m_dStart, pRi1->m_dEnd);
          }
          ritree->TreeToGLine(res);
          ritree->RemoveTree();
        }
        res->SetDefined(true);
        res->SetSorted(true);
        res->SetNetworkId(GetNetworkId());
      } else {
        res->SetDefined(false);
        res->SetSorted(false);
      }
    } else {
      if (GetNetworkId() != pgl2->GetNetworkId()){
        res->SetDefined(false);
        res->SetSorted(false);
      } else {
        res->SetNetworkId(GetNetworkId());
        if (IsSorted() && pgl2->IsSorted()) {
          int i=0;
          int j=0;
          bool newroute = false;
          int iRouteId;
          double start, end;
          while (i < NoOfComponents() && j < pgl2->NoOfComponents()) {
            Get(i, pRi1);
            pgl2->Get(j, pRi2);
            if (pRi1->m_iRouteId < pRi2->m_iRouteId){
              res->AddRouteInterval(pRi1->m_iRouteId,
                                    pRi1->m_dStart,
                                    pRi1->m_dEnd);
              i++;
            } else {
              if (pRi1->m_iRouteId > pRi2->m_iRouteId) {
                res->AddRouteInterval(pRi2->m_iRouteId, pRi2->m_dStart,
                                      pRi2->m_dEnd);
                j++;
              } else {
                if (pRi1->m_dEnd < pRi2->m_dStart) {
                  res->AddRouteInterval(pRi1->m_iRouteId, pRi1->m_dStart,
                                        pRi1->m_dEnd);
                  i++;
                } else {
                  if (pRi2->m_dEnd < pRi1->m_dStart) {
                    res->AddRouteInterval(pRi2->m_iRouteId, pRi2->m_dStart,
                                          pRi2->m_dEnd);
                    j++;
                  } else {
                    iRouteId = pRi1->m_iRouteId;
                    start = min(pRi1->m_dStart, pRi2->m_dStart),
                    end = max(pRi1->m_dEnd, pRi2->m_dEnd);
                    i++;
                    j++;
                    newroute = false;
                    while (i < NoOfComponents() && !newroute){
                      Get(i,pRi1);
                      if (pRi1->m_iRouteId == iRouteId) {
                        if (pRi1->m_dStart <= end) {
                          end = max (pRi1->m_dEnd, end);
                          i++;
                        } else newroute = true;
                      } else newroute = true;
                    }
                    newroute = false;
                    while (j < pgl2->NoOfComponents() && !newroute){
                      pgl2->Get(j,pRi2);
                      if (pRi2->m_iRouteId == iRouteId) {
                        if (pRi2->m_dStart <= end) {
                          end = max (pRi2->m_dEnd, end);
                          j++;
                        } else newroute = true;
                      } else newroute = true;
                    }
                    res->AddRouteInterval(iRouteId, start, end);
                  }
                }
              }
            }
          }
          while (i < NoOfComponents()) {
            Get(i,pRi1);
            res->AddRouteInterval(pRi1->m_iRouteId,
                                  pRi1->m_dStart,
                                  pRi1->m_dEnd);
            i++;
          }
          while (j < pgl2->NoOfComponents()) {
            pgl2->Get(j,pRi2);
            res->AddRouteInterval(pRi2->m_iRouteId, pRi2->m_dStart,
                                  pRi2->m_dEnd);
            j++;
          }
          res->SetDefined(true);
          res->SetSorted(true);
        } else {
          RITree *ritree;
          Get(0,pRi1);
          ritree = new RITree(pRi1->m_iRouteId,
                              pRi1->m_dStart, pRi1->m_dEnd,0,0);
          for (int i = 1; i < NoOfComponents(); i++) {
            Get(i,pRi1);
            ritree->Insert(pRi1->m_iRouteId, pRi1->m_dStart, pRi1->m_dEnd);
          }
          for (int j = 0; j < pgl2->NoOfComponents(); j++) {
            pgl2->Get(j,pRi2);
            ritree->Insert(pRi2->m_iRouteId, pRi2->m_dStart, pRi2->m_dEnd);
          }
          ritree->TreeToGLine(res);
          ritree->RemoveTree();
          res->SetDefined(true);
          res->SetSorted(true);
        }
      }
    }
  }
}

void GLine::Gline2line(Line* res){
  if (IsDefined() && NoOfComponents() >0) {
    Network* pNetwork = NetworkManager::GetNetwork(GetNetworkId());
    const RouteInterval *rI;
    Line *l = new Line(0);
    Line *x = new Line(0);
    for (int i=0; i < this->NoOfComponents(); i++) {
      delete l;
      l = x;
      this->Get(i,rI);
      SimpleLine *pSubline = new SimpleLine(0);
      pNetwork->GetLineValueOfRouteInterval(rI, pSubline);
      Line *partLine = new Line(0);
      pSubline->toLine(*partLine);
      delete pSubline;
      x = SetOp(*l, *partLine, avlseg::union_op);
      delete partLine;
    }
    delete l;
    NetworkManager::CloseNetwork(pNetwork);
    (*res) = *x;
    delete x;
    res->SetDefined(true);
  } else {
    if (IsDefined() && NoOfComponents() == 0) {
      res->SetDefined(true);
    } else {
      res->SetDefined(false);
    }
  }
}

bool GLine::Intersects(GLine *pgl){
  const RouteInterval *pRi1, *pRi2;
  if (!IsSorted()) {
    for (int i = 0; i < NoOfComponents(); i++){
      Get(i, pRi1);
      if (pgl->IsSorted()){
        if (searchUnit(pgl, 0, pgl->NoOfComponents()-1, pRi1)){
          return true;
        };
      } else {
        for (int j = 0 ; j < pgl->NoOfComponents(); j ++){
          pgl->Get(j,pRi2);
          if (pRi1->m_iRouteId == pRi2->m_iRouteId &&
              (!(pRi1->m_dEnd < pRi2->m_dStart ||
              pRi2->m_dStart > pRi1->m_dEnd))){
            return true;
          }
        }
      }
    }
  } else {
    if (pgl->IsSorted()) {
      int i = 0;
      int j = 0;
      while (i<NoOfComponents() && j < pgl->NoOfComponents()) {
        Get(i,pRi1);
        pgl->Get(j,pRi2);
        if (pRi1->m_iRouteId < pRi2->m_iRouteId) i++;
        else
          if (pRi1->m_iRouteId > pRi2->m_iRouteId) j++;
          else
            if (pRi1->m_dStart > pRi2->m_dEnd) j++;
            else
              if (pRi1->m_dEnd < pRi2->m_dStart) i++;
              else return true;
      }
    } else {
        for (int i = 0; i < pgl->NoOfComponents(); i++){
          pgl->Get(i, pRi2);
          if (searchUnit(this, 0, NoOfComponents()-1, pRi2)) return true;
        }
    }
  }
  return false;
}


/*
1.3.2.3 Secondo Type Constructor for class ~GLine~

*/
TypeConstructor gline(
        "gline",                       //name
        GLine::Property,               //property function
        GLine::Out, GLine::In,         //Out and In functions
        0, 0,                          //SaveToList and
                                       //RestoreFromList functions
        GLine::Create, GLine::Delete,  //object creation and deletion
        OpenAttribute<GLine>, SaveAttribute<GLine>,  //open and save functions
        GLine::Close, GLine::CloneGLine,    //object close, and clone
        GLine::Cast,                   //cast function
        GLine::SizeOf,                 //sizeof function
        GLine::Check);                 //kind checking function


/*
1.3.3 class ~GPoint~

1.3.3.1 Constructors

See ~network.h~ class definition of ~GPoint~

1.3.3.2 Methods of class ~GPoint~

*/
Word GPoint::InGPoint( const ListExpr typeInfo,
                       const ListExpr instance,
                       const int errorPos,
                       ListExpr& errorInfo,
                       bool& correct )
{
  if( nl->ListLength( instance ) == 4 )
  {
    if( nl->IsAtom( nl->First(instance) ) &&
        nl->AtomType( nl->First(instance) ) == IntType &&
        nl->IsAtom( nl->Second(instance) ) &&
        nl->AtomType( nl->Second(instance) ) == IntType &&
        nl->IsAtom( nl->Third(instance) ) &&
        nl->AtomType( nl->Third(instance) ) == RealType &&
        nl->IsAtom( nl->Fourth(instance) ) &&
        nl->AtomType( nl->Fourth(instance) ) == IntType )
    {
      GPoint *gp = new GPoint(
        true,
        nl->IntValue( nl->First(instance) ),
        nl->IntValue( nl->Second(instance) ),
        nl->RealValue( nl->Third(instance) ),
        (Side)nl->IntValue( nl->Fourth(instance) ) );
      correct = true;
      return SetWord( gp );
    }
  }

  correct = false;
  return SetWord( Address(0) );
}

ListExpr GPoint::OutGPoint( ListExpr typeInfo, Word value )
{
  GPoint *gp = (GPoint*)value.addr;

  if( gp->IsDefined() )
  {
    return nl->FourElemList(
      nl->IntAtom(gp->GetNetworkId()),
      nl->IntAtom(gp->GetRouteId()),
      nl->RealAtom(gp->GetPosition()),
      nl->IntAtom(gp->GetSide()));
  }
  return nl->SymbolAtom("undef");
}

Word GPoint::CreateGPoint( const ListExpr typeInfo )
{
  return SetWord( new GPoint( false ) );
}

void GPoint::DeleteGPoint( const ListExpr typeInfo, Word& w )
{
  delete (GPoint*)w.addr;
  w.addr = 0;
}

void GPoint::CloseGPoint( const ListExpr typeInfo, Word& w )
{
  delete (GPoint*)w.addr;
  w.addr = 0;
}

Word GPoint::CloneGPoint( const ListExpr typeInfo, const Word& w )
{
  return SetWord( ((GPoint*)w.addr)->Clone() );
}

void* GPoint::CastGPoint( void* addr )
{
  return new (addr) GPoint;
}

int GPoint::SizeOfGPoint()
{
  return sizeof(GPoint);
}

ListExpr GPoint::GPointProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("gpoint"),
                             nl->StringAtom("(<network_id> <route_id> "
                                            "<position> <side>)"),
                             nl->StringAtom("(1 1 0.0 0)"))));
}

bool GPoint::CheckGPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "gpoint" ));
}

/*
Netdistance function computes the network distance between two ~GPoint~s.
Using Dijkstras-Algorithm for shortest path computing

*/

double GPoint::Netdistance (GPoint* pToGPoint){
  GLine* pGLine = new GLine(0);
  double res;
  if (ShortestPath(pToGPoint, pGLine)) {
    res = pGLine->GetLength();
    delete pGLine;
    return res;
  } else {
    delete pGLine;
    return 0;
  }
}
double GPoint::NewNetdistance(GPoint* pToGPoint,GLine* gline)
{
  GPoint* gp1 = new GPoint(true,GetNetworkId(),GetRouteId(),GetPosition());
  GPoint* gp2 = new GPoint(true,pToGPoint->GetNetworkId(),
                pToGPoint->GetRouteId(),pToGPoint->GetPosition());

  double res;
  if (gp1->ShortestPath(gp2, gline)) {
    res = gline->GetLength();

    delete gp1;
    delete gp2;
    return res;
  } else {

    delete gp1;
    delete gp2;
    return 0;
  }

}

/*
Distance function computes the Euclidean Distance between two ~GPoint~s.

*/

double GPoint::Distance (GPoint* pToGPoint){
  if (IsDefined() && pToGPoint->IsDefined() &&
      GetNetworkId() == pToGPoint->GetNetworkId()) {
    Network* pNetwork=NetworkManager::GetNetwork(this->GetNetworkId());
    Point *from = new Point(false);
    pNetwork->GetPointOnRoute(this, from);
    Point *to = new Point(false);
    pNetwork->GetPointOnRoute(pToGPoint,to);
    double res = from->Distance(*to);
    delete from;
    delete to;
    NetworkManager::CloseNetwork(pNetwork);
    return res;
  } else return numeric_limits<double>::max();
}

bool GPoint::Inside(GLine *gl){
  if (!(gl->IsDefined()) || !IsDefined() ||
        gl->NoOfComponents() < 1) return false;
  if (GetNetworkId() != gl->GetNetworkId()) return false;
  const RouteInterval *pCurrRInter;
  if (gl->IsSorted())
    return (searchRouteInterval(this, gl, 0, gl->NoOfComponents()-1));
  else {
    int i = 0;
    while (i < gl->NoOfComponents()) {
      gl->Get(i, pCurrRInter);
      if (pCurrRInter->m_iRouteId == GetRouteId()){
        if(pCurrRInter->m_dStart < GetPosition() &&
           GetPosition() < pCurrRInter->m_dEnd)
          return true;
        if(pCurrRInter->m_dStart > GetPosition() &&
           GetPosition() > pCurrRInter->m_dEnd)
          return true;
        if(fabs(pCurrRInter->m_dStart - GetPosition()) < 0.1)
          return true;
        if(fabs(pCurrRInter->m_dEnd - GetPosition()) < 0.1)
          return true;
      }
      i++;
    }
    return false;
  }
  return false;
}

bool GPoint::operator== (const GPoint& p) const{
  if (!m_bDefined || !p.IsDefined()) return false;
  else {
    if (m_iNetworkId == p.GetNetworkId() &&
      m_xRouteLocation.rid == p.GetRouteId() &&
      m_xRouteLocation.d == p.GetPosition() &&
      (m_xRouteLocation.side == p.GetSide() || m_xRouteLocation.side == 2 ||
       p.GetSide() == 2)) {
      return true;
    } else return false;
  }
}

bool GPoint::operator!= (const GPoint& p) const{
  return !(*this == p);
}

/*
Computes the shortest path between start and end in the network. The path is
returned as gline value.

*/

bool GPoint::ShortestPath(GPoint *to, GLine *result){

  GPoint* start = new GPoint(true,GetNetworkId(),GetRouteId(),GetPosition());
  GPoint* end = new GPoint(true,to->GetNetworkId(),to->GetRouteId(),
                        to->GetPosition());//copy the gpoint
  result->Clear();
  if (start == 0 || end == 0 || !start->IsDefined() ||
      !end->IsDefined()) {
     sendMessage("Both gpoints must exist and be defined.");
     result->SetDefined(false);
      delete start;
      delete end;
     return false;
  }
  // Check wether both points belong to the same network
  if(start->GetNetworkId() != end->GetNetworkId())
  {
    sendMessage("Both gpoints belong to different networks.");
    result->SetDefined(false);
      delete start;
      delete end;
    return false;
  }
  result->SetNetworkId(start->GetNetworkId());
  // Load the network
  Network* pNetwork = NetworkManager::GetNetwork(start->GetNetworkId());
  if(pNetwork == 0)
  {
    sendMessage("Network not found.");
    result->SetDefined(false);
      delete start;
      delete end;
    return false;
  }
  // Get sections where the path should start or end
  TupleId startSectTID = pNetwork->GetTupleIdSectionOnRoute(start);
  Tuple* startSection = pNetwork->GetSection(startSectTID);
  if (startSection == 0) {
    sendMessage("Starting GPoint not found in network.");
    NetworkManager::CloseNetwork(pNetwork);
    result->SetDefined(false);
     delete start;
     delete end;
    return false;
  }
  TupleId lastSectTID = pNetwork->GetTupleIdSectionOnRoute(end);
  Tuple* endSection = pNetwork->GetSection(lastSectTID);
  if (endSection == 0) {
    sendMessage("End GPoint not found in network.");
    startSection->DeleteIfAllowed();
    NetworkManager::CloseNetwork(pNetwork);
    result->SetDefined(false);
      delete start;
      delete end;
    return false;
  }
////////////////////////////////////////////////////
  /* bool junctionpoint = false;
  Point* endp = new Point();
  pNetwork->GetPointOnRoute(end,endp); //end point
  Point* startp = new Point();
  pNetwork->GetPointOnRoute(start,startp);
  if(fabs(endp->GetX() - startp->GetX()) < 0.1 &&
     fabs(endp->GetY() - startp->GetY()) < 0.1){
    delete endp;
    delete startp;
    delete start;
    delete end;
    startSection->DeleteIfAllowed();
    endSection->DeleteIfAllowed();
    NetworkManager::CloseNetwork(pNetwork);
    return false;
  }

  vector<JunctionSortEntry> juns;
  CcInt* routeid = new CcInt(true,end->GetRouteId());
  pNetwork->GetJunctionsOnRoute(routeid,juns);
  for(unsigned int i = 0;i < juns.size();i++){
    Tuple* t = juns[i].m_pJunction;
    if(((CcInt*)t->GetAttribute(JUNCTION_ROUTE1_ID))->GetIntval() ==
        end->GetRouteId() &&
      fabs(((CcReal*)t->GetAttribute(JUNCTION_ROUTE1_MEAS))->GetRealval()-
      end->GetPosition()) < 0.1)
      junctionpoint = true;
    if(((CcInt*)t->GetAttribute(JUNCTION_ROUTE2_ID))->GetIntval() ==
        end->GetRouteId() &&
      fabs(((CcReal*)t->GetAttribute(JUNCTION_ROUTE2_MEAS))->GetRealval()-
      end->GetPosition()) < 0.1)
      junctionpoint = true;
  }
  vector<TupleId> secjunid;
  if(junctionpoint){//it is a junction point
    vector<DirectedSection> sectionlist;
    if(fabs(end->GetPosition()-
      ((CcReal*)endSection->GetAttribute(SECTION_MEAS1))->GetRealval())< 0.1)
      pNetwork->GetAdjacentSections(endSection->GetTupleId(),false,sectionlist);
    else
      pNetwork->GetAdjacentSections(endSection->GetTupleId(),true,sectionlist);
    for(unsigned int i = 0;i < sectionlist.size();i++){
      if(sectionlist[i].GetSectionTid() != endSection->GetTupleId())
        secjunid.push_back(sectionlist[i].GetSectionTid());
    }
  }
  delete endp;
  delete routeid;*/
/////////////////////////////////////////////////////
/*
Calculate the shortest path using dijkstras algorithm.

*/

  if (startSectTID == lastSectTID  ||
      GetRouteId() == to->GetRouteId()) {
    result->AddRouteInterval(start->GetRouteId(), start->GetPosition(),
                            end->GetPosition());
  } else {
/*
Initialize PriorityQueue

*/
    PrioQueue *prioQ = new PrioQueue (0);
    SectIDTree *visitedSect = 0;
    double sectMeas1 =
        ((CcReal*) startSection->GetAttribute(SECTION_MEAS1))->GetRealval();
    double sectMeas2 =
        ((CcReal*) startSection->GetAttribute(SECTION_MEAS2))->GetRealval();

    double dist = 0.0;
    vector<DirectedSection> adjSectionList;
    adjSectionList.clear();
    if (start->GetSide() == 0) {
      dist = start->GetPosition() - sectMeas1;
      pNetwork->GetAdjacentSections(startSectTID, false, adjSectionList);
      SectIDTree *startTree = new SectIDTree(startSectTID,
                                        (TupleId) numeric_limits<long>::max(),
                                             false,
                                             numeric_limits<int>::max());
      visitedSect = startTree;
      for(size_t i = 0;  i < adjSectionList.size(); i++) {
        DirectedSection actNextSect = adjSectionList[i];
        if (actNextSect.GetSectionTid() != startSectTID) {

          PQEntry *actEntry = new PQEntry(actNextSect.GetSectionTid(), dist,
                               actNextSect.GetUpDownFlag(),
                               startSectTID);
          prioQ->Insert(*actEntry, visitedSect) ;
          delete actEntry;
        }
      }
      adjSectionList.clear();
    } else {
      if (start->GetSide() == 1) {
        dist = sectMeas2 - start->GetPosition();
        SectIDTree *startTree = new SectIDTree(startSectTID,
                                        (TupleId) numeric_limits<long>::max(),
                                             true,
                                             numeric_limits<int>::max());
        visitedSect = startTree;
        pNetwork->GetAdjacentSections(startSectTID, true, adjSectionList);
        for(size_t i = 0;  i < adjSectionList.size(); i++) {
          DirectedSection actNextSect = adjSectionList[i];
          if (actNextSect.GetSectionTid() != startSectTID) {
            PQEntry *actEntry = new PQEntry(actNextSect.GetSectionTid(), dist,
                                  actNextSect.GetUpDownFlag(),
                                  startSectTID);

            prioQ->Insert(*actEntry, visitedSect);
            delete actEntry;
          }
        }
        adjSectionList.clear();
      } else {

        dist = start->GetPosition() - sectMeas1;
        pNetwork->GetAdjacentSections(startSectTID, false, adjSectionList);
        bool first = true;
        for(size_t i = 0;  i < adjSectionList.size(); i++) {
          DirectedSection actNextSect = adjSectionList[i];
          if (actNextSect.GetSectionTid() != startSectTID) {
            if (first) {
              first = false;
              SectIDTree *startTree = new SectIDTree(startSectTID,
                                          (TupleId) numeric_limits<long>::max(),
                                             false,
                                             numeric_limits<int>::max());
              visitedSect = startTree;
            }
            PQEntry *actEntry = new PQEntry(actNextSect.GetSectionTid(), dist,
                                 actNextSect.GetUpDownFlag(),
                                 startSectTID);

            prioQ->Insert(*actEntry, visitedSect);
            delete actEntry;
          }
        }
        adjSectionList.clear();
        dist = sectMeas2 -start->GetPosition();
        pNetwork->GetAdjacentSections(startSectTID, true, adjSectionList);
        for(size_t i = 0;  i < adjSectionList.size(); i++) {
          DirectedSection actNextSect = adjSectionList[i];
          if (actNextSect.GetSectionTid() != startSectTID) {
            if (first) {
              first = false;
              SectIDTree *startTree = new SectIDTree(startSectTID,
                                          (TupleId)numeric_limits<long>::max(),
                                          true,
                                          numeric_limits<int>::max());
              visitedSect = startTree;
            }
            PQEntry *actEntry = new PQEntry(actNextSect.GetSectionTid(), dist,
                                        actNextSect.GetUpDownFlag(),
                                        startSectTID);

            prioQ->Insert(*actEntry, visitedSect);
            delete actEntry;
          }
        }
        adjSectionList.clear();
      }
    }
/*
Use priorityQueue to find shortestPath.

*/

    PQEntry *actPQEntry;
    bool found = false;
    vector<PQEntry> candidate;
    while (!prioQ->IsEmpty() && !found){
      actPQEntry = prioQ->GetAndDeleteMin(visitedSect);
      Tuple *actSection = pNetwork->GetSection(actPQEntry->sectID);
      sectMeas1 =
        ((CcReal*) actSection->GetAttribute(SECTION_MEAS1))->GetRealval();
      sectMeas2 =
        ((CcReal*) actSection->GetAttribute(SECTION_MEAS2))->GetRealval();
      dist = actPQEntry->distFromStart + fabs(sectMeas2 - sectMeas1);


//////////////////////////////////////
      /*if(junctionpoint){ //end point is a junction point
        for(unsigned int i = 0;i < secjunid.size();i++){
          if(secjunid[i] == actPQEntry->sectID){
            lastSectTID = actPQEntry->sectID;
            Tuple* sect = pNetwork->GetSection(lastSectTID);
            double m1 =
                  ((CcReal*)sect->GetAttribute(SECTION_MEAS1))->GetRealval();
            double m2 =
                  ((CcReal*)sect->GetAttribute(SECTION_MEAS2))->GetRealval();

            if(actPQEntry->upDownFlag){
              GPoint* temp = new GPoint(true,end->GetNetworkId(),
                        end->GetRouteId(),m2,None);
              *end = *temp;
              delete temp;
            }else{
              GPoint* temp = new GPoint(true,end->GetNetworkId(),
                        end->GetRouteId(),m1,None);
              *end = *temp;
              delete temp;
            }
            sect->DeleteIfAllowed();
            break;
          }
        }
      }*/
////////////////////////////////////

      if (actPQEntry->sectID != lastSectTID) {
/*
Search further in the network unitl reached last section.
Get adjacent sections and insert into priority Queue.

*/
        adjSectionList.clear();
        pNetwork->GetAdjacentSections(actPQEntry->sectID,
                                      actPQEntry->upDownFlag,
                                      adjSectionList);
        for (size_t i = 0; i <adjSectionList.size();i++){
          DirectedSection actNextSect = adjSectionList[i];
          if (actNextSect.GetSectionTid() != actPQEntry->sectID) {
            PQEntry *actEntry = new PQEntry(actNextSect.GetSectionTid(),
                                        dist,
                                        actNextSect.GetUpDownFlag(),
                                        actPQEntry->sectID);
            prioQ->Insert(*actEntry, visitedSect);
            delete actEntry;
          }
        }
        delete actPQEntry;
        actSection->DeleteIfAllowed();
      } else {
/*
Shortest Path found.

Compute last route interval and resulting gline.

*/
//actually not really found
        /*candidate.push_back(*actPQEntry);

        while(!prioQ->IsEmpty()){
          PQEntry* temp = prioQ->GetAndDeleteMin(visitedSect);
        Tuple *act = pNetwork->GetSection(temp->sectID);
     //double m1 =
      //  ((CcReal*) act->GetAttribute(SECTION_MEAS1))->GetRealval();
      //double m2 =
      //  ((CcReal*) act->GetAttribute(SECTION_MEAS2))->GetRealval();

      act->DeleteIfAllowed();
          if(temp->distFromStart >=
            (actPQEntry->distFromStart+fabs(sectMeas2-sectMeas1))){
            delete temp;
            break;
          }
    if(junctionpoint){ //end point is a junction point
        for(unsigned int i = 0;i < secjunid.size();i++){
          if(secjunid[i] == temp->sectID){

            Tuple* sect = pNetwork->GetSection(temp->sectID);
            double m1 =
                  ((CcReal*)sect->GetAttribute(SECTION_MEAS1))->GetRealval();
            double m2 =
                  ((CcReal*)sect->GetAttribute(SECTION_MEAS2))->GetRealval();
            if(temp->distFromStart+m2-m1 >
              actPQEntry->distFromStart+fabs(sectMeas2-sectMeas1)){
              sect->DeleteIfAllowed();
              continue;
            }
            lastSectTID = temp->sectID;
            if(actPQEntry->upDownFlag){
              GPoint* temp = new GPoint(true,end->GetNetworkId(),
                        end->GetRouteId(),m2,None);
              *end = *temp;
              delete temp;
            }else{
              GPoint* temp = new GPoint(true,end->GetNetworkId(),
                        end->GetRouteId(),m1,None);
              *end = *temp;
              delete temp;
            }
            sect->DeleteIfAllowed();
            break;
          }
        }
      }

          if(temp->sectID == lastSectTID){
            candidate.push_back(*temp);
          }else{
            adjSectionList.clear();
            pNetwork->GetAdjacentSections(temp->sectID,
                                      temp->upDownFlag,
                                      adjSectionList);
            Tuple *tempsec = pNetwork->GetSection(temp->sectID);
            double meas1 =
                ((CcReal*) tempsec->GetAttribute(SECTION_MEAS1))->GetRealval();
            double meas2 =
               ((CcReal*) tempsec->GetAttribute(SECTION_MEAS2))->GetRealval();
            dist = temp->distFromStart + fabs(meas2 - meas1);
            for (size_t i = 0; i <adjSectionList.size();i++){
              DirectedSection actNextSect = adjSectionList[i];
            if (actNextSect.GetSectionTid() != temp->sectID) {
                PQEntry *actEntry = new PQEntry(actNextSect.GetSectionTid(),
                                        dist,
                                        actNextSect.GetUpDownFlag(),
                                        temp->sectID);
              prioQ->Insert(*actEntry, visitedSect);
              delete actEntry;
              }
            }
            tempsec->DeleteIfAllowed();
          }
          delete temp;

        }

        //double length;
        delete actPQEntry;
        actSection->DeleteIfAllowed();
        double tempdist = numeric_limits<double>::max();
        actPQEntry = &candidate[0];
        for(unsigned int i = 0; i < candidate.size();i++){
            Tuple *sec = pNetwork->GetSection(candidate[i].sectID);
            double m1 =
              ((CcReal*)sec->GetAttribute(SECTION_MEAS1))->GetRealval();
            double m2 =
              ((CcReal*)sec->GetAttribute(SECTION_MEAS2))->GetRealval();
            if(candidate[i].upDownFlag == true){//UP
              double dist = candidate[i].distFromStart + m2-m1;
              if(dist < tempdist){
                  actPQEntry = &candidate[i];
                  tempdist = dist;
              }
             }else{//DOWN
              double dist = candidate[i].distFromStart + m2-m1;
              if(dist < tempdist){
                  actPQEntry = &candidate[i];
                  tempdist = dist;
                }
            }
            sec->DeleteIfAllowed();
        }

       actSection = pNetwork->GetSection(actPQEntry->sectID);
        sectMeas1 =
        ((CcReal*) actSection->GetAttribute(SECTION_MEAS1))->GetRealval();
        sectMeas2 =
        ((CcReal*) actSection->GetAttribute(SECTION_MEAS2))->GetRealval();*/
///////////////////////////////////////////////

        found = true;
        double startRI, endRI;
        int actRouteId =
            ((CcInt*)actSection->GetAttribute(SECTION_RID))->GetIntval();
        if (actPQEntry->upDownFlag == true) {
          startRI = sectMeas1;
          endRI = end->GetPosition();
        } else {
          startRI = sectMeas2;
          endRI = end->GetPosition();
        }

        actSection->DeleteIfAllowed();
/*
Get the sections used for shortest path and write them in right
order (from start to end gpoint) in the resulting gline. Because dijkstra gives
the sections from end to start we first have to put the result sections on a
stack to turn in right order.

*/
        RIStack *riStack = new RIStack(actRouteId, startRI, endRI);
        int lastSectId = actPQEntry->sectID;
        SectIDTree *pElem = visitedSect->Find(actPQEntry->beforeSectID);
        bool end = false;
        bool upDown;
     //   if (startRI >= endRI) upDown = false;
        if(startRI > endRI || fabs(startRI-endRI) < 0.1) upDown = false;
        else upDown = true;
        while (!end) {
          //GetSection
          actSection = pNetwork->GetSection(pElem->sectID);
          actRouteId =
              ((CcInt*)actSection->GetAttribute(SECTION_RID))->GetIntval();
          sectMeas1 =
            ((CcReal*) actSection->GetAttribute(SECTION_MEAS1))->GetRealval();
          sectMeas2 =
            ((CcReal*) actSection->GetAttribute(SECTION_MEAS2))->GetRealval();
          upDown = pElem->upDownFlag;
          if (pElem->sectID != startSectTID){
            if (upDown)
              riStack->Push(actRouteId, sectMeas1, sectMeas2, riStack);
            else
              riStack->Push(actRouteId, sectMeas2, sectMeas1, riStack);
            lastSectId = pElem->sectID;
            pElem = visitedSect->Find(pElem->beforeSectId);
          } else {
            end = true;
            pNetwork->GetAdjacentSections(startSectTID, true, adjSectionList);
            size_t i = 0;
            bool stsectfound = false;
            while (i < adjSectionList.size() && !stsectfound) {
              DirectedSection adjSection = adjSectionList[i];
              if (adjSection.GetSectionTid() == lastSectId){
                  if(fabs(start->GetPosition()-sectMeas2) > 0.1){
                    stsectfound = true;
                    riStack->Push(actRouteId, start->GetPosition(), sectMeas2,
                              riStack);
                    end = true;
                  }
              }
              i++;
            }
            adjSectionList.clear();
            if (!stsectfound) {
              pNetwork->GetAdjacentSections(startSectTID, false,
                                            adjSectionList);
              i = 0;
              while (i < adjSectionList.size() && !stsectfound) {
                DirectedSection adjSection = adjSectionList[i];
                if (adjSection.GetSectionTid() == lastSectId ){
                    if(fabs(start->GetPosition()-sectMeas1) > 0.1){
                      stsectfound = true;
                      riStack->Push(actRouteId, start->GetPosition(), sectMeas1,
                                riStack);
                      end = true;
                    }
                }
                i++;
              }
              adjSectionList.clear();
            }
          }
        }
        // Cleanup and return result
        riStack->StackToGLine(result);
        riStack->RemoveStack();
//        delete actPQEntry;
      }
    }
    if (visitedSect != 0) visitedSect->Remove();
    prioQ->Destroy();
    delete prioQ;
  }
  startSection->DeleteIfAllowed();
  endSection->DeleteIfAllowed();
  NetworkManager::CloseNetwork(pNetwork);
  result->SetSorted(false);
  result->SetDefined(true);
  delete start;
  delete end;
  return true;
};

/*
Returns the x,y point represented by gpoint.

*/

void GPoint::ToPoint(Point *&res) {
  Network *pNetwork = NetworkManager::GetNetwork(GetNetworkId());
  if (pNetwork != 0) {
    pNetwork->GetPointOnRoute(this, res);
  } else {
    res->SetDefined(false);
  }
  NetworkManager::CloseNetwork(pNetwork);
};

Point* GPoint::ToPoint() const{
  Point *res = new Point(false);
  Network *pNetwork = NetworkManager::GetNetwork(GetNetworkId());
  if (pNetwork != 0) pNetwork->GetPointOnRoute(this, res);
  NetworkManager::CloseNetwork(pNetwork);
  return res;
};

Point* GPoint::ToPoint(Network*& pNetwork) const{
  Point *res = new Point(false);
  if (pNetwork != 0) pNetwork->GetPointOnRoute(this, res);
  return res;
};

/*
Returns the bounding GPoints of the given GLine.

*/

GPoints* GLine::GetBGP (){
  GPoints *result = new GPoints(0);
  if (!IsDefined() || NoOfComponents() == 0) return result;
  else {
    SectTree *sectionTree = 0;
    const RouteInterval *ri;
    DBArray<SectTreeEntry> *actSections = new DBArray<SectTreeEntry> (0);
    Network *pNetwork = NetworkManager::GetNetwork(GetNetworkId());
    bool first = true;
    const SectTreeEntry *nEntry;
    for (int i = 0; i < Size(); i++) {
      Get(i,ri);
      pNetwork->GetSectionsOfRouteInterval(ri, actSections);
      for (int j = 0; j < actSections->Size(); j++) {
        actSections->Get(j, nEntry);
        if (first) {
          first = false;
          sectionTree = new SectTree(const_cast<SectTreeEntry*> (nEntry));
        } else {
          sectionTree->Insert(const_cast<SectTreeEntry*> (nEntry));
        }
      }
      actSections->Clear();
    }
    delete actSections;
    sectionTree->WriteResult(pNetwork, *result, *sectionTree);
    sectionTree->Remove();
    NetworkManager::CloseNetwork(pNetwork);
    return result;
  }
};

/*
1.3.3.3 Secondo Type Constructor for class ~GPoint~

*/

TypeConstructor gpoint(
        "gpoint",                                   //name
        GPoint::GPointProperty,                     //property function
        GPoint::OutGPoint, GPoint::InGPoint,        //Out and In functions
        0,                   0,                     //SaveToList and
                                                    //RestoreFromList functions
        GPoint::CreateGPoint, GPoint::DeleteGPoint, //object creation/deletion
        OpenAttribute<GPoint>,SaveAttribute<GPoint>,//open and save functions
        GPoint::CloseGPoint, GPoint::CloneGPoint,   //object close, and clone
        GPoint::CastGPoint,                         //cast function
        GPoint::SizeOfGPoint,                       //sizeof function
        GPoint::CheckGPoint );                      //kind checking function

/*
Class GPoints implemented by Jianqiu Xu

*/

string edistjoinpointlist = "(rel(tuple((pid int)(p point))))";
enum edistjoinpointlistrelation {POINTSID = 0,POINTSOBJECT};

GPoints::GPoints()
{
  del.refs = 1;
  del.isDelete = true;
}
GPoints::GPoints(int in_iSize):m_xGPoints(in_iSize)
{
  del.refs = 1;
  del.isDelete = true;
}
GPoints::GPoints(GPoints* in_xOther):m_xGPoints(0)
{
  const GPoint* pCurrentInterval;
    for (int i = 0; i < in_xOther->m_xGPoints.Size(); i++)
    {
      // Get next Interval
      in_xOther->m_xGPoints.Get(i, pCurrentInterval);
    m_xGPoints.Append(*pCurrentInterval);
    }
    del.refs=1;
    del.isDelete=true;
}
bool GPoints::IsEmpty()const
{
  return m_xGPoints.Size() == 0;
}
ostream& GPoints::Print(ostream& os)const
{
  for(int i = 0;i < m_xGPoints.Size();i++){
    const GPoint* pGP;
    m_xGPoints.Get(i,pGP);
    os<<"GPoint:"<<i<<" rid "<<pGP->GetRouteId();
    os<<" Position "<<pGP->GetPosition();
    os<<" Side "<<(int)pGP->GetSide()<<endl;
  }
  return os;
}
GPoints& GPoints::operator=(const GPoints& gps)
{
  m_xGPoints.Clear();
  const GPoint* gp;
  for(int i = 0 ; i < gps.Size();i++){
    gps.Get(i,gp);
    m_xGPoints.Append(*gp);
  }
  return *this;
}
int GPoints::NumOfFLOBs()const
{
  return 1;
}
FLOB* GPoints::GetFLOB(const int i)
{
  assert(i >= 0 && i < NumOfFLOBs());
  return &m_xGPoints;
}

size_t GPoints::Sizeof()const
{
  return sizeof(*this);
}

int GPoints::SizeOf()
{
  return sizeof(GPoints);
}
int GPoints::Size()const
{
  return m_xGPoints.Size();
}
GPoints& GPoints::operator+=(const GPoint& gp)
{
  m_xGPoints.Append(gp);
  return *this;
}

GPoints& GPoints::operator-=(const GPoint& gp)
{
  GPoints *nGPs = new GPoints(0);
  const GPoint *actGP;
  for (int i = 0; i < m_xGPoints.Size(); i++) {
    m_xGPoints.Get(i, actGP);
    if (gp != *actGP) nGPs->m_xGPoints.Append(actGP);
  }
  return *nGPs;
}

void GPoints::Get(int i,const GPoint*& pgp)const
{
  assert(i >= 0 && i < m_xGPoints.Size());
  m_xGPoints.Get(i,pgp);
}
ListExpr GPoints:: Property()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("gpoints"),
                    nl->StringAtom("(<gpoint1><gpoint2>...<gpointN>))"),
                             nl->StringAtom("((1 2.0 0)(2 3.0 0))"))));

}

ListExpr GPoints::Out(ListExpr in_xTypeInfo,
                    Word in_xValue)
{
  GPoints *pGPS = (GPoints*)in_xValue.addr;

  if(pGPS->IsEmpty())
  {
    return nl->TheEmptyList();
  }
  const GPoint* pgp;
  pGPS->Get(0,pgp);
   ListExpr result =
    nl->OneElemList(GPoint::OutGPoint(nl->TheEmptyList(),SetWord((void*)pgp)));
  ListExpr last = result;
  for(int i = 1; i < pGPS->Size();i++){
    pGPS->Get(i,pgp);
    last =
    nl->Append(last,GPoint::OutGPoint(nl->TheEmptyList(),SetWord((void *)pgp)));
  }
  return result;
}
Word GPoints::In(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct)
{
    if (nl->ListLength(instance) == 0) {
    correct = false;
    cmsg.inFunError("Empty List");
    return SetWord(Address(0));
    }
  GPoints* pGPS = new GPoints(nl->ListLength(instance));
  ListExpr rest = instance;
   while (!nl->IsEmpty(rest)) {
    ListExpr first = nl->First(rest);
    rest = nl->Rest(rest);
    if (nl->ListLength(first) != 4) {
      correct = false;
      cmsg.inFunError("GPoint incorrect.Expected list of 4 Elements.");
      return SetWord(Address(0));
    }
  GPoint* pgp =
   (GPoint*)GPoint::InGPoint(nl->TheEmptyList(),first,0,errorInfo,correct).addr;
  if(correct){
    (*pGPS) += (*pgp);
    delete pgp;
  }else{
    delete pgp;
    return SetWord(Address(0));
  }
  }
  correct = true;
  pGPS->SetDefined(true);
  return SetWord(pGPS);
}
bool GPoints::OpenGPoints(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo, Word& value)
{
  GPoints* pGPS = (GPoints*)Attribute::Open(valueRecord,offset,typeInfo);
  value = SetWord(pGPS);
  return true;
}
bool GPoints::SaveGPoints(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo, Word& value)
{
  GPoints* pGPS = (GPoints*)value.addr;
  Attribute::Save(valueRecord,offset,typeInfo,pGPS);
  return true;
}
Word GPoints::Create(const ListExpr typeInfo)
{
  return SetWord(new GPoints(0));
}
void GPoints::Delete(const ListExpr typeInfo,
                   Word& w )
{
  GPoints *gp = (GPoints *)w.addr;
  // if (l->del.refs == 1) { l->m_xRouteIntervals.Destroy();}
  if(gp->DeleteIfAllowed() == true)
    w.addr = 0;
}
void GPoints::Close(const ListExpr typeInfo,
                  Word& w )
{
    if(((GPoints*)w.addr)->DeleteIfAllowed() == true)
    w.addr = 0;
}
Word GPoints::CloneGPoints(const ListExpr typeInfo,
                  const Word& w )
{
  return SetWord(new GPoints(*((GPoints*)w.addr)));
}
void* GPoints::Cast(void* addr)
{
  return new (addr) GPoints();
}
bool GPoints::Check( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "gpoints" ));
}

int GPoints::Compare(const Attribute*)const
{
  return false;

}
bool GPoints::Adjacent(const Attribute*)const
{
   return false;
}

GPoints* GPoints::Clone()const
{
  GPoints* res = new GPoints(*this);
  return res;
}
size_t GPoints::HashValue()const
{
  size_t xHash = 0;
  // Iterate over all GPoint objects
  for (int i = 0; i < m_xGPoints.Size(); ++i)
  {
    // Get next Interval
    const GPoint* pCurrentInterval;
    m_xGPoints.Get(i, pCurrentInterval);

    // Add something for each entry
  int iNetworkId = pCurrentInterval->GetNetworkId();
    int iRouteId = pCurrentInterval->GetRouteId();
    double iPosition = pCurrentInterval->GetPosition();
    int iSide = (int)pCurrentInterval->GetSide();
    xHash += iNetworkId + iRouteId + (size_t)iPosition + iSide;
  }
  return xHash;
}
void GPoints::CopyFrom(const StandardAttribute* right)
{
  *this = *((const GPoints *)right);
}

TypeConstructor gpoints( "gpoints",
                         GPoints::Property,
                         GPoints::Out, GPoints::In,
                         0, 0,
                         GPoints::Create, GPoints::Delete,
                         GPoints::OpenGPoints, GPoints::SaveGPoints,
                         GPoints::Close, GPoints::CloneGPoints,
                         GPoints::Cast, GPoints::SizeOf,
                         GPoints::Check);



/*
1.4 Secondo Operators

1.4.1 Operator ~netdistance~

Returns the network distance between two ~Gpoints~ or two ~GLines~. Using
Dijkstras Algorithm for computation of the shortest paths.

*/

ListExpr OpNetNetdistanceTypeMap(ListExpr args){
  if (nl->ListLength(args) != 2) {
    return (nl->SymbolAtom("typeerror"));
  }
  ListExpr param1 = nl->First(args);
  ListExpr param2 = nl->Second(args);
  //ListExpr param3 = nl->Third(args);

  if ((nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&
     nl->IsAtom(param2) && nl->AtomType(param2) == SymbolType &&
     ((nl->SymbolValue(param1) == "gpoint" &&
     nl->SymbolValue(param2) == "gpoint") ||
     (nl->SymbolValue(param1) == "gline" &&
     nl->SymbolValue(param2) == "gline")))) {
    return nl->SymbolAtom("real");
  }
  return nl->SymbolAtom("typeerror");
}

int OpNetNetdistance_gpgp (Word* args, Word& result, int message,
      Word& local, Supplier in_pSupplier){
  GPoint* pFromGPoint = (GPoint*) args[0].addr;
  GPoint* pToGPoint = (GPoint*) args[1].addr;
  result = qp->ResultStorage(in_pSupplier);
  CcReal* pResult = (CcReal*) result.addr;
  if (!(pFromGPoint->IsDefined()) || !(pToGPoint->IsDefined())) {
    cmsg.inFunError("Both gpoint must be defined!");
    return 0;
  };
  double dist =  pFromGPoint->Netdistance(pToGPoint);
  if (dist != numeric_limits<double>::max()) pResult->Set(true,dist);
  else pResult->Set(false, dist);
  return 1;
};

int OpNetNetdistance_glgl (Word* args, Word& result, int message,
      Word& local, Supplier in_pSupplier){
  GLine* pGLine1 = (GLine*) args[0].addr;
  GLine* pGLine2 = (GLine*) args[1].addr;
  CcReal* pResult = (CcReal*) qp->ResultStorage(in_pSupplier).addr;
  result = SetWord(pResult);
  if (!(pGLine1->IsDefined()) || !(pGLine2->IsDefined())) {
    cmsg.inFunError("Both gpoint must be defined!");
    return 0;
  };
  double dist = pGLine1->Netdistance(pGLine2);
  if (dist != numeric_limits<double>::max()) pResult->Set(true, dist);
  else pResult->Set(false, dist);
  return 1;
};

ValueMapping OpNetNetdistancemap[] = {
  OpNetNetdistance_gpgp,
  OpNetNetdistance_glgl
};

int OpNetNetdistanceselect (ListExpr args){
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

const string OpNetNetdistanceSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>A x A x network-> real with A = gpoint or gline" "</text--->"
  "<text>netdistance(GPOINT1,GPOINT2, NETWORK)</text--->"
  "<text>Calculates the network distance of two gpoints resp. glines.</text--->"
  "<text>query netdistance(gp1,gp2, B_NETWORK)</text--->"
  ") )";

Operator networknetdistance (
          "netdistance",               // name
          OpNetNetdistanceSpec,          // specification
          2,
          OpNetNetdistancemap,  // value mapping
          OpNetNetdistanceselect,        // selection function
          OpNetNetdistanceTypeMap        // type mapping
);

/*
1.4.2 Operator ~gpoint2rect~

Returns a rectangle degenerated to a point with coordinates rid, rid, pos, pos.

*/

ListExpr OpGPoint2RectTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 ){
    sendMessage("Expects a list of length 1.");
    return (nl->SymbolAtom( "typeerror" ));
  }

  ListExpr xsource = nl->First(in_xArgs);

  if( (!nl->IsAtom(xsource)) ||
      !nl->IsEqual(xsource, "gpoint"))
  {
    sendMessage("Element must be of type gpoint.");
    return (nl->SymbolAtom("typeerror"));
  }
  return (nl->SymbolAtom("rect"));
}

int OpGPoint2RectValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  result = qp->ResultStorage( in_xSupplier );
  Rectangle<2>* box = static_cast<Rectangle<2>* >(result.addr);
  GPoint* arg = static_cast<GPoint*>(args[0].addr);
  if(!arg->IsDefined()){
    box->SetDefined(false);
  } else {
    (*box) = arg->NetBoundingBox();
  }
  return 0;
} //end ValueMapping

const string OpGPoint2RectSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gpoint -> rect" "</text--->"
  "<text>gpoint2rect(GPOINT)</text--->"
  "<text>Returns a rectangle representing the gpoint.</text--->"
  "<text> gpoint2rect (gpoint) </text--->"
  ") )";

Operator networkgpoint2rect (
          "gpoint2rect",               // name
          OpGPoint2RectSpec,          // specification
          OpGPoint2RectValueMapping,  // value mapping
          Operator::SimpleSelect,        // selection function
          OpGPoint2RectTypeMap        // type mapping
);

/*
1.4.3 Operator ~inside~

Returns true if ths ~GPoint~ is inside the ~GLine~ false elsewhere.

*/
ListExpr OpInsideTypeMap(ListExpr args){
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

int OpInsideValueMap (Word* args, Word& result, int message,
      Word& local, Supplier in_pSupplier) {
  GPoint* pGPoint = (GPoint*) args[0].addr;
  GLine* pGLine = (GLine*) args[1].addr;
  result = qp->ResultStorage(in_pSupplier);
  CcBool* pResult = (CcBool*) result.addr;
  if (pGPoint->GetNetworkId() != pGLine->GetNetworkId() ||
     !pGLine->IsDefined() || !pGPoint->IsDefined()) {
    pResult->Set(false, false);
  }
  pResult->Set(true, pGPoint->Inside(pGLine));
  return 0;
}

const string OpInsideSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gpoint x gline -> bool" "</text--->"
  "<text> _ inside _</text--->"
  "<text>Returns true if gpoint is inside gline false elsewhere.</text--->"
  "<text>GPOINT inside GLINE</text--->"
  ") )";

Operator networkinside (
          "inside",               // name
          OpInsideSpec,          // specification
          OpInsideValueMap,  // value mapping
          Operator::SimpleSelect,        // selection function
          OpInsideTypeMap        // type mapping
);

/*
1.4.4 Operator ~length~

Returns the length of a ~GLine~.

*/
ListExpr OpLengthTypeMap(ListExpr args){
  if (nl->ListLength(args) != 1) {
    return (nl->SymbolAtom("typeerror"));
  }
  ListExpr gline = nl->First(args);
  if (!nl->IsAtom(gline) || nl->AtomType(gline) != SymbolType ||
       nl->SymbolValue(gline) != "gline"){
    return (nl->SymbolAtom("typeerror"));
  }
  return nl->SymbolAtom("real");
}

int OpLengthValueMap (Word* args, Word& result, int message,
      Word& local, Supplier in_pSupplier){
  GLine* pGline = (GLine*) args[0].addr;
  result = qp->ResultStorage(in_pSupplier);
  CcReal* pResult = (CcReal*) result.addr;
  if (!(pGline->IsDefined())) {
    cmsg.inFunError("gline is not defined!");
    return 0;
  };
  pResult-> Set(true, pGline->GetLength());
  return 1;
}

const string OpLengthSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gline -> real" "</text--->"
  "<text>length(_)</text--->"
  "<text>Calculates the length of the gline.</text--->"
  "<text>query length(gline)</text--->"
  ") )";

Operator networklength (
          "length",               // name
          OpLengthSpec,          // specification
          OpLengthValueMap,  // value mapping
          Operator::SimpleSelect,        // selection function
          OpLengthTypeMap        // type mapping
);

/*
1.4.5 Operator ~line2gline~

Translates a spatial ~line~ value into a network ~GLine~ value.

*/

ListExpr OpLine2GLineTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 2 ){
    sendMessage("Expects a list of length 2.");
    return (nl->SymbolAtom( "typeerror" ));
  }

  ListExpr xNetworkIdDesc = nl->First(in_xArgs);
  ListExpr xLineDesc = nl->Second(in_xArgs);

  if( (!nl->IsAtom(xNetworkIdDesc)) ||
      !nl->IsEqual(xNetworkIdDesc, "network"))
  {
    sendMessage("First element must be of type network.");
    return (nl->SymbolAtom("typeerror"));
  }

  if( (!nl->IsAtom( xLineDesc )) ||
      nl->AtomType( xLineDesc ) != SymbolType ||
      nl->SymbolValue( xLineDesc ) != "line" )
  {
    sendMessage("Second element must be of type sline.");
    return (nl->SymbolAtom( "typeerror" ));
  }

  return nl->SymbolAtom( "gline" );
}

int OpLine2GLineValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  //Initialize return value
  GLine* pGLine = (GLine*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pGLine );
  // Get and check input values.
  Network* pNetwork = (Network*)args[0].addr;
  GLine *res = new GLine(0);
  if (pNetwork == 0 || !pNetwork->isDefined()) {
    string strMessage = "Network is not defined.";
    cerr << strMessage << endl;
    sendMessage(strMessage);
    res->SetDefined(false);
    result = SetWord(res);
    return 0;
  }
  res->SetNetworkId(pNetwork->GetId());
  Line* pLine = (Line*)args[1].addr;
  if(pLine == NULL || !pLine->IsDefined()) {
    string strMessage = "line does not exist.";
    cerr << strMessage << endl;
    sendMessage(strMessage);
    res->SetDefined(false);
    result = SetWord(res);
    return 0;
  }
  if (pLine->Size() <= 0) {
    string strMessage = "line is empty";
    cerr <<strMessage << endl;
    sendMessage(strMessage);
    res->SetDefined(true);
    result = SetWord(res);
    return 0;
  }

  const HalfSegment *hs;
  pLine->Get(0,hs);
  RouteInterval *ri =
      pNetwork->Find(hs->GetLeftPoint(), hs->GetRightPoint());
  if (ri!= 0) {
    RITree *riTree = new RITree(ri->m_iRouteId, ri->m_dStart, ri->m_dEnd);
    for (int i = 1; i < pLine->Size();i++) {
      pLine->Get(i,hs);
      ri = pNetwork->Find(hs->GetLeftPoint(), hs->GetRightPoint());
      if (ri!=0)
        riTree->Insert(ri->m_iRouteId, ri->m_dStart, ri->m_dEnd);
    }
    riTree->TreeToGLine(res);
    riTree->RemoveTree();
    res->SetDefined(true);
    res->SetSorted(true);
    result = SetWord(res);
  } else {
    res->SetDefined(false);
    result =SetWord(res);
  }
  return 0;
} //end ValueMapping

const string OpLine2GLineSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>network x line -> gline" "</text--->"
  "<text>sline2gline(_,_)</text--->"
  "<text>Translates a line to a gline value.</text--->"
  "<text>line2gline(B_NETWORK, sline)</text--->"
  ") )";

Operator sline2gline (
          "line2gline",               // name
          OpLine2GLineSpec,          // specification
          OpLine2GLineValueMapping,  // value mapping
          Operator::SimpleSelect,        // selection function
          OpLine2GLineTypeMap        // type mapping
);


/*
1.4.6 Operator ~=~

Returns true if 2. ~GPoints~ are at the same position false elsewhere.

*/

ListExpr OpNetEqualTypeMap(ListExpr args){
  if (nl->ListLength(args) != 2) {
    return (nl->SymbolAtom("typeerror"));
  }
  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args);
  if ((nl->IsAtom(first) && nl->AtomType(first) == SymbolType &&
       nl->IsAtom(second) && nl->AtomType(second) == SymbolType)&&
    (( nl->SymbolValue(first) == "gpoint" &&
       nl->SymbolValue(second) == "gpoint") ||
    (nl->SymbolValue(first) == "gline" &&
       nl->SymbolValue(second) == "gline"))){
    return nl->SymbolAtom("bool");
  }
  return nl->SymbolAtom("typeerror");
}

int OpNetEqual_gpgp (Word* args, Word& result, int message,
      Word& local, Supplier in_pSupplier){
  GPoint* p1 = (GPoint*) args[0].addr;
  GPoint* p2 = (GPoint*) args[1].addr;
  result = qp->ResultStorage(in_pSupplier);
  CcBool* pResult = (CcBool*) result.addr;
  if (!(p1->IsDefined()) || !p2->IsDefined()) {
    cmsg.inFunError("Both gpoints must be defined!");
    pResult->Set(false, false);
    return 0;
  };
  pResult-> Set(true, *p1 == *p2);
  return 1;
}


int OpNetEqual_glgl (Word* args, Word& result, int message,
      Word& local, Supplier in_pSupplier){
  GLine* l1 = (GLine*) args[0].addr;
  GLine* l2 = (GLine*) args[1].addr;
  result = qp->ResultStorage(in_pSupplier);
  CcBool* pResult = (CcBool*) result.addr;
  if (!(l1->IsDefined()) || !l2->IsDefined()) {
    cmsg.inFunError("Both glines must be defined!");
    pResult->Set(false, false);
    return 0;
  };
  pResult-> Set(true, *l1 == *l2);
  return 1;
}

ValueMapping OpNetEqualmap[] = {
  OpNetEqual_gpgp,
  OpNetEqual_glgl
};

int OpNetEqualselect (ListExpr args){
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

const string OpNetEqualSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>A x A -> bool for GPoint and GLine" "</text--->"
  "<text> _ = _</text--->"
  "<text>Returns if two gpoints are equal.</text--->"
  "<text>query gpoint1 = gpoint2</text--->"
  ") )";

Operator netgpequal (
          "=",               // name
          OpNetEqualSpec,          // specification
          2,
          OpNetEqualmap,  // value mapping
          OpNetEqualselect,        // selection function
          OpNetEqualTypeMap        // type mapping
);


/*
1.4.7 Operator ~intersects~

Returns true if two ~Gline~ intersect false elsewhere.

*/

ListExpr OpNetIntersectsTypeMap(ListExpr in_xArgs)
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

int OpNetIntersectsValueMapping(Word* args,
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
//  const RouteInterval *pRi1, *pRi2;
  if (pGLine1->GetNetworkId() != pGLine2->GetNetworkId()) {
    cerr << "glines belong to different networks." << endl;
    pResult->Set(true, false);
    return 0;
  }
  pResult->Set(true, pGLine1->Intersects(pGLine2));
  return 0;
}

const string OpNetIntersectsSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>gline x gline -> bool" "</text--->"
  "<text>intersects(gline, gline)</text--->"
  "<text>Returns true if the both glines intersects false elsewhere.</text--->"
  "<text>intersects(gline, gline)</text--->"
  ") )";

Operator networkintersects (
          "intersects",               // name
          OpNetIntersectsSpec,          // specification
          OpNetIntersectsValueMapping,  // value mapping
          Operator::SimpleSelect,        // selection function
          OpNetIntersectsTypeMap        // type mapping
);


/*
1.4.8 Operator ~junctions~

Returns the junctions relation of the network.

*/
ListExpr OpNetworkJunctionsTypeMap(ListExpr args)
{
  ListExpr arg1;
  if( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if( nl->IsAtom( arg1 ) &&
        nl->AtomType( arg1 ) == SymbolType &&
        nl->SymbolValue( arg1 ) == "network" )
    {
      ListExpr xType;
      nl->ReadFromString(Network::junctionsInternalTypeInfo, xType);
      return xType;
    }
  }
  return (nl->SymbolAtom( "typeerror" ));
}

int OpNetworkJunctionsValueMapping( Word* args, Word& result, int message,
                              Word& local, Supplier s )
{
  Network *network = (Network*)args[0].addr;
  result = SetWord( network->GetJunctions() );

  Relation *resultSt = (Relation*)qp->ResultStorage(s).addr;
  resultSt->Close();
  qp->ChangeResultStorage(s, result);

  return 0;
}

const string OpNetworkJunctionsSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>network -> rel" "</text--->"
  "<text>junctions(_)</text--->"
  "<text>Return the junctions of a network.</text--->"
  "<text>query junctions(network)</text--->"
  ") )";

Operator networkjunctions (
          "junctions",                // name
          OpNetworkJunctionsSpec,          // specification
          OpNetworkJunctionsValueMapping,  // value mapping
          Operator::SimpleSelect,            // trivial selection function
          OpNetworkJunctionsTypeMap        // type mapping
);


/*
1.4.9 Operator ~routes~

Returns the routes relation of the network.

*/
ListExpr OpNetworkRoutesTypeMap(ListExpr args)
{
  ListExpr arg1;
  if( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if( nl->IsAtom( arg1 ) &&
        nl->AtomType( arg1 ) == SymbolType &&
        nl->SymbolValue( arg1 ) == "network" )
    {
      ListExpr xType;
      nl->ReadFromString(Network::routesTypeInfo, xType);
      return xType;
    }
  }
  return (nl->SymbolAtom( "typeerror" ));
}

int OpNetworkRoutesValueMapping( Word* args, Word& result, int message,
                           Word& local, Supplier s )
{
  Network *network = (Network*)args[0].addr;
  Relation* pRoute = network->GetRoutes();
  result =  SetWord( pRoute->Clone());
  Relation *resultSt = (Relation*)qp->ResultStorage(s).addr;
  resultSt->Close();
  qp->ChangeResultStorage(s, result);

  return 0;
}

const string OpNetworkRoutesSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>network -> rel" "</text--->"
  "<text>routes(_)</text--->"
  "<text>Return the routes of a network.</text--->"
  "<text>query routes(network)</text--->"
  ") )";

Operator networkroutes (
          "routes",                // name
          OpNetworkRoutesSpec,              // specification
          OpNetworkRoutesValueMapping,      // value mapping
          Operator::SimpleSelect,               // trivial selection function
          OpNetworkRoutesTypeMap            // type mapping
);

/*
1.4.10 Operator ~sections~

Returns the sections relation of the network.

*/

ListExpr OpNetworkSectionsTypeMap(ListExpr args)
{
  ListExpr arg1;
  if( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if( nl->IsAtom( arg1 ) &&
        nl->AtomType( arg1 ) == SymbolType &&
        nl->SymbolValue( arg1 ) == "network" )
    {
      ListExpr xType;
      nl->ReadFromString(Network::sectionsInternalTypeInfo, xType);
      return xType;
    }
  }
  return (nl->SymbolAtom( "typeerror" ));
}

int OpNetworkSectionsValueMapping( Word* args, Word& result, int message,
                             Word& local, Supplier s )
{
  Network *network = (Network*)args[0].addr;
  result = SetWord( network->GetSections() );

  Relation *resultSt = (Relation*)qp->ResultStorage(s).addr;
  resultSt->Close();
  qp->ChangeResultStorage(s, result);

  return 0;
}

const string OpNetworkSectionsSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>network -> rel" "</text--->"
  "<text>sections(_)</text--->"
  "<text>Return the sections of a network.</text--->"
  "<text>query sections(network)</text--->"
  ") )";

Operator networksections (
          "sections",                       // name
          OpNetworkSectionsSpec,          // specification
          OpNetworkSectionsValueMapping,  // value mapping
          Operator::SimpleSelect,           // trivial selection function
          OpNetworkSectionsTypeMap        // type mapping
);


/*
1.4.11 Operator ~thenetwork~

Creates a network with the given id, from the given routes and junctions
relations.

*/
ListExpr OpNetworkTheNetworkTypeMap(ListExpr in_xArgs)
{
  if(nl->ListLength(in_xArgs) != 3)
    return (nl->SymbolAtom("typeerror"));

  ListExpr xIdDesc = nl->First(in_xArgs);
  ListExpr xRoutesRelDesc = nl->Second(in_xArgs);
  ListExpr xJunctionsRelDesc = nl->Third(in_xArgs);

  if(!nl->IsEqual(xIdDesc, "int"))
  {
    return (nl->SymbolAtom("typeerror"));
  }

  if(!IsRelDescription(xRoutesRelDesc) ||
     !IsRelDescription(xJunctionsRelDesc))
  {
    return (nl->SymbolAtom("typeerror"));
  }

  ListExpr xType;
  nl->ReadFromString(Network::routesTypeInfo, xType);
  if(!CompareSchemas(xRoutesRelDesc, xType))
  {
    return (nl->SymbolAtom("typeerror"));
  }

  nl->ReadFromString(Network::junctionsTypeInfo, xType);
  if(!CompareSchemas(xJunctionsRelDesc, xType))
  {
    return (nl->SymbolAtom("typeerror"));
  }

  return nl->SymbolAtom("network");
}

int OpNetworkTheNetworkValueMapping(Word* args, Word& result,
                               int message, Word& local, Supplier s)
{
  Network* pNetwork = (Network*)qp->ResultStorage(s).addr;

  CcInt* pId = (CcInt*)args[0].addr;
  int iId = pId->GetIntval();

  Relation* pRoutes = (Relation*)args[1].addr;
  Relation* pJunctions = (Relation*)args[2].addr;

  pNetwork->Load(iId,
                 pRoutes,
                 pJunctions);
  result = SetWord(pNetwork);

  return 0;
}

const string OpNetworkTheNetworkSpec  =
  "((\"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\") "
  "(<text>int x rel x rel -> network" "</text--->"
  "<text>thenetwork(_, _, _)</text--->"
  "<text>Creates a network.</text--->"
  "<text>let n = thenetwork(int, routes, junctions)</text--->"
  "))";

Operator networkthenetwork (
          "thenetwork",                // name
          OpNetworkTheNetworkSpec,              // specification
          OpNetworkTheNetworkValueMapping,      // value mapping
          Operator::SimpleSelect,               // trivial selection function
          OpNetworkTheNetworkTypeMap            // type mapping
);

/*
1.4.12 Operator ~no\_components~

Returns the number of ~RouteIntervals~ of the ~GLine~.

*/

ListExpr OpNoComponentsTypeMap(ListExpr args){
  if (nl->ListLength(args) != 1) {
    return (nl->SymbolAtom("typeerror"));
  }
  ListExpr gline = nl->First(args);
  if (!nl->IsAtom(gline) || nl->AtomType(gline) != SymbolType ||
       nl->SymbolValue(gline) != "gline"){
    return (nl->SymbolAtom("typeerror"));
  }
  return nl->SymbolAtom("int");
}

int OpNoComponentsValueMapping (Word* args, Word& result, int message,
      Word& local, Supplier in_pSupplier){
  GLine* pGline = (GLine*) args[0].addr;
  result = qp->ResultStorage(in_pSupplier);
  CcInt* pResult = (CcInt*) result.addr;
  if (!(pGline->IsDefined())) {
    pResult->Set(false, 0);
    return 0;
  };
  pResult-> Set(true, pGline->NoOfComponents());
  return 1;
}

const string OpNoComponentsSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gline -> int" "</text--->"
  "<text>no_components(_)</text--->"
  "<text>Returns the number of route intervals.</text--->"
  "<text>query no_components(gline)</text--->"
  ") )";

Operator networknocomponents (
          "no_components",               // name
          OpNoComponentsSpec,          // specification
          OpNoComponentsValueMapping,  // value mapping
          Operator::SimpleSelect,        // selection function
          OpNoComponentsTypeMap        // type mapping
);


/*
1.4.13 Operator ~point2gpoint~

Translates a spatial ~Point~ value into a network ~GPoint~ value.

*/
ListExpr OpPoint2GPointTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 2 ){
    sendMessage("Expects a list of length 2.");
    return (nl->SymbolAtom( "typeerror" ));
  }

  ListExpr xNetworkIdDesc = nl->First(in_xArgs);
  ListExpr xMPointDesc = nl->Second(in_xArgs);

  if( (!nl->IsAtom(xNetworkIdDesc)) ||
      !nl->IsEqual(xNetworkIdDesc, "network"))
  {
    sendMessage("First element must be of type network.");
    return (nl->SymbolAtom("typeerror"));
  }

  if( (!nl->IsAtom( xMPointDesc )) ||
      nl->AtomType( xMPointDesc ) != SymbolType ||
      nl->SymbolValue( xMPointDesc ) != "point" )
  {
    sendMessage("Second element must be of type point.");
    return (nl->SymbolAtom( "typeerror" ));
  }

  return nl->SymbolAtom( "gpoint" );
}

int OpPoint2GPointValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  GPoint* pGPoint = (GPoint*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pGPoint );
  Network* pNetwork = (Network*)args[0].addr;
  if (pNetwork == 0 || !pNetwork->isDefined()) {
    string strMessage = "Network is not defined.";
    cerr << strMessage << endl;
    sendMessage(strMessage);
    pGPoint->SetDefined(false);
    return 0;
  }

  Point* pPoint = (Point*)args[1].addr;
  if(pPoint == NULL || !pPoint->IsDefined()) {
    string strMessage = "Point does not exist.";
    cerr << strMessage << endl;
    sendMessage(strMessage);
    pGPoint->SetDefined(false);
    return 0;
  }
  GPoint *res = pNetwork->GetNetworkPosOfPoint(*pPoint);
  result = SetWord(res);
  /*GPoint *res = pNetwork->GetNetworkPosOfPoint(*pPoint);
  qp->ChangeResultStorage(in_xSupplier, res);*/
  return 0;
} //end ValueMapping

const string OpPoint2GPointSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>network x point -> gpoint" "</text--->"
  "<text>point2gpoint(networkobject, point)</text--->"
  "<text>Translates a point to a gpoint.</text--->"
  "<text>point2gpoint(b_network, point)</text--->"
  ") )";

Operator point2gpoint (
          "point2gpoint",               // name
          OpPoint2GPointSpec,          // specification
          OpPoint2GPointValueMapping,  // value mapping
          Operator::SimpleSelect,        // selection function
          OpPoint2GPointTypeMap        // type mapping
);

/*
1.4.13 Operator ~gpoint2point~

Translates a ~gpoint into a spatial ~Point~ value.

*/
ListExpr OpGPoint2PointTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 ){
    sendMessage("Expects a list of length 1.");
    return (nl->SymbolAtom( "typeerror" ));
  }

  ListExpr xMPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMPointDesc )) ||
      nl->AtomType( xMPointDesc ) != SymbolType ||
      nl->SymbolValue( xMPointDesc ) != "gpoint" )
  {
    sendMessage("Element must be of type gpoint.");
    return (nl->SymbolAtom( "typeerror" ));
  }

  return nl->SymbolAtom( "point" );
}

int OpGPoint2PointValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  Point* pPoint = (Point*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pPoint );
  GPoint* pGPoint = (GPoint*)args[0].addr;
  if(pGPoint == NULL || !pGPoint->IsDefined()) {
    string strMessage = "Point does not exist.";
    cerr << strMessage << endl;
    sendMessage(strMessage);
    pPoint->SetDefined(false);
    return 0;
  }
  pGPoint->ToPoint(pPoint);
  return 0;
} //end ValueMapping

const string OpGPoint2PointSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gpoint -> point" "</text--->"
  "<text>gpoint2point(gpoint)</text--->"
  "<text>Translates a gpoint to a point.</text--->"
  "<text>gpoint2point(gpoint)</text--->"
  ") )";

Operator gpoint2point (
          "gpoint2point",               // name
          OpGPoint2PointSpec,          // specification
          OpGPoint2PointValueMapping,  // value mapping
          Operator::SimpleSelect,        // selection function
          OpGPoint2PointTypeMap        // type mapping
);


/*
1.4.14 Operator ~polygpoint~

Returns a  stream of all ~GPoint~ values which are at the same network position
as the given ~GPoint~. Including the given  ~GPoint~.

*/

ListExpr OpPolyGPointTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 2 ){
    sendMessage("Expects a list of length 1.");
    return (nl->SymbolAtom( "typeerror" ));
  }

  ListExpr xsource = nl->First(in_xArgs);
  ListExpr xnetwork = nl->Second(in_xArgs);

  if( (!nl->IsAtom(xsource)) ||
      !nl->IsEqual(xsource, "gpoint"))
  {
    sendMessage("First Element must be of type gpoint.");
    return (nl->SymbolAtom("typeerror"));
  }
  if( (!nl->IsAtom(xnetwork)) ||
      !nl->IsEqual(xnetwork, "network"))
  {
    sendMessage("Second Element must be of type network.");
    return (nl->SymbolAtom("typeerror"));
  }

  return nl->TwoElemList(nl->SymbolAtom("stream"),
                         nl->SymbolAtom("gpoint"));
}

int OpPolyGPointValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
   GPointList* localinfo;
   const GPoint* res;
   result = qp->ResultStorage(in_xSupplier);
   switch (message){
      case OPEN:
          local = SetWord(new GPointList((GPoint*)args[0].addr,
                                         (Network*)args[1].addr));
          return 0;
      case REQUEST:
           localinfo = (GPointList*) local.addr;
           res = localinfo->NextGPoint();
           if(res==0){
              return CANCEL;
           } else {
              result = SetWord(new GPoint(*res));
              return YIELD;
           }
      case CLOSE:
           if (local.addr != 0) {
             delete (GPointList*) local.addr;
             local = SetWord(Address(0));
           }
           return 0;
   }
   return 0; // ignore unknown message
} //end ValueMapping


const string OpPolyGPointSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gpoint x network -> stream(gpoint)" "</text--->"
  "<text>gpoint polygpoint</text--->"
  "<text>Returns the gpoint and gpoints with the same network position"
    " if the gpoint is a junction.</text--->"
  "<text> polygpoints (gpoint, network) </text--->"
  ") )";

Operator polygpoints (
          "polygpoints",               // name
          OpPolyGPointSpec,          // specification
          OpPolyGPointValueMapping,  // value mapping
          Operator::SimpleSelect,        // selection function
          OpPolyGPointTypeMap        // type mapping
);


/*
1.4.15 Operator ~routeintervals~

Returns a stream of rectangles representing the ~RouteIntervals~ of the ~GLine~.

*/

ListExpr OpRouteIntervalsTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 ){
    sendMessage("Expects a list of length 1.");
    return (nl->SymbolAtom( "typeerror" ));
  }

  ListExpr xsource = nl->First(in_xArgs);

  if( (!nl->IsAtom(xsource)) ||
      !nl->IsEqual(xsource, "gline"))
  {
    sendMessage("First Element must be of type gline.");
    return (nl->SymbolAtom("typeerror"));
  }
  return nl->TwoElemList(nl->SymbolAtom("stream"),
                         nl->SymbolAtom("rect"));
}

int OpRouteIntervalsValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
   RectangleList* localinfo;
   const Rectangle<2>* res;
   result = qp->ResultStorage(in_xSupplier);
   switch (message){
      case OPEN:
          local = SetWord(new RectangleList((GLine*)args[0].addr));
          return 0;
      case REQUEST:
           localinfo = (RectangleList*) local.addr;
           res = localinfo->NextRectangle();
           if(res==0) return CANCEL;
           else {
              result = SetWord(new Rectangle<2>(*res));
              return YIELD;
           }
      case CLOSE:
           if (local.addr != 0) {
             delete (RectangleList*) local.addr;
             local = SetWord(Address(0));
           }
           return 0;
   }
   return 0; // ignore unknown message
} //end ValueMapping

const string OpRouteIntervalsSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gline -> stream(rect)" "</text--->"
  "<text>routeintervals(GLINE)</text--->"
  "<text>Returns a stream of rectangles representing the route intervals of the"
    " gline.</text--->"
  "<text> routeintervals (gline) </text--->"
  ") )";

Operator networkrouteintervals (
          "routeintervals",               // name
          OpRouteIntervalsSpec,          // specification
          OpRouteIntervalsValueMapping,  // value mapping
          Operator::SimpleSelect,        // selection function
          OpRouteIntervalsTypeMap        // type mapping
);


/*
1.4.16 Operator ~shortest\_path~

Returns the shortest path in the ~Network~ between two ~GPoint~. Using Dijkstra
Algorithm to compute the shortest path.

//TODO: Must be maintained to use a really priority Queue.

*/

ListExpr OpShortestPathTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 2 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xGPoint1Desc = nl->First(in_xArgs);
  ListExpr xGPoint2Desc = nl->Second(in_xArgs);
 // ListExpr xGPoint3Desc = nl->Third(in_xArgs);

  if( (!nl->IsAtom( xGPoint1Desc )) ||
      nl->AtomType( xGPoint1Desc ) != SymbolType ||
      nl->SymbolValue( xGPoint1Desc ) != "gpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }

  if( (!nl->IsAtom( xGPoint2Desc )) ||
      nl->AtomType( xGPoint2Desc ) != SymbolType ||
      nl->SymbolValue( xGPoint2Desc ) != "gpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }
  //if((!nl->IsAtom(xGPoint3Desc))|| (!nl->IsEqual(xGPoint3Desc,"network"))) {
  //  return (nl->SymbolAtom("typeerror"));
  //}

  return nl->SymbolAtom( "gline" );
}

int OpShortestPathValueMapping( Word* args,
                                  Word& result,
                                  int message,
                                  Word& local,
                                  Supplier in_xSupplier )
{

  GPoint *pFromGPoint = (GPoint*) args[0].addr;
  GPoint *pToGPoint = (GPoint*) args[1].addr;
  GLine* pGLine = (GLine*) qp->ResultStorage(in_xSupplier).addr;
  result = SetWord(pGLine);
  pGLine->SetSorted(false);
  pGLine->SetDefined(pFromGPoint->ShortestPath(pToGPoint, pGLine));
  return 0;
}

const string OpShortestPathSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gpoint x gpoint -> gline" "</text--->"
  "<text>shortest_path( _ , _)</text--->"
  "<text>Calculates the shortest path between two gpoints.</text--->"
  "<text>query shortest_path(x, y)</text--->"
  ") )";

Operator shortest_path (
          "shortest_path",               // name
          OpShortestPathSpec,          // specification
          OpShortestPathValueMapping,  // value mapping
          Operator::SimpleSelect,        // trivial selection function
          OpShortestPathTypeMap        // type mapping
);

/*
1.4.17 Operator ~gline2line~

Returns the ~line~ value of the given ~GLine~.

*/

ListExpr OpGLine2LineTypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 ){
    sendMessage("Expects a list of length 1.");
    return (nl->SymbolAtom( "typeerror" ));
  }

  ListExpr xsource = nl->First(in_xArgs);

  if( (!nl->IsAtom(xsource)) ||
      !nl->IsEqual(xsource, "gline"))
  {
    sendMessage("Element must be of type gline.");
    return (nl->SymbolAtom("typeerror"));
  }
  return nl->SymbolAtom( "line" );
}

int OpGLine2LineValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  Line* pLine = (Line*) qp->ResultStorage(in_xSupplier).addr;
  result = SetWord(pLine);
  GLine* pGLine = (GLine*)args[0].addr;
  if (pGLine == NULL || !pGLine->IsDefined()) {
    sendMessage("GLine must be defined!");
    pLine->SetDefined(false);
    return 0;
  }
  pGLine->Gline2line(pLine);
  return 0;
}

const string OpGLine2LineSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gline -> line" "</text--->"
  "<text>gline2line(GLINE)</text--->"
  "<text>Returns the line value of the gline.</text--->"
  "<text> gline2line(gline) </text--->"
  ") )";

Operator networkgline2line (
          "gline2line",               // name
          OpGLine2LineSpec,          // specification
          OpGLine2LineValueMapping,  // value mapping
          Operator::SimpleSelect,        // selection function
          OpGLine2LineTypeMap        // type mapping
);

/*
1.4.18 Operator ~isempty~

Returns if the ~GLine~. is empty.

*/
ListExpr OpNetIsEmptyTypeMap(ListExpr args){
  if (nl->ListLength(args) != 1) {
    return (nl->SymbolAtom("typeerror"));
  }
  ListExpr gline = nl->First(args);
  if (!nl->IsAtom(gline) || nl->AtomType(gline) != SymbolType ||
       nl->SymbolValue(gline) != "gline"){
    return (nl->SymbolAtom("typeerror"));
  }
  return nl->SymbolAtom("bool");
}

int OpNetIsEmptyValueMap (Word* args, Word& result, int message,
      Word& local, Supplier in_pSupplier){
  GLine* pGline = (GLine*) args[0].addr;
  result = qp->ResultStorage(in_pSupplier);
  CcBool* pResult = (CcBool*) result.addr;
  if ((!(pGline->IsDefined())) || pGline->NoOfComponents() == 0) {
    pResult->Set(true, true);
    return 0;
  }  else {
    pResult->Set(true,false);
    return 0;
  }
}

const string OpNetIsEmptySpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gline -> bool" "</text--->"
  "<text>isemtpy(_)</text--->"
  "<text>Returns true if the gline is empty.</text--->"
  "<text>query isempty(gline)</text--->"
  ") )";

Operator networkisempty (
          "isempty",               // name
          OpNetIsEmptySpec,          // specification
          OpNetIsEmptyValueMap,  // value mapping
          Operator::SimpleSelect,        // selection function
          OpNetIsEmptyTypeMap        // type mapping
);

/*
1.4.18 Operator ~union~

Builds the union of the two given glines as sorted gline.

*/

ListExpr OpNetUnionTypeMap(ListExpr args){
  if (nl->ListLength(args) != 2) {
    return (nl->SymbolAtom("typeerror"));
  }
  ListExpr gline1 = nl->First(args);
  ListExpr gline2 = nl->Second(args);

  if (!nl->IsAtom(gline1) || nl->AtomType(gline1) != SymbolType ||
       nl->SymbolValue(gline1) != "gline"){
    return (nl->SymbolAtom("typeerror"));
  }

  if (!nl->IsAtom(gline2) || nl->AtomType(gline2) != SymbolType ||
       nl->SymbolValue(gline2) != "gline"){
    return (nl->SymbolAtom("typeerror"));
  }
  return nl->SymbolAtom("gline");
}

int OpNetUnionValueMap (Word* args, Word& result, int message,
      Word& local, Supplier in_pSupplier){
  GLine *pGL1 = (GLine*) args[0].addr;
  GLine *pGL2 = (GLine*) args[1].addr;
  GLine *pGLine = (GLine*) qp->ResultStorage(in_pSupplier).addr;
  result = SetWord(pGLine);
  pGL1->Uniongl(pGL2, pGLine);
  return 0;
}

const string OpNetUnionSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gline x gline -> gline" "</text--->"
  "<text>_ union _ </text--->"
  "<text>Returns the sorted gline resulting of the union of both "
    "gline.</text--->"
  "<text>query gline1 union gline2</text--->"
  ") )";

Operator networkunion (
          "union",               // name
          OpNetUnionSpec,          // specification
          OpNetUnionValueMap,  // value mapping
          Operator::SimpleSelect,        // selection function
          OpNetUnionTypeMap        // type mapping
);

/*
1.4.28 Operator ~distance~

Returns the Euclidean Distance between two ~Gpoints~ or two ~GLines~.

*/

ListExpr OpNetDistanceTypeMap(ListExpr args){
  if (nl->ListLength(args) != 2) {
    return (nl->SymbolAtom("typeerror"));
  }
  ListExpr param1 = nl->First(args);
  ListExpr param2 = nl->Second(args);

  if ((nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&
     nl->IsAtom(param2) && nl->AtomType(param2) == SymbolType &&
     ((nl->SymbolValue(param1) == "gpoint" &&
     nl->SymbolValue(param2) == "gpoint") ||
     (nl->SymbolValue(param1) == "gline" &&
     nl->SymbolValue(param2) == "gline")))) {
    return nl->SymbolAtom("real");
  }
  return nl->SymbolAtom("typeerror");
}

int OpNetDistance_gpgp (Word* args, Word& result, int message,
      Word& local, Supplier in_pSupplier){
  GPoint* pFromGPoint = (GPoint*) args[0].addr;
  GPoint* pToGPoint = (GPoint*) args[1].addr;
  result = qp->ResultStorage(in_pSupplier);
  CcReal* pResult = (CcReal*) result.addr;
  if (!(pFromGPoint->IsDefined()) || !(pToGPoint->IsDefined())) {
    cmsg.inFunError("Both gpoint must be defined!");
    return 0;
  };
  pResult-> Set(true, pFromGPoint->Distance(pToGPoint));
  return 0;
};

int OpNetDistance_glgl (Word* args, Word& result, int message,
      Word& local, Supplier in_pSupplier){
  GLine* pGLine1 = (GLine*) args[0].addr;
  GLine* pGLine2 = (GLine*) args[1].addr;
  result = qp->ResultStorage(in_pSupplier);
  CcReal* pResult = (CcReal*) result.addr;
  if (!(pGLine1->IsDefined()) || !(pGLine2->IsDefined()) ||
     pGLine1->NoOfComponents() == 0 || pGLine2->NoOfComponents() == 0) {
    cmsg.inFunError("Both gline must be defined! And have at least 1 interval");
    return 0;
  };
  pResult-> Set(true, pGLine1->Distance(pGLine2));
  return 1;
};

ValueMapping OpNetDistancemap[] = {
  OpNetDistance_gpgp,
  OpNetDistance_glgl
};

int OpNetDistanceselect (ListExpr args){
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

const string OpNetDistanceSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>A x A -> real with A = gpoint or gline" "</text--->"
  "<text>distance(GPOINT1,GPOINT2)</text--->"
  "<text>Calculates the Euclidean Distance of gpoints resp. glines.</text--->"
  "<text>query distance(gp1,gp2)</text--->"
  ") )";

Operator networkdistance (
          "distance",               // name
          OpNetDistanceSpec,          // specification
          2,
          OpNetDistancemap,  // value mapping
          OpNetDistanceselect,        // selection function
          OpNetDistanceTypeMap        // type mapping
);


/*
1.5 Creating the ~NetworkAlgebra~

*/

class NetworkAlgebra : public Algebra
{
 public:
  NetworkAlgebra() : Algebra()
  {
    AddTypeConstructor( &network );
    AddTypeConstructor( &gpoint );
    AddTypeConstructor( &gline );
    AddTypeConstructor( &gpoints );

    gpoint.AssociateKind( "DATA" );
    gline.AssociateKind( "DATA" );
    network.AssociateKind( "NETWORK" );
    gpoints.AssociateKind("DATA");


    AddOperator(&networkthenetwork);
    AddOperator(&networkroutes);
    AddOperator(&networkjunctions);
    AddOperator(&networksections);
    AddOperator(&shortest_path);
    AddOperator(&networklength);
    AddOperator(&networknetdistance);
    AddOperator(&point2gpoint);
    AddOperator(&gpoint2point);
    AddOperator(&netgpequal);
    AddOperator(&sline2gline);
    AddOperator(&networkinside);
    AddOperator(&networknocomponents);
    AddOperator(&polygpoints);
    AddOperator(&networkrouteintervals);
    AddOperator(&networkintersects);
    AddOperator(&networkgpoint2rect);
    AddOperator(&networkgline2line);
    AddOperator(&networkisempty);
    AddOperator(&networkunion);
    AddOperator(&networkdistance);
  }
  ~NetworkAlgebra() {};
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

extern "C"
Algebra*
InitializeNetworkAlgebra( NestedList* nlRef,
                          QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new NetworkAlgebra());
}
