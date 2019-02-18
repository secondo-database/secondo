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



1 Implementation of the Geometric Primitives - Helper Classes

*/
#include "GeomPrimitive.h"

#include <limits>

#include "../utility/DbScanPoint.h"
#include "../utility/BitArray.h"
#include "../utility/DbScan.h"
#include "../utility/MathUtils.h"

using namespace pointcloud2;
using namespace std;

/*
2 Implementation Parts of template GeomPrimitive

2.1 projectClusterSetObjID

*/
template<unsigned dual>
void GeomPrimitive<dual>::projectClusterSetObjID(
        const vector<SamplePoint>& sample,
        shared_ptr<MMRCloud> cloud,
        const ParamsAnalyzeGeom& params,
        shared_ptr<ObjectIdManager> objManager,
        Timer& timer) {

    timer.getReportStream() << timer.getInset()
            << getCaption(true) << endl;

    /* use the given sample points to calculate points in dual space each
     * of which represents a possible instance of this primitive. */
    timer.startTask("- projectSampleToDual");
    shared_ptr<vector<DbScanPoint<dual>>> dualPoints =
            this->projectSampleToDual(sample, params, cloud->_cloudBbox);

    /* now cluster the points in dual space, refining large clusters
     * and then getting the weighted center of each cluster. This results
     * in a vector of dual points each of which represents a shape we
     * expect to find in the Pointcloud2. */
    timer.startTask("- clusterAndReduceToCenters");
    vector<PointBase<dual>> shapes = this->clusterAndReduceToCenters(
            dualPoints, params);

    /* iterate over the given shapes, using the mmRTree to identify all
     * candidate points in the Pointcloud2 that may belong to the
     * respective shape. Cluster these points and assign the points in big
     * enough clusters to a new object id. */
    timer.startTask("- matchShapesWithPoints");
    this->matchShapesWithPoints(shapes, cloud, params, objManager);
}


