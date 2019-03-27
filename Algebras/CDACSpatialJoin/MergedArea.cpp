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

*/

#include "MergedArea.h"

#include <assert.h>
#include <algorithm>
#include <string>
#include <sstream>

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
   for (EdgeIndex_t i = edgeIndexStart_; i < edgeIndexEnd_; ++i) {
      const JoinEdge& edge = joinEdges[i];
      // "inline" version of JoinEdge::getIsLeft()
      const bool isLeft = (edge.isLeftAndCounterPart & JoinEdge::IS_LEFT_MASK)
                          == 0;

      // inline version of "if (this->containsCounterPartOf(edge))":
      const EdgeIndex_t counterPart = edge.isLeftAndCounterPart &
                                      JoinEdge::COUNTER_PART_MASK;
      if (counterPart >= edgeIndexStart_ && counterPart < edgeIndexEnd_) {
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
   size_t leftSize = left.size();
   if (leftSize > 1) {
      std::qsort(&left[0], leftSize, sizeof(JoinEdge), JoinEdge::compare);
      // qsort seems to be faster than
      // std::sort(left.begin(), left.end());
   }
   size_t rightSize = right.size();
   if (rightSize  > 1) {
      std::qsort(&right[0], rightSize, sizeof(JoinEdge), JoinEdge::compare);
      // std::sort(right.begin(), right.end());
   }
   size_t completeSize = complete.size();
   if (completeSize > 1) {
      std::qsort(&complete[0], completeSize, sizeof(JoinEdge),
             JoinEdge::compare);
      // std::sort(complete.begin(), complete.end());
   }
}

MergedArea::MergedArea(MergedAreaPtr area1, MergedAreaPtr area2) :
        edgeIndexStart(area1->edgeIndexStart),
        edgeIndexEnd(area2->edgeIndexEnd) {

   // assert (area1->edgeIndexEnd == area2->edgeIndexStart);

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
