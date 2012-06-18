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

April, 2012. Matthias Roth

[TOC]

1 Overview

This header file essentially contains the definition of the class ~MGPointCreator~.

2 Defines and includes

*/

#ifndef __MAPMATCHINGMHTMGPOINTCREATOR_H_
#define __MAPMATCHINGMHTMGPOINTCREATOR_H_

#include "MapMatchingMHT.h"
#include "MHTRouteCandidate.h"
#include <TemporalAlgebra.h>

class MGPoint;
struct RITreeP;
class GPoint;
class UGPoint;

namespace datetime
{
    class DateTime;
}


namespace mapmatch {

class NetworkAdapter;

/*
3 class MGPointCreator

*/
class MGPointCreator : public IMapMatchingMHTResultCreator
{
public:

    MGPointCreator(NetworkAdapter* pNetworkAdapter, MGPoint* pResMGPoint);
    virtual ~MGPointCreator();

    virtual bool CreateResult(const std::vector<MHTRouteCandidate*>&
                                                           rvecRouteCandidates);

private:

    bool Init(void);
    void Finalize(void);

    const GPoint* GetGPoint(const MHTRouteCandidate::PointDataPtr& pData,
                            const int& nNetworkId,
                            const double& dNetworkScale) const;

    void AddUGPoint(const UGPoint& rAktUGPoint);

    bool CalcShortestPath(const GPoint* pGPStart,
                          const GPoint* pGPEnd,
                          const datetime::DateTime& rtimeStart,
                          const datetime::DateTime& rtimeEnd,
                          const bool bCheckSpeed);

    bool ConnectPoints(const GPoint& rGPStart,
                       const GPoint& rGPEnd,
                       const Interval<Instant>& rTimeInterval);

    const Network* m_pNetwork;
    double   m_dNetworkScale;
    MGPoint* m_pResMGPoint;
    RITreeP* m_pRITree;
};


} // end of namespace mapmatch


#endif /* __MAPMATCHINGMHTMGPOINTCREATOR_H_ */
