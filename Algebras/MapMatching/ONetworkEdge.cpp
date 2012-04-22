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

[1] Implementation of the ONetworkEdge class

April, 2012. Matthias Roth

[TOC]

1 Overview

This implementation file contains the implementation of the class ~ONetworkEdge~.


2 Defines and includes

*/

#include "ONetworkEdge.h"
#include "RelationAlgebra.h"
#include "SpatialAlgebra.h"
#include "FTextAlgebra.h"
#include "MapMatchingUtil.h"
using namespace mapmatch;


/*
3 class ONetworkEdge
  Edge of Network based on ordered relation

*/

ONetworkEdge::ONetworkEdge()
:m_pTupleEdge(NULL),
 m_pONetwork(NULL),
 m_dCurveLength(-1.0),
 m_dMaxSpeed(-1.0)
{
}

ONetworkEdge::ONetworkEdge(Tuple* pTupleEdge,
                           const ONetwork* pONetwork,
                           bool bIncReference)
:m_pTupleEdge(pTupleEdge),
 m_pONetwork(pONetwork),
 m_dCurveLength(-1.0),
 m_dMaxSpeed(-1.0)
{
    if (bIncReference && m_pTupleEdge != NULL)
        m_pTupleEdge->IncReference();
}

ONetworkEdge::ONetworkEdge(const ONetworkEdge& rEdge)
:m_pTupleEdge(rEdge.m_pTupleEdge),
 m_pONetwork(rEdge.m_pONetwork),
 m_dCurveLength(rEdge.m_dCurveLength),
 m_dMaxSpeed(rEdge.m_dMaxSpeed)
{
    if (m_pTupleEdge != NULL)
        m_pTupleEdge->IncReference();
}

ONetworkEdge::~ONetworkEdge()
{
    if (m_pTupleEdge != NULL)
        m_pTupleEdge->DeleteIfAllowed();
    m_pTupleEdge = NULL;

    m_pONetwork = NULL;
}

ONetworkEdge& ONetworkEdge::operator=(const ONetworkEdge& rEdge)
{
    if (&rEdge != this)
    {
        if (m_pTupleEdge != NULL)
        {
            m_pTupleEdge->DeleteIfAllowed();
            m_pTupleEdge = NULL;
        }

        m_pTupleEdge = rEdge.m_pTupleEdge;
        if (m_pTupleEdge != NULL)
            m_pTupleEdge->IncReference();

        m_pONetwork = rEdge.m_pONetwork;
        m_dCurveLength = rEdge.m_dCurveLength;
        m_dMaxSpeed = rEdge.m_dMaxSpeed;
    }

    return *this;
}

bool ONetworkEdge::operator==(const ONetworkEdge& rEdge) const
{
    if (m_pTupleEdge != NULL && rEdge.m_pTupleEdge != NULL)
    {
        return (m_pTupleEdge->GetTupleId() ==
                rEdge.m_pTupleEdge->GetTupleId() &&
                m_pONetwork == rEdge.m_pONetwork);
    }
    else
    {
        return m_pTupleEdge == rEdge.m_pTupleEdge;
    }
}

/*int ONetworkEdge::GetSourceId(void) const
{
    if (m_pTupleEdge != NULL)
    {
        return static_cast<CcInt*>(m_pTupleEdge->GetAttribute(0))->GetIntval();
    }
    else
    {
        return 0;
    }
}

int ONetworkEdge::GetTargetId(void) const
{
    if (m_pTupleEdge != NULL)
    {
        return static_cast<CcInt*>(m_pTupleEdge->GetAttribute(1))->GetIntval();
    }
    else
    {
        return 0;
    }
}*/

CcInt* ONetworkEdge::GetSource(void) const
{
    if (m_pTupleEdge != NULL)
    {
        return static_cast<CcInt*>(m_pTupleEdge->GetAttribute(0));
    }
    else
    {
        return NULL;
    }
}

