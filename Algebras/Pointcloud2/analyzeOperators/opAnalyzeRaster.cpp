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



1 Pointcloud2 "AnalyzeRaster" Operator

They identify objects in pointcloud2 and
cluster them.
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
#include "opAnalyzeRaster.h"
#include "ARClassify.h"
#include <math.h>
#include <stack>
#include <set>
#include <bitset>

#include "NestedList.h"


using namespace pointcloud2;

extern NestedList *nl;

/*
1.1 Type Mapping

*/
ListExpr op_analyzeRaster::analyzeRasterTM(ListExpr args)
{

  if (!(nl->HasLength(args, 1) && Pointcloud2::TypeCheck(nl->First(args))))
  {
    return listutils::typeError("wrong input");
  }

  ListExpr cloudType = nl->First(args);

  ListExpr pc2Ref = nl->Second(cloudType);

  if ((nl->IsAtom(pc2Ref) && !nl->IsEqual(pc2Ref,"EUCLID",false))
      || (!nl->IsAtom(pc2Ref)
          && !nl->IsEqual(nl->First(pc2Ref),"EUCLID",false))) {
      return listutils::typeError("Ref has to be EUCLID - use projectUTM");
  }
  //intensity
  int posIntensity;
  if (!nl->IsAtom(pc2Ref)){
    ListExpr type = nl->TheEmptyList();
    ListExpr attrList = nl->Second(Pointcloud2::getTupleType(cloudType));
    posIntensity = listutils::findAttribute(attrList, "Intensity", type);
    //red, blue, green not realized
    //int posRed = listutils::findAttribute(attrList, "Red", type);
    //int posGreen = listutils::findAttribute(attrList, "Green", type);
    //int posBlue = listutils::findAttribute(attrList, "Blue", type);
  }
  else
  {
    posIntensity = 0;
  }

  ListExpr newCloudType, appendage;
  if (Pointcloud2::GET_RASTER_NOT_PC) {
      newCloudType = nl->TwoElemList(
          listutils::basicSymbol<Pointcloud2>(),
          nl->SymbolAtom(Referencesystem::toString(Referencesystem::EUCLID)));
      appendage = nl->FiveElemList(
          nl->TwoElemList(nl->SymbolAtom("Count"),
              listutils::basicSymbol<CcInt>()),
          nl->TwoElemList(nl->SymbolAtom("AltMin"),
              listutils::basicSymbol<CcReal>()),
          nl->TwoElemList(nl->SymbolAtom("AltMax"),
              listutils::basicSymbol<CcReal>()),
          nl->TwoElemList(nl->SymbolAtom("AltAvg"),
              listutils::basicSymbol<CcReal>()),
          nl->TwoElemList(nl->SymbolAtom("IntensityAvg"),
              listutils::basicSymbol<CcInt>())
      );
      cloudType = Pointcloud2::appendAttributesToCloud(newCloudType,
          appendage);
  }

  // we want to return the input cloud-type with these two attributes appended
  appendage = nl->TwoElemList(
      nl->TwoElemList(nl->SymbolAtom("ObjID"),
          listutils::basicSymbol<CcInt>()),
      nl->TwoElemList(nl->SymbolAtom("CatID"),
              listutils::basicSymbol<CcInt>()));

  newCloudType = Pointcloud2::appendAttributesToCloud(cloudType,
          appendage);

  std::cout<<nl->ToString(newCloudType)<<endl;

  //position of attributes ObjID and CatID
  int pos = 0;
  pos = nl->ListLength(
      nl->Second(
          Pointcloud2::getTupleType(newCloudType))) - 2;


  return nl->ThreeElemList(
      nl->SymbolAtom(Symbols::APPEND()),
      nl->TwoElemList(nl->IntAtom(pos), nl->IntAtom(posIntensity)),
      newCloudType);
}

/*
1.2 Value Mapping

*/
int op_analyzeRaster::analyzeRasterVM(Word *args, Word &result, int message,
    Word &local, Supplier s)
{

  result = qp->ResultStorage(s);
  Pointcloud2 *res = static_cast<Pointcloud2 *>(result.addr);

  Pointcloud2 *pc2Source =
      static_cast<Pointcloud2 *>(args[0].addr);

  //starting position of new attributes
  CcInt *posAppend = static_cast<CcInt *>(args[1].addr);
  const size_t pos = posAppend->GetIntval();

  //position of intensity
  const size_t posIntensity = static_cast<CcInt *>(args[2].addr)->GetIntval();

  //calculate memory structure
  const size_t maxMemory = qp->GetMemorySize(s) * 1024 * 1024;

  const ListExpr resultType = GetTupleResultType(s);

  op_analyzeRaster analyzeRaster(pc2Source, maxMemory,
      pos, posIntensity, resultType);

  //raster with overlapping OVERLAP
  using namespace pointcloud2::analyzeRasterConstants;

  //resize raster if complete pc2 fits in memory
  double distX = analyzeRaster._bb.MaxD(0) - analyzeRaster._bb.MinD(0);
  double distY = analyzeRaster._bb.MaxD(1) - analyzeRaster._bb.MinD(1);
  {
    //tie(distX, distY) = rasterSize;
    if (analyzeRaster.getSizeRaster() > std::max(ceil(distX), ceil(distY)))
    {
      const int newSizeRaster = std::max(ceil(distX), ceil(distY));
      analyzeRaster.setSizeRaster(newSizeRaster);
      analyzeRaster.setSizeRasterOverlap(analyzeRaster.getSizeRaster());

      const size_t newPointInMem = ceil(newSizeRaster /
          Pointcloud2::CELL_SIZE_IN_M);
      analyzeRaster.setPointInMem(newPointInMem);
    }
  }
  analyzeRaster.setNeighbor(); analyzeRaster.setNeighborD();

  //statistics about analyzeRaster
  std::cout<<"Number of points of source pointcloud2: "
        <<pc2Source->getPointCount()<<endl;
  std::cout<<"Grid: "<<Pointcloud2::CELL_SIZE_IN_M<<", ";
  std::cout<<"Point density in points/cell: "
      <<pc2Source->getPointCount() /
      ((distX/Pointcloud2::CELL_SIZE_IN_M) *
          (distY/Pointcloud2::CELL_SIZE_IN_M))<<endl;
  Rectangle<3> bboxSource = pc2Source->getBoundingBox();
  std::cout<<"Size of Pc2 in Grids: ";
  std::cout<<(bboxSource.MaxD(0) - bboxSource.MinD(0)) /
      Pointcloud2::CELL_SIZE_IN_M<<" x ";
  std::cout<<(bboxSource.MaxD(1) - bboxSource.MinD(1)) /
      Pointcloud2::CELL_SIZE_IN_M<<endl;
  std::cout<<"Raster in Memory: "<<analyzeRaster._sizeRaster<<endl;
  std::cout<<"Using "<<ceil((bboxSource.MaxD(0) - bboxSource.MinD(0)) /
      analyzeRaster._sizeRasterOverlap)<<" x "
          <<ceil((bboxSource.MaxD(1) - bboxSource.MinD(1)) /
              analyzeRaster._sizeRasterOverlap)
              <<" temporary rasters"<<endl;


  //IDs/Bbox of persisted temporary rasters
  std::vector<SmiFileId> rasters;
  std::vector<Rect3> rastersBbox;

  //build raster and calculate objects
  try{
    analyzeRaster.calculateObjects(rasters, rastersBbox);
  }catch(const std::runtime_error&){
    std::cout<<"error"<<endl;
    return 0;
  }

  //classify objects and find double objects
  std::shared_ptr<std::unordered_map<int,int>> duples =
                      std::make_shared<std::unordered_map<int,int>>();
  std::shared_ptr<std::unordered_map<int,size_t>> classMap =
                      ARClassify::classify(rasters, rastersBbox,
                              analyzeRaster.getPointInMem(), duples);

  //write pc2 back
  analyzeRaster.saveRasterToPc2(rasters, rastersBbox, classMap, duples, res);

  rastersBbox.clear();
  rasters.clear();
  if (local.addr)
  {
    local.addr = nullptr;
  }
  //DEBUG
  std::cout<<"Number of points of result pointcloud2: "
        <<res->getPointCount()<<endl;
  return 0;
}

std::string op_analyzeRaster::getOperatorSpec()
{
  return OperatorSpec(
      " pointcloud2 -> pointcloud2(...,ObjID,CatID)",
      " pc2 analyzeRaster ",
      " Returns classified pc2 ",
      " query pc2 analyzeRaster")
      .getStr();
}

std::shared_ptr<Operator> op_analyzeRaster::getOperator()
{
  return std::make_shared<Operator>("analyzeRaster",
      getOperatorSpec(),
      &op_analyzeRaster::analyzeRasterVM,
      Operator::SimpleSelect,
      &op_analyzeRaster::analyzeRasterTM);
}

