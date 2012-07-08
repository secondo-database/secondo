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

This implementation file contains the implementation of the class
~MapMatchingMHT~.
It is an map matching algorithm based on the
Multiple Hypothesis Technique (MHT)

2 Defines and includes

*/
#include "MapMatchingMHT.h"
#include "MapMatchingUtil.h"
#include "MapMatchingNetworkInterface.h"
#include "MHTRouteCandidate.h"
#include "GPXFileReader.h"

#include <stdio.h>
#include <LogMsg.h>

#include <TemporalAlgebra.h>

namespace mapmatch {

#define MMTRACE(a) cmsg.file("MMTrace.txt") << a << '\n' << std::flush;
#define MMVARTRACE(a) cmsg.file("MMTrace.txt") << #a << a << '\n' << std::flush;


/*
3 class MapMatchingMHT
  Map matching algorithm based on the Multiple Hypothesis Technique (MHT)

3.1 Constructor / Destructor

*/

MapMatchingMHT::MapMatchingMHT(IMMNetwork* pNetwork, MPoint* pMPoint)
:m_pNetwork(pNetwork),
 m_dNetworkScale(pNetwork != NULL ? pNetwork->GetNetworkScale() : 1.0),
 m_pContMMData(new MapMatchDataContainer),
 m_pResCreator(NULL)
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

                m_pContMMData->Append(MMData);

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

                m_pContMMData->Append(MMData);

                ptPrev = ActUPoint.p1;
                timePrev = ActUPoint.timeInterval.end.millisecondsToNull();
            }
        }
    }
}

MapMatchingMHT::MapMatchingMHT(IMMNetwork* pNetwork, std::string strFileName)
:m_pNetwork(pNetwork),
 m_dNetworkScale(pNetwork != NULL ? pNetwork->GetNetworkScale() : 1.0),
 m_pContMMData(new MapMatchDataContainer),
 m_pResCreator(NULL)
{
    try
    {
        GPXFileReader Reader;
        if (Reader.Open(strFileName))
        {
            TrkPointIteratorPtr pIt = Reader.GetTrkPointIterator();
            if (pIt != NULL)
            {
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

                        if (TrkPtData.m_dCourse.IsDefined())
                            MMData.m_dCourse = TrkPtData.m_dCourse.GetValue();

                        if (TrkPtData.m_dSpeed.IsDefined())
                            MMData.m_dSpeed = TrkPtData.m_dSpeed.GetValue();

                        m_pContMMData->Append(MMData);
                    }

                    pIt->Next();
                }
            }
        }
        else
        {
            // Failed read file
        }
    }
    catch(...)
    {
        cmsg.error() << "Error reading file " << strFileName;
        cmsg.send();
    }
}

MapMatchingMHT::MapMatchingMHT(IMMNetwork* pNetwork,
                               shared_ptr<MapMatchDataContainer> pContMMData)
:m_pNetwork(pNetwork),
 m_dNetworkScale(pNetwork != NULL ? pNetwork->GetNetworkScale() : 1.0),
 m_pContMMData(pContMMData),
 m_pResCreator(NULL)
{

}

// Destructor
MapMatchingMHT::~MapMatchingMHT()
{
    m_pNetwork = NULL;
    m_pResCreator = NULL;
}

bool MapMatchingMHT::InitMapMatching(IMapMatchingMHTResultCreator* pResCreator)
{
    if (pResCreator == NULL)
        return false;
    else
    {
        m_pResCreator = pResCreator;

        return (m_pNetwork != NULL && m_pNetwork->IsDefined() &&
                m_pContMMData != NULL && m_pContMMData->Size() > 0);
    }
}

/*
3.2 MapMatchingMHT::DoMatch
Main-Method for MapMatching

*/

