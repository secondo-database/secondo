/*
1 RectangleInfo struct

*/

#pragma once

#include "Base.h"

namespace cdacspatialjoin {

struct RectangleInfo {

   double yMax;

   EdgeIndex_t leftEdgeIndex;
   EdgeIndex_t rightEdgeIndex;

   SET set;
   BlockIndex_t block;
   RowIndex_t row;

   RectangleInfo(double yMax_, SET set_, BlockIndex_t block_, RowIndex_t row_):
        yMax(yMax_), leftEdgeIndex(0), set(set_), block(block_), row(row_) {
   }

   inline EdgeIndex_t getEdgeIndex(bool left) const {
      return left ? leftEdgeIndex : rightEdgeIndex;
   }
};

} // end of namespace cdacspatialjoin