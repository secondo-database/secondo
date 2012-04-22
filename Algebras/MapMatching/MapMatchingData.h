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

#ifndef __MAPMATCHINGDATA_H_
#define __MAPMATCHINGDATA_H_

#include <Point.h>
#include <vector>
//#include "../Tools/Flob/DbArray.h"


namespace mapmatch {


/*
3 ~MapMatchData~
  Input data for map matching (GPS-data)

*/
struct MapMatchData
{
    MapMatchData()
    // Do not initialize data !!
    // because this struct is used in DbArray -> Get-method
    //:m_dLat(0.0), m_dLon(0.0), m_Time(0), m_nFix(-1), m_nSat(-1),
    //m_dHdop(-1.0), m_dVdop(-1.0), m_dPdop(-1.0),
    //m_dCourse(-1.0), m_dSpeed(-1.0)
    {

    }

    MapMatchData(const MapMatchData& rData)
    :m_dLat(rData.m_dLat), m_dLon(rData.m_dLon), m_Time(rData.m_Time),
     m_nFix(rData.m_nFix), m_nSat(rData.m_nSat), m_dHdop(rData.m_dHdop),
     m_dVdop(rData.m_dVdop), m_dPdop(rData.m_dPdop),
     m_dCourse(rData.m_dCourse), m_dSpeed(rData.m_dSpeed)
    {
    }

    MapMatchData(double dLat, double dLon, int64_t Time,
                 int nFix = -1, int nSat = -1, double dHdop = -1.0,
                 double dVdop = -1.0, double dPdop = -1.0,
                 double dCourse = -1.0, double dSpeed = -1.0)
    :m_dLat(dLat), m_dLon(dLon), m_Time(Time),
     m_nFix(nFix), m_nSat(nSat), m_dHdop(dHdop),
     m_dVdop(dVdop), m_dPdop(dPdop),
     m_dCourse(dCourse), m_dSpeed(dSpeed)
    {
    }

    ~MapMatchData()
    {
    }

    inline Point GetPoint(void) const
    {
        return Point(true, m_dLon, m_dLat);
    }

    void Print(ostream& os) const
    {
        os << "Lat: " << m_dLat << ";" << "Lon: " << m_dLon << "; ";
        os << "Time: " << m_Time << ";" << "Fix: " << m_nFix << ";";
        os << "Sat: " << m_nSat << ";" << "Hdop: " << m_dHdop << ";";
        os << "Vdop: " << m_dVdop << ";" << "PDop: " << m_dPdop << ";";
        os << "Course: " << m_dCourse << ";" << "Speed: " << m_dSpeed;
    }

    double m_dLat;
    double m_dLon;
    int64_t m_Time;
    int m_nFix;
    int m_nSat;
    double m_dHdop;
    double m_dVdop;
    double m_dPdop;
    double m_dCourse;
    double m_dSpeed; // m/s
};


/*
4 class ~MapMatchDataContainer~
  Container for MapMatchData

*/
class MapMatchDataContainer
{
public:
    MapMatchDataContainer();
    MapMatchDataContainer(const MapMatchDataContainer& rCont);
    ~MapMatchDataContainer();

    MapMatchDataContainer& operator=(const MapMatchDataContainer& rCont);

    inline void Append(const MapMatchData& rData)
    {
        m_vecData.push_back(new MapMatchData(rData));
    }

    inline void Put(size_t nIdx, MapMatchData& rData)
    {
        MapMatchData* pData = m_vecData[nIdx];
        delete pData;
        m_vecData[nIdx] = new MapMatchData(rData);
    }

    inline size_t Size(void) const
    {
        return m_vecData.size();
    }

    inline const MapMatchData* Get(size_t nIdx) const
    {
        return m_vecData[nIdx];
    }

private:
    //DbArray<MapMatchData> m_arrData;
    std::vector<MapMatchData*> m_vecData;
};


} // end of namespace mapmatch

#endif /* __MAPMATCHINGDATA_H_ */
