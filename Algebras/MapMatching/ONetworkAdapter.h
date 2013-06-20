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

[1] Header File containing class to access ONetwork via IMMNetwork interface

April, 2012. Matthias Roth

[TOC]

1 Overview

This header file contains the class ~ONetworkAdapter~

2 Defines and includes

*/

#ifndef __ONETWORKADAPTER_H_
#define __ONETWORKADAPTER_H_

#include "MapMatchingNetworkInterface.h"

//template<class T> class ONetwork;

#include "ONetwork.h"

//template<class T> class ONetworkEdge;

#include "ONetworkEdge.h"


namespace mapmatch {


/*
3 ~ONetworkAdapter~

*/
template<class T>
class ONetworkAdapter : public IMMNetwork
{
public:
    ONetworkAdapter(ONetwork<T>* pNetwork = NULL);
    ONetworkAdapter(const ONetworkAdapter<T>& rONetworkAdapter);

    virtual ~ONetworkAdapter();

    virtual bool GetSections(
               const Rectangle<2>& rBBox,
               std::vector<shared_ptr<IMMNetworkSection> >& rvecSections) const;

    virtual Rectangle<2> GetBoundingBox(void) const;

    virtual double GetNetworkScale(void) const;

    virtual bool IsDefined(void) const;

    virtual bool CanGetRoadType(void) const;

    // Only used by MapMatchingOEdgeTupleStreamCreator
    Tuple* GetUndefEdgeTuple(void) const;

private:

    ONetwork<T>* m_pNetwork;
};


/*
4 ~ONetworkSectionAdapter~

*/
template<class T>
class ONetworkSectionAdapter : public IMMNetworkSection
{
public:
    ONetworkSectionAdapter();
    ONetworkSectionAdapter(const ONetworkEdge<T>& rEdge,
                           const ONetwork<T>* pONetwork);
    ONetworkSectionAdapter(const ONetworkSectionAdapter<T>&
                                   rONetworkSectionAdapter);
    virtual ~ONetworkSectionAdapter();

    virtual const class SimpleLine* GetCurve(void) const;

    virtual double GetCurveLength(const double dScale) const;

    virtual bool GetCurveStartsSmaller(void) const;

    virtual Point GetStartPoint(void) const;

    virtual Point GetEndPoint(void) const;

    virtual EDirection GetDirection(void) const;

    virtual bool IsDefined(void) const;

    virtual bool GetAdjacentSections(
                const bool _bUpDown,
                std::vector<shared_ptr<IMMNetworkSection> >& vecSections) const;

    virtual std::string GetRoadName(void) const;

    virtual ERoadType GetRoadType(void) const;

    virtual double GetMaxSpeed(void) const;

    virtual bool operator==(const IMMNetworkSection& rSection) const;

    virtual const ONetworkSectionAdapter<T>* CastToONetworkSection(void) const;

    virtual ONetworkSectionAdapter<T>* CastToONetworkSection(void);


    virtual void PrintIdentifier(std::ostream& os) const;

