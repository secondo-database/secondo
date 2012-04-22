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
#include "MapMatchingMHT.h"
#include "MapMatchingNetworkInterface.h"

#include "../Network/NetworkAlgebra.h"
#include "../TemporalNet/TemporalNetAlgebra.h"


namespace mapmatch {


/*
3 class MHTRouteCandidate
  Represents one route candidate for MHT-map matching

*/

MHTRouteCandidate::MHTRouteCandidate(MapMatchingMHT* pMM)
:m_pMM(pMM), m_dScore(0.0),
 m_nCountLastEmptySections(0), m_nCountLastOffRoadPoints(0),
 m_nCountPoints(0), m_bFailed(false)
{
}

MHTRouteCandidate::MHTRouteCandidate(const MHTRouteCandidate& rCandidate)
:m_pMM(rCandidate.m_pMM), m_dScore(rCandidate.m_dScore),
 m_nCountLastEmptySections(rCandidate.m_nCountLastEmptySections),
 m_nCountLastOffRoadPoints(rCandidate.m_nCountLastOffRoadPoints),
 m_nCountPoints(rCandidate.m_nCountPoints),
 m_bFailed(rCandidate.m_bFailed)
{
    const size_t nSegments = rCandidate.m_Segments.size();
    for (size_t i = 0; i < nSegments; ++i)
    {
        RouteSegment* pSegment = rCandidate.m_Segments[i];
        if (pSegment != NULL)
        {
            m_Segments.push_back(new RouteSegment(*pSegment));
        }
    }
}

MHTRouteCandidate::~MHTRouteCandidate()
{
    m_pMM = NULL;

    const size_t nSegments = m_Segments.size();
    for (size_t i = 0; i < nSegments; ++i)
    {
        RouteSegment* pSegment = m_Segments[i];
        delete pSegment;
    }
    m_Segments.clear();
}

MHTRouteCandidate& MHTRouteCandidate::operator=
                                           (const MHTRouteCandidate& rCandidate)
{
    if (this != &rCandidate)
    {
        m_pMM = rCandidate.m_pMM;
        m_dScore = rCandidate.m_dScore;
        m_nCountLastEmptySections = rCandidate.m_nCountLastEmptySections;
        m_nCountLastOffRoadPoints = rCandidate.m_nCountLastOffRoadPoints;
        m_nCountPoints = rCandidate.m_nCountPoints;

        const size_t nSegments = m_Segments.size();
        for (size_t i = 0; i < nSegments; ++i)
        {
            RouteSegment* pSegment = m_Segments[i];
            delete pSegment;
        }
        m_Segments.clear();

        m_bFailed = rCandidate.m_bFailed;
    }

    return *this;
}

void MHTRouteCandidate::AddSection(const shared_ptr<IMMNetworkSection>& pSect)
{
    m_Segments.push_back(new RouteSegment(pSect));

    // Add score for every Section -> prefer candidates with fewer sections
    m_dScore += 5.0;

    ++m_nCountLastEmptySections;
}

void MHTRouteCandidate::RemoveLastSection(void)
{
    if (m_Segments.size() > 0)
    {
        RouteSegment* pSegment = m_Segments.back();

        if (pSegment != NULL &&
            pSegment->IsOffRoad())
        {
            assert(pSegment->GetPoints().size() == 0);

            m_Segments.pop_back();
            delete pSegment;
            //m_dScore -= 5.0; no score for offroad-segment

            if (m_Segments.size() > 0)
                pSegment = m_Segments.back();
            else
                pSegment = NULL;
        }

        if (pSegment != NULL &&
            pSegment->GetPoints().size() == 0 /*only delete empty segments*/ &&
            !pSegment->IsOffRoad())
        {
            m_Segments.pop_back();
            delete pSegment;
            m_dScore -= 5.0;
            if (m_nCountLastEmptySections > 0)
                --m_nCountLastEmptySections;
            else
                assert(false);
        }
        else
        {
            assert(false);
        }
    }
    else
    {
        assert(false);
    }
}

shared_ptr<IMMNetworkSection> MHTRouteCandidate::GetLastSection(void) const
{
    if (m_Segments.size() > 0)
    {
        RouteSegment* pSegment = m_Segments.back();
        if (pSegment != NULL && !pSegment->IsOffRoad())
            return pSegment->GetSection();
        else
        {
            std::vector<RouteSegment*>::const_reverse_iterator it =
                                                            m_Segments.rbegin();
            std::vector<RouteSegment*>::const_reverse_iterator itEnd =
                                                            m_Segments.rend();

            if (it != itEnd)
                ++it;

            while (it != itEnd)
            {
                RouteSegment* pSegment = *it;
                if (pSegment != NULL && !pSegment->IsOffRoad())
                    return pSegment->GetSection();

                ++it;
            }
        }
    }

    static shared_ptr<IMMNetworkSection> pDummy;
    return pDummy;
}

const MHTRouteCandidate::PointData* MHTRouteCandidate::GetLastPoint(void) const
{
    std::vector<RouteSegment*>::const_reverse_iterator it = m_Segments.rbegin();
    std::vector<RouteSegment*>::const_reverse_iterator itEnd =
                                                              m_Segments.rend();

    while (it != itEnd)
    {
        RouteSegment* pSegment = *it;
        if (pSegment != NULL)
        {
            if (pSegment->GetPoints().size() > 0)
                return pSegment->GetPoints().back();
        }

        ++it;
    }

    return NULL;
}

const std::vector<MHTRouteCandidate::PointData*>& MHTRouteCandidate::
                                              GetPointsOfLastSection(void) const
{
    std::vector<RouteSegment*>::const_reverse_iterator it = m_Segments.rbegin();
    std::vector<RouteSegment*>::const_reverse_iterator itEnd =
                                                              m_Segments.rend();

    while (it != itEnd)
    {
        RouteSegment* pSegment = *it;
        if (pSegment != NULL && !pSegment->IsOffRoad())
            return pSegment->GetPoints();

        ++it;
    }

    assert(false);
    static std::vector<PointData*> vecDummy;
    return vecDummy;
}

size_t MHTRouteCandidate::GetCountPointsOfLastSection(void) const
{
    return GetPointsOfLastSection().size();
}

static double CalcScore(const Point& rPtProjection,
                        const HalfSegment& rHSProjection,
                        const shared_ptr<IMMNetworkSection>& pSection,
                        const MapMatchData& rMMData,
                        double dDistance,
                        double dNetworkScale)
{
    if (dDistance < 0.0)
    {
        dDistance = MMUtil::CalcDistance(rPtProjection,
                                         rMMData.GetPoint(),
                                         dNetworkScale);
    }

    double dScore = dDistance;

    if (rMMData.m_dCourse >= 0.)
    {
        // Course is defined -> add heading difference to score

        IMMNetworkSection* pNetworkSection = pSection.get();

        const double dHeadingSection = MMUtil::CalcHeading(pNetworkSection,
                                                           rHSProjection,
                                                           dNetworkScale);

        //assert(dHeadingSection > 0. || AlmostEqual(dHeadingSection, 0.));

        const double dReverseHeadingSection = (dHeadingSection < 180 ?
                               dHeadingSection + 180. : dHeadingSection - 180.);

        const double dHeadingDiff =
                          std::min(abs(rMMData.m_dCourse - dHeadingSection),
                               360. - abs(rMMData.m_dCourse - dHeadingSection));

        const double dReverseHeadingDiff =
                      std::min(abs(rMMData.m_dCourse - dReverseHeadingSection),
                        360. - abs(rMMData.m_dCourse - dReverseHeadingSection));

        dScore += std::min(dHeadingDiff, dReverseHeadingDiff);

        dScore += dHeadingDiff;
    }

    return dScore;
}

void MHTRouteCandidate::AddPoint(const Point& rPointProjection,
                                 const HalfSegment& rHSProjection,
                                 const MapMatchData* pMMData,
                                 double dDistance)
{
    if (m_Segments.size() == 0 || pMMData == NULL)
    {
        assert(false);
        return;
    }

    RouteSegment* pSegment = m_Segments.back();
    if (pSegment == NULL)
    {
        assert(false);
        return;
    }

    if (pSegment->IsOffRoad())
    {
        std::vector<RouteSegment*>::const_reverse_iterator it =
                                                            m_Segments.rbegin();
        std::vector<RouteSegment*>::const_reverse_iterator itEnd =
                                                            m_Segments.rend();

        if (it != itEnd)
            ++it;

        while (it != itEnd)
        {
            pSegment = *it;
            if (pSegment != NULL && !pSegment->IsOffRoad())
            {
                // add previous section
                AddSection(pSegment->GetSection());
                break;
            }

            ++it;
        }
    }

    pSegment = m_Segments.back();
    if (pSegment == NULL || pSegment->IsOffRoad())
    {
        assert(false);
        return;
    }

    const double dScore = CalcScore(rPointProjection,
                                    rHSProjection,
                                    pSegment->GetSection(),
                                    *pMMData,
                                    dDistance,
                                    m_pMM->GetNetworkScale());

    MHTRouteCandidate::PointData* pData =
                              m_Segments.back()->AddPoint(pMMData,
                                                          rPointProjection,
                                                          dScore);
    if (pData != NULL)
    {
        m_dScore += dScore;
        ++m_nCountPoints;
        m_nCountLastEmptySections = 0;
        m_nCountLastOffRoadPoints = 0;
    }
}

#if 0
    if (0)
    {
        if (m_pMM == NULL)
            return;

        const Network* pNetwork = m_pMM->GetNetwork();
        if (pNetwork == NULL)
        {
            assert(false);
            return;
        }

        if (pDataPrev == NULL)
            return;

        const double dNetworkScale = m_pMM->GetNetworkScale();

        double dLen = -1.0;

        GPoint* pGPointPrev = pDataPrev->GetGPoint(pNetwork->GetId(),
                                                   dNetworkScale);
        if (pGPointPrev != NULL)
        {
            GPoint* pGPoint = pData->GetGPoint(pNetwork->GetId(),
                                               dNetworkScale);

            if (pGPoint == NULL)
            {
                assert(false);
                return;
            }

            AttributePtr<GLine> pGLine(new GLine(true));

            if (pGPoint->GetRouteId() == pGPointPrev->GetRouteId())
            {
                // Same route
                pGLine->AddRouteInterval(pGPointPrev->GetRouteId(),
                                         pGPointPrev->GetPosition(),
                                         pGPoint->GetPosition());

                dLen = MMUtil::CalcLengthCurve(pGLine.get(),
                                               pNetwork,
                                               dNetworkScale);
            }
            else
            {
                // GPoints on different routes

                double dPosEnd = -1.0;

                // Calc net-distance previous GPoint <-> endpoint of section
                if (!GetPosOnRoute(pDataPrev->GetSection().GetRoute(),
                                   pDataPrev->GetSection().GetEndPoint(),
                                   dPosEnd))
                {
                    assert(false);
                }

                pGLine->AddRouteInterval(pGPointPrev->GetRouteId(),
                                         pGPointPrev->GetPosition(),
                                         dPosEnd);

                dLen = MMUtil::CalcLengthCurve(pGLine.get(),
                                               pNetwork,
                                               dNetworkScale);

                pGLine->Clear();

                double dPosStart = -1.0;

                // Calc net-distance this GPoint <-> startpoint of section
                if (!GetPosOnRoute(pData->GetSection().GetRoute(),
                                   pData->GetSection().GetStartPoint(),
                                   dPosStart))
                {
                    assert(false);
                }

                pGLine->AddRouteInterval(pGPoint->GetRouteId(),
                                         dPosStart,
                                         pGPoint->GetPosition());

                dLen += MMUtil::CalcLengthCurve(pGLine.get(),
                                                pNetwork,
                                                dNetworkScale);

                // Get length of sections between section of DataPrev
                // and section of Data

                bool bCurrentSectionFound = false;
                std::deque<SectionCandidate>::reverse_iterator it;
                for (it = m_Sections.rbegin(); it != m_Sections.rend(); ++it)
                {

                    if (!bCurrentSectionFound)
                    {
                        bCurrentSectionFound = it->m_Section.GetSectionID() ==
                                             pData->GetSection().GetSectionID();
                    }
                    else
                    {
                        if (it->m_Section.GetSectionID() ==
                                         pDataPrev->GetSection().GetSectionID())
                        {
                            break;
                        }
                        else
                        {
                            assert(it->m_Points.size() == 0); // empty section
                            dLen += it->m_Section.GetCurveLength(dNetworkScale);
                        }
                    }
                }
            }
        }
        else
        {
            // Previous point is offroad-point
            // calculate distance prev GPS-Point <-> projected point

            Point* pPtProjection = pData->GetPointProjection();
            Point* pPtPrevGPS = pDataPrev->GetPointGPS();
            if (pPtProjection != NULL && pPtPrevGPS != NULL)
            {
                dLen = MMUtil::CalcDistance(*pPtProjection,
                                            *pPtPrevGPS,
                                            dNetworkScale);
            }
        }

        if (dLen >= 0.)
        {
            Point* pPtPrevGPS = pDataPrev->GetPointGPS();
            Point* pPtGPS = pDataPrev->GetPointGPS();

            double dGPSDist = MMUtil::CalcDistance(*pPtPrevGPS,
                                                   *pPtGPS,
                                                   dNetworkScale);

            double dAdditionalScore = abs(dGPSDist - dLen);

            pData->AddScore(dAdditionalScore);
            m_dScore += dAdditionalScore;
        }
    }
#endif

void MHTRouteCandidate::AddPoint(const MapMatchData* pMMData)
{
    if (pMMData == NULL)
        return;

    if (m_Segments.size() == 0 ||
        m_Segments.back() == NULL ||
        !m_Segments.back()->IsOffRoad())
    {
        m_Segments.push_back(new RouteSegment()); // Offroad-Segment
    }

    const double dScore = 40. + (pMMData->m_dCourse >= 0 ? 40 : 0);

    MHTRouteCandidate::PointData* pData = m_Segments.back()->AddPoint(pMMData,
                                                                      dScore);
    if (pData != NULL)
    {
        m_dScore += dScore;
        //m_nCountLastEmptySections = 0;
        ++m_nCountLastOffRoadPoints;
        ++m_nCountPoints;
    }
}

void MHTRouteCandidate::RemoveLastPoint(void)
{
    if (m_Segments.size() > 0)
    {
        std::vector<RouteSegment*>::reverse_iterator it = m_Segments.rbegin();
        std::vector<RouteSegment*>::reverse_iterator itEnd = m_Segments.rend();

        while (it != itEnd)
        {
            RouteSegment* pSegment = *it;
            if (pSegment != NULL && pSegment->GetPoints().size() > 0)
            {
                double dScoreOfLastPoint = pSegment->RemoveLastPoint();
                if (dScoreOfLastPoint > 0.0 ||
                    AlmostEqual(dScoreOfLastPoint, 0.0))
                {
                    m_dScore -= dScoreOfLastPoint;
                    --m_nCountPoints;
                    break;
                }
            }

            ++it;
        }

        // recalculate EmptySections
        m_nCountLastEmptySections = 0;
        it = m_Segments.rbegin();
        while(it != itEnd)
        {
            RouteSegment* pSegment = *it;
            if (pSegment != NULL && !pSegment->IsOffRoad())
            {
                if (pSegment->GetPoints().size() == 0)
                    ++m_nCountLastEmptySections;
                else
                    break;
            }

            ++it;
        }

        // recalculate Offroad-Points
        m_nCountLastOffRoadPoints = 0;
        it = m_Segments.rbegin();
        while (it != itEnd)
        {
            RouteSegment* pSegment = *it;
            if (pSegment != NULL && pSegment->IsOffRoad())
            {
                m_nCountLastOffRoadPoints = pSegment->GetPoints().size();
                break;
            }
            else if (pSegment != NULL && !pSegment->IsOffRoad())
            {
                if (pSegment->GetPoints().size() > 0)
                    break;
            }

            ++it;
        }
    }
#if 0
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
            pData = NULL;
        }
        else
        {
            assert(false);
            return;
        }

