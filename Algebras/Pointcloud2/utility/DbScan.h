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



1 DbScan

*/

#pragma once
#include <memory>
#include "BitArray.h"
#include "PointBase.h"
#include "DbScanPoint.h"
#include "Rectangle2.h"
#include "MMRTree2.h"

namespace pointcloud2 {

constexpr size_t SEQUENCE_LENGTH_MAX = 8;

template<unsigned dim>
struct ClusterInfo {
    Rectangle2<dim> _bbox;
    unsigned _size;
    unsigned _cohesion;

    ClusterInfo() :
        _bbox(Rectangle2<dim>()),
        _size(0),
        _cohesion(0) {
    }

    double getNeighborAvg() {
        return (_size == 0) ? 0 : _cohesion / (double)_size;
    }
};

template<unsigned dim>
struct RefineClusterInfo {
    Rectangle2<dim> bbox;
    PointBase<dim> center;
};

template<unsigned dim>
class DbScan {

    static constexpr bool REPORT_TO_CONSOLE = true;
    static constexpr size_t REPORT_LINES_PER_CLUSTER = 0; // 10
    static constexpr int REPORT_CLUSTER_DETAILS_MAX = 10;
    static constexpr int REPORT_CLUSTER_SUMMARY_MAX = 100;
    static constexpr int REPORT_SHAPE_CENTERS_MAX = 100;

/*
1.1 Fields

*/
    // algorithm parameters
    double _eps;
    double _epsSquare; // stores _eps * _eps for performance reasons
    size_t _minPts;

    /* the number of points in the vector _points (1-based); note that this
     * number can be higher than the number of points considered by DbScan
     * if a filter is defined */
    size_t _pointCount;

    /* contains all points, possibly sorted in "RTree order" (i.e. points from
     * the same RTree node have neighbor index positions) */
    std::shared_ptr<std::vector<DbScanPoint<dim>>> _points;

    const bool _useSequences;

    /* unless it contains nullptr, this BitArray represents a filter for
     * the points, i.e. DbScan is to be performed only on points for which
     * BitArray.get(i) is true */
    std::shared_ptr<BitArray> _filter;

    /* A main memory RTree (MMRTree). Each MMRTree entry holds the
     * start index of a point sequence in _points. The end of a sequence
     * is marked by _isLastInSeq. Since MMRTree requires a lot of space
     * (110+ bytes per point), we cannot store every single point in the
     * MMRTree, but rather enter the bbox of small sequences of points.
     * In tcPointcloud2::getAllPoints(), these sequences are created using
     * the persistent RTree nodes of the Pointcloud2 (and further refined by
     * a temporary MMRTree). */
    std::unique_ptr<RtreeT2<dim, PointIndex, MMRTREE_NODE_SIZE_MAX>> _mmRTree;

    // statistical
    std::vector<ClusterInfo<dim>> _clusters;
    // std::vector<size_t> _clusterSizes;
    // std::vector<Rectangle2<dim>> _clusterBboxes;
    size_t _expandCount;
    size_t _distTestCount;

/*
1.2 Constructors

1.2.1 Empty Constructor

empty constructor solely for "template class DbScan[<]3[>];" in .cpp

*/
    DbScan() :
      _eps(0.0),
      _epsSquare(0.0),
      _minPts(0),
      _pointCount(0),
      _points(std::make_shared<std::vector<DbScanPoint<dim>>>()),
      _useSequences(false),
      _mmRTree(new RtreeT2<dim, PointIndex, MMRTREE_NODE_SIZE_MAX>()),
      _clusters(std::vector<ClusterInfo<dim>>()),
      _expandCount(0),
      _distTestCount(0) {
    }

public:
/*
1.2.2 Constructor for Sequences

Creates a DbScan instance for the points in the given 1-based points vector.
If useSequences is true, the points are expected to be grouped into sequences,
with \_isLastInSeq being true for the points at the end of the respective
sequence (e.g. points 1 .. 5 are a sequence if points-[>]at(5).\_isLastInSeq
is true).

*/
    DbScan(const double eps, const size_t minPts,
            std::shared_ptr<std::vector<DbScanPoint<dim>>> points,
            const bool useSequences);

/*
1.2.3 Constructor with Filter

Creates a DbScan instance for a selection of the points in the given
1-based points vector. Only the points for which "true" is set in the given
filter will be considered.

*/
    DbScan(const double eps, const size_t minPts,
            std::shared_ptr<std::vector<DbScanPoint<dim>>> points,
            std::shared_ptr<BitArray> filter);

