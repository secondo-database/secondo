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

[1] Header File of the MapMatching Algebra

April, 2012. Matthias Roth

[TOC]

1 Overview

This header file essentially contains the definition of the class ~MGPointCreator~.

2 Defines and includes

*/

#include "MapMatchingMHTOEdgeTupleStreamCreator.h"
#include "ONetworkEdge.h"
#include "ONetworkAdapter.h"
#include <RelationAlgebra.h>


namespace mapmatch {

/*
3 class ORelCreator

*/

OEdgeTupleStreamCreator::OEdgeTupleStreamCreator()
:m_pTupleBuffer(NULL),
 m_pTupleIterator(NULL)
{
}

OEdgeTupleStreamCreator::~OEdgeTupleStreamCreator()
{
    delete m_pTupleIterator;
    m_pTupleIterator = NULL;

    delete m_pTupleBuffer;
    m_pTupleBuffer = NULL;
}

bool OEdgeTupleStreamCreator::CreateResult(
                     const std::vector<MHTRouteCandidate*>& rvecRouteCandidates)
{
    if (m_pTupleIterator != NULL)
    {
        delete m_pTupleIterator;
        m_pTupleIterator = NULL;
    }

    if (m_pTupleBuffer != NULL)
    {
        delete m_pTupleBuffer;
        m_pTupleBuffer = NULL;
    }

    m_pTupleBuffer = new TupleBuffer;

    bool bPrevCandidateFailed = false; // -> matching failed
                                       // -> don't connect by shortest path

    const size_t nCandidates = rvecRouteCandidates.size();
    for (size_t i = 0; i < nCandidates; ++i)
    {
        bool bCalcShortestPath = ((i > 0) && !bPrevCandidateFailed);

        MHTRouteCandidate* pCandidate = rvecRouteCandidates[i];
        if (pCandidate == NULL)
            continue;

        bPrevCandidateFailed = pCandidate->GetFailed();

        const std::vector<MHTRouteCandidate::RouteSegment*>&
                              vecRouteSegments = pCandidate->GetRouteSegments();

        std::vector<MHTRouteCandidate::RouteSegment*>::const_iterator it =
                                                       vecRouteSegments.begin();
        std::vector<MHTRouteCandidate::RouteSegment*>::const_iterator itEnd =
                                                         vecRouteSegments.end();
        while (it != itEnd)
        {
            MHTRouteCandidate::RouteSegment* pSegment = *it;
            ++it;

            if (pSegment == NULL)
                continue;

            const shared_ptr<IMMNetworkSection>& pSection =
                                                         pSegment->GetSection();
            if (pSection == NULL)
                continue;

            const ONetworkSectionAdapter* pAdapter =
                                              pSection->CastToONetworkSection();
            if (pAdapter == NULL)
            {
                assert(false);
                continue;
            }

            const ONetworkEdge* pEdge = pAdapter->GetNetworkEdge();
            if (pEdge == NULL)
                continue;

            Tuple* pTuple = pEdge->GetTuple();
            if (pTuple == NULL)
                continue;

            m_pTupleBuffer->AppendTuple(pTuple);
        }
    }

    return true;
}


Tuple* OEdgeTupleStreamCreator::GetNextTuple(void) const
{
    if (m_pTupleBuffer == NULL)
        return NULL;

    if (m_pTupleIterator == NULL)
    {
        m_pTupleIterator = m_pTupleBuffer->MakeScan();
    }

    if (m_pTupleIterator != NULL && !m_pTupleIterator->EndOfScan())
    {
        return m_pTupleIterator->GetNextTuple();
    }
    else
    {
        return NULL;
    }
}



} // end of namespace mapmatch

