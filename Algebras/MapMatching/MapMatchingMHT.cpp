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

#define MYTRACE(a) //MyStream << a << '\n'
#define MYVARTRACE(a) //MyStream << #a << a << '\n'

static ofstream MyStream;


void MapMatchingMHT::GetSectionsOfRoute(const NetworkRoute& rNetworkRoute,
        const Region& rRegion, std::vector<NetworkSection>& rVecSectRes)
{
    MYTRACE("GetSectionsOfRoute1");

    if (!rNetworkRoute.IsDefined() || !rRegion.IsDefined())
        return;

    MYTRACE("GetSectionsOfRoute2");

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

    const double dScaleFactor = 1000.0;//TODO m_pNetwork->GetScalefactor();

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
        MYVARTRACE(Route.GetRouteID());
        GetSectionsOfRoute(Route, *pRegionBBox, rVecSectRes);
    }

    while (m_pNetwork->GetRTree()->Next(res))
    {
        NetworkRoute Route(m_pNetwork->GetRoutes()->GetTuple(res.info, false),
                false);
        MYVARTRACE(Route.GetRouteID());
        GetSectionsOfRoute(Route, *pRegionBBox, rVecSectRes);
    }
}

class RouteCandidate
{
public:
    RouteCandidate();
    RouteCandidate(const RouteCandidate& rCandidate);
    ~RouteCandidate();

    RouteCandidate& operator=(const RouteCandidate& rCandidate);

    void AddSection(const NetworkSection& rSection);
    size_t GetSectionCount(void) const;
    const NetworkSection& GetSection(size_t nSection) const;
    const NetworkSection& GetLastSection(void) const;

    void AddPoint(const GPoint& rPoint, const DateTime& rDateTime,
                  bool bClosed, const double dDistance);

    inline double GetScore(void) const
    {
        return m_dScore;
    }

    // A empty section is a section without assigned points
    inline unsigned short GetCountLastEmptySections(void) const
    {
        return m_nCountLastEmptySections;
    }

    inline void MarkAsInvalid(void)
    {
        m_dScore = std::numeric_limits<double>::max();
    }

    struct PointData
    {
        PointData(const GPoint& rGPoint, const DateTime& rDateTime,
                  bool bClosed)
        :m_pGPoint(new GPoint(rGPoint)), m_Time(rDateTime), m_bClosed(bClosed)
        {
        }

        PointData(const PointData& rPointData)
        :m_pGPoint(new GPoint(rPointData.m_pGPoint != NULL ?
                                              (*rPointData.m_pGPoint) : false)),
         m_Time(rPointData.m_Time), m_bClosed(rPointData.m_bClosed)
        {
        }

