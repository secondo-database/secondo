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

[1] Header File of ONetworkEdge (Edge of Network based on ordered relation)

April, 2012. Matthias Roth

[TOC]

1 Overview

This header file contains the class ~ONetworkEdge~

2 Defines and includes

*/

#ifndef __ONETWORKEDGE_H_
#define __ONETWORKEDGE_H_

class Tuple;
class SimpleLine;


template<class T> class ONetwork;

#include <Point.h>

#include <stdio.h>
#include <string.h>
#include "FTextAlgebra.h"
#include "MapMatchingUtil.h"


/*
3 class ONetworkEdge
  Edge of Network based on ordered relation


The template parameter is the int-type used, e.g., CcInt or LongInt.

*/
template<class T>
class ONetworkEdge
{
public:
    ONetworkEdge();
    ONetworkEdge(Tuple* pTupleEdge,
                 const ONetwork<T>* pONetwork,
                 bool bIncReference = true);
    ONetworkEdge(const ONetworkEdge& rEdge);
    ~ONetworkEdge();

    ONetworkEdge& operator=(const ONetworkEdge& rEdge);

    bool operator==(const ONetworkEdge& rEdge) const;

    bool IsDefined(void) const {return m_pTupleEdge != NULL;}

    //int GetSourceId(void) const;

    //int GetTargetId(void) const;

    T* GetSource(void) const;

    T* GetTarget(void) const;

    Point GetSourcePoint(void) const;

    Point GetTargetPoint(void) const;

    SimpleLine* GetCurve(void) const;

    std::string GetRoadName(void) const;

    std::string GetRoadType(void) const;

    double GetMaxSpeed(void) const; // km/h

    T* GetWayId(void) const;

    double GetCurveLength(const double dScale) const;

    void Print(std::ostream& os) const;

    const Tuple* GetTuple(void) const {return m_pTupleEdge;}

private:

    Tuple* m_pTupleEdge;
    const ONetwork<T>* m_pONetwork;
    mutable double m_dCurveLength;
    mutable double m_dMaxSpeed;
};

/*
1 Implementation

*/
template<class T>
ONetworkEdge<T>::ONetworkEdge()
:m_pTupleEdge(NULL),
 m_pONetwork(NULL),
 m_dCurveLength(-1.0),
 m_dMaxSpeed(-1.0)
{
}

template<class T>
ONetworkEdge<T>::ONetworkEdge(Tuple* pTupleEdge,
                           const ONetwork<T>* pONetwork,
                           bool bIncReference)
:m_pTupleEdge(pTupleEdge),
 m_pONetwork(pONetwork),
 m_dCurveLength(-1.0),
 m_dMaxSpeed(-1.0)
{
    if (bIncReference && m_pTupleEdge != NULL)
        m_pTupleEdge->IncReference();
}

template<class T>
ONetworkEdge<T>::ONetworkEdge(const ONetworkEdge<T>& rEdge)
:m_pTupleEdge(rEdge.m_pTupleEdge),
 m_pONetwork(rEdge.m_pONetwork),
 m_dCurveLength(rEdge.m_dCurveLength),
 m_dMaxSpeed(rEdge.m_dMaxSpeed)
{
    if (m_pTupleEdge != NULL)
        m_pTupleEdge->IncReference();
}

template<class T>
ONetworkEdge<T>::~ONetworkEdge()
{
    if (m_pTupleEdge != NULL)
        m_pTupleEdge->DeleteIfAllowed();
    m_pTupleEdge = NULL;

    m_pONetwork = NULL;
}

template<class T>
ONetworkEdge<T>& ONetworkEdge<T>::operator=(const ONetworkEdge<T>& rEdge)
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

template<class T>
bool ONetworkEdge<T>::operator==(const ONetworkEdge<T>& rEdge) const
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

template<class T>
T* ONetworkEdge<T>::GetSource(void) const
{
    if (m_pTupleEdge != NULL && m_pONetwork != NULL)
    {
        const int nIdx = m_pONetwork->m_EdgeAttrIndexes.m_IdxSource;
        if (nIdx >= 0)
            return static_cast<T*>(m_pTupleEdge->GetAttribute(nIdx));
        else
            return NULL;
    }
    else
    {
        return NULL;
    }
}

template<class T>
T* ONetworkEdge<T>::GetTarget(void) const
{
    if (m_pTupleEdge != NULL)
    {
        const int nIdx = m_pONetwork->m_EdgeAttrIndexes.m_IdxTarget;
        if (nIdx >= 0)
            return static_cast<T*>(m_pTupleEdge->GetAttribute(nIdx));
        else
            return NULL;
    }
    else
    {
        return NULL;
    }
}

