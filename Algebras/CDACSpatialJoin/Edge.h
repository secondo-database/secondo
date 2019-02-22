/*
1 BinaryTuple

*/

#pragma once

#include <cstdint>
#include <iostream>

namespace cdacspatialjoin {

// TODO: anpassen! umbenennen?

struct Edge {
   Edge() :
           blockNum(0), row(0),
           xMin(0), xMax(0), yMin(0), yMax(0), zMin(0), zMax(0) {
   }

   uint64_t blockNum;
   uint64_t row;
   double xMin;
   double xMax;
   double yMin;
   double yMax;
   double zMin;
   double zMax;
};

} // end of namespace cdacspatialjoin
