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



0 includes

*/
#pragma once
#include <unordered_map>
#include "Operator.h"
#include "StandardTypes.h"
#include "opAnalyzeRaster.h"

#include "../tcPointcloud2.h"

namespace pointcloud2
{

/*
0 structure used in object map

*/
struct ObjProp
{
  int pointCount;
  size_t cellCount;
  int edgeCellCount;
  double edgeAltDiff;
  int minX;
  int minY;
  double minAlt;
  double maxAlt;
  double averageAlt;
  Rect bbox;
  size_t scanIndex;
  size_t classID;

  std::string toString() const {
      return "(" + std::to_string(pointCount)
          + ", " + std::to_string(cellCount)
          + ", " + std::to_string(edgeCellCount)
          + ", " + std::to_string(edgeAltDiff)
          + ", "+ std::to_string(maxAlt-minAlt)
          + ", "+ std::to_string(averageAlt)
          + ", " + std::to_string(bbox.MinD(0))
          + "-" + std::to_string(bbox.MaxD(0))
          + "/ " + std::to_string(bbox.MinD(1))
          + "-" + std::to_string(bbox.MaxD(1))
          + ") |";
  }

  bool equals(ObjProp& o) {
      return (this->pointCount == o.pointCount)
              && (this->cellCount == o.cellCount)
              && (this->edgeCellCount == o.edgeCellCount)
              && (this->edgeAltDiff == o.edgeAltDiff)
              && (this->minAlt == o.minAlt)
              && (this->maxAlt == o.maxAlt)
              && (this->averageAlt == o.averageAlt)
              && (this->bbox == o.bbox);
  }
};


/*
0 The ARClassify Class

This Class provides the static function classify
for "AnalyzeRaster" Operator.

*/
class ARClassify {

/*
1.1 Parameter

*/
    static constexpr bool RASTER_CLASSIFY_DEBUG = true;
    static constexpr bool RASTER_CLASSIFY_FINDDUPLE_EXTREME = false;

    // defines the number of properties for classification process
    // properties: pointCount, cellCount, maxAlt-minAlt, averageAlt
    static constexpr size_t fvd = 7; // (fvd =feature vector dimensions)
    static constexpr size_t fvdMax = 6; // (fvd =feature vector dimensions)

/*
1.1 private functions

creates a map of objects by reading out the
properties of each cell

*/
    static
    std::unordered_map<int, ObjProp> createObjectMap(
            const std::vector<SmiFileId>& rasters,
            const std::vector<Rect3>& rastersBbox,
            const size_t pointInMem);

/*
~findDuplicates~ searches in objectmap for objects with equal properties
but different objectIDs

*/
    static
    void findDuplicates(std::unordered_map<int, ObjProp> object,
                    std::shared_ptr<std::unordered_map<int,int>> duple);

/*
~testForSameValueObjects~ searches in objectmap for objects with equal
properties but different coordinates.

*/
    static
    void testForSameValueObjects(
            const std::unordered_map<int, ObjProp>& objects);

/*
computes an adjustment for the property values used as part of the
feature vector in dbscan

*/
    static
    void normalize(const std::unordered_map<int, ObjProp>& objects,
            double adjust[fvd]);

/*
for dbscan fills vector of dbscanpoints. with each object is one scanpoint.

*/
    static
    void createDbScanVector(
            std::shared_ptr<std::vector<DbScanPoint<fvdMax> > > dbScanObjects,
            std::unordered_map<int, ObjProp>& objects,
            const double adjust[],
            size_t& ScanObjectIndex);

/*
recalcing bbox for attributes.

*/
    static
    void recalcBbox(Rect& bboxObj, size_t nF, size_t pointInMem);

/*
1.1 public functions

classification: each object gets a classID

*/
public:

    static
    std::shared_ptr<std::unordered_map<int,size_t>>
    classify (std::vector<SmiFileId> rasters,
                    std::vector<Rect3> rastersBbox,
                    const size_t pointInMem,
                    std::shared_ptr<std::unordered_map<int,int>> duple);
};

} // namespace pointcloud2
