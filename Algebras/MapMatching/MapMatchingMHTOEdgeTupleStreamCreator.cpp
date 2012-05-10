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
                                                 const ONetworkAdapter& rNetw,
                                                 EMode eMode)
:m_eMode(eMode),
 m_pTupleType(NULL),
 m_pTupleBuffer(NULL),
 m_pTupleIterator(NULL),
 m_pTupleUndefEdge(NULL),
 m_dNetworkScale(rNetw.GetNetworkScale())
{
    ListExpr tupleType = GetTupleResultType(s);
    m_pTupleType = new TupleType(nl->Second(tupleType));

    m_pTupleUndefEdge = rNetw.GetUndefEdgeTuple();
}

OEdgeTupleStreamCreator::~OEdgeTupleStreamCreator()
{
    if (m_pTupleType != NULL)
        m_pTupleType->DeleteIfAllowed();
    m_pTupleType = NULL;

    if (m_pTupleUndefEdge != NULL)
        m_pTupleUndefEdge->DeleteIfAllowed();
    m_pTupleUndefEdge = NULL;

    delete m_pTupleIterator;
    m_pTupleIterator = NULL;

    delete m_pTupleBuffer;
    m_pTupleBuffer = NULL;
}

bool OEdgeTupleStreamCreator::CreateResult(
                     const std::vector<MHTRouteCandidate*>& rvecRouteCandidates)
{
    Init();

    const size_t nCandidates = rvecRouteCandidates.size();

    for (size_t i = 0; i < nCandidates; ++i)
    {
        MHTRouteCandidate* pCandidate = rvecRouteCandidates[i];
        if (pCandidate == NULL)
            continue;

        DateTime EndTimePrev((int64_t)0);
        const MHTRouteCandidate::PointData* pDataEnd = NULL;

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
                                         pDataEnd,
                                         pData,
                                         dDistance);

            if (pSegment->GetPoints().size() > 0)
                pDataEnd = pSegment->GetPoints().back();
            else
                pDataEnd = NULL;
        }
    }

    return true;
}

