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

This header file essentially contains the definition of the class ~MapMatchingSimple~.

2 Defines and includes

*/
#ifndef __MAP_MATCHING_SIMPLE_H__
#define __MAP_MATCHING_SIMPLE_H__

#include "MapMatchingBase.h"

class Region;
class Point;
class Points;
class GPoint;

namespace mapmatch {

/*
3 class MapMatchingSimple
  Simple Map matching algorithm - !! only for testing purpose !!

*/
class MapMatchingSimple : public MapMatchingBase
{
public:
    /*
    3.1 Constructors and Destructor

    */
    MapMatchingSimple(Network* pNetwork, MPoint* pMPoint);
    ~MapMatchingSimple();

    /*
    3.2 Starts the map matching
        return true if successfull

    */
    bool DoMatch(MGPoint* pResMGPoint);

private:

    GPoint ProcessPoint(const Point& rPoint);

    Point ProcessRoute(const class NetworkRoute& rNetworkRoute,
                       const Region& rRegion, const Point& rPt,
                       double& rdDistance);

    Point ProcessRouteSections(const int nRouteID, const Region& rRegion,
                               const Point& rPt, double& rdDistanceRes);

    // Debug
    void TracePoint(const Point& rPoint);
    Points* m_pTracePoints;
};

} // end of namespace mapmatch

#endif /* __MAP_MATCHING_SIMPLE_H__ */


