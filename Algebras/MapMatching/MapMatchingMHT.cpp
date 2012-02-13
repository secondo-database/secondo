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
#include "MHTRouteCandidate.h"

#include <stdio.h>

#include "../Network/NetworkAlgebra.h"
#include "../TemporalNet/TemporalNetAlgebra.h"

namespace mapmatch {

#define MYTRACE(a) //MyStream << a << '\n'; MyStream.flush()
#define MYVARTRACE(a) //MyStream << #a << a << '\n'; MyStream.flush()

static ofstream MyStream("/home/secondo/Traces/Trace.txt");

/*
3 class MapMatchingMHT
  Map matching algorithm based on the Multiple Hypothesis Technique (MHT)

*/

MapMatchingMHT::MapMatchingMHT(Network* pNetwork, MPoint* pMPoint)
:MapMatchingBase(pNetwork, pMPoint), m_pGeoid(NULL /*new Geoid(Geoid::WGS1984*/)
{
}

// Destructor
MapMatchingMHT::~MapMatchingMHT()
{
    if (m_pGeoid != NULL)
        m_pGeoid->DeleteIfAllowed();
    m_pGeoid = NULL;
}

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
        GetSectionsOfRoute(Route, *pRegionBBox, rVecSectRes);
    }

    while (m_pNetwork->GetRTree()->Next(res))
    {
        NetworkRoute Route(m_pNetwork->GetRoutes()->GetTuple(res.info, false),
                false);
        GetSectionsOfRoute(Route, *pRegionBBox, rVecSectRes);
    }
}

