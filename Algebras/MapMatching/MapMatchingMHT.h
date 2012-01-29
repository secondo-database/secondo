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

This header file essentially contains the definition of the class ~MapMatchingMHT~.

2 Defines and includes

*/
#ifndef __MAP_MATCHING_MHT_H__
#define __MAP_MATCHING_MHT_H__

#include "MapMatchingBase.h"
#include "NetworkSection.h"
#include <vector>

class Region;
class Point;
class Points;
class GPoint;
class MPoint;

namespace datetime
{
  class DateTime;
}

namespace mapmatch {

/*
3 class MapMatchingMHT
  Map matching algorithm based on the Multiple Hypothesis Technique (MHT)

*/
class MapMatchingMHT : public MapMatchingBase
{
public:
    /*
    3.1 Constructors and Destructor

    */
    MapMatchingMHT(Network* pNetwork, MPoint* pMPoint);
    ~MapMatchingMHT();

    /*
    3.2 Starts the map matching
        return true if successfull

    */
    bool DoMatch(MGPoint* pResMGPoint);

private:

    void TripSegmentation(std::vector<MPoint*>& rvecTripParts);

    void GetInitialRouteCandidates(MPoint* pMPoint,
                      std::vector<class RouteCandidate*>& rvecRouteCandidates);

    void DevelopRoutes(MPoint* pMPoint,
                       std::vector<RouteCandidate*>& rvecRouteCandidates);

    void DevelopRoutes(const Point& rPoint,
                       const datetime::DateTime& rTime,
                       bool bClosed,
                       std::vector<RouteCandidate*>& rvecRouteCandidates);

    void ReduceRouteCandidates(std::vector<RouteCandidate*>&
                                                           rvecRouteCandidates);

    RouteCandidate* DetermineBestRouteCandidate(
                       std::vector<RouteCandidate*>& rvecRouteCandidates);

    void CreateCompleteRoute(
                       const std::vector<RouteCandidate*>& rvecRouteSegments);

    void AddAdjacentSections(RouteCandidate* pCandidate,
                          std::vector<RouteCandidate*>& rvecNewRouteCandidates);

    void GetInitialSectionCandidates(const Point& rPoint,
                                     std::vector<NetworkSection>& rVecSectRes);

    void GetSectionsOfRoute(const class NetworkRoute& rNetworkRoute,
                            const Region& rRegion,
                            std::vector<NetworkSection>& rVecSectRes);
};

} // end of namespace mapmatch

#endif /* __MAP_MATCHING_MHT_H__ */