        if (!bOffroadPoint && m_Sections.size() > 0)
        {
            // Remove point from section
            bool bFound = false;

            std::deque<SectionCandidate*>::reverse_iterator it
                                                          = m_Sections.rbegin();
            while(it != m_Sections.rend() && !bFound)
            {
                SectionCandidate* pCandidate = *it;
                if (pCandidate != NULL && pCandidate->GetPoints().size() > 0)
                {
                    pCandidate->RemoveLastPoint();
                    bFound = true;
                }

                ++it;
            }
            assert(bFound);

            // recalculate EmptySections
            m_nCountLastEmptySections = 0;
            it = m_Sections.rbegin();
            while(it != m_Sections.rend())
            {
                SectionCandidate* pCandidate = *it;
                if (pCandidate != NULL && pCandidate->GetPoints().size() == 0)
                {
                    ++m_nCountLastEmptySections;
                }
                else
                {
                    break;
                }

                ++it;
            }

            // recalculate Offroad-Points
            m_nCountLastOffRoadPoints = 0;
            std::vector<PointData*>::reverse_iterator itPts = m_Points.rbegin();
            while (itPts != m_Points.rend())
            {
                PointData* pData = *itPts;
                if (pData == NULL || pData->GetPointProjection() == NULL)
                    ++m_nCountLastOffRoadPoints;
                else
                    break;

                ++itPts;
            }
        }
        else
        {
            assert(m_nCountLastOffRoadPoints > 0);
            --m_nCountLastOffRoadPoints;
        }
    }
