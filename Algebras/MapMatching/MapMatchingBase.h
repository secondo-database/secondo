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

/*
3.1 Constructors and Destructor

*/
    MapMatchingBase(Network* pNetwork, MPoint* pMPoint);

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
                          const datetime::DateTime& rtimeEnd);

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
    MPoint* m_pMPoint;

private:
    MGPoint* m_pResMGPoint;
    RITreeP *m_pRITree;
};

} // end of namespace mapmatch

#endif /* __MAP_MATCHING_BASE_H__ */


