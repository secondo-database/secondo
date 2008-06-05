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

1.1 Implementation of Operator gpoint2rect

The operator returns a rectangle representing the given gpoint.

June 2008 Simone Jandt

Defines, includes, and constants

*/

#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "DBArray.h"
#include "TupleIdentifier.h"
#include "RectangleAlgebra.h"

#include "StandardTypes.h"

#include "TemporalAlgebra.h"
#include "NetworkAlgebra.h"
#include "GLine.h"
#include "GPoint.h"
#include "UGPoint.h"
#include "MGPoint.h"
#include "TemporalNetAlgebra.h"
#include "Network.h"
#include "NetworkManager.h"

#include "OpGPoint2Rect.h"

/*
Typemap function of the operator gpoint2rect

*/
ListExpr OpGPoint2Rect::TypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 ){
    sendMessage("Expects a list of length 1.");
    return (nl->SymbolAtom( "typeerror" ));
  }

  ListExpr xsource = nl->First(in_xArgs);

  if( (!nl->IsAtom(xsource)) ||
      !nl->IsEqual(xsource, "gpoint"))
  {
    sendMessage("First Element must be of type gline.");
    return (nl->SymbolAtom("typeerror"));
  }
  return (nl->SymbolAtom("rect"));
}

/*
Value mapping function of operator ~gpoint2rect~

*/
int OpGPoint2Rect::ValueMapping(Word* args,
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


/*
Specification of operator ~routeintervals~

*/
const string OpGPoint2Rect::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gpoint -> rect" "</text--->"
  "<text>gpoint2rect(GPOINT)</text--->"
  "<text>Returns a rectangle representing the gpoint.</text--->"
  "<text> gpoint2rect (gpoint) </text--->"
  ") )";

/*
Sending a message via the message-center

*/
void OpGPoint2Rect::sendMessage(string in_strMessage)
{
  // Get message-center and initialize message-list
  static MessageCenter* xMessageCenter= MessageCenter::GetInstance();
  NList xMessage;
  xMessage.append(NList("error"));
  xMessage.append(NList().textAtom(in_strMessage));
  xMessageCenter->Send(xMessage);
}