#endif
}

void MHTRouteCandidate::AddScore(double dScore)
{
    m_dScore += dScore;
}

static HalfSegment CalcHSOfEndPoint(const SimpleLine* pLine,
                                    const Point& rPt)
{
    if (pLine == NULL)
        return HalfSegment(true, Point(0.,0.), Point(0.,0.));

    const int nSize = pLine->Size();
    if (nSize > 0)
    {
        HalfSegment hs;
        pLine->Get(0, hs);

        if (hs.GetLeftPoint() == rPt ||
            hs.GetRightPoint() == rPt)
            return hs;

        if (nSize > 1)
        {
            pLine->Get(nSize - 1, hs);
            return hs;
        }
        else
        {
            assert(false);
            return hs;
        }
    }
    else
    {
        assert(false);
        return HalfSegment(true, Point(0.,0.), Point(0.,0.));
    }
}

bool MHTRouteCandidate::CorrectUTurn(void)
{
    if (GetCountPointsOfLastSection() == 0 ||
        m_pMM == NULL ||
        m_Segments.size() < 2)
    {
        assert(false);
        return false;
    }

    const IMMNetwork* pNetwork = m_pMM->GetNetwork();
    if (pNetwork == NULL)
    {
        assert(false);
        return false;
    }

    // Move all points of last segment to endpoint of second last segment

    std::stack<MHTRouteCandidate::PointData> stackPointData;
    int nPoints = 0;
    while ((nPoints = GetCountPointsOfLastSection()) > 0)
    {
        const PointData* pPointData = GetLastPoint();

        stackPointData.push(MHTRouteCandidate::PointData(
                                    pPointData != NULL ? *pPointData :
                                           MHTRouteCandidate::PointData()));
        RemoveLastPoint();
    }

    RemoveLastSection();

    // assign to endpoint of last section
    const shared_ptr<IMMNetworkSection>& pNewSection = GetLastSection();
    if (pNewSection == NULL)
        return false;

    // TODO so nicht
    Point PointNewProjection = (pNewSection->GetDirection() ==
                                IMMNetworkSection::DIR_UP ?
                                        pNewSection->GetEndPoint() :
                                        pNewSection->GetStartPoint());

    if (!PointNewProjection.IsDefined())
        return false;

    while (!stackPointData.empty())
    {
        MHTRouteCandidate::PointData& rData = stackPointData.top();
        const MapMatchData* pMMData = rData.GetMMData();
        if (pMMData != NULL)
        {
            AddPoint(PointNewProjection,
                     CalcHSOfEndPoint(pNewSection->GetCurve(),
                                      PointNewProjection),
                     pMMData,
                     -1.0);
        }

        stackPointData.pop();
    }

    return true;
}

