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

[1] Implementation File containing class to access ONetwork
    via IMMNetwork interface

April, 2012. Matthias Roth

[TOC]

1 Overview

This implementation file contains the implementation of
the classes ~ONetworkAdapter~ and ~ONetworkSectionAdapter~.

2 Defines and includes

*/

#include "ONetworkAdapter.h"
#include "ONetwork.h"
#include "ONetworkEdge.h"
#include "SpatialAlgebra.h"
#include <vector>
#include "MapMatchingUtil.h"


namespace mapmatch {


/*
3 class ONetworkAdapter

*/

ONetworkAdapter::ONetworkAdapter(ONetwork* pNetwork)
:m_pNetwork(pNetwork)
{
}

ONetworkAdapter::ONetworkAdapter(const ONetworkAdapter& rNetworkAdapter)
:m_pNetwork(rNetworkAdapter.m_pNetwork)
{
}

ONetworkAdapter::~ONetworkAdapter()
{
    m_pNetwork = NULL;
}

bool ONetworkAdapter::GetSections(
                const Rectangle<2>& rBBox,
                std::vector<shared_ptr<IMMNetworkSection> >& rvecSections) const
{
    if (m_pNetwork == NULL)
        return false;

    std::vector<ONetworkEdge> vecEdges;
    if (!m_pNetwork->GetEdges(rBBox, vecEdges))
        return false;

    const size_t nEdges = vecEdges.size();
    for (size_t i = 0; i < nEdges; ++i)
    {
        const ONetworkEdge& rEdge = vecEdges[i];
        shared_ptr<IMMNetworkSection> pSection(
                                 new ONetworkSectionAdapter(rEdge, m_pNetwork));
        rvecSections.push_back(pSection);
    }

    return true;
}

Rectangle<2> ONetworkAdapter::GetBoundingBox(void) const
{
    if (m_pNetwork != NULL)
        return m_pNetwork->GetBoundingBox();
    else
        return Rectangle<2>(false);
}

double ONetworkAdapter::GetNetworkScale(void) const
{
    return 1.0;
}

bool ONetworkAdapter::IsDefined(void) const
{
    return m_pNetwork != NULL;
}

bool ONetworkAdapter::CanGetRoadType(void) const
{
    return true;
}

// Only used by MapMatchingOEdgeTupleStreamCreator
Tuple* ONetworkAdapter::GetUndefEdgeTuple(void) const
{
    if (m_pNetwork != NULL)
        return m_pNetwork->GetUndefEdgeTuple();
    else
        return NULL;
}



/*
4 class ONetworkSectionAdapter

*/

ONetworkSectionAdapter::ONetworkSectionAdapter()
:m_pEdge(NULL),
 m_pONetwork(NULL),
 m_eRoadType((ERoadType)-1)
{
}

ONetworkSectionAdapter::ONetworkSectionAdapter(
                            const ONetworkEdge& rEdge,
                            const ONetwork* pONetwork)
:m_pEdge(new ONetworkEdge(rEdge)),
 m_pONetwork(pONetwork),
 m_eRoadType((ERoadType)-1)
{
}

ONetworkSectionAdapter::ONetworkSectionAdapter(
                          const ONetworkSectionAdapter& rONetworkSectionAdapter)
:m_pEdge(rONetworkSectionAdapter.m_pEdge != NULL ?
            new ONetworkEdge(*rONetworkSectionAdapter.m_pEdge) : NULL),
 m_pONetwork(rONetworkSectionAdapter.m_pONetwork),
 m_eRoadType(rONetworkSectionAdapter.m_eRoadType)
{
}

ONetworkSectionAdapter::~ONetworkSectionAdapter()
{
    delete m_pEdge;
    m_pEdge = NULL;

    m_pONetwork = NULL;
}

const SimpleLine* ONetworkSectionAdapter::GetCurve(void) const
{
    if (m_pEdge != NULL)
        return m_pEdge->GetCurve();
    else
        return NULL;
}

double ONetworkSectionAdapter::GetCurveLength(const double dScale) const
{
    if (m_pEdge != NULL)
        return m_pEdge->GetCurveLength(dScale);
    else
        return 0.0;
}

bool ONetworkSectionAdapter::GetCurveStartsSmaller(void) const
{
    if (m_pEdge != NULL) // TODO
        return m_pEdge->GetCurve()->GetStartSmaller();
    else
        return true;
}

Point ONetworkSectionAdapter::GetStartPoint(void) const
{
    if (m_pEdge != NULL)
        return m_pEdge->GetSourcePoint();
    else
        return Point(false);
}

Point ONetworkSectionAdapter::GetEndPoint(void) const
{
    if (m_pEdge != NULL)
        return m_pEdge->GetTargetPoint();
    else
        return Point(false);
}

IMMNetworkSection::EDirection ONetworkSectionAdapter::GetDirection(void) const
{
    if (m_pEdge != NULL)
    {
        return DIR_UP; // TODO
        /*switch(m_pEdge->GetDirection())
        {
        case DirectedNetworkSection::DIR_UP:
            return DIR_UP;
        case DirectedNetworkSection::DIR_DOWN:
            return DIR_DOWN;
        case DirectedNetworkSection::DIR_NONE:
        default:
            return DIR_NONE;
        }*/
    }
    else
    {
        return DIR_NONE;
    }
}

bool ONetworkSectionAdapter::IsDefined(void) const
{
    return m_pEdge != NULL && m_pEdge->IsDefined();
}

bool ONetworkSectionAdapter::GetAdjacentSections(
                const bool _bUpDown,
                std::vector<shared_ptr<IMMNetworkSection> >& rvecSections) const
{
    if (m_pEdge == NULL || m_pONetwork == NULL)
        return false;

    std::vector<ONetworkEdge> vecEdges;
    m_pONetwork->GetAdjacentEdges(*m_pEdge, _bUpDown, vecEdges);

    const int nSource = m_pEdge->GetSource()->GetValue();
    const int nTarget = m_pEdge->GetTarget()->GetValue();

    size_t nEdges = vecEdges.size();
    for (size_t i = 0; i < nEdges; ++i)
    {
        const ONetworkEdge& rEdge = vecEdges[i];

        if ((rEdge.GetSource()->GetValue() == nSource &&
             rEdge.GetTarget()->GetValue() == nTarget)   ||
            (rEdge.GetSource()->GetValue() == nTarget &&
             rEdge.GetTarget()->GetValue() == nSource))
        {
            continue;
        }

        shared_ptr<IMMNetworkSection> pAdjSection(
                                new ONetworkSectionAdapter(rEdge, m_pONetwork));
        rvecSections.push_back(pAdjSection);
    }

    return true;
}

std::string ONetworkSectionAdapter::GetRoadName(void) const
{
    if (m_pEdge != NULL)
    {
        return m_pEdge->GetRoadName();
    }
    else
    {
        return "";
    }
}

IMMNetworkSection::ERoadType ONetworkSectionAdapter::GetRoadType(void) const
{
    if (((int)m_eRoadType) >= 0)
    {
        return m_eRoadType;
    }
    else if (m_pEdge != NULL)
    {
        const std::string& strRoadType = m_pEdge->GetRoadType();

        if (strRoadType.compare("motorway") == 0)
        {
            m_eRoadType = RT_MOTORWAY;
        }
        /*else if (strRoadType.compare("motorway_link") == 0)
        {
            m_eRoadType = RT_MOTORWAY_LINK;
        }*/
        else if (strRoadType.compare("trunk") == 0)
        {
            m_eRoadType = RT_TRUNK;
        }
        /*else if (strRoadType.compare("trunk_link") == 0)
        {
            m_eRoadType = RT_TRUNK_LINK;
        }*/
        else if (strRoadType.compare("primary") == 0)
        {
            m_eRoadType = RT_PRIMARY;
        }
        /*else if (strRoadType.compare("primary_link") == 0)
        {
            m_eRoadType = RT_PRIMARY_LINK;
        }*/
        else if (strRoadType.compare("secondary") == 0)
        {
            m_eRoadType = RT_SECONDARY;
        }
        /*else if (strRoadType.compare("secondary_link") == 0)
        {
            m_eRoadType = RT_SECONDARY_LINK;
        }*/
        else if (strRoadType.compare("tertiary") == 0)
        {
            m_eRoadType = RT_TERTIARY;
        }
        /*else if (strRoadType.compare("tertiary_link") == 0)
        {
            m_eRoadType = RT_TERTIARY_LINK;
        }*/
        else if (strRoadType.compare("living_street") == 0)
        {
            m_eRoadType = RT_LIVING_STREET;
        }
        else if (strRoadType.compare("pedestrian") == 0)
        {
            m_eRoadType = RT_PEDESTRIAN;
        }
        else if (strRoadType.compare("residential") == 0)
        {
            m_eRoadType = RT_RESIDENTIAL;
        }
        else if (strRoadType.compare("service") == 0)
        {
            m_eRoadType = RT_SERVICE;
        }
        else if (strRoadType.compare("track") == 0)
        {
            m_eRoadType = RT_TRACK;
        }
        /*else if (strRoadType.compare("bus_guideway") == 0)
        {
            m_eRoadType = RT_BUS_GUIDEWAY;
        }*/
        /*else if (strRoadType.compare("raceway") == 0)
        {
            m_eRoadType = RT_RACEWAY;
        }*/
        else if (strRoadType.compare("road") == 0)
        {
            m_eRoadType = RT_ROAD;
        }
        else if (strRoadType.compare("path") == 0)
        {
            m_eRoadType = RT_PATH;
        }
        else if (strRoadType.compare("footway") == 0)
        {
            m_eRoadType = RT_FOOTWAY;
        }
        else if (strRoadType.compare("cycleway") == 0)
        {
            m_eRoadType = RT_CYCLEWAY;
        }
        else if (strRoadType.compare("bridleway") == 0)
        {
            m_eRoadType = RT_BRIDLEWAY;
        }
        else if (strRoadType.compare("steps") == 0)
        {
            m_eRoadType = RT_STEPS;
        }
        else if (strRoadType.compare("proposed") == 0)
        {
            m_eRoadType = RT_PROPOSED;
        }
        else if (strRoadType.compare("construction") == 0)
        {
            m_eRoadType = RT_CONSTRUCTION;
        }
        else
        {
            m_eRoadType = RT_OTHER;
        }

        return m_eRoadType;
    }
    else
    {
        return IMMNetworkSection::RT_UNKNOWN;
    }
}

double ONetworkSectionAdapter::GetMaxSpeed(void) const
{
    if (m_pEdge != NULL)
    {
        return m_pEdge->GetMaxSpeed();
    }
    else
    {
        return -1.0;
    }
}

bool ONetworkSectionAdapter::operator==(const IMMNetworkSection& rSection) const
{
    const ONetworkSectionAdapter* pSectionAdapter =
                                               rSection.CastToONetworkSection();
    if (pSectionAdapter != NULL)
    {
        ONetworkEdge* pEdgeOther = pSectionAdapter->m_pEdge;
        if (pEdgeOther != NULL && m_pEdge != NULL)
        {
            return *m_pEdge == *pEdgeOther;
        }
        else
        {
            return m_pEdge == pEdgeOther;
        }
    }
    else
    {
        assert(false);
        return false;
    }
}

const ONetworkSectionAdapter* ONetworkSectionAdapter::CastToONetworkSection(
                                                                     void) const
{
    return this;
}

ONetworkSectionAdapter* ONetworkSectionAdapter::CastToONetworkSection(void)
{
    return this;
}

void ONetworkSectionAdapter::PrintIdentifier(std::ostream& os) const
{
    if (m_pEdge != NULL)
    {
        os << m_pEdge->GetSource()->GetValue()
           << " : "
           << m_pEdge->GetTarget()->GetValue();
    }
    else
        os << "invalid";
}


} // end of namespace mapmatch

