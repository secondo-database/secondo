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
#include "GPXFileReader.h"

#include <stdio.h>

#include "../Network/NetworkAlgebra.h"
#include "../TemporalNet/TemporalNetAlgebra.h"

namespace mapmatch {

#define MYTRACE(a) //MyStream << a << '\n'; MyStream.flush()
#define MYVARTRACE(a) //MyStream << #a << a << '\n'; MyStream.flush()

//static ofstream MyStream("/home/secondo/Traces/Trace.txt");

/*
3 class MapMatchingMHT
  Map matching algorithm based on the Multiple Hypothesis Technique (MHT)

3.1 Constructor / Destructor

*/

MapMatchingMHT::MapMatchingMHT(Network* pNetwork, MPoint* pMPoint)
:MapMatchingBase(pNetwork, pMPoint)
{
}

MapMatchingMHT::MapMatchingMHT(Network* pNetwork, std::string strFileName)
:MapMatchingBase(pNetwork, (DbArray<MapMatchData>*) NULL)
{
    try
    {
        GPXFileReader Reader;
        if (Reader.Open(strFileName))
        {
            CTrkPointIterator* pIt = Reader.GetTrkPointIterator();
            if (pIt != NULL)
            {
                DbArrayPtr<DbArray<MapMatchData> >
                                       pDbaMMData(new DbArray<MapMatchData>(0));

                GPXFileReader::SGPXTrkPointData TrkPtData;
                while(pIt->GetCurrent(TrkPtData))
                {
                    if (TrkPtData.m_Point.IsDefined() &&
                        TrkPtData.m_Time.IsDefined())
                    {
                        MapMatchData MMData(
                                TrkPtData.m_Point.GetY() * m_dNetworkScale,
                                TrkPtData.m_Point.GetX() * m_dNetworkScale,
                                TrkPtData.m_Time.millisecondsToNull());

                        if (TrkPtData.m_nFix.IsDefined())
                            MMData.m_nFix = TrkPtData.m_nFix.GetValue();

                        if (TrkPtData.m_nSat.IsDefined())
                            MMData.m_nSat = TrkPtData.m_nSat.GetValue();

                        if (TrkPtData.m_dHDOP.IsDefined())
                            MMData.m_dHdop = TrkPtData.m_dHDOP.GetValue();

                        if (TrkPtData.m_dVDOP.IsDefined())
                            MMData.m_dVdop = TrkPtData.m_dVDOP.GetValue();

                        if (TrkPtData.m_dPDOP.IsDefined())
                            MMData.m_dPdop = TrkPtData.m_dPDOP.GetValue();


                        pDbaMMData->Append(MMData);
                    }

                    pIt->Next();
                }

                Reader.FreeTrkPointIterator(pIt);

                SetMMData(pDbaMMData);
            }
        }
        else
        {
            // Failed read file
        }
    }
    catch(...)
    {
        cerr << "Error reading file " << strFileName;
    }
}

MapMatchingMHT::MapMatchingMHT(Network* pNetwork,
                               DbArrayPtr<DbArray<MapMatchData> > pDbaMMData)
:MapMatchingBase(pNetwork, pDbaMMData)
{

}

// Destructor
MapMatchingMHT::~MapMatchingMHT()
{
}

void MapMatchingMHT::GetSectionsOfRoute(const NetworkRoute& rNetworkRoute,
                                       const Region& rRegion,
                                       std::vector<NetworkSection>& rVecSectRes)
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

    Point pt(rPoint);
    pt.Scale(1.0 / m_dNetworkScale);
    assert(pt.checkGeographicCoord());

    const double dLength = 0.250; // edge length 250 meters

    //               0
    //      315 +---------+
    //          |\        |
    //          |  \  125 |
    //      270 -----*----- 90
    //          | 125  \  |
    //          |        \|
    //          +---------+  135
    //              180

    Point pt1 = MMUtil::CalcDestinationPoint(pt, 135.0,
                                             sqrt(pow(dLength/2., 2) +
                                             pow(dLength/2., 2)));

    Point pt2 = MMUtil::CalcDestinationPoint(pt, 315.0,
                                             sqrt(pow(dLength/2., 2) +
                                             pow(dLength/2., 2)));

    pt1.Scale(m_dNetworkScale);
    pt2.Scale(m_dNetworkScale);