void MHTRouteCandidate::Print(std::ostream& os) const
{
    os << "*******RouteCandidate********" << endl;

    os << "Score: " << GetScore() << endl;

    os << "CountLastEmptySections: " << GetCountLastEmptySections() << endl;

    os << "Projected Points:" << endl;
    PrintProjectedPoints(os);

    os << endl << "Segments:" << endl;
    for (size_t j = 0; j < m_Segments.size(); ++j)
    {
        if (m_Segments[j]->IsOffRoad())
            os << "Offroad" << endl;
        else
        {
            m_Segments[j]->GetSection()->PrintIdentifier(os);
            os << endl;
        }
    }
}

void MHTRouteCandidate::PrintProjectedPoints(std::ostream& os) const
{
    PointDataIterator it = PointDataBegin();
    PointDataIterator itEnd = PointDataEnd();

    AttributePtr<Points> pPts(new Points((int)1));

    while (it != itEnd)
    {
        const PointData* pData = *it;
        if (pData != NULL)
        {
            const Point* pPoint = pData->GetPointProjection();
            if (pPoint != NULL)
            {
                *pPts += *pPoint;
            }
        }

        ++it;
    }

    pPts->Print(os);
}


/*
4 class MHTRouteCandidate::PointData

*/

MHTRouteCandidate::PointData::PointData()
:m_pData(NULL),
 m_pPointProjection(NULL),
 m_pSection(),
 m_dScore(0.0)
{
}