    ~DbScan() = default;

/*
1.3 Run Methods

1.3.1 run

Performs DBSCAN, writing the result into the DbScanPoint::\_clusterIds.

*/
    void run();

/*
1.3.2 runRecursive

Recursively runs DBSCAN with shrinking epsilon values until the extent of
the clusters is below (maxBboxSize) in any dimension. Returns a vector
if the weighted centers of each cluster.

*/
    std::vector<PointBase<dim>> runRecursive(
            const size_t minClusterSize, const double maxBboxSize);

/*
1.4 Getters

1.4.1 getTotalPointCount

Returns the total number of points stored in the 1-based \_points
vector. Note that DbScan can be performed on a subset of these points
in which case only getUsedPointCount() points are actually scanned.

*/
    size_t getTotalPointCount() const {
        return _pointCount;
    }

/*
1.4.2 getUsedPointCount

Returns the number of points used in DbScan. If no filter is defined,
this is equal to the value returned by getTotalPointCount(); if,
however, a filter is used, this is equal to the number of "true" bits
in the filter.

*/
    size_t getUsedPointCount() const {
        return useFilter() ? _filter->getTrueCount() : _pointCount;
    }

/*
1.4.3 getClusterCount

Returns the number of clusters found during DBSCAN.

*/
    size_t getClusterCount() const {
        return _clusters.size() - 1; // entry [0] is used for noise
    }

/*
1.4.4 getNoiseCount

Returns the number of points identified as noise.

*/
    size_t getNoiseCount() const {
        return _clusters.empty() ? 0 : _clusters[0]._size;
    }

/*
1.4.5 getClusterSize

Returns the number of points assigned to the cluster with the given
cluster ID.

*/
    unsigned getClusterSize(size_t clusterId) const {
        assert(clusterId < _clusters.size());
        return _clusters[clusterId]._size;
    }

/*
1.4.6 getClusterBbox

Returns the bbox of the cluster with the given cluster ID.

*/
    const Rectangle2<dim>& getClusterBbox(size_t clusterId) const {
        // no bbox is stored for noise, so clusterId must be > 0
        assert(clusterId > 0 && clusterId < _clusters.size());
        return _clusters[clusterId]._bbox;
    }

/*
1.4.7 getClusterCohesion

Returns the total number of neighborhood relationships found in the cluster.
Compact, dense clusters will score higher on cohesion than thin clusters
of the same size.

*/
    unsigned getClusterCohesion(size_t clusterId) const {
        assert(clusterId < _clusters.size());
        return _clusters[clusterId]._cohesion;
    }

/*
1.4.8 getClusterId

Returns the cluster ID found for the point at the given 1-based index

*/
    int getClusterId(size_t pointIndex) const {
        return _points.get()->at(pointIndex)._clusterId;
    }

private:
/*
1.4.9 useFilter

Returns true if a filter is defined for the \_points.

*/
    bool useFilter() const { return _filter.get() != 0; }

/*
1.5 Private Methods

1.5.1 expandCluster

Expands a cluster from the given point (if this point is a core point)
and sets clusterId for all points in the cluster.

Returns the ClusterInfo, i.e. the number of points, the bbox, and the cohesion in
the cluster (i.e. in the points which are density-reachable from the given
point). If the given point is no core point, the returned ClusterInfo has
size 0.

*/
    ClusterInfo<dim> expandCluster(size_t index, int clusterID);

/*
1.5.2 getNeighborIndices

Finds the neighbors of the given point (i.e. the points within the \_eps range
from the given point) and returns a vector of their indices in \_allPoints.
The given point itself is not included.

*/
    std::vector<size_t> getNeighborIndices(const DbScanPoint<dim>& point);

/*
1.5.3 evaluateRun

Evaluates a run performed by the runRecursive() method by deciding,

  * which clusters to refuse (for being too small),

  * which clusters to use, by calculating their weighted centers and adding
    them to the given shapes vector (i.e. the result vector of runRecursive())

  * and which clusters to refine (for being larger than maxBboxSize).
    The caller can refine those clusters using another DBSCAN with a smaller
    epsilon. To prepare for that, \_points is reduced to the points
    in clusters that need refinement and information on those clusters is
    added to the given refineClusters vector.

*/
    void evaluateRun(std::vector<PointBase<dim>>& shapes,
            std::vector<RefineClusterInfo<dim>>& refineClusters,
            const size_t minClusterSize, const double maxBboxSize);

/*
1.5.4 addToShapes

Adds the given center of a cluster to the given vector of shapes, and
optionally reports the shape to the console.

*/
    void addToShapes(std::vector<PointBase<dim>>& shapes,
            const PointBase<dim>& center) const;


public:
/*
2 DbScan::iterator

Represents an iterator over the points on which DBSCAN should operate.

*/
    class iterator {
        /* an inner iterator over the filter; nullptr if no filter is used */
        std::unique_ptr<BitArray::iterator> _filterIter;
        /* the total number of points. If a filter is used, the iterator will
         * iterate over a selection of these points only. */
        size_t _pointCount;
        /* the 1-based index that was last returned by next() */
        size_t _index;