/*
1.3 calculate objects
fill raster
compute objects
find local maxima, combine them to significant maxima
and split object by significant maxima

*/
void 
op_analyzeRaster::calculateObjects(std::vector<SmiFileId>& rasters,
    std::vector<Rect3>& rastersBbox) const{
  std::tuple<double, double> &&xDimension = getXDimension();
  std::tuple<double, double> &&yDimension = getYDimension();

  int areaNumber = 0;

  std::cout<<"Start calc Rasters: "<<endl;
  std::cout<<"X: "<<floor(std::get<0>(xDimension))<<
      "-"<<ceil(std::get<1>(xDimension))<<endl;
  std::cout<<"Y: "<<floor(std::get<0>(yDimension))<<
      "-"<<ceil(std::get<1>(yDimension))<<endl;

  double y = std::get<0>(yDimension);
  for (; y < std::get<1>(yDimension); y += _sizeRasterOverlap)
  {
    double x = std::get<0>(xDimension);
    for (; x < std::get<1>(xDimension); x += _sizeRasterOverlap)
    {
      //calculate single raster partition
      auto pc2RasterPart = calculateRasterRartition(x, y);

      rastersBbox.push_back(pc2RasterPart->getBoundingBox());

      //memory structure of raster partition
      std::vector<RasterPoint> rasterMem =
          std::move(fillrastermem(*pc2RasterPart, x, y));

      //flooding & calculation ground & split
      int maxObjectN = flooding(rasterMem, areaNumber, rastersBbox.back());

      //init SMI file for temp raster
      const bool makefixed = true;
      const bool temporary = false;
      SmiSize reclen = sizeof(RasterPointDB);
      SmiRecordFile rf(makefixed, reclen, temporary);

      if (rf.Create())
      {
        std::cout << "temporary SMI_Raster initialized" << endl;
      }
      else
      {
        std::cout << "error initializing temporary SMI_Raster" << endl;
        throw std::runtime_error("error initializing temporary SMI_Raster");
      }

      rasters.push_back(rf.GetFileId());

      //save raster partition to smi-file
      size_t arrayCount = 0;
      for (; arrayCount < _pointInMem2; arrayCount++)
      {
        SmiRecord r;
        SmiRecordId rid;
        RasterPoint bufferRM = rasterMem[arrayCount];
        RasterPointDB buffer = structToSecondo(bufferRM);
        rf.AppendRecord(rid, r);
        r.Write(buffer);
      }
      rf.Close(rf.GetFileId());
      areaNumber = maxObjectN;
    }
  }
}


/*
1.4 fill raster in memory
calculate raster points and their attributes
from partition of the pointcloud2 in memory raster

*/
std::vector<RasterPoint> 
op_analyzeRaster::fillrastermem(Pointcloud2& pc2RasterPart, double x, double y)
const {
  double grid_m = Pointcloud2::CELL_SIZE_IN_M;

  //scan raster cell in pc2
  PcPoint pcPoint;
  SmiRecordFileIterator *it = pc2RasterPart.getFileIterator();
  SmiRecord record;

  std::vector<RasterPoint> rasterMem(_pointInMem2);

  while (it->Next(record))
  {
    pc2RasterPart.getPoint(record.GetId(), &pcPoint);
    //position in raster array
    unsigned int partX = floor((pcPoint._x - x) / grid_m);
    unsigned int partY = floor((pcPoint._y - y) / grid_m);
    unsigned int arrayCount = partX + _pointInMem * partY;
    if (partX >= _pointInMem || partY >= _pointInMem ||
        partX < 0 || partY < 0){
      continue;
    }

    rasterMem[arrayCount].objectID = -1;
    rasterMem[arrayCount].classID = -1;

    rasterMem[arrayCount].count++;
    if (rasterMem[arrayCount].initialized)
    {
      if (rasterMem[arrayCount].minAlt > pcPoint._z)
      {
        rasterMem[arrayCount].minAlt = pcPoint._z;
      }
    }
    else
    {
      rasterMem[arrayCount].minAlt = pcPoint._z;
    }

    if (rasterMem[arrayCount].initialized)
    {
      if (rasterMem[arrayCount].maxAlt < pcPoint._z)
      {
        rasterMem[arrayCount].maxAlt = pcPoint._z;
      }
    }
    else
    {
      rasterMem[arrayCount].maxAlt = pcPoint._z;
    }

    rasterMem[arrayCount].averageAlt =
        (rasterMem[arrayCount].averageAlt *
            (rasterMem[arrayCount].count - 1) + 
            pcPoint._z) / 
            rasterMem[arrayCount].count;

    if (_posIntensity > 0)
    {
      Tuple *elem = pc2RasterPart.getTuple(pcPoint._tupleId);
      CcInt *intensityCcInt = (CcInt*) (elem->GetAttribute(_posIntensity - 1));
      int intensity = intensityCcInt->GetValue();
      rasterMem[arrayCount].averageIntensity =
          (rasterMem[arrayCount].averageIntensity *
              (rasterMem[arrayCount].count - 1) +
              intensity) /
              rasterMem[arrayCount].count;
      elem->DeleteIfAllowed();
    }
  }
  delete it;

  //outside of pc2 initialized = false
  Rect3 bboxPart = pc2RasterPart.getBoundingBox();
  unsigned int partX = floor((bboxPart.MaxD(0) - x) / grid_m) + 1;
  unsigned int partY = floor((bboxPart.MaxD(1) - y) / grid_m) + 1;
  if (partX < _pointInMem)
  {
    for (unsigned int x = partX; x < _pointInMem; x++)
    {
      for (unsigned int y = 0; y < _pointInMem; y++)
      {
        unsigned int arrayCount = x + _pointInMem * y;
        rasterMem[arrayCount].initialized = false;
      }
    }
  }
  if (partY < _pointInMem)
    {
      for (unsigned int arrayCount =  _pointInMem * partY;
          arrayCount < _pointInMem2; arrayCount++)
      {
        rasterMem[arrayCount].initialized = false;
      }
    }
  return rasterMem;
}

/*
1.5 flooding
calculate objects by flooding
call methods
calculate local maxima
calculate significant maxima
split objects

*/
int
op_analyzeRaster::flooding(std::vector<RasterPoint>& rasterMem,
    int areaNumber, const Rect3 rasterBbox) const{
  //bbox of right and upper overlapping
  const double offset = ceil(_sizeRasterOverlap / Pointcloud2::CELL_SIZE_IN_M);
  Rect rightOverlap;
  Rect upperOverlap;
  double min[] = {offset, 0.0};
  double max[] = {_pointInMem + 0., _pointInMem + 0.};
  rightOverlap.Set(true, min, max);
  min[0] = 0.0; min[1] = offset;
  upperOverlap.Set(true, min, max);
  bool xEdge = (rasterBbox.MaxD(0) >= _bb.MaxD(0));
  bool yEdge = (rasterBbox.MaxD(0) >= _bb.MaxD(0));

  //flooding
  for (size_t arrayCount = 0; arrayCount < _pointInMem2;
      arrayCount++)
  {
    if (rasterMem[arrayCount].objectID < 0 &&
        rasterMem[arrayCount].initialized)
    {
      areaNumber++;
      //ground turns false if one edge cell has lower neighbor
      bool ground = true;
      //edge turns 0 if object is open to edge min cells
      size_t edge = Pointcloud2::MAX_CELLS_AT_EDGE;

      double min[] = {(arrayCount % _pointInMem) + 0.,
          (arrayCount / _pointInMem) + 0.};
      double max[] = {(arrayCount % _pointInMem) + 0.,
          (arrayCount / _pointInMem) + 0.};
      Rect bboxObj;
      bboxObj.Set(true, min, max);

      size_t cellCount = 0;
      std::stack<size_t> s;
      s.emplace(arrayCount);

      while (!s.empty())
      {
        size_t nF = s.top();
        s.pop();
        recalcBbox(bboxObj, nF);
        ++cellCount;
        rasterMem[nF].objectID = areaNumber;
        checkNeighbors(s, rasterMem, _neighbor, nF, ground, edge);

        //also diagonal neighbors
        if (_pc2->NEIGHBOUR_CELLS)
        {
          checkNeighbors(s, rasterMem, _neighborD, nF, ground, edge);
        }
      }

      //ground? open to edge? fill objectID & classID with 0 for ground;
      //objects up to 2 points are ground, too
      const size_t min_ground = Pointcloud2::MIN_GROUND_CELLS;
      if ((ground && (cellCount >= min_ground )) || edge == 0 || cellCount <= 2)
      {
        constexpr int areaNumberGround = 0;
        deleteAreaNumber(rasterMem, areaNumber, bboxObj, areaNumberGround);
        --areaNumber;
      }
      else if (cellCount < _pc2->MIN_OBJ_SIZE){
        constexpr int areaNumberNoObj = -1;
        deleteAreaNumber(rasterMem, areaNumber, bboxObj, areaNumberNoObj);
        --areaNumber;
      }
      //erase double objects
      else if ((!xEdge && rightOverlap.Contains(bboxObj)) ||
          (!yEdge && upperOverlap.Contains(bboxObj)))
      {
        constexpr int areaNumberGround = 0;
        deleteAreaNumber(rasterMem, areaNumber, bboxObj, areaNumberGround);
        --areaNumber;
      }
      else
      {
        if (_pc2->SPLIT){
          //find local maximum
          int areaNumberM = calcLocMax(rasterMem, areaNumber, bboxObj);

          //merge local maxima
          calcEdgeForMerge(rasterMem, areaNumber, bboxObj);

          //split object
          splitObjects(rasterMem, areaNumber, bboxObj);

          //start counting with max LocMax
          areaNumber = areaNumberM;
        }
      }
    }
  }
return areaNumber;
}