MHTRouteCandidate::PointData::PointData(
                                  const MapMatchData* pMMData,
                                  const Point& rPointProjection,
                                  const shared_ptr<IMMNetworkSection>& pSection,
                                  const double dScore)
:m_pData(pMMData),
 m_pPointProjection(new Point(rPointProjection)),
 m_pSection(pSection),
 m_dScore(dScore)
{
}

MHTRouteCandidate::PointData::PointData(const MapMatchData* pMMData,
                                        const double dScore)
:m_pData(pMMData),
 m_pPointProjection(NULL),
 m_pSection(),
 m_dScore(dScore)
{
}

MHTRouteCandidate::PointData::PointData(const PointData& rPointData)
:m_pData(rPointData.m_pData),
 m_pPointProjection(rPointData.m_pPointProjection != NULL ?
             new Point(*rPointData.m_pPointProjection) : NULL),
 m_pSection(rPointData.m_pSection),
 m_dScore(rPointData.m_dScore)
{
}

MHTRouteCandidate::PointData& MHTRouteCandidate::PointData::operator=(
                                                    const PointData& rPointData)
{
    if (this != &rPointData)
    {
        m_pData = rPointData.m_pData;

        if (m_pPointProjection != NULL)
            m_pPointProjection->DeleteIfAllowed();
        m_pPointProjection = NULL;

        if (rPointData.m_pPointProjection != NULL)
            m_pPointProjection = new Point(*rPointData.m_pPointProjection);

        m_pSection = rPointData.m_pSection;

        m_dScore = rPointData.m_dScore;
    }

    return *this;
}

