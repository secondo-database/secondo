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

[1] Header File containing struct ~MapMatchData~
    and class ~MapMatchDataContainer~

April, 2012. Matthias Roth

[TOC]

1 Overview

This header file contains the struct ~MapMatchData~
and the class ~MapMatchDataContainer~

2 Defines and includes

*/

#include "MapMatchingData.h"


namespace mapmatch {

/*
3 class ~MapMatchDataContainer~
  Container for MapMatchData

*/

MapMatchDataContainer::MapMatchDataContainer()
{
}

MapMatchDataContainer::MapMatchDataContainer(const MapMatchDataContainer& rCont)
{
    const size_t nSize = rCont.m_vecData.size();
    for (size_t i = 0; i < nSize; ++i)
    {
        MapMatchData* pData = rCont.m_vecData[i];
        if (pData != NULL)
            m_vecData.push_back(new MapMatchData(*pData));
    }
}

MapMatchDataContainer::~MapMatchDataContainer()
{
    const size_t nSize = m_vecData.size();
    for (size_t i = 0; i < nSize; ++i)
    {
        MapMatchData* pData = m_vecData[i];
        delete pData;
    }
    m_vecData.clear();
}

MapMatchDataContainer& MapMatchDataContainer::operator=(
                                             const MapMatchDataContainer& rCont)
{
    if (&rCont != this)
    {
        size_t nSize = m_vecData.size();
        for (size_t i = 0; i < nSize; ++i)
        {
            MapMatchData* pData = m_vecData[i];
            delete pData;
        }
        m_vecData.clear();

        nSize = rCont.m_vecData.size();
        for (size_t i = 0; i < nSize; ++i)
        {
            MapMatchData* pData = rCont.m_vecData[i];
            if (pData != NULL)
                m_vecData.push_back(new MapMatchData(*pData));
        }
    }

    return *this;
}

} // end of namespace mapmatch

