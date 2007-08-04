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
#ifndef OPSHORTESTPATH_H_
#define OPSHORTESTPATH_H_

struct DijkstraStruct
{
  DijkstraStruct()
  {
  }

  DijkstraStruct(int in_iSegmentId,
                 bool in_bDirectionUp,
                 int in_iRouteId,
                 float in_fLength,
                 float in_fD)
  {
    m_iSegmentId = in_iSegmentId;
    m_bDirectionUp = in_bDirectionUp;
    m_iRouteId = in_iRouteId;
    m_fLength = in_fLength;
    m_fD = in_fD;
  }
  
/*

4.1.3 Copy-Constructor

*/

  DijkstraStruct( const DijkstraStruct& in_xDikstraStruct):
    m_iSegmentId(in_xDikstraStruct.m_iSegmentId),
    m_bDirectionUp(in_xDikstraStruct.m_bDirectionUp),
    m_iRouteId(in_xDikstraStruct.m_iRouteId),
    m_fLength(in_xDikstraStruct.m_fLength), 
    m_fD(in_xDikstraStruct.m_fD)
  {
  }
    
  int m_iSegmentId;
  
  bool m_bDirectionUp;
  
  int m_iRouteId;

  float m_fLength;
  
  float m_fD;
  
  bool operator== (const DijkstraStruct& in_xOther) 
  {
    bool bReturn = (m_iSegmentId == in_xOther.m_iSegmentId &&
                    m_bDirectionUp == in_xOther.m_bDirectionUp); 
    return bReturn;
  }
};

// TODO: Umbenennen in DikstraQueue
class PriorityQueue
{
  public:

  void push(DijkstraStruct* in_pStruct)
  {
    // TODO: Zweite Datenstruktur befüllen
    m_xElements.push_back(in_pStruct);
    
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

//    for(size_t i = 0; i < m_xElements.size(); ++i) 
//    {
//      DijkstraStruct xCurrentStruct = m_xElements[i];
//      cout << "Before " << i << ": " 
//             << xCurrentStruct.m_iSegmentId << " "
//             << xCurrentStruct.m_bDirectionUp << endl;
//    }

    for(size_t i = 0; i < m_xElements.size(); ++i) 
    {
      DijkstraStruct* pCurrentStruct = m_xElements[i];
      if(i == 0 ||
         pCurrentStruct->m_fLength < pMinDijkstraStruct->m_fLength)
      {
        pMinDijkstraStruct = pCurrentStruct;
      }
    }
    
    std::vector<DijkstraStruct*>::iterator xIt = m_xElements.begin();
    while(xIt != m_xElements.end()) 
    {
      DijkstraStruct* pCurrent = *xIt;
      if(pCurrent->m_iSegmentId == pMinDijkstraStruct->m_iSegmentId &&
         pCurrent->m_bDirectionUp == pMinDijkstraStruct->m_bDirectionUp)
      {
        m_xElements.erase (xIt);
        break;
      }
      xIt++;
    }

//    for(size_t i = 0; i < m_xElements.size(); ++i) 
//    {
//      DijkstraStruct xCurrentStruct = m_xElements[i];
//      cout << "After " << i << ": "
//           << xCurrentStruct.m_iSegmentId << " " 
//           << xCurrentStruct.m_bDirectionUp << endl;
//    }
    return pMinDijkstraStruct;
  }
  
  bool isEmtpy()
  {
    return m_xElements.empty();
  }

  // TODO: Methode zum Zugriff auf d
  
  // TODO: Methode zum Zugriff auf pi
  
  // TODO: Methode zur durchführung von Relas mit setzen von pi und d

  private:
  
  // TODO: Auf Pointer umbauen
  vector<DijkstraStruct*> m_xElements;
  
  // TODO: Zweite Datenstruktur mit allen Elementen in sortiert
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


  static void Dijkstra(Network* in_pNetwork,
                       int in_iStartSegmentId,
                       int in_iEndSegmentId);


/*
4.1.3 Specification of the operator

*/
  static const string Spec;    
};

#endif /*OPSHORTESTPATH_H_*/