/*
1.6 calculate local maxima in one object

*/
int
op_analyzeRaster::calcLocMax(std::vector<RasterPoint>& rasterMem,
    int areaNumber, Rect bboxObj) const{
  int areaNumberM= areaNumber;

  //local maxima
  for (size_t y = floor(bboxObj.MinD(1)); y <= ceil(bboxObj.MaxD(1)); y++){
    for (size_t x = floor(bboxObj.MinD(0)); x <= ceil(bboxObj.MaxD(0)); x++){
      size_t arrayCount = y * _pointInMem + x;
      //test only cell being part of current object
      if (rasterMem[arrayCount].objectID == areaNumber){
        bool found = true;
        size_t edge = 0;
        ++areaNumberM;
        std::stack<size_t> s;
        s.push(arrayCount);

        double min[] = {x + 0., y + 0.};
        double max[] = {x + 0., y + 0.};
        Rect bboxLoc;
        bboxLoc.Set(true, min, max);
        size_t cellCount = 0;

        while (!s.empty())
        {
          size_t nF = s.top();
          recalcBbox(bboxLoc, nF);
          s.pop();

          ++cellCount;

          rasterMem[nF].objectID = areaNumberM;

          checkNeighborsMaximum(s, rasterMem, _neighbor, nF,
              found, edge, areaNumber);

          //also diagonal neighbors
          if (_pc2->NEIGHBOUR_CELLS)
          {
            size_t noCoundEdge;
            checkNeighborsMaximum(s, rasterMem, _neighborD, nF,
                found, noCoundEdge, areaNumber);
          }
          //no local maximum or nearly complete object is local maximum
          if (!found){
            //delete stack
            std::stack<size_t> e;
            swap(e,s);
          }
        }
        if (!found || edge <= analyzeRasterConstants::MIN_NEIGBOR_LOC_MAX ||
            analyzeRasterConstants::LOC_MAX_NEARLY_OBJ  * bboxObj.Area()
            <= bboxLoc.Area() || cellCount < _pc2->MIN_LOC_MAX_SIZE)
        {
          deleteAreaNumber(rasterMem, areaNumberM, bboxLoc, areaNumber);
          --areaNumberM;
        }
      }
    }//end for
  }//end for
  return areaNumberM;
}

/*
1.7 calculate edge cells of local maxima
and call method for merging local maxima to significant maximum

*/
void
op_analyzeRaster::calcEdgeForMerge(std::vector<RasterPoint>& rasterMem,
    int areaNumber, Rect bboxObj) const{
  for (size_t y = floor(bboxObj.MinD(1)); y <= ceil(bboxObj.MaxD(1)); y++){
    for (size_t x = floor(bboxObj.MinD(0)); x <= ceil(bboxObj.MaxD(0)); x++){
      size_t arrayCount = y * _pointInMem + x;
      if (rasterMem[arrayCount].objectID > areaNumber){
        //test only edge points of local maxima
        constexpr int arraySize = 4;
        bool edge = false;
        for (int i = 0; i < arraySize; ++i)
        {
          size_t arrayCountNew = _neighbor[i] + arrayCount;
          size_t xNew = arrayCountNew % _pointInMem;
          if (arrayCountNew >= 0 && arrayCountNew < _pointInMem2 &&
              xNew < _pointInMem && rasterMem[arrayCountNew].objectID !=
                  rasterMem[arrayCount].objectID  &&
                  rasterMem[arrayCountNew].objectID >= areaNumber){
            edge = true;
          }
        }
        //test for local maximum
        if (edge){
          //compute merge with Horn algorithm for circles
          //fill gap with Gift-Wrapping algorithm (convex hull)
          mergeLocMax(rasterMem, arrayCount, areaNumber);
        }
      }
    }
  }
}

/*
1.8 split objects by significant maxima

*/
void
op_analyzeRaster::splitObjects(std::vector<RasterPoint>& rasterMem,
    int areaNumber, Rect bboxObj) const{
  constexpr int arraySize = 4;
  //data structure for edge points of local maxima and directions to expand
  std::vector<std::tuple<size_t, int>> edgePoints;
  do{
    //fill vector
    edgePoints.clear();
    for (size_t y = floor(bboxObj.MinD(1)); y <= ceil(bboxObj.MaxD(1)); y++)
    {
      for (size_t x = floor(bboxObj.MinD(0)); x <= ceil(bboxObj.MaxD(0)); x++)
      {
        size_t arrayCount = y * _pointInMem + x;
        //cell of local maximum
        if (rasterMem[arrayCount].objectID > areaNumber){
          for (size_t i = 0; i < arraySize; i++)
          {
            int nCell = _neighbor[i] + arrayCount;
            size_t nCellX = nCell % _pointInMem;
            size_t nCellY = nCell / _pointInMem;
            if (nCellX < _pointInMem && nCellY < _pointInMem &&
                rasterMem[nCell].objectID == areaNumber &&
                (rasterMem[nCell].maxAlt < rasterMem[arrayCount].maxAlt ||
                testForOtherMin(rasterMem, arrayCount,
                    _neighbor[i], areaNumber)))
            {
              edgePoints.emplace_back(arrayCount, _neighbor[i]);
            }
            nCell = _neighborD[i] + arrayCount;
            nCellX = nCell % _pointInMem;
            nCellY = nCell / _pointInMem;
            if (nCellX < _pointInMem && nCellY < _pointInMem &&
                rasterMem[nCell].objectID == areaNumber)
            {
              //diagonal only pointed corners
              int bi = (i == 0) ? 3 : (i - 1);
              int ai = i;
              if (_neighbor[ai] + arrayCount < _pointInMem2 &&
                  _neighbor[bi] + arrayCount < _pointInMem2 &&
                  rasterMem[_neighbor[bi] + arrayCount].objectID -
                  rasterMem[_neighbor[ai] + arrayCount].objectID == 0 &&
                  (rasterMem[nCell].maxAlt < rasterMem[arrayCount].maxAlt ||
                  testForOtherMin(rasterMem, arrayCount,
                      _neighbor[i], areaNumber)))
              {
                edgePoints.emplace_back(arrayCount, _neighborD[i]);
              }
            }
          }
        }
      }
    }

    //increase local maxima to find split lines
    for (size_t i = 0; i < edgePoints.size(); i++)
    {
      size_t cellOld = std::get<0>(edgePoints[i]);
      int xNew = std::get<1>(edgePoints[i]) 
               + std::get<0>(edgePoints[i]) % _pointInMem;
      int yNew = std::get<1>(edgePoints[i]) 
               + std::get<0>(edgePoints[i]) / _pointInMem;
      size_t cellNew = std::get<1>(edgePoints[i]) + std::get<0>(edgePoints[i]);
      if ((xNew < 0 && xNew >= (int)_pointInMem &&
          yNew < 0 && yNew >= (int)_pointInMem )||
          rasterMem[cellNew].objectID != areaNumber)
      {
        edgePoints.erase(edgePoints.begin() + i);
      }
      else
      {
        if (rasterMem[cellNew].objectID == areaNumber){
          rasterMem[cellNew].objectID = rasterMem[cellOld].objectID;
        }
      }
    }
  }while (!edgePoints.empty());
}

/*
1.9 test for other minimum
test if between count and other locale maximum lower minimum
or if reaching the edge
true if found

*/
bool
op_analyzeRaster::testForOtherMin(std::vector<RasterPoint>& rasterMem,
    size_t arrayCount, int neighbor, int areaNumber) const{
  bool found = false;
  int count = 1;
  double firstMin = rasterMem[arrayCount].maxAlt;
  while (!found &&
      arrayCount + count * neighbor < _pointInMem2 &&
      rasterMem[arrayCount +
                count * neighbor].objectID <= areaNumber)
  {
    if (rasterMem[arrayCount +
                  count * neighbor].maxAlt <= firstMin ||
        rasterMem[arrayCount +
                        (count) * neighbor].objectID <= 0){
      found = true;
    }
    ++count;

  }
  return found;
}


