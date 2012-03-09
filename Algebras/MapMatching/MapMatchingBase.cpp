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

[1] Implementation of the MapMatching Algebra

January-April, 2012. Matthias Roth

[TOC]

1 Overview

This implementation file contains the implementation of the class ~MapMatchingBase~.
Is is the base class of all Map Matching algorithms

2 Defines and includes

*/
#include "MapMatchingBase.h"
#include "MapMatchingUtil.h"
#include "NetworkRoute.h"

#include <stdio.h>

#include "TemporalNetAlgebra.h"


namespace mapmatch {

// Constructor
MapMatchingBase::MapMatchingBase(Network* pNetwork,
                                 const MPoint* pMPoint)
:m_pNetwork(pNetwork),  // TODO
 m_dNetworkScale(1000.0/*pNetwork != NULL ? pNetwork->GetScalefactor() : 1.0*/),
 m_pDbaMMData(new DbArray<MapMatchData>(0)),
 m_pResMGPoint(NULL), m_pRITree(NULL)
{
    if (pMPoint != NULL)
    {
        Point ptPrev(false);
        int64_t timePrev(0);

        for (int i = 0; i < pMPoint->GetNoComponents(); ++i)
        {
            UPoint ActUPoint(false);
            pMPoint->Get(i, ActUPoint);
            if (!ActUPoint.IsDefined())
                continue;

            if (ActUPoint.p0 != ptPrev ||
                ActUPoint.timeInterval.start.millisecondsToNull() != timePrev)
            {
                MapMatchData MMData(ActUPoint.p0.GetY(),
                                    ActUPoint.p0.GetX(),
                                    ActUPoint.timeInterval.
                                              start.millisecondsToNull());

                m_pDbaMMData->Append(MMData);

                ptPrev = ActUPoint.p0;
                timePrev = ActUPoint.timeInterval.start.millisecondsToNull();
            }

            if (ActUPoint.p1 != ptPrev ||
                ActUPoint.timeInterval.end.millisecondsToNull() != timePrev)
            {
                MapMatchData MMData(ActUPoint.p1.GetY(),
                                    ActUPoint.p1.GetX(),
                                    ActUPoint.timeInterval.
                                              end.millisecondsToNull());

                m_pDbaMMData->Append(MMData);

                ptPrev = ActUPoint.p1;
                timePrev = ActUPoint.timeInterval.end.millisecondsToNull();
            }
        }
    }
}

MapMatchingBase::MapMatchingBase(Network* pNetwork,
                                 DbArrayPtr<DbArray<MapMatchData> > pDbaMMData)
:m_pNetwork(pNetwork),  // TODO
 m_dNetworkScale(1000.0/*pNetwork != NULL ? pNetwork->GetScalefactor() : 1.0*/),
 m_pDbaMMData(pDbaMMData),
 m_pResMGPoint(NULL), m_pRITree(NULL)
{

}

// Destructor
MapMatchingBase::~MapMatchingBase()
{
    m_pNetwork = NULL;
    m_pResMGPoint = NULL;
    if (m_pRITree != NULL)
    {
        m_pRITree->Destroy();
        delete m_pRITree;
        m_pRITree = NULL;
    }
}

bool MapMatchingBase::InitMapMatching(MGPoint* pResMGPoint)
{
    if (pResMGPoint == NULL)
        return false;
    else
    {
        m_pResMGPoint = pResMGPoint;
        m_pResMGPoint->Clear();

        if (m_pNetwork == NULL || !m_pNetwork->IsDefined() ||
            m_pDbaMMData == NULL || m_pDbaMMData->Size() == 0)
        {
            pResMGPoint->SetDefined(false);
            return false;
        }
        else
        {
            pResMGPoint->SetDefined(true);
            pResMGPoint->StartBulkLoad();

            if (m_pRITree != NULL)
                delete m_pRITree;

            m_pRITree = new RITreeP(0);

            return true;
        }
    }
}

void MapMatchingBase::FinalizeMapMatching(void)
{
    if (m_pResMGPoint != NULL && m_pRITree != NULL)
    {
        m_pResMGPoint->EndBulkLoad(true);
        m_pResMGPoint->SetDefined(!m_pRITree->IsEmpty());
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

void MapMatchingBase::AddUGPoint(const UGPoint& rAktUGPoint)
{
    if (m_pResMGPoint != NULL && m_pRITree != NULL)
    {
        m_pResMGPoint->Add(rAktUGPoint);
        m_pRITree->InsertUnit(rAktUGPoint.GetStartPoint().GetRouteId(),
                              rAktUGPoint.GetStartPoint().GetPosition(),
                              rAktUGPoint.GetEndPoint().GetPosition());
    }
}

void MapMatchingBase::AddUnit(const int nRouteID,
        const double dPos1, const double dPos2)
{
    if (m_pRITree != NULL)
    {
        m_pRITree->InsertUnit(nRouteID, dPos1, dPos2);
    }
}

bool MapMatchingBase::CalcShortestPath(const GPoint* pGPStart,
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
                                                  m_pNetwork, m_dNetworkScale);
            dLen /= 1000.; // KM
            datetime::DateTime DiffTime = rtimeEnd - rtimeStart;
            double dDuration = DiffTime.millisecondsToNull() /
                                                    (1000. * 60. *60.); // hours
            if (!AlmostEqual(dDuration, 0.0))
            {
                double dSpeed = dLen / dDuration; // km/h
                if (dSpeed > 250.) // TODO use additional data -
                                   // speed-limits, previous speed, ...
                {
                    ofstream StreamBadNetwork(
                                     "/home/secondo/Traces/BadNetwork.txt",
                                     ios_base::out|ios_base::ate|ios_base::app);
                    StreamBadNetwork << "Too fast : " << endl;
                    StreamBadNetwork << "Speed: " << dSpeed << endl;
                    StreamBadNetwork << "Distance: " << dLen << endl;
                    StreamBadNetwork << "Time: ";
                    rtimeStart.Print(StreamBadNetwork);
                    StreamBadNetwork << endl;
                    rtimeEnd.Print(StreamBadNetwork);
                    StreamBadNetwork << endl;
                    StreamBadNetwork << "GP1: ";
                    pGPStart->Print(StreamBadNetwork);
                    Point pt(false);
                    AttributePtr<Point> pPtStart(pGPStart->ToPoint());
                    pPtStart->Print(StreamBadNetwork);
                    StreamBadNetwork << endl;
                    StreamBadNetwork << "GP2: ";
                    pGPEnd->Print(StreamBadNetwork);
                    AttributePtr<Point> pPtEnd(pGPEnd->ToPoint());
                    pPtEnd->Print(StreamBadNetwork);
                    StreamBadNetwork << endl;
                    return false;
                }
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


bool MapMatchingBase::ConnectPoints(const GPoint& rGPStart,
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

            /*bool bDual = pNetwork->GetDual(ri->GetRouteId());
             bool bMovingUp = true;
             if (ri->GetStartPos() > ri->GetEndPos())
             bMovingUp = false;
             Side side = None;
             if (bDual && bMovingUp)
             side = Up;
             else if (bDual && !bMovingUp)
             side = Down;
             else
             side = None;*/

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

#if 0
            // funktioniert so nicht, da der kürzeste Weg
            // nicht unbedingt über die direkte Verbindung
            // der beiden Routen führen muss

            CcInt Route1Id(GPoint1.GetRouteId());
            CcInt Route2Id(GPoint2.GetRouteId());
            double dRoute1Measure = numeric_limits<double>::max();
            double dRoute2Measure = numeric_limits<double>::max();

            m_pNetwork->GetJunctionMeasForRoutes(&Route1Id, &Route2Id,
                    dRoute1Measure, dRoute2Measure);
            if (dRoute1Measure < numeric_limits<double>::max()
                    && dRoute2Measure < numeric_limits<double>::max())
            {
                NetworkRoute Route1(m_pNetwork->GetRoute(GPoint1.GetRouteId()));
                NetworkRoute Route2(m_pNetwork->GetRoute(GPoint2.GetRouteId()));

                GPoint GPoint1_2(true, m_pNetwork->GetId(), GPoint1.GetRouteId()
                                 ,dRoute1Measure, GPoint1.GetSide());
                GPoint GPoint2_1(true, m_pNetwork->GetId(), GPoint2.GetRouteId()
                                 ,dRoute2Measure, GPoint2.GetSide());

                if (GPoint1.GetPosition() < GPoint1_2.GetPosition())
                {
                    GPoint1.SetSide(Route1.GetStartsSmaller() ? Up : Down);
                    GPoint1_2.SetSide(Route1.GetStartsSmaller() ? Up : Down);
                }
                else
                {
                    GPoint1.SetSide(Route1.GetStartsSmaller() ? Down : Up);
                    GPoint1_2.SetSide(Route1.GetStartsSmaller() ? Down : Up);
                }

                if (GPoint2_1.GetPosition() < GPoint2.GetPosition())
                {
                    GPoint2_1.SetSide(Route2.GetStartsSmaller() ? Up : Down);
                    GPoint2.SetSide(Route2.GetStartsSmaller() ? Up : Down);
                }
                else
                {
                    GPoint2_1.SetSide(Route2.GetStartsSmaller() ? Down : Up);
                    GPoint2.SetSide(Route2.GetStartsSmaller() ? Down : Up);
                }

                double dDistance1 = fabs(
                               GPoint1.GetPosition() - GPoint1_2.GetPosition());
                double dDistance2 = fabs(
                               GPoint2.GetPosition() - GPoint2_1.GetPosition());

                double dDistance = dDistance1 + dDistance2;

                Instant Mid = (rTimeInterval.end - rTimeInterval.start) *
                                                       (dDistance1 / dDistance);

                UGPoint UGPoint1(Interval<Instant>(rTimeInterval.start,
                                                   rTimeInterval.start + Mid,
                                                   rTimeInterval.lc, false),
                                                   GPoint1, GPoint1_2);
                this->AddUGPoint(UGPoint1);

                UGPoint UGPoint2(Interval<Instant>(rTimeInterval.start + Mid,
                                                   rTimeInterval.end,
                                                   true, rTimeInterval.rc),
                                                   GPoint2_1, GPoint2);

                this->AddUGPoint(UGPoint2);

                return true;
            }
            else
            {
                // No direct connection between Routes -> Shortest Path

                return CalcShortestPath(&GPoint1,
                                        &GPoint2,
                                        rTimeInterval.start,
                                        rTimeInterval.end);
            }
#else

            return CalcShortestPath(&GPoint1,
                                    &GPoint2,
                                    rTimeInterval.start,
                                    rTimeInterval.end,
                                    true);
#endif

        }
    }
    else
    {
        return false;
    }
}


} // end of namespace mapmatch



