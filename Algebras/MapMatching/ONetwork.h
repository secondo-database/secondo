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
ONetwork represents a network based on an ordered relation

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
#include "OrderedRelationAlgebra.h"

using mapmatch::AttributePtr;


/*
3 class ONetwork
  Network based on ordered relation

The template parameter is the int-type used.

*/
template<class T>
class ONetwork {
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
                  std::vector<ONetworkEdge<T> >& vecEdges) const;

    bool GetAdjacentEdges(const ONetworkEdge<T>& rEdge,
                          const bool bUpDown,
                          std::vector<ONetworkEdge<T> >& vecEdges) const;

    Rectangle<2> GetBoundingBox(void) const;


    // Only used by MapMatchingOEdgeTupleStreamCreator
    Tuple* GetUndefEdgeTuple(void) const;

private:

    bool GetEdges(const Tuple* pTuple,
                  std::vector<ONetworkEdge<T> >& vecEdges) const;

    OrderedRelation* m_pOrderedRelation;
    RTree2TID* m_pRTreeEdges;
    Relation* m_pIndexEdges;
    bool m_bOwnData;

    OEdgeAttrIndexes m_EdgeAttrIndexes;
    friend class ONetworkEdge<T>;
};


/*
2 Implementation

*/


template<class T>
ONetwork<T>::ONetwork(OrderedRelation* pOrderedRelation,
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

template<class T>
ONetwork<T>::~ONetwork()
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

template<class T>
bool ONetwork<T>::GetEdges(const Rectangle<2>& rBBox,
                        std::vector<ONetworkEdge<T> >& vecEdges) const
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

template<class T>
bool ONetwork<T>::GetEdges(const Tuple* pTuple,
                        std::vector<ONetworkEdge<T> >& vecEdges) const
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
        vecEdges.push_back(ONetworkEdge<T>(pTupleEdge, this, false));

        pTupleEdge = pIt->GetNextTuple();
    }

    delete pIt;
    pIt = NULL;

    return true;
}

template<class T>
bool ONetwork<T>::GetAdjacentEdges(const ONetworkEdge<T>& rEdge,
                                const bool bUpDown,
                                std::vector<ONetworkEdge<T> >& vecEdges) const
{
    if (m_pOrderedRelation == NULL)
        return false;

    vector<void*> vecAttributes(2);

    AttributePtr<T> pMin(new T(true,0));
    AttributePtr<T> pMax(new T(
                      true,numeric_limits<typename T::inttype>::max()));

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
        vecEdges.push_back(ONetworkEdge<T>(pTupleEdge, this, false));

        pTupleEdge = pIt->GetNextTuple();
    }

    delete pIt;
    pIt = NULL;

    return true;
}

template<class T>
Rectangle<2> ONetwork<T>::GetBoundingBox(void) const
{
    if (m_pRTreeEdges != NULL)
        return m_pRTreeEdges->BoundingBox();
    else
        return Rectangle<2>(false);
}


// Only used by MapMatchingOEdgeTupleStreamCreator
template<class T>
Tuple* ONetwork<T>::GetUndefEdgeTuple(void) const
{
    Rectangle<2> BBox = GetBoundingBox();

    if (!BBox.IsDefined() || m_pRTreeEdges == NULL || m_pIndexEdges == NULL)
    {
        return NULL;
    }

    R_TreeLeafEntry<2, TupleId> res;
    if (m_pRTreeEdges->First(BBox, res))
    {
        Tuple* pTuple = m_pIndexEdges->GetTuple(res.info, false);

        if (pTuple != NULL)
        {
            std::vector<ONetworkEdge<T> > vecEdges;
            GetEdges(pTuple, vecEdges);

            pTuple->DeleteIfAllowed(); pTuple = NULL;

            if (vecEdges.size() > 0)
            {
                const Tuple* pEdgeTuple = vecEdges.front().GetTuple();
                if (pEdgeTuple != NULL)
                {
                    Tuple* pTupleRes = pEdgeTuple->Clone();
                    if (pTupleRes != NULL)
                    {
                        const int nAttributes = pTupleRes->GetNoAttributes();
                        for (int i = 0; i < nAttributes; ++i)
                        {
                            Attribute* pAttr = pTupleRes->GetAttribute(i);
                            if (pAttr != NULL)
                                pAttr->SetDefined(false);
                        }
                    }

                    return pTupleRes;
                }
            }
        }

        assert(false);
        return NULL;
    }
    else
    {
        return NULL;
    }
}





#endif /* __ONETWORK_H_ */


