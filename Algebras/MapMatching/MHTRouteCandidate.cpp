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
:m_dScore(0.0), m_nPointsOfLastSection(0), m_nCountLastEmptySections(0),
 m_nCountLastOffRoadPoints(0)
{
}

MHTRouteCandidate::MHTRouteCandidate(const MHTRouteCandidate& rCandidate)
:m_dScore(rCandidate.m_dScore),
 m_nPointsOfLastSection(rCandidate.m_nPointsOfLastSection),
 m_nCountLastEmptySections(rCandidate.m_nCountLastEmptySections),
 m_nCountLastOffRoadPoints(rCandidate.m_nCountLastOffRoadPoints)
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
        m_nPointsOfLastSection = rCandidate.m_nPointsOfLastSection;
        m_nCountLastEmptySections = rCandidate.m_nCountLastEmptySections;
        m_nCountLastOffRoadPoints = rCandidate.m_nCountLastOffRoadPoints;

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

bool MHTRouteCandidate::operator==(const MHTRouteCandidate& rCandidate)
{
    if (this == &rCandidate)
        return true;

    if (AlmostEqual(GetScore(), rCandidate.GetScore()) &&
        GetPoints().size() == rCandidate.GetPoints().size() /*&&
        GetSectionCount() == rCandidate.GetSectionCount()*/)
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

        /*const size_t nSections = GetSectionCount();
        for (size_t i = 0; i < nSections; ++i)
        {
            if (!(GetSection(i) == rCandidate.GetSection(i)))
                return false;
        }*/

        return true;
    }
    else
    {
        return false;
    }
}

void MHTRouteCandidate::AddSection(const DirectedNetworkSection& rSection,
                                   bool bCheckLastPoint)
{
    vector<PointData*> vecPoints;
    if (bCheckLastPoint)
    {
        GetPointsOfLastSection(vecPoints);
    }

    m_Sections.push_back(rSection);
    m_nPointsOfLastSection = 0;
    ++m_nCountLastEmptySections;

    if (vecPoints.size() > 0)
    {
        PointData* pData = vecPoints.back();
        if (pData != NULL)
        {
            // TODO
        }
    }
}

size_t MHTRouteCandidate::GetSectionCount(void) const
{
    return m_Sections.size();
}

const DirectedNetworkSection& MHTRouteCandidate::GetSection(
                                                          size_t nSection) const
{
    if (nSection < m_Sections.size())
        return m_Sections.at(nSection);
    else
    {
        static DirectedNetworkSection SectionUndef(NULL, NULL);
        return SectionUndef;
    }
}

const DirectedNetworkSection& MHTRouteCandidate::GetLastSection(void) const
{
    if (m_Sections.size() > 0)
        return m_Sections.at(m_Sections.size() - 1);
    else
    {
        static DirectedNetworkSection SectionUndef(NULL, NULL);
        return SectionUndef;
    }
}

void MHTRouteCandidate::GetPointsOfLastSection(
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

void MHTRouteCandidate::AddPoint(const GPoint& rGPoint, const Point& rPoint,
                                 const double dDistance,
                                 const DateTime& rDateTime, bool bClosed)
{
    double dScore = dDistance;

    /* Add Netdistance between previous GPoint and current GPoint

    if (m_nCountLastEmptySections > 0 && m_Points.size() > 0)
    {
        // First point of current section -> Add Netdistance

        PointData* pDataPrev = m_Points.back();
        if (pDataPrev != NULL)
        {
            if (pDataPrev->m_pGPoint != NULL &&
                pDataPrev->m_pGPoint->IsDefined())
            {
                // TODO Geoid
                dScore += rGPoint.Netdistance(pDataPrev->m_pGPoint);
            }
        }
    }*/

    PointData* pData = new PointData(rGPoint, rPoint, dScore,
                                     rDateTime, bClosed);
    m_Points.push_back(pData);
    ++m_nPointsOfLastSection;
    m_dScore += dScore;
    m_nCountLastEmptySections = 0;
    m_nCountLastOffRoadPoints = 0;
}

void MHTRouteCandidate::AddPoint(const Point& rPoint,
                                 const double dDistance,
                                 const DateTime& rDateTime, bool bClosed)
{
    PointData* pData = new PointData(rPoint, dDistance,
                                     rDateTime, bClosed);
    m_Points.push_back(pData);
    ++m_nPointsOfLastSection;
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
        if (pData != NULL)
        {
            m_dScore -= pData->m_dScore;
            delete pData;
            --m_nPointsOfLastSection;
            // TODO m_nCountLastEmptySections
        }
    }
}

