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
        RouteSegmentPtr pSegment = rCandidate.m_Segments[i];
        if (pSegment != NULL)
        {
            m_Segments.push_back(pSegment);
            pSegment->IncRef();
        }
    }

    m_SegmentsOutsourced = rCandidate.m_SegmentsOutsourced;
}

MHTRouteCandidate::~MHTRouteCandidate()
{
    m_pMM = NULL;

    const size_t nSegments = m_Segments.size();
    for (size_t i = 0; i < nSegments; ++i)
    {
        RouteSegmentPtr& pSegment = m_Segments[i];
        pSegment->DecRef();
    }
    m_Segments.clear();
    m_SegmentsOutsourced.clear();
}

MHTRouteCandidate& MHTRouteCandidate::operator=
                                           (const MHTRouteCandidate& rCandidate)
{
    if (this != &rCandidate)
    {
        size_t nSegments = m_Segments.size();
        for (size_t i = 0; i < nSegments; ++i)
        {
            RouteSegmentPtr& pSegment = m_Segments[i];
            if (pSegment != NULL)
                pSegment->DecRef();
        }
        m_Segments.clear();

        nSegments = rCandidate.m_Segments.size();
        for (size_t i = 0; i < nSegments; ++i)
        {
            RouteSegmentPtr pSegment = rCandidate.m_Segments[i];
            if (pSegment != NULL)
            {
                m_Segments.push_back(pSegment);
                pSegment->IncRef();
            }
        }

        m_SegmentsOutsourced.clear();
        m_SegmentsOutsourced = rCandidate.m_SegmentsOutsourced;

        m_pMM = rCandidate.m_pMM;
        m_dScore = rCandidate.m_dScore;
        m_nCountLastEmptySections = rCandidate.m_nCountLastEmptySections;
        m_nCountLastOffRoadPoints = rCandidate.m_nCountLastOffRoadPoints;
        m_nCountPoints = rCandidate.m_nCountPoints;
        m_bFailed = rCandidate.m_bFailed;
    }

    return *this;
}

#define SCORE_FOR_SECTION 5.0

void MHTRouteCandidate::AddSection(const shared_ptr<IMMNetworkSection>& pSect)
{
    m_Segments.push_back(RouteSegmentPtr(new RouteSegment(pSect)));

    // Add score for every Section -> prefer candidates with fewer sections
    m_dScore += SCORE_FOR_SECTION;

    ++m_nCountLastEmptySections;

    if (m_Segments.size() >= 20)
    {
        // outsource first segments

        RouteSegmentContPtr pVecSegments(new std::deque<RouteSegmentPtr>());
        m_SegmentsOutsourced.push_back(pVecSegments);

        for (int i = 0; i < 10; ++i)
        {
            RouteSegmentPtr pRouteSegment = m_Segments.front();
            if (pRouteSegment != NULL)
            {
                pRouteSegment->DecRef();
                pVecSegments->push_back(pRouteSegment);
            }
            m_Segments.pop_front();
        }
    }
}

