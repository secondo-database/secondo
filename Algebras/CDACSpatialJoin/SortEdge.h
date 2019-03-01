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
   RectInfoIndex_t rectInfoIndex;
   bool isLeft;

   SortEdge(const double x_, const RectInfoIndex_t rectInfoIndex_,
           const bool isLeft_) :
           x(x_), rectInfoIndex(rectInfoIndex_), isLeft(isLeft_) {
   }

   inline bool operator< (const SortEdge& other) const {
      return (x < other.x) || (x == other.x && isLeft && !other.isLeft);
      // Just sorting by x would not have the desired effect: Suppose our sets
      // A and B each consist of one rectangle only:
      // * rect A: (0,0) - (1,1), SortEdges "x=0, left" and "x=1, right"
      // * rect B: (1,1) - (2,2), SortEdges "x=1, left" and "x=2, right"
      // Sorting by x only (or by x and then by yMin) could place *both*
      // edges of rectangle A before the edges of rectangle B, producing
      // JoinEdges in the following order:
      // * [x=0], yMin=0, yMax=1, left,  rect A
      // * [x=1], yMin=0, yMax=1, right, rect A
      // * [x=1], yMin=1, yMax=2, left,  rect B
      // * [x=2], yMin=1, yMax=2, right, rect B
      // If we now merge the first two JoinEdges, we get a MergedArea with
      // rectangle A in the "complete" set; merging the last two JoinEdges
      // produces a MergedArea with rectangle B in the "complete" set.
      // merging both MergedAreas will produce no output, since there are no
      // spanning left or right edges.
      //
      // If, by contrast, we sort as above, ensuring that at the same x
      // position, all left edges are placed before all right edges, we get
      // the following JoinEdges order:
      // * [x=0], yMin=0, yMax=1, left,    rect A
      // * [x=1], yMin=1, yMax=2, *left*,  rect B
      // * [x=1], yMin=0, yMax=1, *right*, rect A
      // * [x=2], yMin=1, yMax=2, right,   rect B
      // Thus the the intersection will be noticed before any rectangle is
      // added to the "complete" set.
   }

   std::string toString() const {
      std::stringstream st;
      st << "x = " << x << ", "
         << "rectInfo #" << rectInfoIndex << ", "
         << (isLeft ? "left" : "right");
      return st.str();
   }

};

} // end of namespace cdacspatialjoin