void MHTRouteCandidate::MarkAsInvalid(void)
{
    m_dScore = std::numeric_limits<double>::max();
}

void MHTRouteCandidate::Print(std::ostream& os) const
{
    os << "*******RouteCandidate********" << endl;

    os << "Score: " << GetScore() << endl;

    os << "Points:" << endl;
    PrintGPointsAsPoints(os);

    os << endl << "Sections:" << endl;
    for (size_t j = 0; j < GetSectionCount(); ++j)
    {
        os << GetSection(j).GetSectionID() << endl;
    }
}

void MHTRouteCandidate::PrintGPointsAsPoints(std::ostream& os) const
{
    const vector<PointData*>& rvecPoints = GetPoints();

    const size_t nPoints = rvecPoints.size();

    AttributePtr<Points> pPts(new Points(1));
    pPts->StartBulkLoad();

    AttributePtr<Points> pPtsOffRoad(new Points(1));
    pPtsOffRoad->StartBulkLoad();

    for (size_t i = 0; i < nPoints; ++i)
    {
        GPoint* pGPoint = rvecPoints[i]->m_pGPoint;

        if (pGPoint != NULL)
        {
            AttributePtr<Point> pPt(pGPoint->ToPoint());
            if (pPt != NULL)
            {
                *pPts += *pPt;
            }
        }
        else if (rvecPoints[i]->m_pPointGPS != NULL)
        {
            Point* pPt = rvecPoints[i]->m_pPointGPS;
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

MHTRouteCandidate::PointData::PointData(const GPoint& rGPoint,
                                        const Point& rPoint,
                                        const double dScore,
                                        const datetime::DateTime& rDateTime,
                                        bool bClosed)
:m_pGPoint(new GPoint(rGPoint)), m_pPointGPS(new Point(rPoint)),
 m_dScore(dScore), m_Time(rDateTime), m_bClosed(bClosed)
{
}

MHTRouteCandidate::PointData::PointData(const Point& rPoint,
                                        const double dScore,
                                        const datetime::DateTime& rDateTime,
                                        bool bClosed)
:m_pGPoint(NULL), m_pPointGPS(new Point(rPoint)),
 m_dScore(dScore), m_Time(rDateTime), m_bClosed(bClosed)
{
}

MHTRouteCandidate::PointData::PointData(const PointData& rPointData)
:m_pGPoint(rPointData.m_pGPoint != NULL ?
           new GPoint(*rPointData.m_pGPoint) : NULL),
 m_pPointGPS(rPointData.m_pPointGPS != NULL ?
             new Point(*rPointData.m_pPointGPS) : NULL),
 m_dScore(rPointData.m_dScore),
 m_Time(rPointData.m_Time), m_bClosed(rPointData.m_bClosed)
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

        m_dScore = rPointData.m_dScore;

        m_Time = rPointData.m_Time;
        m_bClosed = rPointData.m_bClosed;
    }

    return *this;
}

bool MHTRouteCandidate::PointData::operator==(
                                 const MHTRouteCandidate::PointData& rPointData)
{
    if (this == &rPointData)
        return true;

    if (m_pGPoint != NULL && rPointData.m_pGPoint != NULL &&
        m_pPointGPS != NULL && rPointData.m_pPointGPS != NULL)
    {
        return (m_pGPoint->Compare(*rPointData.m_pGPoint) == 0 &&
                (*m_pPointGPS) == (*rPointData.m_pPointGPS));
    }
    else
    {
        return m_pGPoint == rPointData.m_pGPoint; // Both NULL ?
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
}


} // end of namespace mapmatch


