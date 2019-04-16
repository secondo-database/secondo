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


1 MergedArea class

Represents an area that contains all JoinEdges of both sets in a given
x interval (more specifically, an interval in the order induced by SortEdge).
Invariants of class instances are:

  * all JoinEdge vectors (left/right/complete for sets A/B) are sorted by
    the yMin value of the edges.

  * if two rectangles from different sets are both represented in this area
    by at least one JoinEdge respectively, and if at least one of these
    JoinEdges is a left edge, then a possible intersection of these rectangles
    has already been reported and must not be reported again.

The Merger class is used to merge two adjoining MergedAreas into a new, larger
MergedArea.

*/

#pragma once

#include "JoinEdge.h" // -> ... -> <memory>, <vector>

namespace cdacspatialjoin {

struct MergedArea;
typedef MergedArea* MergedAreaPtr;

struct MergedArea {
   /* the index in JoinState::joinEdges from which the JoinEdges covered by
    * this MergedArea start (inclusive) */
   const EdgeIndex_t edgeIndexStart;
   /* the index in JoinState::joinEdges at which the JoinEdges covered by
    * this MergedArea end (exclusive) */
   const EdgeIndex_t edgeIndexEnd;

   /* the left edges from set A inside this MergedArea, for which the
    * corresponding right edge is not inside this MergedArea */
   JoinEdgeVec leftA;
   /* the right edges from set A inside this MergedArea, for which the
    * corresponding left edge is not inside this MergedArea */
   JoinEdgeVec rightA;
   /* the rectangles from set A represented in this MergedArea by both their
    * left and right edges */
   JoinEdgeVec completeA;

   /* the left edges from set B inside this MergedArea, for which the
    * corresponding right edge is not inside this MergedArea */
   JoinEdgeVec leftB;
   /* the right edges from set B inside this MergedArea, for which the
    * corresponding left edge is not inside this MergedArea */
   JoinEdgeVec rightB;
   /* the rectangles from set B represented in this MergedArea by both their
    * left and right edges */
   JoinEdgeVec completeB;

   /* instantiates an atomic MergedArea which covers an interval in which
    * only JoinEdges from the same set are to be found */
   MergedArea(const std::vector<JoinEdge>& joinEdges,
           EdgeIndex_t edgeIndexStart_, EdgeIndex_t edgeIndexEnd_, SET set);

   /* instantiates a MergedArea that can hold the result of merging the given
    * MergedArea instances. area1 and area2 must be adjoining */
   MergedArea(MergedAreaPtr area1, MergedAreaPtr area2);

   ~MergedArea() = default;

   /* returns a short string representation of this MergedArea for console
    * output*/
   std::string toString() const;

   /* returns the number of JoinEdges encompassed by this MergedArea. This
    * refers to the x-sorted JoinState::joinEdges vector. Note that due to the
    * "complete" vectors, this value may be higher than the value returned by
    * getJoinEdgeCount() */
   EdgeIndex_t getWidth() const { return edgeIndexEnd - edgeIndexStart; }

   /* returns the total number of JoinEdge instances stored in the various
    * vectors of this MergedArea. Note that due to the "complete" vectors, this
    * value may be lower than the value returned by getWidth() */
   size_t getJoinEdgeCount() const;

   /* returns the number of bytes currently used by this MergedArea, including
    * the content of its vectors. */
   size_t getUsedMemory() const;

private:
   /* returns true if the counterpart (i.e. the corresponding left or right
    * edge) of the given edge is found inside this MergedArea */
   /*
   inline bool containsCounterpartOf(const JoinEdge &edge) const {
      const EdgeIndex_t counterPart = edge.getCounterPartEdgeIndex();
      return (counterPart >= edgeIndexStart && counterPart < edgeIndexEnd);
   }
   */

   friend class Merger;
};

} // end of namespace cdacspatialjoin