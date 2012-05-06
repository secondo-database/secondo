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

[1] Implementation of the ONetwork class

April, 2012. Matthias Roth

[TOC]

1 Overview

This implementation file contains the implementation of the class ~ONetwork~.


2 Defines and includes

*/

#include "ONetwork.h"
#include "OrderedRelationAlgebra.h"
#include "RTreeAlgebra.h"
#include "RelationAlgebra.h"
#include "MapMatchingUtil.h"
using mapmatch::AttributePtr;


/*
3 class ONetwork
  Network based on ordered relation

*/

ONetwork::ONetwork(OrderedRelation* pOrderedRelation,
                   RTree2TID* pRTreeEdges,
                   Relation* pIndexEdges,
                   const OEdgeAttrIndexes& rEdgeAttrIndexes)
:m_pOrderedRelation(pOrderedRelation),
 m_pRTreeEdges(pRTreeEdges),
 m_pIndexEdges(pIndexEdges),
 m_bOwnData(false),
 m_EdgeAttrIndexes(rEdgeAttrIndexes)
{
}

ONetwork::~ONetwork()
{
    if (m_bOwnData)
    {
        delete m_pOrderedRelation;
        delete m_pRTreeEdges;
        delete m_pIndexEdges;
    }

    m_pOrderedRelation = NULL;
    m_pRTreeEdges = NULL;
    m_pIndexEdges = NULL;
}

bool ONetwork::GetEdges(const Rectangle<2>& rBBox,
                        std::vector<ONetworkEdge>& vecEdges) const
{
    if (!rBBox.IsDefined() || m_pRTreeEdges == NULL || m_pIndexEdges == NULL)
    {
        assert(false);
        return false;
    }

    R_TreeLeafEntry<2, TupleId> res;
    if (m_pRTreeEdges->First(rBBox, res))
    {
        Tuple* pTuple = m_pIndexEdges->GetTuple(res.info, false);

        if (pTuple != NULL)
        {
            GetEdges(pTuple, vecEdges);

            pTuple->DeleteIfAllowed(); pTuple = NULL;
        }
    }

    while (m_pRTreeEdges->Next(res))
    {
        Tuple* pTuple = m_pIndexEdges->GetTuple(res.info, false);

        if (pTuple != NULL)
        {
            GetEdges(pTuple, vecEdges);

            pTuple->DeleteIfAllowed(); pTuple = NULL;
        }
    }

    return true;
}

bool ONetwork::GetEdges(const Tuple* pTuple,
                        std::vector<ONetworkEdge>& vecEdges) const
{
    if (pTuple == NULL ||
        m_pOrderedRelation == NULL ||
        m_EdgeAttrIndexes.m_IdxSource < 0 ||
        m_EdgeAttrIndexes.m_IdxTarget < 0)
        return false;

    vector<void*> vecAttributes(2);
    vecAttributes[0] = pTuple->GetAttribute(m_EdgeAttrIndexes.m_IdxSource);
    vecAttributes[1] = pTuple->GetAttribute(m_EdgeAttrIndexes.m_IdxTarget);

    vector<SmiKey::KeyDataType> vecAttrTypes(2);
    vecAttrTypes[0] = SmiKey::Integer;
    vecAttrTypes[1] = SmiKey::Integer;

    CompositeKey KeyFrom(vecAttributes, vecAttrTypes, false);
    CompositeKey KeyTo(vecAttributes, vecAttrTypes, true);
    OrderedRelationIterator* pIt = (OrderedRelationIterator*)
                              m_pOrderedRelation->MakeRangeScan(KeyFrom, KeyTo);

    Tuple* pTupleEdge = pIt != NULL ? pIt->GetNextTuple() : NULL;
    while (pTupleEdge != NULL)
    {
        vecEdges.push_back(ONetworkEdge(pTupleEdge, this, false));

        pTupleEdge = pIt->GetNextTuple();
    }

    delete pIt;
    pIt = NULL;

    return true;
}

bool ONetwork::GetAdjacentEdges(const ONetworkEdge& rEdge,
                                const bool bUpDown,
                                std::vector<ONetworkEdge>& vecEdges) const
{
    if (m_pOrderedRelation == NULL)
        return false;

    vector<void*> vecAttributes(2);

    AttributePtr<CcInt> pMin(new CcInt(true,0));
    AttributePtr<CcInt> pMax(new CcInt(true,numeric_limits<int>::max()));

    vecAttributes[0] = bUpDown ? rEdge.GetTarget() : rEdge.GetSource();

    vector<SmiKey::KeyDataType> vecAttrTypes(2);
    vecAttrTypes[0] = SmiKey::Integer;
    vecAttrTypes[1] = SmiKey::Integer;

    vecAttributes[1] = pMin.get();
    CompositeKey KeyFrom(vecAttributes, vecAttrTypes, false);

    vecAttributes[1] = pMax.get();
    CompositeKey KeyTo(vecAttributes, vecAttrTypes, true);

    OrderedRelationIterator* pIt = (OrderedRelationIterator*)
                              m_pOrderedRelation->MakeRangeScan(KeyFrom, KeyTo);

    Tuple* pTupleEdge = pIt != NULL ? pIt->GetNextTuple() : NULL;
    while (pTupleEdge != NULL)
    {
        vecEdges.push_back(ONetworkEdge(pTupleEdge, this, false));

        pTupleEdge = pIt->GetNextTuple();
    }

    delete pIt;
    pIt = NULL;

    return true;
}

Rectangle<2> ONetwork::GetBoundingBox(void) const
{
    if (m_pRTreeEdges != NULL)
        return m_pRTreeEdges->BoundingBox();
    else
        return Rectangle<2>(false);
}