        PointData& operator=(const PointData& rPointData)
        {
            if (this != &rPointData)
            {
                if (m_pGPoint != NULL)
                  m_pGPoint->DeleteIfAllowed();
                m_pGPoint = NULL;

                if (rPointData.m_pGPoint != NULL)
                    m_pGPoint = new GPoint(*rPointData.m_pGPoint);

                m_Time = rPointData.m_Time;
                m_bClosed = rPointData.m_bClosed;
            }

            return *this;
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

    inline const std::vector<PointData*>& GetPoints(void) const
    {
        return m_Points;
    }

    void GetPointsOfLastSection(std::vector<PointData*>& rvecPoints) const;

private:
    vector<PointData*> m_Points;
    vector<NetworkSection> m_Sections; // TODO ggf. DBArray
    double m_dScore;
    size_t m_nPointsOfLastSection;
    unsigned short m_nCountLastEmptySections;
};

RouteCandidate::RouteCandidate()
:m_dScore(0.0), m_nPointsOfLastSection(0), m_nCountLastEmptySections(0)
{
}

RouteCandidate::RouteCandidate(const RouteCandidate& rCandidate)
:m_dScore(rCandidate.m_dScore),
 m_nPointsOfLastSection(rCandidate.m_nPointsOfLastSection),
 m_nCountLastEmptySections(rCandidate.m_nCountLastEmptySections)
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

RouteCandidate& RouteCandidate::operator=(const RouteCandidate& rCandidate)
{
    if (this != &rCandidate)
    {
        m_dScore = rCandidate.m_dScore;
        m_nPointsOfLastSection = rCandidate.m_nPointsOfLastSection;
        m_nCountLastEmptySections = rCandidate.m_nCountLastEmptySections;

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

        for (size_t i = 0; i < rCandidate.m_Points.size(); ++i)
        {
            PointData* pData = rCandidate.m_Points[i];
            if (pData != NULL)
            {
                m_Points.push_back(new PointData(*pData));
            }
        }

        m_Sections.clear();
        for (size_t i = 0; i < rCandidate.m_Sections.size(); ++i)
        {
            m_Sections.push_back(rCandidate.m_Sections[i]);
        }
    }

    return *this;
}

void RouteCandidate::AddSection(const NetworkSection& rSection)
{
    m_Sections.push_back(rSection);
    m_nPointsOfLastSection = 0;
    ++m_nCountLastEmptySections;
}

size_t RouteCandidate::GetSectionCount(void) const
{
    return m_Sections.size();
}

const NetworkSection& RouteCandidate::GetSection(size_t nSection) const
{
    if (nSection < m_Sections.size())
        return m_Sections.at(nSection);
    else
    {
        static NetworkSection SectionUndef(NULL, NULL);
        return SectionUndef;
    }
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

void RouteCandidate::GetPointsOfLastSection(
                                      std::vector<PointData*>& rvecPoints) const
{
    if (m_nPointsOfLastSection > m_Points.size())
    {
        assert(false);
        return;
    }

    for (size_t i = m_nPointsOfLastSection; i > 0; --i)
    {
        rvecPoints.push_back(m_Points.at(m_Points.size() - i));
    }
}

void RouteCandidate::AddPoint(const GPoint& rPoint, const DateTime& rDateTime,
                              bool bClosed, const double dDistance)
{
    PointData* pData = new PointData(rPoint, rDateTime, bClosed);
    m_Points.push_back(pData);
    ++m_nPointsOfLastSection;
    m_dScore += dDistance;
    m_nCountLastEmptySections = 0;
}

bool MapMatchingMHT::DoMatch(MGPoint* pResMGPoint)
{
    //MyStream.open("/home/secondo/MyTrace.txt", ios_base::out | ios_base::app);

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
        AttributePtr<MPoint> pMPoint(*it);
        if (pMPoint == NULL)
            continue;
        *it = NULL;

        // Step 2 - Determination of initial route/segment candidates
        std::vector<RouteCandidate*> vecRouteCandidates;
        GetInitialRouteCandidates(pMPoint.get(), vecRouteCandidates);

        // Step 3 - Route developement
        DevelopRoutes(pMPoint.get(), vecRouteCandidates);

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

    //MyStream.close();

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

    for (size_t i = 0; i < rvecRouteCandidates.size(); ++i)
    {
        RouteCandidate* pCandidate = rvecRouteCandidates[i];

        MYVARTRACE("****** RouteCandidate ******");

        for (size_t j = 0; j < pCandidate->GetSectionCount(); ++j)
        {
            MYVARTRACE(pCandidate->GetSection(j).GetSectionID());
        }
    }
}

void MapMatchingMHT::DevelopRoutes(MPoint* pMPoint,
                              std::vector<RouteCandidate*>& rvecRouteCandidates)
{
    if (pMPoint == NULL || !pMPoint->IsDefined() ||
        m_pNetwork == NULL || !m_pNetwork->IsDefined())
        return;

    const int nNoComponents = pMPoint->GetNoComponents();

    for (int i = 0; i < 60 /*nNoComponents*/; ++i)
    {
        UPoint ActUPoint(false);
        pMPoint->Get(i, ActUPoint);
        if (!ActUPoint.IsDefined())
            continue;

        MYVARTRACE(i);

        DevelopRoutes(ActUPoint.p0, ActUPoint.timeInterval.start,
                      ActUPoint.timeInterval.lc, rvecRouteCandidates);

        MYVARTRACE(rvecRouteCandidates.size());

        DevelopRoutes(ActUPoint.p1, ActUPoint.timeInterval.end,
                      ActUPoint.timeInterval.rc, rvecRouteCandidates);

        ReduceRouteCandidates(rvecRouteCandidates);

        MYVARTRACE(rvecRouteCandidates.size());
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
        {
            continue;
        }

        if (!AssignPoint(pCandidate, rPoint, rTime, bClosed))
        {
            // Point could not be assigned to last section of candidate
            // -> add adjacent sections

            // max 5 sections look ahead
            if (pCandidate->GetCountLastEmptySections() < 5)
            {
                std::vector<RouteCandidate*> vecNewRouteCandidatesLocal;

                const NetworkSection& rSection = pCandidate->GetLastSection();
                if (!rSection.IsDefined())
                {
                    *it = NULL;
                    delete pCandidate;
                    continue;
                }

                const SimpleLine* pCurve = rSection.GetCurve();
                if (pCurve == NULL || !pCurve->IsDefined())
                {
                    *it = NULL;
                    delete pCandidate;
                    continue;
                }

                const Point ptStart = pCurve->StartPoint();
                const Point ptEnd = pCurve->EndPoint();

                const double dDistanceStart = rPoint.Distance(ptStart);
                const double dDistanceEnd = rPoint.Distance(ptEnd);

                //if (AlmostEqual(dDistanceStart, dDistanceEnd)) TODO

                /*MYTRACE("### pCandidate ###");
                for (size_t j = 0; j < pCandidate->GetPoints().size(); ++j)
                {
                    pCandidate->GetPoints()[j]->m_pGPoint->Print(MyStream);
                }*/

                AddAdjacentSections(pCandidate,
                                    dDistanceStart > dDistanceEnd,
                                    vecNewRouteCandidatesLocal);

                /*MYTRACE("### Nach AddAdjacentSections ###");
                for (size_t i = 0; i < vecNewRouteCandidatesLocal.size(); ++i)
                {
                    RouteCandidate* pC = vecNewRouteCandidatesLocal[i];

                    for (size_t j = 0; j < pC->GetPoints().size(); ++j)
                    {
                        pC->GetPoints()[j]->m_pGPoint->Print(MyStream);
                    }
                }*/

                if (vecNewRouteCandidatesLocal.size() > 0)
                {
                    // !! Rekursion !!
                    DevelopRoutes(rPoint, rTime, bClosed,
                                  vecNewRouteCandidatesLocal);

                    vecNewRouteCandidates.insert(vecNewRouteCandidates.end(),
                                             vecNewRouteCandidatesLocal.begin(),
                                             vecNewRouteCandidatesLocal.end());
                    *it = NULL;
                }
                else
                {
                    *it = NULL;
                    delete pCandidate;
                }
            }
            else
            {
                *it = NULL;
                delete pCandidate;
            }
        }
        else
        {
            // add to new list
            vecNewRouteCandidates.push_back(pCandidate);
            // remove from current list
            *it = NULL;
        }
    }

    // TODO nur zum Testen
    for (size_t i = 0; i < rvecRouteCandidates.size(); ++i)
    {
        assert(rvecRouteCandidates[i] == NULL);
    }

    rvecRouteCandidates = vecNewRouteCandidates;
    vecNewRouteCandidates.clear();

    /*MYTRACE("###DevelopRoutesEnde###");
    for (size_t i = 0; i < rvecRouteCandidates.size(); ++i)
    {
        RouteCandidate* pC = rvecRouteCandidates[i];

        for (size_t j = 0; j < pC->GetPoints().size(); ++j)
        {
            pC->GetPoints()[j]->m_pGPoint->Print(MyStream);
        }
    }*/
}

bool MapMatchingMHT::AssignPoint(RouteCandidate* pCandidate,
                                 const Point& rPoint,
                                 const datetime::DateTime& rTime,
                                 bool bClosed)
{
    const NetworkSection& rSection = pCandidate->GetLastSection();
    if (!rSection.IsDefined() || m_pNetwork == NULL)
    {
        return false;
    }

    const SimpleLine* pCurve = rSection.GetCurve();
    if (pCurve == NULL || !pCurve->IsDefined())
    {
        return false;
    }

    double dDistance = 0.0;
    Point PointProjection = MMUtil::CalcOrthogonalProjection(*pCurve, rPoint,
                                                             dDistance);
    if (PointProjection.IsDefined() /*&& dDistance < XXX TODO */)
    {
        // TODO prüfen, ob Länge der Punkte länger als Section ist.

        const NetworkRoute& rRoute = rSection.GetRoute();
        bool startSmaller = rRoute.GetStartsSmaller();
        const SimpleLine* pRouteCurve = rRoute.GetCurve();

        double dPos = 0.0;
        if (pRouteCurve != NULL &&
            pRouteCurve->AtPoint(PointProjection, startSmaller, dPos))
        {
            GPoint ResGPoint(true, m_pNetwork->GetId(), rRoute.GetRouteID(),
                    dPos, None);
            pCandidate->AddPoint(ResGPoint, rTime, bClosed, dDistance);
            return true;
        }
        else
        {
            return false;
        }
    }

    return false;
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

    MYTRACE("ReduceRouteCandidates Start");
    MYVARTRACE(rvecRouteCandidates.size());

    // maximum 30
    while (rvecRouteCandidates.size() > 30)
    {
        RouteCandidate* pCandidate = rvecRouteCandidates.back();
        delete pCandidate;
        rvecRouteCandidates.pop_back();
    }

    MYTRACE("ReduceRouteCandidates Mitte");
    MYVARTRACE(rvecRouteCandidates.size());

    // remove candidates with very bad score
    RouteCandidate* pBestCandidate = rvecRouteCandidates[0];
    const double dBestScore = pBestCandidate->GetScore();

    while (rvecRouteCandidates.size() > 25)
    {
        RouteCandidate* pCandidate = rvecRouteCandidates.back();
        if (pCandidate == NULL || pCandidate->GetScore() > (3 * dBestScore))
        {
            delete pCandidate;
            rvecRouteCandidates.pop_back();
        }
        else
            break;
    }

    MYTRACE("ReduceRouteCandidates Ende");
    MYVARTRACE(rvecRouteCandidates.size());
    MyStream.flush();
}

void MapMatchingMHT::AddAdjacentSections(RouteCandidate* pCandidate,
                           bool bUpDown,
                           std::vector<RouteCandidate*>& rvecNewRouteCandidates)
{
    if (m_pNetwork == NULL)
        return;

    const NetworkSection& rSection = pCandidate->GetLastSection();

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

            /* erst mal nicht wiederverwenden
            if (i == adjSectionList.size() - 1)
            {
                // last adjacent section
                // -> reuse current candidate
                pCandidate->AddSection(adjSection);
                rvecNewRouteCandidates.push_back(pCandidate);
            }
            else*/
            {
                if (pCandidate->GetPoints().size() > 0)
                {
                    double d = 6;
                    d = d*d;

                }
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
    RouteCandidate::PointData* pLastPointOfPrevSection = NULL;

    for (size_t i = 0; i < rvecRouteSegments.size(); ++i)
    {
        bool bCalcShortestPath = (i > 0);

        RouteCandidate* pCandidate = rvecRouteSegments[i];
        if (pCandidate != NULL)
        {
            const std::vector<RouteCandidate::PointData*>& rPoints =
                                                        pCandidate->GetPoints();

            const size_t nPoints = rPoints.size();

            // Find first defines point
            RouteCandidate::PointData* pData1 = NULL;
            size_t j = 0;
            while(j < nPoints && (pData1 == NULL || pData1->m_pGPoint == NULL))
            {
                pData1 = rPoints[j];
                ++j;
            }

            // Process next points
            while (j < nPoints && pData1 != NULL && pData1->m_pGPoint != NULL)
            {
                RouteCandidate::PointData* pData2 = NULL;

                while (j < nPoints &&
                       (pData2 == NULL || pData2->m_pGPoint == NULL))
                {
                    pData2 = rPoints[j];
                    ++j;
                }

                if (bCalcShortestPath && pLastPointOfPrevSection != NULL)
                {
                    // Calculate ShortestPAth between last point of previous
                    // segment and first point of this segment
                    CalcShortestPath(pLastPointOfPrevSection->m_pGPoint,
                            pData1->m_pGPoint, pLastPointOfPrevSection->m_Time,
                            pData1->m_Time);

                    bCalcShortestPath = false;
                }

                if (pData2 != NULL && pData2->m_pGPoint != NULL)
                {
                    const Interval<Instant> timeInterval(pData1->m_Time,
                                                         pData2->m_Time,
                                                         pData1->m_bClosed,
                                                         pData2->m_bClosed);

                    ConnectPoints(*(pData1->m_pGPoint),
                                  *(pData2->m_pGPoint),
                                  timeInterval);

                   pLastPointOfPrevSection = pData2;
                }

                pData1 = pData2;
            }
        }
    }
}

} // end of namespace mapmatch

