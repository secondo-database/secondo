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

This implementation file contains the implementation of the class ~NetworkSection~.


2 Defines and includes

*/

#include "NetworkSection.h"
#include "NetworkRoute.h"
#include "../Network/NetworkAlgebra.h"

namespace mapmatch {

NetworkSection::NetworkSection(Tuple* pTupleSection,
                               Network* pNetwork,
                               bool bIncReference)
:m_pTupleSection(pTupleSection), m_pNetwork(pNetwork), m_pNetworkRoute(NULL)
{
    if (bIncReference && m_pTupleSection != NULL)
        m_pTupleSection->IncReference();
}

NetworkSection::NetworkSection(const NetworkSection& rNetworkSection)
:m_pTupleSection(rNetworkSection.m_pTupleSection),
 m_pNetwork(rNetworkSection.m_pNetwork),
 m_pNetworkRoute(NULL)
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

        delete m_pNetworkRoute; m_pNetworkRoute = NULL;
        if (rNetworkSection.m_pNetworkRoute != NULL)
           m_pNetworkRoute = new NetworkRoute(*rNetworkSection.m_pNetworkRoute);
    }
    return *this;
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
            Tuple* pRouteTuple = m_pNetwork->GetRoute(GetRouteID());
            m_pNetworkRoute = new NetworkRoute(pRouteTuple, false);
            return *m_pNetworkRoute;
        }
        else
        {
            static NetworkRoute Dummy(NULL);
            return Dummy;
        }
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
        return NULL;
    }
}

} // end of namespace mapmatch
