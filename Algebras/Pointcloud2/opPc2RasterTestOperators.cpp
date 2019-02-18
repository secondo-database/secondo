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



0 Pc2RasterTest Operators

Generate Pc2 for testing analyzeRaster
Attribute: TRUE/FALSE: even or random raster

*/

#include "opPc2RasterTestOperators.h"

#include <stdlib.h>
#include <tuple>
#include <random>
#include <chrono>

#include <boost/algorithm/string.hpp>

extern NestedList *nl;
extern QueryProcessor *qp;


using namespace pointcloud2;

ListExpr op_Pc2RasterTest::Pc2RasterTestTM(ListExpr args){
  const std::string err = "two arguments expected ";
  // (bool)
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err + " (wrong number of arguments)");
  }

  if (!CcInt::checkType(nl->First(args))){
    return listutils::typeError(err + " (first argument has to be int)");
  }
  if (!CcBool::checkType(nl->Second(args))){
    return listutils::typeError(err + " (second argument has to be bool)");
  }

  // return (pointcloud2 EUCLID)
  return Pointcloud2::cloudTypeWithParams(
      nl->SymbolAtom(
          Referencesystem::toString(Referencesystem::Type::EUCLID)));;
}

int op_Pc2RasterTest::Pc2RasterTestVMT( Word* args, Word& result, int message,
    Word& local, Supplier s ){

  result = qp->ResultStorage(s);
  Pointcloud2* res = static_cast<pointcloud2::Pointcloud2*>(result.addr);

  size_t rasterSize = static_cast<size_t>(
          ((CcInt*) args[0].addr)->GetIntval());
  bool even = ((CcBool*) args[1].addr)->GetBoolval();
  op_Pc2RasterTest rasterTest(rasterSize, even);

  if (even){
    rasterTest.evenRaster(res);
  } else {
    rasterTest.randomRaster(res);
  }

  // SmiEnvironment::CommitTransaction(); //Commit the last transaction

  if(local.addr){
    local.addr = nullptr;
  }
  return 0;
}


void op_Pc2RasterTest::evenRaster(Pointcloud2 *res) const{
  std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
  res->startInsert();
  for (size_t x = 0; x < _rasterSize; ++x) {
      for (size_t y = 0; y < _rasterSize; ++y){
          double z = 100.0;
          if ((x % 30) < 10 && (y % 50) < 15)
          {
              z = 130.0 
                + std::uniform_int_distribution<size_t>(0, 49)(rng) / 100.0;
              if ((x % 7) < 3 && (y % 7) < 4){
                  z = 135.0;
              }
          }
          //edges z = 110.0
          if (x <= 3 || x >= _rasterSize - 3 || y <= 3 || y >= _rasterSize - 3)
          {
              z = 115.0;
          }
          res->insert({x * 0.1, y * 0.1, z});
      }
  }
  res->finalizeInsert();
}


void op_Pc2RasterTest::randomRaster(Pointcloud2 *res) const{
  constexpr double standardAlt = 100.0;
  constexpr size_t minHigh = 10;
  constexpr size_t maxHigh = 40;
  constexpr size_t maxLocMax = 5;

  const size_t rasterSize2 = pow(_rasterSize,2);
  const size_t amountObj = rasterSize2 / 3000;

  std::vector<double> raster (rasterSize2, standardAlt);

  std::mt19937 
           rng(std::chrono::steady_clock::now().time_since_epoch().count());

  for (size_t i = 0; i < amountObj; ++i){
    size_t objHigh = std::uniform_int_distribution<size_t>(minHigh, 40)(rng);
    size_t startY =
        std::uniform_int_distribution<size_t>(maxHigh, 
                                              _rasterSize - maxHigh)(rng);
    size_t startX =
        std::uniform_int_distribution<size_t>(maxHigh, 
                                              _rasterSize - maxHigh)(rng);
    for(size_t y = startY; y < startY + objHigh; ++y){
      size_t deltaX1 = std::uniform_int_distribution<size_t>(6, maxHigh/2)(rng);
      size_t deltaX2 = std::uniform_int_distribution<size_t>(6, maxHigh/2)(rng);
      for (size_t x = startX - deltaX1; x < startX + deltaX2; ++x){
        size_t deltaAlt = std::uniform_int_distribution<size_t>(0, 49)(rng);
        raster[y * _rasterSize + x] =
            standardAlt + 20.0 + deltaAlt / 100.0;
      }
    }
    size_t locMax = std::uniform_int_distribution<size_t>(0, maxLocMax)(rng);
    while (locMax != 0){
      size_t startMY =
          std::uniform_int_distribution<size_t>(0, objHigh-1)(rng) + startY;
      size_t startMX =
          std::uniform_int_distribution<size_t>(0, objHigh)(rng)
          + startX - objHigh / 2;
      if (raster[startMY * _rasterSize + startMX] != standardAlt){
        size_t objHighM = std::uniform_int_distribution<size_t>(2, 6)(rng);
        for(size_t y = startMY; y < objHighM + startMY; ++y){
          size_t deltaX = std::uniform_int_distribution<size_t>(2, 6)(rng);;
          for (size_t x = startMX - deltaX; x < startX + deltaX; ++x){
            size_t deltaAltM =std::uniform_int_distribution<size_t>(0, 49)(rng);
            raster[y * _rasterSize + x] =
                standardAlt + 25.0 + deltaAltM / 100.0;
          }
        }
        --locMax;
      }
    }
  }
  res->startInsert();
  for (size_t x = 0; x < _rasterSize; ++x){
        for (size_t y = 0; y < _rasterSize; ++y){
          size_t arrayCount = y * _rasterSize + x;
          double z = raster[arrayCount];
          res->insert({x * 0.1, y * 0.1, z});
        }
  }
  res->finalizeInsert();
  raster.clear();
}

std::string op_Pc2RasterTest::getOperatorSpec(){
    return OperatorSpec(
            "-> Pc2(EUCLID)",
            "rasterTestPc2(int size, bool even)",
            "Generate Pc2 for testing analyzeRaster",
            "query rasterTestPc2(150, TRUE)"
    ).getStr();
}

std::shared_ptr<Operator> op_Pc2RasterTest::getOperator(){
    return std::make_shared<Operator>("rasterTestPc2",
                                       getOperatorSpec(),
                                       op_Pc2RasterTest::Pc2RasterTestVMT,
                                       //2,
                                       Operator::SimpleSelect,
                                       //OPImportxyz::importxyzSelect,
                                       &op_Pc2RasterTest::Pc2RasterTestTM);
}

