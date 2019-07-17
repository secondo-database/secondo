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

#include "SelfMergedArea.h"

#include <assert.h>
#include <algorithm>
#include <string>
#include <sstream>

using namespace cdacspatialjoin;
using namespace std;

SelfMergedArea::SelfMergedArea(const vector<JoinEdge>& joinEdges,
        const EdgeIndex_t edgeIndexStart_, const EdgeIndex_t edgeIndexEnd_,
        bool isLeft) :
        edgeIndexStart(edgeIndexStart_),
        edgeIndexEnd(edgeIndexEnd_) {

   // read the JoinEdges from the given vector and enter it to the
   // appropriate vector left or right
   if (isLeft) {
      // an atomic SelfMergedArea can hold only a single left edge
      // assert (edgeIndexStart_ + 1 == edgeIndexEnd_);
      left.reserve(1);
      left.push_back(joinEdges[edgeIndexStart_]);
   } else {
      // transfer the JoinEdges to the vector "right"
      size_t size = edgeIndexEnd_ - edgeIndexStart_;
      right.reserve(size);
      for (EdgeIndex_t i = edgeIndexStart_; i < edgeIndexEnd_; ++i) {
         right.push_back(joinEdges[i]);
      }

      if (size > 1) {
         std::qsort(&right[0], size, sizeof(JoinEdge), JoinEdge::compare);
         // std::sort(right.begin(), right.end());
      }
   }
}

SelfMergedArea::SelfMergedArea(SelfMergedAreaPtr area1,
        SelfMergedAreaPtr area2) :
        edgeIndexStart(area1->edgeIndexStart),
        edgeIndexEnd(area2->edgeIndexEnd) {

   // assert (area1->edgeIndexEnd == area2->edgeIndexStart);

   // the vectors left, right, and complete will be populated by the caller
   // (i.e. by a SelfMerger instance); by then, the exact number of entries
   // will be known (and reserved)
}

std::string SelfMergedArea::toString() const {
   stringstream st;
   // represent the SelfMergedArea by the indices of JoinState::joinEdges
   // that are covered (inclusive, therefore edgeIndexEnd - 1)
   if (edgeIndexStart + 1 == edgeIndexEnd)
      st << edgeIndexStart;
   else
      st << edgeIndexStart << "--" << edgeIndexEnd - 1;
   return st.str();
}

size_t SelfMergedArea::getJoinEdgeCount() const {
   return left.size() + right.size() + complete.size();
}

size_t SelfMergedArea::getUsedMemory() const {
   return sizeof(SelfMergedArea) + getJoinEdgeCount() * sizeof(JoinEdge);
}
