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

#include "DbScan.h"

#include <limits>

#include "MathUtils.h"

using namespace pointcloud2;
using namespace std;

/*
1.2 Constructors

1.2.2 Constructor for Sequences

Creates a DbScan instance for the points in the given 1-based points vector.
If useSequences is true, the points are expected to be grouped into sequences,
with \_isLastInSeq being true for the points at the end of the respective
sequence (e.g. points 1 .. 5 are a sequence if points-[>]at(5).\_isLastInSeq
is true).

*/
template<unsigned dim>
DbScan<dim>::DbScan(const double eps, const size_t minPts,
        std::shared_ptr<std::vector<DbScanPoint<dim>>> points,
        const bool useSequences) :
        _eps(eps),
        _epsSquare(eps * eps),
        _minPts(minPts),

        _pointCount(points.get()->size() - 1),
        _points(points),
        _useSequences(useSequences),
        _filter(std::shared_ptr<BitArray>(nullptr)),
        _mmRTree(new RtreeT2<dim, PointIndex, MMRTREE_NODE_SIZE_MAX>()),

        // statistics
        _clusters(std::vector<ClusterInfo<dim>>()),
        _expandCount(0),
        _distTestCount(0) {
}

/*
1.2.3 Constructor with Filter

Creates a DbScan instance for a selection of the points in the given
1-based points vector. Only the points for which "true" is set in the given
filter will be considered.

*/
template<unsigned dim>
DbScan<dim>::DbScan(const double eps, const size_t minPts,
        std::shared_ptr<std::vector<DbScanPoint<dim>>> points,
        std::shared_ptr<BitArray> filter) :
        _eps(eps),
        _epsSquare(eps * eps),
        _minPts(minPts),

        _pointCount(points.get()->size() - 1),
        _points(points),
        _useSequences(filter.get() == 0),
        _filter(filter),
        _mmRTree(new RtreeT2<dim, PointIndex, MMRTREE_NODE_SIZE_MAX>()),

        // statistics
        _clusters(std::vector<ClusterInfo<dim>>()),
        _expandCount(0),
        _distTestCount(0) {
}

/*
1.3 Run Methods

1.3.1 run

Performs DBSCAN, writing the result into the DbScanPoint::\_clusterIds.

*/
template<unsigned dim>
void DbScan<dim>::run() {
    // construct an MMRTree to point to the start index and point count
    // of each point sequence in allPoints

    constructMMRTree(_mmRTree, *(_points.get()), _useSequences,
            getIterator());

    if (REPORT_TO_CONSOLE) {
        // _mmRTree->printStats(_pointCount, sizeof(DbScanPoint<dim>));
    }

    // a range query in the MMRTree will now return the start index of
    // a point sequence in allPoints. isLastInSeq marks the end of the sequence

    // ------------------------------------------
    // start DBSCAN

    // create entry for noise (at index 0)
    _clusters.clear();
    _clusters.push_back(ClusterInfo<dim>()); // noise count, will be set below

    size_t noiseCount = getUsedPointCount();
    auto iter = getIterator();
    int i;
    while ((i = iter.get()->next()) > 0) {
        DbScanPoint<dim>& point = _points.get()->at(i);
        if ( point._clusterId != SCAN_UNCLASSIFIED )
            continue;

        size_t clusterID = _clusters.size(); // get next free ID
        ClusterInfo<dim> clusterInfo = expandCluster(i, clusterID);
        if ( clusterInfo._size == 0 ) {
            // the given point is noise or a border point (but no core)
            point._clusterId = SCAN_NOISE; // may be changed later
            continue;
        }

        // report the size of the identified cluster
        if (REPORT_TO_CONSOLE && clusterID <= REPORT_CLUSTER_SUMMARY_MAX) {
            cout << "=> Cluster " << clusterID << ": "
                    << formatInt(clusterInfo._size) << " points"
                    << ", cohesion " << formatInt(clusterInfo._cohesion)
                    << " (avg " << clusterInfo.getNeighborAvg()
                    << "), bbox ";
            clusterInfo._bbox.Print(cout); // also prints endl
            if (REPORT_LINES_PER_CLUSTER > 0 &&
                    clusterID <= REPORT_CLUSTER_DETAILS_MAX) {
                cout << endl;
            }
        }

        _clusters.push_back(clusterInfo);
        noiseCount -= clusterInfo._size;
    }
    _clusters[0]._size = noiseCount;

    if (REPORT_TO_CONSOLE) {
        if (REPORT_LINES_PER_CLUSTER > 0) {
            // report noise points
            size_t outCount = 0;
            auto iter = getIterator();
            int i;
            while ((i = iter.get()->next()) > 0) {
                DbScanPoint<dim>& point = _points.get()->at(i);
                if (point._clusterId != SCAN_NOISE)
                    continue;
                cout << point.toString() << endl;
                ++outCount;
                if (outCount >= REPORT_LINES_PER_CLUSTER)
                    break;
            }
        }

        // report summary
        std::cout << "=> Noise: " << formatInt(getNoiseCount()) << " points"
                << endl;
        if (REPORT_LINES_PER_CLUSTER > 0)
            std::cout << endl;
        std::cout << "   " << formatInt(getClusterCount()) << " clusters "
                "found in " << formatInt(_expandCount) <<
                " expand cluster calls" << endl;
        std::cout << "   Distance calculations: " << formatInt(_distTestCount)
                << " = ";
        double n = getUsedPointCount();
        std::cout << _distTestCount / (n * std::log(n)) << " * (n log n) = ";
        std::cout << _distTestCount / (n * n) << " * (n ^ 2)" << endl << endl;
    }
}

