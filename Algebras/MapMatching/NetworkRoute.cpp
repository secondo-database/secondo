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

This implementation file contains the implementation of the class ~NetworkRoute~.


2 Defines and includes

*/

#include "NetworkRoute.h"
#include "../Network/NetworkAlgebra.h"

namespace mapmatch {

NetworkRoute::NetworkRoute(Tuple* pTupleRoute, bool bIncReference)
:m_pTupleRoute(pTupleRoute)
{
    if (bIncReference && m_pTupleRoute != NULL)
        m_pTupleRoute->IncReference();
}

NetworkRoute::NetworkRoute(const NetworkRoute& rNetworkRoute)
:m_pTupleRoute(rNetworkRoute.m_pTupleRoute)
{
    if (m_pTupleRoute != NULL)
      m_pTupleRoute->IncReference();
}

NetworkRoute::~NetworkRoute()
{
    if (m_pTupleRoute != NULL)
        m_pTupleRoute->DeleteIfAllowed();
    m_pTupleRoute = NULL;
}

const NetworkRoute& NetworkRoute::operator=(const NetworkRoute& rNetworkRoute)
{
    if (&rNetworkRoute != this)
    {
        if (m_pTupleRoute != NULL)
        {
            m_pTupleRoute->DeleteIfAllowed();
            m_pTupleRoute = NULL;
        }

        m_pTupleRoute = rNetworkRoute.m_pTupleRoute;
        if (m_pTupleRoute != NULL)
          m_pTupleRoute->IncReference();
    }
    return *this;
}

int NetworkRoute::GetRouteID(void) const
{
    if (m_pTupleRoute != NULL)
    {
        return static_cast<CcInt*>(m_pTupleRoute->
                GetAttribute(ROUTE_ID))->GetIntval();
    }
    else
    {
        return -1;
    }
}

double NetworkRoute::GetRouteLength(void) const
{
    if (m_pTupleRoute != NULL)
    {
        return static_cast<CcReal*>(m_pTupleRoute->
                GetAttribute(ROUTE_LENGTH))->GetRealval();
    }
    else
    {
        return -1;
    }
}

const SimpleLine* NetworkRoute::GetCurve(void) const
{
    if (m_pTupleRoute != NULL)
    {
        return static_cast<SimpleLine*>(m_pTupleRoute->
                GetAttribute(ROUTE_CURVE));
    }
    else
    {
        return NULL;
    }
}

bool NetworkRoute::GetDual(void) const
{
    if (m_pTupleRoute != NULL)
    {
        return static_cast<CcBool*>(m_pTupleRoute->
                GetAttribute(ROUTE_DUAL))->GetBoolval();
    }
    else
    {
        return false;
    }
}

bool NetworkRoute::GetStartsSmaller(void) const
{
    if (m_pTupleRoute != NULL)
    {
        return static_cast<CcBool*>(m_pTupleRoute->
                GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();
    }
    else
    {
        return false;
    }
}

} // end of namespace mapmatch