void OEdgeTupleStreamCreator::Init(void)
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
               const MHTRouteCandidate::PointData* pPrevPointData,
               const MHTRouteCandidate::PointData* pFirstPointofNextSeg,
               double dDistance) // Distance to first point of next segment
{
    const shared_ptr<IMMNetworkSection> pSection = rSegment.GetSection();

    DateTime TimeUndef((int64_t)0);
    TimeUndef.SetDefined(false);

    const std::vector<MHTRouteCandidate::PointData*>& vecPoints =
                                                           rSegment.GetPoints();
    const size_t nPoints = vecPoints.size();

    DateTime TimeStart = rEndTimePrevSegment;
    Point PtStart(false);

    const SimpleLine* pCurve = pSection != NULL ? pSection->GetCurve() : NULL;
    bool bStartsSmaller = pSection != NULL ? pSection->GetCurveStartsSmaller() :
                                             false;

    if (pCurve == NULL && m_eMode == MODE_EDGES) // Offroad
        return TimeUndef;

    if (!TimeStart.IsDefined() || TimeStart.IsZero())
    {
        // first segment
        const MHTRouteCandidate::PointData* pData = nPoints > 0 ?
                                                       vecPoints.front() : NULL;
        if (pData != NULL)
        {
            TimeStart = pData->GetTime();

            if (m_eMode == MODE_EDGES_AND_POSITIONS)
            {
                const Point* pPtProj = pData->GetPointProjection();
                if (pPtProj != NULL && pPtProj->IsDefined())
                    PtStart = *pPtProj;
                else
                    PtStart = pData->GetPointGPS();
            }
        }
        else
        {
            return TimeUndef; // first segment and no point
        }
    }
    else
    {
        if (m_eMode == MODE_EDGES_AND_POSITIONS)
        {
            if (pCurve == NULL)
            {
                // this segment is offroad
                if (pPrevPointData == NULL)
                {
                    const MHTRouteCandidate::PointData* pData =
                                         nPoints > 0 ? vecPoints.front() : NULL;
                    if (pData != NULL)
                    {
                        const Point* pPtProj = pData->GetPointProjection();
                        if (pPtProj != NULL && pPtProj->IsDefined())
                            PtStart = *pPtProj;
                        else
                            PtStart = pData->GetPointGPS();
                    }
                    else
                    {
                        return TimeUndef;
                    }
                }
                else
                {
                    PtStart = *(pPrevPointData->GetPointProjection());
                    if (!PtStart.IsDefined())
                        PtStart = pPrevPointData->GetPointGPS();

                }
                const Point* pPtProj = pPrevPointData->GetPointProjection();
                if (pPtProj != NULL)
                    PtStart = *pPtProj;
            }
            else
            {
                if (pPrevPointData == NULL ||
                    pPrevPointData->GetPointProjection() != NULL) // onroad
                {
                    PtStart = pCurve->StartPoint(bStartsSmaller);
                }
                else
                    PtStart = pPrevPointData->GetPointGPS();
            }
        }
    }

    // Process all points
    if (m_eMode == MODE_EDGES_AND_POSITIONS)
    {
        Point PtEnd(false);
        DateTime TimeEnd((int64_t)0);

        for (size_t i = 0; i < nPoints; ++i)
        {
            MHTRouteCandidate::PointData* pData = vecPoints[i];
            if (pData == NULL)
                continue;

            TimeEnd = pData->GetTime();

            const Point* pPtProj = pData->GetPointProjection();
            if (pPtProj != NULL && pPtProj->IsDefined())
            {
                PtEnd = *pPtProj;
            }
            else
            {
                PtEnd = pData->GetPointGPS();
            }

            if (TimeStart == TimeEnd && AlmostEqual(PtStart, PtEnd))
                continue;

            ProcessPoints(rSegment, TimeStart, TimeEnd, PtStart, PtEnd);

            PtStart = PtEnd;
            TimeStart = TimeEnd;
        }
    }

    // Process endpoint of segment

    if (pFirstPointofNextSeg == NULL ||
        !pFirstPointofNextSeg->GetTime().IsDefined() ||
        pFirstPointofNextSeg->GetPointProjection() == NULL ||
        !pFirstPointofNextSeg->GetPointProjection()->IsDefined())
    {
        // this is the last point or the next segment is offroad
        if (m_eMode == MODE_EDGES_AND_POSITIONS)
        {
            return TimeStart;
        }
        else if (m_eMode == MODE_EDGES)
        {
            const MHTRouteCandidate::PointData* pData = nPoints > 0 ?
                                                        vecPoints.back() : NULL;

            DateTime TimeEnd = pData != NULL ? pData->GetTime() : TimeStart;
            ProcessPoints(rSegment, TimeStart, TimeEnd,
                          Point(false), Point(false));
            return TimeEnd;
        }
        else
        {
            assert(false);
            return TimeUndef;
        }
    }
    else if (pSection != NULL && pCurve != NULL)
    {
        // Calculate distance to endpoint of section

        Point PtEnd = rSegment.HasUTurn() ? pSection->GetStartPoint():
                                            pSection->GetEndPoint();

        const double dDistLastPt2End = MMUtil::CalcDistance(PtStart,
                                                            PtEnd,
                                                            *pCurve,
                                                            m_dNetworkScale);

        double dDistLastPt2FirstPtOfNextSeg = dDistLastPt2End + dDistance;
        if (AlmostEqual(dDistLastPt2FirstPtOfNextSeg, 0.0))
        {
            return TimeStart;
        }

        DateTime Duration = pFirstPointofNextSeg->GetTime() - TimeStart;
        DateTime TimeEnd = TimeStart +
                    DateTime(datetime::durationtype,
                             (uint64_t)((Duration.millisecondsToNull()
                              / dDistLastPt2FirstPtOfNextSeg) *
                                                              dDistLastPt2End));

        ProcessPoints(rSegment, TimeStart, TimeEnd, PtStart, PtEnd);

        return TimeEnd;
    }
    else
    {
        const MHTRouteCandidate::PointData* pData = nPoints > 0 ?
                                                        vecPoints.back() : NULL;
        if (pData != NULL)
            return pData->GetTime();
        else
            return TimeUndef;

    }
}

