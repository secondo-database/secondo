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
#include "MapMatchingUtil.h"

#include <stdio.h>

#include "../Network/NetworkAlgebra.h"
#include "../TemporalNet/TemporalNetAlgebra.h"

namespace mapmatch {

// Constructor
MapMatchingMHT::MapMatchingMHT(Network* pNetwork, MPoint* pMPoint)
:MapMatchingBase(pNetwork, pMPoint)
{
}

// Destructor
MapMatchingMHT::~MapMatchingMHT()
{
}

#define MYTRACE(a) MyStream << a << '\n'
#define MYVARTRACE(a) MyStream << #a << a << '\n'

static ofstream MyStream;


void MapMatchingMHT::GetSectionsOfRoute(const NetworkRoute& rNetworkRoute,
        const Region& rRegion, std::vector<NetworkSection>& rVecSectRes)
{
    if (!rNetworkRoute.IsDefined() || !rRegion.IsDefined())
        return;

    const int nRouteID = rNetworkRoute.GetRouteID();
    const SimpleLine* pRouteCurve = rNetworkRoute.GetCurve();

    if (pRouteCurve != NULL &&
        pRouteCurve->IsDefined() &&
        MMUtil::Intersects(rRegion, *pRouteCurve))
    {
        MYVARTRACE(nRouteID);

        RouteInterval Interval(nRouteID, 0.0,
                std::numeric_limits<double>::max());

        std::vector<TupleId> vecSections;
        m_pNetwork->GetSectionsOfRoutInterval(&Interval, vecSections);

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
                    MYVARTRACE(Section.GetSectionID());
                    rVecSectRes.push_back(Section);
                }
            }
        }
    }
}

void MapMatchingMHT::GetInitialSectionCandidates(const Point& rPoint,
                                      std::vector<NetworkSection>& rVecSectRes)
{
    if (m_pNetwork == NULL || !m_pNetwork->IsDefined() || !rPoint.IsDefined())
        return;

    const double dScaleFactor = m_pNetwork->GetScalefactor();

    const double dDist = 0.001; // TODO Radius 750 Meter
    const Rectangle<2> BBox(true, rPoint.GetX() - dDist * dScaleFactor,
                                  rPoint.GetX() + dDist * dScaleFactor,
                                  rPoint.GetY() - dDist * dScaleFactor,
                                  rPoint.GetY() + dDist * dScaleFactor);
    Region RegionBBox(BBox);

    R_TreeLeafEntry<2, TupleId> res;
    if (m_pNetwork->GetRTree()->First(BBox, res))
    {
        NetworkRoute Route(m_pNetwork->GetRoutes()->GetTuple(res.info, false),
                false);
        GetSectionsOfRoute(Route, RegionBBox, rVecSectRes);
    }

    while (m_pNetwork->GetRTree()->Next(res))
    {
        NetworkRoute Route(m_pNetwork->GetRoutes()->GetTuple(res.info, false),
                false);
        GetSectionsOfRoute(Route, RegionBBox, rVecSectRes);
    }
}

bool MapMatchingMHT::DoMatch(MGPoint* pResMGPoint)
{
    MyStream.open("/home/secondo/MyTrace.txt", ios_base::out | ios_base::app);

    // Initialization
    if (!InitMapMatching(pResMGPoint))
    {
        return false;
    }

    // Step 1 - Subdividing trip
    vector<MPoint*> vecTripParts;
    TripSegmentation(vecTripParts);

    // Steps 2 + 3
    for (vector<MPoint*>::iterator it = vecTripParts.begin();
         it != vecTripParts.end();
         ++it)
    {
        MPoint* pMPoint = *it;
        if (pMPoint != NULL)
        {
            // Step 2 - Determination of initial route/segment candidates

            UPoint FirstUPoint(false);
            int i = 0;
            for (i = 0;
                 i < pMPoint->GetNoComponents() && !FirstUPoint.IsDefined();
                 ++i)
            {
                pMPoint->Get(i, FirstUPoint);
            }

            std::vector<NetworkSection> vecInitialSections;
            if (FirstUPoint.IsDefined())
                GetInitialSectionCandidates(FirstUPoint.p0, vecInitialSections);

            for (std::vector<NetworkSection>::iterator it =
                 vecInitialSections.begin(); it != vecInitialSections.end();
                 ++it)
            {
                const NetworkSection& rActInitalNetworkSection = *it;
                if (!rActInitalNetworkSection.IsDefined())
                    continue;


            }

            // Step 3 - Route developement
            for (i = i + 1; i < pMPoint->GetNoComponents(); ++i)
            {

            }

            // cleanup
            *it = NULL;
            pMPoint->DeleteIfAllowed();
            pMPoint = NULL;
        }
    }

    // Finalize
    FinalizeMapMatching();

    MyStream.close();

    return true;
}


