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

[1] Implementation of GLine in Module Network Algebra

March 2004 Victor Almeida

Mai-Oktober 2007 Martin Scheppokat

[TOC]

1 Overview


This file contains the implementation of ~gline~


2 Defines, includes, and constants

*/
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "DBArray.h"
#include "TupleIdentifier.h"

#include "StandardTypes.h"
#include "Network.h"
#include "GLine.h"
#include "GPoint.h"

#include "OpShortestPath.h"


/*
4.1.2 Typemap function of the operator

*/
ListExpr OpShortestPath::TypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 2 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xGPoint1Desc = nl->First(in_xArgs);
  ListExpr xGPoint2Desc = nl->Second(in_xArgs);

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
  
  return nl->SymbolAtom( "gline" );
}

/*
4.1.2 Value mapping function of the operator

*/
int OpShortestPath::ValueMapping( Word* args, 
                                  Word& result, 
                                  int message,  
                                  Word& local, 
                                  Supplier in_xSupplier )
{
  // Get values
  GLine* pGLine = (GLine*)qp->ResultStorage(in_xSupplier).addr;

  GPoint* pGPoint1 = (GPoint*)args[0].addr;
  GPoint* pGPoint2 = (GPoint*)args[1].addr;
   
  pGLine->SetNetworkId(1);
  pGLine->AddRouteInterval(1, 1.5, 2.5);

  // Set result
  result = SetWord( pGLine ); 
  return 0;
}

/*
4.1.3 Specification of the operator

*/
const string OpShortestPath::Spec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gpoint x gpoint -> gline" "</text--->"
  "<text>shortest_path(_, _)</text--->"
  "<text>Calculates the shortest path between two gpoints.</text--->"
  "<text>let n = shortestpath(x, y)</text--->"
  ") )";