    public:
/*
2.1 constructors

2.1.1 constructor with filter

Instantiates a new iterator which returns only the point indices for which
the respective bit in the given filter is "true". If filter is nullptr,
the iterator returns all point indices from 1 to pointCount.

*/
        iterator(size_t pointCount, std::shared_ptr<BitArray> filter) {
            _filterIter = filter.get() ? filter.get()->getIterator() :
                    std::unique_ptr<BitArray::iterator>(nullptr);
            _pointCount = pointCount;
            _index = 0;
        }

/*
2.1.2 constructor without filter

Instantiates a new iterator which returns all point indices from 1 to the
given pointCount.

*/
        iterator(size_t pointCount) {
            _filterIter = std::unique_ptr<BitArray::iterator>(nullptr);
            _pointCount = pointCount;
            _index = 0;
        }

        ~iterator() = default;

/*
2.2 next() function

Returns the next point index. If a filter is used, the next "true" bit in
the filter is searched its index position returned (0-based or 1-based,
depending the constructor parameter of BitArray). Returns -1 if no more
point indices exist.

*/
        int next() {
            if (_filterIter)
                return _filterIter.get()->next();
            else if (_index < _pointCount)
                return ++_index;
            else
                return -1;
        }
    };

public:
/*
2.3 Iterator getters is DbScan

2.3.1 getIterator

Returns a new iterator which returns only the point indices for which
the respective bit in the given filter is "true". If filter is nullptr,
the iterator returns all point indices from 1 to pointCount.

*/
    std::shared_ptr<iterator> getIterator() {
        return std::make_shared<DbScan::iterator>(_pointCount, _filter);
    }

/*
2.3.2 getStandardIterator

Returns a new iterator which returns all point indices from 1 to the
given pointCount.

*/
    static std::shared_ptr<iterator> getStandardIterator(size_t pointCount) {
        return std::make_shared<DbScan::iterator>(pointCount);
    }

/*
3 Static functions

3.1 constructMMRTree

Constructs the given MMRTree. If useSequences = true, the entries of the
MMRTree will contain the start index of each point sequence in the given
1-based points vector; for useSequences = false, there will be an MMRTree
entry for every point index returned by the given iterator.

*/
    static void constructMMRTree(
            std::unique_ptr<RtreeT2<dim, PointIndex, MMRTREE_NODE_SIZE_MAX>>&
                    mmRTree,
            const std::vector<DbScanPoint<dim>>& points,
            const bool useSequences, std::shared_ptr<DbScan::iterator> iter);

};

} // namespace
