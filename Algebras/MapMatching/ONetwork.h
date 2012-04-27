/*
----
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
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[_] [\_]

[1] Header File of ONetwork (Network based on ordered relation)

April, 2012. Matthias Roth

[TOC]

1 Overview

This header file contains the class ~ONetwork~

2 Defines and includes

*/

#ifndef __ONETWORK_H_
#define __ONETWORK_H_

class OrderedRelation;
class Relation;
#include "RTreeAlgebra.h"
#include "RectangleAlgebra.h"
#include "ONetworkEdge.h"
#include <vector>


/*
3 class ONetwork
  Network based on ordered relation

*/

class ONetwork
{
public:

    struct OEdgeAttrIndexes
    {
        OEdgeAttrIndexes(int IdxSource = -1, int IdxTarget = -1,
                         int IdxSourcePos = -1, int IdxTargetPos = -1,
                         int IdxCurve = -1, int IdxRoadName = -1,
                         int IdxRoadType = -1, int IdxMaxSpeed = -1,
                         int IdxWayId = -1)
        :m_IdxSource(IdxSource), m_IdxTarget(IdxTarget),
         m_IdxSourcePos(IdxSourcePos), m_IdxTargetPos(IdxTargetPos),
         m_IdxCurve(IdxCurve), m_IdxRoadName(IdxRoadName),
         m_IdxRoadType(IdxRoadType), m_IdxMaxSpeed(IdxMaxSpeed),
         m_IdxWayId(IdxWayId)
        {
        }

        int m_IdxSource;
        int m_IdxTarget;
        int m_IdxSourcePos;
        int m_IdxTargetPos;
        int m_IdxCurve;
        int m_IdxRoadName;
        int m_IdxRoadType;
        int m_IdxMaxSpeed;
        int m_IdxWayId;
    };

    ONetwork(OrderedRelation* pOrderedRelation,
             RTree2TID* pRTreeEdges,
             Relation* pIndexEdges,
             const OEdgeAttrIndexes& rEdgeAttrIndexes);
    ~ONetwork();

    bool GetEdges(const Rectangle<2>& rBBox,
                  std::vector<ONetworkEdge>& vecEdges) const;

    bool GetAdjacentEdges(const ONetworkEdge& rEdge,
                          const bool bUpDown,
                          std::vector<ONetworkEdge>& vecEdges) const;

    Rectangle<2> GetBoundingBox(void) const;

private:

    bool GetEdges(const Tuple* pTuple,
                  std::vector<ONetworkEdge>& vecEdges) const;

    OrderedRelation* m_pOrderedRelation;
    RTree2TID* m_pRTreeEdges;
    Relation* m_pIndexEdges;
    bool m_bOwnData;

    OEdgeAttrIndexes m_EdgeAttrIndexes;
    friend class ONetworkEdge;
};


#endif /* __ONETWORK_H_ */


