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

[1] Implementation File containing class ~MPointCreator~

April, 2012. Matthias Roth

[TOC]

1 Overview

This implementation file contains the implementation of
the class ~MPointCreator~.

2 Defines and includes

*/

#include "MapMatchingMHTMPointCreator.h"
#include "MapMatchingUtil.h"
#include "NetworkAdapter.h"
#include <TemporalAlgebra.h>

using datetime::DateTime;


namespace mapmatch {

/*
3 class MPointCreator

*/

MPointCreator::MPointCreator(MPoint* pResMPoint,
                             double dNetworkScale)
:m_pResMPoint(pResMPoint),
 m_dNetworkScale(dNetworkScale)
{
}

MPointCreator::~MPointCreator()
{
    m_pResMPoint = NULL;
}

bool MPointCreator::CreateResult(const std::vector<MHTRouteCandidate*>&
                                                            rvecRouteCandidates)
{
    if (!Init())
        return false;

    const size_t nCandidates = rvecRouteCandidates.size();
    for (size_t i = 0; i < nCandidates; ++i)
    {
        MHTRouteCandidate* pCandidate = rvecRouteCandidates[i];
        if (pCandidate == NULL)
            continue;

        std::vector<const SimpleLine*> vecCurvesBetweenPoints;

        MHTRouteCandidate::RouteSegmentIterator it =
                                                pCandidate->RouteSegmentBegin();
        MHTRouteCandidate::RouteSegmentIterator itEnd =
                                                  pCandidate->RouteSegmentEnd();

        MHTRouteCandidate::PointDataPtr pData1 =
                                              MHTRouteCandidate::PointDataPtr();
        MHTRouteCandidate::RouteSegmentPtr pSegment1 =
                                           MHTRouteCandidate::RouteSegmentPtr();

        while (it != itEnd)
        {
            const MHTRouteCandidate::RouteSegmentPtr& pSegment = *it;
            ++it;
            if (pSegment == NULL)
                continue;

            const std::vector<MHTRouteCandidate::PointDataPtr>&
                                              vecPoints = pSegment->GetPoints();

            if (vecPoints.size() == 0)
            {
                const shared_ptr<IMMNetworkSection>& pSection =
                                                         pSegment->GetSection();
                if (pSection != NULL && pSection->IsDefined())
                    vecCurvesBetweenPoints.push_back(pSection->GetCurve());
            }
            else
            {
                std::vector<MHTRouteCandidate::PointDataPtr>::const_iterator
                                                      itPts = vecPoints.begin();
                std::vector<MHTRouteCandidate::PointDataPtr>::const_iterator
                                                     itPtsEnd = vecPoints.end();

                while (itPts != itPtsEnd && pData1 == NULL)
                {
                    pData1 = *itPts;
                    pSegment1 = pSegment;
                    ++itPts;
                }

                while (itPts != itPtsEnd &&
                       pData1 != NULL)
                {
                    MHTRouteCandidate::PointDataPtr pData2 =
                                              MHTRouteCandidate::PointDataPtr();
                    MHTRouteCandidate::RouteSegmentPtr pSegment2 =
                                           MHTRouteCandidate::RouteSegmentPtr();

                    while (itPts != itPtsEnd && pData2 == NULL)
                    {
                        pData2 = *itPts;
                        pSegment2 = pSegment;
                        ++itPts;
                    }

                    if (pData2 == NULL)
                        continue;

                    ProcessPoints(*pData1,
                                  *pSegment1,
                                  *pData2,
                                  vecCurvesBetweenPoints);

                    vecCurvesBetweenPoints.clear();

                    pData1 = pData2;
                    pSegment1 = pSegment2;
                }
            }
        }
    }

    Finalize();

    return true;
}

bool MPointCreator::Init(void)
{
    if (m_pResMPoint == NULL)
        return false;
    else
    {
        m_pResMPoint->Clear();
        m_pResMPoint->SetDefined(true); // always defined
        m_pResMPoint->StartBulkLoad();

        return true;
    }
}

void MPointCreator::Finalize(void)
{
    if (m_pResMPoint != NULL)
    {
        m_pResMPoint->EndBulkLoad(false);
        m_pResMPoint->SetDefined(true); // always defined
    }
}

void MPointCreator::ProcessPoints(
                         const MHTRouteCandidate::PointData& rData1,
                         const MHTRouteCandidate::RouteSegment& rSegment1,
                         const MHTRouteCandidate::PointData& rData2,
                         std::vector<const SimpleLine*>& vecCurvesBetweenPoints)
{
    Point Pt1(false);
    if (rData1.GetPointProjection() != NULL)
        Pt1 = *(rData1.GetPointProjection());
    else
        Pt1 = rData1.GetPointGPS();

    Point Pt2(false);
    if (rData2.GetPointProjection() != NULL)
        Pt2 = *(rData2.GetPointProjection());
    else
        Pt2 = rData2.GetPointGPS();

    if (AlmostEqual(Pt1, Pt2))
    {
        Interval<Instant> TimeInterval(rData1.GetTime(),
                                       rData2.GetTime(),
                                       true,
                                       rData1.GetTime() == rData2.GetTime());

        AttributePtr<UPoint> pUPoint(new UPoint(TimeInterval, Pt1, Pt1));
        m_pResMPoint->Add(*pUPoint);

        assert(vecCurvesBetweenPoints.size() == 0);
    }
    else
    {
        const shared_ptr<IMMNetworkSection>& pSection1 = rData1.GetSection();
        const shared_ptr<IMMNetworkSection>& pSection2 = rData2.GetSection();

        if (pSection1 == NULL || !pSection1->IsDefined() ||
            pSection2 == NULL || !pSection2->IsDefined())
        {
            // at least one point is offroad

            Interval<Instant> TimeInterval(
                                          rData1.GetTime(),
                                          rData2.GetTime(),
                                          true,
                                          rData1.GetTime() == rData2.GetTime());

            AttributePtr<UPoint> pUPoint(new UPoint(TimeInterval, Pt1, Pt2));
            m_pResMPoint->Add(*pUPoint);
        }
        else if (pSection1 == pSection2) // Same section
        {
            const SimpleLine* pSectionCurve = pSection1->GetCurve();

            AttributePtr<SimpleLine> pSubline(new SimpleLine(0));
            MMUtil::SubLine(pSectionCurve,
                            Pt1,
                            Pt2,
                            pSection1->GetCurveStartsSmaller(),
                            m_dNetworkScale,
                            *pSubline);

            if (pSubline->IsDefined() &&
                pSubline->StartPoint().IsDefined() &&
                pSubline->EndPoint().IsDefined())
            {
                Interval<Instant> TimeInterval(
                                          rData1.GetTime(),
                                          rData2.GetTime(),
                                          true,
                                          rData1.GetTime() == rData2.GetTime());

                //assert(AlmostEqual(Pt1, pSubline->StartPoint()));

                ProcessCurve(*pSubline, TimeInterval);
            }
        }
        else // different sections
        {
            // Calculate total length of curves

            const SimpleLine* pSection1Curve = pSection1->GetCurve();

            AttributePtr<SimpleLine> pSubline1(new SimpleLine(0));
            MMUtil::SubLine(pSection1Curve,
                            Pt1,
                            !rSegment1.HasUTurn() ? pSection1->GetEndPoint() :
                                                    pSection1->GetStartPoint(),
                            pSection1->GetCurveStartsSmaller(),
                            m_dNetworkScale,
                            *pSubline1);

            double dLenCurve1 = MMUtil::CalcLengthCurve(pSubline1.get(),
                                                        m_dNetworkScale);


            const SimpleLine* pSection2Curve = pSection2->GetCurve();

            AttributePtr<SimpleLine> pSubline2(new SimpleLine(0));
            MMUtil::SubLine(pSection2Curve,
                            pSection2->GetStartPoint(),
                            Pt2,
                            pSection2->GetCurveStartsSmaller(),
                            m_dNetworkScale,
                            *pSubline2);

            double dLenCurve2 = MMUtil::CalcLengthCurve(pSubline2.get(),
                                                        m_dNetworkScale);


            double dLength = dLenCurve1 + dLenCurve2;


            const size_t nCurves = vecCurvesBetweenPoints.size();
            for (size_t i = 0; i < nCurves; ++i)
            {
                dLength += MMUtil::CalcLengthCurve(vecCurvesBetweenPoints[i],
                                                   m_dNetworkScale);
            }

            // Total time
            DateTime Duration = rData2.GetTime() - rData1.GetTime();
            Duration.Abs();

            // Process first curve

            DateTime TimeEnd(rData1.GetTime());

            if (!pSubline1 ||
                AlmostEqual(dLenCurve1, 0.0))
            {
                Interval<Instant> TimeInterval(rData1.GetTime(),
                                               TimeEnd,
                                               true, true);
                AttributePtr<UPoint> pUPoint(new UPoint(TimeInterval,
                                                        Pt1, Pt1));
                m_pResMPoint->Add(*pUPoint);
            }
            else
            {
                TimeEnd = rData1.GetTime() +
                           DateTime(datetime::durationtype,
                                    (uint64_t)(((Duration.millisecondsToNull()
                                                / dLength) * dLenCurve1) + .5));
                Interval<Instant> TimeInterval(rData1.GetTime(),
                                               TimeEnd,
                                               true,
                                               rData1.GetTime() == TimeEnd);
                ProcessCurve(*pSubline1, TimeInterval, dLenCurve1);
            }

            // Process curves in between

            for (size_t i = 0; i < nCurves; ++i)
            {
                const SimpleLine* pCurve = vecCurvesBetweenPoints[i];
                if (pCurve == NULL)
                    continue;

                double dLenCurve = MMUtil::CalcLengthCurve(pCurve,
                                                           m_dNetworkScale);
                if (AlmostEqual(dLenCurve, 0.0))
                    continue;

                DateTime TimeStart = TimeEnd;
                TimeEnd = TimeStart +
                          DateTime(datetime::durationtype,
                                    (uint64_t)(((Duration.millisecondsToNull()
                                                / dLength) * dLenCurve) + .5));

                Interval<Instant> TimeInterval(TimeStart,
                                               TimeEnd,
                                               true, false);

                ProcessCurve(*pCurve, TimeInterval, dLenCurve);
            }

            // Process last curve

            if (!pSubline2 ||
                AlmostEqual(dLenCurve2, 0.0))
            {
                DateTime TimeStart = TimeEnd;
                TimeEnd = rData2.GetTime();
                Interval<Instant> TimeInterval(TimeStart,
                                               TimeEnd,
                                               true, true);
                AttributePtr<UPoint> pUPoint(new UPoint(TimeInterval,
                                                        Pt2, Pt2));
                m_pResMPoint->Add(*pUPoint);
            }
            else
            {
                assert(TimeEnd.millisecondsToNull() <=
                       rData2.GetTime().millisecondsToNull());
                DateTime TimeStart = TimeEnd;
                TimeEnd = rData2.GetTime();
                Interval<Instant> TimeInterval(TimeStart,
                                               TimeEnd,
                                               true, false);
                ProcessCurve(*pSubline2, TimeInterval, dLenCurve2);
            }
        }
    }
}

void MPointCreator::ProcessCurve(const SimpleLine& rCurve,
                                 const Interval<Instant> TimeInterval,
                                 double dCurveLength)
{
    double dLength = dCurveLength < 0.0 ?
                           MMUtil::CalcLengthCurve(&rCurve, m_dNetworkScale) :
                           dCurveLength;
    if (AlmostEqual(dLength, 0.0))
    {
        AttributePtr<UPoint> pUPoint(new UPoint(TimeInterval,
                                                rCurve.StartPoint(),
                                                rCurve.EndPoint()));
        m_pResMPoint->Add(*pUPoint);
        return;
    }

    DateTime Duration = TimeInterval.end - TimeInterval.start;
    Duration.Abs();

    const bool bStartsSmaller = rCurve.GetStartSmaller();

    Instant TimeStart(TimeInterval.start);

    const int nHalfSegments = rCurve.Size();
    if (nHalfSegments <= 0)
        return;

    assert(nHalfSegments % 2 == 0);

    LRS lrs(bStartsSmaller ? 0.0 : rCurve.Length(), 0);
    int lrsPosAkt = 0;
    if (!const_cast<SimpleLine&>(rCurve).Get(lrs, lrsPosAkt))
    {
        assert(false);
        return;
    }

    LRS lrsAkt;
    rCurve.Get(lrsPosAkt, lrsAkt);
    HalfSegment hs;
    rCurve.Get(lrsAkt.hsPos, hs);

    double dLengthHS = MMUtil::CalcDistance(hs.GetLeftPoint(),
                                            hs.GetRightPoint(),
                                            m_dNetworkScale);
    DateTime TimeEnd = TimeStart +
                       DateTime(datetime::durationtype,
                                    (uint64_t)(((Duration.millisecondsToNull()
                                                 / dLength) * dLengthHS) + .5));

    Interval<Instant> TimeIntervalAkt(TimeStart, TimeEnd,
                                      true, TimeStart == TimeEnd);

    Point Pt1(false);
    Point Pt2(false);

    if (const_cast<SimpleLine&>(rCurve).StartsSmaller())
    {
        Pt1 = hs.GetDomPoint();
        Pt2 = hs.GetSecPoint();
    }
    else
    {
        Pt1 = hs.GetSecPoint();
        Pt2 = hs.GetDomPoint();
    }

    AttributePtr<UPoint> pUPoint(new UPoint(TimeIntervalAkt, Pt1, Pt2));
    m_pResMPoint->Add(*pUPoint);

    TimeStart = TimeEnd;

    if (bStartsSmaller)
        ++lrsPosAkt;
    else
        --lrsPosAkt;

    while (lrsPosAkt >= 0 && lrsPosAkt < nHalfSegments / 2)
    {
        rCurve.Get(lrsPosAkt, lrsAkt);
        rCurve.Get(lrsAkt.hsPos, hs);

        double dLengthHS = MMUtil::CalcDistance(hs.GetLeftPoint(),
                                                hs.GetRightPoint(),
                                                m_dNetworkScale);

        DateTime TimeEnd = TimeStart +
                           DateTime(datetime::durationtype,
                                    (uint64_t)(((Duration.millisecondsToNull()
                                                 / dLength) * dLengthHS) + .5));

        Interval<Instant> TimeIntervalAkt(TimeStart, TimeEnd,
                                          true, TimeStart == TimeEnd);

        if (AlmostEqual(Pt2, hs.GetDomPoint()))
        {
            Pt1 = hs.GetDomPoint();
            Pt2 = hs.GetSecPoint();
        }
        else
        {
            Pt1 = hs.GetSecPoint();
            Pt2 = hs.GetDomPoint();
        }

        /*if (AlmostEqual(Pt2, rCurve.EndPoint()))
            TimeStart = TimeInterval.end;*/

        AttributePtr<UPoint> pUPoint(new UPoint(TimeIntervalAkt, Pt1, Pt2));
        m_pResMPoint->Add(*pUPoint);

        TimeStart = TimeEnd;

        if (bStartsSmaller)
            ++lrsPosAkt;
        else
            --lrsPosAkt;
    }
}


} // end of namespace mapmatch

