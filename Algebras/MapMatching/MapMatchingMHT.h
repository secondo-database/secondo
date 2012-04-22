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

#include "MapMatchingUtil.h"
#include "MapMatchingData.h"
#include <vector>
#include <string>
#ifdef SECONDO_WIN32
#include <memory>
#else
#include <tr1/memory>
#endif
using std::tr1::shared_ptr;

#include "TemporalAlgebra.h"

class Point;
class MPoint;


namespace mapmatch {

class MHTRouteCandidate;
class IMMNetwork;
class IMMNetworkSection;

/*
3 class MapMatchingMHT
  Map matching algorithm based on the Multiple Hypothesis Technique (MHT)

*/
class MapMatchingMHT
{
public:
/*
3.1 Constructors and Destructor

*/
    MapMatchingMHT(IMMNetwork* pNetwork, MPoint* pMPoint);
    MapMatchingMHT(IMMNetwork* pNetwork,
                   std::string strFileName);
    MapMatchingMHT(IMMNetwork* pNetwork,
                   shared_ptr<MapMatchDataContainer> pContMMData);

    ~MapMatchingMHT();

/*
3.2 Starts map matching
    returns true if successfull

*/
    bool DoMatch(class IMapMatchingMHTResultCreator* pResCreator);


/*
3.3 accessing network

*/
    const IMMNetwork* GetNetwork(void) const {return m_pNetwork;}
    const double GetNetworkScale(void) const {return m_dNetworkScale;}

private:

/*
3.4 Private methods

*/
    bool InitMapMatching(class IMapMatchingMHTResultCreator* pResCreator);

    void TripSegmentation(std::vector<shared_ptr<MapMatchDataContainer> >&
                                                                 rvecTripParts);

    int GetInitialRouteCandidates(const MapMatchDataContainer* pContMMData,
                    int nIdxFirstComponent,
                    std::vector<class MHTRouteCandidate*>& rvecRouteCandidates);

    void GetInitialRouteCandidates(const Point& rPoint,
                          std::vector<MHTRouteCandidate*>& rvecRouteCandidates);

    int DevelopRoutes(const MapMatchDataContainer* pContMMData,
                      int nIndexFirstComponent,
                      std::vector<MHTRouteCandidate*>& rvecRouteCandidates);

    void DevelopRoutes(const MapMatchData* pMMData,
                       std::vector<MHTRouteCandidate*>& rvecRouteCandidates,
                       int nMaxLookAhead = 6);

    void CompleteData(MapMatchDataContainer* pContMMData);

    bool CheckQualityOfGPSFix(const MapMatchData& rMMData);

    enum ENextCandidates
    {
        CANDIDATES_NONE,
        CANDIDATES_UP,
        CANDIDATES_DOWN,
        CANDIDATES_UP_DOWN
    };
    bool AssignPoint(MHTRouteCandidate* pCandidate,
                     const MapMatchData* pMMData,
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
                       class ISectionFilter* pFilter = NULL);

    void TraceRouteCandidates(const std::vector<MHTRouteCandidate*>&
                                                                 rvecCandidates,
                              const char* pszText) const;

/*
3.5 Private member

*/

    IMMNetwork* m_pNetwork;
    double m_dNetworkScale;
    shared_ptr<MapMatchDataContainer> m_pContMMData;
    IMapMatchingMHTResultCreator* m_pResCreator;
};


/*
4 interface ISectionFilter

*/
class ISectionFilter
{
public:
    ISectionFilter(){}
    virtual ~ISectionFilter(){}
    virtual bool IsValid(const IMMNetworkSection* pSection) = 0;
};



/*
5 interface IMapMatchingMHTResultCreator

*/
class IMapMatchingMHTResultCreator
{
public:

    virtual ~IMapMatchingMHTResultCreator() {}

    virtual bool CreateResult(
                const std::vector<MHTRouteCandidate*>& rvecRouteCandidates) = 0;
};


} // end of namespace mapmatch

#endif /* __MAP_MATCHING_MHT_H__ */


