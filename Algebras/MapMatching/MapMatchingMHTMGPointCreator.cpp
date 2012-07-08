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

[1] Implementation File containing class ~MGPointCreator~

April, 2012. Matthias Roth

[TOC]

1 Overview

This implementation file contains the implementation of
the class ~MGPointCreator~.

2 Defines and includes

*/

#include "MapMatchingMHTMGPointCreator.h"
#include "MapMatchingUtil.h"
#include "NetworkAdapter.h"

#include <NetworkAlgebra.h>
#include <TemporalNetAlgebra.h>

#include <LogMsg.h>


namespace mapmatch {

/*
3 class MGPointCreator

*/

MGPointCreator::MGPointCreator(NetworkAdapter* pNetworkAdapter,
                               MGPoint* pResMGPoint)
:m_pNetwork(pNetworkAdapter != NULL ? pNetworkAdapter->GetNetwork() : NULL),
 m_dNetworkScale(pNetworkAdapter != NULL ?
                                      pNetworkAdapter->GetNetworkScale() : 1.0),
 m_pResMGPoint(pResMGPoint),
 m_pRITree(NULL)
{
}

MGPointCreator::~MGPointCreator()
{
    m_pResMGPoint = NULL;
    if (m_pRITree != NULL)
    {
        m_pRITree->Destroy();
        delete m_pRITree;
        m_pRITree = NULL;
    }
    m_pNetwork = NULL;
}

bool MGPointCreator::CreateResult(const std::vector<MHTRouteCandidate*>&
                                                            rvecRouteCandidates)
{
    if (!Init() || m_pNetwork == NULL)
        return false;

    MHTRouteCandidate::PointDataPtr pLastPointOfPrevSection;
    bool bPrevCandidateFailed = false; // -> matching failed
                                       // -> don't connect by shortest path

    const int nNetworkkId = m_pNetwork->GetId();

    for (size_t i = 0; i < rvecRouteCandidates.size(); ++i)
    {
        bool bCalcShortestPath = ((i > 0) && !bPrevCandidateFailed);

        MHTRouteCandidate* pCandidate = rvecRouteCandidates[i];
        if (pCandidate != NULL)
        {
            bPrevCandidateFailed = pCandidate->GetFailed();

            MHTRouteCandidate::PointDataIterator itData =
                                                   pCandidate->PointDataBegin();
            MHTRouteCandidate::PointDataIterator itDataEnd =
                                                     pCandidate->PointDataEnd();

            // Find first defined point
            MHTRouteCandidate::PointDataPtr pData1;
            const GPoint* pGP1 = NULL;
            while(itData != itDataEnd &&
                  (pData1 == NULL ||
                   (pGP1 = GetGPoint(pData1, nNetworkkId, m_dNetworkScale))
                                                                       == NULL))
            {
                pData1 = *itData;
                ++itData;
            }

            // Process next points
            while (itData != itDataEnd && pData1 != NULL && pGP1 != NULL)
            {
                MHTRouteCandidate::PointDataPtr pData2;
                const GPoint* pGP2 = NULL;

                while (itData != itDataEnd &&
                       (pData2 == NULL ||
                        (pGP2 = GetGPoint(pData2, nNetworkkId, m_dNetworkScale))
                                                                       == NULL))
                {
                    pData2 = *itData;
                    ++itData;
                }

                if (bCalcShortestPath && pLastPointOfPrevSection != NULL)
                {
                    // Calculate ShortestPath between last point of previous
                    // candidate and first point of this candidate

                    // ShortestPath calculation only when time difference
                    // is less than 4 minutes
                    const DateTime MaxTimeDiff(durationtype, 240000);

                    if (pData1->GetTime() - pLastPointOfPrevSection->GetTime() <
                                                                    MaxTimeDiff)
                    {
                        CalcShortestPath(GetGPoint(pLastPointOfPrevSection,
                                                   nNetworkkId,
                                                   m_dNetworkScale),
                                         GetGPoint(pData1,
                                                   nNetworkkId,
                                                   m_dNetworkScale),
                                         pLastPointOfPrevSection->GetTime(),
                                         pData1->GetTime(),
                                         true);
                    }

                    bCalcShortestPath = false;
                }

                if (pData2 != NULL && pGP2 != NULL)
                {
                    const Interval<Instant> timeInterval(pData1->GetTime(),
                                                         pData2->GetTime(),
                                                         true  /*LC*/,
                                                         false /*RC*/);

                    ConnectPoints(*pGP1, *pGP2, timeInterval);

                    pLastPointOfPrevSection = pData2;
                }

                pData1 = pData2;
                pGP1 = pGP2;
            }
        }
    }

    Finalize();

    return true;
}

bool MGPointCreator::Init(void)
{
    if (m_pResMGPoint == NULL)
        return false;
    else
    {
        m_pResMGPoint->Clear();

        m_pResMGPoint->SetDefined(true); // always defined

        if (m_pNetwork == NULL || !m_pNetwork->IsDefined())
        {
            return false;
        }
        else
        {
            m_pResMGPoint->StartBulkLoad();

            if (m_pRITree != NULL)
                delete m_pRITree;

            m_pRITree = new RITreeP(0);

            return true;
        }
    }
}

void MGPointCreator::Finalize(void)
{
    if (m_pResMGPoint != NULL && m_pRITree != NULL)
    {
        m_pResMGPoint->EndBulkLoad(true);
        //m_pResMGPoint->SetDefined(!m_pRITree->IsEmpty());
        m_pResMGPoint->SetDefined(true); // always defined
        if (!m_pRITree->IsEmpty())
        {
            m_pRITree->TreeToDbArray(&m_pResMGPoint->m_trajectory, 0);

            m_pResMGPoint->SetTrajectoryDefined(true);
            m_pResMGPoint->m_trajectory.TrimToSize();
            m_pResMGPoint->SetBoundingBoxDefined(false);
        }
    }

    if (m_pRITree != NULL)
    {
        m_pRITree->Destroy();
        delete m_pRITree;
        m_pRITree = NULL;
    }
}

const GPoint* MGPointCreator::GetGPoint(
                                   const MHTRouteCandidate::PointDataPtr& pData,
                                   const int& nNetworkId,
                                   const double& dNetworkScale) const
{
    if (pData == NULL)
        return NULL;

    const shared_ptr<IMMNetworkSection>& pSection = pData->GetSection();
    if (pSection == NULL)
        return NULL;

    const NetworkSectionAdapter* pAdapter = pSection->CastToNetworkSection();
    if (pAdapter == NULL)
        return NULL;

    const DirectedNetworkSection* pNetwSection = pAdapter->GetSection();
    const Point* pPointProjection = pData->GetPointProjection();

    if (pPointProjection == NULL || !pPointProjection->IsDefined() ||
        pNetwSection == NULL || !pNetwSection->IsDefined())
    {
        return NULL;
    }

    const NetworkRoute& rRoute = pNetwSection->GetRoute();
    if (!rRoute.IsDefined())
    {
        return NULL;
    }

    const bool RouteStartsSmaller = rRoute.GetStartsSmaller();
    const SimpleLine* pRouteCurve = rRoute.GetCurve();

    double dPos = 0.0;
    if (pRouteCurve != NULL &&
        MMUtil::GetPosOnSimpleLine(*pRouteCurve,
                                   *pPointProjection,
                                   RouteStartsSmaller,
                                   dNetworkScale,
                                   dPos))
       //pRouteCurve->AtPoint(*pPointProjection, RouteStartsSmaller, dPos))
    {
        return new GPoint(true, nNetworkId, rRoute.GetRouteID(), dPos, None);
    }
    else
    {
        // Projected point could not be matched onto route
        assert(false);
        return NULL;
    }
}

void MGPointCreator::AddUGPoint(const UGPoint& rAktUGPoint)
{
    if (m_pResMGPoint != NULL && m_pRITree != NULL)
    {
        m_pResMGPoint->Add(rAktUGPoint);
        m_pRITree->InsertUnit(rAktUGPoint.GetStartPoint().GetRouteId(),
                              rAktUGPoint.GetStartPoint().GetPosition(),
                              rAktUGPoint.GetEndPoint().GetPosition());
    }
}

bool MGPointCreator::CalcShortestPath(const GPoint* pGPStart,
                                      const GPoint* pGPEnd,
                                      const datetime::DateTime& rtimeStart,
                                      const datetime::DateTime& rtimeEnd,
                                      const bool bCheckSpeed)
{
    if (pGPStart == NULL || !pGPStart->IsDefined() ||
        pGPEnd == NULL || !pGPEnd->IsDefined() ||
        !rtimeStart.IsDefined() || !rtimeEnd.IsDefined() ||
        m_pNetwork == NULL || !m_pNetwork->IsDefined())
    {
        return false;
    }

    const int nNetworkId = m_pNetwork->GetId();

    AttributePtr<GLine> pGlShortestPath(new GLine(false));
    //if (!pGPStart->ShortestPath(pGPEnd, pGlShortestPath.get(), m_pNetwork))
    if (!pGPStart->ShortestPathAStar(pGPEnd, pGlShortestPath.get(), m_pNetwork))
    {
        // ShortestPath calculation failed
        return false;
    }
    else
    {
        // ShortestPath calculation successfull

        // Check Speed
        if (bCheckSpeed)
        {
            double dLen = MMUtil::CalcLengthCurve(pGlShortestPath.get(),
                                                  m_pNetwork,
                                                  m_dNetworkScale);

            if (!MMUtil::CheckSpeed(dLen, rtimeStart,
                                    rtimeEnd,Point(false), Point(false)))
            {
                return false;
            }
        }

        // Create UGPoints

        RouteInterval actRouteInterval;
        Side side = None;

        DateTime timeCurrentStart(rtimeStart);
        DateTime timeCurrentEnd(rtimeStart);

        for (int i = 0; i < pGlShortestPath->NoOfComponents(); ++i)
        {
            pGlShortestPath->Get(i, actRouteInterval);

            Instant timeCurrentEnd = (rtimeEnd - rtimeStart) *
                    (fabs(actRouteInterval.GetEndPos() -
                                           actRouteInterval.GetStartPos())
                            / pGlShortestPath->GetLength()) + timeCurrentStart;

            if (actRouteInterval.GetRouteId() == pGPEnd->GetRouteId() &&
               AlmostEqual(actRouteInterval.GetEndPos(), pGPEnd->GetPosition()))
            {
                timeCurrentEnd = rtimeEnd; // End reached
            }

            if (actRouteInterval.GetStartPos() > actRouteInterval.GetEndPos())
            {
                side = Down; // Moving down
            }
            else if (actRouteInterval.GetStartPos() <
                                                   actRouteInterval.GetEndPos())
            {
                side = Up; //  Moving Up
            }
            else
            {
                side = None;
            }

            AttributePtr<UGPoint> pUGPoint(new UGPoint(
                    Interval<Instant>(timeCurrentStart, timeCurrentEnd,
                                      true, false),
                    nNetworkId, actRouteInterval.GetRouteId(),
                    side, actRouteInterval.GetStartPos(),
                    actRouteInterval.GetEndPos()));

            this->AddUGPoint(*pUGPoint);

            timeCurrentStart = timeCurrentEnd;
        }

        return true;
    }
}

bool MGPointCreator::ConnectPoints(const GPoint& rGPStart,
                                   const GPoint& rGPEnd,
                                   const Interval<Instant>& rTimeInterval)
{
    if (rGPStart.IsDefined() && rGPEnd.IsDefined() &&
        m_pNetwork != NULL && m_pNetwork->IsDefined())
    {
        GPoint GPoint1(rGPStart);
        GPoint GPoint2(rGPEnd);

        if (GPoint1.GetRouteId() == GPoint2.GetRouteId())
        {
            NetworkRoute Route(m_pNetwork->GetRoute(GPoint1.GetRouteId()));

            if (GPoint1.GetPosition() < GPoint2.GetPosition())
            {
                GPoint1.SetSide(Route.GetStartsSmaller() ? Up : Down);
                GPoint2.SetSide(Route.GetStartsSmaller() ? Up : Down);
            }
            else
            {
                GPoint1.SetSide(Route.GetStartsSmaller() ? Down : Up);
                GPoint2.SetSide(Route.GetStartsSmaller() ? Down : Up);
            }

            UGPoint ActUGPoint(rTimeInterval, GPoint1, GPoint2);
            this->AddUGPoint(ActUGPoint);

            return true;
        }
        else
        {
            // Different routes
            return CalcShortestPath(&GPoint1,
                                    &GPoint2,
                                    rTimeInterval.start,
                                    rTimeInterval.end,
                                    true);
        }
    }
    else
    {
        return false;
    }
}

} // end of namespace mapmatch