bool MapMatchingMHT::DoMatch(IMapMatchingMHTResultCreator* pResCreator)
{
    // Initialization
    if (!InitMapMatching(pResCreator))
    {
        return false;
    }

    // Step 1 - Subdividing trip
    // Detect spatial and temporal gaps in input-data
    // Check quality of input-data
    vector<shared_ptr<MapMatchDataContainer> > vecTripSegments;
    TripSegmentation(vecTripSegments);

    // Steps 2-5
    std::vector<MHTRouteCandidate*> vecRouteSegments;

    for (vector<shared_ptr<MapMatchDataContainer> >::iterator
                                                   it = vecTripSegments.begin();
         it != vecTripSegments.end();
         ++it)
    {
        shared_ptr<MapMatchDataContainer> pContMMData(*it);
        if (pContMMData == NULL)
            continue;

        // Step 2 - calculate missing attributes (heading, speed)
        CompleteData(pContMMData.get());

        size_t nIdxFirstComponent = 0;

        while(nIdxFirstComponent >= 0 &&
              nIdxFirstComponent < pContMMData->Size())
        {
            // Step 3 - Determination of initial route/segment candidates
            std::vector<MHTRouteCandidate*> vecRouteCandidates;
            nIdxFirstComponent = GetInitialRouteCandidates(pContMMData.get(),
                                                           nIdxFirstComponent,
                                                           vecRouteCandidates);

            // Step 4 - Route developement
            nIdxFirstComponent = DevelopRoutes(pContMMData.get(),
                                               nIdxFirstComponent,
                                               vecRouteCandidates);

            // Step 5 - Selection of most likely candidate
            MHTRouteCandidate* pBestCandidate = DetermineBestRouteCandidate(
                                                            vecRouteCandidates);
            if (pBestCandidate != NULL)
            {
                vecRouteSegments.push_back(pBestCandidate);

                if (nIdxFirstComponent < pContMMData->Size())
                {
                    // Not all components processed
                    // -> matching failed (bad network)
                    // Don't connect this candidate with next by shortest path
                    pBestCandidate->SetFailed(true);
                }

//#define CALC_AVG_SCORE
#ifdef CALC_AVG_SCORE
                std::ostream& rStream = cmsg.file("MMAvgScorePerPoint.log");
                double dAvgScore = pBestCandidate->GetScore() /
                                               pBestCandidate->GetCountPoints();
                rStream << "AvgScore: " << dAvgScore << ", Points: ";
                rStream << pBestCandidate->GetCountPoints() << endl;
#endif
            }

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
    //          Create result
    CreateCompleteRoute(vecRouteSegments);

    // cleanup
    while (vecRouteSegments.size() > 0)
    {
        MHTRouteCandidate* pCandidate = vecRouteSegments.back();
        delete pCandidate;
        vecRouteSegments.pop_back();
    }

    return true;
}

/*
3.3 MapMatchingMHT::CompleteData
    calculate missing attributes (heading, speed)

*/

void MapMatchingMHT::CompleteData(MapMatchDataContainer* pContMMData)
{
    if (pContMMData == NULL)
        return;

    // Calculate missing heading and speed information
    const size_t nSize = pContMMData->Size();

    if (nSize < 2)
        return;

    const MapMatchData* pDataCurrent = pContMMData->Get(0);
    if (pDataCurrent == NULL)
    {
        assert(false);
        return;
    }

    Point PtCurrent = pDataCurrent->GetPoint();
    const MapMatchData* pDataPrev = NULL;

    for (size_t i = 1; i < nSize; ++i)
    {
        const MapMatchData* pDataNext = pContMMData->Get(i);
        if (pDataNext != NULL)
        {
            Point PtNext = pDataNext->GetPoint();

            MapMatchData DataCurrent(*pDataCurrent);

            bool bModified = false;

            // Calculate missing speed
            // (avg. speed between previous point and current point)

            double dDistancePrevCurrent = -1.0;
            if (pDataPrev != NULL &&
                (DataCurrent.m_dSpeed < 0.0 ||
                 (AlmostEqual(DataCurrent.m_dSpeed, 0.0) &&
                  (dDistancePrevCurrent = MMUtil::CalcDistance(
                                                     PtCurrent,
                                                     pDataPrev->GetPoint(),
                                                     m_dNetworkScale)) > 50.)))
            {
                if (dDistancePrevCurrent < 0.0)
                    dDistancePrevCurrent = MMUtil::CalcDistance(
                                                          PtCurrent,
                                                          pDataPrev->GetPoint(),
                                                          m_dNetworkScale); // m

                long dTimeDiff = abs((long)(DataCurrent.m_Time -
                                            pDataPrev->m_Time)); // ms
                dTimeDiff /= 1000; // s

                DataCurrent.m_dSpeed  = dDistancePrevCurrent / dTimeDiff; // m/s

                bModified = true;
            }

            // Calculate missing heading
            if (DataCurrent.m_dCourse < 0.0 &&  // Course not defined
                (DataCurrent.m_dSpeed > 5.0 ||  // faster than 18 km/h = 5 m/s
                 (DataCurrent.m_dSpeed < 0.0 && // Speed not defined and
                  MMUtil::CalcDistance(PtCurrent, // Distance > 50 m
                                       PtNext,
                                       m_dNetworkScale) > 50.)))
            {
                DataCurrent.m_dCourse = MMUtil::CalcHeading(PtCurrent,
                                                            PtNext,
                                                            false,
                                                            m_dNetworkScale);
                bModified = true;
            }

            if (bModified)
            {
                pContMMData->Put(i - 1, DataCurrent);
                pDataCurrent = pContMMData->Get(i-1);
            }

            pDataPrev = pDataCurrent;
            pDataCurrent = pDataNext;

            PtCurrent = PtNext;
        }
    }
}

/*
3.4 MapMatchingMHT::TripSegmentation
    Detect spatial and temporal gaps in input-data
    Check quality of input-data

*/

void MapMatchingMHT::TripSegmentation(
                std::vector<shared_ptr<MapMatchDataContainer> >& rvecTripParts)
{
    if (m_pNetwork == NULL || !m_pNetwork->IsDefined() || m_pContMMData == NULL)
        return;

    // Detect spatial and temporal gaps in input-data
    // Divide the data if the time gap is longer than 60 seconds or
    // the distance is larger than 750 meters

    const double dMaxDistance = 750.0; // 750 meters
    const int64_t MaxTimeDiff = 60000; // 60 seconds
    const int64_t MaxTimeDiff2 = 180000; // 180 seconds

    shared_ptr<MapMatchDataContainer> pActCont;
    const MapMatchData* pActData = NULL;
    //DateTime prevEndTime(instanttype);
    int64_t  prevEndTime = 0;
    Point    prevEndPoint(false);
    bool     bProcessNext = true;

    const size_t nMMComponents = m_pContMMData->Size();

    const Rectangle<2> rectBoundingBoxNetwork = m_pNetwork->GetBoundingBox();

    for (size_t i = 0; i < nMMComponents; bProcessNext ? i++ : i)
    {
        if (bProcessNext)
        {
            pActData = m_pContMMData->Get(i);
            if (pActData == NULL)
                continue; // process next
        }

        bProcessNext = true;

        if (!pActData->GetPoint().Inside(rectBoundingBoxNetwork))
        {
            // Outside bounding box of network
            continue; // process next unit
        }

        if (!CheckQualityOfGPSFix(*pActData))
        {
            // bad quality of GPS fix
//#define TRACE_BAD_DATA
#ifdef TRACE_BAD_DATA
            std::ostream& rStream = cmsg.file("MMBadData.log");
            pActData->Print(rStream);
            rStream << endl << std::flush;
#endif
            continue; // process next unit
        }

        if (pActCont == NULL)
        {
            // create new Data
            pActCont =
                   shared_ptr<MapMatchDataContainer>(new MapMatchDataContainer);

            // Add data
            pActCont->Append(*pActData);

            prevEndTime = pActData->m_Time;
            prevEndPoint = pActData->GetPoint();
        }
        else
        {
            const double dDistance = MMUtil::CalcDistance(prevEndPoint,
                                                          pActData->GetPoint(),
                                                          m_dNetworkScale);

            bool bGap = dDistance > dMaxDistance;

            if (!bGap && ((pActData->m_Time - prevEndTime) > MaxTimeDiff))
            {
                // Temporal gap
                // check, whether the distance is very small
                // (stop at traffic light)

                bGap = (dDistance > 100.0 ||
                        (pActData->m_Time - prevEndTime) >  MaxTimeDiff2);
            }

            if (bGap)
            {
//#define TRACE_GAPS
#ifdef TRACE_GAPS
                std::ostream rStreamGaps = cmsg.file("MMGaps.txt");
                rStreamGaps << "*****************" << endl;
                rStreamGaps << "Gap detected : ";
                pActData->Print(rStreamGaps);
                rStreamGaps << endl;
                rStreamGaps << "TimeDiff: " << (pActData->m_Time - prevEndTime);
                rStreamGaps << endl;
                rStreamGaps << "Distance: " << dDistance;
                rStreamGaps << endl << std::flush;
#endif

                // gap detected -> finalize current array
                if (pActCont->Size() >= 10)
                {
                    rvecTripParts.push_back(pActCont);
                    pActCont = shared_ptr<MapMatchDataContainer>();
                }
                else
                {
                    // less than 10 components -> drop
                    pActCont = shared_ptr<MapMatchDataContainer>();
                }

                bProcessNext = false; // Process ActData once again
            }
            else
            {
                // no gap between current and previous Data

                pActCont->Append(*pActData);
                prevEndTime = pActData->m_Time;
                prevEndPoint = pActData->GetPoint();
            }
        }
    }

    // finalize last Array
    if (pActCont != NULL)
    {
        rvecTripParts.push_back(pActCont);
    }
}

/*
3.5 MapMatchingMHT::CheckQualityOfGPSFix

*/

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
3.6 MapMatchingMHT::GetInitialRouteCandidates
    Find first route candidates

*/

int MapMatchingMHT::GetInitialRouteCandidates(
                           const MapMatchDataContainer* pContMMData,
                           int nIdxFirstComponent,
                           std::vector<MHTRouteCandidate*>& rvecRouteCandidates)
{
    if (pContMMData == NULL || pContMMData->Size() == 0)
        return -1;

    // Get first (defined) point and search for sections
    // in the vicinity of the point.
    // Process next (defined) point, if no section was found.
    size_t nIndexFirst = nIdxFirstComponent;
    const size_t nSize = pContMMData->Size();

    while (rvecRouteCandidates.size() == 0 &&
           nIndexFirst < nSize)
    {
        const MapMatchData* pData = pContMMData->Get(nIndexFirst);
        if (pData != NULL)
        {
            // get first section candidates
            GetInitialRouteCandidates(pData->GetPoint(), rvecRouteCandidates);
        }

        if (rvecRouteCandidates.size() == 0)
            ++nIndexFirst;
    }

    return rvecRouteCandidates.size() > 0 ? nIndexFirst: -1;
}

void MapMatchingMHT::GetInitialRouteCandidates(const Point& rPoint,
                           std::vector<MHTRouteCandidate*>& rvecRouteCandidates)
{
    if (m_pNetwork == NULL)
        return;

    Point pt(rPoint);
    pt.Scale(1.0 / m_dNetworkScale);
    if (!pt.checkGeographicCoord())
    {
        cmsg.error() << "Invalid geographic coord" << endl;
        cmsg.send();
        return;
    }

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

    std::vector<shared_ptr<IMMNetworkSection> > vecSections;
    m_pNetwork->GetSections(BBox, vecSections);

    const size_t nSections = vecSections.size();
    for (size_t i = 0; i < nSections; ++i)
    {
        MHTRouteCandidate* pCandidate = new MHTRouteCandidate(this);
        pCandidate->AddSection(vecSections[i]);

        rvecRouteCandidates.push_back(pCandidate);
    }
}


/*
3.7 MapMatchingMHT::DevelopRoutes

*/

int MapMatchingMHT::DevelopRoutes(const MapMatchDataContainer* pContMMData,
                           int nIndexFirstComponent,
                           std::vector<MHTRouteCandidate*>& rvecRouteCandidates)
{
    if (pContMMData == NULL ||
        m_pNetwork == NULL || !m_pNetwork->IsDefined() ||
        nIndexFirstComponent < 0)
        return -1;

    const size_t nNoComponents = pContMMData->Size();

    Point ptPrev(false);
    int64_t timePrev = 0;

    for (size_t i = nIndexFirstComponent; i < nNoComponents; ++i)
    {
        const MapMatchData* pActData = pContMMData->Get(i);
        if (pActData == NULL)
            continue;

        if (ptPrev != pActData->GetPoint() &&
            timePrev != pActData->m_Time)
        {
            // Develop routes with point p0
            DevelopRoutes(pActData,
                          rvecRouteCandidates);

            ptPrev = pActData->GetPoint();
            timePrev = pActData->m_Time;

            TraceRouteCandidates(rvecRouteCandidates,
                                 "##### Nach DevelopRoutes #####");

            // Reduce Routes
            ReduceRouteCandidates(rvecRouteCandidates);
            TraceRouteCandidates(rvecRouteCandidates,
                                 "##### Nach Reduce #####");
        }

        if (!CheckRouteCandidates(rvecRouteCandidates))
        {
            ostream& rStreamBadNetwork = cmsg.file("MMBadNetwork.log");
            rStreamBadNetwork << "CheckRouteCandidates failed: " << endl;
            pActData->Print(rStreamBadNetwork);
            rStreamBadNetwork << endl << std::flush;

            // Matching failed - Restart with next component
            return i+1;
        }
    }

    return nNoComponents;
}

class EndPtDistanceFilter: public ISectionFilter
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

    bool IsValid(const IMMNetworkSection* pSection)
    {
#if 1
        const SimpleLine* pCurve = pSection != NULL ? pSection->GetCurve() : 0;

        if (pCurve == NULL || !pCurve->IsDefined() ||
            !m_rPoint.IsDefined())
            return false;

        Point Pt(false);

        if (pSection->GetDirection() == IMMNetworkSection::DIR_UP)
            Pt = pCurve->EndPoint(pSection->GetCurveStartsSmaller());
        else
            Pt = pCurve->StartPoint(pSection->GetCurveStartsSmaller());

        if (!Pt.IsDefined())
            return false;

        return MMUtil::CalcDistance(Pt, m_rPoint, m_dScale) < m_dMaxDistance;
#else
        if (pSection == NULL || !m_rPoint.IsDefined())
            return false;

        const Point& rPt = pSection->GetEndPoint();
        if (!rPt.IsDefined())
            return false;

        return MMUtil::CalcDistance(rPt, m_rPoint, m_dScale) < m_dMaxDistance;
#endif
    }

private:
    const Point& m_rPoint;
    const double& m_dMaxDistance;
    const double& m_dScale;
};


bool MapMatchingMHT::CheckRoadType(
                                  const shared_ptr<IMMNetworkSection>& pSection,
                                  const MapMatchData& rMMData)
{
    if (pSection == NULL)
        return false;

    if (rMMData.m_dSpeed > 0.0)
    {
        // m/s -> km/h
        const double dSpeedKMH = rMMData.m_dSpeed * 60. * 60. / 1000.;
        /*const double dMaxSpeed = pSection->GetMaxSpeed();

        if (dMaxSpeed > 0.0) // Defined
        {
            double dFactor = 1.0;
            if (dMaxSpeed <= 30.0)
                dFactor = 2.2;
            else if (dMaxSpeed <= 50.0)
                dFactor = 1.8;
            else
                dFactor = 1.4;

            if (dSpeedKMH > (dMaxSpeed * dFactor))
                return false;
        }
        else*/
        {
            IMMNetworkSection::ERoadType eRoadType = pSection->GetRoadType();
            if (eRoadType == IMMNetworkSection::RT_PEDESTRIAN ||
                eRoadType == IMMNetworkSection::RT_PATH ||
                eRoadType == IMMNetworkSection::RT_FOOTWAY ||
                eRoadType == IMMNetworkSection::RT_STEPS)
            {
                if (dSpeedKMH > 20.0)
                    return false;
            }
            else if (eRoadType == IMMNetworkSection::RT_CYCLEWAY ||
                     eRoadType == IMMNetworkSection::RT_BRIDLEWAY)
            {
                if (dSpeedKMH > 50.0)
                    return false;
            }
        }

    }

    return true;
}


void MapMatchingMHT::DevelopRoutes(const MapMatchData* pMMData,
                          std::vector<MHTRouteCandidate*>& rvecRouteCandidates,
                          int nMaxLookAhead)
{
    if (pMMData == NULL)
        return;

    const bool bCheckRoadType = m_pNetwork->CanGetRoadType();

    std::vector<MHTRouteCandidate*> vecNewRouteCandidates;

    std::vector<MHTRouteCandidate*>::iterator it;
    std::vector<MHTRouteCandidate*>::iterator itEnd = rvecRouteCandidates.end();
    for (it = rvecRouteCandidates.begin(); it != itEnd; ++it)
    {
        MHTRouteCandidate* pCandidate = *it;
        if (pCandidate == NULL)
        {
            continue;
        }

        // Check Road
        if (bCheckRoadType &&
            !CheckRoadType(pCandidate->GetLastSection(), *pMMData))
        {
            *it = NULL;
            delete pCandidate;
            pCandidate = NULL;
            continue;
        }

        // Projection
        ENextCandidates eNextCandidates = CANDIDATES_NONE;
        if (!AssignPoint(pCandidate, pMMData, eNextCandidates))
        {
            // Point could not be assigned to last section of candidate
            // -> add adjacent sections

            // max nMaxLookAhead sections look ahead
            // no look ahead when processing first point of route
            if (pCandidate->GetCountPoints() > 0 &&
                pCandidate->GetCountLastEmptySections() <= nMaxLookAhead)
            {
                std::vector<MHTRouteCandidate*> vecNewRouteCandidatesLocal;

                const shared_ptr<IMMNetworkSection>& pSection =
                                                   pCandidate->GetLastSection();
                if (pSection == NULL || !pSection->IsDefined())
                {
                    *it = NULL;
                    delete pCandidate;
                    pCandidate = NULL;
                    continue;
                }

                const SimpleLine* pCurve = pSection->GetCurve();
                if (pCurve == NULL || !pCurve->IsDefined())
                {
                    *it = NULL;
                    delete pCandidate;
                    pCandidate = NULL;
                    continue;
                }

                const bool bStartsSmaller = pSection->GetCurveStartsSmaller();
                const Point& rPtStart = pCurve->StartPoint(bStartsSmaller);
                const Point& rPtEnd = pCurve->EndPoint(bStartsSmaller);

                //const Point& rPtStart = pSection->GetStartPoint();
                //const Point& rPtEnd = pSection->GetEndPoint();

                if (!rPtStart.IsDefined() || !rPtEnd.IsDefined())
                {
                    ostream& rStreamBadNetwork = cmsg.file("MMBadNetwork.log");
                    rStreamBadNetwork << "Undefined start- or endpoint: ";
                    rStreamBadNetwork << "Section: ";
                    pSection->PrintIdentifier(rStreamBadNetwork);
                    rStreamBadNetwork << endl << std::flush;

                    *it = NULL;
                    delete pCandidate;
                    pCandidate = NULL;
                    continue;
                }

                const Point& rPoint = pMMData->GetPoint();

                const double dDistanceStart = MMUtil::CalcDistance(
                                                               rPoint,
                                                               rPtStart,
                                                               m_dNetworkScale);

                const double dDistanceEnd = MMUtil::CalcDistance(
                                                               rPoint,
                                                               rPtEnd,
                                                               m_dNetworkScale);

                const bool bUpDown = dDistanceStart > dDistanceEnd;

                const bool bUpDownDriving =
                        (pSection->GetDirection() == IMMNetworkSection::DIR_UP);

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
                    // when distance of endpoint of adjacent section
                    // to GPS-Point is smaller
                    EndPtDistanceFilter Filter(rPoint,
                                         std::min(dDistanceStart, dDistanceEnd),
                                         //bUpDownDriving ? dDistanceEnd :
                                         //                 dDistanceStart,
                                         m_dNetworkScale);

                    AddAdjacentSections(pCandidate, bUpDownDriving,
                                        vecNewRouteCandidatesLocal, &Filter);
                }

                if (vecNewRouteCandidatesLocal.size() > 0)
                {
                    // !! Rekursion !!
                    DevelopRoutes(pMMData,
                                  vecNewRouteCandidatesLocal,
                                  nMaxLookAhead);

                    vecNewRouteCandidates.insert(vecNewRouteCandidates.end(),
                            vecNewRouteCandidatesLocal.begin(),
                            vecNewRouteCandidatesLocal.end());
                }

                *it = NULL;

                if (pCandidate->GetCountLastEmptySections() == 0)
                {
                    // "Offroad"-Point
                    pCandidate->AddPoint(pMMData);

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
                // Point was assigned, but check also adjacent sections.
                // Remove assigned point and assign to adjacent sections.

                std::vector<MHTRouteCandidate*> vecNewRouteCandidatesLocal;

                if (eNextCandidates == CANDIDATES_UP ||
                    eNextCandidates == CANDIDATES_UP_DOWN)
                {
                    AddAdjacentSections(pCandidate, true,
                                        vecNewRouteCandidatesLocal,
                                        NULL,
                                        true /* RemoveLastPoint */);
                }

                if (eNextCandidates == CANDIDATES_DOWN ||
                    eNextCandidates == CANDIDATES_UP_DOWN)
                {
                    AddAdjacentSections(pCandidate, false,
                                        vecNewRouteCandidatesLocal,
                                        NULL,
                                        true /* RemoveLastPoint */);
                }

                if (vecNewRouteCandidatesLocal.size() > 0)
                {
                    // !! Rekursion !!
                    DevelopRoutes(pMMData, vecNewRouteCandidatesLocal, 2);

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
3.8 MapMatchingMHT::AssignPoint
Assign point to route candidate, if possible

*/

bool MapMatchingMHT::AssignPoint(MHTRouteCandidate* pCandidate,
                                 const MapMatchData* pMMData,
                                 MapMatchingMHT::ENextCandidates& eNext)
{
    if (pMMData == NULL)
        return false;

    eNext = CANDIDATES_NONE;

    const shared_ptr<IMMNetworkSection>& pSection =
                                                   pCandidate->GetLastSection();
    if (pSection == NULL || !pSection->IsDefined() || m_pNetwork == NULL)
    {
        return false;
    }

    const SimpleLine* pCurve = pSection->GetCurve();
    if (pCurve == NULL || !pCurve->IsDefined())
    {
        return false;
    }

    const Point& rPoint(pMMData->GetPoint());

    double dDistance = 0.0;
    bool bIsOrthogonal = false;
    HalfSegment HSProjection;
    Point PointProjection = MMUtil::CalcProjection(*pCurve, rPoint,
                                                   dDistance, bIsOrthogonal,
                                                   m_dNetworkScale,
                                                   &HSProjection);

    if (PointProjection.IsDefined())
    {
        // Check if the startpoint or endpoint has been reached.
        const bool bStartsSmaller = pSection->GetCurveStartsSmaller();
        const Point ptStart = pCurve->StartPoint(bStartsSmaller);
        const Point ptEnd = pCurve->EndPoint(bStartsSmaller);

        //const Point ptStart = pSection->GetStartPoint();
        //const Point ptEnd = pSection->GetEndPoint();

        bool bIsEndPoint = false;

        // Only assign to endpoint if it is orthogonal or distance <= 150 m
        if (!bIsOrthogonal)
        {
            if (pSection->GetDirection() == IMMNetworkSection::DIR_UP)
            {
                if (AlmostEqual(PointProjection, ptEnd))
                    bIsEndPoint = true;
            }
            else if (pSection->GetDirection() == IMMNetworkSection::DIR_DOWN)
            {
                if (AlmostEqual(PointProjection, ptStart))
                    bIsEndPoint = true;
            }
            else
            {
                assert(false);
            }

            if (bIsEndPoint)
            {
                // Only assign to endpoint if distance <= 150.0 m
                if (dDistance > 150.0)
                    return false;
            }
        }

        // Only assign to startpoint if it is orthogonal
        if (!bIsOrthogonal && !bIsEndPoint)
        {
            if (pSection->GetDirection() == IMMNetworkSection::DIR_UP)
            {
                if (AlmostEqual(PointProjection, ptStart))
                    return false;
            }
            else if (pSection->GetDirection() == IMMNetworkSection::DIR_DOWN)
            {
                if (AlmostEqual(PointProjection, ptEnd))
                    return false;
            }
        }

        bool bLookAtAdjacent = bIsEndPoint;

        if (!bLookAtAdjacent)
        {
            // Check distance travelled by GPS-Points
            const vector<MHTRouteCandidate::PointDataPtr>&
                    vecPointsLastSection = pCandidate->GetPointsOfLastSection();
            const size_t nPointsLastSection = vecPointsLastSection.size();

            vector<Point> vecPoints;
            for (size_t i = 0; i < nPointsLastSection; ++i)
            {
                vecPoints.push_back(vecPointsLastSection[i]->GetPointGPS());
            }
            vecPoints.push_back(rPoint);

            double dDistanceTravelled = MMUtil::CalcDistance(vecPoints,
                                                             m_dNetworkScale);

            // Add distance from startpoint to first projected point
            const Point* pFirstProjectedPoint = &PointProjection;
            if (nPointsLastSection > 0 && vecPointsLastSection[0] != NULL)
            {
                pFirstProjectedPoint =
                                  vecPointsLastSection[0]->GetPointProjection();
            }

            if (pSection->GetDirection() == IMMNetworkSection::DIR_UP &&
                pFirstProjectedPoint != NULL)
            {
                dDistanceTravelled += MMUtil::CalcDistance(
                                                          ptStart,
                                                          *pFirstProjectedPoint,
                                                          m_dNetworkScale);
            }
            else if (pSection->GetDirection() == IMMNetworkSection::DIR_DOWN &&
                     pFirstProjectedPoint != NULL)
            {
                dDistanceTravelled += MMUtil::CalcDistance(
                                                          ptEnd,
                                                          *pFirstProjectedPoint,
                                                          m_dNetworkScale);
            }

            /*if (pFirstProjectedPoint != NULL)
            {
                dDistanceTravelled += MMUtil::CalcDistance(
                                                          ptStart,
                                                          *pFirstProjectedPoint,
                                                          m_dNetworkScale);
            }*/

            const double dLenCurve = pSection->GetCurveLength(m_dNetworkScale);


            // If traveled (GPS-)distance is larger than 80% of curve length
            // then look at adjacent sections
            bLookAtAdjacent = dDistanceTravelled > (dLenCurve / 100. * 80.);
        }

//#define CHECK_HEADING_DIFF
#ifdef CHECK_HEADING_DIFF
        if (!bLookAtAdjacent && pMMData->m_dCourse >= 0.0 /* Course defined */)
        {
            double dDirectionSection = MMUtil::CalcHeading(pSection.get(),
                                                           HSProjection,
                                                           m_dNetworkScale);

            double dDiff = std::min(abs(pMMData->m_dCourse - dDirectionSection),
                            360. - abs(pMMData->m_dCourse - dDirectionSection));

            bLookAtAdjacent = dDiff > 85.;
        }
#endif

        if (bLookAtAdjacent)
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

            if ((pSection->GetDirection() == IMMNetworkSection::DIR_UP &&
                 eNext != CANDIDATES_UP) ||
                (pSection->GetDirection() == IMMNetworkSection::DIR_DOWN &&
                 eNext != CANDIDATES_DOWN))
            {
                eNext = CANDIDATES_UP_DOWN;
            }
        }

        pCandidate->AddPoint(PointProjection,
                             HSProjection,
                             pMMData,
                             dDistance);
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
    if (pRC1 == NULL || pRC2 == NULL)
    {
        // Pointer == NULL -> very bad score
        return (pRC1 != NULL);
    }
    return pRC1->GetScore() < pRC2->GetScore();
}


static bool LastPointsEqual(const MHTRouteCandidate* pRouteCandidate1,
                            const MHTRouteCandidate* pRouteCandidate2,
                            int nCnt)
{
    MHTRouteCandidate::PointDataIterator itData  =
                                            pRouteCandidate1->PointDataRBegin();
    MHTRouteCandidate::PointDataIterator itDataEnd =
                                              pRouteCandidate2->PointDataREnd();
    MHTRouteCandidate::PointDataIterator itData2 =
                                            pRouteCandidate2->PointDataRBegin();
    MHTRouteCandidate::PointDataIterator itData2End =
                                              pRouteCandidate2->PointDataREnd();


    while (nCnt > 0 && itData != itDataEnd && itData2 != itData2End)
    {
        const MHTRouteCandidate::PointDataPtr& pData1 = *itData;
        const MHTRouteCandidate::PointDataPtr& pData2 = *itData2;

        if (!(*pData1 == *pData2))
            return false;

        --nCnt;

        ++itData;
        ++itData2;
    }

    return true;
}


/*
3.9 MapMatchingMHT::ReduceRouteCandidates
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

    const bool bInInitPhase = (rvecRouteCandidates[0]->GetCountPoints() < 5);

    if (bInInitPhase && rvecRouteCandidates.size() < 20)
        return;

    // maximum 20
    while (rvecRouteCandidates.size() > 20)
    {
        MHTRouteCandidate* pCandidate = rvecRouteCandidates.back();
        rvecRouteCandidates.pop_back();
        if (pCandidate != NULL)
            delete pCandidate;
    }

    // Find duplicates
    size_t nCandidates = rvecRouteCandidates.size();
    for (size_t i = 0; i < nCandidates; ++i)
    {
        MHTRouteCandidate* pRouteCandidate1 = rvecRouteCandidates[i];
        if (pRouteCandidate1 == NULL)
            continue;

        const shared_ptr<IMMNetworkSection>& pLastSection1 =
                                             pRouteCandidate1->GetLastSection();
        if (pLastSection1 == NULL)
            continue;

        for (size_t j = i + 1; j < nCandidates; ++j)
        {
            MHTRouteCandidate* pRouteCandidate2 = rvecRouteCandidates[j];
            if (pRouteCandidate2 == NULL)
                continue;

            const shared_ptr<IMMNetworkSection>& pLastSection2 =
                                             pRouteCandidate2->GetLastSection();
            if (pLastSection2 == NULL)
                continue;

            if (!bInInitPhase &&
                 *pLastSection1 == *pLastSection2 &&
                LastPointsEqual(pRouteCandidate1, pRouteCandidate2, 4))
            {
                rvecRouteCandidates[j] = NULL;
                delete pRouteCandidate2;
            }
        }
    }

    std::sort(rvecRouteCandidates.begin(),
              rvecRouteCandidates.end(),
              RouteCandidateScoreCompare);

    // remove invalid candidates
    while (rvecRouteCandidates.size() > 1)
    {
        MHTRouteCandidate* pCandidate = rvecRouteCandidates.back();
        if (pCandidate == NULL)
        {
            rvecRouteCandidates.pop_back();
        }
        else
        {
            const MHTRouteCandidate::PointDataPtr& pDataLastPt =
                                                     pCandidate->GetLastPoint();

            if (pDataLastPt != NULL && pDataLastPt->GetDistance() > 200.)
            {
                // Too far away -> remove
                delete pCandidate;
                rvecRouteCandidates.pop_back();
            }
            else
            {
                break;
            }
        }
    }
}

/*
3.10 MapMatchingMHT::CheckRouteCandidates

*/

bool MapMatchingMHT::CheckRouteCandidates(const std::vector<MHTRouteCandidate*>&
                                                            rvecRouteCandidates)
{
    size_t nFailedCandidates = 0;
    const size_t nCandidates = rvecRouteCandidates.size();

    if (nCandidates == 0)
        return false;

    if (rvecRouteCandidates[0] == NULL) // Candidates should be sorted by
        return false;                   // score -> ReduceRouteCandidates
                                        // Best candidate is invalid

    if (nCandidates == 1)
    {
        const MHTRouteCandidate::PointDataPtr& pDataLastPt =
                                         rvecRouteCandidates[0]->GetLastPoint();

        if (pDataLastPt->GetDistance() > 200.)
            return false; // Best candidate is too far away
    }

    for (size_t i = 0; i < nCandidates; ++i)
    {
        const MHTRouteCandidate* pCandidate = rvecRouteCandidates[i];
        if (pCandidate == NULL || pCandidate->GetCountLastOffRoadPoints() > 0)
            ++nFailedCandidates;
    }

    return (nFailedCandidates != nCandidates);
}

/*
3.11 MapMatchingMHT::CheckUTurn

*/

bool MapMatchingMHT::CheckUTurn(const MHTRouteCandidate* pCandidate,
                                bool bUpDown,
                                bool& rbCorrectUTurn,
                                double& rdAdditionalUTurnScore)
{
    if (pCandidate == NULL || pCandidate->GetCountRouteSegments() <= 1)
    {
        // First section -> do not assign adjacent sections (No U-Turn)
        return false;
    }

    const shared_ptr<IMMNetworkSection>& pSection =
                                                   pCandidate->GetLastSection();
    if (pSection == NULL)
        return false;

    size_t nPointsOfLastSection = pCandidate->GetCountPointsOfLastSection();

    if (nPointsOfLastSection == 0)
    {
        // No points assigned
        return false;
    }
    else if (nPointsOfLastSection == 1)
    {
        // U-Turn and only one assigned point to last section

        //rbCorrectUTurn = true;
        return false;
    }
    else if (nPointsOfLastSection == 2)
    {
        // U-Turn and only two assigned points to last section
        // -> Remove last points and assign these points to
        // endpoint of previous section

        rbCorrectUTurn = true;
    }
    else if (nPointsOfLastSection <= 10)
    {
        // more than 2 points ->
        // check distance between projected points

        const std::vector<MHTRouteCandidate::PointDataPtr>& rvecPoints =
                                           pCandidate->GetPointsOfLastSection();

        const double dMaxAllowedDistance = 15.0; // 15 m

        //assert(rvecPoints.size() == nPointsOfLastSection);

        double dMaxDistance = 0.0;

        for (size_t i = 0;
             i < nPointsOfLastSection - 1 && dMaxDistance < dMaxAllowedDistance;
             ++i)
        {
            const MHTRouteCandidate::PointDataPtr& pData1 = rvecPoints[i];
            const Point* pPt1 = ((pData1 != NULL) ?
                                           pData1->GetPointProjection() : NULL);

            if (pPt1 == NULL || !pPt1->IsDefined())
                continue;

            for (size_t j = i + 1;
                 j < nPointsOfLastSection && dMaxDistance < dMaxAllowedDistance;
                 ++j)
            {
                const MHTRouteCandidate::PointDataPtr& pData2 = rvecPoints[j];
                const Point* pPt2 = ((pData2 != NULL) ?
                                           pData2->GetPointProjection() : NULL);

                if (pPt2 != NULL && pPt2->IsDefined())
                {
                    double dDist = MMUtil::CalcDistance(*pPt1,
                                                        *pPt2,
                                                        m_dNetworkScale);

                    if (dDist > dMaxDistance)
                    {
                        dMaxDistance = dDist;
                    }
                }
            }
        }

        rbCorrectUTurn = (dMaxDistance < dMaxAllowedDistance);

        Point PtMaxDistance1(false);
        Point PtMaxDistance2(false);

        if (!rbCorrectUTurn || nPointsOfLastSection > 5)
        {
            double dMaxDistanceGPS = 0.0;
            for (size_t i = 0; i < nPointsOfLastSection - 1; ++i)
            {
                const MHTRouteCandidate::PointDataPtr& pData1 = rvecPoints[i];

                const Point& rPt1 = pData1 != NULL ?
                                           pData1->GetPointGPS() : Point(false);

                for (size_t j = i + 1; j < nPointsOfLastSection; ++j)
                {
                    const MHTRouteCandidate::PointDataPtr& pData2 =
                                                                  rvecPoints[j];
                    const Point& rPt2 = pData2 != NULL ?
                                           pData2->GetPointGPS() : Point(false);

                    if (rPt1.IsDefined() && rPt2.IsDefined())
                    {
                        double dDist = MMUtil::CalcDistance(rPt1, rPt2,
                                                            m_dNetworkScale);

                        if (dDist > dMaxDistance)
                        {
                            dMaxDistanceGPS = dDist;

                            PtMaxDistance1 = rPt1;
                            PtMaxDistance2 = rPt2;
                        }
                    }
                }
            }
        }

        if ((!rbCorrectUTurn || nPointsOfLastSection > 5 )&&
             PtMaxDistance1.IsDefined() && PtMaxDistance2.IsDefined())
        {
            // Check direction
            double dDirectionGPS = MMUtil::CalcHeading(PtMaxDistance1,
                                                       PtMaxDistance2,
                                                       false /*AtEndPoint*/,
                                                       m_dNetworkScale);

            const Point& rPtSectionStart = pSection->GetStartPoint();
            const Point& rPtSectionEnd = pSection->GetEndPoint();

            if (rPtSectionStart.IsDefined() && rPtSectionEnd.IsDefined())
            {
                double dDirectionSection = MMUtil::CalcHeading(rPtSectionStart,
                                                               rPtSectionEnd,
                                                               false,
                                                               m_dNetworkScale);

                double dDiff = std::min(abs(dDirectionGPS - dDirectionSection),
                                 360. - abs(dDirectionGPS - dDirectionSection));

                rbCorrectUTurn = (abs(dDiff) > 45.0);
            }
        }
    }

    // Calculate Score for U-Turn (only when no UTurn-correction)
    const SimpleLine* pCurve = pSection->GetCurve();
    if (!rbCorrectUTurn && pCurve != NULL && pCurve->IsDefined())
    {
        const Point& rPtRef = pSection->GetStartPoint();

        const vector<MHTRouteCandidate::PointDataPtr>&
                    vecPointsLastSection = pCandidate->GetPointsOfLastSection();

        // Calculate maximum distance of points of last
        // section to End-/StartPoint
        const size_t nPoints = vecPointsLastSection.size();
        for (size_t i = 0; i < nPoints; ++i)
        {
            const Point* pPt = vecPointsLastSection[i]->GetPointProjection();
            if (pPt != NULL)
            {
                double dDist = MMUtil::CalcDistance(*pPt,
                                                    rPtRef,
                                                    m_dNetworkScale);

                rdAdditionalUTurnScore = std::max(rdAdditionalUTurnScore,
                                                  dDist);
            }
        }
    }

    rdAdditionalUTurnScore *= 2.0;

    return true;
}

/*
3.12 MapMatchingMHT::AddAdjacentSections
adds adjacent sections to route candidates

*/

void MapMatchingMHT::AddAdjacentSections(
                        const MHTRouteCandidate* pCandidate,
                        bool bUpDown,
                        std::vector<MHTRouteCandidate*>& rvecNewRouteCandidates,
                        ISectionFilter* pFilter,
                        bool bRemoveLastPoint)
{
    if (m_pNetwork == NULL || pCandidate == NULL)
        return;

    if (pCandidate->GetCountRouteSegments() <= 1 &&
        pCandidate->GetCountPointsOfLastSection() == 0)
    {
        return;
    }

    const shared_ptr<IMMNetworkSection>& pSection =
                                                   pCandidate->GetLastSection();
    if (pSection == NULL || !pSection->IsDefined())
        return;

    bool bUTurn = false;
    bool bCorrectUTurn = false;
    double dAdditionalUTurnScore = 0.0;

    if ((pSection->GetDirection() == IMMNetworkSection::DIR_UP && !bUpDown) ||
        (pSection->GetDirection() == IMMNetworkSection::DIR_DOWN && bUpDown))
    {
        // U-Turn

        bUTurn = true;

        if (!CheckUTurn(pCandidate, bUpDown,
                        bCorrectUTurn, dAdditionalUTurnScore))
        {
            return;
        }
    }

#if 0 // TODO endless recursion
    if (bCorrectUTurn)
    {
        // make copy of current candidate
        MHTRouteCandidate* pNewCandidate = new MHTRouteCandidate(*pCandidate);
        if (!pNewCandidate->CorrectUTurn())
        {
            delete pNewCandidate;
        }
        else
        {
            rvecNewRouteCandidates.push_back(pNewCandidate);
        }
        // U-Turn - don't add adjacent sections
        return;
    }
#endif

    vector<shared_ptr<IMMNetworkSection> > vecAdjSections;
    pSection->GetAdjacentSections(bUpDown, vecAdjSections);

    const size_t nSections = vecAdjSections.size();
    for (size_t i = 0; i < nSections; i++)
    {
        shared_ptr<IMMNetworkSection>& pAdjSection = vecAdjSections[i];
        if (pAdjSection == NULL)
            continue;

        // Additional filter
        if (pFilter != NULL && !pFilter->IsValid(pAdjSection.get()))
            continue;

        // make copy of current candidate
        MHTRouteCandidate* pNewCandidate = new MHTRouteCandidate(*pCandidate);
        if (bRemoveLastPoint)
        {
            pNewCandidate->RemoveLastPoint();
            if (pNewCandidate->GetCountLastEmptySections() > 5)
            {
                delete pNewCandidate; pNewCandidate = NULL;
                return;
            }
        }

#if 1
        if (bCorrectUTurn)
        {
            // Assign to end node of previous section

            if (!pNewCandidate->CorrectUTurn())
            {
                delete pNewCandidate;
                pNewCandidate = NULL;
                continue;
            }
            else if (
                  (AlmostEqual(pNewCandidate->GetLastSection()->GetStartPoint(),
                               pAdjSection->GetStartPoint()) &&
                   AlmostEqual(pNewCandidate->GetLastSection()->GetEndPoint(),
                               pAdjSection->GetEndPoint())) ||

                  (AlmostEqual(pNewCandidate->GetLastSection()->GetStartPoint(),
                               pAdjSection->GetEndPoint()) &&
                   AlmostEqual(pNewCandidate->GetLastSection()->GetEndPoint(),
                               pAdjSection->GetStartPoint()))

                     /*pNewCandidate->GetLastSection()->GetSectionID() ==
                     pAdjSection->GetSectionID()*/)
            {
                // last section is identical to adjSection
                rvecNewRouteCandidates.push_back(pNewCandidate);
                continue;
            }
        }
        else
#endif
        if (bUTurn)
        {

            // Add Score for U-Turn
            pNewCandidate->SetUTurn(dAdditionalUTurnScore);
        }

        // add adjacent section
        pNewCandidate->AddSection(pAdjSection);

        rvecNewRouteCandidates.push_back(pNewCandidate);
    }
}

/*
3.13 MapMatchingMHT::DetermineBestRouteCandidate
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
        strFileName << "GPoints_" << nCall << "_" << i << ".log";
        std::ostream& rStream = cmsg.file(strFileName.str());
        rvecRouteCandidates[i]->Print(rStream);
        rStream.flush();
    }
    ++nCall;
#endif

    if (rvecRouteCandidates.size() > 0)
        return rvecRouteCandidates.front();
    else
        return NULL;
}

/*
3.14 MapMatchingMHT::CreateCompleteRoute
concatenates routes, create result

*/

void MapMatchingMHT::CreateCompleteRoute(
                     const std::vector<MHTRouteCandidate*>& rvecRouteCandidates)
{
    if (m_pResCreator != NULL)
    {
        m_pResCreator->CreateResult(rvecRouteCandidates);
    }
    else
    {
        assert(false);
    }
}

/*
3.15 MapMatchingMHT::TraceRouteCandidates
Debugging of route candidates

*/

void MapMatchingMHT::TraceRouteCandidates(
                          const std::vector<MHTRouteCandidate*>& rvecCandidates,
                          const char* pszText) const
{
//#define TRACE_ROUTE_CANDIDATES
#ifdef TRACE_ROUTE_CANDIDATES
    std::ostream& rStream = cmsg.file("RouteCandidates.txt");

    rStream << pszText << endl;

    for (size_t i = 0; i < rvecCandidates.size(); ++i)
    {
        MHTRouteCandidate* pCandidate = rvecCandidates[i];
        if (pCandidate != NULL)
            pCandidate->Print(rStream);
        rStream << endl;
    }
    rStream.flush();
#endif
}


} // end of namespace mapmatch

