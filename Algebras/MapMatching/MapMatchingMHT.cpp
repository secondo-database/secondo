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

This implementation file contains the implementation of the class ~MapMatchingMHT~.
It is an map matching algorithm based on the Multiple Hypothesis Technique (MHT)

2 Defines and includes

*/
#include "MapMatchingMHT.h"
#include "NetworkRoute.h"
#include "NetworkSection.h"

#include <stdio.h>

#include "../Network/NetworkAlgebra.h"
#include "../TemporalNet/TemporalNetAlgebra.h"

namespace mapmatch {

// Constructor
MapMatchingMHT::MapMatchingMHT(Network* pNetwork, MPoint* pMPoint)
:MapMatchingBase(pNetwork, pMPoint), m_pTracePoints(NULL)
{
}

// Destructor
MapMatchingMHT::~MapMatchingMHT()
{
}

#define MYTRACE(a) MyStream << a << '\n'
#define MYVARTRACE(a) MyStream << #a << a << '\n'

extern bool Intersects(const Region& rRegion, const SimpleLine& rSLine);

static ofstream MyStream;

extern Point CalcOrthogonalProjection(const SimpleLine& rLine, const Point& rPt,
                                      double& rdDistanceRes);

Point MapMatchingMHT::ProcessRoute(const NetworkRoute& rNetworkRoute,
                                   const Region& rRegion, const Point& rPt,
                                   double& rdDistance)
{
    if (rNetworkRoute.IsDefined())
    {
        int nRouteID = rNetworkRoute.GetRouteID();
        const SimpleLine* pRouteCurve = rNetworkRoute.GetCurve();

        if (pRouteCurve != NULL && pRouteCurve->IsDefined() &&
            Intersects(rRegion, *pRouteCurve))
        {
            MYTRACE(nRouteID);

            return ProcessRouteSections(nRouteID, rRegion, rPt, rdDistance);
        }
    }

    return Point(false);
}

Point MapMatchingMHT::ProcessRouteSections(const int nRouteID,
                                           const Region& rRegion,
                                           const Point& rPt,
                                           double& rdDistanceRes)
{
    RouteInterval Interval(nRouteID, 0.0, std::numeric_limits<double>::max());
    std::vector<TupleId> vecSections;
    m_pNetwork->GetSectionsOfRoutInterval(&Interval, vecSections);

    MYTRACE("xxxx Sections of Route xxx");

    Point ResPoint(false /*not defined*/);
    double dShortestDistance = std::numeric_limits<double>::max();

    for (std::vector<TupleId>::const_iterator it = vecSections.begin();
         it != vecSections.end(); ++it)
    {
        NetworkSection Section(m_pNetwork->GetSection(*it), false);
        if (Section.IsDefined())
        {
            const SimpleLine* pSectionCurve = Section.GetCurve();
            if (pSectionCurve != NULL && Intersects(rRegion, *pSectionCurve))
            {
                int sid = Section.GetSectionID();
                MYVARTRACE(sid);

                //pSectionCurve->Print(MyStream);

                double dDistance = 0.0;
                Point PointProjection =
                      CalcOrthogonalProjection(*pSectionCurve, rPt, dDistance);
                if (PointProjection.IsDefined())
                {
                    MYVARTRACE(PointProjection);
                    /*double dPosition = 0.0;
                    if (pSectionCurve->AtPoint(PointProjection,
                                               Section.GetCurveStartsSmaller(),
                                               dPosition))
                    {
                        MYVARTRACE(dPosition);
                    }*/
                    if (dDistance < dShortestDistance)
                    {
                        ResPoint = PointProjection;
                        dShortestDistance = dDistance;
                    }
                }
            }
        }
    }

    rdDistanceRes = dShortestDistance;
    return ResPoint;
}

GPoint MapMatchingMHT::ProcessPoint(const Point& rPoint)
{
    if (m_pNetwork == NULL)
    {
        return GPoint(false);
    }

    MYTRACE("++++++++++++++++++++++++++++");
    MYTRACE("ProcessPoint");
    MYVARTRACE(rPoint);

    const double dScaleFactor = 1000;// TODO m_pNetwork->GetScalefactor();
    MYVARTRACE(dScaleFactor);

    //const Rectangle<2> bbox( true, 7894.28, 7897.28, 49483.7, 49484.4);

    //const double dDist = 0.1; // Testnetz
    const double dDist = 0.001;
    const Rectangle<2> BBox(true, rPoint.GetX() - dDist * dScaleFactor,
                                  rPoint.GetX() + dDist * dScaleFactor,
                                  rPoint.GetY() - dDist * dScaleFactor,
                                  rPoint.GetY() + dDist * dScaleFactor);

    BBox.Print(MyStream);
    Region RegionBBox(BBox);

    RegionBBox.Print(MyStream);

    Point PointRes(false);
    NetworkRoute NetworkRouteRes(NULL);
    double dShortestDistance = std::numeric_limits<double>::max();

    R_TreeLeafEntry<2, TupleId> res;
    if (m_pNetwork->GetRTree()->First(BBox, res))
    {
        NetworkRoute Route(m_pNetwork->GetRoutes()->
                GetTuple(res.info, false), false);
        PointRes = ProcessRoute(Route, RegionBBox, rPoint, dShortestDistance);
        NetworkRouteRes = Route;

        MYVARTRACE(Route.GetRouteID());
    }

    while (m_pNetwork->GetRTree()->Next(res))
    {
        NetworkRoute Route(m_pNetwork->GetRoutes()->GetTuple(res.info, false),
                           false);

        MYVARTRACE(Route.GetRouteID());

        double dDistance = 0.0;
        Point ptRes = ProcessRoute(Route, RegionBBox, rPoint, dDistance);
        if (ptRes.IsDefined() && dDistance < dShortestDistance)
        {
            PointRes = ptRes;
            dShortestDistance = dDistance;
            NetworkRouteRes = Route;
        }
    }

    if (PointRes.IsDefined())
    {
        TracePoint(PointRes);

        bool startSmaller = NetworkRouteRes.GetStartsSmaller();
        const SimpleLine* pRouteCurve = NetworkRouteRes.GetCurve();

        double dPos = 0.0;
        if (pRouteCurve->AtPoint(PointRes, startSmaller, dPos))
        {
            return GPoint(true, m_pNetwork->GetId(),
                          NetworkRouteRes.GetRouteID(), dPos, None);
        }
    }

    return GPoint(false);
}

bool MapMatchingMHT::DoMatch(MGPoint* pResMGPoint)
{
    // cout << "MapMatchingMHT::DoMatch called" << endl;

    MyStream.open("/home/secondo/MyTrace.txt", ios_base::out | ios_base::app);

    /*vector<int> vecIDs;
    vecIDs.push_back(1);
    vecIDs.push_back(2);
    vecIDs.push_back(3);
    vecIDs.push_back(4);
    vecIDs.push_back(5);
    vecIDs.push_back(6);
    vecIDs.push_back(7);
    vecIDs.push_back(8);
    vecIDs.push_back(9);
    vecIDs.push_back(10);

    for (size_t j = 0; j < vecIDs.size(); j++)
    {
        MYTRACE("*************************");
        MYVARTRACE(vecIDs[j]);
        MYTRACE("*************************");

        bool bUpDown = false;
        for (int k = 0; k < 2; k++)
        {
            MYVARTRACE(bUpDown);
            vector<DirectedSection> adjSectionList;
            m_pNetwork->GetAdjacentSections(vecIDs[j], bUpDown,
                                            adjSectionList);
            for (size_t i = 0; i < adjSectionList.size(); i++)
            {
                DirectedSection actNextSect = adjSectionList[i];

                TupleId tid = actNextSect.GetSectionTid();
                MYVARTRACE(tid);
                bool bUpDownFlag = actNextSect.GetUpDownFlag();
                MYVARTRACE(bUpDownFlag);
            }
            bUpDown = true;
            MYTRACE("--------------------------");
        }
    }*/

    // Initialization
    if (!InitMapMatching(pResMGPoint))
    {
        return false;
    }

    m_pTracePoints = new Points(1);
    m_pTracePoints->StartBulkLoad();

    // Processing Units
    for (int i = 0; i < m_pMPoint->GetNoComponents(); ++i)
    {
        UPoint ActUPoint(false);
        m_pMPoint->Get(i, ActUPoint);

        if (!ActUPoint.IsDefined())
            continue;

        GPoint Point1 = ProcessPoint(ActUPoint.p0);
        GPoint Point2 = ProcessPoint(ActUPoint.p1);

        MYTRACE("+++++++++++++ 2 Punkte +++++++++++++++");
        Point1.Print(MyStream);
        Point2.Print(MyStream);

        if (Point1.IsDefined() && Point2.IsDefined())
        {
            if (Point1.GetRouteId() == Point2.GetRouteId())
            {
                NetworkRoute Route(m_pNetwork->GetRoute(Point1.GetRouteId()));

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

                if (Point1.GetPosition() < Point2.GetPosition())
                {
                    Point1.SetSide(Route.GetStartsSmaller() ? Up : Down);
                    Point2.SetSide(Route.GetStartsSmaller() ? Up : Down);
                }
                else
                {
                    Point1.SetSide(Route.GetStartsSmaller() ? Down : Up);
                    Point2.SetSide(Route.GetStartsSmaller() ? Down : Up);
                }

                UGPoint ActUGPoint(ActUPoint.timeInterval, Point1, Point2);
                this->AddUGPoint(ActUGPoint);
            }
            else
            {
                // Different routes
                CcInt Route1Id(Point1.GetRouteId());
                CcInt Route2Id(Point2.GetRouteId());
                double dRoute1Measure = numeric_limits<double>::max();
                double dRoute2Measure = numeric_limits<double>::max();

                m_pNetwork->GetJunctionMeasForRoutes(&Route1Id,
                        &Route2Id, dRoute1Measure, dRoute2Measure);
                if (dRoute1Measure < numeric_limits<double>::max() &&
                    dRoute2Measure < numeric_limits<double>::max())
                {
                    NetworkRoute Route1(m_pNetwork->GetRoute(
                                                        Point1.GetRouteId()));
                    NetworkRoute Route2(m_pNetwork->GetRoute(
                                                        Point1.GetRouteId()));

                    GPoint Point1_2(true, m_pNetwork->GetId(),
                                    Point1.GetRouteId(), dRoute1Measure,
                                    Point1.GetSide());
                    GPoint Point2_1(true, m_pNetwork->GetId(),
                                    Point2.GetRouteId(), dRoute2Measure,
                                    Point2.GetSide());

                    if (Point1.GetPosition() < Point1_2.GetPosition())
                    {
                        Point1.SetSide(Route1.GetStartsSmaller() ? Up : Down);
                        Point1_2.SetSide(Route1.GetStartsSmaller() ? Up : Down);
                    }
                    else
                    {
                        Point1.SetSide(Route1.GetStartsSmaller() ? Down : Up);
                        Point1_2.SetSide(Route1.GetStartsSmaller() ? Down : Up);
                    }

                    if (Point2_1.GetPosition() < Point2.GetPosition())
                    {
                        Point2_1.SetSide(Route2.GetStartsSmaller() ? Up : Down);
                        Point2.SetSide(Route2.GetStartsSmaller() ? Up : Down);
                    }
                    else
                    {
                        Point2_1.SetSide(Route2.GetStartsSmaller() ? Down : Up);
                        Point2.SetSide(Route2.GetStartsSmaller() ? Down : Up);
                    }

                    // TODO Bessere zeitliche Aufteilung
                    UGPoint UGPoint1(Interval<Instant>(
                                          ActUPoint.timeInterval.start,
                                          ActUPoint.timeInterval.start +
                                          (ActUPoint.timeInterval.end -
                                            ActUPoint.timeInterval.start) / 2,
                                          ActUPoint.timeInterval.lc, false),
                                                       Point1, Point1_2);
                    this->AddUGPoint(UGPoint1);

                    UGPoint UGPoint2(Interval<Instant>(
                                          ActUPoint.timeInterval.start +
                                          (ActUPoint.timeInterval.end -
                                            ActUPoint.timeInterval.start) / 2,
                                          ActUPoint.timeInterval.end,
                                          true, ActUPoint.timeInterval.rc),
                                          Point2_1, Point2);
                    this->AddUGPoint(UGPoint2);

                }
                else
                {
                    // No direct connection between Routes -> Shortest Path
                    MYTRACE("Shortest Path needed");

                }
            }
        }
    }

    m_pTracePoints->EndBulkLoad(false, false, false);

    ofstream TracePointStream("/home/secondo/Points.txt", ios_base::out);
    m_pTracePoints->Print(TracePointStream);
    TracePointStream.close();

    // Finalize
    FinalizeMapMatching();

    MyStream.close();

    return true;
}


void MapMatchingMHT::TracePoint(const Point& rPoint)
{
    (*m_pTracePoints) += rPoint;
}

} // end of namespace mapmatch



