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



1 The Pointcloud2 AnalyzeRaster Operator
analyzeRaster: pointcloud2(R,tuple(Z1...Zn))
-> pointcloud2(R,tuple(Z1...Zn,ObjID:int,CatID:int))
use only with metric pointClouds
if non metric use project operator to transfer to UTM
Parameters:

  * CELL\_SIZE\_IN\_M 0.15; min: 0.01 max: 100.0

  * DELTA\_ALT\_IN\_M 5.0; min: 0.0 max: 100.0

  * NEIGHBOUR\_CELLS false; true 8 cells, false only 4

  * THRESHOLD\_MERGE 0.5; min: 0.0, no limits

  * DISTANCE\_SIG\_MAXIMA 0.1; min: 0.0, no limits

  * OVERLAP\_RASTERS 0.1; min: 0.0% max: 50.0%

  * MIN\_GROUND\_CELLS 100; min: 1 no limits

  * MAX\_CELLS\_AT\_EDGE 3

  * RASTER\_CLASSIFY\_EPSILON 1.0

  * RASTER\_CLASSIFY\_MINPTS 3

*/
#pragma once
#include <unordered_map>
#include "Operator.h"
#include "StandardTypes.h"

#include "../tcPointcloud2.h"

namespace pointcloud2
{

namespace analyzeRasterConstants
{
  /*
            CONSTANTS
   */
const size_t MIN_NEIGBOR_LOC_MAX = 3;
//factor for comparing locale maximum and object to ignore maximum
const double LOC_MAX_NEARLY_OBJ = 0.8;

} // namespace analyzeRasterConstants


  //one raster cell
struct RasterPoint
{
  int objectID = 0;
  int classID = 0;
  int count = 0;
  double minAlt = 0.0;
  double maxAlt = 0.0;
  double averageAlt = 0.0;
  int averageIntensity = 0.0;
  bool initialized = true;
};

//one raster cell in SMI file
struct RasterPointDB
{
  CcInt objectID;
  CcInt classID;
  CcInt count;
  CcReal minAlt;
  CcReal maxAlt;
  CcReal averageAlt;
  CcInt averageIntensity;
  CcBool initialized;
};

//one raster cell for save
struct RasterPointSave
{

int objectID;
int classID;
};

//one raster cell in SMI file for save
struct RasterPointDBSave
{
CcInt objectID;
CcInt classID;
};



class op_analyzeRaster
{
  /*
        PRIVATE MEMBERS
   */
  Pointcloud2* _pc2 = nullptr;
  Rect3 _bb;
  size_t _maxMemory;
  size_t _pointInMem, _pointInMem2;
  size_t _posNewAttributes;
  size_t _posIntensity;
  unsigned _sizeRaster;
  unsigned _sizeRasterOverlap;
  ListExpr _resultType;
  std::vector<int> _neighborD;
  std::vector<int> _neighbor;

  /*
        ALGEBRA METHODS
  */
  static ListExpr analyzeRasterTM(ListExpr args);

  static int analyzeRasterVM(Word *args, Word &result, int message,
      Word &local, Supplier s);

  std::string getOperatorSpec();

  /*
        PRIVATE METHODS
  */

  /*
   euclidian distance
   */
  inline double distance_m(double x1, double y1, double x2, double y2) const
  {
    //EUCLID
    return std::sqrt(std::pow((x1 - x2), 2) + std::pow((y1 - y2), 2));
  }


  /*
   calculate Bbox of memory part
  */
  std::unique_ptr<Pointcloud2, std::function<void(Pointcloud2*)>>
      calculateRasterRartition(double x, double y) const;

  /*
  calculate Pc2 partition
  */
  std::unique_ptr<Pointcloud2, std::function<void(Pointcloud2*)>>
      calculateRasterRartition(Rect3 bboxRaster) const;

  /*
   calculate raster points and attributes for memory structure
  */
  std::vector<RasterPoint>
  fillrastermem(Pointcloud2& pc2RasterPart, double x, double y) const;


   /*
   raster points from intern to secondo format
   */
   RasterPointDB
   structToSecondo(RasterPoint grid) const;


//   /*
//   raster points from secondo to intern format
//   */
//   RasterPoint
//   secondoToStruct(RasterPointDB bufferDB) const;

   /*
   raster points from secondo to intern format
   only ObjectID and ClassID
   */
   RasterPointSave
   secondoToStructSave(RasterPointDBSave bufferDB) const;

  /*
   *calculate objects by flooding
   *calculate local maxima
   *calculate significant maxima
   *split objects
  */
  int
  flooding(std::vector<RasterPoint>& rasterMem, int areaNumber,
      const Rect3 rasterBbox) const;

  /**
   *calculate local maxima in one object
   */
  int
  calcLocMax(std::vector<RasterPoint>& rasterMem,
      int areaNumber, Rect bboxObj) const;

