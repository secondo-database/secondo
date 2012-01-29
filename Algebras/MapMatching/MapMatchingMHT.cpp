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
            NetworkSection Section(m_pNetwork->GetSection(*it),
                                   m_pNetwork, false);
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
    AttributePtr<Region> pRegionBBox(new Region(BBox));

    R_TreeLeafEntry<2, TupleId> res;
    if (m_pNetwork->GetRTree()->First(BBox, res))
    {
        NetworkRoute Route(m_pNetwork->GetRoutes()->GetTuple(res.info, false),
                false);
        GetSectionsOfRoute(Route, *pRegionBBox, rVecSectRes);
    }

    while (m_pNetwork->GetRTree()->Next(res))
    {
        NetworkRoute Route(m_pNetwork->GetRoutes()->GetTuple(res.info, false),
                false);
        GetSectionsOfRoute(Route, *pRegionBBox, rVecSectRes);
    }
}

class RouteCandidate
{
public:
    RouteCandidate();
    RouteCandidate(const RouteCandidate& rCandidate);
    ~RouteCandidate();

    void AddSection(const NetworkSection& rSection);
    size_t GetSectionCount(void) const;
    const NetworkSection& GetLastSection(void) const;

    void AddPoint(const GPoint& rPoint, const DateTime& rDateTime,
                  bool bClosed, const double dDistance);

    inline double GetScore(void) const
    {
        return m_dScore;
    }

    struct PointData
    {
        PointData(const GPoint& rGPoint, const DateTime& rDateTime,
                  bool bClosed)
        :m_pGPoint(new GPoint(rGPoint)), m_Time(rDateTime),
                   m_bClosed(bClosed)
        {
        }

        ~PointData()
        {
            if (m_pGPoint != NULL)
                m_pGPoint->DeleteIfAllowed();
            m_pGPoint = NULL;
        }

        GPoint* m_pGPoint;
        DateTime m_Time;
        bool m_bClosed;
    };

    inline const std::vector<PointData*>& GetPoints(void)
    {
        return m_Points;
    }

private:
    vector<PointData*> m_Points;
    vector<NetworkSection> m_Sections; // TODO ggf. DBArray
    double m_dScore;
};

RouteCandidate::RouteCandidate()
:m_dScore(0.0)
{
}

RouteCandidate::RouteCandidate(const RouteCandidate& rCandidate)
:m_dScore(rCandidate.m_dScore)
{
    for (size_t i = 0; i < rCandidate.m_Points.size(); ++i)
    {
        PointData* pData = rCandidate.m_Points[i];
        if (pData != NULL)
        {
            m_Points.push_back(new PointData(*pData));
        }
    }

    for (size_t i = 0; i < rCandidate.m_Sections.size(); ++i)
    {
        m_Sections.push_back(rCandidate.m_Sections[i]);
    }
}

RouteCandidate::~RouteCandidate()
{
    for (size_t i = 0; i < m_Points.size(); ++i)
    {
        PointData* pPointData = m_Points[i];
        if (pPointData != NULL)
        {
            delete pPointData;
            pPointData = NULL;
        }

        m_Points[i] = NULL;
    }

    m_Points.clear();
}

void RouteCandidate::AddSection(const NetworkSection& rSection)
{
    m_Sections.push_back(rSection);
}

size_t RouteCandidate::GetSectionCount(void) const
{
    return m_Sections.size();
}

const NetworkSection& RouteCandidate::GetLastSection(void) const
{
    if (m_Sections.size() > 0)
        return m_Sections.at(m_Sections.size() - 1);
    else
    {
        static NetworkSection SectionUndef(NULL, NULL);
        return SectionUndef;
    }
}

void RouteCandidate::AddPoint(const GPoint& rPoint, const DateTime& rDateTime,
                              bool bClosed, const double dDistance)
{
    m_Points.push_back(new PointData(rPoint, rDateTime, bClosed));
    m_dScore += dDistance;
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
    vector<MPoint*> vecTripSegments;
    TripSegmentation(vecTripSegments);

    // Steps 2-4
    std::vector<RouteCandidate*> vecRouteSegments;

    for (vector<MPoint*>::iterator it = vecTripSegments.begin();
         it != vecTripSegments.end();
         ++it)
    {
        MPoint* pMPoint = *it;
        if (pMPoint == NULL)
            continue;
        *it = NULL;

        // Step 2 - Determination of initial route/segment candidates
        std::vector<RouteCandidate*> vecRouteCandidates;
        GetInitialRouteCandidates(pMPoint, vecRouteCandidates);

        // Step 3 - Route developement
        DevelopRoutes(pMPoint, vecRouteCandidates);

        // Step 4 - Selection of most likely candidate
        RouteCandidate* pBestCandidate =
                                DetermineBestRouteCandidate(vecRouteCandidates);
        vecRouteSegments.push_back(pBestCandidate);

        // cleanup
        while (vecRouteCandidates.size() > 0)
        {
            RouteCandidate* pCandidate = vecRouteCandidates.back();
            if (pCandidate != pBestCandidate)
                delete pCandidate;
            vecRouteCandidates.pop_back();
        }

        pMPoint->DeleteIfAllowed(); pMPoint = NULL;
    }

    // Step 6 - Treatment of gaps between trip segments
    CreateCompleteRoute(vecRouteSegments);

    // cleanup
    while (vecRouteSegments.size() > 0)
    {
        RouteCandidate* pCandidate = vecRouteSegments.back();
        delete pCandidate;
        vecRouteSegments.pop_back();
    }

    // Finalize
    FinalizeMapMatching();

    MyStream.close();

    return true;
}