/*
1.3.2 runRecursive

Recursively runs DBSCAN with shrinking epsilon values until the extent of
the clusters is below (maxBboxSize) in any dimension. Returns a vector
if the weighted centers of each cluster.

*/
template<unsigned dim>
std::vector<PointBase<dim>> DbScan<dim>::runRecursive(
        const size_t minClusterSize, const double maxBboxSize) {

    assert (_filter.get() == 0);

    vector<PointBase<dim>> resultingShapes;

    // Information on clusters that are still to be refined is stored in
    // "refineClusters". If no part of such a cluster is at some point added
    // to the resultingShapes (because at stricter parameters, the cluster
    // completely dissolves), the center of the original cluster will be
    // used (despite the fact that this cluster is too big)
    std::vector<RefineClusterInfo<dim>> refineClusters;

    size_t iteration = 0;
    do {
        if (REPORT_TO_CONSOLE) {
            cout << endl << "Running DbScan (iteration " << ++iteration << ") "
                    "with " << formatInt(_pointCount) << " points: "
                    "eps = " << _eps << ", minPts = " << formatInt(_minPts)
                    << endl;
        }

        // start DbScan
        run();

        // evaluate the run, adding entries to "centers" and reducing
        // _points and _pointCount to the points that require another run:
        evaluateRun(resultingShapes, refineClusters,
                minClusterSize, maxBboxSize);

        // now raise the requirements for clusters in order to get smaller
        // clusters for the remaining points
        _eps *= 0.5;
        _epsSquare = _eps * _eps;
        // _minPts should not be changed

        // exit loop if no more runs are required
        if (_pointCount <= _minPts)
            break;

        // clear MMRTree, cluster information and statistics for the next run
        _mmRTree.reset(new RtreeT2<dim, PointIndex, MMRTREE_NODE_SIZE_MAX>());
        _clusters.clear();
        _expandCount = 0;
        _distTestCount = 0;
    } while (_pointCount > _minPts);


    // for clusters that dissolved in refinement, the center of the original,
    // unrefined cluster will be used. If a cluster did not dissolve (but
    // rather a refined cluster was added to the resultingShapes), the
    // respective entry in refineClusters was already removed
    for (RefineClusterInfo<dim>& cluster : refineClusters)
        addToShapes(resultingShapes, cluster.center);

    if (REPORT_TO_CONSOLE) {
        cout << endl << formatInt(resultingShapes.size()) << " shapes found "
                "in " << formatInt(iteration) << " iterations."
                << endl << endl;
    }

    return resultingShapes;
}



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
template <unsigned dim>
ClusterInfo<dim> DbScan<dim>::expandCluster(size_t index, int clusterID) {
    ++_expandCount;

    ClusterInfo<dim> clusterInfo { };
    std::vector<size_t> seedIndices { index };
    size_t i = 0;
    do {
        // get the current seed point and its neighbors
        DbScanPoint<dim>& seed = _points.get()->at(seedIndices[i]);
        std::vector<size_t> neighbors = getNeighborIndices(seed);

        // if the current seed is not a core point, ignore it
        // (if it is the first point, no cluster was found, i.e. count == 0)
        if ( neighbors.size() < _minPts )
            continue;

        // if seed is the first point, a cluster was found
        if (clusterInfo._size == 0) {
            seed._clusterId = clusterID;
            clusterInfo._bbox = seed.getBoundingBox2(0.0);
            ++clusterInfo._size;

            // DEBUG: report seed point
            if (REPORT_TO_CONSOLE
                    && clusterInfo._size <= REPORT_LINES_PER_CLUSTER
                    && clusterID <= REPORT_CLUSTER_DETAILS_MAX)
                cout << seed.toString() << endl;
        }

        // seed is a core point; check whether its neighbor points
        // are yet unclassified or were previously classified as noise
        for (auto j = neighbors.begin(); j != neighbors.end(); ++j) {
            DbScanPoint<dim>& neighbor = _points.get()->at(*j);

            if ( neighbor._clusterId == SCAN_UNCLASSIFIED
              || neighbor._clusterId == SCAN_NOISE) {

                if (neighbor._clusterId == SCAN_UNCLASSIFIED)
                    seedIndices.push_back(*j);
                neighbor._clusterId = clusterID;
                ++clusterInfo._size;
                clusterInfo._bbox.Extend(neighbor.getBoundingBox2(0.0));

                // DEBUG: report neighbor point
                if (REPORT_TO_CONSOLE
                        && clusterInfo._size <= REPORT_LINES_PER_CLUSTER
                        && clusterID <= REPORT_CLUSTER_DETAILS_MAX)
                    cout << neighbor.toString() << endl;
            }
        }
        clusterInfo._cohesion += neighbors.size();
    } while (++i < seedIndices.size());

    return clusterInfo;
    // also returns count which is 0, if the given point is no seed (but
    // noise); otherwise, (count) points are density-connected with the given
    // core. bbox is the bounding box of all points in the cluster
}

