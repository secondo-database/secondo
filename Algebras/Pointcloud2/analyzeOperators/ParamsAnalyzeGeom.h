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



3 Parameter class

Contains only parameters that determine the
algorithm.
Parts of this are intended to be toyed
with by the user.

*/
#pragma once
#include <cstddef>

namespace pointcloud2 {

class ParamsAnalyzeGeom {
public:
    // static parameters

    /* the maximum number of points in a point sequence as represented by
     * a leaf of the MMRTree. If this value is low, the MMRTree will need
     * more main memory; if it is high, more points will be tested when
     * matching geometric shapes with the points in the Pointcloud2.  */
    static int _maxMMRCloudSequenceLength;


    // -----------------------------------------------------------------
    // parameters that influence the analysis and can be set by the user

    /* the minimum size (diameter) of objects that we wish to find in the
     * analysis. If this value is low, we need our sample points to be
     * more dense (and thus more numerous) */
    double _minimumObjectExtent;


    // -----------------------------------------------------------------
    // parameters guessed from the above parameters or the Pointcloud2 itself

    /* the typical distance between a point in the Pointcloud2(!) and its
     * "neighbor" points (note that this means points in the cloud, not
     * just points in the sample) */
    double _typicalPointDistance;

    /* the number of points in the sample from which dual points are
     * calculated */
    size_t _sampleSize;

    /* an estimated typical distance between a point in the sample(!) and
     * its "neighbor" points */
    double _typicalSampleDistance;

    /* the "diameter" of a neighborhood (actually a bbox is used). Dual points
     * are calculated from points randomly selected from the neighborhood of
     * a given center point. If this value is too high, a lot of useless
     * dual points are created (noise in dual space). If the value is too low,
     * diffusion of the points leads to widely scattered dual points. */
    double _neighborhoodDiameter;

    /* the minimum number of points required in a neighborhood for it to
     * be used to create tuples */
    size_t _neighborhoodSizeMin = 16;

    /* determines how many dual points will be created for each point in the
     * sample. At least one time, each point will be chosen as the "center
     * point" of a neighborhood; then, a tuple is created from this point and
     * the required number of extra points randomly selected from the same
     * neighborhood, and a dual point is calculated from the tuple. */
    size_t _dualPointsPerSamplePoint = 1;

    /* the epsilon value initially used for scanning the dual space. If the
     * DBSCAN finds large clusters, eps will recursively be diminished
     * to get smaller clusters */
    double _dualScanEps = 0.1;
    /* the minimum number of neighbor points required by the DBSCAN
     * algorithm when scanning the dual space for clusters */
    size_t _dualScanMinPts = 4;
    /* the maximum extent (in each dimension) of a cluster's bbox for that
     * cluster to be accepted without further refinement. For clusters that
     * exceed this value in at least one dimension, another scan will be run
     * with a smaller epsilon value. */
    double _dualScanMaxBboxExtent = 0.3;
    /* the number of dual points expected in a cluster. Clusters that contain
     * fewer dual points will be ignored. Note that large clusters undergo
     * refinement (with smaller epsilon values); if this refinement results
     * in one or several clusters below this limit, the weighted center of
     * the initial cluster will be used, so the cluster will not be refused
     * altogether */
    size_t _dualScanRequiredClusterSize = 20;

    /* the tolerated distance from an actual point in the Pointcloud2 to an
     * (ideal) geometric primitive. If the distance is below this limit, the
     * the point to be assigned to that primitive */
    double _matchTolerance = 0.1;
    /* the epsilon value used for DBSCAN when identifying objects among all
     * points assigned to a given geometric primitive */
    double _matchScanEps = 0.3;
    /* the minPts value used for DBSCAN when identifying objects among all
     * points assigned to a given geometric primitive */
    size_t _matchScanMinPts = 4;
    /* the minimum cohesion (i.e. number of neighborhood relationships)
     * required in a cluster in DBSCAN. For instance, a cluster of 25 points
     * with an average of 4 neighbors has cohesion 100. Clusters with less
     * cohesion will be ignored (i.e. the corresponding points will not be
     * assigned to an object) */
    size_t _matchScanRequiredCohesion = 100;
    /* the minimum number of points required in a cluster in DBSCAN. Clusters
     * with fewer points will be ignored (i.e. the corresponding points will
     * not be assigned to an object) */
    // size_t _matchScanRequiredClusterSize = 10;

    /* the minimum number of points required for an object. If at the end
     * of the analyzeGeom process, an object has fewer points (left), it will
     * not be assigned to those points. Not that points assigned to an object
     * may be reassigned if later in analyzeGeom it is found that they can
     * be assigned to a larger object. */
    size_t _requiredObjSize = 10;

    ParamsAnalyzeGeom(const int pointCount, const double typicalPointDistance);

    ~ParamsAnalyzeGeom() = default;
};
}
