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

#ifndef MAPMATCHINGMHTORELCREATOR_H_
#define MAPMATCHINGMHTORELCREATOR_H_

#include "MapMatchingMHT.h"
#include "MHTRouteCandidate.h"
#include <vector>

class Tuple;
class TupleBuffer;
class TupleType;
class GenericRelationIterator;

namespace datetime
{
    class DateTime;
}
using datetime::DateTime;


namespace mapmatch {

/*
3 class OEdgeTupleStreamCreator

*/
class OEdgeTupleStreamCreator : public IMapMatchingMHTResultCreator
{
public:
    enum EMode
    {
        MODE_EDGES,
        MODE_EDGES_AND_POSITIONS
    };

    OEdgeTupleStreamCreator(Supplier s,
                            const class ONetworkAdapter& rNetw,
                            EMode eMode);
    virtual ~OEdgeTupleStreamCreator();

    virtual bool CreateResult(const std::vector<MHTRouteCandidate*>&
                                                           rvecRouteCandidates);

    Tuple* GetNextTuple(void) const;

private:

    void Init(void);

    const MHTRouteCandidate::PointData* GetFirstPointOfNextSegment(
              const std::vector<MHTRouteCandidate::RouteSegment*>& rvecSegments,
              size_t nIdx,
              double& dDistance);

    DateTime ProcessSegment(
                       const MHTRouteCandidate::RouteSegment& rSegment,
                       const DateTime& rEndTimePrevSegment,
                       const MHTRouteCandidate::PointData* pPrevPointData,
                       const MHTRouteCandidate::PointData* pFirstPointofNextSeg,
                       double dDistance); // Distance to first point
                                          // of next segment

    DateTime ProcessSegment_Edges(
                       const MHTRouteCandidate::RouteSegment& rSegment,
                       const DateTime& rEndTimePrevSegment,
                       const MHTRouteCandidate::PointData* pPrevPointData,
                       const MHTRouteCandidate::PointData* pFirstPointofNextSeg,
                       double dDistance); // Distance to first point
                                          // of next segment

    DateTime ProcessSegment_EdgesAndPositions(
                       const MHTRouteCandidate::RouteSegment& rSegment,
                       const DateTime& rEndTimePrevSegment,
                       const MHTRouteCandidate::PointData* pPrevPointData,
                       const MHTRouteCandidate::PointData* pFirstPointofNextSeg,
                       double dDistance); // Distance to first point
                                          // of next segment

    void ProcessPoints(const MHTRouteCandidate::RouteSegment& rSegment,
                       const DateTime& rTimeStart,
                       const DateTime& rTimeEnd,
                       const Point& rPtStart,
                       const Point& rPtEnd);

    const Tuple* GetEdgeTuple(const MHTRouteCandidate::RouteSegment& rSegment);

    void CreateTuple(const MHTRouteCandidate::RouteSegment& rSegment,
                     const DateTime& rTimeStart,
                     const DateTime& rTimeEnd);

    void CreateTuple(const MHTRouteCandidate::RouteSegment& rSegment,
                     const DateTime& rTimeStart,
                     const DateTime& rTimeEnd,
                     const Point& rPtStart,
                     const Point& rPtEnd,
                     const double dPosStart,
                     const double dPosEnd);

    EMode m_eMode;
    TupleType* m_pTupleType;
    TupleBuffer* m_pTupleBuffer;
    mutable GenericRelationIterator* m_pTupleIterator;
    Tuple* m_pTupleUndefEdge;
    double m_dNetworkScale;
};


} // end of namespace mapmatch


#endif /* MAPMATCHINGMHTORELCREATOR_H_ */