void MapMatchingMHT::TripSegmentation(std::vector<MPoint*>& rvecTripParts)
{
    if (m_pMPoint == NULL || !m_pMPoint->IsDefined() ||
        m_pNetwork == NULL || !m_pNetwork->IsDefined())
        return;

    // Detect spatial and temporal gaps in the MPoint (m_pMPoint)
    // Divide the MPoint if the time gap is longer than 120 seconds or
    // the distance is larger than 500 meters

    const double dMaxDistance = 500.0; // 500 meter
    const DateTime MaxTimeDiff(durationtype, 120000); // 2 minutes
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

void MapMatchingMHT::GetInitialRouteCandidates(MPoint* pMPoint,
                              std::vector<RouteCandidate*>& rvecRouteCandidates)
{
    if (pMPoint == NULL || !pMPoint->IsDefined())
        return;

    // get first (defined) point
    UPoint FirstUPoint(false);
    for (int i = 0; i < pMPoint->GetNoComponents() && !FirstUPoint.IsDefined();
            ++i)
    {
        pMPoint->Get(i, FirstUPoint);
    }

    // get first section candidates
    std::vector<NetworkSection> vecInitialSections;
    if (FirstUPoint.IsDefined())
        GetInitialSectionCandidates(FirstUPoint.p0, vecInitialSections);

    // process first section candidates
    // create route candidates
    for (std::vector<NetworkSection>::iterator it = vecInitialSections.begin();
            it != vecInitialSections.end(); ++it)
    {
        const NetworkSection& rActInitalNetworkSection = *it;
        if (!rActInitalNetworkSection.IsDefined())
            continue;

        RouteCandidate* pCandidate = new RouteCandidate;
        pCandidate->AddSection(rActInitalNetworkSection);

        rvecRouteCandidates.push_back(pCandidate);
    }
}

void MapMatchingMHT::DevelopRoutes(MPoint* pMPoint,
                              std::vector<RouteCandidate*>& rvecRouteCandidates)
{
    if (pMPoint == NULL || !pMPoint->IsDefined() ||
        m_pNetwork == NULL || !m_pNetwork->IsDefined())
        return;

    std::vector<RouteCandidate*> vecNewRouteCandidates;

    for (int i = 0; i < pMPoint->GetNoComponents(); ++i)
    {
        UPoint ActUPoint(false);
        pMPoint->Get(i, ActUPoint);
        if (!ActUPoint.IsDefined())
            continue;

        DevelopRoutes(ActUPoint.p0, ActUPoint.timeInterval.start,
                      ActUPoint.timeInterval.lc, rvecRouteCandidates);

        DevelopRoutes(ActUPoint.p1, ActUPoint.timeInterval.end,
                      ActUPoint.timeInterval.rc, rvecRouteCandidates);

        ReduceRouteCandidates(rvecRouteCandidates);
    }
}

void MapMatchingMHT::DevelopRoutes(const Point& rPoint,
                             const DateTime& rTime,
                             bool bClosed,
                             std::vector<RouteCandidate*>& rvecRouteCandidates)
{
    std::vector<RouteCandidate*> vecNewRouteCandidates;

    std::vector<RouteCandidate*>::iterator it;
    for (it = rvecRouteCandidates.begin();
         it != rvecRouteCandidates.end();
         ++it)
    {
        RouteCandidate* pCandidate = *it;
        if (pCandidate == NULL)
            continue;

        const NetworkSection& rSection = pCandidate->GetLastSection();
        if (!rSection.IsDefined()) // TODO Kandidat ist ungültig
            continue;

        const SimpleLine* pCurve = rSection.GetCurve();
        if (pCurve == NULL || !pCurve->IsDefined()) // TODO
            continue;

        double dDistance = 0.0;
        Point PointProjection = MMUtil::CalcOrthogonalProjection(*pCurve,
                                                             rPoint, dDistance);
        if (PointProjection.IsDefined())
        {
            const NetworkRoute& rRoute = rSection.GetRoute();
            bool startSmaller = rRoute.GetStartsSmaller();
            const SimpleLine* pRouteCurve = rRoute.GetCurve();

            double dPos = 0.0;
            if (pRouteCurve != NULL &&
                pRouteCurve->AtPoint(PointProjection, startSmaller, dPos))
            {
                GPoint ResGPoint(true, m_pNetwork->GetId(),
                                 rRoute.GetRouteID(), dPos, None);
                pCandidate->AddPoint(ResGPoint, rTime, bClosed, dDistance);
            }

            *it = NULL;
            vecNewRouteCandidates.push_back(pCandidate);
        }
        else
        {
            //pCurve->

            if (1 /*wenn erkannt wurde, dass das Ende erreicht wurde*/)
            {
                AddAdjacentSections(pCandidate, vecNewRouteCandidates);
                if (vecNewRouteCandidates.size() > 0)
                {
                    *it = NULL;
                }
            }
        }
    }

    rvecRouteCandidates.clear(); // TODO prüfen, ob alle NULL sind

    rvecRouteCandidates = vecNewRouteCandidates;
}

static bool RouteCandidateCompare(const RouteCandidate* pRC1,
                                  const RouteCandidate* pRC2)
{
    // RC1 < RC2
    if (pRC1 == NULL || pRC1 == NULL)
        return pRC1 < pRC2;
    return pRC1->GetScore() < pRC2->GetScore();
}


void MapMatchingMHT::ReduceRouteCandidates(std::vector<RouteCandidate*>&
                                                            rvecRouteCandidates)
{
    if (rvecRouteCandidates.size() <= 25) // minimum 25 candidates
        return;

    std::sort(rvecRouteCandidates.begin(),
              rvecRouteCandidates.end(),
              RouteCandidateCompare);

    // maximum 30
    while (rvecRouteCandidates.size() > 30)
    {
        RouteCandidate* pCandidate = rvecRouteCandidates.back();
        delete pCandidate;
        rvecRouteCandidates.pop_back();
    }

    // remove candidates with very bad score
    RouteCandidate* pBestCandidate = rvecRouteCandidates[0];
    const double dBestScore = pBestCandidate->GetScore();

    while (rvecRouteCandidates.size() > 25)
    {
        RouteCandidate* pCandidate = rvecRouteCandidates.back();
        if (pCandidate == NULL || pCandidate->GetScore() > (5 * dBestScore))
            rvecRouteCandidates.pop_back();
        else
            break;
    }
}

void MapMatchingMHT::AddAdjacentSections(RouteCandidate* pCandidate,
                           std::vector<RouteCandidate*>& rvecNewRouteCandidates)
{
    if (m_pNetwork == NULL)
        return;

    const NetworkSection& rSection = pCandidate->GetLastSection();

    bool bUpDown = true;//pCandidate->MovingUp(); // TODO
    vector<DirectedSection> adjSectionList;
    m_pNetwork->GetAdjacentSections(rSection.GetSectionID(), bUpDown,
                                    adjSectionList);

    if (adjSectionList.size() > 0)
    {
        for (size_t i = 0; i < adjSectionList.size(); i++)
        {
            Tuple* pSectionTuple = m_pNetwork->GetSection(
                                             adjSectionList[i].GetSectionTid());
            NetworkSection adjSection(pSectionTuple, m_pNetwork, false);

            if (i == adjSectionList.size() - 1)
            {
                // last adjacent section
                // -> reuse current candidate
                pCandidate->AddSection(adjSection);
                rvecNewRouteCandidates.push_back(pCandidate);
            }
            else
            {
                // make copy of current candidate
                RouteCandidate* pNewCandidate = new RouteCandidate(*pCandidate);
                pNewCandidate->AddSection(adjSection);

                rvecNewRouteCandidates.push_back(pNewCandidate);
            }
        }
    }
    else
    {
        // no adjacent sections
        MYTRACE("no adjacent sections");
        MYVARTRACE(rSection.GetSectionID());
    }
}

RouteCandidate* MapMatchingMHT::DetermineBestRouteCandidate(
                        std::vector<RouteCandidate*>& rvecRouteCandidates)
{
    std::sort(rvecRouteCandidates.begin(),
              rvecRouteCandidates.end(),
              RouteCandidateCompare);

    if (rvecRouteCandidates.size() > 0)
        return rvecRouteCandidates.front();
    else
        return NULL;
}

void MapMatchingMHT::CreateCompleteRoute(
                          const std::vector<RouteCandidate*>& rvecRouteSegments)
{
    GPoint* pPrevGPoint = NULL;

    for (size_t i = 0; i < rvecRouteSegments.size(); ++i)
    {
        if (pPrevGPoint != NULL)
        {
        }

        RouteCandidate* pCandidate = rvecRouteSegments[i];
        if (pCandidate != NULL)
        {
            const std::vector<RouteCandidate::PointData*>& rPoints =
                                                        pCandidate->GetPoints();

            for (size_t j = 0; j < rPoints.size(); ++j)
            {
                RouteCandidate::PointData* pData = rPoints[i];
                if (pData == NULL)
                    continue;

                /*pData->m_pGPoint;
                pData->m_Time;
                pData->m_bClosed;*/
            }
        }
    }
}

} // end of namespace mapmatch