void 
op_analyzeRaster::checkNeighbors(std::stack<size_t>& s,
    std::vector<RasterPoint>& rasterMem,
    std::vector<int> neighbor, size_t nF, bool& ground, size_t& edge) const{
  const double delta_alt = _pc2->DELTA_ALT_IN_M;
  constexpr int arraySize = 4;

  for (int i = 0; i < arraySize; i++)
  {
    int nN = neighbor[i] + nF;
    const int x = nN % _pointInMem;
    const int y = nN / _pointInMem;
    if (x >= 0 && x < (int)_pointInMem &&
        y >= 0 && y < (int)_pointInMem && rasterMem[nN].initialized)
    {
      const auto absAverange = std::abs(rasterMem[nN].maxAlt -
          rasterMem[nF].maxAlt);
      if (rasterMem[nN].objectID < 0 &&
          absAverange < delta_alt)
      {
        s.emplace(nN);
      }
      //ground: neighbor of edge cell must not be lower altitude
      else {
        if ((rasterMem[nF].maxAlt - rasterMem[nN].maxAlt)
            > delta_alt)
        {
          ground = false;
        }
      }
    }
    else{
      //objects open to edge are ground if more the EDGE_MAX edge cells
      if (edge >= 1){
        --edge;
      }
    }
  }
}

/*
1.12 check neighbor cells for flooding to find local maxima
definition for new neighbor cell
is part of object and all cells have same height roughly
is higher than all neighbor cells
has minimum of three neighbor cells

*/
void 
op_analyzeRaster::checkNeighborsMaximum(std::stack<size_t>& s,
    std::vector<RasterPoint>& rasterMem, std::vector<int> neighbor,
    size_t nF, bool& found, size_t& edge, int const areaNumber) const{
  const double threshold_loc = _pc2->THRESHOLD_MERGE;
  constexpr int arraySize = 4;

  for (int i = 0; i < arraySize; i++)
  {
    int nN = neighbor[i] + nF;
    int x = nN % _pointInMem;
    int y = nN / _pointInMem;
    if (x >= 0 && x < (int)_pointInMem && y >= 0 && y < (int)_pointInMem &&
        rasterMem[nN].objectID == areaNumber)
    {
      const auto absAverange = std::abs(rasterMem[nN].maxAlt -
          rasterMem[nF].maxAlt);
      if (absAverange <= threshold_loc)
      {
        s.push(nN);
      }
      //local max are higher than all neighbours
      else {
        ++ edge;
        if ((rasterMem[nF].maxAlt - rasterMem[nN].maxAlt)
            < 0)
        {
          found = false;
        }
      }
    }
  }
}

/*
1.13 delete object
overwrite one areaNumber with other areaNumber

*/
void
op_analyzeRaster::deleteAreaNumber(std::vector<RasterPoint>& rasterMem,
    int areaNumber, Rect bboxLoc, int areaNumberOld) const{

  for (size_t y = floor(bboxLoc.MinD(1)); y <= ceil(bboxLoc.MaxD(1)); y++){
    for (size_t x = floor(bboxLoc.MinD(0)); x <= ceil(bboxLoc.MaxD(0)); x++){
      size_t arrayCount = y * _pointInMem + x;
      if (rasterMem[arrayCount].objectID == areaNumber){
        rasterMem[arrayCount].objectID = areaNumberOld;
      }
    }
  }
}

/*
1.14 merge local maxima

*/
void op_analyzeRaster::mergeLocMax(std::vector<RasterPoint>& rasterMem,
    size_t const arrayCount, int const areaNumber) const{
  const int areaNumberLocMax = rasterMem[arrayCount].objectID;

  //initialize data structure for all points to be merged with local maximum
  std::vector<std::pair <size_t,size_t>> newLocMax =
      std::move(fillVectorWithLocMax(rasterMem, arrayCount));

  bool countLocMax =
      calcNearLocMaxByHorn(rasterMem, arrayCount, areaNumber, newLocMax);

  //fill gap by Gift-Wrapping algorithm (convex hull)
  if (countLocMax){
    std::vector<std::pair <size_t,size_t>> hullLocMax =
        std::move(calcConvexHull(newLocMax));

    //fill convex hull to compute significant maximum by edge list algorithm
    fillConvexHull(rasterMem, hullLocMax, areaNumberLocMax);
  }
}

/*
1.15 fill vector with local maxima
preparation for calculating convex hull

*/
std::vector<std::pair <size_t,size_t>>
op_analyzeRaster::fillVectorWithLocMax(std::vector<RasterPoint>& rasterMem,
    size_t const arrayCount) const{
  const int areaNumberLocMax = rasterMem[arrayCount].objectID;

  const size_t x_0 = arrayCount % _pointInMem;
  const size_t y_0 = arrayCount / _pointInMem;

  //data structure for all points of merged locMax
  std::vector<std::pair <size_t,size_t>> newLocMax;
  newLocMax.emplace_back(x_0, y_0);

  bool foundP;
  int delta = 0;
  do
  {
    ++delta;
    foundP = false;
    for (int i = - delta; i <= + delta; i++){
      size_t count;
      if ((((int)x_0 + i) >= 0) && ((x_0 + i) < _pointInMem)){
        if ((y_0 + delta) < _pointInMem){
          count = (y_0 + delta) * _pointInMem + (x_0 + i);
          if (rasterMem[count].objectID == areaNumberLocMax){
            newLocMax.emplace_back(x_0 + i, y_0 + delta);
            foundP = true;
          }
        }
        if (((int)y_0 - delta) >= 0){
          count = (y_0 - delta) * _pointInMem + (x_0 + i);
          if (rasterMem[count].objectID == areaNumberLocMax){
            newLocMax.emplace_back(x_0 + i, y_0 - delta);
            foundP = true;
          }
        }
      }
      //no double corners
      if (delta != abs(i) && (((int)y_0 + i) >= 0)
          && ((y_0 + i) < _pointInMem)){
        if ((x_0 + delta) < _pointInMem){
          count = (y_0 + i) * _pointInMem + (x_0 + delta);
          if (rasterMem[count].objectID == areaNumberLocMax){
            newLocMax.emplace_back(x_0 + delta, y_0 + i);
            foundP = true;
          }
        }
        if (((int)x_0 - delta) >= 0){
          count = (y_0 + i) * _pointInMem + (x_0 - delta);
          if (rasterMem[count].objectID == areaNumberLocMax){
            newLocMax.emplace_back(x_0 - delta, y_0 + i);
            foundP = true;
          }
        }
      }
    }//end for
  }//end do
  while (foundP);
  return newLocMax;
}

/*
1.16 add local maximum to be merged to vector
preparation for calculating convex hull

*/
void
op_analyzeRaster::fillVectorWithOtherLocMax(std::vector<RasterPoint>& rasterMem,
    const size_t arrayCount, const int areaNumberLocMax,
    std::vector<std::pair <size_t,size_t>>& newLocMax) const{
  const int areaNumberLocMaxOther = rasterMem[arrayCount].objectID;

  const size_t x_0 = arrayCount % _pointInMem;
  const size_t y_0 = arrayCount / _pointInMem;

  newLocMax.emplace_back(x_0, y_0);
  rasterMem[arrayCount].objectID = areaNumberLocMax;

  bool foundP;
  int delta = 0;
  do
  {
    ++delta;
    foundP = false;
    for (int i = - delta; i <= + delta; i++){
      size_t count;
      if ((((int)x_0 + i) >= 0) && ((x_0 + i) < _pointInMem)){
        if ((y_0 + delta) < _pointInMem){
          count = (y_0 + delta) * _pointInMem + (x_0 + i);
          if (rasterMem[count].objectID == areaNumberLocMaxOther){
            newLocMax.emplace_back(x_0 + i, y_0 + delta);
            rasterMem[count].objectID = areaNumberLocMax;
            foundP = true;
          }
        }
        if (((int)y_0 - delta) >= 0){
          count = (y_0 - delta) * _pointInMem + (x_0 + i);
          if (rasterMem[count].objectID == areaNumberLocMaxOther){
            newLocMax.emplace_back(x_0 + i, y_0 - delta);
            rasterMem[count].objectID = areaNumberLocMax;
            foundP = true;
          }
        }
      }
      //no double corners
      if (delta != abs(i) && (((int)y_0 + i) >= 0)
          && ((y_0 + i) < _pointInMem)){
        if ((x_0 + delta) < _pointInMem){
          count = (y_0 + i) * _pointInMem + (x_0 + delta);
          if (rasterMem[count].objectID == areaNumberLocMaxOther){
            newLocMax.emplace_back(x_0 + delta, y_0 + i);
            rasterMem[count].objectID = areaNumberLocMax;
            foundP = true;
          }
        }
        if (((int)x_0 - delta) >= 0){
          count = (y_0 + i) * _pointInMem + (x_0 - delta);
          if (rasterMem[count].objectID == areaNumberLocMaxOther){
            newLocMax.emplace_back(x_0 - delta, y_0 + i);
            rasterMem[count].objectID = areaNumberLocMax;
            foundP = true;
          }
        }
      }
    }//end for
  }//end do
  while (foundP);
}

