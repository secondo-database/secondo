/*
1 MergedArea class

*/

#include <assert.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>

#include "MergedArea.h"

using namespace cdacspatialjoin;
using namespace std;

MergedArea::MergedArea(const vector<JoinEdge>& joinEdges,
                       const EdgeIndex_t edgeIndexStart_,
                       const EdgeIndex_t edgeIndexEnd_,
                       const SET set) :
        edgeIndexStart(edgeIndexStart_),
        edgeIndexEnd(edgeIndexEnd_) {

   JoinEdgeVec& left = (set == SET::A) ? leftA : leftB;
   JoinEdgeVec& right = (set == SET::A) ? rightA : rightB;
   JoinEdgeVec& complete = (set == SET::A) ? completeA : completeB;

   // read the JoinEdges from the given vector and enter them to the
   // appropriate vector left, right, or complete
   for (EdgeIndex_t i = edgeIndexStart; i < edgeIndexEnd; ++i) {
      const JoinEdge& edge = joinEdges[i];
      const bool isLeft = edge.getIsLeft();

      if (this->containsCounterpartOf(edge)) {
         if (isLeft) {
            complete.push_back(edge);
         } else {
            // ignore edge (since the corresponding left edge is in this same
            // MergedArea and therefore is added to the "complete" vector
         }
      } else {
         if (isLeft)
            left.push_back(edge);
         else
            right.push_back(edge);
      }
   }

   // sort vectors by the yMin value of their JoinEdge entries to comply with
   // the MergedArea invariants
   if (left.size() > 1)
      std::sort(left.begin(), left.end());
   if (right.size() > 1)
      std::sort(right.begin(), right.end());
   if (complete.size() > 1)
      std::sort(complete.begin(), complete.end());
}

MergedArea::MergedArea(MergedAreaPtr& area1, MergedAreaPtr& area2) :
        edgeIndexStart(area1->edgeIndexStart),
        edgeIndexEnd(area2->edgeIndexEnd) {

   assert (area1->edgeIndexEnd == area2->edgeIndexStart);

   // the vectors left, right, and complete will be populated by the caller
   // (i.e. by a Merger instance); by then, the exact number of entries
   // will be known (and reserved)
}

std::string MergedArea::toString() const {
   stringstream st;
   // represent the MergedArea by the indices of JoinState::joinEdges
   // that are covered (inclusive, therefor edgeIndexEnd - 1)
   if (edgeIndexStart + 1 == edgeIndexEnd)
      st << edgeIndexStart;
   else
      st << edgeIndexStart << "--" << edgeIndexEnd - 1;
   return st.str();
}
