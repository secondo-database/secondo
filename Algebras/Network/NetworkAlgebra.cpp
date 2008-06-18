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

February 2008 - June 2008 Simone Jandt

1.1 Defines, includes, and constants

*/

#include <sstream>
#include <time.h>

#include "TupleIdentifier.h"
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
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

/*
1.2.2 ~searchRouteInterval~

Method for binary search after a route interval in a sorted ~GLine~.
Used for example by operator ~inside~.

*/

bool searchRouteInterval(GPoint *pGPoint, GLine *pGLine, size_t low,
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

/*
1.2.6 ~chkPoint~

Almost similar to operator ~checkPoint~ but additional returning a difference
value if the point is not exactly on the ~sline~.

Used by operator ~point2gpoint~

*/

bool chkPoint (SimpleLine *route, Point point, bool startSmaller, double &pos,
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

bool chkPoint03 (SimpleLine *route, Point point, bool startSmaller, double &pos,
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

/*
1.2.4 ~checkPoint~

Returns true if a ~point~ is part of a ~sline~, false elsewhere. If the point
is part of the sline his distance from the start is computed also. Used by
operator ~sline2gline~.

*/
bool checkPoint (SimpleLine *route, Point point, bool startSmaller,
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
        if (fabs (pCurrJunction.getRouteMeas() - gp->GetPosition()) < 0.01) {
          found = true;
          test = new GPoint(true, gp->GetNetworkId(),
                            pCurrJunction.getOtherRouteId(),
                            pCurrJunction.getOtherRouteMeas(),
                            None);
          aliasGP.Append(*test);
        }
        i++;
      }
      while (found && i < xJunctions.size()) {
        pCurrJunction = xJunctions[i];
        if (fabs(pCurrJunction.getRouteMeas() - gp->GetPosition()) <0.01) {
          test = new GPoint(true, gp->GetNetworkId(),
                            pCurrJunction.getOtherRouteId(),
                            pCurrJunction.getOtherRouteMeas(),
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
      elem = new Rectangle<2>(true, (double) ri->m_iRouteId,
                              (double) ri->m_iRouteId, ri->m_dStart,
                              ri->m_dEnd);
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
1.2.10 struct ~DijkstraStruct~

Used for Dijkstras-Algorithm to compute the shortest path between two ~GPoint~.

*/

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

/*
1.2.11 class ~PriorityQueue~

Used for shortest path computing in Dijkstras Algorithm.

*/

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

/*
1.2.12 ~Dijkstra~

Modified version of dikstras algorithm calculating shortest path in graphs.

Whereas sedgewick's version of disktra operates on the nodes of the graph this
version will process an array of edges. This change is necessary to take into
account that not all transitions between connecting edges are possible. The
preceding edge has to be known when looking for the next one.

*/
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
      int iAdjacentSectionTid = xAdjacentSection.getSectionTid();
      bool bAdjacentUpDownFlag = xAdjacentSection.getUpDownFlag();
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
      "(rel (tuple ((rid int) (meas1 real) (meas2 real) "
      "(dual bool) (curve sline)(curveStartsSmaller bool) (rrc int))))";


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
m_pBTreeJunctionsByRoute1(0),
m_pBTreeJunctionsByRoute2(0),
m_xAdjacencyList(0),
m_xSubAdjacencyList(0)
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
    delete m_pBTreeJunctionsByRoute1;
    return;
  }

  OpenAdjacencyList(in_xValueRecord,
                    inout_iOffset);

  OpenSubAdjacencyList(in_xValueRecord,
                       inout_iOffset);

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
m_pBTreeJunctionsByRoute1(0),
m_pBTreeJunctionsByRoute2(0),
m_xAdjacencyList(0),
m_xSubAdjacencyList(0)
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
  delete m_pBTreeJunctionsByRoute1;
  delete m_pBTreeJunctionsByRoute2;
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
  m_pBTreeJunctionsByRoute1->DeleteFile();
  delete m_pBTreeJunctionsByRoute1; m_pBTreeJunctionsByRoute1 = 0;
  m_pBTreeJunctionsByRoute2->DeleteFile();
  delete m_pBTreeJunctionsByRoute2; m_pBTreeJunctionsByRoute2 = 0;
  m_xAdjacencyList.Destroy();
  m_xSubAdjacencyList.Destroy();
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
  int iSectionTid;
  while((pRoute = pRoutesIt->GetNextTuple()) != 0)
  {
    iSectionTid = 0;
    // Current position on route - starting at the beginning of the route
    double dCurrentPosOnRoute = 0;
    SimpleLine* pRouteCurve = (SimpleLine*)pRoute->GetAttribute(ROUTE_CURVE);
    int iTupleId = pRoute->GetTupleId();
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
    for(size_t i = 0; i < xJunctions.size(); i++)
    {
      // Get next junction
      xCurrentEntry = xJunctions[i];

      // Find values for the new section
      double dStartPos = dCurrentPosOnRoute;
      double dEndPos = xCurrentEntry.getRouteMeas();

      // If the first junction is at the very start of the route, no
      // section will be added
      if(xCurrentEntry.getRouteMeas() == 0)
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
        pNewSection->PutAttribute(SECTION_RRC, new CcInt(true, iTupleId));
        pNewSection->PutAttribute(SECTION_CURVE, pLine);
        pNewSection->PutAttribute(SECTION_CURVE_STARTS_SMALLER,
                                  new CcBool(true, bLineStartsSmaller));
        m_pSections->AppendTuple(pNewSection);
        iSectionTid = pNewSection->GetTupleId();
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
      if (pRouteCurve->Length() - xCurrentEntry.getRouteMeas() < 0.01) {
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
      int iTupleId = pRoute->GetTupleId();

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
      pNewSection->PutAttribute(SECTION_RRC, new CcInt(true, iTupleId));
      pNewSection->PutAttribute(SECTION_CURVE, pLine);
      pNewSection->PutAttribute(SECTION_CURVE_STARTS_SMALLER,
                                new CcBool(true, bLineStartsSmaller));
      m_pSections->AppendTuple(pNewSection);
      iSectionTid = pNewSection->GetTupleId();
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
        if(xEntryBehind.getRouteMeas() - xEntry.getRouteMeas() < 0.01 )
        {
          // Two junctions at the same place. In this case they do have
          // the same up-pointers
          if(xEntryBehind.m_bFirstRoute)
          {
            int iTid = xEntryBehind.getUpSectionId();
            xAttrs.push_back(new TupleIdentifier(true, iTid));
          }
          else
          {
            int iTid = xEntryBehind.getUpSectionId();
            xAttrs.push_back(new TupleIdentifier(true, iTid));
          }
        }
        else
        {
          // Junctions not on the same place. The down-pointer of the second is
          // the up-pointer of the first.
          if(xEntryBehind.m_bFirstRoute)
          {
            int iTid = xEntryBehind.getDownSectionId();
            xAttrs.push_back(new TupleIdentifier(true, iTid));
          }
          else
          {
            int iTid = xEntryBehind.getDownSectionId();
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
        if (fabs(xEntry.getRouteMeas() - xEntryBehind.getRouteMeas()) < 0.01) {
          if(xEntryBehind.m_bFirstRoute) {
            int iTid = xEntryBehind.getUpSectionId();
            xAttrs.push_back(new TupleIdentifier(true, iTid));
          } else {
            int iTid = xEntryBehind.getUpSectionId();
            xAttrs.push_back(new TupleIdentifier(true, iTid));
          }
          m_pJunctions->UpdateTuple(xEntry.m_pJunction,
                                      xIndices,
                                      xAttrs);
        } else {
          if(xEntryBehind.m_bFirstRoute)
            {
              int iTid = xEntryBehind.getDownSectionId();
              xAttrs.push_back(new TupleIdentifier(true, iTid));
            }
            else
            {
              int iTid = xEntryBehind.getDownSectionId();
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
    CcInt* pCc = (CcInt*)pCurrentJunction->GetAttribute(JUNCTION_CC);
    int iCc = pCc->GetIntval();
    ConnectivityCode xCc(iCc);

    //////////////////////////////////
    //
    // Retrieve the four sections - if they exist
    //
    // (This should also be possible without loading the Section itself)
    //
    TupleIdentifier* pTid;
    Tuple* pAUp = 0;
    Tuple* pADown = 0;
    Tuple* pBUp = 0;
    Tuple* pBDown = 0;

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

    //////////////////////////////////
    //
    // If a section is existing and the transition is possible
    // it will be added to the list.
    //

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
      int iIndex = 2 * (xLastPair.m_iFirstSectionTid - 1);
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

void Network::FillAdjacencyPair(Tuple* in_pFirstSection,
                                    bool in_bFirstUp,
                                    Tuple* in_pSecondSection,
                                    bool in_bSecondUp,
                                    ConnectivityCode in_xCc,
                                    Transition in_xTransition,
                                    vector<DirectedSectionPair> &inout_xPairs)
{
  if(in_pFirstSection != 0 &&
     in_pSecondSection != 0 &&
     in_xCc.IsPossible(in_xTransition))
    {
      inout_xPairs.push_back(DirectedSectionPair(in_pFirstSection->GetTupleId(),
                                                in_bFirstUp,
                                                in_pSecondSection->GetTupleId(),
                                                in_bSecondUp));
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

Tuple* Network::GetSectionOnRoute(GPoint* in_xGPoint)
{
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
    iSectionId = xCurrentEntry.getDownSectionId();
    juncpos = xCurrentEntry.getRouteMeas();
    if(juncpos > in_xGPoint->GetPosition())
    {
      break;
    }
    if (juncpos != 0 && fabs(juncpos - in_xGPoint->GetPosition()) < 0.01){
      break;
    }
    iSectionId = xCurrentEntry.getUpSectionId();
  }
  for(size_t i = 0; i < xJunctions.size(); i++)
  {
    // Get next junction
    JunctionSortEntry xCurrentEntry = xJunctions[i];
    xCurrentEntry.m_pJunction->DeleteIfAllowed();
  }

  if(iSectionId == 0)
  {
    return 0;
  }
  Tuple* pSection = m_pSections->GetTuple(iSectionId);


  // Return the section
  return pSection;
}

Point* Network::GetPointOnRoute(GPoint* in_pGPoint)
{
  CcInt* pRouteId = new CcInt(true, in_pGPoint->GetRouteId());

  BTreeIterator* pRoutesIter = m_pBTreeRoutes->ExactMatch(pRouteId);
  delete pRouteId;

  int NextSuccess = pRoutesIter->Next();
  assert(NextSuccess); // No ASSERT with side effect, please!
  Tuple* pRoute = m_pRoutes->GetTuple(pRoutesIter->GetId());
  assert(pRoute != 0);
  SimpleLine* pLine = (SimpleLine*)pRoute->GetAttribute(ROUTE_CURVE);
  assert(pLine != 0);
  Point* pPoint = new Point(false);
  pLine->AtPosition(in_pGPoint->GetPosition(),true, *pPoint);
  pRoute->DeleteIfAllowed();
  delete pRoutesIter;
  return pPoint;
}

Relation* Network::GetSectionsInternal()
{
  return m_pSections;
}

Relation *Network::GetSections()
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

void Network::GetAdjacentSections(int in_iSectionId,
                                  bool in_bUpDown,
                                  vector<DirectedSection> &inout_xSections)
{
  int iIndex = (2 * (in_iSectionId - 1)) + (in_bUpDown ? 1 : 0);
  const AdjacencyListEntry* xEntry;
  m_xAdjacencyList.Get(iIndex, xEntry);
  if(xEntry->m_iHigh != -1)
  {
    int iLow = xEntry->m_iLow;
    int iHigh = xEntry->m_iHigh;

    for (int i = iLow; i <= iHigh; i++)
    {
      const DirectedSection* xSection;
      m_xSubAdjacencyList.Get(i, xSection);

      bool bUpDownFlag = ((DirectedSection*)xSection)->getUpDownFlag();
      int iSectionTid = ((DirectedSection*)xSection)->getSectionTid();
      inout_xSections.push_back(DirectedSection(iSectionTid,
      bUpDownFlag));

    }
  }
}

/*
~Out~-function of type constructor ~network~

*/
ListExpr Network::Out(ListExpr typeInfo)
{

  ///////////////////////
  // Output of all routes
  GenericRelationIterator *pRoutesIter = m_pRoutes->MakeScan();
  Tuple *pCurrentRoute;
  ListExpr xLast, xNext, xRoutes;
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
  ListExpr xJunctions;
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
  SaveAdjacencyList(in_xValueRecord,
                    inout_iOffset,
                    fileId);

  SaveSubAdjacencyList(in_xValueRecord,
                       inout_iOffset,
                       fileId);

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
    return SetWord(0);
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
  n->Destroy();
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

void Network::SaveAdjacencyList(SmiRecord& in_xValueRecord,
                                size_t& inout_iOffset,
                                SmiFileId& fileId)
{

 /*
  // old implementation
  int iSize = m_xAdjacencyList.Size();
  in_xValueRecord.Write(&iSize,
                        sizeof(int),
                        inout_iOffset);
  inout_iOffset += sizeof(int);

  for (int i = 0; i < m_xAdjacencyList.Size(); ++i)
  {
    // Read current entry
    const AdjacencyListEntry* xEntry;
    m_xAdjacencyList.Get(i, xEntry);

    // Write high
    in_xValueRecord.Write(&xEntry->m_iHigh,
                          sizeof(int),
                          inout_iOffset);
    inout_iOffset += sizeof(int);

    // Write low
    in_xValueRecord.Write(&xEntry->m_iLow,
                          sizeof(int),
                          inout_iOffset);
    inout_iOffset += sizeof(int);
  }
 */

 // new implementation by Th. Behr

 // save to a lob if required
 if(m_xAdjacencyList.IsLob()){
    m_xAdjacencyList.SaveToLob(fileId);
 }

 // write the root of the DBArray

 int size = sizeof(DBArray<AdjacencyListEntry>);
 in_xValueRecord.Write(&m_xAdjacencyList,size,inout_iOffset);
 inout_iOffset += size;

 if(!m_xAdjacencyList.IsLob()){
    // not a lob, store the data into the record
    int extensionsize = m_xAdjacencyList.GetFLOBSize();
    if(extensionsize>0){
       char* extensionElement = (char *) malloc(extensionsize);
       m_xAdjacencyList.WriteTo(extensionElement);
       in_xValueRecord.Write(extensionElement,extensionsize,inout_iOffset);
       inout_iOffset += extensionsize;
       free(extensionElement);
    }
 }

}

void Network::SaveSubAdjacencyList(SmiRecord& in_xValueRecord,
                                   size_t& inout_iOffset,
                                   SmiFileId& fileId)
{
  /*
  // old implementation
  int iSize = m_xSubAdjacencyList.Size();
  in_xValueRecord.Write(&iSize,
                        sizeof(int),
                        inout_iOffset);
  inout_iOffset += sizeof(int);

  for (int i = 0; i < m_xSubAdjacencyList.Size(); ++i)
  {
    // Read current entry
    const DirectedSection* xEntry;
    m_xSubAdjacencyList.Get(i, xEntry);

    // Write high
    int iSectionTid = ((DirectedSection*)xEntry)->getSectionTid();
    in_xValueRecord.Write(&iSectionTid,
                          sizeof(int),
                          inout_iOffset);
    inout_iOffset += sizeof(int);

    // Write low
    bool bUpDownFlag = ((DirectedSection*)xEntry)->getUpDownFlag();
    in_xValueRecord.Write(&bUpDownFlag,
                          sizeof(bool),
                          inout_iOffset);
    inout_iOffset += sizeof(bool);
  }

  */

  // new Implementation by Th. Behr

  // save lob data
  if(m_xSubAdjacencyList.IsLob()){
     m_xSubAdjacencyList.SaveToLob(fileId);
  }
  // save the root of the DBArray
  int size = sizeof(DBArray<DirectedSection>);
  in_xValueRecord.Write(&m_xSubAdjacencyList,size,inout_iOffset);
  inout_iOffset += size;

  int extensionsize = 0;
  char *extensionElement = 0;
  if(!m_xSubAdjacencyList.IsLob()){
    // not a lob, save the data into the record
    extensionsize = m_xSubAdjacencyList.GetFLOBSize();
    if(extensionsize>0){
       extensionElement = (char *) malloc(extensionsize);
       m_xSubAdjacencyList.WriteTo(extensionElement);
       in_xValueRecord.Write(extensionElement,extensionsize,inout_iOffset);
       inout_iOffset += extensionsize;
       free(extensionElement);
    }
 }
}

void Network::OpenAdjacencyList(SmiRecord& in_xValueRecord,
                         size_t& inout_iOffset)
{
  /*
  // old implementation
  // Read length from record
  int iSize;
  in_xValueRecord.Read( &iSize,
                        sizeof( int ),
                        inout_iOffset );
  inout_iOffset += sizeof( int );

  for (int i = 0; i < iSize; ++i)
  {
  // Read high
  int iHigh;
  in_xValueRecord.Read( &iHigh,
                        sizeof( int ),
                        inout_iOffset );
  inout_iOffset += sizeof( int );

  // Read low
  int iLow;
  in_xValueRecord.Read( &iLow,
                        sizeof( int ),
                        inout_iOffset );
  inout_iOffset += sizeof( int );

    m_xAdjacencyList.Append(AdjacencyListEntry(iLow, iHigh));
  }
  */

  // new implementation by Th. Behr

  // read the root of the dbarray
  int size = sizeof(DBArray<AdjacencyListEntry>);
  in_xValueRecord.Read(&m_xAdjacencyList,size,inout_iOffset);
  inout_iOffset += size;
  // cast the data to a new DBArray
  new (&m_xAdjacencyList) DBArray<AdjacencyListEntry>;
  // if the array is no lob, read the data directly
  if(!m_xAdjacencyList.IsLob()){
     unsigned int extSize = m_xAdjacencyList.GetFLOBSize();
     char* data = (char*) malloc(extSize);
     in_xValueRecord.Read(data,extSize,inout_iOffset);
     inout_iOffset += extSize;
     m_xAdjacencyList.ReadFrom(data);
  }
}

void Network::OpenSubAdjacencyList(SmiRecord& in_xValueRecord,
                            size_t& inout_iOffset)
{

  /*
  // Read length from record
  int iSize;
  in_xValueRecord.Read( &iSize,
                        sizeof( int ),
                        inout_iOffset );
  inout_iOffset += sizeof( int );

  for (int i = 0; i < iSize; ++i)
  {
  // Read SectionTid
  int iSectionTid;
  in_xValueRecord.Read( &iSectionTid,
                        sizeof( int ),
                        inout_iOffset );
  inout_iOffset += sizeof( int );

  // Read UpDownFlag
  bool bUpDownFlag;
  in_xValueRecord.Read( &bUpDownFlag,
                        sizeof( bool ),
                        inout_iOffset );
  inout_iOffset += sizeof( bool );

    m_xSubAdjacencyList.Append(DirectedSection(iSectionTid,
                                               bUpDownFlag));
  }
  */

  // new implementation by Th. Behr

  // read the dbarray's root
  int size = sizeof(DBArray<DirectedSection>);
  in_xValueRecord.Read(&m_xSubAdjacencyList,size,inout_iOffset);
  inout_iOffset += size;
  // cast the data to a new DBArray
  new (&m_xSubAdjacencyList) DBArray<DirectedSection>;
  if(!m_xSubAdjacencyList.IsLob()){
     // read non-lob data
     unsigned int extSize = m_xSubAdjacencyList.GetFLOBSize();
     char* data = (char*) malloc(extSize);
     in_xValueRecord.Read(data,extSize,inout_iOffset);
     inout_iOffset += extSize;
     m_xSubAdjacencyList.ReadFrom(data);
  }
}

int Network::isDefined() {
  return m_bDefined;
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
    rid1meas = 0.0;
    rid2meas = 0.0;
  }
}

/*
Return sLine Value from RouteId

*/

void Network::GetLineValueOfRouteInterval (const RouteInterval *in_ri,
                                           SimpleLine &out_Line){
  CcInt* pRouteId = new CcInt(true, in_ri->m_iRouteId);
  BTreeIterator* pRoutesIter = m_pBTreeRoutes->ExactMatch(pRouteId);
  delete pRouteId;
  int NextSuccess = pRoutesIter->Next();
  assert(NextSuccess); // No ASSERT with side effect, please!
  Tuple* pRoute = m_pRoutes->GetTuple(pRoutesIter->GetId());
  assert(pRoute != 0);
  SimpleLine* pLine = (SimpleLine*)pRoute->GetAttribute(ROUTE_CURVE);
  assert(pLine != 0);
  bool startSmaller =
      ((CcBool*) pRoute->GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();
  pLine->SubLine(in_ri->m_dStart, in_ri->m_dEnd, startSmaller, out_Line);
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
  }

  GLine::GLine(int in_iSize):
    m_xRouteIntervals(in_iSize)
  {
    m_bSorted = false;
    m_dLength = 0;
  }

  GLine::GLine(GLine* in_xOther):
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
}

/*
1.3.2.2 Methods of class ~GLine~

*/
void GLine::SetNetworkId(int in_iNetworkId)
{
  m_iNetworkId = in_iNetworkId;
  m_bDefined = true;
}

void GLine::AddRouteInterval(int in_iRouteId,
                             double in_dStart,
                             double in_dEnd)
{
  m_xRouteIntervals.Append(RouteInterval(in_iRouteId,
                                         in_dStart,
                                         in_dEnd));
  m_dLength = m_dLength + fabs(in_dEnd - in_dStart);
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
               const int errorPos, ListExpr& errorInfo, bool& correct){
  GLine* pGline = new GLine(0);
  if (nl->ListLength(instance) != 2) {
    correct = false;
    cmsg.inFunError("Expecting (networkid (list of routeintervals))");
    return SetWord(Address(0));
  }
  ListExpr FirstElem = nl->First(instance);
  ListExpr SecondElem = nl->Second(instance);
  if (!nl->IsAtom(FirstElem) || !nl->AtomType(FirstElem)== IntType) {
    correct = false;
    cmsg.inFunError("Networkadress is not evaluable");
    return SetWord(Address(0));
  }
  if (nl->IsEmpty(SecondElem)) {
    correct = false;
    cmsg.inFunError("List of routeintervals is empty.");
    return SetWord(Address(0));
  }
  pGline->SetNetworkId(nl->IntValue(FirstElem));
  while (!nl->IsEmpty(SecondElem)) {
    ListExpr start = nl->First(SecondElem);
    SecondElem = nl->Rest(SecondElem);
    if (nl->ListLength(start) != 3) {
      correct = false;
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

  if(!pGline->IsDefined())
  {
    return nl->SymbolAtom( "undef" );
  }

  ListExpr xLast, xNext;
  bool bFirst = true;
  ListExpr xNetworkId = nl->IntAtom(pGline->m_iNetworkId);
  ListExpr xRouteIntervals;

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

void GLine::Delete(const ListExpr typeInfo,
                   Word& w )
{
 // GLine *l = (GLine *)w.addr;
  w.addr = 0;
}

void GLine::Close(const ListExpr typeInfo,
                  Word& w )
{
  delete (GLine*)w.addr;
  w.addr = 0;
}

Word GLine::Clone(const ListExpr typeInfo,
                  const Word& w )
{
  return SetWord( ((GLine*)w.addr)->Clone() );
}

void* GLine::Cast(void* addr)
{
  return new (addr) GLine;
}

int GLine::SizeOf()
{
  return sizeof(GLine);
}

size_t GLine::Sizeof() const
{
  return sizeof(GLine);
}

bool GLine::Adjacent( const Attribute* arg ) const
{
  return false;
}

/*
Compare not implemented yet

*/
int GLine::Compare( const Attribute* arg ) const
{
  return false;
}

Attribute* GLine::Clone() const
{
  GLine* xOther = (GLine*)this;
  return new GLine(xOther);
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
  return &m_xRouteIntervals;
}

void GLine::CopyFrom(const StandardAttribute* right)
{
  m_xRouteIntervals.Clear();
  *this = *(const GLine *)right;
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
Distance method computes the network distance between two glines. Uses network
distance method of ~GPoint~.

*/

double GLine::distance (GLine* pgl2){
  GLine* pgl1 = (GLine*) this;
  double minDist = numeric_limits<double>::max();
  double aktDist = numeric_limits<double>::max();
  double lastDist = numeric_limits<double>::max();
  Network* pNetwork = NetworkManager::GetNetwork(pgl1->GetNetworkId());
  if (pNetwork == NULL) {
    cmsg.inFunError("Network not correct.");
    NetworkManager::CloseNetwork(pNetwork);
    return minDist;
  }
  if (pgl1->GetNetworkId() != pNetwork->GetId() ||
      pgl2->GetNetworkId() != pNetwork->GetId()) {
    cmsg.inFunError("Both glines must belong to the network.");
    NetworkManager::CloseNetwork(pNetwork);
    return minDist;
  }
  vector<JunctionSortEntry> juncsRoute;
  juncsRoute.clear();
  JunctionSortEntry pCurrJunc;
  pCurrJunc.m_pJunction = 0;
  vector<GPoint> gpointlistgline1;
  gpointlistgline1.clear();
  vector<GPoint> gpointlistgline2;
  gpointlistgline2.clear();
  const RouteInterval *pCurrRInterval1, *pCurrRInterval2;
  int i, j;
  size_t k;
  i = 0;
  j = 0;
  while (i < pgl1->NoOfComponents()) {
    pgl1->Get(i, pCurrRInterval1);
    while (j < pgl2->NoOfComponents()){
      pgl2->Get(j, pCurrRInterval2);
      if (pCurrRInterval1->m_iRouteId == pCurrRInterval2->m_iRouteId) {
        if ((pCurrRInterval1->m_dStart <= pCurrRInterval2->m_dStart &&
           pCurrRInterval2->m_dStart <= pCurrRInterval1->m_dEnd) ||
           (pCurrRInterval2->m_dStart <= pCurrRInterval1->m_dStart &&
           pCurrRInterval1->m_dStart <= pCurrRInterval2->m_dEnd) ||
           (pCurrRInterval1->m_dStart <= pCurrRInterval2->m_dEnd &&
           pCurrRInterval2->m_dEnd <= pCurrRInterval1->m_dEnd) ||
           (pCurrRInterval2->m_dStart <= pCurrRInterval1->m_dEnd &&
           pCurrRInterval1->m_dEnd <= pCurrRInterval2->m_dEnd)) {
          minDist = 0.0;
          return minDist;
        } else {
          aktDist = fabs(pCurrRInterval1->m_dStart - pCurrRInterval2->m_dStart);
          if (aktDist < minDist) minDist = aktDist;
          aktDist = fabs(pCurrRInterval1->m_dEnd - pCurrRInterval2->m_dStart);
          if (aktDist < minDist) minDist = aktDist;
          aktDist = fabs(pCurrRInterval1->m_dStart - pCurrRInterval2->m_dEnd);
          if (aktDist < minDist) minDist = aktDist;
          aktDist = fabs(pCurrRInterval1->m_dStart - pCurrRInterval2->m_dEnd);
          if (aktDist < minDist) minDist = aktDist;
        }
      }
      j++;
    }
    i++;
  }
  i = 0;
  while (i < pgl1->NoOfComponents()) {
    pgl1->Get(i, pCurrRInterval1);
    gpointlistgline1.push_back(GPoint(true, pgl1->GetNetworkId(),
                             pCurrRInterval1->m_iRouteId,
                             pCurrRInterval1->m_dStart));
    juncsRoute.clear();
    CcInt iRouteId(true, pCurrRInterval1->m_iRouteId);
    pNetwork->GetJunctionsOnRoute(&iRouteId, juncsRoute);
    k = 0;
    while ( k < juncsRoute.size()) {
      pCurrJunc = juncsRoute[k];
      if ((pCurrRInterval1->m_dStart < pCurrJunc.getRouteMeas() &&
          pCurrRInterval1->m_dEnd > pCurrJunc.getRouteMeas()) ||
          (pCurrRInterval1->m_dStart > pCurrJunc.getRouteMeas() &&
          pCurrRInterval1->m_dEnd < pCurrJunc.getRouteMeas())) {
        gpointlistgline1.push_back(GPoint(true, pgl1->GetNetworkId(),
                                 pCurrRInterval1->m_iRouteId,
                                 pCurrJunc.getRouteMeas()));
      }
      k++;
    }
    gpointlistgline1.push_back(GPoint(true, pgl1->GetNetworkId(),
                             pCurrRInterval1->m_iRouteId,
                             pCurrRInterval1->m_dEnd));
    i++;
  }
  j= 0;
  while (j < pgl2->NoOfComponents()){
    pgl2->Get(j, pCurrRInterval2);
    gpointlistgline2.push_back(GPoint(true, pgl2->GetNetworkId(),
                             pCurrRInterval2->m_iRouteId,
                             pCurrRInterval2->m_dStart));
    juncsRoute.clear();
    CcInt iRouteId(true, pCurrRInterval1->m_iRouteId);
    pNetwork->GetJunctionsOnRoute(&iRouteId, juncsRoute);
    k = 0;
    while ( k < juncsRoute.size()) {
      pCurrJunc = juncsRoute[k];
      if ((pCurrRInterval2->m_dStart < pCurrJunc.getRouteMeas() &&
           pCurrRInterval2->m_dEnd > pCurrJunc.getRouteMeas()) ||
          (pCurrRInterval1->m_dStart > pCurrJunc.getRouteMeas() &&
           pCurrRInterval1->m_dEnd < pCurrJunc.getRouteMeas())) {
        gpointlistgline2.push_back(GPoint(true, pgl1->GetNetworkId(),
                                      pCurrRInterval2->m_iRouteId,
                                      pCurrJunc.getRouteMeas()));
      }
      k++;
    }
    gpointlistgline2.push_back(GPoint(true, pgl1->GetNetworkId(),
                                   pCurrRInterval2->m_iRouteId,
                                   pCurrRInterval2->m_dEnd));
    j++;
  }
  for (size_t l = 0; l < gpointlistgline1.size(); l++){
    lastDist = numeric_limits<double>::max();
    for (size_t m = 0; m < gpointlistgline2.size(); m++) {
      if (gpointlistgline1[l].GetRouteId() != gpointlistgline2[m].GetRouteId()){
        aktDist = gpointlistgline1[l].distance(&gpointlistgline2[m]);
        if (aktDist < minDist) minDist = aktDist;
        if (minDist == 0) {
          juncsRoute.clear();
          gpointlistgline1.clear();
          gpointlistgline2.clear();
          NetworkManager::CloseNetwork(pNetwork);
          return minDist;
        }
        if (aktDist > lastDist) {
          int aktRouteId = gpointlistgline2[m].GetRouteId();
          while (m < gpointlistgline2.size()-1 &&
                 gpointlistgline2[m+1].GetRouteId() == aktRouteId) {
            m++;
          }
        } else {
          lastDist = aktDist;
        }
      }
      if ((m < gpointlistgline2.size()-1) &&
           (gpointlistgline2[m].GetRouteId() !=
            gpointlistgline2[m+1].GetRouteId()))
          lastDist = numeric_limits<double>::max();
    }
  }
  juncsRoute.clear();
  gpointlistgline1.clear();
  gpointlistgline2.clear();
  NetworkManager::CloseNetwork(pNetwork);
  return minDist;
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
        GLine::Close, GLine::Clone,    //object close, and clone
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
Distance function computes the network distance between two ~GPoint~s.
Using Dijkstras-Algorithm for shortest path computing

*/

double GPoint::distance (GPoint* pToGPoint){
  GPoint* pFromGPoint = (GPoint*) this;
  GLine* pGLine = new GLine(0);
  cout << *pFromGPoint;
  cout << *pToGPoint << endl;
  Network *pNetwork = NetworkManager::GetNetwork(pFromGPoint->GetNetworkId());
  if (pFromGPoint->GetNetworkId() != pNetwork->GetId() ||
     pToGPoint->GetNetworkId() != pNetwork->GetId()) {
    cmsg.inFunError("Both gpoints must belong to the network.");
    NetworkManager::CloseNetwork(pNetwork);
    return 0.0;
  };
  pGLine->SetNetworkId(pNetwork->GetId());
  Tuple* pFromSection = pNetwork->GetSectionOnRoute(pFromGPoint);
  Tuple* pToSection = pNetwork->GetSectionOnRoute(pToGPoint);
  Point* pToPoint = pNetwork->GetPointOnRoute(pToGPoint);
  if (pToSection == 0 || pFromSection == 0) {
     cmsg.inFunError("Start or End not found.");
     if (pFromSection != 0 ) pFromSection->DeleteIfAllowed();
     if (pToSection != 0) pToSection->DeleteIfAllowed();
     NetworkManager::CloseNetwork(pNetwork);
     return 0.0;
  }
  Dijkstra(pNetwork, pFromSection->GetTupleId(), pFromGPoint,
           pToSection->GetTupleId(), pToGPoint, pToPoint, pGLine);
  delete pToPoint;
  pFromSection->DeleteIfAllowed();
  pToSection->DeleteIfAllowed();
  NetworkManager::CloseNetwork(pNetwork);
  return pGLine->GetLength();
}

bool GPoint::operator== (const GPoint& p) const{
  if (!m_bDefined || !p.IsDefined()) {
    return false;
  } else {
    if (m_iNetworkId == p.GetNetworkId() &&
      m_xRouteLocation.rid == p.GetRouteId() &&
      m_xRouteLocation.d == p.GetPosition() &&
      (m_xRouteLocation.side == p.GetSide() || m_xRouteLocation.side == 2 ||
       p.GetSide() == 2)) {
      return true;
    } else {
      return false;
    }
  }
}

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
1.4 Secondo Operators

1.4.1 Operator ~distance~

Returns the network distance between two ~Gpoints~ or two ~GLines~. Using
Dijkstras Algorithm for computation of the shortest paths.

*/

ListExpr OpNetDistanceTypeMap(ListExpr args){
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
  pResult-> Set(true, pFromGPoint->distance(pToGPoint));
  return 1;
};

int OpNetDistance_glgl (Word* args, Word& result, int message,
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
  "( <text>A x A x network-> real with A = gpoint or gline" "</text--->"
  "<text>netdistance(GPOINT1,GPOINT2, NETWORK)</text--->"
  "<text>Calculates the network distance of two gpoints resp. glines.</text--->"
  "<text>query netdistance(gp1,gp2, B_NETWORK)</text--->"
  ") )";

Operator networkdistance (
          "netdistance",               // name
          OpNetDistanceSpec,          // specification
          2,
          OpNetDistancemap,  // value mapping
          OpNetDistanceselect,        // selection function
          OpNetDistanceTypeMap        // type mapping
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
   GPoint *pGPoint = (GPoint*) args[0].addr;
   result = qp->ResultStorage(in_xSupplier);
   if (pGPoint == NULL || !pGPoint->IsDefined()) {
     result = SetWord(false);
     return 0;
   }
   result = SetWord(new Rectangle<2>(true, (double) pGPoint->GetRouteId(),
                              (double) pGPoint->GetRouteId(),
                              pGPoint->GetPosition(),
                              pGPoint->GetPosition()));
   return 0; // ignore unknown message
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
  if (pGLine->IsSorted()) {
    if (searchRouteInterval(pGPoint, pGLine, 0, pGLine->NoOfComponents()-1)){
       pResult->Set(true, true);
       return 0;
    }
    pResult->Set(true, false);
    return 0;
  } else {
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
    return 0;
  }
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
1.4.5 Operator ~sline2gline~

Translates a spatial ~sline~ value into a network ~GLine~ value.

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
      nl->SymbolValue( xLineDesc ) != "sline" )
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
  if (pNetwork == 0 || !pNetwork->isDefined()) {
    string strMessage = "Network is not defined.";
    cerr << strMessage << endl;
    sendMessage(strMessage);
    pGLine->SetDefined(false);
    return 0;
  }
  pGLine->SetNetworkId(pNetwork->GetId());
  SimpleLine* pLine = (SimpleLine*)args[1].addr;
  if(pLine == NULL || !pLine->IsDefined()) {
    string strMessage = "sline does not exist.";
    cerr << strMessage << endl;
    sendMessage(strMessage);
    pGLine->SetDefined(false);
    return 0;
  }
  if (pLine->Size() == 0) {
    pGLine->SetDefined(true);
    return 0;
  }
  const HalfSegment *hs;
  Relation *pRoutes = pNetwork->GetRoutes();
  RITree *tree;
  pLine->Get(0,hs);
  Tuple *pCurrentRoute;
  int iRouteTid, iRouteId;
  CcInt *pRouteId;
  SimpleLine *pRouteCurve;
  bool bLeftFound, bRightFound;
  double leftPos, rightPos;
  GenericRelationIterator* pRoutesIt = pRoutes->MakeScan();
  bool found = false;
  while( (pCurrentRoute = pRoutesIt->GetNextTuple()) != 0 && !found) {
    iRouteTid = -1;
    pRouteId = (CcInt*) pCurrentRoute->GetAttribute(ROUTE_ID);
    iRouteId = pRouteId->GetIntval();
    pRouteCurve = (SimpleLine*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
    bLeftFound = checkPoint(pRouteCurve, hs->GetLeftPoint(), true, leftPos);
    if (bLeftFound) {
      bRightFound =
          checkPoint(pRouteCurve, hs->GetRightPoint(), true, rightPos);
      if (bRightFound) {
        found = true;
        if (leftPos > rightPos) {
          tree = new RITree(iRouteId, rightPos, leftPos, 0,0);
        } else {
          tree = new RITree(iRouteId, leftPos, rightPos, 0,0);
        }
      }
    }
    pCurrentRoute->DeleteIfAllowed();
  }
  delete pRoutesIt;
  for (int i = 1 ; i < pLine->Size(); i++) {
    pLine->Get(i, hs);
    GenericRelationIterator* pRoutesIt = pRoutes->MakeScan();
    found = false;
    while( (pCurrentRoute = pRoutesIt->GetNextTuple()) != 0 && !found) {
      iRouteTid = -1;
      pRouteId = (CcInt*) pCurrentRoute->GetAttribute(ROUTE_ID);
      iRouteId = pRouteId->GetIntval();
      pRouteCurve = (SimpleLine*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
      bLeftFound = checkPoint(pRouteCurve, hs->GetLeftPoint(), true, leftPos);
      if (bLeftFound) {
        bRightFound =
            checkPoint(pRouteCurve, hs->GetRightPoint(), true, rightPos);
        if (bRightFound) {
          found = true;
          if (leftPos > rightPos) {
            tree->insert(iRouteId, rightPos, leftPos);
          } else {
            tree->insert(iRouteId, leftPos, rightPos);
          }
        }
      }
      pCurrentRoute->DeleteIfAllowed();
    }
    delete pRoutesIt;
  } // end for pLine-Components
  GLine *resG = new GLine(0);
  tree->treeToGLine(resG);
  result =  SetWord(resG);
  resG->SetDefined(true);
  resG->SetNetworkId(pNetwork->GetId());
  resG->SetSorted(true);
  return 0;
} //end ValueMapping

const string OpLine2GLineSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>network x sline -> gline" "</text--->"
  "<text>sline2gline(_,_)</text--->"
  "<text>Translates a sline to a gline value.</text--->"
  "<text>sline2gline(B_NETWORK, sline)</text--->"
  ") )";

Operator sline2gline (
          "sline2gline",               // name
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
  if (!nl->IsAtom(first) || nl->AtomType(first) != SymbolType ||
       nl->SymbolValue(first) != "gpoint"){
    return (nl->SymbolAtom("typeerror"));
  }
  if (!nl->IsAtom(second) || nl->AtomType(second) != SymbolType ||
       nl->SymbolValue(second) != "gpoint"){
    return (nl->SymbolAtom("typeerror"));
  }
  return nl->SymbolAtom("bool");
}

int OpNetEqualValueMapping (Word* args, Word& result, int message,
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

const string OpNetEqualSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gpoint x gpoint -> bool" "</text--->"
  "<text> _ = _</text--->"
  "<text>Returns if two gpoints are equal.</text--->"
  "<text>query gpoint1 = gpoint2</text--->"
  ") )";

Operator netgpequal (
          "=",               // name
          OpNetEqualSpec,          // specification
          OpNetEqualValueMapping,  // value mapping
          Operator::SimpleSelect,        // selection function
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
  const RouteInterval *pRi1, *pRi2;
  if (pGLine1->GetNetworkId() != pGLine2->GetNetworkId()) {
    cerr << "glines belong to different networks." << endl;
    pResult->Set(true, false);
    return 0;
  }
  if (!pGLine1->IsSorted()) {
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
  } else {
    if (pGLine2->IsSorted()) {
      int i = 0;
      int j = 0;
      while (i<pGLine1->NoOfComponents() && j < pGLine2->NoOfComponents()) {
        pGLine1->Get(i,pRi1);
        pGLine2->Get(j,pRi2);
        if (pRi1->m_iRouteId < pRi2->m_iRouteId) i++;
        else
          if (pRi1->m_iRouteId > pRi2->m_iRouteId) j++;
          else
            if (pRi1->m_dStart > pRi2->m_dEnd) j++;
            else
              if (pRi1->m_dEnd < pRi2->m_dStart) i++;
              else {
                pResult->Set(true,true);
                return 0;
              }
      }
    } else {
      for (int i = 0; i < pGLine2->NoOfComponents(); i++){
        pGLine2->Get(i, pRi2);
        if (searchUnit(pGLine1, 0, pGLine1->NoOfComponents()-1, pRi2)){
          pResult->Set(true, true);
          return 0;
        };
      }
    }
  }
  pResult->Set(true, false);
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
  int iNetworkId = pNetwork->GetId();

  Point* pPoint = (Point*)args[1].addr;
  if(pPoint == NULL || !pPoint->IsDefined()) {
    string strMessage = "Point does not exist.";
    cerr << strMessage << endl;
    sendMessage(strMessage);
    pGPoint->SetDefined(false);
    return 0;
  }

  bool bPointFound;
  double dPos, difference;
  int iRouteTid;
  Tuple *pCurrentRoute;
  SimpleLine *pRouteCurve;
  Relation* pRoutes = pNetwork->GetRoutes();
  // Compute Position in Network for the ~point~
  Point uPoint = *pPoint;
  GenericRelationIterator* pRoutesIt = pRoutes->MakeScan();
  while( (pCurrentRoute = pRoutesIt->GetNextTuple()) != 0 ) {
    iRouteTid = -1;
    pRouteCurve = (SimpleLine*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
    bPointFound = chkPoint(pRouteCurve, uPoint, true, dPos, difference);
    if (bPointFound) {
      iRouteTid = pCurrentRoute->GetTupleId();
      *pGPoint = GPoint(true, iNetworkId, iRouteTid, dPos, None);
      pCurrentRoute->DeleteIfAllowed();
      break;
    }
    pCurrentRoute->DeleteIfAllowed();
  }
  delete pRoutesIt;
  if (iRouteTid == -1 ) {
    GenericRelationIterator* pRoutesIt = pRoutes->MakeScan();
    while( (pCurrentRoute = pRoutesIt->GetNextTuple()) != 0 ) {
      iRouteTid = -1;
      pRouteCurve = (SimpleLine*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
      bPointFound = chkPoint03(pRouteCurve, uPoint, true, dPos, difference);
      if (bPointFound) {
        iRouteTid = pCurrentRoute->GetTupleId();
        *pGPoint = GPoint(true, iNetworkId, iRouteTid, dPos, None);
        pCurrentRoute->DeleteIfAllowed();
        break;
      }
      pCurrentRoute->DeleteIfAllowed();
    }
    delete pRoutesIt;
  }
  if (iRouteTid == -1) {
    string strMessage = "Point not found in network.";
    cerr << strMessage << endl;
    sendMessage(strMessage);
    return 0;
  }
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
             local = SetWord(0);
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
           if(res==0){
              return CANCEL;
           } else {
              result = SetWord(new Rectangle<2>(*res));
              return YIELD;
           }
      case CLOSE:
           if (local.addr != 0) {
             delete (RectangleList*) local.addr;
             local = SetWord(0);
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
  // Get values
  GLine* pGLine = (GLine*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pGLine );

  GPoint* pFromGPoint = (GPoint*)args[0].addr;
  GPoint* pToGPoint = (GPoint*)args[1].addr;

  if (pFromGPoint == 0 || pToGPoint == 0 || !pFromGPoint->IsDefined() ||
      !pToGPoint->IsDefined()) {
     sendMessage("Both gpoints must exist and be defined.");
     return 0;
  }
  // Check wether both points belong to the same network
  if(pFromGPoint->GetNetworkId() != pToGPoint->GetNetworkId())
  {
    sendMessage("Both gpoints belong to different networks.");
    return 0;
  }

  // Load the network
  Network* pNetwork = NetworkManager::GetNetwork(pFromGPoint->GetNetworkId());
  if(pNetwork == 0)
  {
    sendMessage("Network with id does not exist.");
    return 0;
  }
  // Get sections where the path should start or end
  Tuple* pFromSection = pNetwork->GetSectionOnRoute(pFromGPoint);
  if (pFromSection == 0) {
    sendMessage("Starting GPoint not found in network.");
    pFromSection->DeleteIfAllowed();
    NetworkManager::CloseNetwork(pNetwork);
    return 0;
  }
  Tuple* pToSection = pNetwork->GetSectionOnRoute(pToGPoint);
  if (pToSection == 0) {
    sendMessage("End GPoint not found in network.");
    pFromSection->DeleteIfAllowed();
    pToSection->DeleteIfAllowed();
    NetworkManager::CloseNetwork(pNetwork);
    return 0;
  }
  Point* pToPoint = pNetwork->GetPointOnRoute(pToGPoint);

  pGLine->SetNetworkId(pNetwork->GetId());

  // Calculate the shortest path
  Dijkstra(pNetwork,
           pFromSection->GetTupleId(),
           pFromGPoint,
           pToSection->GetTupleId(),
           pToGPoint,
           pToPoint,
           pGLine);

  // Cleanup and return
  delete pToPoint;
  pFromSection->DeleteIfAllowed();
  pToSection->DeleteIfAllowed();
  NetworkManager::CloseNetwork(pNetwork);
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
1.4.17 Operator ~gline2sline~

Returns the ~sline~ value of the given ~GLine~.

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
  return nl->SymbolAtom( "sline" );
}

int OpGLine2LineValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  SimpleLine* pLine = (SimpleLine*) qp->ResultStorage(in_xSupplier).addr;
  result = SetWord(pLine);
  pLine->SetDefined(true);
  GLine* pGLine = (GLine*)args[0].addr;
  if (pGLine == NULL || !pGLine->IsDefined()) {
    sendMessage("GLine must be defined!");
    pLine->SetDefined(false);
    return 0;
  }
  Network* pNetwork = NetworkManager::GetNetwork(pGLine->GetNetworkId());
  const RouteInterval *rI;
  const HalfSegment *hs;
  SimpleLine *resLine = new SimpleLine(1);
  resLine->StartBulkLoad();
  for (int i=0; i < pGLine->NoOfComponents(); i++) {
    pGLine->Get(i,rI);
    SimpleLine pSubline = *new SimpleLine(1);
    pNetwork->GetLineValueOfRouteInterval(rI, pSubline);
    for (int j = 0; j < pSubline.Size(); j++) {
      pSubline.Get(j,hs);
      resLine->operator+=(*hs);
    }
  }
  resLine->EndBulkLoad();
  result = SetWord(resLine);
  resLine->SetDefined(true);
  NetworkManager::CloseNetwork(pNetwork);
  return 0;
}

const string OpGLine2LineSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gline -> sline" "</text--->"
  "<text>gline2sline(GLINE)</text--->"
  "<text>Returns the line value of the gline.</text--->"
  "<text> gline2sline(gline) </text--->"
  ") )";

Operator networkgline2sline (
          "gline2sline",               // name
          OpGLine2LineSpec,          // specification
          OpGLine2LineValueMapping,  // value mapping
          Operator::SimpleSelect,        // selection function
          OpGLine2LineTypeMap        // type mapping
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

    gpoint.AssociateKind( "DATA" );
    gline.AssociateKind( "DATA" );
    network.AssociateKind( "DATA" );

    AddOperator(&networkthenetwork);
    AddOperator(&networkroutes);
    AddOperator(&networkjunctions);
    AddOperator(&networksections);
    AddOperator(&shortest_path);
    AddOperator(&networklength);
    AddOperator(&networkdistance);
    AddOperator(&point2gpoint);
    AddOperator(&netgpequal);
    AddOperator(&sline2gline);
    AddOperator(&networkinside);
    AddOperator(&networknocomponents);
    AddOperator(&polygpoints);
    AddOperator(&networkrouteintervals);
    AddOperator(&networkintersects);
    AddOperator(&networkgpoint2rect);
    AddOperator(&networkgline2sline);
  }
  ~NetworkAlgebra() {};
};

NetworkAlgebra networkAlgebra;

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

  return (&networkAlgebra);
}