/*
1.5.2 getNeighborIndices

Finds the neighbors of the given point (i.e. the points within the \_eps range
from the given point) and returns a vector of their indices in \_allPoints.
The given point itself is not included.

*/
template <unsigned dim>
std::vector<size_t> DbScan<dim>::getNeighborIndices(
        const DbScanPoint<dim>& point) {
    std::vector<size_t> neighborIndices;

    // iterate over all intersecting entries in the MMRTree
    Rectangle2<dim> searchBox = point.getBoundingBox2(_eps);
    auto it = _mmRTree->find(searchBox);
    const PointIndex *startIndex;
    while( (startIndex = it->next()) != 0) {
        PointIndex index = *startIndex;
        // test points in the sequence starting from startIndex; the last
        // point in the sequence is marked with _isLastInSeq.
        bool sequenceContinues;
        do {
            assert (index <= _pointCount); // _points is 1-based
            DbScanPoint<dim>& target = _points.get()->at(index);
            if (&point != &target) { // do not enter the point itself
                ++_distTestCount; // statistical
                if (point.calculateDistanceSquare(target) <= _epsSquare)
                    neighborIndices.push_back(index);
            }
            sequenceContinues = _useSequences && !target._isLastInSeq;
            ++index;
        } while (sequenceContinues);
    }
    delete it;

    /* previously, the simple O(n^2) method tested all points:
    for(size_t i = 1; i <= _pointCount; ++i) { // _allPoints is 1-based
        DbScanPoint<dim>* targetPtr = _allPoints.get() + i;
        if (&point == targetPtr) // do not enter the point itself
            continue;
        if (point.calculateDistanceSquare(targetPtr) <= _epsSquare)
            neighborIndices.push_back(i);
    }
    */

    return neighborIndices;
}

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
template<unsigned dim>
void DbScan<dim>::evaluateRun(std::vector<PointBase<dim>>& shapes,
        std::vector<RefineClusterInfo<dim>>& refineClusters,
        const size_t minClusterSize, const double maxBboxSize) {

    const int REFUSE_CLUSTER = -1; // must be a negative value

    size_t clusterCount = getClusterCount();

    BitArray refine = BitArray(clusterCount, true); // 1-based
    refine.initializeAll(false);

    // First we have to decide,
    // - which clusters to refuse (for being too small),
    // - which to refine (for being too big),
    // - and which to use by adding an entry to "centers"
    // for the clusters that are refined or used, the weighted center is
    // calculated.

    // map clusters to a 0-based index position in "centers"
    size_t centerCount = 0;
    vector<int> centerIndex = vector<int>(1 + clusterCount, REFUSE_CLUSTER);

    // iterate over clusters
    for (size_t i = 1; i <= clusterCount; ++i) { // 1-based
        ClusterInfo<dim>& clusterInfo = _clusters[i];
        // is this cluster too small?
        if (clusterInfo._size < minClusterSize) {
            centerIndex[i] = REFUSE_CLUSTER;
            continue;
        }

        // get the maximum extent of this cluster's bbox
        double maxExtent = 0.0;
        Rectangle2<dim>& bbox = clusterInfo._bbox;
        for (size_t d = 0; d < dim; ++d)
            maxExtent = MAX(maxExtent, bbox.MaxD(d) - bbox.MinD(d));

        // does this cluster need to be refined?
        if (maxExtent > maxBboxSize)
            refine.set(i, true);

        // use centerIndex[i] to refer to an entry in centers
        // (which is yet to be created)
        centerIndex[i] = centerCount++;
    }

    // create a vector for the clusters' centers (except for those clusters
    // that will be refused
    PointBase<dim> empty;
    empty.clear();
    vector<PointBase<dim>> centers = vector<PointBase<dim>>(
            centerCount, empty);

    // iterate over points and depending a point's cluster
    // - ignore point if it is noise or its cluster is refused
    // - copy it to beginning of _points and calculate its weighted center
    //   if its cluster needs refinement
    // - also calculate the weighted center if cluster can be used
    size_t writeIndex = 0;
    // no getIterator() needed here since runRecursive() allows no filter
    for (size_t readIndex = 1; readIndex <= _pointCount; ++readIndex) {
        DbScanPoint<dim>& point = _points.get()->at(readIndex);
        // if the point belongs to noise, ignore it
        int clusterId = point._clusterId;
        assert (clusterId <= clusterCount);
        if (clusterId == SCAN_NOISE)
            continue;

        // if the point's cluster is too small, ignore it
        int centerInd = centerIndex[clusterId];
        if (centerInd == REFUSE_CLUSTER)
            continue;

        // calculate the center of the clusters that will be used or refined
        PointBase<dim>& center = centers[centerInd];
        for (int i = 0; i < dim; ++i)
            center._coords[i] += point._coords[i];

        // if the point's cluster needs refinement, copy it and reset the
        // destination point's _clusterId for the next run
        if (refine.get(clusterId)) {
            ++writeIndex; // _points is 1-based, so start writing at entry 1
            DbScanPoint<dim>& dest = _points.get()->at(writeIndex);
            if (writeIndex < readIndex)  // otherwise, writeIndex == readIndex
                dest.copyCoordsFrom(_points.get()->at(readIndex));
            dest._clusterId = SCAN_UNCLASSIFIED;
            dest._isLastInSeq = true;
        }
    }

    // the newly added entries in "centers" now contain the sum of the
    // coordinates of all points in the respective cluster. They are now
    // divided by the number of points in the cluster to get the weighted
    // center of the cluster
    size_t refuseCount = 0;
    size_t refineCount = 0;
    size_t useCount = 0;
    for (size_t clusterId = 1; clusterId <= clusterCount; ++clusterId) {
        int centerInd = centerIndex[clusterId];
        if (centerInd == REFUSE_CLUSTER) {
            ++refuseCount;
            continue;
        }
        PointBase<dim>& center = centers[centerInd];
        size_t clusterSize = _clusters[clusterId]._size;
        for (int i = 0; i < dim; ++i)
            center._coords[i] /= static_cast<double>(clusterSize);

        // since this center will either be added to shapes or is guaranteed
        // to be considered in later iterations, remove possible entries
        // from the refineClusters vector
        for (int i = 0; i < refineClusters.size(); ++i) {
            RefineClusterInfo<dim>& cluster = refineClusters[i];
            if (center.isInsideBbox(cluster.bbox)) {
                // remove this entry by replacing it with the last entry
                if (i < refineClusters.size() - 1)
                    refineClusters[i] = refineClusters.back();
                refineClusters.pop_back();
                break;
            }
        }

        if (refine.get(clusterId)) {
            // if the cluster needs to be refined, remember its bbox and
            // center to ensure that some part of it will be added to
            // the shapes later: if  the cluster dissolves in refinement,
            // the center of this ClusterInfo will be used as a shape
            RefineClusterInfo<dim> refineCluster;
            refineCluster.bbox = _clusters[clusterId]._bbox;
            refineCluster.center = center;
            refineClusters.push_back(refineCluster);
            ++refineCount;
        } else {
            // if the cluster can be used, add it to the resulting shapes
            addToShapes(shapes, center);
            ++useCount;
        }
    }

    if (REPORT_TO_CONSOLE) {
        cout << "refusing " << refuseCount << " clusters (too few points), "
                "using " << useCount << " (dense enough), "
                "refining " << refineCount << " (too extensive)." << endl;
    }
    // the first (writeIndex) entries of _points now hold the points
    // whose clusters need refinement. We now reduce _points to these points
    // (all others are points obsolete since either they were refused or
    // their center has been calculated above and stored in "shapes")
    _pointCount = writeIndex; // writeIndex is 1-based
    _points.get()->resize(1 + _pointCount); // _points is 1-based
}