CcInt* ONetworkEdge::GetTarget(void) const
{
    if (m_pTupleEdge != NULL)
    {
        return static_cast<CcInt*>(m_pTupleEdge->GetAttribute(1));
    }
    else
    {
        return NULL;
    }
}

Point ONetworkEdge::GetSourcePoint(void) const
{
    if (m_pTupleEdge != NULL)
    {
        Point* pPtSource = static_cast<Point*>(m_pTupleEdge->GetAttribute(2));
        return pPtSource != NULL ? *pPtSource : Point(false);
    }
    else
    {
        return Point(false);
    }
}

Point ONetworkEdge::GetTargetPoint(void) const
{
    if (m_pTupleEdge != NULL)
    {
        Point* pPtTarget = static_cast<Point*>(m_pTupleEdge->GetAttribute(3));
        return pPtTarget != NULL ? *pPtTarget : Point(false);
    }
    else
    {
        return Point(false);
    }
}

SimpleLine* ONetworkEdge::GetCurve(void) const
{
    if (m_pTupleEdge != NULL)
    {
        return static_cast<SimpleLine*>(m_pTupleEdge->GetAttribute(6));
    }
    else
    {
        return NULL;
    }
}

std::string ONetworkEdge::GetRoadName(void) const
{
    if (m_pTupleEdge != NULL)
    {
        FText* pRoadName = static_cast<FText*>(m_pTupleEdge->GetAttribute(7));
        if (pRoadName != NULL && pRoadName->IsDefined())
            return pRoadName->GetValue();
        else
            return "";
    }
    else
    {
        return "";
    }
}

std::string ONetworkEdge::GetRoadType(void) const
{
    if (m_pTupleEdge != NULL)
    {
        FText* pRoadType = static_cast<FText*>(m_pTupleEdge->GetAttribute(8));
        if (pRoadType != NULL && pRoadType->IsDefined())
            return pRoadType->GetValue();
        else
            return "";
    }
    else
    {
        return "";
    }
}

static double convStrToDouble (const char* pszStr)
{
    if (pszStr == NULL)
        return 0.0;

    return atof(pszStr);
}

double ONetworkEdge::GetMaxSpeed(void) const
{
    /*
     * http://wiki.openstreetmap.org/wiki/Key:maxspeed
     *
     *  maxspeed=60       -> km/h
     *  maxspeed=50 mph   -> mph
     *  maxspeed=10 knots -> knots
     *
     */

    if (m_pTupleEdge != NULL && m_dMaxSpeed < 0.0)
    {
        FText* pMaxSpeed = static_cast<FText*>(m_pTupleEdge->GetAttribute(9));
        if (pMaxSpeed != NULL && pMaxSpeed->IsDefined())
        {
            m_dMaxSpeed = convStrToDouble(pMaxSpeed->GetValue().c_str());

            // TODO Umrechnung mph, knots
        }
        else
            m_dMaxSpeed = 0.0;
    }

    return m_dMaxSpeed;
}

double ONetworkEdge::GetCurveLength(const double dScale) const
{
    if (m_dCurveLength < 0.)
    {
        m_dCurveLength = MMUtil::CalcLengthCurve(GetCurve(), dScale);
    }

    return m_dCurveLength;
}


void ONetworkEdge::Print(std::ostream& os) const
{
    os << "Source: ";
    GetSource()->Print(os);
    os << endl;
    os << "Target: ";
    GetTarget()->Print(os);
    os << endl;
    os << "Source-Point: ";
    GetSourcePoint().Print(os);
    os << endl;
    os << "Target-Point: ";
    GetTargetPoint().Print(os);
    os << endl;
    os << "Curve: ";
    GetCurve()->Print(os);
    os << endl;
    os << "RoadName: " << GetRoadName() << endl;
    os << "RoadType: " << GetRoadType() << endl;
    os << "MaxSpeed: " << GetMaxSpeed() << endl;
}

