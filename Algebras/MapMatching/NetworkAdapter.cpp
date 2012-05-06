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

[1] Implementation File containing class to access Network
    via IMMNetwork interface

April, 2012. Matthias Roth

[TOC]

1 Overview

This implementation file contains the implementation of
the classes ~NetworkAdapter~ and ~NetworkSectionAdapter~.

2 Defines and includes

*/

#include "NetworkAdapter.h"
#include "NetworkAlgebra.h"
#include "SpatialAlgebra.h"
#include <vector>
#include "MapMatchingUtil.h"
#include "NetworkRoute.h"
#include "NetworkSection.h"


namespace mapmatch {


/*
3 class NetworkAdapter

*/

NetworkAdapter::NetworkAdapter(Network* pNetwork, double dNetworkScale)
:m_pNetwork(pNetwork),
 m_dNetworkScale(dNetworkScale)
{
}

NetworkAdapter::NetworkAdapter(const NetworkAdapter& rNetworkAdapter)
:m_pNetwork(rNetworkAdapter.m_pNetwork),
 m_dNetworkScale(rNetworkAdapter.m_dNetworkScale)
{
}

NetworkAdapter::~NetworkAdapter()
{
    m_pNetwork = NULL;
}

bool NetworkAdapter::GetSections(
                const Rectangle<2>& rBBox,
                std::vector<shared_ptr<IMMNetworkSection> >& rvecSections) const
{
    if (m_pNetwork == NULL)
        return false;

    AttributePtr<Region> pRegionBBox(new Region(rBBox));

    R_TreeLeafEntry<2, TupleId> res;
    if (m_pNetwork->GetRTree()->First(rBBox, res))
    {
        NetworkRoute Route(m_pNetwork->GetRoutes()->GetTuple(res.info, false),
                           false);

        GetSectionsOfRoute(Route, *pRegionBBox, rvecSections);
    }

    while (m_pNetwork->GetRTree()->Next(res))
    {
        NetworkRoute Route(m_pNetwork->GetRoutes()->GetTuple(res.info, false),
                          false);
        GetSectionsOfRoute(Route, *pRegionBBox, rvecSections);
    }

    return true;
}

void NetworkAdapter::GetSectionsOfRoute(
                 const NetworkRoute& rNetworkRoute,
                 const Region& rRegion,
                 std::vector<shared_ptr<IMMNetworkSection> >& rvecSectRes) const
{
    if (!rNetworkRoute.IsDefined() || !rRegion.IsDefined())
        return;

    const int nRouteID = rNetworkRoute.GetRouteID();
    const SimpleLine* pRouteCurve = rNetworkRoute.GetCurve();

    if (pRouteCurve != NULL &&
        pRouteCurve->IsDefined() &&
        MMUtil::Intersects(rRegion, *pRouteCurve))  // TODO
    {
        RouteInterval Interval(nRouteID, 0.0,
                std::numeric_limits<double>::max());

        std::vector<TupleId> vecSections;
        m_pNetwork->GetSectionsOfRoutInterval(&Interval, vecSections);

        std::vector<TupleId>::const_iterator itEnd = vecSections.end();

        for (std::vector<TupleId>::const_iterator it = vecSections.begin();
             it != itEnd; ++it)
        {
            NetworkSection Section(m_pNetwork->GetSection(*it),
                                   m_pNetwork, false);
            if (Section.IsDefined())
            {
                const SimpleLine* pSectionCurve = Section.GetCurve();
                if (pSectionCurve != NULL &&
                    MMUtil::Intersects(rRegion, *pSectionCurve))
                {
                    DirectedNetworkSection SectionUp(Section,
                                                DirectedNetworkSection::DIR_UP);

                    shared_ptr<IMMNetworkSection> pSectionUp(
                                          new NetworkSectionAdapter(SectionUp));
                    rvecSectRes.push_back(pSectionUp);

                    DirectedNetworkSection SectionDown(Section,
                                              DirectedNetworkSection::DIR_DOWN);

                    shared_ptr<IMMNetworkSection> pSectionDown(
                                        new NetworkSectionAdapter(SectionDown));
                    rvecSectRes.push_back(pSectionDown);
                }
            }
        }
    }
}

Rectangle<2> NetworkAdapter::GetBoundingBox(void) const
{
    if (m_pNetwork != NULL)
        return m_pNetwork->BoundingBox();
    else
        return Rectangle<2>(false);
}

double NetworkAdapter::GetNetworkScale(void) const
{
    return m_dNetworkScale;
}

bool NetworkAdapter::IsDefined(void) const
{
    return m_pNetwork != NULL && m_pNetwork->IsDefined();
}



/*
4 class NetworkSectionAdapter

*/

NetworkSectionAdapter::NetworkSectionAdapter()
:m_pSection(NULL)
{
}

NetworkSectionAdapter::NetworkSectionAdapter(
                            const DirectedNetworkSection& rSection)
:m_pSection(new DirectedNetworkSection(rSection))
{
}

NetworkSectionAdapter::NetworkSectionAdapter(
                            const NetworkSectionAdapter& rNetworkSectionAdapter)
:m_pSection(rNetworkSectionAdapter.m_pSection != NULL ?
          new DirectedNetworkSection(*rNetworkSectionAdapter.m_pSection) : NULL)
{
}

NetworkSectionAdapter::~NetworkSectionAdapter()
{
    delete m_pSection;
    m_pSection = NULL;
}

const SimpleLine* NetworkSectionAdapter::GetCurve(void) const
{
    if (m_pSection != NULL)
        return m_pSection->GetCurve();
    else
        return NULL;
}

double NetworkSectionAdapter::GetCurveLength(const double dScale) const
{
    if (m_pSection != NULL)
        return m_pSection->GetCurveLength(dScale);
    else
        return 0.0;
}

bool NetworkSectionAdapter::GetCurveStartsSmaller(void) const
{
    if (m_pSection != NULL)
        return m_pSection->GetCurveStartsSmaller();
    else
        return true;
}

Point NetworkSectionAdapter::GetStartPoint(void) const
{
    if (m_pSection != NULL)
        return m_pSection->GetStartPoint();
    else
        return Point(false);
}

Point NetworkSectionAdapter::GetEndPoint(void) const
{
    if (m_pSection != NULL)
        return m_pSection->GetEndPoint();
    else
        return Point(false);
}

IMMNetworkSection::EDirection NetworkSectionAdapter::GetDirection(void) const
{
    if (m_pSection != NULL)
    {
        switch(m_pSection->GetDirection())
        {
        case DirectedNetworkSection::DIR_UP:
            return DIR_UP;
        case DirectedNetworkSection::DIR_DOWN:
            return DIR_DOWN;
        case DirectedNetworkSection::DIR_NONE:
        default:
            return DIR_NONE;
        }
    }
    else
    {
        return DIR_NONE;
    }
}

bool NetworkSectionAdapter::IsDefined(void) const
{
    return m_pSection != NULL && m_pSection->IsDefined();
}

bool NetworkSectionAdapter::GetAdjacentSections(
                 const bool _bUpDown,
                 std::vector<shared_ptr<IMMNetworkSection> >& vecSections) const
{
    if (m_pSection == NULL)
        return false;

    const Network* pNetwork = m_pSection->GetNetwork();
    if (pNetwork == NULL)
        return false;

    /*bool bUpDown = GetDirection() == INetworkSection::DIR_UP;

    if (!_bUpDown)
        bUpDown = !bUpDown;*/

    vector<DirectedSection> vecAdjSections;
    pNetwork->GetAdjacentSections(m_pSection->GetSectionID(),
                                  _bUpDown, vecAdjSections);

    const size_t nAdjSections = vecAdjSections.size();
    for (size_t i = 0; i < nAdjSections; ++i)
    {
        Tuple* pSectionTuple = pNetwork->GetSection(
                                             vecAdjSections[i].GetSectionTid());
        DirectedNetworkSection adjSection(
                                      pSectionTuple,
                                      pNetwork,
                                      false,
                                      vecAdjSections[i].GetUpDownFlag() ?
                                              DirectedNetworkSection::DIR_UP :
                                              DirectedNetworkSection::DIR_DOWN);

        if (adjSection.GetSectionID() == m_pSection->GetSectionID())
            continue;

        shared_ptr<IMMNetworkSection> pAdjSection(
                                         new NetworkSectionAdapter(adjSection));
        vecSections.push_back(pAdjSection);
    }

    return true;
}

bool NetworkSectionAdapter::operator==(const IMMNetworkSection& rSection) const
{
    const NetworkSectionAdapter* pSectionAdapter =
                                                rSection.CastToNetworkSection();
    if (pSectionAdapter != NULL)
    {
        DirectedNetworkSection* pSectionOther = pSectionAdapter->m_pSection;
        if (pSectionOther != NULL && m_pSection != NULL)
        {
            return *m_pSection == *pSectionOther;
        }
        else
        {
            return m_pSection == pSectionOther;
        }
    }
    else
    {
        assert(false);
        return false;
    }
}

const NetworkSectionAdapter* NetworkSectionAdapter::CastToNetworkSection(void)
                                                                           const
{
    return this;
}

NetworkSectionAdapter* NetworkSectionAdapter::CastToNetworkSection(void)
{
    return this;
}

void NetworkSectionAdapter::PrintIdentifier(std::ostream& os) const
{
    if (m_pSection != NULL)
        os << m_pSection->GetSectionID();
    else
        os << "invalid";
}


} // end of namespace mapmatch