template<class T>
Point ONetworkEdge<T>::GetSourcePoint(void) const
{
    if (m_pTupleEdge != NULL && m_pONetwork != NULL)
    {
        const int nIdx = m_pONetwork->m_EdgeAttrIndexes.m_IdxSourcePos;
        if (nIdx >= 0)
        {
            Point* pPtSource = static_cast<Point*>(
                                              m_pTupleEdge->GetAttribute(nIdx));
            return pPtSource != NULL ? *pPtSource : Point(false);
        }
        else
            return Point(false);
    }
    else
    {
        return Point(false);
    }
}

template<class T>
Point ONetworkEdge<T>::GetTargetPoint(void) const
{
    if (m_pTupleEdge != NULL && m_pONetwork != NULL)
    {
        const int nIdx = m_pONetwork->m_EdgeAttrIndexes.m_IdxTargetPos;
        if (nIdx >= 0)
        {
            Point* pPtTarget = static_cast<Point*>(
                                              m_pTupleEdge->GetAttribute(nIdx));
            return pPtTarget != NULL ? *pPtTarget : Point(false);
        }
        else
        {
            return Point(false);
        }
    }
    else
    {
        return Point(false);
    }
}

template<class T>
SimpleLine* ONetworkEdge<T>::GetCurve(void) const
{
    if (m_pTupleEdge != NULL && m_pONetwork != NULL)
    {
        const int nIdx = m_pONetwork->m_EdgeAttrIndexes.m_IdxCurve;
        if (nIdx >= 0)
        {
            return static_cast<SimpleLine*>(m_pTupleEdge->GetAttribute(nIdx));
        }
        else
            return NULL;
    }
    else
    {
        return NULL;
    }
}

template<class T>
std::string ONetworkEdge<T>::GetRoadName(void) const
{
    if (m_pTupleEdge != NULL && m_pONetwork != NULL)
    {
        const int nIdx = m_pONetwork->m_EdgeAttrIndexes.m_IdxRoadName;
        if (nIdx >= 0)
        {
            FText* pRoadName = static_cast<FText*>(
                                              m_pTupleEdge->GetAttribute(nIdx));
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
    else
    {
        return "";
    }
}

template<class T>
std::string ONetworkEdge<T>::GetRoadType(void) const
{
    if (m_pTupleEdge != NULL && m_pONetwork != NULL)
    {
        const int nIdx = m_pONetwork->m_EdgeAttrIndexes.m_IdxRoadType;
        if (nIdx >= 0)
        {
            FText* pRoadType = static_cast<FText*>(
                                              m_pTupleEdge->GetAttribute(nIdx));
            if (pRoadType != NULL && pRoadType->IsDefined())
                return pRoadType->GetValue();
            else
                return "";
        }
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

template<class T>
double ONetworkEdge<T>::GetMaxSpeed(void) const
{
    /*
     * http://wiki.openstreetmap.org/wiki/Key:maxspeed
     *
     *  maxspeed=60       -> km/h
     *  maxspeed=50 mph   -> mph
     *  maxspeed=10 knots -> knots
     *
     */

    if (m_pTupleEdge != NULL && m_pONetwork != NULL && m_dMaxSpeed < 0.0)
    {
        const int nIdx = m_pONetwork->m_EdgeAttrIndexes.m_IdxMaxSpeed;
        if (nIdx >= 0)
        {
            FText* pMaxSpeed = static_cast<FText*>(
                                              m_pTupleEdge->GetAttribute(nIdx));
            if (pMaxSpeed != NULL && pMaxSpeed->IsDefined())
            {
                m_dMaxSpeed = convStrToDouble(pMaxSpeed->GetValue().c_str());

                // convert to km/h
                if (pMaxSpeed->GetValue().find("mph") != string::npos)
                {
                    m_dMaxSpeed *= 1.609344;
                }
                else if (pMaxSpeed->GetValue().find("knots") != string::npos)
                {
                    m_dMaxSpeed *= 1.852;
                }
            }
            else
                m_dMaxSpeed = 0.0;
        }
        else
            m_dMaxSpeed = 0.0;
    }

    return m_dMaxSpeed;
}

template<class T>
T* ONetworkEdge<T>::GetWayId(void) const
{
    if (m_pTupleEdge != NULL && m_pONetwork != NULL)
    {
        const int nIdx = m_pONetwork->m_EdgeAttrIndexes.m_IdxWayId;
        if (nIdx >= 0)
            return static_cast<T*>(m_pTupleEdge->GetAttribute(nIdx));
        else
            return NULL;
    }
    else
    {
        return NULL;
    }
}

template<class T>
double ONetworkEdge<T>::GetCurveLength(const double dScale) const
{
    if (m_dCurveLength < 0.)
    {
        m_dCurveLength = mapmatch::MMUtil::CalcLengthCurve(GetCurve(), dScale);
    }

    return m_dCurveLength;
}


template<class T>
void ONetworkEdge<T>::Print(std::ostream& os) const
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
    os << "WayId: " << GetWayId() << endl;
}




#endif /* __ONETWORKEDGE_H_ */
