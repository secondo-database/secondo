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

1.1 Implementation of Operator MPoint2MGPoint

The operator translates moving points into network based moving points

Mai-Oktober 2007 Martin Scheppokat

January 2008 Simone Jandt (Value Mapping changed. Units of mgpoint are joined
if the speed difference between two units is smaller than 0.0000000001.)

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
#include "UGPoint.h"
#include "MGPoint.h"
#include "TemporalNetAlgebra.h"
#include "Network.h"
#include "NetworkManager.h"

#include "OpMPoint2MGPoint.h"

/*
Sets parameter movingUp and side for the given Unit

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
getUnitValues gets the parameter Values of the given Unit

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
Checks linevalue for the point and computes the position from the point on
the line.

*/

  bool checkPoint (Line *route, Point point, bool startSmaller, double &pos,
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
      if (!(route->IsSimple())) {
        return false;
      } else {
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
  }
  return result;
}

bool checkPoint03 (Line *route, Point point, bool startSmaller, double &pos,
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
      if (!(route->IsSimple())) {
        return false;
      } else {
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
  }
  return result;
}


ListExpr OpMPoint2MGPoint::TypeMap(ListExpr in_xArgs)
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
    sendMessage("Expected network as first argument.");
    return (nl->SymbolAtom("typeerror"));
  }

  if( (!nl->IsAtom( xMPointDesc )) ||
      nl->AtomType( xMPointDesc ) != SymbolType ||
      nl->SymbolValue( xMPointDesc ) != "mpoint" )
  {
    sendMessage("Expected mpoint as second argument.");
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
- At each crossing of streets a new unit has to be started
- Their may not exist parallel sections (with junctions at the start position)
- A change of the direction on a route is only allowed at crossings (and only
  if the opposite lane can be reached at the crossing e.g. a u-turn is allowed).

*/
int OpMPoint2MGPoint::ValueMapping(Word* args,
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
  pMGPoint->Clear();
  pMGPoint->SetDefined(true);
/*
Get and check input values.

*/
//   CcInt* pNetworkId = (CcInt*)args[0].addr;
//   int iNetworkId = pNetworkId->GetIntval();
//   Network* pNetwork = NetworkManager::GetNetwork(iNetworkId);
  Network *pNetwork = (Network*)args[0].addr;
  if (pNetwork->isDefined() < 1) {
    string strMessage = "Network is not defined.";
    cerr << strMessage << endl;
    sendMessage(strMessage);
    pMGPoint->SetDefined(false);
    return 0;
  }
  int iNetworkId = pNetwork->GetId();
  MPoint* pMPoint = (MPoint*)args[1].addr;
  if(pMPoint == NULL || pMPoint->GetNoComponents() < 1 ||
     !pMPoint->IsDefined()) {
    string strMessage = "MPoint does not exist.";
    cerr << strMessage << endl;
    sendMessage(strMessage);
    pMGPoint->SetDefined(false);
    return 0;
  }
/*
Some initialisations

*/
  const UPoint *pCurrentUnit;
  Point uStartPoint, uEndPoint, juncPoint, uFirstEndPoint;
  bool bStartFound, bEndFound, bMovingUp, bNewRouteFound, bDual;
  bool bMGPointCurrMovingUp, bSecondEndCheck, bSecCheckDual,bAdjSecCheck;
  bool bAdjAddCheck, bAdjSecCheckDual;
  CcBool *pDual;
  double dStartPosOld, difference, firstDifference, aktDifference, dFirstEndPos;
  double dStartPos, dEndPos, speedDifference, dAktUnitSpeed, dAktUnitDistance;
  double dMGPointCurrDistance, dMGPointCurrEndPos, dMGPointCurrStartPos;
  double dMGPointCurrSpeed, rid1meas, rid2meas, dSecondCheckEndPos, dSecDiff;
  double dAdjSecCheckEndPos, dFirstAdjSecCheckDiff, dAdjAddCheckEndPos;
  double dAdjSecCheckDiff;
  int iRouteTid, iRouteId, iOldSectionTid, iCurrentSectionTid, iLastRouteId;
  int iCurrentSectionRid, iCurrentSectionRTid, iMGPointCurrRId;
  int iSecCheckRouteId, iAdjSecCheckRid;
  CcInt *pCurrentSectionRid, *pCurrentSectionRTid, *pRouteId;
  Tuple *pCurrentRoute, *pOldSectionTuple, *pCurrentSectionT, *pTestRoute;
  Line *pRouteCurve, *pLastRouteCurve, *pTestRouteCurve;
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


/*
Translate every Unit of MPoint to MGPoint Units.

Compute Position in Network for the (start) of the first Unit of MGPoint

*/

  pMGPoint->StartBulkLoad();
  pMPoint->Get(0, pCurrentUnit);
  getUnitValues(pCurrentUnit, uEndPoint, uStartPoint, aktUnitStartTime,
                  aktUnitEndTime, lAktUnitStartTime, lAktUnitEndTime,
                  lAktUnitDuration);
  GenericRelationIterator* pRoutesIt = pRoutes->MakeScan();
  while( (pCurrentRoute = pRoutesIt->GetNextTuple()) != 0 ) {
    iRouteTid = -1;
    pRouteId = (CcInt*) pCurrentRoute->GetAttribute(ROUTE_ID);
    iRouteId = pRouteId->GetIntval();
    pRouteCurve = (Line*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
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
        break;
      }
    }
    pCurrentRoute->DeleteIfAllowed();
  }

  delete pRoutesIt;
  if (iRouteTid == -1 ) {
    string strMessage = "First Unit of mpoint not found in network.";
    cerr << strMessage << endl;
    sendMessage(strMessage);
    pMGPoint->EndBulkLoad();
    //delete pRoutes;
//     NetworkManager::CloseNetwork(pNetwork);
    return 0;
  }
  pCurrentRoute = pRoutes->GetTuple(iRouteTid);
  pTestRoute = pRoutes->GetTuple(iRouteTid);
  pRouteCurve = (Line*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
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
    bAdjSecCheck = false;
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
      }
    }
    if (bEndFound) {
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
            pMGPoint->Add(UGPoint(Interval<Instant>(tMGPointCurrStartTime,
                                               tMGPointCurrEndTime,
                                               true,
                                               false),
                             iNetworkId,
                             iMGPointCurrRId,
                             sMGPointCurrSide,
                             dMGPointCurrStartPos,
                             dMGPointCurrEndPos));
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

        pMGPoint->Add(UGPoint(Interval<Instant>(tMGPointCurrStartTime,
                                                tMGPointCurrEndTime,
                                                true,
                                                false),
                                iNetworkId,
                                iMGPointCurrRId,
                                sMGPointCurrSide,
                                dMGPointCurrStartPos,
                                dMGPointCurrEndPos));
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
            string strMessage = "No last section for adjacent sections found.";
            cerr << strMessage << endl;
            sendMessage(strMessage);
            pMGPoint->EndBulkLoad();
            pCurrentRoute->DeleteIfAllowed();
            pTestRoute->DeleteIfAllowed();
            //delete pRoutes;
//             NetworkManager::CloseNetwork(pNetwork);
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
            string strMessage = "No section on cylce found";
            cerr << strMessage << endl;
            sendMessage(strMessage);
            pMGPoint->EndBulkLoad();
            pCurrentRoute->DeleteIfAllowed();
            pTestRoute->DeleteIfAllowed();
            //delete pRoutes;
//             NetworkManager::CloseNetwork(pNetwork);
            return 0;
          }
          iOldSectionTid = pOldSectionTuple->GetTupleId();
          pOldSectionTuple->DeleteIfAllowed();
          pNetwork->GetAdjacentSections(iOldSectionTid, false,
                                        pAdjacentSections);
        }
        for (size_t i = 0; i < pAdjacentSections.size(); i++) {
          DirectedSection pCurrentDirectedSection = pAdjacentSections[i];
          iCurrentSectionTid = pCurrentDirectedSection.getSectionTid();
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
            pRouteCurve = (Line*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
            bEndFound = checkPoint(pRouteCurve, uEndPoint, true, dEndPos,
                                  aktDifference);
            if (!bEndFound) {
              if (!bAdjSecCheck) {
                bAdjSecCheck = checkPoint03(pRouteCurve, uEndPoint, true,
                                   dAdjSecCheckEndPos, dFirstAdjSecCheckDiff);
                if (bAdjSecCheck) {
                  bAdjSecCheckDual = bDual;
                  iAdjSecCheckRid = pCurrentRoute->GetTupleId();
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
                      (Line*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
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
                        pCurrentDirectedSection.getSectionTid();
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
                          (Line*) pTestRoute->GetAttribute(ROUTE_CURVE);
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
                               (Line*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
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
                if (tPassJunction == aktUnitStartTime) {
                    tPassJunction = tPassJunction + correcture;
                }
                pMGPoint->Add(UGPoint(Interval<Instant>(aktUnitStartTime,
                                                tPassJunction,
                                                true,
                                                false),
                                iNetworkId,
                                iLastRouteId,
                                sMGPointCurrSide,
                                dMGPointCurrEndPos,
                                rid1meas));
                bNewRouteFound = true;
                pDual = (CcBool*) pCurrentRoute->GetAttribute(ROUTE_DUAL);
                bDual = pDual->GetBoolval();
                setMoveAndSide(rid2meas, dEndPos, bMovingUp, bDual, side);
                iRouteId = iCurrentSectionRid;
                dAktUnitDistance = fabs (dEndPos - rid2meas);
                if (tPassJunction == aktUnitEndTime) {
                    aktUnitEndTime = aktUnitEndTime + correcture;
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
              }//end ifelse bStartFound
            } // end if bEndFound
          } //end if CurrentRid != RouteId
        } //end for AdjacentSections
      } //end ifelse bEndFound
      if (!bNewRouteFound && bSecondEndCheck) {
          pCurrentRoute->DeleteIfAllowed();
          pCurrentRoute = pRoutes->GetTuple(iSecCheckRouteId);
          pRouteCurve = (Line*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
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
          pRouteCurve = (Line*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
          pRouteId = (CcInt*) pCurrentRoute->GetAttribute(ROUTE_ID);
          iRouteId = pRouteId->GetIntval();
          pDual = (CcBool*) pCurrentRoute->GetAttribute(ROUTE_DUAL);
          bDual = pDual->GetBoolval();
          bNewRouteFound = true;
          dEndPos = dAdjSecCheckEndPos;
          bStartFound = checkPoint(pRouteCurve, uStartPoint,
                                                true, dStartPos, difference);
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
          iCurrentSectionTid = pCurrentDirectedSection.getSectionTid();
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
            pRouteCurve = (Line*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
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
              pMGPoint->Add(UGPoint(Interval<Instant>(aktUnitStartTime,
                                                tPassJunction,
                                                true,
                                                false),
                                iNetworkId,
                                iLastRouteId,
                                sMGPointCurrSide,
                                dMGPointCurrEndPos,
                                rid1meas));
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
      if (!bNewRouteFound) {
          //should not happen
          cout << "MPoint is not longer found on network." << endl;
          string strMessage = "MPoint is not longer found on network.";
          cerr << strMessage << endl;
          sendMessage(strMessage);
          pMGPoint->EndBulkLoad();
          pCurrentRoute->DeleteIfAllowed();
          pTestRoute->DeleteIfAllowed();
          //delete pRoutes;

//           NetworkManager::CloseNetwork(pNetwork);
          return 0;
      }
  } //end for units MPoint
  pMGPoint->Add(UGPoint(Interval<Instant>(tMGPointCurrStartTime,
                                                tMGPointCurrEndTime,
                                                true,
                                                false),
                              iNetworkId,
                              iMGPointCurrRId,
                              sMGPointCurrSide,
                              dMGPointCurrStartPos,
                              dMGPointCurrEndPos));
  pMGPoint->EndBulkLoad();
  pCurrentRoute->DeleteIfAllowed();
  pTestRoute->DeleteIfAllowed();
  //pRoutes->Delete();
  pAdjacentSections.clear();
  // delete &pAdjacentSections;
  cout << "Translation ok" << endl;
  return 0;
} //end ValueMapping


/*
Specification of operator ~mpoint2mgpoint~

*/
const string OpMPoint2MGPoint::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>mpoint -> mgpoint" "</text--->"
  "<text>mpoint2mgpoint(Networkobject, mpoint)</text--->"
  "<text>Finds a path in a network for a moving point.</text--->"
  "<text>mpoint2mgpoint(B_NETWORK, x)</text--->"
  ") )";

/*
Sending a message via the message-center

*/
void OpMPoint2MGPoint::sendMessage(string in_strMessage)
{
  // Get message-center and initialize message-list
  static MessageCenter* xMessageCenter= MessageCenter::GetInstance();
  NList xMessage;
  xMessage.append(NList("error"));

  xMessage.append(NList().textAtom(in_strMessage));
  xMessageCenter->Send(xMessage);
}
