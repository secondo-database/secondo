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
#include "GLine.h"
#include "GPoint.h"
#include "Network.h"
#include "NetworkManager.h"

#include "OpShortestPath.h"
#include "Messages.h"


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

  // Check wether both points belong to the same network
  if(pGPoint1->GetNetworkId() != pGPoint2->GetNetworkId())
  {
    sendMessage("Both gpoints belong to different networks.");
    return 0;
  }
  
  // Load the network
  Network* pNetwork = NetworkManager::GetNetwork(pGPoint1->GetNetworkId());
  if(pNetwork == 0)
  {
    ostringstream xStream;
    xStream << pGPoint1->GetNetworkId() << char(0);
    string strMessage = "Network with id '" +
                        xStream.str() +
                        "' does not exist on this database.";   
    sendMessage(strMessage);
    return 0;
  }
  
  // Get sections where the path should start or end 
  Tuple* pFromSection = pNetwork->GetSectionIdOnRoute(pGPoint1);  
  Tuple* pToSection = pNetwork->GetSectionIdOnRoute(pGPoint2);
    
  if(pToSection == 0 ||
     pFromSection == 0)
  {
    sendMessage("Start or End not found. Possibly the route has no " 
                "junctions or is not part of the network.");   
    if(pFromSection != 0)
    {
      pFromSection->DeleteIfAllowed();
    }
    if(pToSection != 0)
    {
      pToSection->DeleteIfAllowed();
    }
    NetworkManager::CloseNetwork(pNetwork);
    return 0;
  }

  pGLine->SetNetworkId(1);

  // Calculate the shortest path
  Dijkstra(pNetwork, 
           pFromSection->GetTupleId(), 
           pToSection->GetTupleId(),
           pGLine);

  // Cleanup and return
  NetworkManager::CloseNetwork(pNetwork);
  return 0;
}

/*
Modified version of dikstras algorithm calculating shortest path in graphs.

Whereas sedgewick's version of disktra operates on the nodes of the graph this
version will process an array of edges. This change is necessary to take into
account that not all transitions between connecting edges are possible. The 
preceding edge has to be known when looking for the next one. 

*/ 
void OpShortestPath::Dijkstra(Network* in_pNetwork,
                              int in_iStartSegmentId,
                              int in_iEndSegmentId,
                              GLine* in_pGLine )
{
  // Specialized data structure with all edges of the graph. This 
  // structure will not only support the access to the remaining edges 
  // with the fewest weight but also to all edges by their index.
  PriorityQueue xQ;

  /////////////////////////////////////////
  //
  // InitializeSingleSource
  //
  // Get all Sections and fill them into Q. All weights but
  // the one for the starting segment are set to infinity 
  Relation* pSections = in_pNetwork->GetSectionsInternal();
  GenericRelationIterator* pSectionsIt = pSections->MakeScan();
  Tuple* pSection;
  while( (pSection = pSectionsIt->GetNextTuple()) != 0 )
  {
    // Get values
    int iSegmentId = pSectionsIt->GetTupleId();
    CcInt* xRouteId = (CcInt*)pSection->GetAttribute(SECTION_RID); 
    int iRouteId = xRouteId->GetIntval();
    CcReal* xMeas1 = (CcReal*)pSection->GetAttribute(SECTION_MEAS1); 
    float fMeas1 = xMeas1->GetRealval();
    CcReal* xMeas2 = (CcReal*)pSection->GetAttribute(SECTION_MEAS2); 
    float fMeas2 = xMeas2->GetRealval();
    
    // Set weights
    float fD = float(1e29);
    if(iSegmentId == in_iStartSegmentId)
    {
      fD = 0;
    }

    // Put one struct for each direction in the map (up and down)
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

  /////////////////////////////////
  //
  // Now dikstras algorithm will handle each node in
  // the queue recalculation their weights.
  //
  while(! xQ.isEmtpy())
  {
    DijkstraStruct* pCurrent = xQ.pop();
    
    // Get values
    int iCurrentSectionTid = pCurrent->m_iSectionTid;
    bool bCurrentUpDownFlag = pCurrent->m_bUpDownFlag;

    // Get all adjacent sections
    vector<DirectedSection> xAdjacentSections;
    xAdjacentSections.clear();
    in_pNetwork->GetAdjacentSections(iCurrentSectionTid,
                                     bCurrentUpDownFlag,
                                     xAdjacentSections);
                                
    // Iterate over adjacent sections                                     
    for(size_t i = 0;  i < xAdjacentSections.size(); i++) 
    {
      // Load the structure belonging to the adjacent section
      DirectedSection xAdjacentSection = xAdjacentSections[i];
      int iAdjacentSectionTid = xAdjacentSection.getSectionTid();
      bool bAdjacentUpDownFlag = xAdjacentSection.getUpDownFlag();
      DijkstraStruct* pAdjacent = xQ.get(iAdjacentSectionTid,
                                         bAdjacentUpDownFlag);

      // Relax the weight if a shorter path has been found
      if(pAdjacent->m_fD > pCurrent->m_fD + pCurrent->Length())
      {
        // Calculate new distance
        pAdjacent->m_fD = pCurrent->m_fD + pCurrent->Length();

        // Set current as predecessor of adjacent section
        pAdjacent->m_iPiSectionTid = pCurrent->m_iSectionTid;
        pAdjacent->m_bPiUpDownFlag = pCurrent->m_bUpDownFlag;
      }
    }    
  }
  // Now all weights and all predecessors have been found.
    
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
    string strMessage = "Destination is not reachable from the start. Either "
                        "the points are located in disjoint parts of the "
                        "network or the ConnectityCode prevents reaching the "
                        "destination.";   
    sendMessage(strMessage);
    return;
  }

  // Add last interval of the route 
  in_pGLine->AddRouteInterval(pStruct->m_iRouteId, 
                              pStruct->m_fMeas1, 
                              pStruct->m_fMeas2);
  // Add all predecessors until the start is reached.
  while(pStruct->m_iSectionTid != in_iStartSegmentId)
  {
    pStruct = xQ.get(pStruct->m_iPiSectionTid,
                     pStruct->m_bPiUpDownFlag);
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

  
/*
Sending a message via the message-center

*/
void OpShortestPath::sendMessage(string in_strMessage)
{
  // Get message-center and initialize message-list
  static MessageCenter* xMessageCenter= MessageCenter::GetInstance();
  NList xMessage;
  xMessage.append(NList("simple")); 

  // Split the string into parts of maximum length. Append each part
  // to the message
  size_t iCurrentPos = 0;
  int iRemainingLength = in_strMessage.length(); 
  while(iRemainingLength > 0)  
  {
    int iSubstrLength = MAX_STRINGSIZE <? iRemainingLength;
    string strCurrentPart = in_strMessage.substr(iCurrentPos, 
                                                 iSubstrLength);    
    xMessage.append(NList(strCurrentPart));
    
    // Calculate next position
    iCurrentPos += iSubstrLength;
    iRemainingLength -= iSubstrLength;
  }
    
  xMessageCenter->Send(xMessage);
}
