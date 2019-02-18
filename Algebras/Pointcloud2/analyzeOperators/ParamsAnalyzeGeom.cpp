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
#include "ParamsAnalyzeGeom.h"
#include <cmath>

#include "../tcPointcloud2.h"

using namespace pointcloud2;

int ParamsAnalyzeGeom::_maxMMRCloudSequenceLength = -1;
// or MMRTREE_NODE_SIZE_MAX?

ParamsAnalyzeGeom::ParamsAnalyzeGeom(
        const int pointCount, const double typicalPointDistance) {

    // settings given by the user
    _typicalPointDistance = typicalPointDistance;
    _minimumObjectExtent = Pointcloud2::MINIMUM_OBJECT_EXTENT;
    double neighborhoodSize = Pointcloud2::NEIGHBORHOOD_SIZE;
    double softDiffusion = Pointcloud2::SOFT_DIFFUSION;
    double roughDiffusion = Pointcloud2::ROUGH_DIFFUSION;


    // parameters calculated from the above parameters

    // 1. _sampleSize and _typicalSampleDistance
    // -----------------------------------------
    // to recognize an object, our sample should hold at least 50 points of
    // this object, or rather 100. So, we need approx. 10 points per dimension
    // since we are recognizing surfaces:
    const double REQUIRED_OBJ_POINTS_PER_DIM = 10.0;
    // we therefore want to choose a sample size that provides the required
    // typical distance between sample points:
    double suggestSampleDist =
            _minimumObjectExtent / REQUIRED_OBJ_POINTS_PER_DIM;
    // if, for instances, a typical point distance is 0.1, but for samples,
    // a distance of 0.2 is enough (so, selectivity 2 for per dimension),
    // we can choose 1 sample point from 2 * 2 = 4 original points
    // (as we are talking surfaces!)
    double sampleSelectivity1D = suggestSampleDist / _typicalPointDistance;
    double sampleSelectivity2D = sampleSelectivity1D * sampleSelectivity1D;
    // this leads to the sample size which, of course, must be somehow bounded
    _sampleSize = (size_t)(pointCount / sampleSelectivity2D);
    _sampleSize = MAX(_sampleSize, 1000); // 1K
    _sampleSize = MIN(_sampleSize, 1000000); // 1M
    _sampleSize = MIN(_sampleSize, size_t(pointCount));
    // now we calculate the above values again with the actual sample size
    sampleSelectivity2D = pointCount / static_cast<double>(_sampleSize);
    sampleSelectivity1D = std::sqrt(sampleSelectivity2D);
    _typicalSampleDistance = _typicalPointDistance * sampleSelectivity1D;


    // 2. neighborhoods for dual point calculation
    // -------------------------------------------
    double NEIGHBORS_PER_DIMENSION = std::sqrt(neighborhoodSize);
    // with _neighborhoodDiameter = 10.0 * _typicalSampleDistance, an equal
    // distribution of sample points along the XY plane would result
    // in neighborhoods that contain approx. 100 sample points.
    _neighborhoodDiameter = NEIGHBORS_PER_DIMENSION * _typicalSampleDistance;
    // if the points of an object have _typicalSampleDistance, even a
    // neighborhood at the corner of an object should usually contain 20+
    // points.
    _neighborhoodSizeMin = 16; // points
    // since we assume that even for small objects, we have 100 sample points,
    // it is enough to create 1 dual point from each sample point (in
    // combination with a number of random points from its neighborhood as
    // required by the respective geometric primitive)
    _dualPointsPerSamplePoint = 1;


    // 3. DBSCAN in dual space
    // -----------------------
    // the creation of dual points is performed by the geometric primitives
    // in a way that tries to maintain the meaning of a "typical point
    // distance" (e.g. we do not use angles in dual space)
    // therefore, we can use multiples of point distances for the epsilon
    // value. Note that *point* distance rather than *sample* distance
    // should be used here since the proximity of the dual points does not
    // depend on the size of our sample, but rather on the precision of
    // the point data (diffusion)
    _dualScanEps = 1.0 * _typicalPointDistance; // not sample dist here!
    // points in dual space will often have 6, 8, or 10 neighbors, so for
    // dual point clusters that belong to an actual shape, 4 should be
    // easy to fulfill.
    _dualScanMinPts = 4;
    // clusters in dual space are recursively refined by reducing _dualScanEps
    // until a cluster's bbox is small enough (in each dimension):
    _dualScanMaxBboxExtent = 8.0 * _dualScanEps;
    // even for small objects with just 100 sample points, a lot of points
    // should be inside the cluster (unless diffusion is very high)
    _dualScanRequiredClusterSize = 20; // points


    // 4. Matching shapes to objects in point data
    // --------------------------------------------
    // at this point, our sample is irrelevant since now we are working
    // with the complete point data. We tolerate some diffusion (also bearing
    // in mind that because of diffusion, the shapes obtained above are
    // probably not a perfect match)  // not sample dist here!
    _matchTolerance = roughDiffusion * _typicalPointDistance;
    // for all points that could potentially belong to a given shape,
    // another DBSCAN is performed to keep only clusters of actual points.
    // For _matchScanEps, often 1.4 or 1.5 is enough, depending on the
    // "soft diffusion", i.e. how far points deviate from a regular grid
    _matchScanEps = (1.4 + softDiffusion) * _typicalPointDistance;
    _matchScanMinPts = 4; // or 3 (but not 2, otherwise mere intersection
                          // lines will be regarded as a shape)
    _matchScanRequiredCohesion = 100; // 100 neighborhood relationships
    // _matchScanRequiredClusterSize = 16;
}
