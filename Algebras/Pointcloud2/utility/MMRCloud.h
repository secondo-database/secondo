/*
----
This file is part of SECONDO.

Copyright (C) 2019,
Faculty of Mathematics and Computer Science,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



1 The MMRCloud class

Encapsulates a main memory representation of all points in a Pointcloud2,
using a vector of points and an MMRTree2 constructed over sequences of
points in the vector. Points are represented as DbScanPoints.

These conceptual sequences represent groups of leaves in the persistent RTree,
i.e. points that are geometrically close. These may however be scattered across
the entire SMI-File of the Cloud.
Therefore, this class uses a main memory vector that holds all points in
such sequences and a mapping index-array into the file that for each Point's
index in the vector, holds its corresponding Record-ID.
This allows us to save in the MMRTree not all Points, but rather only
the start of every sequence and thereby reduce build-up time and
memory consumption significantly.

*/

#include <vector>
#include <memory>
#include <limits>
#include <iostream>

#include "BitArray.h"
#include "PointBase.h"
#include "DbScanPoint.h"
#include "DbScan.h"
#include "../tcPointcloud2.h"

namespace pointcloud2 {

typedef RtreeT2<DIMENSIONS, PointIndex, MMRTREE_NODE_SIZE_MAX> MMCloudTreeT;


class MMRCloud {
    static bool REPORT_TO_CONSOLE;
    static bool REPORT_DETAILED;

public:
    /* The MMRTree constructed over the Cloud does not store individual
     * points, but rather the first point of a point sequence of neighboring
     * points. The size of a sequence is primarily determined by the number
     * of points stored in each node of the Pointcloud2's persistent RTree,
     * (theoretically, max. 76 points, usually about 70% of that).
     * If however maxSequenceLength is smaller, the points in the RTree node
     * will be refined into several sequences.
     * If maxSequenceLength is higher or -1,
     * the sequences will not be refined */
    MMRCloud(Pointcloud2* cloud, int maxSequenceLength);

    std::shared_ptr<std::vector<DbScanPoint<DIMENSIONS>>> _points;

    /* contains the _points indices of the first point of every point sequence
     * as retrieved from the persistent RTree */
    std::unique_ptr<MMCloudTreeT> _tree;

    /* the bounding box of all points in this Cloud */
    Rectangle<DIMENSIONS> _cloudBbox;

    /* Contains the SmiRecordId for every index in _points,
     * i.e. every Point in the Cloud. */
    std::vector<PointIndex> _indexOfSmiId;

    /* Holds "true" for every point in the Cloud which can still be assigned
     * to a new ObjId, or "false" for a point already assigned to an ObjID
     * in a previous call of analyzeGeom. */
    std::shared_ptr<BitArray> _bitMask;

    /* Estimates a typical distance between neighbor points in this cloud
     * by searching the nearest neighbors of (sampleSize) points and
     * calculating the median of the distances */
    double estimateTypicalPointDistance(const size_t sampleSize) const;
};
}
