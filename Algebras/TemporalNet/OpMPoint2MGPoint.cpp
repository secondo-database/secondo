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
- A change of the direction on a route is only allowed at crossings (and only
  if the opposite lane can be reached at the crossing e.g. a u-turn is allowed).
  
*/
int OpMPoint2MGPoint::ValueMapping(Word* args, 
                                   Word& result, 
                                   int message,  
                                   Word& local, 
                                   Supplier in_xSupplier)
{
  // Get (empty) return value
  cout << "M2MGPoint" << endl;
  MGPoint* pMGPoint = (MGPoint*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pMGPoint ); 

  // Get input values
  CcInt* pNetworkId = (CcInt*)args[0].addr;
  int iNetworkId = pNetworkId->GetIntval();
  MPoint* pMPoint = (MPoint*)args[1].addr;  
  
  // This section will be valid as long as the moving point 
  // moves somewhere over it. If not it is to be replaced
  // by an adjacent section 
  Tuple* pCurrentSection;
  int iCurrentSectionTid;
  // The direction the point is moving
  bool bCurrentMovingUp;
  // Line of the current section. 
  Line* pCurrentSectionCurve;
  
  if(pMPoint->GetNoComponents() == 0)
  {
    string strMessage = "MPoint is Empty.";   
    cout << endl << strMessage << endl << endl;
    sendMessage(strMessage);
    return 0;
  }
  

  /////////////////////
  // 
  // Find first and second point of the mpoint.
  //
  // The second point must not be equal to the first one.
  const UPoint *pFirstUnit;
  pMPoint->Get(0, pFirstUnit);  
  Point xFirstPoint = pFirstUnit->p0;
  Point xSecondPoint;
  for (int i = 0; i < pMPoint->GetNoComponents(); i++) 
  {
    // Get start and end of current unit
    const UPoint *pCurrentUnit;
    pMPoint->Get(i, pCurrentUnit);
  
    xSecondPoint = pCurrentUnit->p1;
    
    if(xFirstPoint != xSecondPoint)
    {
      break;
    }
  }
  // If start-point and end-point are still equal the mpoint does not 
  // move. Thus we can take any section the point lies on and just proceed.

  cout << "First: " << xFirstPoint << endl;
  cout << "Second: " << xSecondPoint << endl;
  
    
  // Load the network
  Network* pNetwork = NetworkManager::GetNetwork(iNetworkId);
    
    
  /////////////////////
  // 
  // Find first section
  //
  Relation* pSections = pNetwork->GetSectionsInternal();   
  GenericRelationIterator* pSectionsIt = pSections->MakeScan();
  while( (pCurrentSection = pSectionsIt->GetNextTuple()) != 0 )
  {
    // TODO: Maybe support this search with a BTree
    pCurrentSectionCurve = (Line*)pCurrentSection->GetAttribute(SECTION_CURVE);
    

    double dFirstPos;
    bool bFirstOnLine = pCurrentSectionCurve->AtPoint(xFirstPoint, 
                                                      true, 
                                                      dFirstPos);

    double dSecondPos;
    bool bSecondOnLine = pCurrentSectionCurve->AtPoint(xSecondPoint, 
                                                       true, 
                                                       dSecondPos);


    if(bFirstOnLine && bSecondOnLine)
    {
      bCurrentMovingUp = dFirstPos < dSecondPos;
      iCurrentSectionTid = pCurrentSection->GetTupleId();
      cout << "Section: (" << iCurrentSectionTid 
           << ", " << bCurrentMovingUp << ")" << endl;
      pCurrentSection->DeleteIfAllowed(); 
      break;
    }
        
    pCurrentSection->DeleteIfAllowed(); 
  }
  // TODO: Fehlerbehandlung wenn kein Segment gefunden wurde
  delete pSectionsIt;

  pSections = pNetwork->GetSectionsInternal();   
  pCurrentSection = pSections->GetTuple(iCurrentSectionTid);


  /////////////////////
  // 
  // Process all units of the MPoint
  // finding one section after another
  //
  // New units are added to the moving point unsorted in a bulk-load.
  // The bulk load has to be ended before the point can be used.
  pMGPoint->Clear();
  pMGPoint->StartBulkLoad();
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

    // The 
    // start-point of the unit has to be on the same segment 
    // because we either have been following the segment for 
    // some units or just changed at a crossing. In the later
    // case the start is at the crossing.
    // Calculate position of Start-Point on segment.
    double dStartPositionOnLine;
    bool bStartPointOnLine; 
    bStartPointOnLine = pCurrentSectionCurve->AtPoint(xStart, 
                                                      true, 
                                                      dStartPositionOnLine);
    // TODO: FEhlerbehandlung, wenn der Startpunkt nicht auf der Linie lag.
    
    // We know, that the start is on the current section.
    // Check wether the end is on it to.
    double dEndPositionOnLine;
    bool bEndPointOnLine = pCurrentSectionCurve->AtPoint(xEnd, 
                                                         true, 
                                                         dEndPositionOnLine);
    
    // Maybe the direction changed. If this is the case we have to find 
    // the adjacent section in the opposite direction
    bool bChangedDirection = (bCurrentMovingUp && 
                             dStartPositionOnLine > dEndPositionOnLine) ||
                             (!bCurrentMovingUp && 
                             dStartPositionOnLine < dEndPositionOnLine);
                             
    /////////////////////
    // 
    // Find alternative next segment
    //
    if(!bEndPointOnLine ||
       bChangedDirection)
    {
      cout << "Off-Line." << endl;
      
      // Weg auf aktuellem Segment beenden. Hierzu muss zunächst erkannt
      // werden, ob das Segment bei 0 oder bei Length endet. Danach muss
      // der Zeitpunkt bestimmt werden, zu dem der Punkt erreicht ist. Da
      // die Bewegung linear ist und die Länge bis zum Ende sowie die Länge
      // der Unit des MPoint bekannt sind sollte dies kein Problem sein.
      // Danach UNIT einfüben und neue Start-Zeit merken.
      if(bCurrentMovingUp)
      {
        // For a change to another segment the end of the last one has
        // to be reached.
        if(dStartPositionOnLine < pCurrentSectionCurve->Length())
        {
          // End not reached. Invent pseudo-segment
          CcInt* xRouteId;
          xRouteId = (CcInt*)pCurrentSection->GetAttribute(SECTION_RID);
          int iRouteId = xRouteId->GetIntval();
          CcReal* xMeas1; 
          xMeas1 = (CcReal*)pCurrentSection->GetAttribute(SECTION_MEAS1); 
          float fMeas1 = xMeas1->GetRealval();      
          CcReal* xMeas2; 
          xMeas2 = (CcReal*)pCurrentSection->GetAttribute(SECTION_MEAS2); 
          float fMeas2 = xMeas2->GetRealval();      
          cout << "New PseudoUnit " 
               << " Section: " << pCurrentSection->GetTupleId()
               << " Route: " << iRouteId
               << " From: " << fMeas1 + dStartPositionOnLine
               << " To: " << fMeas2
               << endl;
//          pMGPoint->Add(UGPoint(pCurrentUnit->timeInterval,
//                                iNetworkId,
//                                iRouteId,
//                                bCurrentMovingUp ? Up : Down,
//                                dStartPositionOnLine,
//                                fMeas2));         
          xStart = pCurrentSectionCurve->EndPoint(true);
        }
      }
      else // Moving down
      {
        // For a change to another segment the start of the last one has
        // to be reached.
        if(dStartPositionOnLine > 0)
        {
          // Start not reached. Invent pseudo-segment
          CcInt* xRouteId;
          xRouteId = (CcInt*)pCurrentSection->GetAttribute(SECTION_RID);
          int iRouteId = xRouteId->GetIntval();
          CcReal* xMeas1; 
          xMeas1 = (CcReal*)pCurrentSection->GetAttribute(SECTION_MEAS1); 
          float fMeas1 = xMeas1->GetRealval();      
          cout << "New PseudoUnit " 
               << " Section: " << pCurrentSection->GetTupleId()
               << " Route: " << iRouteId
               << " From: " << fMeas1 + dStartPositionOnLine
               << " To: " << fMeas1
               << endl;
//          pMGPoint->Add(UGPoint(pCurrentUnit->timeInterval,
//                                iNetworkId,
//                                iRouteId,
//                                bCurrentMovingUp ? Up : Down,
//                                dStartPositionOnLine,
//                                fMeas1));         
          xStart = pCurrentSectionCurve->StartPoint(true);
        }
      }
      // iv.start.ToDouble() 
      // double starttime,
      // Instant start(instanttype);
      // start.ReadFrom(starttime);
//                      Interval<Instant> iv(start,
//                                     end,
//                                     true,
//                                     rc);
//      
      
      // Find new section:
      vector<DirectedSection> xAdjacentSections;
      xAdjacentSections.clear();
      pNetwork->GetAdjacentSections(pCurrentSection->GetTupleId(),
                                    bCurrentMovingUp,
                                    xAdjacentSections);
      bool bNewSectionFound = false;
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
                                                      true, 
                                                      dNextEndPositionOnLine);
        if(bPointOnNextLine)
        {
          // New section found
          
          // Delete the last
//          pCurrentSection->DeleteIfAllowed();
          
          // Take the new one
          pCurrentSection = pNextSection;
          pCurrentSectionCurve = pNextSectionCurve;
          bCurrentMovingUp = bNextUpDownFlag;
          dEndPositionOnLine = dNextEndPositionOnLine;
          pCurrentSectionCurve->AtPoint(xStart, 
                                        true, 
                                        dStartPositionOnLine);
          bNewSectionFound = true;
          break;
        }
      }
      if(!bNewSectionFound && bChangedDirection)
      {
        // Propably we changed the direction on the route
        bCurrentMovingUp = !bCurrentMovingUp;
      }
    }

    // TODO: Wat happens if no section was found?
      
    /////////////////////
    // 
    // Calculate points on route
    //
    CcInt* xRouteId;
    xRouteId = (CcInt*)pCurrentSection->GetAttribute(SECTION_RID);
    int iRouteId = xRouteId->GetIntval();
    CcReal* xMeas1; 
    xMeas1 = (CcReal*)pCurrentSection->GetAttribute(SECTION_MEAS1); 
    float fMeas1 = xMeas1->GetRealval();
    CcBool* xCurveStartsSmaller;
    xCurveStartsSmaller = 
        (CcBool*)pCurrentSection->GetAttribute(SECTION_CURVE_STARTS_SMALLER);
    bool bCurveStartsSmaller = xCurveStartsSmaller->GetBoolval();
    double dFrom;
    double dTo;

    if(bCurveStartsSmaller)
    {
      dFrom = fMeas1 + dStartPositionOnLine;
      dTo = fMeas1 + dEndPositionOnLine;
    }
    else
    {
      double dLength = pCurrentSectionCurve->Length();
      dFrom = fMeas1 + dLength - dStartPositionOnLine;
      dTo = fMeas1 + dLength - dEndPositionOnLine;
    }

    /////////////////////
    // 
    // Create new unit for mgpoint
    //
     
    
    cout << "New Unit " 
         << " Section: " << pCurrentSection->GetTupleId()
         << " Route: " << iRouteId
         << " From: " << dFrom 
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
  NetworkManager::CloseNetwork(pNetwork);
  
  delete pSectionsIt;
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