  /**
    *merge local maxima to significant maxima
  */
  void
  calcEdgeForMerge(std::vector<RasterPoint>& rasterMem,
      int areaNumber, Rect bboxObj) const;

  /**
    *split objects by significant maxima
  */
  void
  splitObjects(std::vector<RasterPoint>& rasterMem,
      int areaNumber, Rect bboxObj) const;

  /*
   *test if between count and other locale maximum lower minimum
   *true if found
  */
  bool
  testForOtherMin(std::vector<RasterPoint>& rasterMem,
      size_t arrayCount, int neighbor, int areaNumber) const;

  /*
   *recalculate bbox for objects by newly discoverd points
  */
  void recalcBbox(Rect& bboxObj, size_t nF) const;


  /**
   *overwrite with other areaNumber
  */
  void
  deleteAreaNumber(std::vector<RasterPoint>& rasterMem,
      int areaNumber, Rect bboxLoc, int areaNumberOld) const;


  /**
   *put neighbor cells in stack for flooding
   */
  void checkNeighbors(std::stack<size_t> &s, 
                      std::vector<RasterPoint>& rasterMem,
                      std::vector<int> neighbor, 
                      size_t nF, bool& ground, size_t& edge) const;

  /*
   *neighbor check for flooding to find local maxima
   *definition: is part of object and all cells have same height roughly
   *is higher than all neighbor cells (min 2*THRESHOLD_MERGE)
   *has minimum of three neighbor cells
  */
  void checkNeighborsMaximum(std::stack<size_t>& s, 
      std::vector<RasterPoint>& rasterMem,
      std::vector<int> neighbor, size_t nF, bool& found, size_t& edge,
      int const areaNumber) const;

  /*
   *merge local maxima
  */
  void mergeLocMax(std::vector<RasterPoint>& rasterMem,
      size_t const arrayCount, int const areaNumber) const;

  /**
   *compute merge with Horn algorithm for circle(r)
  */
  bool calcNearLocMaxByHorn(std::vector<RasterPoint>& rasterMem,
      size_t const arrayCount, int const areaNumber,
      std::vector<std::pair <size_t,size_t>>& newLocMax) const;

  /*
   *fill vector with local maxima as preparation of calculating convex hull
  */
  std::vector<std::pair <size_t,size_t>>
  fillVectorWithLocMax(std::vector<RasterPoint>& rasterMem,
      size_t const arrayCount) const;

  /*
   * add local maximum to be merged to vector
   */
  void
  fillVectorWithOtherLocMax(std::vector<RasterPoint>& rasterMem,
      const size_t arrayCount, const int areaNumberLocMax,
      std::vector<std::pair <size_t,size_t>>& newLocMax) const;

  /**
    *calculate convex hull by Gift-Wrapping algorithm
    */
  std::vector<std::pair <size_t,size_t>>
  calcConvexHull(std::vector<std::pair <size_t,size_t>> newLocMax) const;

  /*
    *fill convex hull to compute significant maximum by edge list algorithm
  */
  void
  fillConvexHull(std::vector<RasterPoint>& rasterMem,
      std::vector<std::pair <size_t,size_t>> hullLocMax,
      const int areaNumberLocMax) const;

  int distanceComp(std::pair <size_t,size_t> a,
      std::pair <size_t,size_t> b,
      std::pair <size_t,size_t> c) const;

  /*
    *test for new local maximum in distance of central local maxima
  */
  void testOtherLocMax(std::vector<RasterPoint>& rasterMem,
      size_t const xMin, size_t const xMax, size_t const y,
      const int areaNumberObj, const int areaNumberLocMax,
      bool& countLocMax, 
      std::vector<std::pair <size_t,size_t>>& newLocMax) const;


  /*
    *calculating points in edge line of convex hull
    *ignores horizontal lines
  */
  bool computeEdgeLine(std::vector<std::pair <size_t,size_t>> &edgeList,
      const std::pair<size_t, size_t> a, 
      const std::pair<size_t, size_t> b) const;


  /*
   *write back raster to pointcloud2
  */
  void saveRasterToPc2(std::vector<SmiFileId>& rasters,
      std::vector<Rect3> rastersBbox,
      std::shared_ptr<std::unordered_map<int,size_t>> classMap,
      std::shared_ptr<std::unordered_map<int,int>> duples,
      Pointcloud2* res) const;

  /*
   *write points
  */
  void savePoint(Pointcloud2* res,
      const Pointcloud2& rasterPart,
      const Rect3 rasterBbox,
      const SmiRecord& record,
      PcPoint& pcPoint,
      std::vector<RasterPointSave> const &rasterMem,
      std::shared_ptr<std::unordered_map<int,size_t>> classMap) const;

