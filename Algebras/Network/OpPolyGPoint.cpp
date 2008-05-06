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

1.1 Implementation of Operator PolyGPoints

The operator checks if the given gpoint is at a junction and returns the gpoint
and the corresponding gpoints which name the same junction as stream of gpoint.

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
#include "UGPoint.h"
#include "MGPoint.h"
#include "TemporalNetAlgebra.h"
#include "Network.h"
#include "NetworkManager.h"

#include "OpPolyGPoint.h"

class GPointList{
public:
/*
~Constructor~

Creates a GPointList from a given gpoint.

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
/////////////////////////////////////////////////////////////////
//       size_t low = 0;
//       size_t high = xJunctions.size()-1;
//       size_t mid, i;
//       while (low <= high && !found) {
//         mid = (low + high) /2;
//         if (mid < xJunctions.size() && mid > 0){
//           pCurrJunction = xJunctions[mid];
//           if (fabs(pCurrJunction.getRouteMeas() - gp->GetPosition())<0.01) {
//             i = mid - 1;
//             while (i>= 0 && !found) {
//               pCurrJunction = xJunctions[i];
//             if(fabs (pCurrJunction.getRouteMeas()-gp->GetPosition()) < 0.01)
//                 i--;
//               else {
//                 found = true;
//                 i++;
//                 pCurrJunction = xJunctions[i];
//               }
//             }
//             test = new GPoint(true, gp->GetNetworkId(),
//                              pCurrJunction.getOtherRouteId(),
//                              pCurrJunction.getOtherRouteMeas(),
//                              None);
//             aliasGP.Append(*test);
//             i++;
//             found = false;
//             while (!found && i < xJunctions.size()){
//               pCurrJunction = xJunctions[i];
//               if(fabs(pCurrJunction.getRouteMeas()-gp->GetPosition())<0.01){
//                 test = new GPoint(true, gp->GetNetworkId(),
//                                  pCurrJunction.getOtherRouteId(),
//                                  pCurrJunction.getOtherRouteMeas(),
//                                  None);
//                 aliasGP.Append(*test);
//                 i++;
//               } else {
//                found = true;
//               }
//             }
//           } else {
//             if ((pCurrJunction.getRouteMeas() - gp->GetPosition()) > 0.0)
//               high = mid -1;
//             else {
//               if ((pCurrJunction.getRouteMeas() - gp->GetPosition()) < 0.0)
//                 low = mid + 1;
//             }
//           }
//         }
//////////////////////////////////////////////////////////////////
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

/*
~Destroys~ the GPointList

*/

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
Typemap function of the operator polygpoints

*/
ListExpr OpPolyGPoint::TypeMap(ListExpr in_xArgs)
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

/*
Value mapping function of operator ~polygpoints~

*/
int OpPolyGPoint::ValueMapping(Word* args,
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


/*
Specification of operator ~polygpoints~

*/
const string OpPolyGPoint::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gpoint x network -> stream(gpoint)" "</text--->"
  "<text>gpoint polygpoint</text--->"
  "<text>Returns the gpoint and gpoints with the same network position"
    " if the gpoint is a junction.</text--->"
  "<text> polygpoints (gpoint, network) </text--->"
  ") )";

/*
Sending a message via the message-center

*/
void OpPolyGPoint::sendMessage(string in_strMessage)
{
  // Get message-center and initialize message-list
  static MessageCenter* xMessageCenter= MessageCenter::GetInstance();
  NList xMessage;
  xMessage.append(NList("error"));
  xMessage.append(NList().textAtom(in_strMessage));
  xMessageCenter->Send(xMessage);
}
