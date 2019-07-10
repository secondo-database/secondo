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


1 JoinEdge struct

*/

#pragma once

#include "Base.h" // -> <memory>, <vector>

namespace cdacspatialjoin {

struct JoinEdge {
   static constexpr EdgeIndex_t IS_LEFT_MASK = 0x8000000;
   static constexpr EdgeIndex_t COUNTER_PART_MASK = IS_LEFT_MASK - 1;

   double yMin;
   double yMax;

   uint32_t isLeftAndCounterPart;
   SetRowBlock_t address;

   JoinEdge(const double yMin_, const double yMax_,
            const EdgeIndex_t counterPartEdgeIndex_, const bool isLeft_,
            const SetRowBlock_t address_) :
           yMin(yMin_),
           yMax(yMax_),
           isLeftAndCounterPart(
                   (isLeft_ ? 0 : IS_LEFT_MASK) | counterPartEdgeIndex_),
           address(address_) {

   }

   ~JoinEdge() = default;

   inline bool getIsLeft() const {
      return (isLeftAndCounterPart & IS_LEFT_MASK) == 0;
   }

   /* used by MergedArea constructor */
   inline bool operator<(const JoinEdge& other) const {
      return (yMin < other.yMin);
   }

   static int compare(const void* a, const void* b) {
      // cp. SortEdge::operator<
      const double ay = ((JoinEdge*)a)->yMin;
      const double by = ((JoinEdge*)b)->yMin;
      if (ay < by)
         return -1;
      else
         return (ay == by) ? 0 : 1;
   }

   /*
   inline EdgeIndex_t getCounterPartEdgeIndex() const {
      return isLeftAndCounterPart & COUNTER_PART_MASK;
   }
   */
};

typedef std::vector<JoinEdge> JoinEdgeVec;

} // end of namespace cdacspatialjoin