  /*
   *erase double objects in overlappings
  */
  void recalcOverlapping(std::vector<RasterPointSave> &rasterMem,
      const Rect3 rasterBbox, const size_t i,
      const std::shared_ptr<std::unordered_map<int,int>> duples,
      std::vector <RasterPointSave> &rasterOverlapH,
      std::vector<std::vector<RasterPointSave>> &rasterOverlapUnder,
      std::vector<std::vector<RasterPointSave>> &rasterOverlapUnderEdge,
      std::vector<std::vector<RasterPointSave>> &rasterOverlapOver,
      std::vector<std::vector<RasterPointSave>> &rasterOverlapOverEdge) const;

public:

  explicit op_analyzeRaster() = default;

  op_analyzeRaster(Pointcloud2 *pc2,
      size_t maxMemory,
      size_t pos,
      size_t posIntensity,
      const ListExpr resultType
      )
  : _pc2(pc2),
    _maxMemory(maxMemory),
    _posNewAttributes(pos),
    _posIntensity(posIntensity),
    _resultType(resultType)
  {
    _bb = _pc2->getBoundingBox();

    //use 3/4 of memory for raster partition
    _pointInMem = sqrt(0.7 * _maxMemory / sizeof(RasterPoint));
    _pointInMem2 = pow(_pointInMem, 2);
    _sizeRaster = ceil(_pointInMem * Pointcloud2::CELL_SIZE_IN_M);
    _sizeRasterOverlap = ceil((1.0 - Pointcloud2::OVERLAP_RASTERS) *
        (double)_pointInMem * Pointcloud2::CELL_SIZE_IN_M);
    //vectors for neighbor search
    _neighborD = {
        (int)_pointInMem - 1,
        (int)_pointInMem + 1
        - (int)_pointInMem +1,
        - (int)_pointInMem - 1,
    };

    _neighbor = {
        -1,
        (int)_pointInMem,
        1,
        - (int)_pointInMem
    };

  }

  ~op_analyzeRaster() = default;

  std::shared_ptr<Operator> getOperator();

  size_t getPointInMem() const { return _pointInMem; };
  size_t getPointInMem2() const { return _pointInMem2; };
  size_t getPositionNewAttributes() const { return _posNewAttributes; };
  unsigned getSizeRaster() const { return _sizeRaster; };
  unsigned getSizeRasterOverlap() const { return _sizeRasterOverlap; };

  void setPointInMem(size_t pointInMem) {
    _pointInMem = pointInMem;
    _pointInMem2 = pow(_pointInMem, 2);
  };
  void setSizeRaster(unsigned sizeRaster) { _sizeRaster = sizeRaster; };
  void setSizeRasterOverlap(unsigned sizeRasterOverlap) {
    _sizeRasterOverlap = sizeRasterOverlap;
  };
  void setNeighbor(){
    _neighbor = {
        -1,
        (int)_pointInMem,
        1,
        -(int)_pointInMem
    };
  };
  void setNeighborD(){
    _neighborD = {
        - (int)_pointInMem - 1,
        (int)_pointInMem - 1,
        (int)_pointInMem + 1,
        - (int)_pointInMem + 1
    };
  };

  static RasterPoint
  secondoToStruct(RasterPointDB bufferDB);
  /**
   *calculate objects:
   *fill raster
   *compute objects
   *find local maxima, combine them to significant maxima
   *and split object by significant maxima
   */
   void
   calculateObjects(std::vector<SmiFileId>& rasters,
       std::vector<Rect3>& rastersBbox) const;

   /**
    *
    */
   inline std::tuple<double, double> getRasterSize() const
        {
     double x1 = _bb.MinD(0);
     double y1 = _bb.MinD(1);
     double x2 = _bb.MaxD(0);
     double y2 = _bb.MaxD(1);
     double distX = distance_m(x1, y1, x2, y1);
     double distY = distance_m(x1, y1, x1, y2);

     return std::tuple<double, double>(distX, distY);
        }

   /**
    *
    */
   inline std::tuple<double, double> getXDimension() const
        {
     return std::tuple<double, double>(_bb.MinD(0), _bb.MaxD(0));
        }

   /**
    *
    */
   inline std::tuple<double, double> getYDimension() const
        {
     return std::tuple<double, double>(_bb.MinD(1), _bb.MaxD(1));
        }


   /**
    *
   */
   inline double GRID_PER_M2 () const
   {
     return 1.0 / pow(Pointcloud2::CELL_SIZE_IN_M, 2);
   }


   /**
    *
    */
   inline unsigned getMemoryUsageOfRaster(
       const std::tuple<double, double> &rasterSize) const
   {
     double rasterX, rasterY;
     std::tie(rasterX, rasterY) = rasterSize;
     return ceil(rasterX *
         rasterY *
         GRID_PER_M2() *
         sizeof(RasterPoint));
   }

};
} // namespace pointcloud2