void MapMatchingMHT::TripSegmentation(std::vector<MPoint*>& rvecTripParts)
{
    if (m_pMPoint == NULL || m_pNetwork == NULL)
        return;

    // Detect spatial and temporal gaps in the MPoint (m_pMPoint)
    // Divide the MPoint if the time gap is longer than 120 seconds or
    // the distance is larger than 500 meters

    const double dMaxDistance = 500.0; // 500 meter
    const DateTime MaxTimeDiff(instanttype, 120000); // 2 minutes
    const double dScale = m_pNetwork->GetScalefactor();
    const Geoid  GeodeticSystem(Geoid::WGS1984);

    MPoint*  pActMPoint = NULL;
    UPoint   ActUPoint(false);
    DateTime prevEndTime(instanttype);
    Point    prevEndPoint(false);
    bool     bProcessNext = true;

    for (int i = 0; i < m_pMPoint->GetNoComponents(); bProcessNext ? i++ : i)
    {
        if (bProcessNext)
            m_pMPoint->Get(i, ActUPoint);

        bProcessNext = true;

        if (!ActUPoint.IsDefined())
        {
          ++i; // process next unit
          continue;
        }

        if (pActMPoint == NULL)
        {
            // create new MPoint
            pActMPoint = new MPoint((m_pMPoint->GetNoComponents() - i) / 4);
            pActMPoint->StartBulkLoad();

            // Check current UPoint
            CcReal Distance(0.0);
            ActUPoint.Length(GeodeticSystem, Distance);
            if (ActUPoint.timeInterval.end - ActUPoint.timeInterval.start
                    > MaxTimeDiff || Distance.GetRealval() > dMaxDistance )
            {
                // gap within UPoint
                // only use p1
                ActUPoint.p0 = ActUPoint.p1;
                ActUPoint.timeInterval.start = ActUPoint.timeInterval.end;
            }

            // Add unit
            pActMPoint->Add(ActUPoint);
            prevEndTime = ActUPoint.timeInterval.end;
            prevEndPoint = (ActUPoint.p1 * (1 / dScale) /* ->lat/lon */);
        }
        else
        {
            bool bValid = true;
            if (ActUPoint.timeInterval.start - prevEndTime > MaxTimeDiff ||
                prevEndPoint.DistanceOrthodrome(ActUPoint.p0 * (1 / dScale),
                                                GeodeticSystem,
                                                bValid) > dMaxDistance)
            {
                // gap detected -> finalize current MPoint
                pActMPoint->EndBulkLoad(false, false);
                if (pActMPoint->GetNoComponents() >= 10)
                {
                    rvecTripParts.push_back(pActMPoint);
                    pActMPoint = NULL;
                }
                else
                {
                    // less than 10 components -> drop
                    pActMPoint->DeleteIfAllowed();
                    pActMPoint = NULL;
                }

                bProcessNext = false; // Process ActUPoint once again
            }
            else
            {
                // no gap between current and previous UPoint

                // Check current UPoint
                CcReal Distance(0.0);
                ActUPoint.Length(GeodeticSystem, Distance);
                if (ActUPoint.timeInterval.end - ActUPoint.timeInterval.start
                        > MaxTimeDiff || Distance.GetRealval() > dMaxDistance)
                {
                    // gap within UPoint
                    // Assign p0 to current MPoint and p1 to new MPoint

                    Interval<Instant> I(ActUPoint.timeInterval);
                    I.end = I.start;
                    pActMPoint->Add(UPoint(I, ActUPoint.p0, ActUPoint.p1));

                    // finalize current MPoint
                    pActMPoint->EndBulkLoad(false, false);
                    if (pActMPoint->GetNoComponents() >= 10)
                    {
                        rvecTripParts.push_back(pActMPoint);
                        pActMPoint = NULL;
                    }
                    else
                    {
                        // less than 10 components -> drop
                        pActMPoint->DeleteIfAllowed();
                        pActMPoint = NULL;
                    }

                    ActUPoint.p0 = ActUPoint.p1;
                    ActUPoint.timeInterval.start = ActUPoint.timeInterval.end;

                    bProcessNext = false; // Process ActUPoint once again
                }
                else
                {
                    pActMPoint->Add(ActUPoint);
                    prevEndTime = ActUPoint.timeInterval.end;
                    prevEndPoint =
                            (ActUPoint.p1 * (1 / dScale) /* ->lat/lon */);
                }
            }
        }
    }

    // finalize last MPoint
    if (pActMPoint != NULL)
    {
        pActMPoint->EndBulkLoad(false, false);
        rvecTripParts.push_back(pActMPoint);
    }
}


} // end of namespace mapmatch



