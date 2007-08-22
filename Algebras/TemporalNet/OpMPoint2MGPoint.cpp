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
#include "UGPoint.h"
#include "MGPoint.h"
#include "TemporalNetAlgebra.h"
#include "Network.h"
#include "NetworkManager.h"

#include "OpMPoint2MGPoint.h"

/*
4.1.2 Typemap function of the operator

*/
ListExpr OpMPoint2MGPoint::TypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 2 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xNetworkIdDesc = nl->First(in_xArgs);
  ListExpr xMPointDesc = nl->Second(in_xArgs);

  if( (!nl->IsAtom(xNetworkIdDesc)) ||
      !nl->IsEqual(xNetworkIdDesc, "int"))
  {
    return (nl->SymbolAtom("typeerror"));
  }

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

The method will first look for a segment at the start of the mpoint.

Preconditions: 
- The moving point has to be exactly over the routes of the network.
- The trajectory of the mpoint must not be disjoint i.e. each unit starts 
  where the one before ended.
- At each crossing of streets a new unit has to be started - even if the
  mpoint moves straigt ahead. 
- Their may not exist parralel sections (with junctions at the start position)

*/
int OpMPoint2MGPoint::ValueMapping(Word* args, 
                                   Word& result, 
                                   int message,  
                                   Word& local, 
                                   Supplier in_xSupplier)
{
  // Get (empty) return value
  MGPoint* pMGPoint = (MGPoint*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pMGPoint ); 

  // New units are added to the moving point unsorted in a bulk-load.
  // The bulk load has to be ended before the point can be used.
  pMGPoint->StartBulkLoad();

  // Get input values
  CcInt* pNetworkId = (CcInt*)args[0].addr;
  int iNetworkId = pNetworkId->GetIntval();
  MPoint* pMPoint = (MPoint*)args[1].addr;

  // Load the network
  Network* pNetwork = NetworkManager::GetNetwork(iNetworkId);
  
  Relation* pSections = pNetwork->GetSectionsInternal();   
  GenericRelationIterator* pSectionsIt = pSections->MakeScan();
  
  // This section will be valid as long as the moving point 
  // moves somewhere over it. If not it is to be replaced
  // by an adjacent section 
  Tuple* pCurrentSection;
  // The direction the point is moving
  bool bCurrentMovingUp;
  // Line of the current section. 
  Line* pCurrentSectionCurve;
  
  if(pMPoint->GetNoComponents() == 0)
  {
    pMGPoint->EndBulkLoad();
    return 0;
  }
  

  /////////////////////
  // 
  // Find first section
  //
  const UPoint *pFirstUnit;
  pMPoint->Get(0, pFirstUnit);  
  Point xStart = pFirstUnit->p0;
  while( (pCurrentSection = pSectionsIt->GetNextTuple()) != 0 )
  {
    // TODO: Maybe support this search with a BTree
    pCurrentSectionCurve = (Line*)pCurrentSection->GetAttribute(SECTION_CURVE);

    Point xSectionStart = pCurrentSectionCurve->StartPoint(true);
    if(xSectionStart.IsDefined() &&
       xStart == xSectionStart)
    {
      bCurrentMovingUp = true;
      break;
    }
    
    Point xSectionEnd = pCurrentSectionCurve->EndPoint(true);
    if(xSectionEnd.IsDefined() && 
       xStart == xSectionEnd)
    {
      // TODO: Their might be anoter
      bCurrentMovingUp = false;
      break;
    }
    
    pCurrentSection->DeleteIfAllowed(); 
  }
  // TODO: Fehlerbehandlung wenn kein Segment gefunden wurde
  delete pSectionsIt;


  /////////////////////
  // 
  // Process all units off the MPoint
  // finding on section after another
  //
  for (int i = 0; i < pMPoint->GetNoComponents(); i++) 
  {
    // Get start and end of current unit
    const UPoint *pCurrentUnit;
    pMPoint->Get(i, pCurrentUnit);
    Point xStart = pCurrentUnit->p0;
    Point xEnd = pCurrentUnit->p1;
    cout << "Unit " << i << " "  
         << "Start: " << xStart << " "
         << "End: " << xEnd << " " 
         << endl;
    
    // TODO: Check if start = last_end
    
    // We know, that the start is on the current section.
    // Check wether the end is on it to.
    double dEndPositionOnLine;
    bool bEndPointOnLine = pCurrentSectionCurve->AtPoint(xEnd, 
                                                         bCurrentMovingUp, 
                                                         dEndPositionOnLine);
    
    /////////////////////
    // 
    // Find alternative next segment
    //
    if(!bEndPointOnLine)
    {
      cout << "Off-Line." << endl;
      
      // Find new section:
      vector<DirectedSection> xAdjacentSections;
      xAdjacentSections.clear();
      pNetwork->GetAdjacentSections(pCurrentSection->GetTupleId(),
                                    bCurrentMovingUp,
                                    xAdjacentSections);
                                
      // Iterate over adjacent sections                                     
      for(size_t i = 0;  i < xAdjacentSections.size(); i++) 
      {
        // Load the structure belonging to the adjacent section
        DirectedSection xAdjacentSection = xAdjacentSections[i];
        int iNextSectionTid = xAdjacentSection.getSectionTid();
        bool bNextUpDownFlag = xAdjacentSection.getUpDownFlag();

        // Load section and curve     
        Tuple* pNextSection = pSections->GetTuple(iNextSectionTid);
        Line* pNextSectionCurve;
        pNextSectionCurve = (Line*)pNextSection->GetAttribute(SECTION_CURVE);
        
        // Check wether the end is on this section (if so dPositionOnLine
        // will reflect the position on the segment)
        bool bPointOnNextLine;
        double dNextEndPositionOnLine;
        bPointOnNextLine = pNextSectionCurve->AtPoint(xEnd, 
                                                      bNextUpDownFlag, 
                                                      dNextEndPositionOnLine);
        if(bPointOnNextLine)
        {
          // New section found
          
          // Delete the last
          pCurrentSection->DeleteIfAllowed();
          
          // Take the new one
          pCurrentSection = pNextSection;
          pCurrentSectionCurve = pNextSectionCurve;
          bCurrentMovingUp = bNextUpDownFlag;
          dEndPositionOnLine = dNextEndPositionOnLine;
        }
      }
    }

    // TODO: Wat happens if no section was found?
      
    /////////////////////
    // 
    // Calculate points on route
    //
      
    // We allready proved the end-point is on the route. The 
    // start-point of the unit has to be on the same segment 
    // because we either have been following the segment for 
    // some units or just changed at a crossing. In the later
    // case the start is at the crossing.
    // Calculate position of Start-Point on segment.
    double dStartPositionOnLine;
    bool bStartPointOnLine; 
    bStartPointOnLine = pCurrentSectionCurve->AtPoint(xStart, 
                                                      bCurrentMovingUp, 
                                                      dStartPositionOnLine);
    // TODO: FEhlerbehandlung, wenn der Startpunkt nicht auf der Linie lag.
    CcInt* xRouteId;
    xRouteId = (CcInt*)pCurrentSection->GetAttribute(SECTION_RID);
    int iRouteId = xRouteId->GetIntval();
    CcReal* xMeas1; 
    xMeas1 = (CcReal*)pCurrentSection->GetAttribute(SECTION_MEAS1); 
    float fMeas1 = xMeas1->GetRealval();
    CcReal* xMeas2; 
    xMeas2 = (CcReal*)pCurrentSection->GetAttribute(SECTION_MEAS2); 
    float fMeas2 = xMeas2->GetRealval();      
    double dFrom;
    double dTo;
    if(bCurrentMovingUp)
    {
      dFrom = fMeas1 + dStartPositionOnLine;
      dTo = fMeas1 + dEndPositionOnLine;
    }
    else
    {
      dFrom = fMeas2 - dStartPositionOnLine;
      dTo = fMeas2 - dEndPositionOnLine;
    }

    /////////////////////
    // 
    // Create new unit for mgpoint
    //
    cout << "New Unit \n" 
         << " Section: " << pCurrentSection->GetTupleId() << endl
         << " Route: " << iRouteId << endl
         << " From: " << dFrom << endl 
         << " To: " << dTo
         << endl;
    pMGPoint->Add(UGPoint(pCurrentUnit->timeInterval,
                          iNetworkId,
                          iRouteId,
                          bCurrentMovingUp ? Up : Down,
                          dFrom,
                          dTo));         
  }


  // Units were added to the moving point. They are sorted and 
  // the bulk-load is ended:
  pMGPoint->EndBulkLoad();
  return 0;
}


/*
4.1.3 Specification of the operator

*/
const string OpMPoint2MGPoint::Spec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>mpoint -> mgpoint" "</text--->"
  "<text>mpoint2mgpoint(networkid, mpoint)</text--->"
  "<text>Finds a path in a network for a moving point.</text--->"
  "<text>mpoint2mgpoint(1, x)</text--->"
  ") )";
