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

The operator translates points into network based gpoints

February 2008 Simone Jandt

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

#include "OpPoint2GPoint.h"

/*
Checks linevalue for the point and computes the position from the point on
the line.

*/

bool chkPoint (Line *route, Point point, bool startSmaller, double &pos,
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

bool chkPoint03 (Line *route, Point point, bool startSmaller, double &pos,
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


/*
Typemap function of the operator

*/
ListExpr OpPoint2GPoint::TypeMap(ListExpr in_xArgs)
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

/*
Value mapping function of operator ~point2gpoint~

*/
int OpPoint2GPoint::ValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
/*
Initialize return value

*/
  GPoint* pGPoint = (GPoint*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pGPoint );
/*
Get and check input values.

*/
//   CcInt* pNetworkId = (CcInt*)args[0].addr;
//   int iNetworkId = pNetworkId->GetIntval();
//   Network* pNetwork = NetworkManager::GetNetwork(iNetworkId);
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
  Line *pRouteCurve;
  Relation* pRoutes = pNetwork->GetRoutes();


/*
Compute Position in Network for the ~point~

*/

  Point uPoint = *pPoint;
  GenericRelationIterator* pRoutesIt = pRoutes->MakeScan();
  while( (pCurrentRoute = pRoutesIt->GetNextTuple()) != 0 ) {
    iRouteTid = -1;
    pRouteCurve = (Line*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
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
      pRouteCurve = (Line*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
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
 //   delete pRoutes;
//    NetworkManager::CloseNetwork(pNetwork);
    return 0;
  }
 // pRoutes->Delete();
//   NetworkManager::CloseNetwork(pNetwork);
  return 0;
} //end ValueMapping


/*
Specification of operator ~mpoint2mgpoint~

*/
const string OpPoint2GPoint::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>point -> gpoint" "</text--->"
  "<text>point2gpoint(networkobject, point)</text--->"
  "<text>Translates a point to a gpoint.</text--->"
  "<text>point2gpoint(B_NETWORK, point)</text--->"
  ") )";

/*
Sending a message via the message-center

*/
void OpPoint2GPoint::sendMessage(string in_strMessage)
{
  // Get message-center and initialize message-list
  static MessageCenter* xMessageCenter= MessageCenter::GetInstance();
  NList xMessage;
  xMessage.append(NList("error"));
  xMessage.append(NList().textAtom(in_strMessage));
  xMessageCenter->Send(xMessage);
}