    const ONetworkEdge<T>* GetNetworkEdge(void) const {return m_pEdge;}

private:
    ONetworkEdge<T>* m_pEdge;
    const ONetwork<T>* m_pONetwork;
    mutable ERoadType m_eRoadType;
};


/*
2 Implementation

*/

/*
2.1 class ONetworkAdapter

*/
template<class T>
ONetworkAdapter<T>::ONetworkAdapter(ONetwork<T>* pNetwork)
:m_pNetwork(pNetwork)
{
}

template<class T>
ONetworkAdapter<T>::ONetworkAdapter(const ONetworkAdapter<T>& rNetworkAdapter)
:m_pNetwork(rNetworkAdapter.m_pNetwork)
{
}

template<class T>
ONetworkAdapter<T>::~ONetworkAdapter()
{
    m_pNetwork = NULL;
}

template<class T>
bool ONetworkAdapter<T>::GetSections(
                const Rectangle<2>& rBBox,
                std::vector<shared_ptr<IMMNetworkSection> >& rvecSections) const
{
    if (m_pNetwork == NULL)
        return false;

    std::vector<ONetworkEdge<T> > vecEdges;
    if (!m_pNetwork->GetEdges(rBBox, vecEdges))
        return false;

    const size_t nEdges = vecEdges.size();
    for (size_t i = 0; i < nEdges; ++i)
    {
        const ONetworkEdge<T>& rEdge = vecEdges[i];
        shared_ptr<IMMNetworkSection> pSection(
                       new ONetworkSectionAdapter<T>(rEdge, m_pNetwork));
        rvecSections.push_back(pSection);
    }

    return true;
}

template<class T>
Rectangle<2> ONetworkAdapter<T>::GetBoundingBox(void) const
{
    if (m_pNetwork != NULL)
        return m_pNetwork->GetBoundingBox();
    else
        return Rectangle<2>(false);
}

template<class T>
double ONetworkAdapter<T>::GetNetworkScale(void) const
{
    return 1.0;
}

template<class T>
bool ONetworkAdapter<T>::IsDefined(void) const
{
    return m_pNetwork != NULL;
}

template<class T>
bool ONetworkAdapter<T>::CanGetRoadType(void) const
{
    return true;
}

// Only used by MapMatchingOEdgeTupleStreamCreator
template<class T>
Tuple* ONetworkAdapter<T>::GetUndefEdgeTuple(void) const
{
    if (m_pNetwork != NULL)
        return m_pNetwork->GetUndefEdgeTuple();
    else
        return NULL;
}


/*
2.2 class ONetworkSectionAdapter

*/
template<class T>
ONetworkSectionAdapter<T>::ONetworkSectionAdapter()
:m_pEdge(NULL),
 m_pONetwork(NULL),
 m_eRoadType((ERoadType)-1)
{
}

template<class T>
ONetworkSectionAdapter<T>::ONetworkSectionAdapter(
                            const ONetworkEdge<T>& rEdge,
                            const ONetwork<T>* pONetwork)
:m_pEdge(new ONetworkEdge<T>(rEdge)),
 m_pONetwork(pONetwork),
 m_eRoadType((ERoadType)-1)
{
}

template<class T>
ONetworkSectionAdapter<T>::ONetworkSectionAdapter(
            const ONetworkSectionAdapter<T>& rONetworkSectionAdapter)
:m_pEdge(rONetworkSectionAdapter.m_pEdge != NULL ?
            new ONetworkEdge<T>(*rONetworkSectionAdapter.m_pEdge) : NULL),
 m_pONetwork(rONetworkSectionAdapter.m_pONetwork),
 m_eRoadType(rONetworkSectionAdapter.m_eRoadType)
{
}

template<class T>
ONetworkSectionAdapter<T>::~ONetworkSectionAdapter()
{
    delete m_pEdge;
    m_pEdge = NULL;

    m_pONetwork = NULL;
}

template<class T>
const SimpleLine* ONetworkSectionAdapter<T>::GetCurve(void) const
{
    if (m_pEdge != NULL)
        return m_pEdge->GetCurve();
    else
        return NULL;
}

template<class T>
double ONetworkSectionAdapter<T>::GetCurveLength(const double dScale) const
{
    if (m_pEdge != NULL)
        return m_pEdge->GetCurveLength(dScale);
    else
        return 0.0;
}

template<class T>
bool ONetworkSectionAdapter<T>::GetCurveStartsSmaller(void) const
{
    if (m_pEdge != NULL)
        return m_pEdge->GetCurve()->GetStartSmaller();
    else
        return true;
}

template<class T>
Point ONetworkSectionAdapter<T>::GetStartPoint(void) const
{
    if (m_pEdge != NULL)
        return m_pEdge->GetSourcePoint();
    else
        return Point(false);
}

template<class T>
Point ONetworkSectionAdapter<T>::GetEndPoint(void) const
{
    if (m_pEdge != NULL)
        return m_pEdge->GetTargetPoint();
    else
        return Point(false);
}

template<class T>
IMMNetworkSection::EDirection 
            ONetworkSectionAdapter<T>::GetDirection(void) const
{
    if (m_pEdge != NULL)
    {
        return DIR_UP;
    }
    else
    {
        return DIR_NONE;
    }
}

template<class T>
bool ONetworkSectionAdapter<T>::IsDefined(void) const
{
    return m_pEdge != NULL && m_pEdge->IsDefined();
}

template<class T>
bool ONetworkSectionAdapter<T>::GetAdjacentSections(
                const bool _bUpDown,
                std::vector<shared_ptr<IMMNetworkSection> >& rvecSections) const
{
    if (m_pEdge == NULL || m_pONetwork == NULL)
        return false;

    std::vector<ONetworkEdge<T> > vecEdges;
    m_pONetwork->GetAdjacentEdges(*m_pEdge, _bUpDown, vecEdges);

    const typename T::inttype nSource = m_pEdge->GetSource()->GetValue();
    const typename T::inttype nTarget = m_pEdge->GetTarget()->GetValue();

    size_t nEdges = vecEdges.size();
    for (size_t i = 0; i < nEdges; ++i)
    {
        const ONetworkEdge<T>& rEdge = vecEdges[i];

        if ((rEdge.GetSource()->GetValue() == nSource &&
             rEdge.GetTarget()->GetValue() == nTarget)   ||
            (rEdge.GetSource()->GetValue() == nTarget &&
             rEdge.GetTarget()->GetValue() == nSource))
        {
            continue;
        }

        shared_ptr<IMMNetworkSection> pAdjSection(
                   new ONetworkSectionAdapter<T>(rEdge, m_pONetwork));
        rvecSections.push_back(pAdjSection);
    }

    return true;
}

template<class T>
std::string ONetworkSectionAdapter<T>::GetRoadName(void) const
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

template<class T>
IMMNetworkSection::ERoadType ONetworkSectionAdapter<T>::GetRoadType(void) const
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

template<class T>
double ONetworkSectionAdapter<T>::GetMaxSpeed(void) const
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

template<class T>
bool ONetworkSectionAdapter<T>::operator==(
                     const IMMNetworkSection& rSection) const
{
    const ONetworkSectionAdapter<T>* pSectionAdapter =
                      (ONetworkSectionAdapter<T>*)  &rSection;
    if (pSectionAdapter != NULL)
    {
        ONetworkEdge<T>* pEdgeOther = pSectionAdapter->m_pEdge;
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

template<class T>
const ONetworkSectionAdapter<T>* 
ONetworkSectionAdapter<T>::CastToONetworkSection( void) const
{
    return this;
}

template<class T>
ONetworkSectionAdapter<T>* 
ONetworkSectionAdapter<T>::CastToONetworkSection(void)
{
    return this;
}

template<class T>
void ONetworkSectionAdapter<T>::PrintIdentifier(std::ostream& os) const
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

#endif /* __ONETWORKADAPTER_H_ */