/*
1.17 compute local maxima to be merged
by Horn algorithm for drawing circles
Horn algorithm is a derivation of Bresenham

*/
bool
op_analyzeRaster::calcNearLocMaxByHorn(std::vector<RasterPoint>& rasterMem,
    size_t const arrayCount, int const areaNumber,
    std::vector<std::pair <size_t,size_t>>& newLocMax) const{
  bool countLocMax = false;

  //max. distance next local maximum in cells
  const int r = ceil(_pc2->DISTANCE_SIG_MAXIMA
      / _pc2->CELL_SIZE_IN_M);

  const int areaNumberLocMax = rasterMem[arrayCount].objectID;

  const int x_0 = arrayCount % _pointInMem;
  const int y_0 = arrayCount / _pointInMem;
  int x = 0;
  int y = r;

  //test in x direction -R, R
  testOtherLocMax(rasterMem, ((x_0 - r < 0) ? 0 : (size_t)(x_0 - r)),
      (((size_t)(x_0 + r) > _pointInMem) ? _pointInMem : (size_t)(x_0 + r)),
      (size_t)y_0, areaNumber, areaNumberLocMax, countLocMax, newLocMax);
  //test y = R, -R
  if (y_0 - r >= 0){
    size_t arrayCount = (size_t)(x_0 + (y_0 - r) * _pointInMem);
    if (rasterMem[arrayCount].objectID > areaNumber
        && rasterMem[arrayCount].objectID != areaNumberLocMax){
      rasterMem[arrayCount].objectID = areaNumberLocMax;
      countLocMax = true;
      newLocMax.emplace_back((size_t)x_0, (size_t)(y_0 - r));
    }
  }
  if ((size_t)(y_0 + r) < _pointInMem){
    size_t arrayCount = (size_t)(x_0 + (y_0 + r) * _pointInMem);
    if (rasterMem[arrayCount].objectID > areaNumber
        && rasterMem[arrayCount].objectID != areaNumberLocMax){
      rasterMem[arrayCount].objectID = areaNumberLocMax;
      countLocMax = true;
      newLocMax.emplace_back((size_t)x_0, (size_t)(y_0 + r));
    }
  }

  int f = 1 - r;
  while (x < y){
    ++x;
    if (f < 0){
      f += 2 * x -1;
    } else{
      f += 2 * (x - y);
      --y;
    }

    //four tests for computed lines in x direction in circle (r)
    if ((size_t)(y + y_0) < _pointInMem){
      testOtherLocMax(rasterMem,
          (((x + x_0 - r) < 0) ? 0 : (size_t)(x + x_0 - r)),
          (((size_t)(x + x_0 + r) > _pointInMem)
              ? _pointInMem : (size_t)(x + x_0 + r)),
              (size_t)(y + y_0),
              areaNumber, areaNumberLocMax, countLocMax, newLocMax);
    }
    if ((y_0 - y) >= 0){
      testOtherLocMax(rasterMem,
          (((x_0 + x - r) < 0) ? 0 : (size_t)(x_0 + x - r)),
          (((size_t)(x_0 + x + r) > _pointInMem)
              ? _pointInMem : (size_t)(x_0 + x + r)),
              (size_t)(y_0 - y),
              areaNumber, areaNumberLocMax, countLocMax, newLocMax);
    }
    if ((size_t)(x + y_0) < _pointInMem){
      testOtherLocMax(rasterMem,
          (((y + x_0 - r) < 0) ? 0 : (size_t)(y + x_0 - r)) ,
          (((size_t)(y + x_0 + r) > _pointInMem)
              ? _pointInMem : (size_t)(y + x_0 + r)),
              (size_t)(x + y_0),
              areaNumber, areaNumberLocMax, countLocMax, newLocMax);
    }
    if ((y_0 - x) >= 0){
      testOtherLocMax(rasterMem,
          (((y + x_0 - r) < 0) ? 0 : (size_t)(y+ x_0 - r)),
          (((size_t)(y + x_0 + r) > _pointInMem)
              ? _pointInMem : (size_t)(y + x_0 + r)),
              (size_t)(y_0 - x),
              areaNumber, areaNumberLocMax, countLocMax, newLocMax);
    }
  }
  return countLocMax;
}

/*
1.18 calculate convex hull
by Gift Wrapping algorithm
to merge local maxima to significant maximum

*/
std::vector<std::pair <size_t,size_t>>
op_analyzeRaster::calcConvexHull(
    const std::vector<std::pair <size_t,size_t>> newLocMax) const{
  //leftmost point
  size_t firstpoint = 0;
  size_t yFirst = newLocMax[0].second;
  size_t n = newLocMax.size();
  bool sameY = true;
  for (size_t i = 0; i < n; i++){
    if ((std::get<0>(newLocMax[i]) < std::get<0>(newLocMax[firstpoint]))){
      firstpoint = i;
    } else if (((std::get<0>(newLocMax[i]) ==
        std::get<0>(newLocMax[firstpoint])))
        && (std::get<1>(newLocMax[i]) < std::get<1>(newLocMax[firstpoint]))){
      firstpoint = i;
    }
    if (newLocMax[i].second != yFirst){
      sameY = false;
    }
  }
  std::vector<std::pair <size_t,size_t>> hullLocMax;

  if (!sameY && n > 3){
    size_t current = firstpoint;
    hullLocMax.emplace_back(newLocMax[firstpoint].first,
        newLocMax[firstpoint].second);
    std::vector<size_t> collinear;
    size_t nextTarget;
    while (true) {
      nextTarget = 0;
      for (size_t i = 0; i < n; i++){
        if (i == current){
          continue;
        }
        int orientation =
            ((int)newLocMax[current].second
            - (int)newLocMax[i].second) *
            ((int)newLocMax[current].first
                - (int)newLocMax[nextTarget].first) -
            ((int)newLocMax[current].first
                - (int)newLocMax[i].first) *
            ((int)newLocMax[current].second
                - (int)newLocMax[nextTarget].second);
        if (orientation > 0){
          nextTarget = i;
          collinear.clear();
        }
        else if (orientation == 0)
        {
          if(distanceComp(newLocMax[current],
              newLocMax[nextTarget], newLocMax[i]) < 0) {
            collinear.push_back(nextTarget);
            nextTarget = i;
          }else{
            collinear.push_back(i);
          }
        }
      }
      for (auto coll : collinear){
        hullLocMax.emplace_back(newLocMax[coll].first, newLocMax[coll].second);
      }
      if (nextTarget == firstpoint){
        break;
      }
      hullLocMax.emplace_back(newLocMax[nextTarget].first,
           newLocMax[nextTarget].second);
      current = nextTarget;
    }
    hullLocMax.emplace_back(newLocMax[nextTarget].first,
         newLocMax[nextTarget].second);
  }
  else
  {
    for (size_t i = 0; i < newLocMax.size(); i++){
      hullLocMax.emplace_back(std::get<0>(newLocMax[i]),
        std::get<1>(newLocMax[i]));
    }
    hullLocMax.emplace_back(std::get<0>(newLocMax[0]),
      std::get<1>(newLocMax[0]));
  }
  return hullLocMax;
}

int op_analyzeRaster::distanceComp(std::pair <size_t,size_t> a,
    std::pair <size_t,size_t> b,
    std::pair <size_t,size_t> c) const{
   int y1 = (int)a.second - b.second;
   int y2 = (int)a.second - c.second;
   int x1 = (int)a.first - b.first;
   int x2 = (int)a.first - c.first;

   int item1 = (y1*y1 + x1*x1);
   int item2 = (y2*y2 + x2*x2);

   if(item1 == item2)
      return 0;             //when b and c are in same distance from a
   else if(item1 < item2)
      return -1;          //when b is closer to a
   return 1;              //when c is closer to a
}