/*
2.2 project sample to dual

*/
template<unsigned dual>
shared_ptr<vector<DbScanPoint<dual>>>
GeomPrimitive<dual>::projectSampleToDual(
        const vector<SamplePoint>& sample,
        const ParamsAnalyzeGeom& params,
        const Rectangle<DIMENSIONS>& bboxOfEntireCloud) {

    size_t sampleSize = sample.size();
    /* "sample" is a random sample of points from the whole
     * Pointcloud2 in an undefined order. If the cloud contains e.g. 100
     * shapes to be identified, and if e.g. dual = 4 (so 4 points are needed
     * to calculate a dual point), the chance of randomly selecting 4 points
     * from the _same_ shape is 1 out of 1 million (100^3). And we'd need
     * many such dual points to get a cluster in the dual space while
     * 99.9999% of the points in the dual space would be noise. So, random
     * selection does not work.
     *
     * Instead, we will insert the points from sample into an MMRTree.
     * For each point p, the MMRTree will give us more points that are close
     * enough to p to be part of the same shape. Only such tuples will be
     * used to create a dual point. Despite the costs of the MMRTree this
     * will be far more efficient than random selection.
     *
     * Once we have a "neighborhood" of one core point p, we use it for some
     * other points q, r, s, ... that are close to the core of this
     * neighborhood. We use a BitArray to remember which points have
     * already been treated. Points at the edges of a neighborhood will be
     * treated later (in the context of a different neighborhood to which
     * they are more central).
     */

    // create the result vector. For each point in sample, we will
    // create (TUPLE_COUNT_PER_CENTER) tuples.
    shared_ptr<vector<DbScanPoint<dual>>> result = std::make_shared<
            vector<DbScanPoint<dual>>>();
    // since we introduced an additional "challenge", not every sample point
    // is turned into a dual point, so we can omit the next line:
    // result->reserve(1 + params._dualPointsPerSamplePoint * sampleSize);

    // since DbScan needs _dualPoints to be 1-based, add an empty entry
    // that will not be used
    DbScanPoint<dual> dualPoint;
    dualPoint.initialize();
    dualPoint.clearCoords();
    result->push_back(dualPoint);

    // construct an MMRTree from the sample
    MMCloudTreeT tree = MMCloudTreeT();
    for (unsigned i = 0; i < sampleSize; ++i)
        tree.insert(sample[i].getBoundingBox2(), i );

    if (REPORT_TO_CONSOLE) {
        using std::cout;
        using std::endl;
        cout << endl << "==================== " << getCaption(true) << endl;
        cout << "using MMTree to create dual points "
                "from neighborhoods:" << endl;
        cout << "sampleSize = " << formatInt(sampleSize) << endl;
        cout << "tree.bbox = (" << DbScanPoint<DIMENSIONS>::getStringForBBox(
                tree.getBBox().toRectangleAttr()) << endl;
        cout << "neighborhoodDiameter = "
                << params._neighborhoodDiameter << endl;
        if (REPORT_DETAILED)
            cout << "points used as center points marked by *" << endl;
     }
    // construct a BitArray that holds "true" for every point in the sample
    // which still needs to be used as the "center point" for dual point
    // construction. Noise points with too few neighbor points cannot be
    // used as center points, therefore the term "candidates"
    BitArray centerCandidates { sampleSize , false };
    centerCandidates.initializeAll(true);

    // iterate over the points that have not been treated yet (note that bits
    // get changed during the process, so the iterator will not "stop" at
    // every point)
    vector<PointIndex> neighbors; // contains indices into sample
    size_t smallNeighborhoodCount = 0;
    size_t largeNeighborhoodCount = 0;
    auto bitsIter = centerCandidates.getIterator();
    int i;
    while ((i = bitsIter->nextUncached()) >= 0) {
        // get the point for which a neighborhood will be retrieved
        const SamplePoint& coreOfNeighborhood = sample[i];
        Rectangle2<DIMENSIONS> neighBbox = coreOfNeighborhood.getBoundingBox2(
                params._neighborhoodDiameter / 2.0);

        // retrieve the indices of all points in this neighborhood
        // (including the coreOfNeighborhood point itself)
        neighbors.clear(); // reuse the vector so it does not need to
                           // resize and allocate thousands of times
        auto treeIter = tree.find(neighBbox);
        const PointIndex* neighborPtr;
        while ((neighborPtr = treeIter->next()) != 0)
            neighbors.push_back(*neighborPtr);
        delete treeIter;

        if (REPORT_DETAILED) {
            std::cout << "neighborhood of " << i <<
                    (centerCandidates.get(i) ? " " : "*") << ":";
            for (PointIndex n : neighbors) {
                std::cout << " " << n <<
                        (centerCandidates.get(n) ? " " : "*");
            }
            std::cout << std::endl;
        }

        // if the neighborhood is too small, we must keep this core for later
        // and try with a higher neighborhoodRadius
        if (neighbors.size() < params._neighborhoodSizeMin) {
            ++smallNeighborhoodCount;
            continue;
        }
        ++largeNeighborhoodCount;

        // we use a BitArray to create random selections of (getTupleSize())
        // neighborIndices. The BitArray prevents tuples from containing the
        // same point twice
        BitArray neighborIndexSelector = BitArray(neighbors.size(), false);
        neighborIndexSelector.initializeAll(true);

        // now every point close to the core of the neighborhood (including
        // the core point itself) can be used to construct a tuple, unless the
        // point has already been used
        for (PointIndex neighborIndex = 0; neighborIndex < neighbors.size();
                ++neighborIndex) {
            PointIndex j = neighbors[neighborIndex];
            // if the point has already been used because it was in close
            // to the center of a previously treated neighborhood, skip it
            if (!centerCandidates.get(j))
                continue;

            // calculate the minimum distance from this point to the border
            // of the neighBbox to determine whether the point is close
            // enough to the center
            const SamplePoint& centerCandidate = sample[j];
            if (!centerCandidate.isCloseToCenter(neighBbox))
                continue; // this point is not close enough to the center

            // create tuples from this point and others from the neighborhood
            // and project them to the dual space
            neighborIndexSelector.set(neighborIndex, false);
            vector<DbScanPoint<dual>> projected_sample =
                    this->projectNeighborhoodToDual(neighbors, j,
                            neighborIndexSelector, sample, params,
                            bboxOfEntireCloud);

            result->insert(result->end(), projected_sample.begin(),
                    projected_sample.end());

            neighborIndexSelector.set(neighborIndex, true);

            // mark this center point as treated, so bitsIter will skip it
            // if it comes along this point later
            centerCandidates.set(j, false);
        }

        if (REPORT_DETAILED) {
            std::cout << " ->  changed to " << i <<
                    (centerCandidates.get(i) ? " " : "*") << ":";
            for (size_t n : neighbors) {
                std::cout << " " << n << (centerCandidates.get(n)
                        ? " " : "*");
            }
            std::cout << std::endl;
        }
    }

    if (REPORT_TO_CONSOLE) {
        std::cout << formatInt(result->size() - 1) << " dual points were "
                "constructed from " << formatInt(largeNeighborhoodCount)
                << " neighborhoods" << std::endl;
        std::cout << formatInt(smallNeighborhoodCount)
                << " neighborhoods were considered too small (< "
                << params._neighborhoodSizeMin << " points) to be used "
                "for dual point construction." << std::endl;
        std::cout << formatInt(centerCandidates.getTrueCount())
                << " points were never used as center points (noise?)"
                << std::endl;
    }
    // TODO: wenn mit diesem neighborhoodRadius viele Punkte noch nicht als
    // center points genutzt wurden (z.B. bei bits.getTrueCount() > 0.1 *
    // sampleSize), dann war vielleicht neighborhoodRadius insgesamt zu klein.
    // Dann neighborhoodRadius verdoppeln und für diese Punkte erneut über
    // bits iterieren? Oder: grundsätzlich einen zweiten Durchgang machen
    // und neighborhoodRadius dabei an einem typicalObjectExtent orientieren!

    return result;
}