bool MapMatchingMHT::DoMatch(MGPoint* pResMGPoint)
{
    // Initialization
    if (!InitMapMatching(pResMGPoint))
    {
        return false;
    }

    // Step 1 - Subdividing trip
    vector<MPoint*> vecTripSegments;
    TripSegmentation(vecTripSegments);

    // Steps 2-4
    std::vector<MHTRouteCandidate*> vecRouteSegments;

    for (vector<MPoint*>::iterator it = vecTripSegments.begin();
         it != vecTripSegments.end();
         ++it)
    {
        AttributePtr<MPoint> pMPoint(*it);
        if (pMPoint == NULL)
            continue;
        *it = NULL;

        // Step 2 - Determination of initial route/segment candidates
        std::vector<MHTRouteCandidate*> vecRouteCandidates;
        int nIndexFirstUPoint = GetInitialRouteCandidates(pMPoint.get(),
                                                          vecRouteCandidates);

        // Step 3 - Route developement
        DevelopRoutes(pMPoint.get(), nIndexFirstUPoint, vecRouteCandidates);

        // Step 4 - Selection of most likely candidate
        MHTRouteCandidate* pBestCandidate =
                                DetermineBestRouteCandidate(vecRouteCandidates);
        vecRouteSegments.push_back(pBestCandidate);

        // cleanup
        while (vecRouteCandidates.size() > 0)
        {
            MHTRouteCandidate* pCandidate = vecRouteCandidates.back();
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
        MHTRouteCandidate* pCandidate = vecRouteSegments.back();
        delete pCandidate;
        vecRouteSegments.pop_back();
    }

    // Finalize
    FinalizeMapMatching();

    MYTRACE("MapMatchingMHT - end");

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

    const int nMPointComponents = m_pMPoint->GetNoComponents();

    for (int i = 0; i < nMPointComponents; bProcessNext ? i++ : i)
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
                ActUPoint.timeInterval.lc = ActUPoint.timeInterval.rc = true;
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
                    I.lc = I.rc = true;
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
                    ActUPoint.timeInterval.lc =
                            ActUPoint.timeInterval.rc = true;

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

int MapMatchingMHT::GetInitialRouteCandidates(MPoint* pMPoint,
                           std::vector<MHTRouteCandidate*>& rvecRouteCandidates)
{
    if (pMPoint == NULL || !pMPoint->IsDefined())
        return -1;

    // Get first (defined) point and search for sections
    // in the vicinity of the point.
    // Process next (defined) point, if no section was found.
    UPoint FirstUPoint(false);
    std::vector<NetworkSection> vecInitialSections;
    int nIndexFirstUPoint = 0;

    while (vecInitialSections.size() == 0 &&
           nIndexFirstUPoint < pMPoint->GetNoComponents())
    {
        MYVARTRACE(nIndexFirstUPoint);

        pMPoint->Get(nIndexFirstUPoint, FirstUPoint);

        // get first section candidates
        if (FirstUPoint.IsDefined())
            GetInitialSectionCandidates(FirstUPoint.p0, vecInitialSections);

        if (vecInitialSections.size() == 0)
            ++nIndexFirstUPoint;
    }

    // process initial section candidates
    // -> create route candidates
    for (std::vector<NetworkSection>::iterator it = vecInitialSections.begin();
         it != vecInitialSections.end(); ++it)
    {
        const NetworkSection& rActInitalNetworkSection = *it;
        if (!rActInitalNetworkSection.IsDefined())
            continue;

        // Direction Up
        DirectedNetworkSection SectionUp(rActInitalNetworkSection,
                                         DirectedNetworkSection::DIR_UP);
        MHTRouteCandidate* pCandidateUp = new MHTRouteCandidate;
        pCandidateUp->AddSection(SectionUp);

        rvecRouteCandidates.push_back(pCandidateUp);

        // Direction Down
        DirectedNetworkSection SectionDown(rActInitalNetworkSection,
                                           DirectedNetworkSection::DIR_DOWN);
        MHTRouteCandidate* pCandidateDown = new MHTRouteCandidate;
        pCandidateDown->AddSection(SectionDown);

        rvecRouteCandidates.push_back(pCandidateDown);
    }

    return rvecRouteCandidates.size() > 0 ? nIndexFirstUPoint : -1;
}

void MapMatchingMHT::DevelopRoutes(MPoint* pMPoint, int nIndexFirstComponent,
                           std::vector<MHTRouteCandidate*>& rvecRouteCandidates)
{
    if (pMPoint == NULL || !pMPoint->IsDefined() ||
        m_pNetwork == NULL || !m_pNetwork->IsDefined() ||
        nIndexFirstComponent < 0)
        return;

    const int nNoComponents = pMPoint->GetNoComponents();

    for (int i = nIndexFirstComponent; i < nNoComponents; ++i)
    {
        //TracePoints << i << endl;

        UPoint ActUPoint(false);
        pMPoint->Get(i, ActUPoint);
        if (!ActUPoint.IsDefined())
            continue;

        // Develop routes with point p0
        DevelopRoutes(ActUPoint.p0, ActUPoint.timeInterval.start,
                      ActUPoint.timeInterval.lc, rvecRouteCandidates);

        TraceRouteCandidates(rvecRouteCandidates,
                             "##### Nach DevelopRoutes 1 #####");

        // Reduce Routes
        ReduceRouteCandidates(rvecRouteCandidates);
        TraceRouteCandidates(rvecRouteCandidates,
                             "##### Nach Reduce 1 #####");

        // Develop routes with point p1
        DevelopRoutes(ActUPoint.p1, ActUPoint.timeInterval.end,
                      ActUPoint.timeInterval.rc, rvecRouteCandidates);

        TraceRouteCandidates(rvecRouteCandidates,
                             "##### Nach DevelopRoutes 2 #####");

        // Reduce Routes
        ReduceRouteCandidates(rvecRouteCandidates);
        TraceRouteCandidates(rvecRouteCandidates,
                             "##### Nach Reduce 2 #####");
    }
}

void MapMatchingMHT::DevelopRoutes(const Point& rPoint,
                          const DateTime& rTime,
                          bool bClosed,
                          std::vector<MHTRouteCandidate*>& rvecRouteCandidates)
{
    std::vector<MHTRouteCandidate*> vecNewRouteCandidates;

    std::vector<MHTRouteCandidate*>::iterator it;
    for (it = rvecRouteCandidates.begin();
         it != rvecRouteCandidates.end();
         ++it)
    {
        MHTRouteCandidate* pCandidate = *it;
        if (pCandidate == NULL)
        {
            continue;
        }

        ENextCandidates eNextCandidates = CANDIDATES_NONE;
        if (!AssignPoint(pCandidate, rPoint, rTime, bClosed, eNextCandidates))
        {
            // Point could not be assigned to last section of candidate
            // -> add adjacent sections

            // max 5 sections look ahead
            // no look ahead when processing first point of route
            if (pCandidate->GetPoints().size() > 0 &&
                pCandidate->GetCountLastEmptySections() < 5)
            {
                std::vector<MHTRouteCandidate*> vecNewRouteCandidatesLocal;

                const DirectedNetworkSection& rSection =
                                                   pCandidate->GetLastSection();
                if (!rSection.IsDefined())
                {
                    *it = NULL;
                    delete pCandidate; pCandidate = NULL;
                    continue;
                }

                const SimpleLine* pCurve = rSection.GetCurve();
                if (pCurve == NULL || !pCurve->IsDefined())
                {
                    *it = NULL;
                    delete pCandidate; pCandidate = NULL;
                    continue;
                }

                const bool bStartsSmaller = rSection.GetCurveStartsSmaller();
                const Point ptStart = pCurve->StartPoint(bStartsSmaller);
                const Point ptEnd = pCurve->EndPoint(bStartsSmaller);

                const double dDistanceStart = rPoint.Distance(ptStart,
                                                              m_pGeoid);
                const double dDistanceEnd = rPoint.Distance(ptEnd, m_pGeoid);

                //if (AlmostEqual(dDistanceStart, dDistanceEnd)) TODO

                bool bUpDown = dDistanceStart > dDistanceEnd;

                if (pCandidate->GetCountPointsOfLastSection() == 0)
                {
                    // Don't go back to previous section,
                    // because this point already was processed
                    // with previous section(s)
                    if ((rSection.GetDirection() ==
                                  DirectedNetworkSection::DIR_UP && !bUpDown) ||
                         (rSection.GetDirection() ==
                                  DirectedNetworkSection::DIR_DOWN && bUpDown))
                    {
                        *it = NULL;
                        delete pCandidate; pCandidate = NULL;
                        continue;
                    }
                }

                AddAdjacentSections(pCandidate, bUpDown,
                                        vecNewRouteCandidatesLocal);

                /*if (bUpDown &&
                    rSection.GetDirection() != DirectedNetworkSection::DIR_UP)
                {
                    AddAdjacentSections(pCandidate, false,
                                        vecNewRouteCandidatesLocal);
                }
                else if (!bUpDown &&
                    rSection.GetDirection() != DirectedNetworkSection::DIR_DOWN)
                {
                    AddAdjacentSections(pCandidate, true,
                            vecNewRouteCandidatesLocal);
                }*/

                if (vecNewRouteCandidatesLocal.size() > 0)
                {
                    // !! Rekursion !!
                    DevelopRoutes(rPoint, rTime, bClosed,
                                  vecNewRouteCandidatesLocal);

                    vecNewRouteCandidates.insert(vecNewRouteCandidates.end(),
                                             vecNewRouteCandidatesLocal.begin(),
                                             vecNewRouteCandidatesLocal.end());
                }

                *it = NULL;

                if (pCandidate->GetCountLastEmptySections() == 0)
                {
                    // Only add GPS-Point to Candidate (no GPoint)
                    // Calculate Distance of last point to current point
                    /*double dDistance = 0.0;
                    if (pCandidate->GetPoints().size() > 0)
                    {
                        std::vector<PointData*>&vecPt = pCandidate->GetPoints();
                        MHTRouteCandidate::PointData* pData = vecPt.back();

                        if (pData->m_pGPoint != NULL &&
                            pData->m_pGPoint->IsDefined())
                        {
                            AttributePtr<Point> pPrevPoint(
                                                   pData->m_pGPoint->ToPoint());
                            dDistance = pPrevPoint->Distance(rPoint, m_pGeoid);
                        }
                        else if (pData->m_pPointGPS != NULL &&
                                 pData->m_pPointGPS->IsDefined())
                            dDistance = pData->
                                        m_pPointGPS->Distance(rPoint, m_pGeoid);
                        else
                            dDistance = pCurve->Distance(rPoint, m_pGeoid);
                    }
                    else
                    {
                        dDistance = pCurve->Distance(rPoint, m_pGeoid);
                    }*/

                    //pCandidate->AddPoint(rPoint, dDistance, rTime, bClosed);
                    pCandidate->AddPoint(rPoint, 30, rTime, bClosed); // TODO

                    vecNewRouteCandidates.push_back(pCandidate);
                }
                else
                {
                    delete pCandidate;
                }
            }
            else
            {
                // too many empty sections
                *it = NULL;
                delete pCandidate;
            }
        }
        else
        {
            // Point was assigned to candidate
            // add to new list
            vecNewRouteCandidates.push_back(pCandidate);
            // remove from current list
            *it = NULL;

            if (eNextCandidates != CANDIDATES_NONE)
            {
                // Point was assigned, but check adjacent sections, too.

                // Remove assigned point and assign to adjacent sections

                MHTRouteCandidate CandidateCopy(*pCandidate);
                CandidateCopy.RemoveLastPoint();

                std::vector<MHTRouteCandidate*> vecNewRouteCandidatesLocal;

                if (eNextCandidates == CANDIDATES_UP ||
                    eNextCandidates == CANDIDATES_UP_DOWN)
                {
                    AddAdjacentSections(&CandidateCopy, true,
                                        vecNewRouteCandidatesLocal);
                }

                if (eNextCandidates == CANDIDATES_DOWN ||
                    eNextCandidates == CANDIDATES_UP_DOWN)
                {
                    AddAdjacentSections(&CandidateCopy, false,
                                        vecNewRouteCandidatesLocal);
                }

                if (vecNewRouteCandidatesLocal.size() > 0)
                {
                    // !! Rekursion !!
                    DevelopRoutes(rPoint, rTime, bClosed,
                                  vecNewRouteCandidatesLocal);

                    vecNewRouteCandidates.insert(
                                vecNewRouteCandidates.end(),
                                vecNewRouteCandidatesLocal.begin(),
                                vecNewRouteCandidatesLocal.end());
                }
            }
        }
    }

    rvecRouteCandidates = vecNewRouteCandidates;
    vecNewRouteCandidates.clear();
}

// modified copy of SimpleLine::AtPoint
static bool GetPosOnSimpleLine( const SimpleLine& rLine,
                                const Point& p,
                                bool startsSmaller,
                                double tolerance,
                                double& result)

{
    if (rLine.IsEmpty() || !p.IsDefined())
    {
        return false;
    }

    bool found = false;
    HalfSegment hs;
    const Rectangle<2> rectBounding(true,
                                    p.GetX() - tolerance,
                                    p.GetX() + tolerance,
                                    p.GetY() - tolerance,
                                    p.GetY() + tolerance);

    for (int nPos = 0; nPos < rLine.Size(); ++nPos)
    {
        rLine.Get(nPos, hs);

        double dDistance = hs.Distance(rectBounding);

        if (hs.IsLeftDomPoint() && AlmostEqual(dDistance, 0.0))
        {
            found = true;
            break;
        }
    }

    if (found)
    {
        LRS lrs;
        rLine.Get(hs.attr.edgeno, lrs);
        rLine.Get(lrs.hsPos, hs);
        result = lrs.lrsPos + p.Distance(hs.GetDomPoint());

        if (!startsSmaller)
            result = rLine.Length() - result;

        /*if (tolerance != 0.0)
        {
            if (AlmostEqualAbsolute(result, 0.0, tolerance))
                result = 0;
            else if (AlmostEqualAbsolute(result, rLine.Length(), tolerance))
                result = rLine.Length();
        }
        else*/
        {
            if (AlmostEqual(result, 0.0))
                result = 0.0;
            else if (AlmostEqual(result, rLine.Length()))
                result = rLine.Length();
        }

        return true;
    }
    return false;
}

static double CalcDistance(const std::vector<const Point*>& rvecPoints,
                           const Geoid* pGeoid = NULL)
{
    double dDistance = 0.0;

    const size_t nSize = rvecPoints.size();
    if (nSize > 0)
    {
        const Point* pPrevPoint = rvecPoints[0];

        for (size_t i = 1; i < nSize; ++i)
        {
            const Point* pActPoint = rvecPoints[i];
            if (pActPoint != NULL)
            {
                if (pActPoint != NULL)
                    dDistance += pPrevPoint->Distance(*pActPoint, pGeoid);

                pPrevPoint = pActPoint;
            }
        }
    }

    return dDistance;
}

static double CalcLengthCurve(const SimpleLine* pCurve,
                              const Geoid* pGeoid = NULL)
{
    if (pGeoid != NULL)
    {
        bool bValid = true;
        double dLengthCurve = pCurve->Length(*pGeoid, bValid);
        if (bValid)
            return dLengthCurve;
        else
            return pCurve->Length();
    }
    else
    {
        return pCurve->Length();
    }
}

bool MapMatchingMHT::AssignPoint(MHTRouteCandidate* pCandidate,
                                 const Point& rPoint,
                                 const datetime::DateTime& rTime,
                                 bool bClosed,
                                 MapMatchingMHT::ENextCandidates& eNext)
{
    eNext = CANDIDATES_NONE;

    const DirectedNetworkSection& rSection = pCandidate->GetLastSection();
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
    bool bIsOrthogonal = false;
    Point PointProjection = MMUtil::CalcProjection(*pCurve, rPoint,
                                                   dDistance, bIsOrthogonal,
                                                   m_pGeoid);
    if (PointProjection.IsDefined())
    {
        // Check if the startnode or endpoint has been reached.
        const bool bStartsSmaller = rSection.GetCurveStartsSmaller();
        const Point ptStart = pCurve->StartPoint(bStartsSmaller);
        const Point ptEnd = pCurve->EndPoint(bStartsSmaller);

        const double dDistanceStart = rPoint.Distance(ptStart, m_pGeoid);
        const double dDistanceEnd = rPoint.Distance(ptEnd, m_pGeoid);

        // Never assign to endnode
        if (rSection.GetDirection() == DirectedNetworkSection::DIR_UP)
        {
            if (AlmostEqual(PointProjection, ptEnd))
                return false;
        }
        else if (rSection.GetDirection() == DirectedNetworkSection::DIR_DOWN)
        {
            if (AlmostEqual(PointProjection, ptStart))
                return false;
        }
        else
        {
            if (AlmostEqual(PointProjection, ptStart) ||
                AlmostEqual(PointProjection, ptEnd))
                return false;
        }

        // Do not assign to startnode if it is not orthogonal
        if (!bIsOrthogonal)
        {
            if (rSection.GetDirection() == DirectedNetworkSection::DIR_UP)
            {
                if (AlmostEqual(PointProjection, ptStart))
                    return false;
            }
            else
                if (rSection.GetDirection() == DirectedNetworkSection::DIR_DOWN)
            {
                if (AlmostEqual(PointProjection, ptEnd))
                    return false;
            }
        }

        // Check "length" of GPS-Points
        vector<MHTRouteCandidate::PointData*> vecPointsLastSection;
        pCandidate->GetPointsOfLastSection(vecPointsLastSection);
        const size_t nPointsLastSection = vecPointsLastSection.size();

        vector<const Point*> vecPoints;
        for (size_t i = 0; i < nPointsLastSection; ++i)
        {
            vecPoints.push_back(vecPointsLastSection[i]->GetPointGPS());
        }

        vecPoints.push_back(&rPoint);

        // Calculate travelled distance ('length' of GPS-Points)
        double dDistanceTravelled = CalcDistance(vecPoints, m_pGeoid);

        /* // Add Distance from Start-Point to first projected point
        Point* pFirstProjectedPoint = &PointProjection;
        if (nPointsLastSection > 0 && vecPointsLastSection[0] != NULL)
        {
            pFirstProjectedPoint = vecPointsLastSection[0]->m_pPointProjection;
        }

        if (rSection.GetDirection() == DirectedNetworkSection::DIR_UP &&
            pFirstProjectedPoint != NULL)
        {
            dDistanceTravelled += ptStart.Distance(*pFirstProjectedPoint,
                                                   m_pGeoid);
        }
        else if (rSection.GetDirection() == DirectedNetworkSection::DIR_DOWN &&
                 pFirstProjectedPoint != NULL)
        {
            dDistanceTravelled += ptEnd.Distance(*pFirstProjectedPoint,
                                                 m_pGeoid);
        }*/

        const double dLengthCurve = CalcLengthCurve(pCurve, m_pGeoid);

        // If traveled (GPS-)distance ist larger than 85%,
        // then look at adjacent sections, too
        if (dDistanceTravelled > (dLengthCurve / 100. * 85.))
        {
            if (dDistanceStart > dDistanceEnd)
                eNext = CANDIDATES_UP;
            else
                eNext = CANDIDATES_DOWN;

            if ((rSection.GetDirection() == DirectedNetworkSection::DIR_UP &&
                 eNext != CANDIDATES_UP) ||
                (rSection.GetDirection() == DirectedNetworkSection::DIR_DOWN &&
                 eNext != CANDIDATES_DOWN))
            {
                eNext = CANDIDATES_UP_DOWN;
            }

        }

        // Check Distance
        // If distance of projected point to GPS-Point is larger than
        // ?? meter, then look at adjacent sections, too.
        // TODO

        // Get Position of Projected point on route
        const NetworkRoute& rRoute = rSection.GetRoute();
        const bool RouteStartsSmaller = rRoute.GetStartsSmaller();
        const SimpleLine* pRouteCurve = rRoute.GetCurve();

        double dPos = 0.0;
        if (pRouteCurve != NULL &&
            GetPosOnSimpleLine(*pRouteCurve, PointProjection,
                               RouteStartsSmaller, 0.000001 * 1000, dPos))
            //pRouteCurve->AtPoint(PointProjection, RouteStartsSmaller, dPos))
        {
            GPoint ResGPoint(true, m_pNetwork->GetId(), rRoute.GetRouteID(),
                    dPos, None);
            pCandidate->AddPoint(ResGPoint, rPoint, PointProjection,
                                 dDistance, rTime, bClosed);

            return true;
        }
        else
        {
            // Projected point could not be matched onto route
            assert(false);
            return false;
        }
    }
    else
    {
        // Projection failed
        return false;
    }


    return false;
}

static bool RouteCandidateCompare(const MHTRouteCandidate* pRC1,
                                  const MHTRouteCandidate* pRC2)
{
    // RC1 < RC2
    if (pRC1 == NULL || pRC1 == NULL)
        return pRC1 < pRC2;
    return pRC1->GetScore() < pRC2->GetScore();
}

void MapMatchingMHT::ReduceRouteCandidates(std::vector<MHTRouteCandidate*>&
                                                            rvecRouteCandidates)
{
    if (rvecRouteCandidates.size() <= 25) // minimum 25 candidates
        return;

    std::sort(rvecRouteCandidates.begin(),
              rvecRouteCandidates.end(),
              RouteCandidateCompare);

    // Remove duplicates
    for (size_t i = 0; i < rvecRouteCandidates.size(); /*empty*/)
    {
        size_t j = i + 1;
        for (j = i + 1; j < rvecRouteCandidates.size(); ++j)
        {
            MHTRouteCandidate* pRouteCandidate1 = rvecRouteCandidates[i];
            MHTRouteCandidate* pRouteCandidate2 = rvecRouteCandidates[j];

            if (*pRouteCandidate1 == *pRouteCandidate2)
            {
                rvecRouteCandidates[j]->MarkAsInvalid();
            }
            else
            {
                // The candidates are sorted by score -
                // Candidates with different score are not equal
                break;
            }
        }
        i = j + 1;
    }

    std::sort(rvecRouteCandidates.begin(),
              rvecRouteCandidates.end(),
              RouteCandidateCompare);

    // maximum 30
    while (rvecRouteCandidates.size() > 30)
    {
        MHTRouteCandidate* pCandidate = rvecRouteCandidates.back();
        delete pCandidate;
        rvecRouteCandidates.pop_back();
    }

    // remove candidates with very bad score
    MHTRouteCandidate* pBestCandidate = rvecRouteCandidates[0];
    const double dBestScore = pBestCandidate->GetScore();

    while (rvecRouteCandidates.size() > 25)
    {
        MHTRouteCandidate* pCandidate = rvecRouteCandidates.back();
        if (pCandidate == NULL || pCandidate->GetScore() > (3 * dBestScore))
        {
            delete pCandidate;
            rvecRouteCandidates.pop_back();
        }
        else
            break;
    }
}

void MapMatchingMHT::AddAdjacentSections(const MHTRouteCandidate* pCandidate,
                        bool bUpDown,
                        std::vector<MHTRouteCandidate*>& rvecNewRouteCandidates)
{
    if (m_pNetwork == NULL)
        return;

    const NetworkSection& rSection = pCandidate->GetLastSection();

    //const int nDebugSectionID = rSection.GetSectionID();

    vector<DirectedSection> adjSectionList;
    m_pNetwork->GetAdjacentSections(rSection.GetSectionID(), bUpDown,
                                    adjSectionList);

    if (adjSectionList.size() > 0)
    {
        for (size_t i = 0; i < adjSectionList.size(); i++)
        {
            Tuple* pSectionTuple = m_pNetwork->GetSection(
                                             adjSectionList[i].GetSectionTid());
            DirectedNetworkSection adjSection(pSectionTuple, m_pNetwork, false);

            if (adjSection.GetSectionID() == rSection.GetSectionID())
                            continue;

            if (adjSectionList[i].GetUpDownFlag())
                adjSection.SetDirection(DirectedNetworkSection::DIR_UP);
            else
                adjSection.SetDirection(DirectedNetworkSection::DIR_DOWN);

            // make copy of current candidate
            MHTRouteCandidate* pNewCandidate =
                                             new MHTRouteCandidate(*pCandidate);
            pNewCandidate->AddSection(adjSection);

            rvecNewRouteCandidates.push_back(pNewCandidate);
        }
    }
    else
    {
        // no adjacent sections
    }
}

MHTRouteCandidate* MapMatchingMHT::DetermineBestRouteCandidate(
                        std::vector<MHTRouteCandidate*>& rvecRouteCandidates)
{
    std::sort(rvecRouteCandidates.begin(),
              rvecRouteCandidates.end(),
              RouteCandidateCompare);

//#define TRACE_GPOINTS_OF_CANDIDATES
#ifdef TRACE_GPOINTS_OF_CANDIDATES
    for (size_t i = 0; i < rvecRouteCandidates.size(); ++i)
    {
        std::stringstream strFileName;
        strFileName << "/home/secondo/Traces/GPoints_" << i << ".txt";
        ofstream Stream(strFileName.str().c_str());
        rvecRouteCandidates[i]->PrintGPointsAsPoints(Stream);
    }
#endif

    if (rvecRouteCandidates.size() > 0)
        return rvecRouteCandidates.front();
    else
        return NULL;
}

void MapMatchingMHT::CreateCompleteRoute(
                       const std::vector<MHTRouteCandidate*>& rvecRouteSegments)
{
    MHTRouteCandidate::PointData* pLastPointOfPrevSection = NULL;

    for (size_t i = 0; i < rvecRouteSegments.size(); ++i)
    {
        bool bCalcShortestPath = (i > 0);

        MHTRouteCandidate* pCandidate = rvecRouteSegments[i];
        if (pCandidate != NULL)
        {
            const std::vector<MHTRouteCandidate::PointData*>& rPoints =
                                                        pCandidate->GetPoints();
            const size_t nPoints = rPoints.size();

            // Find first defined point
            MHTRouteCandidate::PointData* pData1 = NULL;
            size_t j = 0;
            while(j < nPoints &&
                  (pData1 == NULL || pData1->GetGPoint() == NULL))
            {
                pData1 = rPoints[j];
                ++j;
            }

            // Process next points
            while (j < nPoints && pData1 != NULL &&
                   pData1->GetGPoint() != NULL)
            {
                MHTRouteCandidate::PointData* pData2 = NULL;

                while (j < nPoints &&
                       (pData2 == NULL || pData2->GetGPoint() == NULL))
                {
                    pData2 = rPoints[j];
                    ++j;
                }

                if (bCalcShortestPath && pLastPointOfPrevSection != NULL)
                {
                    // Calculate ShortestPath between last point of previous
                    // segment and first point of this segment

                    // ShortestPath calculation only when time difference
                    // is less than 4 minutes
                    const DateTime MaxTimeDiff(durationtype, 240000);

                    if (pData1->GetTime() - pLastPointOfPrevSection->GetTime() <
                                                                    MaxTimeDiff)
                    {
                        CalcShortestPath(pLastPointOfPrevSection->GetGPoint(),
                                         pData1->GetGPoint(),
                                         pLastPointOfPrevSection->GetTime(),
                                         pData1->GetTime());
                    }

                    bCalcShortestPath = false;
                }

                if (pData2 != NULL && pData2->GetGPoint() != NULL)
                {
                    const Interval<Instant> timeInterval(pData1->GetTime(),
                                                         pData2->GetTime(),
                                                         pData1->GetClosed(),
                                                         pData2->GetClosed());

                    ConnectPoints(*(pData1->GetGPoint()),
                                  *(pData2->GetGPoint()),
                                  timeInterval);

                   pLastPointOfPrevSection = pData2;
                }

                pData1 = pData2;
            }
        }
    }
}

void MapMatchingMHT::TraceRouteCandidates(
                          const std::vector<MHTRouteCandidate*>& rvecCandidates,
                          const char* pszText) const
{
//#define TRACE_ROUTE_CANDIDATES
#ifdef TRACE_ROUTE_CANDIDATES
    ofstream Stream("/home/secondo/Traces/RouteCandidates.txt",
                    ios_base::out|ios_base::ate|ios_base::app);

    Stream << pszText << endl;

    for (size_t i = 0; i < rvecCandidates.size(); ++i)
    {
        MHTRouteCandidate* pCandidate = rvecCandidates[i];
        if (pCandidate != NULL)
            pCandidate->Print(Stream);
        Stream << endl;
    }
#endif
}


} // end of namespace mapmatch

