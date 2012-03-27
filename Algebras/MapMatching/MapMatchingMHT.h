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
#include "../Tools/Flob/DbArray.h"
#include <vector>

class Region;
class Point;
class Points;
class GPoint;
class MPoint;
class Geoid;
class UPoint;
class Network;

namespace datetime
{
  class DateTime;
}

namespace mapmatch {

class MHTRouteCandidate;

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
    MapMatchingMHT(Network* pNetwork,
                   std::string strFileName);
    MapMatchingMHT(Network* pNetwork,
                   DbArrayPtr<DbArray<MapMatchData> > pDbaMMData);

    ~MapMatchingMHT();

/*
3.2 Starts the map matching
     return true if successfull

*/
    bool DoMatch(MGPoint* pResMGPoint);

private:

/*
3.3 Private methods

*/
    void TripSegmentation(std::vector<DbArrayPtr<DbArray<MapMatchData> > >&
                                                                 rvecTripParts);

    int GetInitialRouteCandidates(const DbArray<MapMatchData>* pDbaMMData,
                    int nIdxFirstComponent,
                    std::vector<class MHTRouteCandidate*>& rvecRouteCandidates);

    int DevelopRoutes(const DbArray<MapMatchData>* pDbaMMData,
                    int nIndexFirstComponent,
                    std::vector<MHTRouteCandidate*>& rvecRouteCandidates);

    void DevelopRoutes(const MapMatchData& rMMData,
                       std::vector<MHTRouteCandidate*>& rvecRouteCandidates);

    void CheckData(DbArray<MapMatchData>* pDbaMMData);

    bool CheckQualityOfGPSFix(const MapMatchData& rMMData);

    enum ENextCandidates
    {
        CANDIDATES_NONE,
        CANDIDATES_UP,
        CANDIDATES_DOWN,
        CANDIDATES_UP_DOWN
    };
    bool AssignPoint(MHTRouteCandidate* pCandidate,
                     const MapMatchData& rMMData,
                     /*OUT*/ ENextCandidates& eNextCandidates);

    void ReduceRouteCandidates(std::vector<MHTRouteCandidate*>&
                                                           rvecRouteCandidates);

    bool CheckRouteCandidates(const std::vector<MHTRouteCandidate*>&
                                                           rvecRouteCandidates);

    bool CheckUTurn(const MHTRouteCandidate* pCandidate, bool bUpDown,
                    bool& rbCorrectUTurn, double& rdAdditionalUTurnScore);

    MHTRouteCandidate* DetermineBestRouteCandidate(
                          std::vector<MHTRouteCandidate*>& rvecRouteCandidates);

    void CreateCompleteRoute(
                      const std::vector<MHTRouteCandidate*>& rvecRouteSegments);

    void AddAdjacentSections(const MHTRouteCandidate* pCandidate,
                       bool bUpDown,
                       std::vector<MHTRouteCandidate*>& rvecNewRouteCandidates,
                       class SectionFilter* pFilter = NULL);

    void GetInitialSectionCandidates(const Point& rPoint,
                                     std::vector<NetworkSection>& rVecSectRes);

    void GetSectionsOfRoute(const class NetworkRoute& rNetworkRoute,
                            const Region& rRegion,
                            std::vector<NetworkSection>& rVecSectRes);

    void TraceRouteCandidates(const std::vector<MHTRouteCandidate*>&
                                                                 rvecCandidates,
                              const char* pszText) const;

/*
3.4 Private member

*/

};


class SectionFilter
{
public:
    SectionFilter(){}
    virtual ~SectionFilter(){}
    virtual bool IsValid(const DirectedNetworkSection& rSection) = 0;
};

} // end of namespace mapmatch

#endif /* __MAP_MATCHING_MHT_H__ */