/*
2.3 project neighborhood to dual

*/
template<unsigned dual>
vector<DbScanPoint<dual>> GeomPrimitive<dual>::projectNeighborhoodToDual(
        const vector<PointIndex>& neighbors,
        const PointIndex centerPoint,
        const BitArray& neighborIndexSelector,
        const vector<SamplePoint>& sample,
        const ParamsAnalyzeGeom& params,
        const Rectangle<DIMENSIONS>& bboxOfEntireCloud) {

    unsigned tupleSize = getTupleSize();
    vector<SamplePoint> tuple; // contains indices into sample
    tuple.reserve(tupleSize);
    vector<DbScanPoint<dual>> result;

    // construct tuples from center2 and randomly selected
    // points in the same neighborhood
    for (size_t t = 0; t < params._dualPointsPerSamplePoint; ++t) {
        // use point as center point
        tuple.push_back(sample[centerPoint]);

        // add further points from the neighborhood to the tuple;
        // these points may be at the edge of neighBbox
        // the iterator returns (tupleSize - 1) randomly selected, unique
        // values; the value (centerPoint) was already excluded
        auto iter = neighborIndexSelector.getIterator(tupleSize - 1);
        int neighborIndex;
        while ((neighborIndex = iter->next()) >= 0)
            tuple.push_back(sample[neighbors[neighborIndex]]);

        // create the point in dual space
        unique_ptr<vector<DbScanPoint<dual>>> projected_tuple =
                projectTupleToDual(tuple, params, bboxOfEntireCloud);

        if( projected_tuple ) {
            result.insert(result.end(), projected_tuple->begin(),
                    projected_tuple->end());
        }
        tuple.clear();
    }

    // the resulting points in sample each represent one
    // geometric shape in regular space
    return result;
}

/*
2.4 cluster and reduce to centers

*/
template<unsigned dual>
vector<PointBase<dual>> GeomPrimitive<dual>::clusterAndReduceToCenters(
        shared_ptr<vector<DbScanPoint<dual>>> dualPoints,
        const ParamsAnalyzeGeom& params) {

    DbScan<dual> dbscan { params._dualScanEps, params._dualScanMinPts,
        dualPoints, false };
    vector<PointBase<dual>> shapes = dbscan.runRecursive(
            params._dualScanRequiredClusterSize,
            params._dualScanMaxBboxExtent);

    return shapes;
}

