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

This header file essentially contains the definition of the class ~MapMatchingBase~.

2 Defines and includes

*/
#ifndef __MAP_MATCHING_BASE_H__
#define __MAP_MATCHING_BASE_H__

#include "TemporalAlgebra.h"
#include "MapMatchingUtil.h"

class Network;
class MPoint;
class GPoint;
class UGPoint;
class MGPoint;
struct RITreeP;

namespace datetime
{
  class DateTime;
}


namespace mapmatch {

/*
3 class MapMatchingBase
It is the base class of all Map Matching algorithms

*/

class MapMatchingBase
{
public:

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

        MapMatchData(MapMatchData& rData)
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

        void Print(ostream& os)
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
        double m_dSpeed;
    };

/*
3.1 Constructors and Destructor

*/
    MapMatchingBase(Network* pNetwork,
                    const MPoint* pMPoint);

    MapMatchingBase(Network* pNetwork,
                    DbArrayPtr<DbArray<MapMatchData> > pDbaMMData);

    ~MapMatchingBase();

protected:

/*
3.2 Initializing and Finalizing of the map matching algorithm

*/
    bool InitMapMatching(MGPoint* pResMGPoint);
    void FinalizeMapMatching(void);

/*
   3.3 Adds a new UGPoint to the result-MGPoint

*/
    void AddUGPoint(const UGPoint& rAktUGPoint);
    void AddUnit(const int nRouteID, const double dPos1, const double dPos2);

/*
   3.4 ShortestPath calculation
   creates and adds ~UGPoints~ to result

*/
    bool CalcShortestPath(const GPoint* pGPStart, const GPoint* pGPEnd,
                          const datetime::DateTime& rtimeStart,
                          const datetime::DateTime& rtimeEnd,
                          const bool bCheckSpeed);

/*
   3.4 ConnectPoints
   Connects two GPoints. Similar shortest path.
   ShortestPath is only used if points lie on different
   routes that are not directly connected.
   creates and adds ~UGPoints~ to result

*/
    bool ConnectPoints(const GPoint& rGPStart,
                       const GPoint& rGPEnd,
                       const Interval<Instant>& rTimeInterval);
/*
   3.5 protected member

*/

    Network* m_pNetwork;
    double m_dNetworkScale;

    DbArrayPtr<DbArray<MapMatchData> > m_pDbaMMData;
    void SetMMData(DbArrayPtr<DbArray<MapMatchData> > pDbaMMData)
                                                    {m_pDbaMMData = pDbaMMData;}


private:
    MGPoint* m_pResMGPoint;
    RITreeP *m_pRITree;
};

} // end of namespace mapmatch

#endif /* __MAP_MATCHING_BASE_H__ */


