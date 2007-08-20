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

#include "TemporalAlgebra.h"
#include "NetworkAlgebra.h"
#include "GPoint.h"
#include "TemporalNetAlgebra.h"
#include "Network.h"

#include "OpMPoint2MGPoint.h"

/*
4.1.2 Typemap function of the operator

*/
ListExpr OpMPoint2MGPoint::TypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMPointDesc )) ||
      nl->AtomType( xMPointDesc ) != SymbolType ||
      nl->SymbolValue( xMPointDesc ) != "mpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }
  
  return nl->SymbolAtom( "mgpoint" );
}

/*
4.1.2 Value mapping function of the operator

*/
int OpMPoint2MGPoint::ValueMapping( Word* args, 
                                  Word& result, 
                                  int message,  
                                  Word& local, 
                                  Supplier in_xSupplier )
{
  // Get values
  MGPoint* pMGPoint = (MGPoint*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pMGPoint ); 

  MPoint* pMPoint = (MPoint*)args[0].addr;

  bool bIsObject = SecondoSystem::GetCatalog()->IsObjectName("X_NETWORK_M2MG");

  if(!bIsObject)
  {
    // Fehlerbehandlung
    cerr << "X_NETWORK is not a secondo-object" << endl;
    return 0;
  }

  bool bDefined;
  Word xValue;
 
  bool bOk = SecondoSystem::GetCatalog()->GetObject("X_NETWORK_M2MG",
                                                    xValue,
                                                    bDefined);
                                         

  if(!bDefined || !bOk)
  {
    // Fehlerbehandlung
    cerr << "Could not load object." << endl;
    return 0;
  }           
  
  Network* pNetwork = (Network*)xValue.addr;
  
  Relation* pSections = pNetwork->GetSectionsInternal();   
  GenericRelationIterator* pSectionsIt = pSections->MakeScan();
  Tuple* pSection;

  const UPoint *pFirstUnit;
  pMPoint->Get(0, pFirstUnit);
  
  Point xStart = pFirstUnit->p0;

 
  // Find first section
  // TODO: Support this search with a BTree
  while( (pSection = pSectionsIt->GetNextTuple()) != 0 )
  {
    int iSegmentId = pSectionsIt->GetTupleId();
//    CcInt* xRouteId = (CcInt*)pSection->GetAttribute(SECTION_RID); 
//    int iRouteId = xRouteId->GetIntval();
//    CcReal* xMeas1 = (CcReal*)pSection->GetAttribute(SECTION_MEAS1); 
//    float fMeas1 = xMeas1->GetRealval();
//    CcReal* xMeas2 = (CcReal*)pSection->GetAttribute(SECTION_MEAS2); 
//    float fMeas2 = xMeas2->GetRealval();
    Line *pCurve = (Line*)pSection->GetAttribute( SECTION_CURVE );

    Point xSectionStart = pCurve->StartPoint(true);
    cout << "Start: (" 
         << xSectionStart.GetX() << ", " 
         << xSectionStart.GetY() << ")" 
         << endl;
    if(xStart == xSectionStart)
    {
      cout << iSegmentId << endl;
    }
    
    Point xSectionEnd = pCurve->EndPoint(true);
    cout << "End: (" 
         << xSectionEnd.GetX() << ", " 
         << xSectionEnd.GetY() << ")" 
         <<endl;
    if(xStart == xSectionEnd)
    {
      cout << iSegmentId << endl;
    }
    
    pSection->DeleteIfAllowed(); 
  }
  delete pSectionsIt;


  for (int i = 0; i < pMPoint->GetNoComponents(); i++) 
  {
    const UPoint *pCurrentUnit;

    pMPoint->Get(i, pCurrentUnit);
  }

  return 0;
}



/*
4.1.3 Specification of the operator

*/
const string OpMPoint2MGPoint::Spec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>mpoint -> mgpoint" "</text--->"
  "<text>mpoint2mgpoint(mpoint)</text--->"
  "<text>Finds a path in a network for a moving point.</text--->"
  "<text>mpoint2mgpoint(x)</text--->"
  ") )";
