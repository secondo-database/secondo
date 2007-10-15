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

[1] Implementation of Operator simplify

Mai-Oktober 2007 Martin Scheppokat

1 Overview


This file contains the implementation of operator simplify


2 Defines, includes, and constants

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

#include "OpSimplify.h"

/*
4.1.2 Typemap function of the operator

*/
ListExpr OpSimplify::TypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 2 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMPointDesc = nl->First(in_xArgs);
  ListExpr xEpsilonDesc = nl->Second(in_xArgs);

  if( (!nl->IsAtom( xMPointDesc )) ||
      nl->AtomType( xMPointDesc ) != SymbolType ||
      nl->SymbolValue( xMPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }
  
  if( (!nl->IsAtom( xEpsilonDesc)) ||
      nl->AtomType( xEpsilonDesc ) != SymbolType ||
      nl->SymbolValue( xEpsilonDesc ) != "real" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }
  
  return nl->SymbolAtom( "mgpoint" );
}

/*
4.1.2 Value mapping function of the operator

  
*/
int OpSimplify::ValueMapping(Word* args, 
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
    string strMessage = "MPoint is Empty.";   
    cout << endl << strMessage << endl << endl;
    return 0;
  }

  pMGPointSimplified->Clear();
  pMGPointSimplified->StartBulkLoad();
  
  int iNetworkId;
  int iStartRouteId = 0;
  Side xStartSide;
  double dStartSpeed;

  Instant xStartStartTime;
  double dStartStartPosition;
  double dLastEndPosition;
  Instant xLastEndTime;
  
  for (int i = 0; i < pMGPoint->GetNoComponents(); i++) 
  {
    //////////////////////////////
    //
    // Get values for current unit
    //
    //////////////////////////////
    const UGPoint *pCurrentUnit;
    pMGPoint->Get(i, pCurrentUnit);

    // Duration
    Instant xCurrentStartTime = pCurrentUnit->timeInterval.start;
    LONGTYPE lCurrentStartTime = xCurrentStartTime.GetAllMilliSeconds();
    Instant xCurrentEndTime = pCurrentUnit->timeInterval.end;
    LONGTYPE lCurrentEndTime = xCurrentEndTime.GetAllMilliSeconds();
    LONGTYPE lCurrentDuration = lCurrentEndTime - lCurrentStartTime;        

    // Distance
    GPoint xCurrentStart = pCurrentUnit->p0;
    GPoint xCurrentEnd = pCurrentUnit->p1;
    iNetworkId = xCurrentStart.GetNetworkId();
    int iCurrentRouteId = xCurrentStart.GetRouteId();
    Side xCurrentSide = xCurrentStart.GetSide();
    double dCurrentStartPosition = xCurrentStart.GetPosition();
    double dCurrentEndPosition = xCurrentEnd.GetPosition();  
    double dCurrentDistance = dCurrentEndPosition - dCurrentStartPosition;

    // Speed
    double dCurrentSpeed = dCurrentDistance / lCurrentDuration;    
    cout << "Speed: " << dCurrentSpeed << endl;
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
    if( i == pMGPoint->GetNoComponents() -1)
    {
      // Last loop - create last unit
      pMGPointSimplified->Add(UGPoint(Interval<Instant>(xStartStartTime, 
                                                        xCurrentEndTime, 
                                                        true, 
                                                        false),
                              iNetworkId,
                              iStartRouteId,
                              xStartSide,
                              dStartStartPosition,
                              dCurrentEndPosition));
      
    }
    else if( iCurrentRouteId != iStartRouteId ||
             xCurrentSide != xStartSide || 
             dSpeedDifference > dEpsilon)
    {
      // Create new unit
      pMGPointSimplified->Add(UGPoint(Interval<Instant>(xStartStartTime, 
                                                        xLastEndTime, 
                                                        true, 
                                                        false),
                              iNetworkId,
                              iStartRouteId,
                              xStartSide,
                              dStartStartPosition,
                              dLastEndPosition));
      // Set new Start-Values
      iStartRouteId = iCurrentRouteId;
      xStartSide = xCurrentSide;
      dStartSpeed = dCurrentSpeed; 
      xStartStartTime = xCurrentStartTime;
      dStartStartPosition = dCurrentStartPosition;
    }
    
    // Set Last-Values for next loop
    dLastEndPosition = dCurrentEndPosition; 
    xLastEndTime = xCurrentEndTime;
  }
  

  // Units were added to the moving point. They are sorted and 
  // the bulk-load is ended:
  pMGPointSimplified->EndBulkLoad();
  return 0;
}


/*
4.1.3 Specification of the operator

*/
const string OpSimplify::Spec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint x real -> mgpoint" "</text--->"
  "<text>simplify(0.0001, mgpoint)</text--->"
  "<text>Removes unecessary units from a mgpoint.</text--->"
  "<text>simplify(0.0001, mgpoint)</text--->"
  ") )";