/*
1.19 fill convex hull
to compute significant maximum by edge list algorithm

*/
void
op_analyzeRaster::fillConvexHull(std::vector<RasterPoint>& rasterMem,
    std::vector<std::pair <size_t,size_t>> hullLocMax,
    const int areaNumberLocMax) const{
  //compute edge list
  std::vector<std::pair <size_t,size_t>> edgeList;
  for (size_t i = 0; i < hullLocMax.size() - 1; i++){
    bool calc = computeEdgeLine(edgeList, hullLocMax[i], hullLocMax[i+1]);
    //in y-increasing or decreasing segments no double points
    if (calc){
      size_t posPlus = i + 1;
      size_t posPlusPlus = ((posPlus + 1) < hullLocMax.size()) ? posPlus +1 : 1;
      int deltaSeg1 = (int) hullLocMax[i].second
          - (int) hullLocMax[posPlus].second;
      int deltaSeg2 = (int) hullLocMax[posPlus].second
          - (int) hullLocMax[posPlusPlus].second;
      if ((deltaSeg1 > 0 && deltaSeg2 > 0) ||
          (deltaSeg1 < 0 && deltaSeg2 < 0))
      {
        edgeList.pop_back();
      }
      else
      {
        while (deltaSeg2 == 0)
        {
          posPlusPlus = ((posPlusPlus + 1) < hullLocMax.size()) ?
              posPlusPlus +1 : 1;
          deltaSeg2 = (int) hullLocMax[posPlus].second
              - (int) hullLocMax[posPlusPlus].second;
        }
        if ((deltaSeg1 > 0 && deltaSeg2 > 0) ||
                  (deltaSeg1 < 0 && deltaSeg2 < 0)){
          edgeList.pop_back();
        }
      }
    }
  }

  //sort by y and when equal by x
  std::sort(edgeList.begin(), edgeList.end(),
      [](const std::pair<size_t, size_t> &a,
          const std::pair<size_t, size_t> &b)
          {if (a.second == b.second){
            return (a.first < b.first);
          }
          return (a.second > b.second);});

  //fill points between pairs with same y in x direction
  for (size_t i = 0; i < edgeList.size(); i += 2)
  {
    assert(edgeList[i].first <= edgeList[i+1].first);
    for (size_t x = edgeList[i].first;
        x <= edgeList[i+1].first; ++x)
    {
      size_t arrayCount = edgeList[i].second * _pointInMem + x;
      rasterMem[arrayCount].objectID = areaNumberLocMax;
    }
  }
}

/*
1.20 test for new local maximum
in distance of central local maxima and flood

*/
void
op_analyzeRaster::testOtherLocMax(std::vector<RasterPoint>& rasterMem,
    size_t const xMin, size_t const xMax, size_t const y,
    const int areaNumberObj, const int areaNumberLocMax,
    bool& countLocMax, std::vector<std::pair <size_t,size_t>>& newLocMax) const{

  const size_t arrayCountMin = xMin + y * _pointInMem;
  const size_t arrayCountMax = xMax + y * _pointInMem;
  for (size_t i = arrayCountMin; i <= arrayCountMax; i++){
    int areaNumberLocMax2 = rasterMem[i].objectID;
    if (areaNumberLocMax2 > areaNumberObj
        && areaNumberLocMax2 != areaNumberLocMax){
      countLocMax = true;

      //flood merged locMax by areaNumberLocMax
      fillVectorWithOtherLocMax(rasterMem, i, areaNumberLocMax, newLocMax);
    }
  }
}

/*
1.21 calculating points in edge line of convex hull
ignores horizontal lines

*/
bool
op_analyzeRaster::computeEdgeLine(
    std::vector<std::pair <size_t,size_t>> &edgeList,
    const std::pair<size_t, size_t> a, 
    const std::pair<size_t, size_t> b) const{
  //ignore horizontal lines
  if (a.second != b.second)
  {
    if (a.first != b.first)
    {
      int x1 = (int)a.first; int y1 = (int)a.second;
      int x2 = (int)b.first; int y2 = (int)b.second;
      if (y1 < y2){
        for (int y = y1; y <= y2; y++)
        {
          double m = (x2 - x1 + 0.) / (y2 - y1 + 0.);
          double n = (y1 * x2 - y2 * x1 + 0.) / (x2 - x1 + 0.);
          int x = rint(m * (y - n));
          edgeList.emplace_back(std::make_pair((size_t) x, (size_t) y));
        }
      }
      else
      {
        for (int y = y1; y >= y2; y--)
        {
          double m = (x2 - x1 + 0.) / (y2 - y1 + 0.);
          double n = (y1 * x2 - y2 * x1 + 0.) / (x2 - x1 + 0.);
          int x = m * (y - n);
          edgeList.emplace_back(std::make_pair((size_t) x, (size_t) y));
        }
      }
      return true;
    }
    else
    {
      if (a.second < b.second){
        for (size_t y = a.second; y <= b.second; y++)
        {
          edgeList.emplace_back(std::make_pair(a.first, y));
        }
      }
      else
      {
        for (size_t y = a.second; y >= b.second; y--)
        {
          edgeList.emplace_back(std::make_pair(a.first, y));
        }
      }
      return true;
    }
  }
  return false;
}

/*
1.22 write back raster to pointcloud2

*/
void
op_analyzeRaster::saveRasterToPc2(std::vector<SmiFileId>& rasters,
    std::vector<Rect3> rastersBbox,
    std::shared_ptr<std::unordered_map<int,size_t>> classMap,
    std::shared_ptr<std::unordered_map<int,int>> duples,
    Pointcloud2* res) const{

  std::cout<<"Reading "<<rasters.size()<<" temporary rasters"<<endl;

  if (Pointcloud2::GET_RASTER_NOT_PC) {
      res->startInsert();
      TupleType *tt = new TupleType(nl->Second(nl->Second(_resultType)));
      cout<<"pointcloud rastercount should be:"
              <<std::to_string(rasters.size()*_pointInMem2)<<endl;
      //_pc2->clear();
      for (size_t i = 0; i < rasters.size(); i++)
        {
          //open tempRaster
          SmiFileId tempRasterId = rasters[i];
          std::cout << "Loading temporary raster: " << tempRasterId << endl;

          //vector<RasterPoint> rasterMem(_pointInMem2);
          RasterPoint rasterMem;

          const bool makefixed = true;
          const bool temporary = false;
          SmiSize reclen = sizeof(RasterPointDBSave);
          SmiRecordFile rf(makefixed, reclen, temporary);


          if (rf.Open(tempRasterId))
          {
            SmiRecordId rid = 1;
            RasterPointDB bufferDB;

            size_t arrayCount = 0;
            const Rectangle<3>& fileBbox = rastersBbox[i];
            const int rasterOffsetX = floor(
                    fileBbox.MinD(0) / Pointcloud2::CELL_SIZE_IN_M);
            const int rasterOffsetY = floor(
                    fileBbox.MinD(1) / Pointcloud2::CELL_SIZE_IN_M);
            PcPoint point;
            for (; arrayCount < _pointInMem2; arrayCount++)
            {
              SmiRecord record;
              rf.SelectRecord(rid, record, SmiFile::ReadOnly);
              record.Read(&bufferDB, reclen);
              rasterMem = secondoToStruct(bufferDB);

              Tuple *elem = new Tuple(tt);
              elem->PutAttribute(0, new CcInt(rasterMem.count));
              elem->PutAttribute(1, new CcReal(rasterMem.minAlt));
              elem->PutAttribute(2, new CcReal(rasterMem.maxAlt));
              elem->PutAttribute(3, new CcReal(rasterMem.averageAlt));
              elem->PutAttribute(4, new CcInt(rasterMem.averageIntensity));
              elem->PutAttribute(5, new CcInt(rasterMem.objectID));
              int classID = 0;
              if (rasterMem.objectID!=0)
                  try {
                      classID = classMap.get()->at(rasterMem.objectID);
                  } catch(std::exception &e) {
                  }
              elem->PutAttribute(6, new CcInt(classID));
              point._x = ((arrayCount % _pointInMem) + 0.) +
                      rasterOffsetX;
              point._y = ((arrayCount / _pointInMem) + 0.) +
                      rasterOffsetY;
              point._z = rasterMem.averageAlt;
              res->insert(point, elem);
              ++rid;
            }
          }
          rf.Close(tempRasterId);
          rf.Drop();
        }
      res->finalizeInsert();
      tt->DeleteIfAllowed();
      return;
  }
  else
  {

    //horizontal overlapping
    std::vector <RasterPointSave> rasterOverlapH;
    //two vertical lines of overlapping
    std::vector<std::vector<RasterPointSave>> rasterOverlapUnder;
    std::vector<std::vector<RasterPointSave>> rasterOverlapUnderEdge;
    std::vector<std::vector<RasterPointSave>> rasterOverlapOver;
    std::vector<std::vector<RasterPointSave>> rasterOverlapOverEdge;

    res->startInsert();

    for (size_t i = 0; i < rasters.size(); i++)
    {
      //open tempRaster
      SmiFileId tempRasterId = rasters[i];
      std::cout << "Loading temporary raster: " << tempRasterId << endl;

      std::vector<RasterPointSave> rasterMem(_pointInMem2);

      const bool makefixed = true;
      const bool temporary = false;
      SmiSize reclen = sizeof(RasterPointDBSave);
      SmiRecordFile rf(makefixed, reclen, temporary);

      //size_t arrayCount = 0;
      if (rf.Open(tempRasterId))
      {
        SmiRecordId rid = 1;
        RasterPointDBSave bufferDB;
        //RasterPointSave buffer;
        size_t arrayCount = 0;

        for (; arrayCount < rasterMem.size(); arrayCount++)
        {
          SmiRecord record;
          rf.SelectRecord(rid, record, SmiFile::ReadOnly);
          record.Read(&bufferDB, reclen);
          rasterMem[arrayCount] = secondoToStructSave(bufferDB);
          ++rid;
        }
      }
      rf.Close(tempRasterId);
      rf.Drop();

      if (rasters.size() > 1)
      {
        recalcOverlapping(rasterMem, rastersBbox[i], i, duples,
            rasterOverlapH, rasterOverlapUnder, rasterOverlapUnderEdge,
            rasterOverlapOver, rasterOverlapOverEdge);
      }

      //calculate part of raster rartition to write in this pass
      double xMin = rastersBbox[i].MinD(0);
      double yMin = rastersBbox[i].MinD(1);
      double xMax = rastersBbox[i].MaxD(0);
      double yMax = rastersBbox[i].MaxD(1);

      double overlapXMax = (xMax < _bb.MaxD(0)) ?
          (xMin + _sizeRasterOverlap) : xMax;
      double overlapYMax = (yMax < _bb.MaxD(1)) ?
          (yMin + _sizeRasterOverlap) : yMax;
      double min[] = {xMin, yMin, rastersBbox[i].MinD(2)};
      double max[] = {overlapXMax, overlapYMax, rastersBbox[i].MaxD(2)};
      Rect3 rasterBbox;
      rasterBbox.Set(true, min, max);

      //load pc2-source with bbox of actual partition
      auto pc2RasterPart = calculateRasterRartition(rasterBbox);

      //save to pc2
      PcPoint pcPoint;
      SmiRecordFileIterator *it;
      it = pc2RasterPart->getFileIterator();
      SmiRecord record;

      while (it->Next(record))
      {
        savePoint(res, *pc2RasterPart, rastersBbox[i],
            record, pcPoint, rasterMem, classMap);
      }
      delete it;

      std::cout << "save to pc2, pass " << i + 1 <<endl;
    }
    res->finalizeInsert();
  }
}