bool MHTRouteCandidate::PointData::operator==(
                           const MHTRouteCandidate::PointData& rPointData) const
{
    if (this == &rPointData)
        return true;

    if (m_pPointProjection != NULL && rPointData.m_pPointProjection != NULL &&
        m_pData != NULL && rPointData.m_pData != NULL)
    {
        return ((*m_pPointProjection) == (*rPointData.m_pPointProjection) &&
                (m_pData->GetPoint() == rPointData.m_pData->GetPoint()));
    }
    else
    {
        return m_pPointProjection == rPointData.m_pPointProjection &&
               m_pData == rPointData.m_pData;
    }
}

MHTRouteCandidate::PointData::~PointData()
{
    m_pData = NULL;

    if (m_pPointProjection != NULL)
        m_pPointProjection->DeleteIfAllowed();
    m_pPointProjection = NULL;

    //m_pSection = NULL;
}

#if 0
const GPoint* MHTRouteCandidate::PointData::GetGPoint(const int& nNetworkId,
                                              const double& dNetworkScale) const
{
    if (m_pGPoint != NULL)
        return m_pGPoint;

    if (m_pPointProjection == NULL || !m_pPointProjection->IsDefined() ||
        !m_Section.IsDefined())
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
        MMUtil::GetPosOnSimpleLine(*pRouteCurve,
                                   *m_pPointProjection,
                                   RouteStartsSmaller,
                                   0.000001 * dNetworkScale,
                                   dPos))
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
#endif

void MHTRouteCandidate::PointData::Print(std::ostream& os) const
{
    os << "PointGPS: ";
    if (m_pData != NULL)
    {
        m_pData->GetPoint().Print(os);
    }
    else
    {
        os << "NULL";
    }
    os << endl;

    os << "PointProjection: ";
    if (m_pPointProjection != NULL)
    {
        m_pPointProjection->Print(os);
    }
    else
    {
        os << "NULL";
    }
    os << endl;

    os << "Section: ";
    if (m_pSection != NULL)
    {
        m_pSection->PrintIdentifier(os);
    }
    else
    {
        os << "NULL";
    }
    os << endl;

    os << "Score: " << m_dScore << endl;
}


/*
6 class MHTRouteCandidate::PointDataIterator

*/

MHTRouteCandidate::PointDataIterator::PointDataIterator(
                                                   const PointDataIterator& rIt)
:m_ItRouteSegment(rIt.m_ItRouteSegment),
 m_ItPointData(rIt.m_ItPointData),
 m_ItRouteSegment_R(rIt.m_ItRouteSegment_R),
 m_ItPointData_R(rIt.m_ItPointData_R),
 m_bReverse(rIt.m_bReverse),
 m_pRouteCandidate(rIt.m_pRouteCandidate)
{
}

MHTRouteCandidate::PointDataIterator::PointDataIterator(
                                            const MHTRouteCandidate* pCandidate,
                                            bool bBegin, bool bReverse)
