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

*/
#include "MMRCloud.h"

using namespace pointcloud2;

bool MMRCloud::REPORT_TO_CONSOLE = true;
bool MMRCloud::REPORT_DETAILED = REPORT_TO_CONSOLE && false;

MMRCloud::MMRCloud(Pointcloud2* cloud, int maxSequenceLength) {
    size_t point_count = cloud->getPointCount();

    /* get the points from the cloud contains; indexOfSmiId contains the
     * _points index for every SmiRecordId used in the cloud */
    _points = std::make_shared<std::vector<DbScanPoint<DIMENSIONS>>>();
    _points.get()->resize(1 + point_count);// 1-based
    _indexOfSmiId = cloud->getAllPoints(*(_points.get()), maxSequenceLength);

    // create an MMRTree2 for the _points, using the sequences given above
    // by the nodes of the persistent RTree
    _tree = std::unique_ptr<MMCloudTreeT>( new MMCloudTreeT() );
    DbScan<DIMENSIONS>::constructMMRTree(
            _tree, *(_points.get()), true,
            DbScan<DIMENSIONS>::getStandardIterator(point_count));
    _cloudBbox = _tree->getBBox().toRectangleAttr();

    // create a BitArray and initialize it with "true" for every point.
    // setting a points's bit to "false" will exclude this point from
    // further calculations
    _bitMask = std::make_shared<BitArray>(cloud->getPointCount(), true);
    _bitMask->initializeAll(true);
}

double MMRCloud::estimateTypicalPointDistance(const size_t sampleSize)
    const {
    const int NEIGHBOR_COUNT = 1;

    size_t sampleCount = std::min(sampleSize, _points->size() - 1);
    if (REPORT_TO_CONSOLE) {
        std::cout << "estimating typical point distance from " << sampleCount
                << " points" << std::endl;
    }

    std::vector<double> values;
    values.reserve(sampleCount);

    BitArray sample { _points->size() - 1, true };
    sample.initializeRandom(sampleCount);
    auto iter = sample.getIterator();
    int i;
    int seqStart = 0;
    int seqEnd = 0;
    while ((i = iter->next()) >= 0) {
        assert (i > 0 && i < int(_points->size()));
        if (i > seqEnd) {
            seqStart = i;
            while(seqStart > 1 && !_points->at(seqStart -1)._isLastInSeq)
                --seqStart;
            seqEnd = i;
            while (seqEnd + 1 < int(_points->size()) &&
                    !_points->at(seqEnd)._isLastInSeq)
                ++seqEnd;
            ++seqEnd; // let seqEnd be the start index of the next sequence
            if (seqEnd - seqStart < NEIGHBOR_COUNT + 1)
                continue; // just one point in this sequence
        } // otherwise i belongs to the same sequence as the previous point

        double minDistSq[NEIGHBOR_COUNT];
        for (int d = 0; d < NEIGHBOR_COUNT; ++d)
            minDistSq[d] = std::numeric_limits<double>::max();

        DbScanPoint<DIMENSIONS>& point1 = _points->at(i);
        for (int j = seqStart; j < seqEnd; ++j) {
            if (j == i)
                continue;
            assert (j > 0 && j < int(_points->size()));
            DbScanPoint<DIMENSIONS>& point2 = _points->at(j);
            double distSq = point1.calculateDistanceSquare(point2);
            if (distSq > minDistSq[NEIGHBOR_COUNT - 1])
                continue;

            int insertAt = -1;
            for (int d = 0; d < NEIGHBOR_COUNT; ++d) {
                if (distSq < minDistSq[d]) {
                    insertAt = d;
                    break;
                }
            }
            if (insertAt >= 0) {
                for (int d = NEIGHBOR_COUNT - 1; d > insertAt; --d)
                    minDistSq[d] = minDistSq[d - 1];
                minDistSq[insertAt] = distSq;
            }
        }
        double valueSq = minDistSq[NEIGHBOR_COUNT - 1];
        values.push_back(valueSq);
    }
    std::sort(values.begin(), values.end());
    double median = std::sqrt(values[(size_t)(0.5 * values.size())]);
    if (REPORT_TO_CONSOLE) {
        if (REPORT_DETAILED) {
            size_t reportCount = std::min(values.size(), size_t(100));
            double fac = (double)values.size() / reportCount;
            for (size_t i = 0; i < reportCount; ++i) {
                size_t index = (size_t)(i * fac);
                double value = values[index];
                std::cout << ((i == (reportCount / 2)) ? "=> " : "   ");
                std::cout << std::sqrt(value) << std::endl;
            }
        }
        std::cout << "found median " << median << std::endl;
    }
    return median;
}
