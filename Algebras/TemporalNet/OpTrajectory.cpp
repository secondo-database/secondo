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

1.1 Implementation of Operator Trajectory

The operator finds the trajectory of a moving points 

Oktober 2007 Martin Scheppokat

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
#include "GLine.h"
#include "UGPoint.h"
#include "MGPoint.h"
#include "TemporalNetAlgebra.h"
#include "Network.h"
#include "NetworkManager.h"

#include "OpTrajectory.h"

/*
Typemap function of the operator

*/
ListExpr OpTrajectory::TypeMap(ListExpr in_xArgs)
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

/*
Value mapping function of operator ~trajectory~

*/
int OpTrajectory::ValueMapping(Word* args, 
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


    
  for (int i = 0; i < pMGPoint->GetNoComponents(); i++) 
  {
    // Get start and end of current unit
    const UGPoint *pCurrentUnit;
    pMGPoint->Get(i, pCurrentUnit);

    pGLine->SetNetworkId(pCurrentUnit->p0.GetNetworkId());
    pGLine->SetDefined(true);
    pGLine->AddRouteInterval(pCurrentUnit->p0.GetRouteId(), 
                             pCurrentUnit->p0.GetPosition(), 
                             pCurrentUnit->p1.GetPosition());  
  }

  return 0;
}


/*
Specification of operator ~trajectory~

*/
const string OpTrajectory::Spec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint -> gline" "</text--->"
  "<text>trajectory(mgpoint)</text--->"
  "<text>Calculates the trajectory for a moving gpoint.</text--->"
  "<text>trajectory(mgpoint)</text--->"
  ") )";

