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

This implementation file contains the implementation of the class ~MapMatchingBase~.
Is is the base class of all Map Matching algorithms

2 Defines and includes

*/
#include "MapMatchingBase.h"
#include "MapMatchingUtil.h"

#include <stdio.h>

#include "../Network/NetworkAlgebra.h"
#include "../TemporalNet/TemporalNetAlgebra.h"


namespace mapmatch {

// Constructor
MapMatchingBase::MapMatchingBase(Network* pNetwork, MPoint* pMPoint)
:m_pNetwork(pNetwork), m_pMPoint(pMPoint), m_pResMGPoint(NULL), m_pRITree(NULL)
{
}

// Destructor
MapMatchingBase::~MapMatchingBase()
{
    m_pNetwork = NULL;
    m_pMPoint = NULL;
    m_pResMGPoint = NULL;
    if (m_pRITree != NULL)
    {
        m_pRITree->Destroy();
        delete m_pRITree;
        m_pRITree = NULL;
    }
}

bool MapMatchingBase::InitMapMatching(MGPoint* pResMGPoint)
{
    if (pResMGPoint == NULL)
        return false;
    else
    {
        m_pResMGPoint = pResMGPoint;
        m_pResMGPoint->Clear();

        if (m_pNetwork == NULL || !m_pNetwork->IsDefined() ||
            m_pMPoint == NULL || !m_pMPoint->IsDefined() ||
            m_pMPoint->IsEmpty() || m_pMPoint->GetNoComponents() == 0)
        {
            pResMGPoint->SetDefined(false);
            return false;
        }
        else
        {
            pResMGPoint->SetDefined(true);
            pResMGPoint->StartBulkLoad();

            if (m_pRITree != NULL)
                delete m_pRITree;

            m_pRITree = new RITreeP(0);

            return true;
        }
    }
}

void MapMatchingBase::FinalizeMapMatching(void)
{
    if (m_pResMGPoint != NULL && m_pRITree != NULL)
    {
        m_pResMGPoint->EndBulkLoad(true);
        m_pResMGPoint->SetDefined(!m_pRITree->IsEmpty());
        if (!m_pRITree->IsEmpty())
        {
            m_pRITree->TreeToDbArray(&m_pResMGPoint->m_trajectory, 0);

            m_pResMGPoint->SetTrajectoryDefined(true);
            m_pResMGPoint->m_trajectory.TrimToSize();
            m_pResMGPoint->SetBoundingBoxDefined(false);
        }
    }

    if (m_pRITree != NULL)
    {
        m_pRITree->Destroy();
        delete m_pRITree;
        m_pRITree = NULL;
    }
}

void MapMatchingBase::AddUGPoint(const UGPoint& rAktUGPoint)
{
    if (m_pResMGPoint != NULL && m_pRITree != NULL)
    {
        m_pResMGPoint->Add(rAktUGPoint);
        m_pRITree->InsertUnit(rAktUGPoint.GetStartPoint().GetRouteId(),
                              rAktUGPoint.GetStartPoint().GetPosition(),
                              rAktUGPoint.GetEndPoint().GetPosition());
    }
}

void MapMatchingBase::AddUnit(const int nRouteID,
        const double dPos1, const double dPos2)
{
    if (m_pRITree != NULL)
    {
        m_pRITree->InsertUnit(nRouteID, dPos1, dPos2);
    }
}

bool MapMatchingBase::CalcShortestPath(const GPoint* pGPStart,
                                       const GPoint* pGPEnd,
                                       const datetime::DateTime& rtimeStart,
                                       const datetime::DateTime& rtimeEnd)
{
    if (pGPStart == NULL || !pGPStart->IsDefined() ||
        pGPEnd == NULL || !pGPEnd->IsDefined() ||
        !rtimeStart.IsDefined() || !rtimeEnd.IsDefined() ||
        m_pNetwork == NULL || !m_pNetwork->IsDefined())
    {
        return false;
    }

    const int nNetworkId = m_pNetwork->GetId();

    AttributePtr<GLine> pGlShortestPath(new GLine(false));
    if (!pGPStart->ShortestPathAStar(pGPEnd, pGlShortestPath.get(), m_pNetwork))
    {
        // ShortestPath calculation failed
        return false;
    }
    else
    {
        // ShortestPath calculation successfull
        // Create UGPoints

        RouteInterval actRouteInterval;
        Side side = None;

        DateTime timeCurrentStart(rtimeStart);
        DateTime timeCurrentEnd(rtimeStart);

        for (int i = 0; i < pGlShortestPath->NoOfComponents(); ++i)
        {
            pGlShortestPath->Get(i, actRouteInterval);

            Instant timeCurrentEnd = (rtimeEnd - rtimeStart) *
                    (fabs(actRouteInterval.GetEndPos() -
                                           actRouteInterval.GetStartPos())
                            / pGlShortestPath->GetLength()) + timeCurrentStart;

            if (actRouteInterval.GetRouteId() == pGPEnd->GetRouteId() &&
               AlmostEqual(actRouteInterval.GetEndPos(), pGPEnd->GetPosition()))
            {
                timeCurrentEnd = rtimeEnd; // End reached
            }

            if (actRouteInterval.GetStartPos() > actRouteInterval.GetEndPos())
            {
                side = Down; // Moving down
            }
            else if (actRouteInterval.GetStartPos() <
                                                   actRouteInterval.GetEndPos())
            {
                side = Up; //  Moving Up
            }
            else
            {
                side = None;
            }

            AttributePtr<UGPoint> pUGPoint(new UGPoint(
                    Interval<Instant>(timeCurrentStart, timeCurrentEnd,
                                      true, false),
                    nNetworkId, actRouteInterval.GetRouteId(),
                    side, actRouteInterval.GetStartPos(),
                    actRouteInterval.GetEndPos()));

            this->AddUGPoint(*pUGPoint);

            timeCurrentStart = timeCurrentEnd;
        }
    }
}


} // end of namespace mapmatch