/*
1.5.4 addToShapes

Adds the given center of a cluster to the given vector of shapes, and
optionally reports the shape to the console.

*/
template<unsigned dim>
void DbScan<dim>::addToShapes(std::vector<PointBase<dim>>& shapes,
        const PointBase<dim>& center) const {
    shapes.push_back(center);
    if (REPORT_TO_CONSOLE && shapes.size() <= REPORT_SHAPE_CENTERS_MAX) {
        cout << "Shape found at dual point " << center.toString() << endl;
    }
}

/*
3 Static functions

3.1 constructMMRTree

Constructs the given MMRTree. If useSequences = true, the entries of the
MMRTree will contain the start index of each point sequence in the given
1-based points vector; for useSequences = false, there will be an MMRTree
entry for every point index returned by the given iterator.

*/
template<unsigned dim>
void DbScan<dim>::constructMMRTree(
        std::unique_ptr<RtreeT2<dim, PointIndex, MMRTREE_NODE_SIZE_MAX>>&
                mmRTree,
        const std::vector<DbScanPoint<dim>>& points,
        const bool useSequences, std::shared_ptr<DbScan::iterator> iter) {
    // construct an MMRTree to point to the start index and point count
    // of each point sequence in "points"
    bool startSequence = true;
    size_t nodeStartIndex; // the node's start index in points
    int i;
    double min[dim];
    double max[dim];
    while ((i = iter.get()->next()) > 0) {
        if (startSequence) {
            nodeStartIndex = i;
        }
        const DbScanPoint<dim>& point = points[i];
        for (unsigned d = 0; d < dim; ++d) {
            double coord = point._coords[d];
            min[d] = startSequence ? coord : MIN(min[d], coord);
            max[d] = startSequence ? coord : MAX(max[d], coord);
        }
        // is this the last point of the current node?
        bool endOfSequence = !useSequences || point._isLastInSeq;
        if (endOfSequence) {
            // add the bounding box of the points in the current node
            // as a new entry to the mmRTree
            const Rectangle2<dim> nodeBox = Rectangle2<dim>( min, max );
            mmRTree->insert(nodeBox, nodeStartIndex);
        }
        startSequence = endOfSequence;
    }
}


// ensure defined references for the dimensions needed
// see stackoverflow.com/questions/8752837/
// undefined-reference-to-template-class-constructor
namespace pointcloud2 {
template class DbScan<3>;
template class DbScan<4>;
template class DbScan<5>;
template class DbScan<6>;
}

