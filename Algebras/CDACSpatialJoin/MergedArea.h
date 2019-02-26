/*
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

#include <memory>
#include <vector>

#include "JoinEdge.h"

namespace cdacspatialjoin {

class MergedArea;
typedef std::shared_ptr<MergedArea> MergedAreaPtr;

class MergedArea {
   /* the index in JoinState::joinEdges from which the JoinEdges covered by
    * this MergedArea start (inclusive) */
   EdgeIndex_t edgeIndexStart;
   /* the index in JoinState::joinEdges at which the JoinEdges covered by
    * this MergedArea end (exclusive) */
   EdgeIndex_t edgeIndexEnd;

   // TODO: ggf. auch ausprobieren, ob joinEdges-Indizes statt Instanzen
   // effizienter wären

   /* the left edges from set A inside this MergedArea, for which the
    * corresponding right edge is not inside this MergedArea */
   std::vector<JoinEdge> leftA;
   /* the right edges from set A inside this MergedArea, for which the
    * corresponding left edge is not inside this MergedArea */
   std::vector<JoinEdge> rightA;
   /* the rectangles from set A represented in this MergedArea by both their
    * left and right edges */
   std::vector<JoinEdge> completeA;

   /* the left edges from set B inside this MergedArea, for which the
    * corresponding right edge is not inside this MergedArea */
   std::vector<JoinEdge> leftB;
   /* the right edges from set B inside this MergedArea, for which the
    * corresponding left edge is not inside this MergedArea */
   std::vector<JoinEdge> rightB;
   /* the rectangles from set B represented in this MergedArea by both their
    * left and right edges */
   std::vector<JoinEdge> completeB;

public:
   /* instantiates an atomic MergedArea which covers an interval in which
    * only JoinEdges from the same set are to be found */
   MergedArea(const std::vector<JoinEdge>& joinEdges,
           EdgeIndex_t edgeIndexStart_, EdgeIndex_t edgeIndexEnd_, SET set);

   /* instantiates a MergedArea that can hold the result of merging the given
    * MergedArea instances. area1 and area2 must be adjoining */
   MergedArea(MergedAreaPtr& area1, MergedAreaPtr& area2);

   ~MergedArea() = default;

   /* returns the left edges from the given set inside this MergedArea */
   inline std::vector<JoinEdge>& getLeft(SET set) {
      return (set == SET::A) ? leftA : leftB;
   }

   /* returns the right edges from the given set inside this MergedArea */
   inline std::vector<JoinEdge>& getRight(SET set) {
      return (set == SET::A) ? rightA : rightB;
   }

   /* returns the rectangles from the given set fully represented inside this
    * MergedArea */
   inline std::vector<JoinEdge>& getComplete(SET set) {
      return (set == SET::A) ? completeA : completeB;
   }

   /* returns a short string representation of this MergedArea for console
    * output*/
   std::string toString() const;

private:
   /* returns true if the counterpart (i.e. the corresponding left or right
    * edge) of the given edge is found inside this MergedArea */
   inline bool containsCounterpartOf(const JoinEdge &edge) const {
      return (edge.counterPartEdgeIndex >= edgeIndexStart &&
              edge.counterPartEdgeIndex < edgeIndexEnd);
}

friend class Merger;
};

} // end of namespace cdacspatialjoin