/*
2.5 match shapes with points

*/
template<unsigned dual>
void GeomPrimitive<dual>::matchShapesWithPoints(
        const vector<PointBase<dual>>& shapes,
        shared_ptr<MMRCloud> cloud,
        const ParamsAnalyzeGeom& params,
        shared_ptr<ObjectIdManager> objManager) const{

    if (REPORT_TO_CONSOLE) {
        cout << "-------------------- Matching " << getCaption(false)
                << " shapes with actual points in cloud:" << endl << endl;
    }

    // create a filter which can be used to perform DbScan on a subset
    // of "points" (without copying the points to an additional array)
    std::shared_ptr<BitArray> scanFilter = std::make_shared<BitArray>(
            cloud->_points->size() + 1, true);

    // iterate over the shapes which were found in the dual space by DbScan
    unsigned shapeIndex = 0;
    for (const PointBase<dual> shape : shapes) {
        ++shapeIndex;
        if (REPORT_TO_CONSOLE) {
            cout << "matching " << getCaption(false) << " "
                    << formatInt(shapeIndex) << " / "
                    << formatInt(shapes.size()) << " with points in cloud"
                    << endl;
        }

        // get the predicates that enable us to efficiently determine whether
        // a given bbox or point possibly intersects with, or is part of
        // the current shape
        BboxPredicate bboxPredicate = this->getBboxPredicateForShape(shape,
                params);
        GeomPredicate pointPredicate = this->getPredicateForShape(shape,
                params);

        // points near this shape cannot simply be assigned to the shape
        // but need clustering first. Therefore, we initialize the scanFilter
        // and will set a "true" bit for each candidate point
        scanFilter->clear();

        // find sequences which are in a bbox that (possibly) intersects
        // with the current shape. The iterator returns the start index of
        // a sequence of points (as storing all individual points in the
        // mmRTree would use up too much memory). These sequences were
        // created in Pointcloud2::getAlloints(); each sequence represents
        // the points stored in the persistent RTree
        auto iter = cloud->_tree->find(&bboxPredicate);
        size_t matchLeafCount = 0; // the number of matching mmRTree leafs
        size_t testPointCount = 0;
        const PointIndex* startIndexPtr;
        while ((startIndexPtr = iter->next()) != 0) {
            // the mmRTree iterator found a leaf with a bbox that fulfills
            // the bboxPredicate
            ++matchLeafCount;

            // iterate over the points in this sequence
            PointIndex index = *startIndexPtr;
            do {
                // get the next point and test it
                DbScanPoint<DIMENSIONS>& point = cloud->_points->at(index);
                ++testPointCount;
                if (pointPredicate(point)) {
                    // found a candidate! Activate its bit in the scanFilter
                    scanFilter->set(index, true);
                    point._clusterId = SCAN_UNCLASSIFIED;
                }
                if (point._isLastInSeq)
                    break; // end of point sequence
                ++index;
            } while (true);
        }
        delete iter;

        if (REPORT_TO_CONSOLE) {
            size_t foundCount = scanFilter->getTrueCount();
            cout << formatInt(foundCount) << " points in "
                    << formatInt(matchLeafCount) << " mmRTree leafs found "
                    << "(= " << (foundCount / (double)testPointCount * 100.0)
                    << "% of tested points) for " << getCaption(false)
                    << " " << shape.toString() << endl;
        }

        // if no candidates were found, continue with next shape
        if (scanFilter->getTrueCount() == 0)
            continue;

        // now we need to cluster the candidates. For reasons of efficiency,
        // we do not copy them into a points vector of their own but rather
        // pass our scanFilter to DbScan which will then operate only on
        // the points that are "true" in the filter.
        DbScan<DIMENSIONS> dbscan { params._matchScanEps,
            params._matchScanMinPts, cloud->_points, scanFilter };
        dbscan.run();

        // DbScan has now entered _clusterIds into the candidate points.
        // From the clusters found by DbScan we now select those which we
        // actually want to keep. The same shape can lead to many clusters
        // (e.g. flat roofs on the same plane) and therefore to many objIds.
        size_t clusterCount = dbscan.getClusterCount();
        vector<int> newObjIDs(1 + clusterCount, SCAN_UNCLASSIFIED);
        size_t useObjCount = 0;
        for (size_t clusterId = 1; clusterId <= clusterCount; ++clusterId) {
            unsigned clusterSize = dbscan.getClusterSize(clusterId);
            unsigned clusterCohesion = dbscan.getClusterCohesion(clusterId);
            if (clusterCohesion >= params._matchScanRequiredCohesion) {
                // previously:
                // if (clusterSize >= params._matchScanRequiredClusterSize) {

                const Rectangle2<DIMENSIONS>& clusterBbox =
                        dbscan.getClusterBbox(clusterId);
                newObjIDs[clusterId] = objManager->addObject(
                        clusterSize, clusterCohesion, clusterBbox.Perimeter());
                ++useObjCount;
            } // otherwise, keep SCAN_UNCLASSIFIED as cluster is too small
        }

        // now enter the new clusterIds into the points, using scanFilter to
        // iterate over them. All this is still done within the main memory
        // and only at the very end made persistent in the Pointcloud2.
        // Note that a point that has previously been assigned to an object
        // may be reassigned now if the current object encompasses more points
        // than the previously assigned object. This is decided by the
        // objectManager.
        auto idIter = scanFilter->getIterator();
        int pointIndex;
        size_t assignCount = 0;
        while ((pointIndex = idIter->next()) >= 0) {
            DbScanPoint<DIMENSIONS>& point = cloud->_points->at(pointIndex);
            int newObjID = newObjIDs[point._clusterId];
            if (newObjID != SCAN_UNCLASSIFIED && objManager->
                    setNewObjIdIfObjectBigger(pointIndex, newObjID)) {
                // once assigned to an object, we could "deactivate" a point
                // to prevent it from being assigned to a different geometric
                // primitive, using cloud->_bitMask->set(pointIndex, false);
                // however, experiments have shown that the quality of the
                // analysis is much better if we allow the point to be
                // reassigned to an object that may fit much better than the
                // one found here.
                ++assignCount;
            }
        }

        if (REPORT_TO_CONSOLE) {
            cout << "+ " << formatInt(assignCount) << " points "
                    "assigned to " << useObjCount << " "
                    << (useObjCount == 1 ? "object" : "objects") << " in "
                    << getCaption(false) << " " << shape.toString
                    () << endl << endl;
        }

    } // next shape
}

/*
2.6 is point far outside

*/
template<unsigned dual>
bool GeomPrimitive<dual>::isPointFarOutside(const double x, const double y,
        const double z, const Rectangle<DIMENSIONS>& bbox) {
    // points are considered "far outside" if their distance to the bbox is
    // larger than (MAX_FACTOR * maximum bbox range) in at least one dimension
    const double MAX_FACTOR = 2.0;

    double dist = 0.0;
    dist = MAX(dist, bbox.MinD(0) - x);
    dist = MAX(dist, x - bbox.MaxD(0));
    dist = MAX(dist, bbox.MinD(1) - y);
    dist = MAX(dist, y - bbox.MaxD(1));
    dist = MAX(dist, bbox.MinD(2) - z);
    dist = MAX(dist, z - bbox.MaxD(2));
    if (dist == 0.0) // the point is inside the bbox
        return false;

    double diam = 0.0;
    for (unsigned i = 0; i < DIMENSIONS; ++i)
        diam = MAX(diam, bbox.MaxD(i) - bbox.MinD(i));

    return (dist / diam > MAX_FACTOR);
}

template class GeomPrimitive<3>;
template class GeomPrimitive<4>;
template class GeomPrimitive<5>;
template class GeomPrimitive<6>;