:m_ItRouteSegment(),
 m_ItPointData(),
 m_ItRouteSegment_R(),
 m_ItPointData_R(),
 m_bReverse(bReverse),
 m_pRouteCandidate(pCandidate)
{
    if (pCandidate == NULL)
        return;

    if (bBegin)
    {
        if (bReverse)
        {
            m_ItRouteSegment_R = pCandidate->m_Segments.rbegin();
            if (m_ItRouteSegment_R != pCandidate->m_Segments.rend())
            {
                const RouteSegment* pSegment = *m_ItRouteSegment_R;
                if (pSegment != NULL)
                {
                    m_ItPointData_R = pSegment->GetPoints().rbegin();
                    while (pSegment != NULL &&
                           m_ItPointData_R == pSegment->GetPoints().rend() &&
                           m_ItRouteSegment_R != pCandidate->m_Segments.rend())
                    {
                        this->operator ++();
                        if (m_ItRouteSegment_R != pCandidate->m_Segments.rend())
                            pSegment = *m_ItRouteSegment_R;
                        else
                            pSegment = NULL;
                    }
                }
                else
                {
                    assert(false);
                }
            }
        }
        else
        {
            m_ItRouteSegment = pCandidate->m_Segments.begin();
            if (m_ItRouteSegment != pCandidate->m_Segments.end())
            {
                const RouteSegment* pSegment = *m_ItRouteSegment;
                if (pSegment != NULL)
                {
                    m_ItPointData = pSegment->GetPoints().begin();
                    while (pSegment != NULL &&
                           m_ItPointData == pSegment->GetPoints().end() &&
                           m_ItRouteSegment != pCandidate->m_Segments.end())
                    {
                        this->operator ++();
                        if (m_ItRouteSegment != pCandidate->m_Segments.end())
                            pSegment = *m_ItRouteSegment;
                        else
                            pSegment = NULL;
                    }
                }
                else
                {
                    assert(false);
                }
            }
        }
    }
    else
    {
        // end
        if (bReverse)
        {
            m_ItRouteSegment_R = pCandidate->m_Segments.rend();
        }
        else
        {
            m_ItRouteSegment = pCandidate->m_Segments.end();
        }
    }
}

MHTRouteCandidate::PointDataIterator::~PointDataIterator()
{
    m_pRouteCandidate = NULL;
}

MHTRouteCandidate::PointDataIterator& MHTRouteCandidate::PointDataIterator::
                                         operator=(const PointDataIterator& rIt)
{
    m_bReverse = rIt.m_bReverse;

    m_ItRouteSegment = rIt.m_ItRouteSegment;
    m_ItPointData = rIt.m_ItPointData;

    m_ItRouteSegment_R = rIt.m_ItRouteSegment_R;
    m_ItPointData_R = rIt.m_ItPointData_R;

    return *this;
}

bool MHTRouteCandidate::PointDataIterator::operator==(
                                             const PointDataIterator& rIt) const
{
    if (m_bReverse)
    {
        return (rIt.m_ItPointData_R == m_ItPointData_R &&
                rIt.m_ItRouteSegment_R == m_ItRouteSegment_R);
    }
    else
    {
        return (rIt.m_ItPointData == m_ItPointData &&
                rIt.m_ItRouteSegment == m_ItRouteSegment);
    }
}

bool MHTRouteCandidate::PointDataIterator::operator!=(
                                             const PointDataIterator& rIt) const
{
    if (m_bReverse)
    {
        return (rIt.m_ItPointData_R != m_ItPointData_R ||
                rIt.m_ItRouteSegment_R != m_ItRouteSegment_R);
    }
    else
    {
        return (rIt.m_ItPointData != m_ItPointData ||
                rIt.m_ItRouteSegment != m_ItRouteSegment);
    }
}

const MHTRouteCandidate::PointData* MHTRouteCandidate::PointDataIterator::
                                                               operator*() const
{
    if (m_bReverse)
    {
        return *m_ItPointData_R;
    }
    else
    {
        return *m_ItPointData;
    }
}

