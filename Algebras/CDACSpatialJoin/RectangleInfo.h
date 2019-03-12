/*
1 RectangleInfo struct

*/

#pragma once

#include "Base.h"

namespace cdacspatialjoin {

struct RectangleInfo {
   double yMin;
   double yMax;

   EdgeIndex_t leftEdgeIndex;
   EdgeIndex_t rightEdgeIndex;

   SetRowBlock_t address;

   RectangleInfo(const double yMin_, const double yMax_,
                 const SetRowBlock_t address_):
        yMin(yMin_), yMax(yMax_), leftEdgeIndex(0), rightEdgeIndex(0),
        address(address_) {
   }

   inline EdgeIndex_t getEdgeIndex(const bool left) const {
      return left ? leftEdgeIndex : rightEdgeIndex;
   }
};

} // end of namespace cdacspatialjoin