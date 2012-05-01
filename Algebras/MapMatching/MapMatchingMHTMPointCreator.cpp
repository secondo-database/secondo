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

    const MHTRouteCandidate::PointData* pLastPointOfPrevSection = NULL;
    bool bPrevCandidateFailed = false; // -> matching failed

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

        MHTRouteCandidate::PointData* pData1 = NULL;

        while (it != itEnd)
        {
            MHTRouteCandidate::RouteSegment* pSegment = *it;
            ++it;
            if (pSegment == NULL)
                continue;

            const SimpleLine* pSectionCurve = NULL;
            const shared_ptr<IMMNetworkSection>& pSection =
                                                         pSegment->GetSection();
            if (pSection != NULL && pSection->IsDefined())
                pSectionCurve = pSection->GetCurve();

            const std::vector<MHTRouteCandidate::PointData*>& vecPoints =
                                                          pSegment->GetPoints();

            std::vector<MHTRouteCandidate::PointData*>::const_iterator
                                                      itPts = vecPoints.begin();
            std::vector<MHTRouteCandidate::PointData*>::const_iterator
                                                     itPtsEnd = vecPoints.end();

            while (itPts != itPtsEnd && pData1 == NULL)
            {
                pData1 = *itPts;
                ++itPts;
            }

            while (itPts != itPtsEnd &&
                   pData1 != NULL)
            {
                MHTRouteCandidate::PointData* pData2 = NULL;

                while (itPts != itPtsEnd && pData2 == NULL)
                {
                    pData2 = *itPts;
                    ++itPts;
                }

                if (pData2 == NULL)
                    continue;

                ProcessPoints(*pData1, *pData2);

                pData1 = pData2;
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


void MPointCreator::ProcessPoints(const MHTRouteCandidate::PointData& rData1,
                                  const MHTRouteCandidate::PointData& rData2)
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

    if (0)
    {
        Interval<Instant> TimeInterval(rData1.GetTime(),
                                       rData2.GetTime(),
                                       true, false);

        AttributePtr<UPoint> pUPoint(new UPoint(
                                     Interval<Instant>(rData1.GetTime(),
                                                       rData2.GetTime(),
                                                       true, false),
                                     Pt1, Pt2));
        m_pResMPoint->Add(*pUPoint);
    }
    else
    {
        const shared_ptr<IMMNetworkSection>& pSection1 = rData1.GetSection();
        const shared_ptr<IMMNetworkSection>& pSection2 = rData2.GetSection();

        if (pSection1 != NULL && pSection1->IsDefined() &&
            pSection2 != NULL && pSection2->IsDefined() &&
            pSection1 == pSection2)
        {
            const SimpleLine* pSectionCurve = pSection1->GetCurve();

            double dPos1 = -1.0;
            MMUtil::GetPosOnSimpleLine(*pSectionCurve,
                                       Pt1,
                                       pSection1->GetCurveStartsSmaller(),
                                       0.000001 * m_dNetworkScale,
                                       dPos1);

            double dPos2 = -1.0;
            MMUtil::GetPosOnSimpleLine(*pSectionCurve,
                                       Pt2,
                                       pSection1->GetCurveStartsSmaller(),
                                       0.000001 * m_dNetworkScale,
                                       dPos2);

            AttributePtr<SimpleLine> pSubline(new SimpleLine(0));
            pSectionCurve->SubLine(dPos1,
                                   dPos2,
                                   pSection1->GetCurveStartsSmaller(),
                                   *pSubline);

            if (pSubline->IsDefined())
            {
                Interval<Instant> TimeInterval(rData1.GetTime(),
                                               rData2.GetTime(),
                                               true, false);
                ProcessCurve(*pSubline, TimeInterval);
            }
        }
        else
        {
            /*const SimpleLine* pSectionCurve1 = pSection1->GetCurve();
            double dPos1 = -1.0;
            MMUtil::GetPosOnSimpleLine(*pSectionCurve1,
                                       pSection1->GetEndPoint(),
                                       pSection1->GetCurveStartsSmaller(),
                                       0.000001 * m_dNetworkScale,
                                       dPos1);


            const SimpleLine* pSectionCurve2 = pSection2->GetCurve();



            pSection2->GetStartPoint();*/


            Interval<Instant> TimeInterval(rData1.GetTime(),
                                           rData2.GetTime(),
                                           true, false);

            AttributePtr<UPoint> pUPoint(new UPoint(
                                             Interval<Instant>(rData1.GetTime(),
                                                               rData2.GetTime(),
                                                               true, false),
                                             Pt1, Pt2));
            m_pResMPoint->Add(*pUPoint);
        }
    }
}

void MPointCreator::ProcessCurve(const SimpleLine& rCurve,
                                 const Interval<Instant> TimeInterval)
{
    cout << "*********************************" << endl;
    cout << "StartPoint Curve:";
    rCurve.StartPoint().Print(cout);
    cout << "EndPoint Curve:" << endl;
    rCurve.EndPoint().Print(cout);
    cout << "*********************************"  << endl;

    double dLength = MMUtil::CalcLengthCurve(&rCurve, m_dNetworkScale);
    if (AlmostEqual(dLength, 0.0))
        return;

    const datetime::DateTime Duration = TimeInterval.end - TimeInterval.start;

    const bool bStartsSmaller = rCurve.GetStartSmaller();

    Instant TimeStart(TimeInterval.start);

    const int nHalfSegments = rCurve.Size();
    bool bValid = true;
    for (int i = 0; bValid && i < nHalfSegments; ++i)
    {
        HalfSegment hs;
        if (!bStartsSmaller)
            rCurve.Get(i, hs);
        else
            rCurve.Get(i, hs);

        //if (hs.IsLeftDomPoint())
        {
            double dLengthHS = MMUtil::CalcDistance(hs.GetLeftPoint(),
                                                    hs.GetRightPoint(),
                                                    m_dNetworkScale);

            datetime::DateTime TimeEnd = TimeStart + datetime::DateTime(
                                                         datetime::durationtype,
                                       (uint64_t)((Duration.millisecondsToNull()
                                                   / dLength) * dLengthHS));

            Interval<Instant> TimeInterval(TimeStart,
                                           TimeEnd,
                                           true, false);

            Point Pt1 = hs.GetDomPoint();
            Point Pt2 = hs.GetSecPoint();

            AttributePtr<UPoint> pUPoint(new UPoint(TimeInterval, Pt1, Pt2));
            m_pResMPoint->Add(*pUPoint);

            TimeStart = TimeEnd;
        }
    }
}


} // end of namespace mapmatch

