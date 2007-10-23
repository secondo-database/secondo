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

The operator translates moving points into network based moving points

Mai-Oktober 2007 Martin Scheppokat

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

#include "OpMPoint2MGPoint.h"

/*
Typemap function of the operator

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
Value mapping function of operator ~mpoint2mgpoint~

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
  MGPoint* pMGPoint = (MGPoint*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pMGPoint ); 

  // Get input values
  CcInt* pNetworkId = (CcInt*)args[0].addr;
  int iNetworkId = pNetworkId->GetIntval();
  MPoint* pMPoint = (MPoint*)args[1].addr;  

  if(pMPoint->GetNoComponents() == 0)
  {
    string strMessage = "MPoint is Empty.";   
    cerr << endl << strMessage << endl << endl;
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

    
  // Load the network
  Network* pNetwork = NetworkManager::GetNetwork(iNetworkId);
        
    
  /////////////////////
  // 
  // Find first section
  //
  int iStartSectionTid;
  Tuple* pStartSection;
  Line* pStartSectionCurve;
  bool bStartMovingUp;
  Relation* pSections = pNetwork->GetSectionsInternal();   
  GenericRelationIterator* pSectionsIt = pSections->MakeScan();
  while( (pStartSection = pSectionsIt->GetNextTuple()) != 0 )
  {
    // TODO: Maybe support this search with a BTree
    pStartSectionCurve = (Line*)pStartSection->GetAttribute(SECTION_CURVE);
    
    double dFirstPos;
    bool bFirstOnLine = pStartSectionCurve->AtPoint(xFirstPoint, 
                                                    true, 
                                                    dFirstPos);

    double dSecondPos;
    bool bSecondOnLine = pStartSectionCurve->AtPoint(xSecondPoint, 
                                                     true, 
                                                     dSecondPos);


    if(bFirstOnLine && bSecondOnLine)
    {
      iStartSectionTid = pStartSection->GetTupleId();
      bStartMovingUp = dFirstPos < dSecondPos;
      pStartSection->DeleteIfAllowed(); 
      break;
    }
        
    pStartSection->DeleteIfAllowed(); 
  }
  // TODO: Fehlerbehandlung wenn kein Segment gefunden wurde
  delete pSectionsIt;

  // This section will be valid as long as the moving point 
  // moves somewhere over it. If not it is to be replaced
  // by an adjacent section.
  // All of the following value are always up to date 
  pSections = pNetwork->GetSectionsInternal();   
  int iCurrentSectionTid = iStartSectionTid;
  Tuple* pCurrentSection = pSections->GetTuple(iCurrentSectionTid);
  Line* pCurrentSectionCurve = 
     (Line*)pCurrentSection->GetAttribute(SECTION_CURVE);;
  CcBool* xCurrentCurveStartsSmaller;
  xCurrentCurveStartsSmaller = 
      (CcBool*)pCurrentSection->GetAttribute(SECTION_CURVE_STARTS_SMALLER);
  bool bCurrentCurveStartsSmaller = xCurrentCurveStartsSmaller->GetBoolval();
  bool bCurrentMovingUpOnCurve = bStartMovingUp;
  bool bCurrentMovingUp = bCurrentCurveStartsSmaller ? 
                          bCurrentMovingUpOnCurve : 
                          !bCurrentMovingUpOnCurve;
  CcInt* xCurrentRouteId = (CcInt*)pCurrentSection->GetAttribute(SECTION_RID);
  int iCurrentRouteId = xCurrentRouteId->GetIntval();
  CcReal* xCurrentMeas1; 
  xCurrentMeas1 = (CcReal*)pCurrentSection->GetAttribute(SECTION_MEAS1); 
  double dCurrentMeas1 = xCurrentMeas1->GetRealval();
  CcReal* xCurrentMeas2; 
  xCurrentMeas2 = (CcReal*)pCurrentSection->GetAttribute(SECTION_MEAS2); 
  double dCurrentMeas2 = xCurrentMeas2->GetRealval();      


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
    
    
    // Check wether start (should be) and end-positions are on line. If so
    // the positions on that line are beeing calculated
    double dStartPositionOnCurve;
    bool bStartPointOnCurve; 
    bStartPointOnCurve = pCurrentSectionCurve->AtPoint(xStart, 
                                                       true, 
                                                       dStartPositionOnCurve);
    double dEndPositionOnCurve;
    bool bEndPointOnCurve = pCurrentSectionCurve->AtPoint(xEnd, 
                                                          true, 
                                                          dEndPositionOnCurve);
    
    // Maybe the direction changed. If this is the case we have to find 
    // the adjacent section in the opposite direction
    bool bChangedDirection = (bCurrentMovingUpOnCurve && 
                             dStartPositionOnCurve > dEndPositionOnCurve) ||
                             (!bCurrentMovingUpOnCurve && 
                             dStartPositionOnCurve < dEndPositionOnCurve);
                             
    /////////////////////
    // 
    // Find alternative next segment
    //
    if(!bEndPointOnCurve || bChangedDirection)
    {
      
      // Weg auf aktuellem Segment beenden. Hierzu muss zunächst erkannt
      // werden, ob das Segment bei 0 oder bei Length endet. Danach muss
      // der Zeitpunkt bestimmt werden, zu dem der Punkt erreicht ist. Da
      // die Bewegung linear ist und die Länge bis zum Ende sowie die Länge
      // der Unit des MPoint bekannt sind sollte dies kein Problem sein.
      // Danach UNIT einfüben und neue Start-Zeit merken.
      if(bCurrentMovingUpOnCurve && 
         dStartPositionOnCurve < pCurrentSectionCurve->Length())
      {
        // For a change to another segment the end of the last one has
        // to be reached.
        // End not reached. Invent pseudo-segment
        xStart = pCurrentSectionCurve->EndPoint(true);
      }
      else if (!bCurrentMovingUpOnCurve &&
               dStartPositionOnCurve > 0) 
      {
        // For a change to another segment the start of the last one has
        // to be reached.
        // Start not reached. Invent pseudo-segment
        xStart = pCurrentSectionCurve->StartPoint(true);
      }
      
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
        bool bNextMovingUp = xAdjacentSection.getUpDownFlag();

        // Load section and curve     
        Tuple* pNextSection = pSections->GetTuple(iNextSectionTid);
        Line* pNextSectionCurve;
        pNextSectionCurve = (Line*)pNextSection->GetAttribute(SECTION_CURVE);
        
        // Check wether the end is on this section (if so dPositionOnCurve
        // will reflect the position on the segment)
        bool bNextEndOnCurve;
        double dNextEndPositionOnCurve;
        bNextEndOnCurve = pNextSectionCurve->AtPoint(xEnd, 
                                                     true, 
                                                     dNextEndPositionOnCurve);
        if(bNextEndOnCurve)
        {
          // New section found
          
          // Take the new one
          bNewSectionFound = true;
          pCurrentSection = pNextSection;
          pCurrentSectionCurve = pNextSectionCurve;
          xCurrentCurveStartsSmaller = 
        (CcBool*)pCurrentSection->GetAttribute(SECTION_CURVE_STARTS_SMALLER);
          bCurrentCurveStartsSmaller = xCurrentCurveStartsSmaller->GetBoolval();
          bCurrentMovingUp = bNextMovingUp;         
          bCurrentMovingUpOnCurve = bCurrentCurveStartsSmaller ? 
                                    bCurrentMovingUp : 
                                    !bCurrentMovingUp;
          
          
          dEndPositionOnCurve = dNextEndPositionOnCurve;
          pCurrentSectionCurve->AtPoint(xStart, 
                                        true, 
                                        dStartPositionOnCurve);
          xCurrentRouteId = (CcInt*)pCurrentSection->GetAttribute(SECTION_RID);
          iCurrentRouteId = xCurrentRouteId->GetIntval();
          xCurrentMeas1 = (CcReal*)pCurrentSection->GetAttribute(SECTION_MEAS1);
          dCurrentMeas1 = xCurrentMeas1->GetRealval();
          xCurrentMeas2 = (CcReal*)pCurrentSection->GetAttribute(SECTION_MEAS2);
          dCurrentMeas2 = xCurrentMeas2->GetRealval();      
          break;
        }
      }
      if(!bNewSectionFound && bChangedDirection)
      {
        // Propably we changed the direction on the route
        bCurrentMovingUp = !bCurrentMovingUp;
        bCurrentMovingUpOnCurve = !bCurrentMovingUpOnCurve;
      }
    }
          
    /////////////////////
    // 
    // Calculate points on route
    //  
    double dFrom;
    double dTo;
    if(bCurrentCurveStartsSmaller)
    {
      dFrom = dCurrentMeas1 + dStartPositionOnCurve;
      dTo = dCurrentMeas1 + dEndPositionOnCurve;
    }
    else
    {
      double dLength = pCurrentSectionCurve->Length();
      dFrom = dCurrentMeas1 + dLength - dStartPositionOnCurve;
      dTo = dCurrentMeas1 + dLength - dEndPositionOnCurve;
    }

    /////////////////////
    // 
    // Create new unit for mgpoint
    //
    pMGPoint->Add(UGPoint(pCurrentUnit->timeInterval,
                          iNetworkId,
                          iCurrentRouteId,
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
Specification of operator ~mpoint2mgpoint~

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