void OEdgeTupleStreamCreator::ProcessPoints(
                                const MHTRouteCandidate::RouteSegment& rSegment,
                                const DateTime& rTimeStart,
                                const DateTime& rTimeEnd,
                                const Point& rPtStart,
                                const Point& rPtEnd)
{
    if (m_eMode == MODE_EDGES)
    {
        CreateTuple(rSegment, rTimeStart, rTimeEnd);
    }
    else if (m_eMode == MODE_EDGES_AND_POSITIONS)
    {
        double dPosStart = -1.0;
        double dPosEnd = -1.0;

        const shared_ptr<IMMNetworkSection>& pSection = rSegment.GetSection();
        if (pSection != NULL && pSection->IsDefined())
        {
            const SimpleLine* pCurve = pSection->GetCurve();
            if (pCurve != NULL && !pCurve->IsEmpty())
            {
                bool bStartsSmaller = pSection->GetCurveStartsSmaller();

                if (rPtStart.IsDefined())
                    pCurve->AtPoint(rPtStart, bStartsSmaller, 0.0, dPosStart);

                if (rPtEnd.IsDefined())
                    pCurve->AtPoint(rPtEnd, bStartsSmaller, 0.0, dPosEnd);
            }
        }

        CreateTuple(rSegment,
                    rTimeStart, rTimeEnd,
                    rPtStart, rPtEnd,
                    dPosStart, dPosEnd);
    }
    else
    {
        assert(false);
    }
}

const Tuple* OEdgeTupleStreamCreator::GetEdgeTuple(
                                const MHTRouteCandidate::RouteSegment& rSegment)
{
    const shared_ptr<IMMNetworkSection> pSection = rSegment.GetSection();
    if (pSection == NULL)
    {
        return m_pTupleUndefEdge;
    }

    const ONetworkSectionAdapter* pAdapter = pSection->CastToONetworkSection();
    if (pAdapter == NULL)
    {
        assert(false); // only for ONetwork
        return NULL;
    }

    const ONetworkEdge* pEdge = pAdapter->GetNetworkEdge();
    if (pEdge == NULL)
    {
        assert(false);
        return NULL;
    }

    return pEdge->GetTuple();
}

void OEdgeTupleStreamCreator::CreateTuple(
                                const MHTRouteCandidate::RouteSegment& rSegment,
                                const DateTime& rTimeStart,
                                const DateTime& rTimeEnd)
{
    const Tuple* pTupleEdge = GetEdgeTuple(rSegment);
    if (pTupleEdge == NULL)
    {
        return;
    }

    // Create new Tuple and copy attributes
    Tuple* pResultTuple = new Tuple(m_pTupleType);
    int i = 0;
    while (i < pTupleEdge->GetNoAttributes())
    {
        pResultTuple->CopyAttribute(i, pTupleEdge, i);
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

void OEdgeTupleStreamCreator::CreateTuple(
                                const MHTRouteCandidate::RouteSegment& rSegment,
                                const DateTime& rTimeStart,
                                const DateTime& rTimeEnd,
                                const Point& rPtStart,
                                const Point& rPtEnd,
                                const double dPosStart,
                                const double dPosEnd)
{
    const Tuple* pTupleEdge = GetEdgeTuple(rSegment);
    if (pTupleEdge == NULL)
    {
        //assert(false);
        return;
    }

    // Create new Tuple and copy attributes
    Tuple* pResultTuple = new Tuple(m_pTupleType);
    int i = 0;
    while (i < pTupleEdge->GetNoAttributes())
    {
        pResultTuple->CopyAttribute(i, pTupleEdge, i);
        ++i;
    }

    // Create new attributes
    DateTime* pTimeStart = new DateTime(rTimeStart);
    pResultTuple->PutAttribute(i, pTimeStart);
    ++i;

    DateTime* pTimeEnd = new DateTime(rTimeEnd);
    pResultTuple->PutAttribute(i, pTimeEnd);
    ++i;

    Point* pPtStart = new Point(rPtStart);
    pResultTuple->PutAttribute(i, pPtStart);
    ++i;

    Point* pPtEnd = new Point(rPtEnd);
    pResultTuple->PutAttribute(i, pPtEnd);
    ++i;

    CcReal* pPosStart = NULL;
    if (dPosStart < 0.0)
        pPosStart = new CcReal(false);
    else
        pPosStart = new CcReal(true, dPosStart);

    pResultTuple->PutAttribute(i, pPosStart);
    ++i;

    CcReal* pPosEnd = NULL;
    if (dPosEnd < 0.0)
        pPosEnd = new CcReal(false);
    else
        pPosEnd = new CcReal(true, dPosEnd);

    pResultTuple->PutAttribute(i, pPosEnd);

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