    const Rectangle<2> BBox(true, std::min(pt1.GetX(), pt2.GetX()),
                                  std::max(pt1.GetX(), pt2.GetX()),
                                  std::min(pt1.GetY(), pt2.GetY()),
                                  std::max(pt1.GetY(), pt2.GetY()));

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

/*
3.2 MapMatchingMHT::DoMatch
Main-Method for MapMatching

*/

bool MapMatchingMHT::DoMatch(MGPoint* pResMGPoint)
{
    // Initialization
    if (!InitMapMatching(pResMGPoint))
    {
        return false;
    }

    // Step 1 - Subdividing trip
    vector<DbArrayPtr<DbArray<MapMatchData> > > vecTripSegments;
    TripSegmentation(vecTripSegments);

    // Steps 2-4
    std::vector<MHTRouteCandidate*> vecRouteSegments;

    for (vector<DbArrayPtr<DbArray<MapMatchData> > >::iterator
                                                   it = vecTripSegments.begin();
         it != vecTripSegments.end();
         ++it)
    {
        DbArrayPtr<DbArray<MapMatchData> > pDbaMMData(*it);
        if (pDbaMMData == NULL)
            continue;
        *it = NULL;

        int nIdxFirstComponent = 0;

        while(nIdxFirstComponent >= 0 &&
              nIdxFirstComponent < pDbaMMData->Size())
        {
            // Step 2 - Determination of initial route/segment candidates
            std::vector<MHTRouteCandidate*> vecRouteCandidates;
            nIdxFirstComponent = GetInitialRouteCandidates(pDbaMMData.get(),
                                                           nIdxFirstComponent,
                                                           vecRouteCandidates);

            // Step 3 - Route developement
            nIdxFirstComponent = DevelopRoutes(pDbaMMData.get(),
                                               nIdxFirstComponent,
                                               vecRouteCandidates);

            // Step 4 - Selection of most likely candidate
            MHTRouteCandidate* pBestCandidate = DetermineBestRouteCandidate(
                                                            vecRouteCandidates);
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

    return true;
}

bool MapMatchingMHT::CheckQualityOfGPSFix(const MapMatchData& rMMData)
{
    // Number of satellites

    int nSat = rMMData.m_nSat;

    if (!(nSat < 0) && nSat < 2) // minimum 2 satellites
    {                            // nSat < 0 => undefined
        return false;
    }


    // Fix -> possible values:
    // -1 - undefined
    // 0  - none
    // 2  - 2d
    // 3  - 3d
    // 4  - DGPS
    // 5  - pps (military)

    int nFix = rMMData.m_nFix;

    if (!(nFix < 0) && nFix < 2) // minimum 2d
    {                            // nFix < 0 => undefined
        return false;
    }

    // HDOP (Horizontal Dilution Of Precision)

    double dHDOP = rMMData.m_dHdop;

    if (dHDOP > 7.)
    {
        return false;
    }

    // PDOP (Positional Dilution Of Precision)

    double dPDOP = rMMData.m_dPdop;

    if (dPDOP > 7.)
    {
        return false;
    }

    return true; // Quality Ok or no data found
}


/*
3.3 MapMatchingMHT::TripSegmentation
    Detect spatial and temporal gaps in input-data

*/

void MapMatchingMHT::TripSegmentation(
                std::vector<DbArrayPtr<DbArray<MapMatchData> > >& rvecTripParts)
{
    if (m_pNetwork == NULL || !m_pNetwork->IsDefined() || m_pDbaMMData == NULL)
        return;

//#define TRACE_BAD_DATA
#ifdef TRACE_BAD_DATA
    ofstream streamBadData("/home/secondo/Traces/BadData.txt");
#endif

    // Detect spatial and temporal gaps in input-data
    // Divide the data if the time gap is longer than 40 seconds or
    // the distance is larger than 500 meters

    const double dMaxDistance = 750.0; // 750 meter
    //const DateTime MaxTimeDiff(durationtype, 60000); // 60 seconds
    int64_t MaxTimeDiff = 60000; // 60 seconds
    const Geoid  GeodeticSystem(Geoid::WGS1984);

    DbArrayPtr<DbArray<MapMatchData> > pActArray(NULL);
    MapMatchData  ActData(0.0, 0.0, 0);
    //DateTime prevEndTime(instanttype);
    int64_t  prevEndTime = 0;
    Point    prevEndPoint(false);
    bool     bProcessNext = true;

    const int nMMComponents = m_pDbaMMData->Size();

    const Rectangle<2> rectBoundingBoxNetwork = m_pNetwork->BoundingBox();

    for (int i = 0; i < nMMComponents; bProcessNext ? i++ : i)
    {
        if (bProcessNext)
        {
            if (!m_pDbaMMData->Get(i, ActData))
                continue; // process next
        }

        bProcessNext = true;

        if (!ActData.GetPoint().Inside(rectBoundingBoxNetwork))
        {
            // Outside bounding box of network
            continue; // process next unit
        }

        if (!CheckQualityOfGPSFix(ActData))
        {
            // bad quality of GPS fix
#ifdef TRACE_BAD_DATA
            ActData.Print(streamBadData);
            streamBadData << endl;
#endif
            continue; // process next unit
        }

        if (pActArray == NULL)
        {
            // create new Data
            pActArray = DbArrayPtr<DbArray<MapMatchData> >
                                                (new DbArray<MapMatchData>(10));

            // Add data
            pActArray->Append(ActData);

            prevEndTime = ActData.m_Time;
            prevEndPoint = (ActData.GetPoint() * (1 / m_dNetworkScale));
        }
        else
        {
            bool bValid = true;
            if ((ActData.m_Time - prevEndTime) > MaxTimeDiff ||
                prevEndPoint.DistanceOrthodrome(ActData.GetPoint()  *
                                                        (1.0 / m_dNetworkScale),
                                                GeodeticSystem,
                                                bValid) > dMaxDistance)
            {
                // gap detected -> finalize current array
                if (pActArray->Size() >= 10)
                {
                    rvecTripParts.push_back(pActArray);
                    pActArray = DbArrayPtr<DbArray<MapMatchData> >(NULL);
                }
                else
                {
                    // less than 10 components -> drop
                    pActArray = DbArrayPtr<DbArray<MapMatchData> >(NULL);
                }

                bProcessNext = false; // Process ActData once again
            }
            else
            {
                // no gap between current and previous Data

                pActArray->Append(ActData);
                prevEndTime = ActData.m_Time;
                prevEndPoint = (ActData.GetPoint() * (1.0 / m_dNetworkScale));
            }
        }
    }

    // finalize last Array
    if (pActArray != NULL)
    {
        rvecTripParts.push_back(pActArray);
    }
}

/*
3.4 MapMatchingMHT::GetInitialRouteCandidates
    Find first route candidates

*/

int MapMatchingMHT::GetInitialRouteCandidates(
                           const DbArray<MapMatchData>* pDbaMMData,
                           int nIdxFirstComponent,
                           std::vector<MHTRouteCandidate*>& rvecRouteCandidates)
{
    if (pDbaMMData == NULL || pDbaMMData->Size() == 0)
        return -1;

    // Get first (defined) point and search for sections
    // in the vicinity of the point.
    // Process next (defined) point, if no section was found.
    std::vector<NetworkSection> vecInitialSections;
    int nIndexFirst = nIdxFirstComponent;

    while (vecInitialSections.size() == 0 &&
           nIndexFirst < pDbaMMData->Size())
    {
        MapMatchData Data(0.0, 0.0, 0);
        if (pDbaMMData->Get(nIndexFirst, Data))
        {
            // get first section candidates
            GetInitialSectionCandidates(Data.GetPoint(), vecInitialSections);
        }

        if (vecInitialSections.size() == 0)
            ++nIndexFirst;
    }

    bool bCheckGeographicCoord = true;

    // process initial section candidates
    // -> create route candidates
    for (std::vector<NetworkSection>::iterator it = vecInitialSections.begin();
         it != vecInitialSections.end(); ++it)
    {
        const NetworkSection& rActInitalNetworkSection = *it;
        if (!rActInitalNetworkSection.IsDefined())
            continue;

        if (bCheckGeographicCoord)
        {
            Point ptStart(rActInitalNetworkSection.GetStartPoint());
            ptStart.Scale(1.0 / m_dNetworkScale);
            assert(ptStart.checkGeographicCoord());
            Point ptEnd(rActInitalNetworkSection.GetEndPoint());
            ptEnd.Scale(1.0 / m_dNetworkScale);
            assert(ptEnd.checkGeographicCoord());
            bCheckGeographicCoord = false; // Only check first section
        }

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

    return rvecRouteCandidates.size() > 0 ? nIndexFirst: -1;
}

/*
3.5 MapMatchingMHT::DevelopRoutes

*/

int MapMatchingMHT::DevelopRoutes(const DbArray<MapMatchData>* pDbaMMData,
                           int nIndexFirstComponent,
                           std::vector<MHTRouteCandidate*>& rvecRouteCandidates)
{
    if (pDbaMMData == NULL ||
        m_pNetwork == NULL || !m_pNetwork->IsDefined() ||
        nIndexFirstComponent < 0)
        return -1;

    const int nNoComponents = pDbaMMData->Size();

    Point ptPrev(false);
    //DateTime timePrev(0.0);
    int64_t timePrev = 0;

    for (int i = nIndexFirstComponent; i < nNoComponents; ++i)
    {
        //TracePoints << i << endl;
        MapMatchData ActData(0.0, 0.0, 0);
        if (!pDbaMMData->Get(i, ActData))
            continue;

        if (ptPrev != ActData.GetPoint() &&
            timePrev != ActData.m_Time)
        {
            // Develop routes with point p0
            DevelopRoutes(ActData.GetPoint(),
                          DateTime(ActData.m_Time),
                          rvecRouteCandidates);

            ptPrev = ActData.GetPoint();
            timePrev = ActData.m_Time;

            TraceRouteCandidates(rvecRouteCandidates,
                                 "##### Nach DevelopRoutes #####");

            // Reduce Routes
            ReduceRouteCandidates(rvecRouteCandidates);
            TraceRouteCandidates(rvecRouteCandidates,
                                 "##### Nach Reduce #####");
        }

        if (!CheckRouteCandidates(rvecRouteCandidates))
        {
            ofstream StreamBadNetwork("/home/secondo/Traces/BadNetwork.txt",
            ios_base::out|ios_base::ate|ios_base::app);
            StreamBadNetwork << "Match point failed: " << endl;
            ActData.Print(StreamBadNetwork);
            StreamBadNetwork << endl;

            // Matching failed - Restart with next component
            return i+1;
        }
    }

    return nNoComponents;
}

class EndPtDistanceFilter: public SectionFilter
{
public:
    EndPtDistanceFilter(const Point& rPoint,
                        const double& dMaxDistance,
                        const double& dScale)
    :m_rPoint(rPoint), m_dMaxDistance(dMaxDistance), m_dScale(dScale)
    {
    }

    virtual ~EndPtDistanceFilter()
    {
    }

    bool IsValid(const DirectedNetworkSection& rSection)
    {
        const SimpleLine* pCurve = rSection.GetCurve();

        if (pCurve == NULL || !pCurve->IsDefined())
            return false;

        const bool bStartsSmaller = rSection.GetCurveStartsSmaller();

        double dDistance = 0.0;

        if (rSection.GetDirection() == DirectedNetworkSection::DIR_UP)
            dDistance = MMUtil::CalcDistance(pCurve->EndPoint(bStartsSmaller),
                                             m_rPoint, m_dScale);
        else
            dDistance = MMUtil::CalcDistance(pCurve->StartPoint(bStartsSmaller),
                                             m_rPoint, m_dScale);

        return dDistance < m_dMaxDistance;
    }

private:
    const Point& m_rPoint;
    const double& m_dMaxDistance;
    const double& m_dScale;
};


void MapMatchingMHT::DevelopRoutes(const Point& rPoint,
                          const DateTime& rTime,
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
        if (!AssignPoint(pCandidate, rPoint, rTime, eNextCandidates))
        {
            // Point could not be assigned to last section of candidate
            // -> add adjacent sections

            // max 6 sections look ahead
            // no look ahead when processing first point of route
            if (pCandidate->GetPoints().size() > 0 &&
                pCandidate->GetCountLastEmptySections() <= 5)
            {
                std::vector<MHTRouteCandidate*> vecNewRouteCandidatesLocal;

                const DirectedNetworkSection& rSection =
                                                   pCandidate->GetLastSection();
                if (!rSection.IsDefined())
                {
                    *it = NULL;
                    delete pCandidate;
                    pCandidate = NULL;
                    continue;
                }

                const SimpleLine* pCurve = rSection.GetCurve();
                if (pCurve == NULL || !pCurve->IsDefined())
                {
                    *it = NULL;
                    delete pCandidate;
                    pCandidate = NULL;
                    continue;
                }

                const bool bStartsSmaller = rSection.GetCurveStartsSmaller();
                const Point ptStart = pCurve->StartPoint(bStartsSmaller);
                const Point ptEnd = pCurve->EndPoint(bStartsSmaller);

                if (!ptStart.IsDefined() || !ptEnd.IsDefined())
                {
                    ofstream StreamBadNetwork(
                                 "/home/secondo/Traces/BadNetwork.txt",
                                 ios_base::out | ios_base::ate | ios_base::app);

                    StreamBadNetwork << "Undefined start- or endpoint: ";
                    StreamBadNetwork << "Section: " << rSection.GetSectionID();
                    StreamBadNetwork << endl;

                    *it = NULL;
                    delete pCandidate;
                    pCandidate = NULL;
                    continue;
                }

                const double dDistanceStart = MMUtil::CalcDistance(
                                                               rPoint,
                                                               ptStart,
                                                               m_dNetworkScale);

                const double dDistanceEnd = MMUtil::CalcDistance(
                                                               rPoint,
                                                               ptEnd,
                                                               m_dNetworkScale);

                bool bUpDown = dDistanceStart > dDistanceEnd;

                bool bUpDownDriving = (rSection.GetDirection() ==
                                                DirectedNetworkSection::DIR_UP);

                if (pCandidate->GetCountPointsOfLastSection() == 0)
                {
                    // Don't go back to previous section,
                    // because this point already was processed
                    // with previous section(s)
                    if (bUpDownDriving == bUpDown)
                    {
                        AddAdjacentSections(pCandidate, bUpDown,
                                            vecNewRouteCandidatesLocal);
                    }
                }
                else
                {
                    AddAdjacentSections(pCandidate, bUpDown,
                                        vecNewRouteCandidatesLocal);
                }

                if (bUpDown != bUpDownDriving)
                {
                    // add sections in "driving"-direction
                    // when distance of endpoint of section
                    // to GPS-Point is smaller
                    EndPtDistanceFilter Filter(rPoint,
                                         std::min(dDistanceStart, dDistanceEnd),
                                         m_dNetworkScale);

                    AddAdjacentSections(pCandidate, bUpDownDriving,
                                        vecNewRouteCandidatesLocal, &Filter);
                }

                if (vecNewRouteCandidatesLocal.size() > 0)
                {
                    // !! Rekursion !!
                    DevelopRoutes(rPoint, rTime, vecNewRouteCandidatesLocal);

                    vecNewRouteCandidates.insert(vecNewRouteCandidates.end(),
                            vecNewRouteCandidatesLocal.begin(),
                            vecNewRouteCandidatesLocal.end());
                }

                *it = NULL;

                if (pCandidate->GetCountLastEmptySections() == 0)
                {
                    // Only add GPS-Point to Candidate (no GPoint)
                    // "Offroad"-Point
                    // Default-Distance: 40.0

                    pCandidate->AddPoint(rPoint, 40., rTime);

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
                    DevelopRoutes(rPoint, rTime,
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

/*
3.6 MapMatchingMHT::AssignPoint
Assign point to route candidate, if possible

*/

bool MapMatchingMHT::AssignPoint(MHTRouteCandidate* pCandidate,
                                 const Point& rPoint,
                                 const datetime::DateTime& rTime,
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
                                                   m_dNetworkScale);

    if (PointProjection.IsDefined())
    {
        // Check if the startnode or endpoint has been reached.
        const bool bStartsSmaller = rSection.GetCurveStartsSmaller();
        const Point ptStart = pCurve->StartPoint(bStartsSmaller);
        const Point ptEnd = pCurve->EndPoint(bStartsSmaller);

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

        // Only assign to startnode if it is orthogonal
        if (!bIsOrthogonal)
        {
            if (rSection.GetDirection() == DirectedNetworkSection::DIR_UP)
            {
                if (AlmostEqual(PointProjection, ptStart) /*&&
                    (pCandidate->GetCountPointsOfLastSection() > 0 ||
                     dDistance > 25.0)*/) // TODO
                    return false;
            }
            else if (rSection.GetDirection() ==
                                          DirectedNetworkSection::DIR_DOWN)
            {
                if (AlmostEqual(PointProjection, ptEnd) /*&&
                    (pCandidate->GetCountPointsOfLastSection() > 0 ||
                    dDistance > 25.0)*/) // TODO
                    return false;
            }
        }

        // Check "length" of GPS-Points
        const vector<MHTRouteCandidate::PointData*>& vecPointsLastSection =
                                           pCandidate->GetPointsOfLastSection();
        const size_t nPointsLastSection = vecPointsLastSection.size();

        vector<const Point*> vecPoints;
        for (size_t i = 0; i < nPointsLastSection; ++i)
        {
            vecPoints.push_back(vecPointsLastSection[i]->GetPointGPS());
        }

        vecPoints.push_back(&rPoint);

        // Calculate travelled distance ('length' of GPS-Points)
        double dDistanceTravelled = MMUtil::CalcDistance(vecPoints,
                                                         m_dNetworkScale);

        // Add Distance from Start-Point to first projected point
        Point* pFirstProjectedPoint = &PointProjection;
        if (nPointsLastSection > 0 && vecPointsLastSection[0] != NULL)
        {
            pFirstProjectedPoint =
                                  vecPointsLastSection[0]->GetPointProjection();
        }

        if (rSection.GetDirection() == DirectedNetworkSection::DIR_UP &&
            pFirstProjectedPoint != NULL)
        {
            dDistanceTravelled += MMUtil::CalcDistance(ptStart,
                                                       *pFirstProjectedPoint,
                                                       m_dNetworkScale);
        }
        else if (rSection.GetDirection() == DirectedNetworkSection::DIR_DOWN &&
                 pFirstProjectedPoint != NULL)
        {
            dDistanceTravelled += MMUtil::CalcDistance(ptEnd,
                                                       *pFirstProjectedPoint,
                                                       m_dNetworkScale);
        }

        const double dLengthCurve = rSection.GetCurveLength(m_dNetworkScale);

        // If traveled (GPS-)distance ist larger than 85% of curve length
        // then look at adjacent sections, too
        if (dDistanceTravelled > (dLengthCurve / 100. * 85.))
        {
            const double dDistanceStart = MMUtil::CalcDistance(rPoint,
                                                               ptStart,
                                                               m_dNetworkScale);

            const double dDistanceEnd = MMUtil::CalcDistance(rPoint,
                                                             ptEnd,
                                                             m_dNetworkScale);

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

        pCandidate->AddPoint(rPoint, PointProjection,
                             rSection.GetRoute(),
                             dDistance, rTime);
        return true;
    }
    else
    {
        // Projection failed
        return false;
    }


    return false;
}

static bool RouteCandidateScoreCompare(const MHTRouteCandidate* pRC1,
                                       const MHTRouteCandidate* pRC2)
{
    // RC1 < RC2
    if (pRC1 == NULL || pRC1 == NULL)
        return pRC1 < pRC2;
    return pRC1->GetScore() < pRC2->GetScore();
}


static bool LastPointsEqual(const MHTRouteCandidate* pRouteCandidate1,
                            const MHTRouteCandidate* pRouteCandidate2,
                            int nCnt)
{
    std::vector<MHTRouteCandidate::PointData*>::const_reverse_iterator it1 =
                                         pRouteCandidate1->GetPoints().rbegin();
    std::vector<MHTRouteCandidate::PointData*>::const_reverse_iterator it2 =
                                         pRouteCandidate2->GetPoints().rbegin();

    while (nCnt > 0 &&
           it1 != pRouteCandidate1->GetPoints().rend() &&
           it2 != pRouteCandidate2->GetPoints().rend())
    {
        MHTRouteCandidate::PointData* pData1 = *it1;
        MHTRouteCandidate::PointData* pData2 = *it2;

        if (!(*pData1 == *pData2))
            return false;

        --nCnt;

        ++it1;
        ++it2;
    }

    return true;
}


/*
3.7 MapMatchingMHT::ReduceRouteCandidates
Route reduction - removes unlikely routes

*/

void MapMatchingMHT::ReduceRouteCandidates(std::vector<MHTRouteCandidate*>&
                                                            rvecRouteCandidates)
{
    if (rvecRouteCandidates.size() == 0)
        return;

    std::sort(rvecRouteCandidates.begin(),
              rvecRouteCandidates.end(),
              RouteCandidateScoreCompare);

    const bool bInInitPhase = (rvecRouteCandidates[0]->GetPoints().size() < 5);

    if (bInInitPhase &&
        rvecRouteCandidates.size() < 20)
        return;

    MHTRouteCandidate* pBestCandidate = rvecRouteCandidates[0];
    const size_t nPoints = pBestCandidate->GetPoints().size();
    const double dBestAvgScorePerPoint = pBestCandidate->GetScore() / nPoints;

    // remove candidates with very bad score
    size_t nCandidates = rvecRouteCandidates.size();
    for (size_t i = 0; !bInInitPhase && i < nCandidates; ++i)
    {
        MHTRouteCandidate* pCandidate = rvecRouteCandidates[i];

        if (pCandidate == NULL ||
            pCandidate->GetCountLastOffRoadPoints() > 2 ||
            ((pCandidate->GetScore() / nPoints) > (6 * dBestAvgScorePerPoint)))
        {
            pCandidate->MarkAsInvalid();
        }
    }

    std::sort(rvecRouteCandidates.begin(),
                  rvecRouteCandidates.end(),
                  RouteCandidateScoreCompare);

    // remove invalid candidates
    while (rvecRouteCandidates.size() > 1)
    {
        MHTRouteCandidate* pCandidate = rvecRouteCandidates.back();
        if (pCandidate == NULL || pCandidate->IsInvalid())
        {
            delete pCandidate;
            rvecRouteCandidates.pop_back();
        }
        else
        {
            break;
        }
    }

    // Find duplicates
    nCandidates = rvecRouteCandidates.size();
    for (size_t i = 0; i < nCandidates; ++i)
    {
        MHTRouteCandidate* pRouteCandidate1 = rvecRouteCandidates[i];
        if (pRouteCandidate1->IsInvalid())
            continue;

        for (size_t j = i + 1; j < nCandidates; ++j)
        {
            MHTRouteCandidate* pRouteCandidate2 = rvecRouteCandidates[j];
            if (pRouteCandidate2->IsInvalid())
                continue;

            if (*pRouteCandidate1 == *pRouteCandidate2)
            {
                rvecRouteCandidates[j]->MarkAsInvalid();
            }
            else if (!bInInitPhase &&
                     (pRouteCandidate1->GetLastSection() ==
                                       pRouteCandidate2->GetLastSection()) &&
                     LastPointsEqual(pRouteCandidate1, pRouteCandidate2, 5))
            {
                rvecRouteCandidates[j]->MarkAsInvalid();
            }
        }
    }

    std::sort(rvecRouteCandidates.begin(),
              rvecRouteCandidates.end(),
              RouteCandidateScoreCompare);

    // maximum 25
    while (rvecRouteCandidates.size() > 25)
    {
        MHTRouteCandidate* pCandidate = rvecRouteCandidates.back();
        rvecRouteCandidates.pop_back();
        delete pCandidate;
    }

    // remove invalid candidates
    while (rvecRouteCandidates.size() > 1)
    {
        MHTRouteCandidate* pCandidate = rvecRouteCandidates.back();
        if (pCandidate == NULL || pCandidate->IsInvalid())
        {
            delete pCandidate;
            rvecRouteCandidates.pop_back();
        }
        else
        {
            break;
        }
    }
}

/*
3.8 MapMatchingMHT::CheckRouteCandidates

*/

bool MapMatchingMHT::CheckRouteCandidates(const std::vector<MHTRouteCandidate*>&
                                                            rvecRouteCandidates)
{
    size_t nFailedCandidates = 0;
    const size_t nCandidates = rvecRouteCandidates.size();

    if (nCandidates == 0)
    {
        assert(false);
        return false;
    }

    if (rvecRouteCandidates[0]->IsInvalid()) // Candidates should be sorted by
        return false;                        // score -> ReduceRouteCandidates
                                             // Best candidate is invalid

    if (rvecRouteCandidates[0]->GetLastPoint()->GetScore() > 350.)
        return false; // Too bad scale for last point of best candidate

    for (size_t i = 0; i < nCandidates; ++i)
    {
        MHTRouteCandidate* pCandidate = rvecRouteCandidates[i];
        if (pCandidate == NULL || pCandidate->GetCountLastOffRoadPoints() > 0)
            ++nFailedCandidates;
    }

    return (nFailedCandidates != nCandidates);
}

/*
3.9 MapMatchingMHT::AddAdjacentSections
adds adjacent sections to route candidates

*/

void MapMatchingMHT::AddAdjacentSections(const MHTRouteCandidate* pCandidate,
                        bool bUpDown,
                        std::vector<MHTRouteCandidate*>& rvecNewRouteCandidates,
                        SectionFilter* pFilter)
{
    if (m_pNetwork == NULL || pCandidate == NULL)
        return;

    bool bMoveLastPoint = false;

    const DirectedNetworkSection& rSection = pCandidate->GetLastSection();

    double dAdditionalUTurnScore = 0.0;

    if ((rSection.GetDirection() ==
            DirectedNetworkSection::DIR_UP && !bUpDown) ||
        (rSection.GetDirection() ==
            DirectedNetworkSection::DIR_DOWN && bUpDown))
    {
        // U-Turn

        if (pCandidate->GetCountPointsOfLastSection() == 1)
        {
            // U-Turn and only one assigned point to last section
            // -> Remove last point and assign this point to
            // start point of adjacent section

            bMoveLastPoint = true;
        }

        // Calculate Score for U-Turn
        const SimpleLine* pCurve = rSection.GetCurve();
        if (pCurve != NULL && pCurve->IsDefined())
        {
            const Point ptRef = bUpDown ? rSection.GetEndPoint() :
                                          rSection.GetStartPoint();

            const vector<MHTRouteCandidate::PointData*>& vecPointsLastSection =
                                           pCandidate->GetPointsOfLastSection();

            // Calculate maximum distance of points of last
            // section to End-/StartPoint
            const size_t nPoints = vecPointsLastSection.size();
            for (size_t i = 0; i < nPoints; ++i)
            {
                Point* pPt = vecPointsLastSection[i]->GetPointProjection();
                if (pPt != NULL)
                {
                    double dDist = MMUtil::CalcDistance(*pPt,
                                                        ptRef,
                                                        m_dNetworkScale);

                    dAdditionalUTurnScore = std::max(dAdditionalUTurnScore,
                                                     dDist);
                }
            }
        }

        dAdditionalUTurnScore *= 2.0;
    }

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

            // Additional filter
            if (pFilter != NULL && !pFilter->IsValid(adjSection))
                continue;

            // make copy of current candidate
            MHTRouteCandidate* pNewCandidate =
                                             new MHTRouteCandidate(*pCandidate);

            if (bMoveLastPoint)
            {
                MHTRouteCandidate::PointData PointDataLast(
                        pNewCandidate->GetLastPoint() != NULL ?
                                *(pNewCandidate->GetLastPoint()):
                                MHTRouteCandidate::PointData());
                pNewCandidate->RemoveLastPoint();

                // Assign to start node of new section
                Point PointNewProjection(adjSection.GetDirection() ==
                                            DirectedNetworkSection::DIR_UP ?
                                                    adjSection.GetStartPoint() :
                                                    adjSection.GetEndPoint());

                if (!PointNewProjection.IsDefined())
                {
                    ofstream StreamBadNetwork(
                                     "/home/secondo/Traces/BadNetwork.txt",
                                     ios_base::out|ios_base::ate|ios_base::app);

                    StreamBadNetwork << "Undefined start- or endpoint: ";
                    StreamBadNetwork << "Section: ";
                    StreamBadNetwork << adjSection.GetSectionID();
                    StreamBadNetwork << endl;

                    delete pNewCandidate; pNewCandidate = NULL;
                    continue;
                }

                double dDistance = MMUtil::CalcDistance(PointNewProjection,
                                                   *PointDataLast.GetPointGPS(),
                                                   m_dNetworkScale);

                pNewCandidate->AddPoint(*PointDataLast.GetPointGPS(),
                                        PointNewProjection,
                                        PointDataLast.GetNetworkRoute(),
                                        dDistance,
                                        PointDataLast.GetTime());
            }
            else
            {
                // add Score for U-Turn
                pNewCandidate->AddScore(dAdditionalUTurnScore);
            }

            // add adjacent section
            pNewCandidate->AddSection(adjSection);

            rvecNewRouteCandidates.push_back(pNewCandidate);
        }
    }
    else
    {
        // no adjacent sections
    }
}

/*
3.10 MapMatchingMHT::DetermineBestRouteCandidate
finds the best route candidate

*/

MHTRouteCandidate* MapMatchingMHT::DetermineBestRouteCandidate(
                           std::vector<MHTRouteCandidate*>& rvecRouteCandidates)
{
    std::sort(rvecRouteCandidates.begin(),
              rvecRouteCandidates.end(),
              RouteCandidateScoreCompare);

//#define TRACE_BEST_CANDIDATES
#ifdef TRACE_BEST_CANDIDATES
    static int nCall = 0;
    for (size_t i = 0; i < rvecRouteCandidates.size(); ++i)
    {
        std::stringstream strFileName;
        strFileName << "/home/secondo/Traces/GPoints_" << nCall;
        strFileName << "_" << i << ".txt";
        ofstream Stream(strFileName.str().c_str());
        rvecRouteCandidates[i]->Print(Stream);
    }
    ++nCall;
#endif

    if (rvecRouteCandidates.size() > 0)
        return rvecRouteCandidates.front();
    else
        return NULL;
}

/*
3.11 MapMatchingMHT::CreateCompleteRoute
concatenates routes

*/

#if 0
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
                                                         //pData1->GetClosed(),
                                                         true,
                                                         //pData2->GetClosed(),
                                                         false);

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
#else

void MapMatchingMHT::CreateCompleteRoute(
                       const std::vector<MHTRouteCandidate*>& rvecRouteSegments)
{
    MHTRouteCandidate::PointData* pLastPointOfPrevSection = NULL;

    const int nNetworkkId = m_pNetwork->GetId();

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
                  (pData1 == NULL || pData1->GetGPoint(nNetworkkId) == NULL))
            {
                pData1 = rPoints[j];
                ++j;
            }

            // Process next points
            while (j < nPoints && pData1 != NULL &&
                   pData1->GetGPoint(nNetworkkId) != NULL)
            {
                MHTRouteCandidate::PointData* pData2 = NULL;

                while (j < nPoints &&
                       (pData2 == NULL ||
                               pData2->GetGPoint(nNetworkkId) == NULL))
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
                        CalcShortestPath(
                                pLastPointOfPrevSection->GetGPoint(nNetworkkId),
                                pData1->GetGPoint(nNetworkkId),
                                pLastPointOfPrevSection->GetTime(),
                                pData1->GetTime(), true);
                    }

                    bCalcShortestPath = false;
                }

                if (pData2 != NULL && pData2->GetGPoint(nNetworkkId) != NULL)
                {
                    const Interval<Instant> timeInterval(pData1->GetTime(),
                                                         pData2->GetTime(),
                                                         true  /*LC*/,
                                                         false /*RC*/);

                    ConnectPoints(*(pData1->GetGPoint(nNetworkkId)),
                                  *(pData2->GetGPoint(nNetworkkId)),
                                  timeInterval);

                   pLastPointOfPrevSection = pData2;
                }

                pData1 = pData2;
            }
        }
    }
}

#endif

/*
3.11 MapMatchingMHT::TraceRouteCandidates
Debugging of route candidates

*/

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
            pCandidate->Print(Stream, m_pNetwork->GetId());
        Stream << endl;
    }
#endif
}


} // end of namespace mapmatch

