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

\setcounter{tocdepth}{2}
\tableofcontents


1 SortEdge class

*/

#pragma once

#include <string>

#include "Base.h" // -> <memory>

namespace cdacspatialjoin {

/* the integer type used for vector<RectangleInfo> indices */
typedef uint32_t RectInfoIndex_t;

struct SortEdge {
   double x;
   RectInfoIndex_t rectInfoIndex;
   bool isLeft;

   SortEdge(const double x_, const RectInfoIndex_t rectInfoIndex_,
           const bool isLeft_) :
           x(x_), rectInfoIndex(rectInfoIndex_), isLeft(isLeft_) {
   }

   ~SortEdge() = default;

   /*
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
   */

   static int compare(const void* a, const void* b) {
      // cp. SortEdge::operator<
      const double ax = ((SortEdge*)a)->x;
      const double bx = ((SortEdge*)b)->x;
      if (ax < bx)
         return -1;
      if (ax > bx)
         return 1;
      return (((SortEdge*)a)->isLeft ? -1 : 0)
             + (((SortEdge*)b)->isLeft ?  1 : 0);
   }

   std::string toString() const;

};

} // end of namespace cdacspatialjoin
