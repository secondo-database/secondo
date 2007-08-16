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
#include "GLine.h"
#include "GPoint.h"
#include "Network.h"

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
  result = SetWord( pGLine ); 

  GPoint* pGPoint1 = (GPoint*)args[0].addr;
  GPoint* pGPoint2 = (GPoint*)args[1].addr;

  bool bDefined;
  Word xValue;

  bool bIsObject = SecondoSystem::GetCatalog()->IsObjectName("X_NETWORK");

  if(!bIsObject)
  {
    // Fehlerbehandlung
    cerr << "X_NETWORK is not a secondo-object" << endl;
    return 0;
  }
  
  
  

  bool bOk = SecondoSystem::GetCatalog()->GetObject("X_NETWORK",
                                                    xValue,
                                                    bDefined);
                                         

  if(!bDefined || !bOk)
  {
    // Fehlerbehandlung
    cerr << "Could not load object." << endl;
    return 0;
  }           
  
  Network* pNetwork = (Network*)xValue.addr;
  
  Tuple* pFromSection = pNetwork->GetSectionIdOnRoute(pGPoint1);
  Tuple* pToSection = pNetwork->GetSectionIdOnRoute(pGPoint2);
  pGLine->SetNetworkId(1);

  Dijkstra(pNetwork, 
           pFromSection->GetTupleId(), 
           pToSection->GetTupleId(),
           pGLine);

  SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom( "network" ),
                                           xValue);
 
  return 0;
}

void OpShortestPath::Dijkstra(Network* in_pNetwork,
                              int in_iStartSegmentId,
                              int in_iEndSegmentId,
                              GLine* in_pGLine )
{
  Relation* pSections = in_pNetwork->GetSectionsInternal();

//  int iSectionCount = pSections->GetNoTuples();
  vector<int> xPi;
  PriorityQueue xQ;
   
  GenericRelationIterator* pSectionsIt = pSections->MakeScan();
  Tuple* pSection;

  //InitializeSingleSource
  while( (pSection = pSectionsIt->GetNextTuple()) != 0 )
  {
    int iSegmentId = pSectionsIt->GetTupleId();
    CcInt* xRouteId = (CcInt*)pSection->GetAttribute(SECTION_RID); 
    int iRouteId = xRouteId->GetIntval();
    CcReal* xMeas1 = (CcReal*)pSection->GetAttribute(SECTION_MEAS1); 
    float fMeas1 = xMeas1->GetRealval();
    CcReal* xMeas2 = (CcReal*)pSection->GetAttribute(SECTION_MEAS2); 
    float fMeas2 = xMeas2->GetRealval();
    
    float fD = float(1e29);
    if(iSegmentId == in_iStartSegmentId)
    {
      fD = 0;
    }

    xQ.push(new DijkstraStruct(iSegmentId,
                                true,
                                iRouteId,
                                fMeas1,
                                fMeas2,
                                fD,
                                -1,
                                true));
    xQ.push(new DijkstraStruct(iSegmentId,
                           false,
                           iRouteId,
                           fMeas1,
                           fMeas2,
                           fD,
                           -1,
                           true));

    pSection->DeleteIfAllowed();
  }
  delete pSectionsIt;

//  for each vertex v in V[G]
//  {
//    do d[v] = infty;
//    pi[v] = NIL
//  }
//  d[s] = 0;
//  
//  
//  
//  // Priority Queue with all vertices of the Graph which are not
//  // in S keyed by their d-values.
//  Q = Vertices(G);
//  
  while(! xQ.isEmtpy())
  {
    DijkstraStruct* pCurrent = xQ.pop();
    int iCurrentSectionTid = pCurrent->m_iSectionTid;
    bool bCurrentUpDownFlag = pCurrent->m_bUpDownFlag;
//    cout << "Section: " 
//         << iCurrentSectionTid 
//         << " " 
//         << bCurrentUpDownFlag << endl;
//    
//    S = S with u
//    
//    for each adjazent vertex v in adj[u]

    vector<DirectedSection> xAdjacentSections;
    xAdjacentSections.clear();
    in_pNetwork->GetAdjacentSections(iCurrentSectionTid,
                                     bCurrentUpDownFlag,
                                     xAdjacentSections);
                                     
    for(size_t i = 0;  i < xAdjacentSections.size(); i++) 
    {
      DirectedSection xAdjacentSection = xAdjacentSections[i];
      int iAdjacentSectionTid = xAdjacentSection.getSectionTid();
      bool bAdjacentUpDownFlag = xAdjacentSection.getUpDownFlag();
      
//      cout << "(" <<iCurrentSectionTid << ", " << bCurrentUpDownFlag
//           << ")->(" 
//           << iAdjacentSectionTid << ", " << bAdjacentUpDownFlag << ")" 
//           << endl;

      DijkstraStruct* pAdjacent = xQ.get(iAdjacentSectionTid,
                                         bAdjacentUpDownFlag);

      // Relax
      if(pAdjacent->m_fD > pCurrent->m_fD + pCurrent->Length())
      {

        // Calculate new distance
        pAdjacent->m_fD = pCurrent->m_fD + pCurrent->Length();

//      cout << "(" << iAdjacentSectionTid << ", " << bAdjacentUpDownFlag << ")"
//             << " = " << pAdjacent->m_fD
//             << endl;

        
        // Set current as predecessor of adjacent section
        pAdjacent->m_iPiSectionTid = pCurrent->m_iSectionTid;
        pAdjacent->m_bPiUpDownFlag = pCurrent->m_bUpDownFlag;
      }
    }    
  }
    
  // Find the route starting at the end looking at the pi-entries pointing
  // at a predecessor.  
  DijkstraStruct* pStruct = xQ.get(in_iEndSegmentId,
                                   false);
 
  if(pStruct->m_iPiSectionTid == -1)
  {
    // End not reachable from start. Either the graph consists of two
    // parts or the ConnectivityCode prevents driving on all path from 
    // start to end.
    // Return empty GLine
    return;
  }
 
//  cout << "(" << pStruct->m_iSectionTid 
//       << ", " 
//       << pStruct->m_bUpDownFlag << ")" 
//       << endl;
  in_pGLine->AddRouteInterval(pStruct->m_iRouteId, 
                              pStruct->m_fMeas1, 
                              pStruct->m_fMeas2);
  while(pStruct->m_iSectionTid != in_iStartSegmentId)
  {
    pStruct = xQ.get(pStruct->m_iPiSectionTid,
                     pStruct->m_bPiUpDownFlag);
//    cout << "(" << pStruct->m_iSectionTid 
//         << ", " 
//         << pStruct->m_bUpDownFlag << ")" 
//         << endl;
    in_pGLine->AddRouteInterval(pStruct->m_iRouteId, 
                                pStruct->m_fMeas1, 
                                pStruct->m_fMeas2);
  }
  
      
                                         
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
