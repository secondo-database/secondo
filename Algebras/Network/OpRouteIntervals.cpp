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

1.1 Implementation of Operator routeintervals

The operator returns a stream rectangles representing the routeintervals of the
given gline.

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

#include "OpRouteIntervals.h"

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

/*
~Destroys~ the RectangleList

*/

   ~RectangleList(){}

/*
~NextRectangle~

This function returns the next rectangle from the RectangleList.
If there is no more route interval in the  List the result will be
0. This function creates a new GPoint instance via the new
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
Typemap function of the operator routeintervals

*/
ListExpr OpRouteIntervals::TypeMap(ListExpr in_xArgs)
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

/*
Value mapping function of operator ~routeintervals~

*/
int OpRouteIntervals::ValueMapping(Word* args,
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


/*
Specification of operator ~routeintervals~

*/
const string OpRouteIntervals::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gline -> stream(rect)" "</text--->"
  "<text>routeintervals(GLINE)</text--->"
  "<text>Returns a stream of rectangles representing the route intervals of the"
    " gline.</text--->"
  "<text> routeintervals (gline) </text--->"
  ") )";

/*
Sending a message via the message-center

*/
void OpRouteIntervals::sendMessage(string in_strMessage)
{
  // Get message-center and initialize message-list
  static MessageCenter* xMessageCenter= MessageCenter::GetInstance();
  NList xMessage;
  xMessage.append(NList("error"));
  xMessage.append(NList().textAtom(in_strMessage));
  xMessageCenter->Send(xMessage);
}
