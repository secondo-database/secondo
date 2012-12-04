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

[1] Implementation of the NetworkSection class

January-April, 2012. Matthias Roth

[TOC]

1 Overview

This implementation file contains the implementation of the class
~NetworkSection~.


2 Defines and includes

*/

#include "NetworkSection.h"
#include "NetworkRoute.h"
#include "../Network/NetworkAlgebra.h"
#include "MapMatchingUtil.h"
using namespace mapmatch;
using namespace network;

/*
3 class NetworkSection
  accessing the attributes of a Network-Section

*/

NetworkSection::NetworkSection()
:m_pTupleSection(NULL), m_pNetwork(NULL),
 m_pNetworkRoute(NULL), m_dCurveLength(-1.0)
{
}

NetworkSection::NetworkSection(Tuple* pTupleSection,
                               const Network* pNetwork,
                               bool bIncReference)
:m_pTupleSection(pTupleSection), m_pNetwork(pNetwork),
 m_pNetworkRoute(NULL), m_dCurveLength(-1.0)
{
    if (bIncReference && m_pTupleSection != NULL)
        m_pTupleSection->IncReference();
}

NetworkSection::NetworkSection(const NetworkSection& rNetworkSection)
:m_pTupleSection(rNetworkSection.m_pTupleSection),
 m_pNetwork(rNetworkSection.m_pNetwork),
 m_pNetworkRoute(NULL), m_dCurveLength(rNetworkSection.m_dCurveLength)
{
    if (m_pTupleSection != NULL)
        m_pTupleSection->IncReference();

    if (rNetworkSection.m_pNetworkRoute != NULL)
        m_pNetworkRoute = new NetworkRoute(*rNetworkSection.m_pNetworkRoute);
}

NetworkSection::~NetworkSection()
{
    if (m_pTupleSection != NULL)
        m_pTupleSection->DeleteIfAllowed();
    m_pTupleSection = NULL;
    m_pNetwork = NULL;
    if (m_pNetworkRoute != NULL)
        delete m_pNetworkRoute;
    m_pNetworkRoute = NULL;
}

const NetworkSection& NetworkSection::operator=(
        const NetworkSection& rNetworkSection)
{
    if (&rNetworkSection != this)
    {
        if (m_pTupleSection != NULL)
        {
            m_pTupleSection->DeleteIfAllowed();
            m_pTupleSection = NULL;
        }

        m_pTupleSection = rNetworkSection.m_pTupleSection;
        if (m_pTupleSection != NULL)
            m_pTupleSection->IncReference();

        m_pNetwork = rNetworkSection.m_pNetwork;

        if (m_pNetworkRoute != NULL)
            delete m_pNetworkRoute;
        m_pNetworkRoute = NULL;

        if (rNetworkSection.m_pNetworkRoute != NULL)
           m_pNetworkRoute = new NetworkRoute(*rNetworkSection.m_pNetworkRoute);

        m_dCurveLength = rNetworkSection.m_dCurveLength;
    }
    return *this;
}

bool NetworkSection::operator==(const NetworkSection& rSection) const
{
    if (m_pTupleSection != NULL && rSection.m_pTupleSection != NULL)
    {
        return (m_pTupleSection->GetTupleId() ==
                rSection.m_pTupleSection->GetTupleId());
    }
    else
    {
        return m_pTupleSection == rSection.m_pTupleSection;
    }
}

int NetworkSection::GetRouteID(void) const
{
    if (m_pTupleSection != NULL)
    {
        return static_cast<CcInt*>(m_pTupleSection->
                GetAttribute(SECTION_RID))->GetIntval();
    }
    else
    {
        return -1;
    }
}

const NetworkRoute& NetworkSection::GetRoute(void) const
{
    if (m_pNetworkRoute == NULL)
    {
        if (m_pNetwork != NULL)
        {
            TupleIdentifier* pId = this->GetRRC();
            if (pId != NULL)
            {
                Tuple* pRouteTuple = m_pNetwork->GetRoute(pId->GetTid());
                m_pNetworkRoute = new NetworkRoute(pRouteTuple, false);
                return *m_pNetworkRoute;
            }
        }

        static NetworkRoute Dummy(NULL);
        return Dummy;
    }
    else
    {
        return *m_pNetworkRoute;
    }
}

double NetworkSection::GetMeas1(void) const
{
    if (m_pTupleSection != NULL)
    {
        return static_cast<CcReal*>(m_pTupleSection->
                GetAttribute(SECTION_MEAS1))->GetRealval();
    }
    else
    {
        return -1.0;
    }
}

