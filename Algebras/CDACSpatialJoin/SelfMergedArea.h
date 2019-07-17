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


1 SelfMergedArea class

SelfMergedArea is used for a self join and represents an area that contains
all JoinEdges in a given x interval (more specifically, an interval in the
order induced by SortEdge). Invariants of class instances are:

  * all JoinEdge vectors (left/right/complete) are sorted by the yMin value
    of the edges.

  * if two rectangles are both represented in this area by at least one
    JoinEdge respectively, and if at least one of these JoinEdges is a left
    edge, then a possible intersection of these rectangles
    has already been reported and must not be reported again.

The SelfMerger class is used to merge two adjoining SelfMergedAreas into a new,
larger SelfMergedArea.

*/

#pragma once

#include "JoinEdge.h" // -> ... -> <memory>, <vector>

namespace cdacspatialjoin {

struct SelfMergedArea;
typedef SelfMergedArea* SelfMergedAreaPtr;

struct SelfMergedArea {
   /* the index in JoinState::joinEdges from which the JoinEdges covered by
    * this SelfMergedArea start (inclusive) */
   const EdgeIndex_t edgeIndexStart;
   /* the index in JoinState::joinEdges at which the JoinEdges covered by
    * this SelfMergedArea end (exclusive) */
   const EdgeIndex_t edgeIndexEnd;

   /* the left edges inside this SelfMergedArea, for which the
    * corresponding right edge is not inside this SelfMergedArea */
   JoinEdgeVec left;
   /* the right edges inside this SelfMergedArea, for which the
    * corresponding left edge is not inside this SelfMergedArea */
   JoinEdgeVec right;
   /* the rectangles represented in this SelfMergedArea by both their
    * left and right edges */
   JoinEdgeVec complete;

   /* instantiates an atomic SelfMergedArea from a single JoinEdges */
   SelfMergedArea(const std::vector<JoinEdge>& joinEdges,
         EdgeIndex_t edgeIndexStart_, EdgeIndex_t edgeIndexEnd_,
         bool isLeft);

   /* instantiates a MergedArea that can hold the result of merging the given
    * SelfMergedArea instances. area1 and area2 must be adjoining */
   SelfMergedArea(SelfMergedAreaPtr area1, SelfMergedAreaPtr area2);

   ~SelfMergedArea() = default;

   /* returns a short string representation of this MergedArea for console
    * output*/
   std::string toString() const;

   /* returns the number of JoinEdges encompassed by this SelfMergedArea. This
    * refers to the x-sorted JoinState::joinEdges vector. Note that due to the
    * "complete" vectors, this value may be higher than the value returned by
    * getJoinEdgeCount() */
   EdgeIndex_t getWidth() const { return edgeIndexEnd - edgeIndexStart; }

   /* returns the total number of JoinEdge instances stored in the various
    * vectors of this SelfMergedArea. Note that due to the "complete" vectors,
    * this value may be lower than the value returned by getWidth() */
   size_t getJoinEdgeCount() const;

   /* returns the number of bytes currently used by this SelfMergedArea,
    * including the content of its vectors. */
   size_t getUsedMemory() const;

private:
   /* returns true if the counterpart (i.e. the corresponding left or right
    * edge) of the given edge is found inside this SelfMergedArea */
   /*
   inline bool containsCounterpartOf(const JoinEdge &edge) const {
      const EdgeIndex_t counterPart = edge.getCounterPartEdgeIndex();
      return (counterPart >= edgeIndexStart && counterPart < edgeIndexEnd);
   }
   */

   friend class SelfMerger;
};

} // end of namespace cdacspatialjoin