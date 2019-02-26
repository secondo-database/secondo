/*
1 JoinEdge struct

*/

#pragma once

#include <memory>

#include "Base.h"

namespace cdacspatialjoin {

struct JoinEdge {
   double yMin;
   double yMax;

   EdgeIndex_t counterPartEdgeIndex;

   bool isLeft;
   SET set;
   BlockIndex_t block;
   RowIndex_t row;

   JoinEdge(double yMin_, double yMax_, EdgeIndex_t counterPartEdgeIndex_,
           bool isLeft_, SET set_, BlockIndex_t block_, RowIndex_t row_);

   ~JoinEdge() = default;

   inline bool operator<(const JoinEdge &other) const {
      return (yMin < other.yMin);
   }

   std::string toString() const;
};

} // end of namespace cdacspatialjoin