MHTRouteCandidate::PointDataIterator& MHTRouteCandidate::PointDataIterator::
                                                                    operator++()
{
    if (m_pRouteCandidate == NULL)
    {
        assert(false);
        return *this;
    }

    if (m_bReverse)
    {
        RouteSegment* pSegment = *m_ItRouteSegment_R;

        if (pSegment != NULL && m_ItPointData_R != pSegment->GetPoints().rend())
        {
            ++m_ItPointData_R;
            if (m_ItPointData_R == pSegment->GetPoints().rend())
            {
                return this->operator ++();
            }
        }
        else
        {
            ++m_ItRouteSegment_R;
            if (m_ItRouteSegment_R != m_pRouteCandidate->m_Segments.rend())
            {
                RouteSegment* pSegment = *m_ItRouteSegment_R;
                if (pSegment != NULL)
                {
                    m_ItPointData_R = pSegment->GetPoints().rbegin();
                    if (m_ItPointData_R == pSegment->GetPoints().rend())
                    {
                        return this->operator ++();
                    }
                }
                else
                    assert(false);
            }
            else
            {
                m_ItPointData_R =
                              std::vector<PointData*>::const_reverse_iterator();
            }
        }
    }
    else
    {
        RouteSegment* pSegment = *m_ItRouteSegment;

        if (pSegment != NULL && m_ItPointData != pSegment->GetPoints().end())
        {
            ++m_ItPointData;
            if (m_ItPointData == pSegment->GetPoints().end())
            {
                return this->operator ++();
            }
        }
        else
        {
            ++m_ItRouteSegment;
            if (m_ItRouteSegment != m_pRouteCandidate->m_Segments.end())
            {
                RouteSegment* pSegment = *m_ItRouteSegment;
                if (pSegment != NULL)
                {
                    m_ItPointData = pSegment->GetPoints().begin();
                    if (m_ItPointData == pSegment->GetPoints().end())
                    {
                        return this->operator ++();
                    }
                }
                else
                    assert(false);
            }
            else
            {
                m_ItPointData = std::vector<PointData*>::const_iterator();
            }
        }
    }

    return *this;
}


/*
6 class MHTRouteCandidate::RouteSegment

*/

MHTRouteCandidate::RouteSegment::RouteSegment(void)
:m_pSection()
{
}

MHTRouteCandidate::RouteSegment::RouteSegment
                                 (const shared_ptr<IMMNetworkSection>& pSection)
:m_pSection(pSection)
{
}

MHTRouteCandidate::RouteSegment::RouteSegment
                                            (const RouteSegment& rCandidate)
:m_pSection(rCandidate.m_pSection)
{
    const std::vector<PointData*>& rvecData = rCandidate.GetPoints();
    const size_t nPoints = rvecData.size();
    for (size_t i = 0; i < nPoints; ++i)
    {
        const PointData* pData = rvecData[i];
        if (pData != NULL)
            m_Points.push_back(new PointData(*pData));
    }
}

MHTRouteCandidate::RouteSegment::~RouteSegment()
{
    const size_t nPoints = m_Points.size();
    for (size_t i = 0; i < nPoints; ++i)
    {
        PointData* pData = m_Points[i];
        delete pData;
    }

    m_Points.clear();
}

MHTRouteCandidate::PointData* MHTRouteCandidate::RouteSegment::AddPoint(
                                                  const MapMatchData* pMMData,
                                                  const Point& rPointProjection,
                                                  const double dScore)
{
    PointData* pData = new PointData(pMMData,
                                     rPointProjection,
                                     GetSection(),
                                     dScore);
    m_Points.push_back(pData);

    return pData;
}

MHTRouteCandidate::PointData* MHTRouteCandidate::RouteSegment::AddPoint(
                                                  const MapMatchData* pMMData,
                                                  const double dScore)
{
    if (!IsOffRoad())
    {
        assert(false);
        return NULL;
    }

    PointData* pData = new PointData(pMMData,
                                     dScore);
    m_Points.push_back(pData);

    return pData;
}

double MHTRouteCandidate::RouteSegment::RemoveLastPoint(void)
{
    if (m_Points.size() > 0)
    {
        PointData* pData = m_Points.back();
        m_Points.pop_back();
        double dScore = 0.0;
        if (pData != NULL)
        {
            dScore = pData->GetScore();
            delete pData;
        }

        return dScore;
    }
    else
    {
        return -1.0;
    }
}

bool MHTRouteCandidate::RouteSegment::IsOffRoad(void) const
{
    return m_pSection == NULL || !m_pSection->IsDefined();
}

void MHTRouteCandidate::RouteSegment::Print(std::ostream& os) const
{
    os << "Section: ";
    if (m_pSection != NULL)
    {
        m_pSection->PrintIdentifier(os);
    }
    else
    {
        os << "NULL";
    }
    os << endl;

    if (m_Points.size() > 0)
    {
        std::vector<PointData*>::const_iterator it = m_Points.begin();
        while (it != m_Points.end())
        {
            os << "PointData : " << endl;
            PointData* pData = *it;
            if (pData != NULL)
            {
                pData->Print(os);
            }
            else
            {
                os << "NULL-Data";
            }

            os << endl;

            ++it;
        }
    }
    else
    {
        os << "no points" << endl;
    }
}

} // end of namespace mapmatch