double NetworkSection::GetMeas2(void) const
{
    if (m_pTupleSection != NULL)
    {
        return static_cast<CcReal*>(m_pTupleSection->
                GetAttribute(SECTION_MEAS1))->GetRealval();
    }
    else
    {
        return -1.0;
    }
}

bool NetworkSection::GetDual(void) const
{
    if (m_pTupleSection != NULL)
    {
        return static_cast<CcBool*>(m_pTupleSection->
                GetAttribute(SECTION_DUAL))->GetBoolval();
    }
    else
    {
        return false;
    }
}

const SimpleLine* NetworkSection::GetCurve(void) const
{
    if (m_pTupleSection != NULL)
    {
        return static_cast<SimpleLine*>(m_pTupleSection->
                GetAttribute(SECTION_CURVE));
    }
    else
    {
        return NULL;
    }
}

bool NetworkSection::GetCurveStartsSmaller(void) const
{
    if (m_pTupleSection != NULL)
    {
        return static_cast<CcBool*>(m_pTupleSection->
                GetAttribute(SECTION_CURVE_STARTS_SMALLER))->GetBoolval();
    }
    else
    {
        return false;
    }
}

double NetworkSection::GetCurveLength(const double dScale) const
{
    if (m_dCurveLength < 0.)
    {
        m_dCurveLength = MMUtil::CalcLengthCurve(GetCurve(), dScale);
    }

    return m_dCurveLength;
}

TupleIdentifier* NetworkSection::GetRRC(void) const
{
    if (m_pTupleSection != NULL)
    {
        return static_cast<TupleIdentifier*>(m_pTupleSection->
                GetAttribute ( SECTION_RRC ));
    }
    else
    {
        return NULL;
    }
}

int NetworkSection::GetSectionID(void) const
{
    if (m_pTupleSection != NULL)
    {
        return static_cast<CcInt*>(m_pTupleSection->
                GetAttribute(SECTION_SID))->GetIntval();
    }
    else
    {
        return 0;
    }
}

Point NetworkSection::GetStartPoint(void) const
{
    const SimpleLine* pCurve = GetCurve();
    if (pCurve == NULL || !pCurve->IsDefined())
    {
        return Point(false);
    }
    else
    {
        const bool bStartsSmaller = GetCurveStartsSmaller();
        return pCurve->StartPoint(bStartsSmaller);
    }
}

Point NetworkSection::GetEndPoint(void) const
{
    const SimpleLine* pCurve = GetCurve();
    if (pCurve == NULL || !pCurve->IsDefined())
    {
        return Point(false);
    }
    else
    {
        const bool bStartsSmaller = GetCurveStartsSmaller();
        return pCurve->EndPoint(bStartsSmaller);
    }
}


/*
4 class DirectedNetworkSection
  accessing the attributes of a directed Network-Section

*/

DirectedNetworkSection::DirectedNetworkSection()
:NetworkSection(),
 m_eDirection(DirectedNetworkSection::DIR_NONE)
{
}

DirectedNetworkSection::DirectedNetworkSection(Tuple* pTupleSection,
                                               const Network* pNetwork,
                                               bool bIncReference,
                                               const EDirection eDirection)
:NetworkSection(pTupleSection, pNetwork, bIncReference),
 m_eDirection(eDirection)
{
}

DirectedNetworkSection::DirectedNetworkSection(
                                  const DirectedNetworkSection& rNetworkSection)
:NetworkSection(rNetworkSection),
 m_eDirection(rNetworkSection.m_eDirection)
{
}

DirectedNetworkSection::DirectedNetworkSection(
                                  const NetworkSection& rNetworkSection,
                                  const EDirection eDirection)
:NetworkSection(rNetworkSection),
 m_eDirection(eDirection)
{
}

DirectedNetworkSection::~DirectedNetworkSection()
{
}

const DirectedNetworkSection& DirectedNetworkSection::operator=
                                (const DirectedNetworkSection& rNetworkSection)
{
    if (&rNetworkSection != this)
    {
        NetworkSection::operator =(rNetworkSection);
        m_eDirection = rNetworkSection.m_eDirection;
    }

    return *this;
}

bool DirectedNetworkSection::operator==
                                  (const DirectedNetworkSection& rSection) const
{
    if (NetworkSection::operator==(rSection))
    {
        return m_eDirection == rSection.m_eDirection;
    }
    else
    {
        return false;
    }
}


Point DirectedNetworkSection::GetStartPoint(void) const
{
    return NetworkSection::GetStartPoint();
}

Point DirectedNetworkSection::GetEndPoint(void) const
{
    return NetworkSection::GetEndPoint();
}