/*
1.23 write points to pc2
with ObjectID and ClassID

*/
void op_analyzeRaster::savePoint(Pointcloud2* res, 
    const Pointcloud2& rasterPart,
    const Rect3 rasterBbox,
    const SmiRecord& record,
    PcPoint& pcPoint,
    std::vector<RasterPointSave> const &rasterMem,
    std::shared_ptr<std::unordered_map<int,size_t>> classMap) const{
  const double grid_m = Pointcloud2::CELL_SIZE_IN_M;

  double x = rasterBbox.MinD(0);
  double y = rasterBbox.MinD(1);

  rasterPart.getPoint(record.GetId(), &pcPoint);

  //position in raster array
  unsigned int partX = floor((pcPoint._x - x) / grid_m);
  unsigned int partY = floor((pcPoint._y - y) / grid_m);
  unsigned int arrayCount = partX + _pointInMem * partY;
  if (partX < _pointInMem && partY < _pointInMem &&
      partX >= 0 && partY >= 0)

  {
    TupleType *tt = new TupleType(nl->Second(nl->Second(_resultType)));
    Tuple *elem = new Tuple(tt);
    //pc2 with attributes? then copy
    if (getPositionNewAttributes() > 0)
    {
      Tuple *elemSrc = rasterPart.getTuple(pcPoint._tupleId);
      for (size_t i = 0; i < getPositionNewAttributes(); i++)
      {
        elem->CopyAttribute(i, elemSrc, i);
      }
      elemSrc->DeleteIfAllowed();
    }
    const int objectID = rasterMem[arrayCount].objectID;
    elem->PutAttribute(getPositionNewAttributes(),
        new CcInt(objectID));

    int classID = 0;
    if (objectID!=0)
        try {
            classID = classMap.get()->at(objectID);
        } catch(std::exception &e) {
        }
    elem->PutAttribute(getPositionNewAttributes() + 1,
        new CcInt(classID));
    res->insert(pcPoint, elem);
    tt->DeleteIfAllowed();
  }
}

/*
1.24 erase double objects in overlappings
using memory structure for overlappings tested in other raster partitions

*/
void
op_analyzeRaster::recalcOverlapping(std::vector<RasterPointSave> &rasterMem,
    const Rect3 rasterBbox, const size_t i,
    const std::shared_ptr<std::unordered_map<int,int>> duples,
    std::vector <RasterPointSave> &rasterOverlapH,
    std::vector<std::vector<RasterPointSave>> &rasterOverlapUnder,
    std::vector<std::vector<RasterPointSave>> &rasterOverlapUnderEdge,
    std::vector<std::vector<RasterPointSave>> &rasterOverlapOver,
    std::vector<std::vector<RasterPointSave>> &rasterOverlapOverEdge) const{

  //structure for overlapping
  Rect3 bboxSource = _pc2->getBoundingBox();
  const size_t rastersInX = ceil((bboxSource.MaxD(0) - bboxSource.MinD(0)) /
      _sizeRasterOverlap);
  const size_t rastersInY = ceil((bboxSource.MaxD(1) - bboxSource.MinD(1)) /
      _sizeRasterOverlap);

  const unsigned int offset = _sizeRaster - _sizeRasterOverlap;
  const unsigned int overlapRXMin =
      ceil(offset / Pointcloud2::CELL_SIZE_IN_M);
  const unsigned int overlapRXMax =
      _pointInMem - overlapRXMin -1;
  const unsigned int overlapRYMin =
      ceil(offset / Pointcloud2::CELL_SIZE_IN_M);
  const unsigned int overlapRYMax =
      _pointInMem - overlapRYMin -1;

  //test for objects in saved overlapping
  //horizontal line under no edge
  if (i / rastersInX > 0)
  {
    if (rastersInX != 1 && i % rastersInX != 0){
      for (size_t arrayCount = 0;
          arrayCount < rasterOverlapUnderEdge.front().size(); arrayCount++){
        int objID = rasterOverlapUnderEdge.front().at(arrayCount).objectID;
        if (objID > 0){
          size_t arrayCountR = (arrayCount / overlapRXMin) * _pointInMem
              + (arrayCount % overlapRXMin);
          rasterMem[arrayCountR].objectID = objID;
        }
      }
      rasterOverlapUnderEdge.erase(rasterOverlapUnderEdge.begin());
    }

    bool first = ((i % rastersInX) == 0);
    bool last = ((i % rastersInX) == rastersInX - 1);
    size_t line;
    if (first && last){
      line = _pointInMem;
    }
    else if (first || last){
      line = _pointInMem - overlapRXMin;
    }
    else {
      line = _pointInMem - 2 * overlapRXMin;
    }
    for (size_t arrayCount = 0;
        arrayCount < rasterOverlapUnder.front().size(); arrayCount++){
      int objID = rasterOverlapUnder.front().at(arrayCount).objectID;
      if (objID > 0){
        size_t arrayCountR = (arrayCount / line) * _pointInMem
            + (arrayCount % line);
        rasterMem[arrayCountR].objectID = objID;
      }
    }
    rasterOverlapUnder.erase(rasterOverlapUnder.begin());
  }

  //vertical line no edge
  if (rastersInX != 1 && (i + 1) % rastersInX == 0)
  {
    bool first = (i / rastersInX == 0);
    for (size_t arrayCount = 0;
        arrayCount < rasterOverlapH.size(); arrayCount++){
      int objID = rasterOverlapH[arrayCount].objectID;
      if (objID > 0){
        size_t arrayCountR = (arrayCount / overlapRXMin) * _pointInMem
            + overlapRXMax + (arrayCount % overlapRXMin);
        if (!first){
          arrayCountR += (overlapRYMin) * _pointInMem;
        }
        rasterMem[arrayCountR].objectID = objID;
      }
    }
    rasterOverlapH.clear();
  }

  //recalculate overlapping structure
  //horizontal line under no edge
  //under
  if (rastersInY != 1 && (i / rastersInX) > 0 && (i + 1) % rastersInX != 0)
  {
    for (size_t arrayCount = 0;
        arrayCount < rasterOverlapUnderEdge[0].size(); arrayCount++){
      size_t arrayCountR = (arrayCount / overlapRXMin) * _pointInMem
          + (arrayCount % overlapRXMin) + overlapRXMax;
      const int objID = rasterMem[arrayCountR].objectID;
      if (objID > 0){
        rasterOverlapUnderEdge[0][arrayCount].objectID = objID;
        rasterOverlapUnderEdge[0][arrayCount].classID =
            rasterMem[arrayCountR].classID;
      }
    }
  }
  //over
  if (rastersInY != 1 && i / rastersInX < rastersInY -1 && i % rastersInX != 0)
  {
    size_t numRecalc = rastersInX - 2;
    for (size_t arrayCount = 0;
        arrayCount < rasterOverlapOverEdge[numRecalc].size(); arrayCount++){
      size_t arrayCountR =
          (arrayCount / overlapRXMin + overlapRYMax) * _pointInMem
          + (arrayCount % overlapRXMin) + overlapRXMax;
      const int objID = rasterMem[arrayCountR].objectID;
      if (objID > 0){
        rasterOverlapOverEdge[numRecalc][arrayCount].objectID = objID;
        rasterOverlapOverEdge[numRecalc][arrayCount].classID =
            rasterMem[arrayCountR].classID;
      }
    }
  }


  //generate new overlapping structure
  //horizontal line no edge
  if (i / rastersInX < rastersInY - 1)
  {
    rasterOverlapOver.resize(rasterOverlapOver.size()+1);
    size_t startAC = ((i % rastersInX) == 0) ? (_pointInMem * overlapRYMax) :
        (_pointInMem * overlapRYMax + overlapRXMin);
    size_t endAC = ((i % rastersInX) == rastersInX- 1) ?
        _pointInMem2 : (_pointInMem2 - overlapRYMin);
    for (size_t arrayCount = startAC; arrayCount < endAC; arrayCount++){
      const int objID = rasterMem[arrayCount].objectID;
      const int classID = rasterMem[arrayCount].classID;
      rasterOverlapOver.back().push_back({objID, classID});
      if (((i % rastersInX) == rastersInX- 1) ?
          ((arrayCount + 1) % _pointInMem == 0) :
          ((arrayCount + 1 +overlapRXMin) % _pointInMem == 0)){
        arrayCount += overlapRXMin + 1;
      }
    }
    if (rastersInX != 1 && !(i % rastersInX) == rastersInX- 1){
      rasterOverlapOverEdge.resize(rasterOverlapOverEdge.size()+1);
      for (size_t arrayCount = (overlapRYMax * _pointInMem) + overlapRXMax;
          arrayCount < _pointInMem2; arrayCount++){
        const int objID = rasterMem[arrayCount].objectID;
        const int classID = rasterMem[arrayCount].classID;
        rasterOverlapOverEdge.back().push_back({objID, classID});
        if ((arrayCount + 1) % _pointInMem == 0){
          arrayCount += overlapRXMax + 1;
        }
      }
    }
  }
  //vertical line no edge
  if ((i +1) % rastersInX != 0)
  {
    size_t startAC = ((i / rastersInX) == 0) ?
        overlapRXMax : (_pointInMem * overlapRYMin + overlapRXMax);
    size_t endAC = ((i / rastersInX) == rastersInY - 1) ?
        _pointInMem2 : (_pointInMem * overlapRYMax);
    for (size_t arrayCountR = startAC; arrayCountR < endAC; arrayCountR++){
      const int objID = rasterMem[arrayCountR].objectID;
      const int classID = rasterMem[arrayCountR].classID;
      rasterOverlapH.push_back({objID, classID});
      if ((arrayCountR + 1) % _pointInMem == 0){
        arrayCountR += overlapRXMax + 1;
      }
    }
  }
  //new x-line
  if ((i +1) % rastersInX == 0){
    rasterOverlapUnder.swap(rasterOverlapOver);
    rasterOverlapUnderEdge.swap(rasterOverlapOverEdge);
  }
}

