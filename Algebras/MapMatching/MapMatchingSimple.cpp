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

This implementation file contains the implementation of the class ~MapMatchingSimple~.
Simple Map matching algorithm - !! only for testing purpose !!

2 Defines and includes

*/
#include "MapMatchingSimple.h"
#include "NetworkRoute.h"
#include "NetworkSection.h"
#include "MapMatchingUtil.h"

#include <stdio.h>

#include "../Network/NetworkAlgebra.h"
#include "../TemporalNet/TemporalNetAlgebra.h"

namespace mapmatch {

// Constructor
MapMatchingSimple::MapMatchingSimple(Network* pNetwork, MPoint* pMPoint)
:MapMatchingBase(pNetwork, pMPoint), m_pTracePoints(NULL)
{
}

// Destructor
MapMatchingSimple::~MapMatchingSimple()
{
}

#define MYTRACE(a) MyStream << a << '\n'
#define MYVARTRACE(a) MyStream << #a << a << '\n'

static ofstream MyStream;

Point MapMatchingSimple::ProcessRoute(const NetworkRoute& rNetworkRoute,
                                      const Region& rRegion, const Point& rPt,
                                      double& rdDistance)
{
    if (rNetworkRoute.IsDefined())
    {
        int nRouteID = rNetworkRoute.GetRouteID();
        const SimpleLine* pRouteCurve = rNetworkRoute.GetCurve();

        if (pRouteCurve != NULL && pRouteCurve->IsDefined() &&
            MMUtil::Intersects(rRegion, *pRouteCurve))
        {
            MYTRACE(nRouteID);

            return ProcessRouteSections(nRouteID, rRegion, rPt, rdDistance);
        }
    }

    return Point(false);
}

Point MapMatchingSimple::ProcessRouteSections(const int nRouteID,
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
            if (pSectionCurve != NULL &&
                MMUtil::Intersects(rRegion, *pSectionCurve))
            {
                int sid = Section.GetSectionID();
                MYVARTRACE(sid);

                //pSectionCurve->Print(MyStream);

                double dDistance = 0.0;
                Point PointProjection = MMUtil::CalcOrthogonalProjection(
                                                *pSectionCurve, rPt, dDistance,
                                                m_dNetworkScale);
                if (PointProjection.IsDefined())
                {
                    MYVARTRACE(PointProjection);
                    /*double dPosition = 0.0;
                    if (pSectionCurve->AtPoint(PointProjection,
                                   Section.GetCurveStartsSmaller(), dPosition))
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

GPoint MapMatchingSimple::ProcessPoint(const Point& rPoint)
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

    Geoid GeodeticSystem(Geoid::WGS1984);

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
        NetworkRoute Route(m_pNetwork->GetRoutes()->
                                 GetTuple(res.info, false), false);

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

bool MapMatchingSimple::DoMatch(MGPoint* pResMGPoint)
{
    // cout << "MapMatchingSimple::DoMatch called" << endl;

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
            m_pNetwork->GetAdjacentSections(vecIDs[j], bUpDown, adjSectionList);
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

    //m_pMPoint->Simplify();

    // Processing Units
    for (int i = 0; i < m_pDbaMMData->Size(); i += 2)
    {
        MapMatchData ActData1;
        m_pDbaMMData->Get(i, ActData1);

        MapMatchData ActData2;
        m_pDbaMMData->Get(i, ActData2);


        GPoint Point1 = ProcessPoint(ActData1.GetPoint());
        GPoint Point2 = ProcessPoint(ActData2.GetPoint());

        MYTRACE("+++++++++++++ 2 Punkte +++++++++++++++");
        Point1.Print(MyStream);
        Point2.Print(MyStream);

        const Interval<Instant> timeInterval(ActData1.m_Time, ActData2.m_Time,
                                             true, true);

        ConnectPoints(Point1, Point2, timeInterval);
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


void MapMatchingSimple::TracePoint(const Point& rPoint)
{
    (*m_pTracePoints) += rPoint;
}

} // end of namespace mapmatch



