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

[1] Header File of the MapMatching Algebra

January-April, 2012. Matthias Roth

[TOC]

1 Overview

This header file contains utilities for map matching

2 Defines and includes

*/
#ifndef __MAP_MATCHING_UTILITY_H__
#define __MAP_MATCHING_UTILITY_H__

#include <stdio.h>
class Region;
class SimpleLine;
class HalfSegment;
class Point;

namespace mapmatch {

/*
3 ~MMUtil~

*/
class MMUtil
{
public:
    static bool Intersects(const Region& rRegion, const SimpleLine& rSLine);

    static double CalcOrthogonalProjection(const HalfSegment& rHalfSegment,
                                           const Point& rPt, Point& rPtRes);

    static Point CalcOrthogonalProjection(const SimpleLine& rLine,
                                          const Point& rPt,
                                          double& rdDistanceRes);

};


/*
4 ~AttributePtr~
  Helper class for managing Attribute pointers

*/
template<class Type>
class AttributePtr
{
public:
    typedef AttributePtr<Type> _Myt;

    AttributePtr(Type* pA, bool bIncReference = false)
    :m_pA(pA)
    {
        if (bIncReference && m_pA != NULL)
            m_pA = dynamic_cast<Type*>(m_pA->Copy());
    }

    AttributePtr(const _Myt& rAttributePtr)
    :m_pA(rAttributePtr.m_pA)
    {
        if (m_pA != NULL)
            m_pA = dynamic_cast<Type*>(m_pA->Copy());
    }

    ~AttributePtr()
    {
        if (m_pA != NULL)
        {
            m_pA->DeleteIfAllowed();
            m_pA = NULL;
        }
    }

    const AttributePtr& operator=(const _Myt& rAttributePtr)
    {
        if (&rAttributePtr != this)
        {
            if (m_pA != NULL)
            {
                m_pA->DeleteIfAllowed();
                m_pA = NULL;
            }

            m_pA = rAttributePtr.m_pA;
            if (m_pA != NULL)
                m_pA = dynamic_cast<Type*>(m_pA->Copy());
        }
        return *this;
    }

    void reset(Type* pA)
    {
        if (pA != m_pA)
        {
            if (m_pA != NULL)
            {
                m_pA->DeleteIfAllowed();
                m_pA = NULL;
            }

            m_pA = pA;
            if (m_pA != NULL)
                m_pA = dynamic_cast<Type*>(m_pA->Copy());
        }
    }

    Type& operator*() const
    {
        return *m_pA;
    }

    Type* operator->() const
    {
        return m_pA;
    }

    operator bool() const
    {
        return (m_pA != NULL);
    }

    Type* get(void) const
    {
        return m_pA;
    }

private:
    Type* m_pA;
};



} // end of namespace mapmatch

#endif /* __MAP_MATCHING_UTILITY_H__ */