void MHTRouteCandidate::RemoveLastSection(void)
{
    if (m_Segments.size() > 0)
    {
        RouteSegmentPtr pSegment = m_Segments.back();

        if (pSegment != NULL &&
            pSegment->IsOffRoad())
        {
            assert(pSegment->GetPoints().size() == 0);

            pSegment->DecRef();
            m_Segments.pop_back();
            //m_dScore -= SCORE_FOR_SECTION; no score for offroad-segment

            if (m_Segments.size() > 0)
                pSegment = m_Segments.back();
            else
                pSegment = RouteSegmentPtr();
        }

        if (pSegment != NULL &&
            pSegment->GetPoints().size() == 0 /*only delete empty segments*/ &&
            !pSegment->IsOffRoad())
        {
            pSegment->DecRef();
            m_Segments.pop_back();
            m_dScore -= SCORE_FOR_SECTION;
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

MHTRouteCandidate::RouteSegmentPtr MHTRouteCandidate::GetLastOnroadSegment(
                         std::deque<RouteSegmentPtr>::reverse_iterator* pItRet)
{
    if (m_Segments.size() > 0)
    {
        if (pItRet != NULL)
            *pItRet = m_Segments.rend();

        RouteSegmentPtr pSegment = m_Segments.back();
        if (pSegment != NULL && !pSegment->IsOffRoad())
        {
            if (pItRet != NULL)
            {
                *pItRet = m_Segments.rbegin();
            }

            return pSegment;
        }
        else
        {
            std::deque<RouteSegmentPtr>::reverse_iterator it =
                                                            m_Segments.rbegin();
            std::deque<RouteSegmentPtr>::reverse_iterator itEnd =
                                                            m_Segments.rend();

            if (it != itEnd)
                ++it;

            while (it != itEnd)
            {
                RouteSegmentPtr pSegment = *it;
                if (pSegment != NULL && !pSegment->IsOffRoad())
                {
                    if (pItRet != NULL)
                        *pItRet = it;

                    return pSegment;
                }

                ++it;
            }
        }
    }

    return MHTRouteCandidate::RouteSegmentPtr();
}

shared_ptr<IMMNetworkSection> MHTRouteCandidate::GetLastSection(void) const
{
    const RouteSegmentPtr& pSegment = const_cast<MHTRouteCandidate*>(this)->
                                                     GetLastOnroadSegment(NULL);
    if (pSegment != NULL)
        return pSegment->GetSection();
    else
    {
        static shared_ptr<IMMNetworkSection> pDummy;
        return pDummy;
    }
}

const MHTRouteCandidate::PointDataPtr
                                     MHTRouteCandidate::GetLastPoint(void) const
{
    std::deque<RouteSegmentPtr>::const_reverse_iterator it =
                                                            m_Segments.rbegin();
    std::deque<RouteSegmentPtr>::const_reverse_iterator itEnd =
                                                              m_Segments.rend();

    while (it != itEnd)
    {
        const RouteSegmentPtr& pSegment = *it;
        if (pSegment != NULL)
        {
            if (pSegment->GetPoints().size() > 0)
                return pSegment->GetPoints().back();
        }

        ++it;
    }

    return MHTRouteCandidate::PointDataPtr();
}

const std::vector<MHTRouteCandidate::PointDataPtr>& MHTRouteCandidate::
                                              GetPointsOfLastSection(void) const
{
    std::deque<RouteSegmentPtr>::const_reverse_iterator it =
                                                            m_Segments.rbegin();
    std::deque<RouteSegmentPtr>::const_reverse_iterator itEnd =
                                                              m_Segments.rend();
    while (it != itEnd)
    {
        const RouteSegmentPtr& pSegment = *it;
        if (pSegment != NULL && !pSegment->IsOffRoad())
            return pSegment->GetPoints();

        ++it;
    }

    assert(false);
    static std::vector<PointDataPtr> vecDummy;
    return vecDummy;
}

size_t MHTRouteCandidate::GetCountPointsOfLastSection(void) const
{
    return GetPointsOfLastSection().size();
}


#define DISTANCE_FACTOR 2.0

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

    double dScore = DISTANCE_FACTOR * dDistance;

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

    RouteSegmentPtr pSegment = m_Segments.back();
    if (pSegment == NULL)
    {
        assert(false);
        return;
    }

    if (pSegment->IsOffRoad())
    {
        std::deque<RouteSegmentPtr>::const_reverse_iterator it =
                                                            m_Segments.rbegin();
        std::deque<RouteSegmentPtr>::const_reverse_iterator itEnd =
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

        pSegment = m_Segments.back();
        if (pSegment == NULL || pSegment->IsOffRoad())
        {
            assert(false);
            return;
        }

        assert(pSegment->GetRefCount() == 1);
    }
    else
    {
        if (pSegment->GetRefCount() != 1) // not unique
        {
            pSegment->DecRef();
            pSegment = RouteSegmentPtr(new RouteSegment(*pSegment));
            m_Segments.pop_back();
            m_Segments.push_back(pSegment);
        }
    }

    const double dScore = CalcScore(rPointProjection,
                                    rHSProjection,
                                    pSegment->GetSection(),
                                    *pMMData,
                                    dDistance,
                                    m_pMM->GetNetworkScale());

    const PointDataPtr pData = pSegment->AddPoint(pMMData,
                                                  rPointProjection,
                                                  dDistance,
                                                  dScore);
    if (pData != NULL)
    {
        m_dScore += dScore;
        ++m_nCountPoints;
        m_nCountLastEmptySections = 0;
        m_nCountLastOffRoadPoints = 0;
    }
}

void MHTRouteCandidate::AddPoint(const MapMatchData* pMMData)
{
    if (pMMData == NULL)
        return;

    if (m_Segments.size() == 0 ||
        m_Segments.back() == NULL ||
        !m_Segments.back()->IsOffRoad())
    {
        // add Offroad-Segment
        m_Segments.push_back(RouteSegmentPtr(new RouteSegment()));
    }

    const double dScore = DISTANCE_FACTOR * 40. +
                                           (pMMData->m_dCourse >= 0 ? 40. : 0.);

    RouteSegmentPtr pSegment = m_Segments.back();
    if (pSegment->GetRefCount() != 1 /* not unique */)
    {
        pSegment->DecRef();
        pSegment = RouteSegmentPtr(new RouteSegment(*pSegment));
        m_Segments.pop_back();
        m_Segments.push_back(pSegment);
    }

    const PointDataPtr pData = pSegment->AddPoint(pMMData, dScore);
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
        std::deque<RouteSegmentPtr>::reverse_iterator it = m_Segments.rbegin();
        std::deque<RouteSegmentPtr>::reverse_iterator itEnd =m_Segments.rend();

        while (it != itEnd)
        {
            RouteSegmentPtr pSegment = *it;
            if (pSegment != NULL && pSegment->GetPoints().size() > 0)
            {
                if (pSegment->GetRefCount() != 1) // not unique
                {
                    pSegment->DecRef();
                    pSegment = RouteSegmentPtr(new RouteSegment(*pSegment));
                    *it = pSegment;
                }

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
            RouteSegmentPtr pSegment = *it;
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
            RouteSegmentPtr pSegment = *it;
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
}

void MHTRouteCandidate::SetUTurn(double dAdditionalScore)
{
    std::deque<RouteSegmentPtr>::reverse_iterator it;
    RouteSegmentPtr pSegment = GetLastOnroadSegment(&it);
    if (pSegment != NULL)
    {
        if (pSegment->GetRefCount() != 1)
        {
            // not unique
            pSegment->DecRef();
            RouteSegmentPtr pSegmentNew =
                                   RouteSegmentPtr(new RouteSegment(*pSegment));

            if (m_Segments.rend() != it)
            {
                *it = pSegmentNew;
                pSegment = pSegmentNew;
            }
            else
            {
                pSegment->IncRef();
                pSegmentNew->DecRef();
                assert(false);
            }
        }

        pSegment->SetUTurn(true);
    }
    else
    {
        assert(false);
    }

    m_dScore += dAdditionalScore;
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

    std::stack<MHTRouteCandidate::PointDataPtr> stackPointData;
    int nPoints = 0;
    while ((nPoints = GetCountPointsOfLastSection()) > 0)
    {
        stackPointData.push(GetLastPoint());
        RemoveLastPoint();
    }

    RemoveLastSection();

    // assign to endpoint of last section
    const shared_ptr<IMMNetworkSection>& pNewSection = GetLastSection();
    if (pNewSection == NULL)
        return false;

    Point PointNewProjection = (pNewSection->GetDirection() ==
                                IMMNetworkSection::DIR_UP ?
                                        pNewSection->GetEndPoint() :
                                        pNewSection->GetStartPoint());

    if (!PointNewProjection.IsDefined())
        return false;

    while (!stackPointData.empty())
    {
        MHTRouteCandidate::PointDataPtr pData = stackPointData.top();
        const MapMatchData* pMMData = pData != NULL ? pData->GetMMData() : NULL;
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

    RouteSegmentIterator it = RouteSegmentBegin();
    RouteSegmentIterator itEnd = RouteSegmentEnd();

    os << endl << "Segments:" << endl;
    for (/*empty*/; it != itEnd; ++it)
    {
        if ((*it)->IsOffRoad())
            os << "Offroad" << endl;
        else
        {
            (*it)->GetSection()->PrintIdentifier(os);
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
        const PointDataPtr& pData = *it;
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
 m_dDistance(0.0),
 m_dScore(0.0)
{
}

MHTRouteCandidate::PointData::PointData(
                                  const MapMatchData* pMMData,
                                  const Point& rPointProjection,
                                  const shared_ptr<IMMNetworkSection>& pSection,
                                  const double dDistance,
                                  const double dScore)
:m_pData(pMMData),
 m_pPointProjection(new Point(rPointProjection)),
 m_pSection(pSection),
 m_dDistance(dDistance),
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
 m_dDistance(rPointData.m_dDistance),
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

        m_dDistance = rPointData.m_dDistance;

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
            m_ItRouteSegment_R = pCandidate->RouteSegmentRBegin();
            if (m_ItRouteSegment_R != pCandidate->RouteSegmentREnd())
            {
                RouteSegmentPtr pSegment = *m_ItRouteSegment_R;
                if (pSegment != NULL)
                {
                    m_ItPointData_R = pSegment->GetPoints().rbegin();
                    while (pSegment != NULL &&
                           m_ItPointData_R == pSegment->GetPoints().rend() &&
                           m_ItRouteSegment_R != pCandidate->RouteSegmentREnd())
                    {
                        this->operator ++();
                        if (m_ItRouteSegment_R !=
                                                 pCandidate->RouteSegmentREnd())
                            pSegment = *m_ItRouteSegment_R;
                        else
                            pSegment = RouteSegmentPtr();
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
            m_ItRouteSegment = pCandidate->RouteSegmentBegin();
            if (m_ItRouteSegment != pCandidate->RouteSegmentEnd())
            {
                RouteSegmentPtr pSegment = *m_ItRouteSegment;
                if (pSegment != NULL)
                {
                    m_ItPointData = pSegment->GetPoints().begin();
                    while (pSegment != NULL &&
                           m_ItPointData == pSegment->GetPoints().end() &&
                           m_ItRouteSegment != pCandidate->RouteSegmentEnd())
                    {
                        this->operator ++();
                        if (m_ItRouteSegment != pCandidate->RouteSegmentEnd())
                            pSegment = *m_ItRouteSegment;
                        else
                            pSegment = RouteSegmentPtr();
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
            m_ItRouteSegment_R = pCandidate->RouteSegmentREnd();
        }
        else
        {
            m_ItRouteSegment = pCandidate->RouteSegmentEnd();
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

const MHTRouteCandidate::PointDataPtr MHTRouteCandidate::PointDataIterator::
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
        RouteSegmentPtr pSegment = *m_ItRouteSegment_R;

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
            if (m_ItRouteSegment_R != m_pRouteCandidate->RouteSegmentREnd())
            {
                RouteSegmentPtr pSegment = *m_ItRouteSegment_R;
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
                            std::vector<PointDataPtr>::const_reverse_iterator();
            }
        }
    }
    else
    {
        RouteSegmentPtr pSegment = *m_ItRouteSegment;

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
            if (m_ItRouteSegment != m_pRouteCandidate->RouteSegmentEnd())
            {
                RouteSegmentPtr pSegment = *m_ItRouteSegment;
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
                m_ItPointData = std::vector<PointDataPtr>::const_iterator();
            }
        }
    }

    return *this;
}


/*
6 class MHTRouteCandidate::RouteSegment

*/

MHTRouteCandidate::RouteSegment::RouteSegment(void)
:m_pSection(),
 m_bUTurn(false),
 m_nRefCount(1)
{
}

MHTRouteCandidate::RouteSegment::RouteSegment
                                 (const shared_ptr<IMMNetworkSection>& pSection)
:m_pSection(pSection),
 m_bUTurn(false),
 m_nRefCount(1)
{
}

MHTRouteCandidate::RouteSegment::RouteSegment
                                            (const RouteSegment& rCandidate)
:m_pSection(rCandidate.m_pSection),
 m_bUTurn(rCandidate.m_bUTurn),
 m_nRefCount(1)
{
    m_Points = rCandidate.GetPoints();
}

MHTRouteCandidate::RouteSegment::~RouteSegment()
{
    m_Points.clear();
    assert(m_nRefCount == 0);
}

const MHTRouteCandidate::PointDataPtr MHTRouteCandidate::RouteSegment::AddPoint(
                                                  const MapMatchData* pMMData,
                                                  const Point& rPointProjection,
                                                  const double dDistance,
                                                  const double dScore)
{
    PointDataPtr pData = PointDataPtr(new PointData(pMMData,
                                                    rPointProjection,
                                                    GetSection(),
                                                    dDistance,
                                                    dScore));
    m_Points.push_back(pData);

    return pData;
}

const MHTRouteCandidate::PointDataPtr MHTRouteCandidate::RouteSegment::AddPoint(
                                                  const MapMatchData* pMMData,
                                                  const double dScore)
{
    if (!IsOffRoad())
    {
        assert(false);
        return MHTRouteCandidate::PointDataPtr();
    }

    PointDataPtr pData = PointDataPtr(new PointData(pMMData,
                                                    dScore));
    m_Points.push_back(pData);

    return pData;
}

double MHTRouteCandidate::RouteSegment::RemoveLastPoint(void)
{
    if (m_Points.size() > 0)
    {
        const PointDataPtr& pData = m_Points.back();
        double dScore = 0.0;
        if (pData != NULL)
        {
            dScore = pData->GetScore();
        }
        m_Points.pop_back();

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
        std::vector<PointDataPtr>::const_iterator it = m_Points.begin();
        while (it != m_Points.end())
        {
            os << "PointData : " << endl;
            const PointDataPtr& pData = *it;
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


/*
7 class MHTRouteCandidate::RouteSegmentIterator

*/

MHTRouteCandidate::RouteSegmentIterator::RouteSegmentIterator(
                                                const RouteSegmentIterator& rIt)
:m_ItRouteSegment(rIt.m_ItRouteSegment),
 m_ItRouteSegment_R(rIt.m_ItRouteSegment_R),
 m_nIdxContainer(rIt.m_nIdxContainer),
 m_bReverse(rIt.m_bReverse),
 m_pRouteCandidate(rIt.m_pRouteCandidate)
{
}

MHTRouteCandidate::RouteSegmentIterator::RouteSegmentIterator(
                                            const MHTRouteCandidate* pCandidate,
                                            bool bBegin, bool bReverse)
:m_ItRouteSegment(),
 m_ItRouteSegment_R(),
 m_nIdxContainer(-1),
 m_bReverse(bReverse),
 m_pRouteCandidate(pCandidate)
{
    if (pCandidate == NULL)
        return;

    if (bBegin)
    {
        if (bReverse)
        {
            m_nIdxContainer = -1; // -1 => pCandidate->m_Segments
            m_ItRouteSegment_R = pCandidate->m_Segments.rbegin();

            if (m_ItRouteSegment_R == pCandidate->m_Segments.rend())
            {
                m_nIdxContainer = -2; // end
                assert(pCandidate->m_SegmentsOutsourced.size() == 0);
            }
        }
        else
        {
            if (pCandidate->m_SegmentsOutsourced.size() > 0)
            {
                m_nIdxContainer = 0;
                while (m_nIdxContainer <
                                 (int)pCandidate->m_SegmentsOutsourced.size() &&
                   (pCandidate->m_SegmentsOutsourced[m_nIdxContainer] == NULL ||
                    pCandidate->m_SegmentsOutsourced[m_nIdxContainer]->size()
                                                                          == 0))
                {
                    ++m_nIdxContainer;
                }

                if (m_nIdxContainer <
                                   (int)pCandidate->m_SegmentsOutsourced.size())
                {
                    m_ItRouteSegment =
                     pCandidate->m_SegmentsOutsourced[m_nIdxContainer]->begin();
                }
                else
                {
                    m_nIdxContainer = -1; // -1 => pCandidate->m_Segments
                    m_ItRouteSegment = pCandidate->m_Segments.begin();

                    if (m_ItRouteSegment == pCandidate->m_Segments.end())
                    {
                        m_nIdxContainer = -2; // end
                    }
                }
            }
            else
            {
                m_nIdxContainer = -1; // -1 => pCandidate->m_Segments
                m_ItRouteSegment = pCandidate->m_Segments.begin();

                if (m_ItRouteSegment == pCandidate->m_Segments.end())
                {
                    m_nIdxContainer = -2; // end
                }
            }
        }
    }
    else
    {
        // end
        m_nIdxContainer = -2; // -2 => end
    }
}

MHTRouteCandidate::RouteSegmentIterator::~RouteSegmentIterator()
{
    m_pRouteCandidate = NULL;
}

MHTRouteCandidate::RouteSegmentIterator&
                  MHTRouteCandidate::RouteSegmentIterator::
                                      operator=(const RouteSegmentIterator& rIt)
{
    m_ItRouteSegment = rIt.m_ItRouteSegment;
    m_ItRouteSegment_R = rIt.m_ItRouteSegment_R;
    m_nIdxContainer = rIt.m_nIdxContainer;
    m_bReverse = rIt.m_bReverse;
    m_pRouteCandidate = rIt.m_pRouteCandidate;

    return *this;
}

bool MHTRouteCandidate::RouteSegmentIterator::operator==(
                                          const RouteSegmentIterator& rIt) const
{
    if (m_nIdxContainer == -2 || rIt.m_nIdxContainer == -2) // end
        return m_nIdxContainer == rIt.m_nIdxContainer;
    else
    {
        if (m_bReverse)
        {
            return (m_nIdxContainer == rIt.m_nIdxContainer &&
                    rIt.m_ItRouteSegment_R == m_ItRouteSegment_R);
        }
        else
        {
            return (m_nIdxContainer == rIt.m_nIdxContainer &&
                    rIt.m_ItRouteSegment == m_ItRouteSegment);
        }
    }
}

bool MHTRouteCandidate::RouteSegmentIterator::operator!=(
                                          const RouteSegmentIterator& rIt) const
{
   return !(this->operator ==(rIt));
}

const MHTRouteCandidate::RouteSegmentPtr
                      MHTRouteCandidate::RouteSegmentIterator::operator*() const
{
    if (m_bReverse)
    {
        return *m_ItRouteSegment_R;
    }
    else
    {
        return *m_ItRouteSegment;
    }
}

MHTRouteCandidate::RouteSegmentIterator&
                           MHTRouteCandidate::RouteSegmentIterator::operator++()
{
    if (m_pRouteCandidate == NULL || m_nIdxContainer == -2 /*end*/)
    {
        assert(false);
        return *this;
    }

    if (m_bReverse)
    {
        const std::deque<RouteSegmentPtr>& rCont = (m_nIdxContainer == -1) ?
                    m_pRouteCandidate->m_Segments :
                    *(m_pRouteCandidate->m_SegmentsOutsourced[m_nIdxContainer]);

        if (m_ItRouteSegment_R != rCont.rend())
        {
            ++m_ItRouteSegment_R;
            if (m_ItRouteSegment_R == rCont.rend())
            {
                return this->operator ++();
            }
        }
        else
        {
            if (m_nIdxContainer == -1)
            {
                // process last outsourced container
                m_nIdxContainer =
                       (int) m_pRouteCandidate->m_SegmentsOutsourced.size() - 1;
            }
            else
            {
                --m_nIdxContainer;
            }

            if (m_nIdxContainer < 0 ||
                m_nIdxContainer >=
                           (int) m_pRouteCandidate->m_SegmentsOutsourced.size())
            {
                m_nIdxContainer = -2; // end
            }
            else
            {
                const std::deque<RouteSegmentPtr>& rContNew =
                    *(m_pRouteCandidate->m_SegmentsOutsourced[m_nIdxContainer]);
                m_ItRouteSegment_R = rContNew.rbegin();
                if (m_ItRouteSegment_R == rContNew.rend())
                {
                    return this->operator ++();
                }
            }
        }
    }
    else
    {
        const std::deque<RouteSegmentPtr>& rCont = (m_nIdxContainer == -1) ?
                    m_pRouteCandidate->m_Segments :
                    *(m_pRouteCandidate->m_SegmentsOutsourced[m_nIdxContainer]);

        if (m_ItRouteSegment != rCont.end())
        {
            ++m_ItRouteSegment;
            if (m_ItRouteSegment == rCont.end())
            {
                return this->operator ++();
            }
        }
        else
        {
            if (m_nIdxContainer == -1)
            {
                m_nIdxContainer = -2; // end
            }
            else
            {
                ++m_nIdxContainer;
                if (m_nIdxContainer >=
                           (int) m_pRouteCandidate->m_SegmentsOutsourced.size())
                {
                    m_nIdxContainer = -1; // m_pRouteCandidate->m_Segments
                    m_ItRouteSegment = m_pRouteCandidate->m_Segments.begin();
                    if (m_ItRouteSegment == m_pRouteCandidate->m_Segments.end())
                    {
                        return this->operator ++();
                    }
                }
                else
                {
                    const std::deque<RouteSegmentPtr>& rContNew =
                    *(m_pRouteCandidate->m_SegmentsOutsourced[m_nIdxContainer]);
                    m_ItRouteSegment = rContNew.begin();
                    if (m_ItRouteSegment == rContNew.end())
                    {
                        return this->operator ++();
                    }
                }
            }
        }
    }

    return *this;
}


} // end of namespace mapmatch


