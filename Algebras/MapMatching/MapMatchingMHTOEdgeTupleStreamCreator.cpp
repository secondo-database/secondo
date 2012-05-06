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
#include <DateTime.h>


namespace mapmatch {

/*
3 class OEdgeTupleStreamCreator

*/

OEdgeTupleStreamCreator::OEdgeTupleStreamCreator(Supplier s,
                                                 const ONetworkAdapter& rNetw)
:m_pTupleType(NULL),
 m_pTupleBuffer(NULL),
 m_pTupleIterator(NULL),
 m_dNetworkScale(rNetw.GetNetworkScale())
{
    ListExpr tupleType = GetTupleResultType(s);
    m_pTupleType = new TupleType(nl->Second(tupleType));
}

OEdgeTupleStreamCreator::~OEdgeTupleStreamCreator()
{
    if (m_pTupleType != NULL)
        m_pTupleType->DeleteIfAllowed();
    m_pTupleType = NULL;

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

    const size_t nCandidates = rvecRouteCandidates.size();

    for (size_t i = 0; i < nCandidates; ++i)
    {
        MHTRouteCandidate* pCandidate = rvecRouteCandidates[i];
        if (pCandidate == NULL)
            continue;

        DateTime EndTimePrev((int64_t)0);

        const std::vector<MHTRouteCandidate::RouteSegment*>&
                              vecRouteSegments = pCandidate->GetRouteSegments();

        const size_t nRouteSegments = vecRouteSegments.size();
        for (size_t j = 0; j < nRouteSegments; ++j)
        {
            MHTRouteCandidate::RouteSegment* pSegment = vecRouteSegments[j];
            if (pSegment == NULL)
                continue;

            double dDistance = 0.0;
            const MHTRouteCandidate::PointData* pData =
                                    GetFirstPointOfNextSegment(vecRouteSegments,
                                                               j,
                                                               dDistance);
            EndTimePrev = ProcessSegment(*pSegment,
                                         EndTimePrev,
                                         pData,
                                         dDistance);
        }
    }

    return true;
}

const MHTRouteCandidate::PointData* OEdgeTupleStreamCreator::
                                                     GetFirstPointOfNextSegment(
              const std::vector<MHTRouteCandidate::RouteSegment*>& rvecSegments,
              size_t nIdx,
              double& dDistance)
{
    ++nIdx;
    dDistance = 0.0;

    const size_t nSegments = rvecSegments.size();
    while (nIdx < nSegments)
    {
        const MHTRouteCandidate::RouteSegment* pSegment = rvecSegments[nIdx];
        if (pSegment == NULL)
        {
            ++nIdx;
            continue;
        }

        const shared_ptr<IMMNetworkSection> pSect = pSegment->GetSection();
        if (pSect != NULL && pSect->GetCurve() != NULL)
        {
            if (pSegment->GetPoints().size() == 0)
            {
                dDistance += pSect->GetCurveLength(m_dNetworkScale);
            }
            else
            {
                const MHTRouteCandidate::PointData* pData =
                                              pSegment->GetPoints().front();
                if (pData != NULL && pData->GetPointProjection() != NULL)
                {
                    const SimpleLine* pCurve = pSect->GetCurve();
                    const Point& rPtStart = pSect->GetStartPoint();
                    const Point* pPt = pData->GetPointProjection();

                    const double dDistStart2FirstPt =
                                          MMUtil::CalcDistance(rPtStart,
                                                               *pPt,
                                                               *pCurve,
                                                               m_dNetworkScale);

                    dDistance += dDistStart2FirstPt;
                    return pData;
                }
                else
                {
                    assert(false);
                }
            }
        }

        ++nIdx;
    }

    return NULL;
}

DateTime OEdgeTupleStreamCreator::ProcessSegment(
               const MHTRouteCandidate::RouteSegment& rSegment,
               const DateTime& rEndTimePrevSegment,
               const MHTRouteCandidate::PointData* pFirstPointofNextSeg,
               double dDistance) // Distance to first point of next segment
{
    if (rSegment.IsOffRoad()) // No edge
        return DateTime((int64_t)0);
    else
    {
        const shared_ptr<IMMNetworkSection> pSection = rSegment.GetSection();
        if (pSection == NULL)
        {
            assert(false);
            return DateTime((int64_t)0);
        }

        DateTime TimeUndef((int64_t)0);
        TimeUndef.SetDefined(false);

        if (rSegment.GetPoints().size() == 0)
        {
            // No assigned points

            if (!rEndTimePrevSegment.IsDefined() ||
                rEndTimePrevSegment.IsZero())
            {
                CreateTuple(rSegment, TimeUndef, TimeUndef);
                return TimeUndef;
            }

            if (pFirstPointofNextSeg == NULL ||
                !pFirstPointofNextSeg->GetTime().IsDefined())
            {
                CreateTuple(rSegment, rEndTimePrevSegment, rEndTimePrevSegment);
                return rEndTimePrevSegment;
            }

            const double dLenCurve = pSection->GetCurveLength(m_dNetworkScale);

            double dDistStartPt2FirstPtOfNextSeg = dLenCurve + dDistance;
            if (AlmostEqual(dDistStartPt2FirstPtOfNextSeg, 0.0))
            {
                CreateTuple(rSegment, rEndTimePrevSegment, rEndTimePrevSegment);
                return rEndTimePrevSegment;
            }

            DateTime TimeStart = rEndTimePrevSegment;
            DateTime Duration = pFirstPointofNextSeg->GetTime() - TimeStart;
            DateTime TimeEnd = TimeStart +
                    DateTime(datetime::durationtype,
                             (uint64_t)((Duration.millisecondsToNull()
                              / dDistStartPt2FirstPtOfNextSeg) * dLenCurve));
            CreateTuple(rSegment, TimeStart, TimeEnd);
            return TimeEnd;
        }
        else
        {
            // points assigned

            DateTime TimeStart = rEndTimePrevSegment;

            if (!TimeStart.IsDefined() || TimeStart.IsZero())
            {
                const MHTRouteCandidate::PointData* pData =
                                                   rSegment.GetPoints().front();
                if (pData != NULL)
                {
                    TimeStart = pData->GetTime();
                }
                else
                {
                    assert(false);
                }
            }

            const SimpleLine* pCurve = pSection->GetCurve();
            const MHTRouteCandidate::PointData* pData =
                                                    rSegment.GetPoints().back();
            if (pCurve == NULL ||
                pData == NULL || pData->GetPointProjection() == NULL)
            {
                assert(false);
                CreateTuple(rSegment, TimeStart, TimeStart);
                return TimeStart;
            }

            if (pFirstPointofNextSeg == NULL ||
                !pFirstPointofNextSeg->GetTime().IsDefined())
            {
                // this is the last point
                DateTime TimeEnd = pData->GetTime();
                CreateTuple(rSegment, TimeStart, TimeEnd);
                return TimeEnd;
            }

            const Point* pPtProjection = pData->GetPointProjection();

            const double dDistLastPt2End = MMUtil::CalcDistance(
                                                    *pPtProjection,
                                                    rSegment.HasUTurn() ?
                                                      pSection->GetStartPoint():
                                                      pSection->GetEndPoint(),
                                                    *pCurve,
                                                    m_dNetworkScale);

            double dDistLastPt2FirstPtOfNextSeg = dDistLastPt2End + dDistance;
            if (AlmostEqual(dDistLastPt2FirstPtOfNextSeg, 0.0))
            {
                CreateTuple(rSegment, TimeStart, TimeStart);
                return TimeStart;
            }

            DateTime Duration = pFirstPointofNextSeg->GetTime() -
                                                               pData->GetTime();
            DateTime TimeEnd = pData->GetTime() +
                    DateTime(datetime::durationtype,
                             (uint64_t)((Duration.millisecondsToNull()
                              / dDistLastPt2FirstPtOfNextSeg) *
                                                              dDistLastPt2End));
            CreateTuple(rSegment, TimeStart, TimeEnd);
            return TimeEnd;
        }
    }
}

void OEdgeTupleStreamCreator::CreateTuple(
                                const MHTRouteCandidate::RouteSegment& rSegment,
                                const DateTime& rTimeStart,
                                const DateTime& rTimeEnd)
{
    const shared_ptr<IMMNetworkSection> pSection = rSegment.GetSection();
    if (pSection == NULL)
    {
        // Offroad
        return;
    }

    const ONetworkSectionAdapter* pAdapter = pSection->CastToONetworkSection();
    if (pAdapter == NULL)
    {
        assert(false);
        return;
    }

    const ONetworkEdge* pEdge = pAdapter->GetNetworkEdge();
    if (pEdge == NULL)
    {
        assert(false);
        return;
    }

    const Tuple* pActTuple = pEdge->GetTuple();
    if (pActTuple == NULL)
    {
        assert(false);
        return;
    }

    // Create new Tuple and copy attributes
    Tuple* pResultTuple = new Tuple(m_pTupleType);
    int i = 0;
    while (i < pActTuple->GetNoAttributes())
    {
        pResultTuple->CopyAttribute(i, pActTuple, i);
        ++i;
    }

    // Create new attributes
    DateTime* pTimeStart = new DateTime(rTimeStart);
    pResultTuple->PutAttribute(i, pTimeStart);
    ++i;

    DateTime* pTimeEnd = new DateTime(rTimeEnd);
    pResultTuple->PutAttribute(i, pTimeEnd);

    // Add new tuple to buffer
    m_pTupleBuffer->AppendTuple(pResultTuple);

    pResultTuple->DeleteIfAllowed();
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

