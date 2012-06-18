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
        MHTRouteCandidate::PointDataPtr pDataEnd =
                                              MHTRouteCandidate::PointDataPtr();

        MHTRouteCandidate::RouteSegmentIterator it =
                                                pCandidate->RouteSegmentBegin();
        MHTRouteCandidate::RouteSegmentIterator itEnd =
                                                pCandidate->RouteSegmentEnd();


        for (/*empty*/; it != itEnd; ++it)
        {
            const MHTRouteCandidate::RouteSegmentPtr& pSegment = *it;
            if (pSegment == NULL)
                continue;

            double dDistance = 0.0;
            const MHTRouteCandidate::PointDataPtr& pData =
                                    GetFirstPointOfNextSegment(it,
                                                               itEnd,
                                                               dDistance);
            EndTimePrev = ProcessSegment(*pSegment,
                                         EndTimePrev,
                                         pDataEnd,
                                         pData,
                                         dDistance);

            if (pSegment->GetPoints().size() > 0)
                pDataEnd = pSegment->GetPoints().back();
            else
                pDataEnd = MHTRouteCandidate::PointDataPtr();
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

const MHTRouteCandidate::PointDataPtr OEdgeTupleStreamCreator::
                                                     GetFirstPointOfNextSegment(
            MHTRouteCandidate::RouteSegmentIterator itSegments,
            MHTRouteCandidate::RouteSegmentIterator itSegmentsEnd,
            double& dDistance)
{
    ++itSegments;
    dDistance = 0.0;

    while (itSegments != itSegmentsEnd)
    {
        const MHTRouteCandidate::RouteSegmentPtr& pSegment = *itSegments;
        if (pSegment == NULL)
        {
            ++itSegments;
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
                const MHTRouteCandidate::PointDataPtr pData =
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

        ++itSegments;
    }

    return MHTRouteCandidate::PointDataPtr();
}

DateTime OEdgeTupleStreamCreator::ProcessSegment(
               const MHTRouteCandidate::RouteSegment& rSegment,
               const DateTime& rEndTimePrevSegment,
               const MHTRouteCandidate::PointDataPtr& pPrevPointData,
               const MHTRouteCandidate::PointDataPtr& pFirstPointofNextSeg,
               double dDistance) // Distance to first point of next segment
{
    if (m_eMode == MODE_EDGES)
    {
        return ProcessSegment_Edges(rSegment,
                                    rEndTimePrevSegment,
                                    pPrevPointData,
                                    pFirstPointofNextSeg,
                                    dDistance);
    }
    else if (m_eMode == MODE_EDGES_AND_POSITIONS)
    {
        return ProcessSegment_EdgesAndPositions(rSegment,
                                                rEndTimePrevSegment,
                                                pPrevPointData,
                                                pFirstPointofNextSeg,
                                                dDistance);
    }
    else
    {
        assert(false);
        DateTime TimeUndef((int64_t)0);
        TimeUndef.SetDefined(false);
        return TimeUndef;
    }
}

static Point GetLastProjectedPoint(
                const std::vector<MHTRouteCandidate::PointDataPtr>& rvecPts,
                DateTime& rTimeLastPt)
{
    std::vector<MHTRouteCandidate::PointDataPtr>::const_reverse_iterator
                                                         itEnd = rvecPts.rend();
    std::vector<MHTRouteCandidate::PointDataPtr>::const_reverse_iterator it;

    Point PtRes(false);

    for (it = rvecPts.rbegin(); it != itEnd && !PtRes.IsDefined(); ++it)
    {
        const MHTRouteCandidate::PointDataPtr pData = *it;
        if (pData != NULL)
        {
            const Point* pPtProj = pData->GetPointProjection();
            if (pPtProj != NULL && pPtProj->IsDefined())
            {
                PtRes = *pPtProj;
                rTimeLastPt = pData->GetTime();
            }
        }
    }

    return PtRes;
}

DateTime OEdgeTupleStreamCreator::ProcessSegment_Edges(
                    const MHTRouteCandidate::RouteSegment& rSegment,
                    const DateTime& rEndTimePrevSegment,
                    const MHTRouteCandidate::PointDataPtr& pPrevPointData,
                    const MHTRouteCandidate::PointDataPtr& pFirstPointofNextSeg,
                    double dDistance) // Distance to first point of next segment
{
    const shared_ptr<IMMNetworkSection> pSection = rSegment.GetSection();
    if (pSection == NULL)
        return DateTime((int64_t)0);

    const std::vector<MHTRouteCandidate::PointDataPtr>& vecPoints =
                                                           rSegment.GetPoints();
    const size_t nPoints = vecPoints.size();

    const SimpleLine* pCurve = pSection->GetCurve();

    if (rSegment.IsOffRoad() || pCurve == NULL) // Offroad
    {
        return DateTime((int64_t)0);
    }

    // Get first point
    Point PtStart = pSection->GetStartPoint();
    DateTime TimeStart = rEndTimePrevSegment;

    if (!TimeStart.IsDefined() || TimeStart.IsZero())
    {
        // first segment -> Get first  projected point
        for (size_t i = 0; i < nPoints; ++i)
        {
            const MHTRouteCandidate::PointDataPtr& pData = vecPoints[i];
            if (pData != NULL)
            {
                const Point* pPtProj = pData->GetPointProjection();
                if (pPtProj != NULL && pPtProj->IsDefined())
                {
                    PtStart = *pPtProj;
                    TimeStart = pData->GetTime();
                    break;
                }
            }
        }

        if (!TimeStart.IsDefined() || TimeStart.IsZero())
            return DateTime((int64_t)0);; // first segment and no point
    }

    // get last point
    DateTime TimeLastPt((int64_t) 0);
    TimeLastPt.SetDefined(false);

    Point PtLast(GetLastProjectedPoint(vecPoints, TimeLastPt));

    // Process endpoint of segment

    if (pFirstPointofNextSeg == NULL ||
        !pFirstPointofNextSeg->GetTime().IsDefined() ||
        pFirstPointofNextSeg->GetPointProjection() == NULL ||
        !pFirstPointofNextSeg->GetPointProjection()->IsDefined())
    {
        // this is the last point or the next segment is offroad

        if (!PtLast.IsDefined())
        {
            PtLast = PtStart;
            TimeLastPt = TimeStart;
        }

        ProcessPoints(rSegment, TimeStart, TimeLastPt, PtStart, PtLast);
        return TimeLastPt;
    }
    else
    {
        Point PtEnd = rSegment.HasUTurn() ? pSection->GetStartPoint():
                                            pSection->GetEndPoint();

        // Calculate distance to endpoint of section
        double dDist2End = 0.0;
        DateTime Duration((int64_t)0); // Time to first point of next segment

        DateTime TimeEnd((int64_t)0);

        if (!PtLast.IsDefined())
        {
            dDist2End = pSection->GetCurveLength(m_dNetworkScale);
            Duration = pFirstPointofNextSeg->GetTime() - TimeStart;
            TimeEnd = TimeStart;
        }
        else
        {
            dDist2End = MMUtil::CalcDistance(PtLast,
                                             PtEnd,
                                             *pCurve,
                                             m_dNetworkScale);
            Duration = pFirstPointofNextSeg->GetTime() - TimeLastPt;
            TimeEnd = TimeLastPt;
        }

        double dDist2FirstPtOfNextSeg = dDist2End + dDistance;

        if (!AlmostEqual(dDist2FirstPtOfNextSeg, 0.0))
        {
            TimeEnd += DateTime(datetime::durationtype,
                             (uint64_t)((Duration.millisecondsToNull()
                                        / dDist2FirstPtOfNextSeg) * dDist2End));

            if (!MMUtil::CheckSpeed(dDist2FirstPtOfNextSeg,
                                    TimeStart, TimeEnd, PtStart, PtEnd))
            {
                return TimeEnd;
            }
        }

        ProcessPoints(rSegment, TimeStart, TimeEnd, PtStart, PtEnd);

        return TimeEnd;
    }
}

DateTime OEdgeTupleStreamCreator::ProcessSegment_EdgesAndPositions(
                    const MHTRouteCandidate::RouteSegment& rSegment,
                    const DateTime& rEndTimePrevSegment,
                    const MHTRouteCandidate::PointDataPtr& pPrevPointData,
                    const MHTRouteCandidate::PointDataPtr& pFirstPointofNextSeg,
                    double dDistance) // Distance to first point of next segment
{
    const shared_ptr<IMMNetworkSection> pSection = rSegment.GetSection();
    const std::vector<MHTRouteCandidate::PointDataPtr>& vecPoints =
                                                           rSegment.GetPoints();
    const size_t nPoints = vecPoints.size();

    DateTime TimeStart = rEndTimePrevSegment;
    Point PtStart(false);

    const SimpleLine* pCurve = pSection != NULL ? pSection->GetCurve() : NULL;
    bool bStartsSmaller = pSection != NULL ? pSection->GetCurveStartsSmaller() :
                                             false;

    if (!TimeStart.IsDefined() || TimeStart.IsZero())
    {
        // first segment
        MHTRouteCandidate::PointDataPtr pData = (nPoints > 0) ?
                                                  vecPoints.front() :
                                              MHTRouteCandidate::PointDataPtr();

        if (pData != NULL)
        {
            TimeStart = pData->GetTime();

            const Point* pPtProj = pData->GetPointProjection();
            if (pPtProj != NULL && pPtProj->IsDefined())
               PtStart = *pPtProj;
            else
               PtStart = pData->GetPointGPS();
        }
        else
        {
            return DateTime((int64_t)0); // first segment and no point
        }
    }
    else
    {
        if (pCurve == NULL)
        {
            // this segment is offroad
            if (pPrevPointData == NULL)
            {
                MHTRouteCandidate::PointDataPtr pData = (nPoints > 0) ?
                                                          vecPoints.front() :
                                              MHTRouteCandidate::PointDataPtr();

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
                    return DateTime((int64_t)0);
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

    // Process all points

    Point PtEnd(false);
    DateTime TimeEnd((int64_t)0);

    for (size_t i = 0; i < nPoints; ++i)
    {
        const MHTRouteCandidate::PointDataPtr& pData = vecPoints[i];
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

    // Process endpoint of segment

    if (pFirstPointofNextSeg == NULL ||
        !pFirstPointofNextSeg->GetTime().IsDefined() ||
        pFirstPointofNextSeg->GetPointProjection() == NULL ||
        !pFirstPointofNextSeg->GetPointProjection()->IsDefined())
    {
        // this is the last point or the next segment is offroad
        return TimeStart;
    }
    else if (pSection != NULL && pCurve != NULL)
    {
        // Calculate distance to endpoint of section

        Point PtEndSegment = rSegment.HasUTurn() ? pSection->GetStartPoint():
                                                   pSection->GetEndPoint();

        const double dDistLastPt2End = MMUtil::CalcDistance(PtStart,
                                                            PtEndSegment,
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

        if (TimeStart != TimeEnd && !AlmostEqual(PtStart, PtEndSegment))
        {
            if (MMUtil::CheckSpeed(dDistLastPt2End, TimeStart, TimeEnd,
                                   PtStart, PtEnd,
                                   rSegment.GetSection()->GetRoadType(),
                                   rSegment.GetSection()->GetMaxSpeed()))
            {
                ProcessPoints(rSegment,
                              TimeStart, TimeEnd,
                              PtStart, PtEndSegment);
            }
        }

        return TimeEnd;
    }
    else
    {
        MHTRouteCandidate::PointDataPtr pData = (nPoints > 0) ?
                                                  vecPoints.back() :
                                              MHTRouteCandidate::PointDataPtr();
        if (pData != NULL)
            return pData->GetTime();
        else
            return DateTime((int64_t)0);

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
                const bool bStartsSmaller = pSection->GetCurveStartsSmaller();

                const Point& rPtStartCurve = pCurve->StartPoint(bStartsSmaller);
                const Point& rPtEndCurve = pCurve->EndPoint(bStartsSmaller);

                // calculate PosStart
                if (AlmostEqual(rPtStart, rPtStartCurve) ||
                    AlmostEqual(rPtStart, rPtEndCurve))
                {
                    pCurve->AtPoint(rPtStart, bStartsSmaller, 0.0, dPosStart);
                }

                if (dPosStart < 0.0)
                {
                    MMUtil::GetPosOnSimpleLine(*pCurve,
                                               rPtStart,
                                               bStartsSmaller,
                                               m_dNetworkScale,
                                               dPosStart);
                }

                // calculate PosEnd
                if (AlmostEqual(rPtEnd, rPtStartCurve) ||
                    AlmostEqual(rPtEnd, rPtEndCurve))
                {
                    pCurve->AtPoint(rPtEnd, bStartsSmaller, 0.0, dPosEnd);
                }

                if (dPosEnd < 0.0)
                {
                    MMUtil::GetPosOnSimpleLine(*pCurve,
                                               rPtEnd,
                                               bStartsSmaller,
                                               m_dNetworkScale,
                                               dPosEnd);
                }
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

