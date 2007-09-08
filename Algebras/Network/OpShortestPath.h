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

1 Declaration of operator shortest path


March 2004 Victor Almeida

Mai-Oktober 2007 Martin Scheppokat

1.1 Overview


This file contains the implementation of ~gline~


2.1 Defines, includes, and constants

*/
#ifndef OPSHORTESTPATH_H_
#define OPSHORTESTPATH_H_

struct DijkstraStruct
{
  DijkstraStruct()
  {
  }

  DijkstraStruct(int in_iSectionTid,
                 bool in_bUpDownFlag,
                 int in_iRouteId,
                 float in_fMeas1,
                 float in_fMeas2,
                 float in_fD,
                 float in_fHeuristicDistanceToEnd,
                 int in_iPiSectionTid,
                 bool in_bPiUpDownFlag)
  {
    m_iSectionTid = in_iSectionTid;
    m_bUpDownFlag = in_bUpDownFlag;
    m_iRouteId = in_iRouteId;
    m_fMeas1 = in_fMeas1;
    m_fMeas2 = in_fMeas2;
    m_fD = in_fD;
    m_fHeuristicDistanceToEnd = in_fHeuristicDistanceToEnd;
    m_iPiSectionTid = in_iPiSectionTid;
    m_bPiUpDownFlag = in_bPiUpDownFlag;
  }
  
/*

4.1.3 Copy-Constructor

*/

  DijkstraStruct( const DijkstraStruct& in_xDikstraStruct):
    m_iSectionTid(in_xDikstraStruct.m_iSectionTid),
    m_bUpDownFlag(in_xDikstraStruct.m_bUpDownFlag),
    m_iRouteId(in_xDikstraStruct.m_iRouteId),
    m_fMeas1(in_xDikstraStruct.m_fMeas1), 
    m_fMeas2(in_xDikstraStruct.m_fMeas2), 
    m_fD(in_xDikstraStruct.m_fD),
    m_fHeuristicDistanceToEnd(in_xDikstraStruct.m_fHeuristicDistanceToEnd),
    m_iPiSectionTid(in_xDikstraStruct.m_iPiSectionTid),
    m_bPiUpDownFlag(in_xDikstraStruct.m_bPiUpDownFlag)
  {
  }
    
  int m_iSectionTid;
  
  bool m_bUpDownFlag;
  
  int m_iRouteId;

  float m_fMeas1;

  float m_fMeas2;
  
  float m_fD;

  float m_fHeuristicDistanceToEnd;

  int m_iPiSectionTid;
  
  bool m_bPiUpDownFlag;

  float Length()
  {
    return m_fMeas2 - m_fMeas1;
  }
  
  float DistanceEstimation()
  {
    return m_fD + m_fHeuristicDistanceToEnd;
  }
  
  
  bool operator== (const DijkstraStruct& in_xOther) 
  {
    bool bReturn = (m_iSectionTid == in_xOther.m_iSectionTid &&
                    m_bUpDownFlag == in_xOther.m_bUpDownFlag); 
    return bReturn;
  }
};

// TODO: Umbenennen in DikstraQueue
class PriorityQueue
{
  public:
  
  PriorityQueue()
  {
  }

  void push(DijkstraStruct* in_pStruct)
  {
    // TODO: Zweite Datenstruktur befüllen
    m_xQueue.push_back(in_pStruct);
    size_t iIndex = 2 * (in_pStruct->m_iSectionTid - 1);
    if(in_pStruct->m_bUpDownFlag)
    {
      iIndex++;
    }
    if(m_xElements.size() < iIndex + 1)
    {
      m_xElements.resize(iIndex + 1);
    }
    m_xElements[iIndex] = in_pStruct;
    
  }
  
  // TODO: Delete-Operator
  
  DijkstraStruct* pop()
  {
    
    // ----------------------------------------------------------
    // For testing only !
    // This implementation needs O(n)
    // To be replaced
    // ----------------------------------------------------------
    DijkstraStruct* pMinDijkstraStruct;

//    for(size_t i = 0; i < m_xQueue.size(); ++i) 
//    {
//      DijkstraStruct xCurrentStruct = m_xQueue[i];
//      cout << "Before " << i << ": " 
//             << xCurrentStruct.m_iSegmentId << " "
//             << xCurrentStruct.m_bDirectionUp << endl;
//    }

    for(size_t i = 0; i < m_xQueue.size(); ++i) 
    {
      DijkstraStruct* pCurrentStruct = m_xQueue[i];
      if(i == 0 ||
         pCurrentStruct->DistanceEstimation() < 
         pMinDijkstraStruct->DistanceEstimation())
      {
        pMinDijkstraStruct = pCurrentStruct;
      }
    }
    
    std::vector<DijkstraStruct*>::iterator xIt = m_xQueue.begin();
    while(xIt != m_xQueue.end()) 
    {
      DijkstraStruct* pCurrent = *xIt;
      if(pCurrent->m_iSectionTid == pMinDijkstraStruct->m_iSectionTid &&
         pCurrent->m_bUpDownFlag == pMinDijkstraStruct->m_bUpDownFlag)
      {
        m_xQueue.erase (xIt);
        m_xS.push_back(pCurrent);

        break;
      }
      xIt++;
    }

//    for(size_t i = 0; i < m_xQueue.size(); ++i) 
//    {
//      DijkstraStruct xCurrentStruct = m_xQueue[i];
//      cout << "After " << i << ": "
//           << xCurrentStruct.m_iSegmentId << " " 
//           << xCurrentStruct.m_bDirectionUp << endl;
//    }
    return pMinDijkstraStruct;
  }
  
  bool isEmtpy()
  {
    return m_xQueue.empty();
  }

  DijkstraStruct* get(int in_iSectionTid,
                      bool in_bUpDownFlag)
  {
    size_t iIndex = 2 * (in_iSectionTid - 1);
    if(in_bUpDownFlag)
    {
      iIndex++;
    }
    return m_xElements[iIndex];
  }

  int getSSize()
  {
    return m_xS.size();
  }

  DijkstraStruct* getS(int in_iIndex)
  {
    return m_xS[in_iIndex];
  }

  private:
  
  // TODO: Auf Pointer umbauen
  vector<DijkstraStruct*> m_xElements;
  
  // TODO: Zweite Datenstruktur mit allen Elementen in sortiert
  vector<DijkstraStruct*> m_xQueue;
  
  vector<DijkstraStruct*> m_xS;
  
};

class OpShortestPath
{
  public:
  
/*
4.1.2 Typemap function of the operator

*/
  static ListExpr TypeMap(ListExpr args);

/*
4.1.2 Value mapping function of the operator

*/
  static int ValueMapping( Word* in_pArgs, 
                           Word& in_xResult, 
                           int in_iMessage, 
                           Word& in_xLocal, 
                           Supplier in_xSupplier );

/*
4.1.3 Specification of the operator

*/
  static const string Spec;    

  private:

/*
Dikstra's Algorithm

*/
  static void Dijkstra(Network* in_pNetwork,
                       int in_iStartSegmentId,
                       GPoint* in_pFromGPoint,
                       int in_iEndSegmentId,
                       GPoint* in_pToGPoint,
                       Point* in_pToPoint,
                       GLine* in_pGLine);

/*
Sending a message via the message-center

*/
  static void sendMessage(string in_strMessage);
};

#endif /*OPSHORTESTPATH_H_*/
