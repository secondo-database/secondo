/*
1 SortEdge class

*/

#pragma once

#include <memory>
#include <cstdint>
#include <iostream>
#include <string>
#include <sstream>

#include "Base.h"

namespace cdacspatialjoin {

struct SortEdge {
   double x;
   double yMin;
   RectInfoIndex_t rectInfoIndex;
   bool isLeft;

   SortEdge(double x_, double yMin_, RectInfoIndex_t rectInfoIndex_,
           bool isLeft_) :
           x(x_), yMin(yMin_), rectInfoIndex(rectInfoIndex_), isLeft(isLeft_) {
   }

   inline bool operator< (const SortEdge& other) const {
      // sort by x, then by yMin
      return (x < other.x) || (x == other.x && yMin < other.yMin);
   }

   std::string toString() const {
      std::stringstream st;
      st << "x = " << x << ", yMin = " << yMin << "; "
         << "rectInfo #" << rectInfoIndex << ", "
         << (isLeft ? "left" : "right");
      return st.str();
   }

};

} // end of namespace cdacspatialjoin