/*
1.25 recalculate bbox by newly discovered points

*/
void
op_analyzeRaster::recalcBbox(Rect& bboxObj, size_t nF) const{

  double minX = bboxObj.MinD(0);
  double maxX = bboxObj.MaxD(0);
  double minY = bboxObj.MinD(1);
  double maxY = bboxObj.MaxD(1);

  if (minX > (nF % _pointInMem)){
    minX = (nF % _pointInMem) + 0.;
  }
  if (maxX < (nF % _pointInMem)){
    maxX = (nF % _pointInMem) + 0.;
  }
  if (minY > (nF / _pointInMem)){
    minY = (nF / _pointInMem) + 0.;
  }
  if (maxY < (nF / _pointInMem)){
    maxY = (nF / _pointInMem) + 0.;
  }
  double min[] = {minX, minY};
  double max[] = {maxX, maxY};
  bboxObj.Set(true, min, max);
}

/*
1.26 convert rasterpoint to secondo representation

*/
RasterPointDB
op_analyzeRaster::structToSecondo(RasterPoint grid) const{
  RasterPointDB buffer;
  buffer.averageAlt.Set(true,grid.averageAlt);
  buffer.averageIntensity.Set(true,grid.averageIntensity);
  buffer.classID.Set(true,grid.classID);
  buffer.objectID.Set(true,grid.objectID);
  buffer.count.Set(true,grid.count);
  buffer.initialized.Set(true,grid.initialized);
  buffer.maxAlt.Set(true,grid.maxAlt);
  buffer.minAlt.Set(true,grid.minAlt);
  return buffer;
}

/*
1.27 convert secondo representation to rasterpoint

*/
RasterPoint
op_analyzeRaster::secondoToStruct(RasterPointDB bufferDB){
  RasterPoint buffer;
  buffer.averageAlt = bufferDB.averageAlt.GetRealval();
  buffer.averageIntensity = bufferDB.averageIntensity.GetIntval();
  buffer.classID = bufferDB.classID.GetIntval();
  buffer.objectID = bufferDB.objectID.GetIntval();
  buffer.count = (size_t) bufferDB.count.GetIntval();
  buffer.initialized = bufferDB.initialized.GetBoolval();
  buffer.maxAlt = bufferDB.maxAlt.GetRealval();
  buffer.minAlt = bufferDB.minAlt.GetRealval();
  buffer.objectID = bufferDB.objectID.GetIntval();
  return buffer;
}

/*
1.28 convert secondo representation to rasterpoint
reduced struct for save back

*/
RasterPointSave
op_analyzeRaster::secondoToStructSave(RasterPointDBSave bufferDB) const{
  RasterPointSave buffer;
  buffer.classID = bufferDB.classID.GetIntval();
  buffer.objectID = bufferDB.objectID.GetIntval();
  return buffer;
}

/*
1.29 calculate raster rartition
calculate bbox for pc2 partition

*/
std::unique_ptr<Pointcloud2, std::function<void(Pointcloud2*)>>
    op_analyzeRaster::calculateRasterRartition(double x,
        double y) const
{
  double minMax[] = {
      x,
      double(x + _sizeRaster),
      y,
      double(y + _sizeRaster),
      _pc2->getBoundingBox().MinD(2),
      _pc2->getBoundingBox().MaxD(2)
  };
  Rect3 bboxRaster({true, minMax});

  const PcBox intersectionBbox =
      bboxRaster.Intersection(_pc2->getBoundingBox());
  assert (!intersectionBbox.IsEmpty());

  return calculateRasterRartition(bboxRaster);
}

/*
1.30 calculate raster partition
calculate partition of pc2 to calculate temporary representation
or if complete raster fits in memory use source pc2

*/
std::unique_ptr<Pointcloud2, std::function<void(Pointcloud2*)>>
    op_analyzeRaster::calculateRasterRartition(Rect3 bboxRaster) const{
  ListExpr type = nl->Second(nl->Second(nl->Second(_resultType)));
  ListExpr refPc2 = nl->SymbolAtom(Referencesystem::toString(
      pointcloud2::Referencesystem::EUCLID));
  if (nl->ListLength(type) == 2){
    type = nl->OneElemList(refPc2);
  }
  else
  {
    ListExpr tuple = nl->OneElemList(nl->First(type));
    type = nl->Rest(type);
    ListExpr last = tuple;
    while (!nl->HasLength(type, 2)){
      last = nl->Append(last, nl->First(type));
      type = nl->Rest(type);
    }
    type = nl->TwoElemList(nl->First(
        nl->Second(nl->Second(_resultType))), tuple);
    type = nl->TwoElemList(refPc2, type);
  }
  size_t sizeR = _sizeRaster;
  size_t sizeROverlap = _sizeRasterOverlap;
  if (sizeR != sizeROverlap) {
      std::unique_ptr<Pointcloud2, std::function<void(Pointcloud2*)>>
          pc2RasterPart(new Pointcloud2(type), [&type](Pointcloud2* p) {
              Word w;
              w.addr = p;
              Pointcloud2::Delete(type, w);
          });
      pc2RasterPart->copySelectionFrom(_pc2, &bboxRaster);
      return pc2RasterPart;
  } else {
      std::unique_ptr<Pointcloud2, std::function<void(Pointcloud2*)>>
          pc2RasterPart(_pc2, [](Pointcloud2* p) {
              // do NOT delete _pc2
          });
      return pc2RasterPart;
  }
}
