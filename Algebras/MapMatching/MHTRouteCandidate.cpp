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

[1] Implementation of utilities for map matching

January-April, 2012. Matthias Roth

[TOC]

1 Overview

This implementation file contains the implementation of
the class ~MHTRouteCandidate~.

2 Defines and includes

*/

#include "MHTRouteCandidate.h"
#include "MapMatchingUtil.h"

#include "../Network/NetworkAlgebra.h"
#include "../TemporalNet/TemporalNetAlgebra.h"


namespace mapmatch {


/*
3 class MHTRouteCandidate
  Represents one route candidate for MHT-map matching

*/

MHTRouteCandidate::MHTRouteCandidate()
:m_dScore(0.0),
 m_nCountLastEmptySections(0), m_nCountLastOffRoadPoints(0)
{
}

MHTRouteCandidate::MHTRouteCandidate(const MHTRouteCandidate& rCandidate)
:m_dScore(rCandidate.m_dScore),
 m_nCountLastEmptySections(rCandidate.m_nCountLastEmptySections),
 m_nCountLastOffRoadPoints(rCandidate.m_nCountLastOffRoadPoints)
{
    const size_t nPoints = rCandidate.m_Points.size();
    for (size_t i = 0; i < nPoints; ++i)
    {
        PointData* pData = rCandidate.m_Points[i];
        if (pData != NULL)
        {
            m_Points.push_back(new PointData(*pData));
        }
    }

    const size_t nSections = rCandidate.m_Sections.size();
    for (size_t i = 0; i < nSections; ++i)
    {
        m_Sections.push_back(rCandidate.m_Sections[i]);
    }
}

MHTRouteCandidate::~MHTRouteCandidate()
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

MHTRouteCandidate& MHTRouteCandidate::operator=
                                           (const MHTRouteCandidate& rCandidate)
{
    if (this != &rCandidate)
    {
        m_dScore = rCandidate.m_dScore;
        m_nCountLastEmptySections = rCandidate.m_nCountLastEmptySections;
        m_nCountLastOffRoadPoints = rCandidate.m_nCountLastOffRoadPoints;

        const size_t nPoints = m_Points.size();
        for (size_t i = 0; i < nPoints; ++i)
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

        const size_t nPointsCandidate = rCandidate.m_Points.size();
        for (size_t i = 0; i < nPointsCandidate; ++i)
        {
            PointData* pData = rCandidate.m_Points[i];
            if (pData != NULL)
            {
                m_Points.push_back(new PointData(*pData));
            }
        }

        m_Sections.clear();
        const size_t nSectionsCandidate = rCandidate.m_Sections.size();
        for (size_t i = 0; i < nSectionsCandidate; ++i)
        {
            m_Sections.push_back(rCandidate.m_Sections[i]);
        }
    }

    return *this;
}

bool MHTRouteCandidate::operator==(const MHTRouteCandidate& rCandidate)
{
    if (this == &rCandidate)
        return true;

    if (AlmostEqual(GetScore(), rCandidate.GetScore()) &&
        GetPoints().size() == rCandidate.GetPoints().size() &&
        GetLastSection() == rCandidate.GetLastSection() &&
        GetLastSection().GetDirection() ==
                     rCandidate.GetLastSection().GetDirection())
    {
        const size_t nPoints = GetPoints().size();
        for (size_t i = 0; i < nPoints; ++i)
        {
            PointData* pData1 = GetPoints()[i];
            PointData* pData2 = rCandidate.GetPoints()[i];

            if (pData1 != NULL && pData2 != NULL)
            {
                if (!(*pData1 == *pData2))
                    return false;
            }
            else if (pData1 != pData2)
                return false;
        }

        return true;
    }
    else
    {
        return false;
    }
}

void MHTRouteCandidate::AddSection(const DirectedNetworkSection& rSection)
{
    m_Sections.push_back(rSection);

    // Add score for every Section -> prefer candidates with fewer sections
    m_dScore += 5.0;

    ++m_nCountLastEmptySections;

    // Don't save more than 6 sections
    while (m_Sections.size() > 6)
    {
        m_Sections.pop_front();
    }
}

const DirectedNetworkSection& MHTRouteCandidate::GetLastSection(void) const
{
    if (m_Sections.size() > 0)
        return m_Sections.back().m_Section;
    else
    {
        static DirectedNetworkSection SectionUndef(NULL, NULL);
        return SectionUndef;
    }
}

const MHTRouteCandidate::PointData* MHTRouteCandidate::GetLastPoint(void) const
{
    if (m_Points.size() > 0)
        return m_Points.back();
    else
        return NULL;
}

const std::vector<MHTRouteCandidate::PointData*>& MHTRouteCandidate::
                                              GetPointsOfLastSection(void) const
{
    if (m_Sections.size() == 0)
    {
        assert(false);
        static std::vector<PointData*> vecDummy;
        return vecDummy;
    }

    return m_Sections.back().m_Points;
}

void MHTRouteCandidate::AddPoint(const Point& rPoint,
                                 const Point& rPointProjection,
                                 const DirectedNetworkSection& rSection,
                                 const double dDistance,
                                 const DateTime& rDateTime)
{
    double dScore = dDistance;

    PointData* pData = new PointData(rPoint,
                                     rPointProjection, rSection,
                                     dScore, rDateTime);
    m_Points.push_back(pData);
    assert(m_Sections.size() > 0);
    m_Sections.back().m_Points.push_back(pData);

    m_dScore += dScore;
    m_nCountLastEmptySections = 0;
    m_nCountLastOffRoadPoints = 0;
}

void MHTRouteCandidate::AddPoint(const Point& rPoint,
                                 const double dDistance,
                                 const DateTime& rDateTime)
{
    PointData* pData = new PointData(rPoint, dDistance,
                                     rDateTime);
    m_Points.push_back(pData);
    m_dScore += dDistance;
    //m_nCountLastEmptySections = 0;
    ++m_nCountLastOffRoadPoints;
}

void MHTRouteCandidate::RemoveLastPoint(void)
{
    if (m_Points.size() > 0)
    {
        PointData* pData = m_Points.back();
        m_Points.pop_back();

        bool bOffroadPoint = false;

        if (pData != NULL)
        {
            bOffroadPoint = (pData->GetPointProjection() == NULL);
            m_dScore -= pData->GetScore();
            delete pData;
        }
        else
        {
            assert(false);
        }

        if (!bOffroadPoint)
        {
            // Remove point from section and
            // recalculate EmptySections
            m_nCountLastEmptySections = 0;

            bool bFound = false;

            std::deque<SectionCandidate>::reverse_iterator it =
                                                            m_Sections.rbegin();
            for (/*empty*/; it != m_Sections.rend() && !bFound; ++it)
            {
                if (it->m_Points.size() > 0)
                {
                    it->m_Points.pop_back();
                    bFound = true;
                    if (it->m_Points.size() == 0)
                    {
                        ++m_nCountLastEmptySections;

                        ++it;
                        for (/*empty*/;
                             it != m_Sections.rend() &&
                             it->m_Points.size() == 0;
                             ++it)
                        {
                            ++m_nCountLastEmptySections;
                        }
                    }
                }
                else
                {
                    ++m_nCountLastEmptySections;
                }
            }

            assert(bFound);


            // recalculate Offroad-Points
            m_nCountLastOffRoadPoints = 0;
            std::vector<PointData*>::reverse_iterator itPts = m_Points.rbegin();
            for (/*empty*/;
                 itPts != m_Points.rend() &&
                 (*itPts)->GetPointProjection() == NULL;
                 ++itPts)
            {
                ++m_nCountLastOffRoadPoints;
            }
        }
        else
        {
            assert(m_nCountLastOffRoadPoints > 0);
            --m_nCountLastOffRoadPoints;
        }
    }
}

void MHTRouteCandidate::AddScore(double dScore)
{
    m_dScore += dScore;
}

void MHTRouteCandidate::MarkAsInvalid(void)
{
    m_dScore = std::numeric_limits<double>::max();
}

bool MHTRouteCandidate::IsInvalid(void) const
{
    return AlmostEqual(m_dScore, std::numeric_limits<double>::max());
}


bool MHTRouteCandidate::CorrectUTurn(const DirectedNetworkSection&
                                                                  rNextAdjacent,
                                     Network& rNetwork,
                                     double dNetworkScale)
{
    assert(GetCountPointsOfLastSection() > 0);

    // Assign to end node of section
    Point PointNewProjection(false);
    DirectedNetworkSection NewSection;

    // find previous section, which is not empty and
    // which is the predecessor of rNextAdjacent
    // (stop at first non-empty section)

    std::deque<SectionCandidate>::reverse_iterator it = m_Sections.rbegin();
    if (it != m_Sections.rend())
    {
        ++it;
    }
    else
    {
        assert(false);
        return false;
    }

    bool bFound = false;

    for (/*empty*/; it != m_Sections.rend() && !bFound; ++it)
    {
        const SectionCandidate& rSectionCandidate = *it;

        // empty section (Section with no point assignt) ->
        // Check adjacent sections

        const bool bUpDown = (rSectionCandidate.m_Section.GetDirection()
                                         == DirectedNetworkSection::DIR_UP);

        vector<DirectedSection> adjSectionList;
        rNetwork.GetAdjacentSections(
                rSectionCandidate.m_Section.GetSectionID(), bUpDown,
                adjSectionList);

        for (size_t i = 0; i < adjSectionList.size() && !bFound; ++i)
        {
            Tuple* pSectionTuple = rNetwork.GetSection(
                                         adjSectionList[i].GetSectionTid());
            DirectedNetworkSection adjSection(pSectionTuple,
                                              &rNetwork,
                                              false);

            bFound = (adjSection.GetSectionID() ==
                                              rNextAdjacent.GetSectionID());
        }

        if (bFound)
        {
            NewSection = rSectionCandidate.m_Section;

            PointNewProjection = (NewSection.GetDirection() ==
                                  DirectedNetworkSection::DIR_UP ?
                                          NewSection.GetEndPoint() :
                                          NewSection.GetStartPoint());

        }

        if (rSectionCandidate.m_Points.size() > 0)
        {
            break;
        }
    }

    if (!bFound)
    {
        // ????
        // Assign to start-node of rNextAdjacent
        NewSection = rNextAdjacent;

        PointNewProjection = (NewSection.GetDirection() ==
                              DirectedNetworkSection::DIR_UP ?
                                      NewSection.GetStartPoint() :
                                      NewSection.GetEndPoint());
    }

    if (!PointNewProjection.IsDefined())
    {
        ofstream StreamBadNetwork(
                                 "/home/secondo/Traces/BadNetwork.txt",
                                 ios_base::out | ios_base::ate | ios_base::app);

        StreamBadNetwork << "Undefined start- or endpoint: ";
        StreamBadNetwork << "Section: ";
        StreamBadNetwork << NewSection.GetSectionID();
        StreamBadNetwork << endl;

        return false;
    }


    std::stack<MHTRouteCandidate::PointData> stackPointData;

    while (GetCountPointsOfLastSection() > 0)
    {
        stackPointData.push(MHTRouteCandidate::PointData(
                                               GetLastPoint() != NULL ?
                                               *GetLastPoint():
                                               MHTRouteCandidate::PointData()));
         RemoveLastPoint();
    }


    while (!stackPointData.empty())
    {
        MHTRouteCandidate::PointData& Data = stackPointData.top();

        double dDistance = MMUtil::CalcDistance(PointNewProjection,
                *Data.GetPointGPS(), dNetworkScale);

        AddPoint(*Data.GetPointGPS(), PointNewProjection,
                 NewSection, dDistance, Data.GetTime());

        stackPointData.pop();
    }

    return true;
}

void MHTRouteCandidate::Print(std::ostream& os, int nNetworkId) const
{
    os << "*******RouteCandidate********" << endl;

    os << "Score: " << GetScore() << endl;

    os << "CountLastEmptySections: " << GetCountLastEmptySections() << endl;

    os << "GPoints:" << endl;
    PrintGPoints(os, nNetworkId);

    os << "GPoints as Points:" << endl;
    PrintGPointsAsPoints(os, nNetworkId);

    os << endl << "Sections:" << endl;
    for (size_t j = 0; j < m_Sections.size(); ++j)
    {
        os << m_Sections[j].m_Section.GetSectionID() << endl;
    }
}

void MHTRouteCandidate::PrintGPoints(std::ostream& os,
                                     int nNetworkId) const
{
    const vector<PointData*>& rvecPoints = GetPoints();

    const size_t nPoints = rvecPoints.size();

    AttributePtr<GPoints> pGPts(new GPoints((int)nPoints));

    for (size_t i = 0; i < nPoints; ++i)
    {
        GPoint* pGPoint = rvecPoints[i]->GetGPoint(nNetworkId);

        if (pGPoint != NULL)
        {
            *pGPts += *pGPoint;
        }
    }

    pGPts->Print(os);
}

void MHTRouteCandidate::PrintGPointsAsPoints(std::ostream& os,
                                             int nNetworkId) const
{
    const vector<PointData*>& rvecPoints = GetPoints();

    const size_t nPoints = rvecPoints.size();

    AttributePtr<Points> pPts(new Points((int)nPoints));
    pPts->StartBulkLoad();

    AttributePtr<Points> pPtsOffRoad(new Points(1));
    pPtsOffRoad->StartBulkLoad();

    for (size_t i = 0; i < nPoints; ++i)
    {
        GPoint* pGPoint = rvecPoints[i]->GetGPoint(nNetworkId);

        if (pGPoint != NULL)
        {
            AttributePtr<Point> pPt(pGPoint->ToPoint());
            if (pPt != NULL)
            {
                *pPts += *pPt;
            }
        }
        else if (rvecPoints[i]->GetPointGPS() != NULL)
        {
            Point* pPt = rvecPoints[i]->GetPointGPS();
            *pPtsOffRoad += *pPt;
        }
    }

    pPts->EndBulkLoad(false, false, false);
    pPtsOffRoad->EndBulkLoad(false, false, false);

    pPts->Print(os);
    pPtsOffRoad->Print(os);
}




/*
4 struct MHTRouteCandidate::PointData

*/

MHTRouteCandidate::PointData::PointData()
:m_pGPoint(NULL),
 m_pPointGPS(NULL),
 m_pPointProjection(NULL),
 m_dScore(0.0)
{
}

MHTRouteCandidate::PointData::PointData(const Point& rPointGPS,
                                        const Point& rPointProjection,
                                        const DirectedNetworkSection& rSection,
                                        const double dScore,
                                        const datetime::DateTime& rDateTime)
:m_pGPoint(NULL),
 m_pPointGPS(new Point(rPointGPS)),
 m_pPointProjection(new Point(rPointProjection)),
 m_Section(rSection),
 m_dScore(dScore),
 m_Time(rDateTime)
{
}

MHTRouteCandidate::PointData::PointData(const Point& rPoint,
                                        const double dScore,
                                        const datetime::DateTime& rDateTime)
:m_pGPoint(NULL),
 m_pPointGPS(new Point(rPoint)),
 m_pPointProjection(NULL),
 m_Section(),
 m_dScore(dScore),
 m_Time(rDateTime)
{
}

MHTRouteCandidate::PointData::PointData(const PointData& rPointData)
:m_pGPoint(rPointData.m_pGPoint != NULL ?
           new GPoint(*rPointData.m_pGPoint) : NULL),
 m_pPointGPS(rPointData.m_pPointGPS != NULL ?
             new Point(*rPointData.m_pPointGPS) : NULL),
 m_pPointProjection(rPointData.m_pPointProjection != NULL ?
             new Point(*rPointData.m_pPointProjection) : NULL),
 m_Section(rPointData.m_Section),
 m_dScore(rPointData.m_dScore),
 m_Time(rPointData.m_Time)
{
}

MHTRouteCandidate::PointData& MHTRouteCandidate::PointData::operator=(
                                                    const PointData& rPointData)
{
    if (this != &rPointData)
    {
        if (m_pGPoint != NULL)
            m_pGPoint->DeleteIfAllowed();
        m_pGPoint = NULL;

        if (rPointData.m_pGPoint != NULL)
            m_pGPoint = new GPoint(*rPointData.m_pGPoint);

        if (m_pPointGPS != NULL)
            m_pPointGPS->DeleteIfAllowed();
        m_pPointGPS = NULL;

        if (rPointData.m_pPointGPS != NULL)
            m_pPointGPS = new Point(*rPointData.m_pPointGPS);

        if (m_pPointProjection != NULL)
            m_pPointProjection->DeleteIfAllowed();
        m_pPointProjection = NULL;

        if (rPointData.m_pPointProjection != NULL)
            m_pPointProjection = new Point(*rPointData.m_pPointProjection);

        m_Section = rPointData.m_Section;

        m_dScore = rPointData.m_dScore;

        m_Time = rPointData.m_Time;
    }

    return *this;
}

bool MHTRouteCandidate::PointData::operator==(
                           const MHTRouteCandidate::PointData& rPointData) const
{
    if (this == &rPointData)
        return true;

    if (m_pPointProjection != NULL && rPointData.m_pPointProjection != NULL &&
        m_pPointGPS != NULL && rPointData.m_pPointGPS != NULL)
    {
        return ((*m_pPointProjection) == (*rPointData.m_pPointProjection) &&
                (*m_pPointGPS) == (*rPointData.m_pPointGPS));
    }
    else
    {
        return m_pPointProjection == rPointData.m_pPointProjection;
    }
}

MHTRouteCandidate::PointData::~PointData()
{
    if (m_pGPoint != NULL)
        m_pGPoint->DeleteIfAllowed();
    m_pGPoint = NULL;

    if (m_pPointGPS != NULL)
        m_pPointGPS->DeleteIfAllowed();
    m_pPointGPS = NULL;

    if (m_pPointProjection != NULL)
        m_pPointProjection->DeleteIfAllowed();
    m_pPointProjection = NULL;
}

GPoint* MHTRouteCandidate::PointData::GetGPoint(int nNetworkId) const
{
    if (m_pGPoint != NULL)
        return m_pGPoint;

    if (!m_Section.IsDefined() ||
        m_pPointProjection == NULL || !m_pPointProjection->IsDefined())
    {
        return NULL;
    }

    const NetworkRoute& rRoute = m_Section.GetRoute();
    if (!rRoute.IsDefined())
    {
        return NULL;
    }

    const bool RouteStartsSmaller = rRoute.GetStartsSmaller();
    const SimpleLine* pRouteCurve = rRoute.GetCurve();

    double dPos = 0.0;
    if (pRouteCurve != NULL &&
        MMUtil::GetPosOnSimpleLine(*pRouteCurve, *m_pPointProjection,
                                   RouteStartsSmaller, 0.000001 * 1000, dPos))
       //pRouteCurve->AtPoint(PointProjection, RouteStartsSmaller, dPos))
    {
        m_pGPoint = new GPoint(true, nNetworkId, rRoute.GetRouteID(),
                               dPos, None);
        return m_pGPoint;
    }
    else
    {
        // Projected point could not be matched onto route
        assert(false);
        return NULL;
    }
}


} // end of namespace mapmatch


