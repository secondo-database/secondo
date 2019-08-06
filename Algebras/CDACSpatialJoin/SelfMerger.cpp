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


1 SelfMerger

*/

#include <assert.h>
#include <iostream>
#include <iomanip>

#include "SelfMerger.h"
#include "Utils.h"

using namespace cdacspatialjoin;
using namespace std;


/*
1 SelfMerger class

*/
SelfMerger::SelfMerger(SelfMergedAreaPtr area1_, SelfMergedAreaPtr area2_,
               const bool isLastMerge_, IOData* const ioData_) :
        area1(area1_),
        area2(area2_),
        isLastMerge(isLastMerge_),
        ioData(ioData_),
        result(new SelfMergedArea(area1, area2)),
        currentTask(TASK::initialize),
        reportType(0),
        reportSubType(0),
        indexSBegin(0),
        indexS(0),
        indexTBegin(0),
        indexT(0) {

#ifdef CDAC_SPATIAL_JOIN_DETAILED_REPORT_TO_CONSOLE
   cout << endl;
   cout << "merge " << area1->toString() << " | " << area2->toString() << endl;
#endif

   // pre-process given areas by calculating from them the temporary vectors
   // leftSpan, rightSpan, and leftRight
   JoinEdge::removeCompleteRectangles(area1_->left, area2_->right,
           leftSpan, rightSpan, leftRight, !isLastMerge,
           result->edgeIndexStart, result->edgeIndexEnd);

   currentTask = TASK::report;
}

SelfMerger::~SelfMerger() {
   delete area1;
   delete area2;
}

bool SelfMerger::merge() {
   static constexpr unsigned REPORT_TYPE_COUNT = 2;

   // outer loop of reportPairs() calls
   while (currentTask == TASK::report && reportType < REPORT_TYPE_COUNT) {
      bool done;
      switch(reportType) {
         case 0:
            done = leftSpan.empty() ||
                   reportPairs(leftSpan, area2->left, area2->complete);
            break;
         case 1:
            done = rightSpan.empty() ||
                   reportPairs(rightSpan, area1->left, area1->complete);
            break;
         default:
            assert (false); // unexpected case
            break;
      }

      if (!done)
         return false; // outTBlock full, continue at next merge() call

      // reportPairs() was completed for the given reportType,
      // proceed with next reportType
      ++reportType;
      if (reportType == REPORT_TYPE_COUNT)
         currentTask = TASK::postProcess; // report fully completed
   }

   // post-processing: create the result SelfMergedArea
   if (currentTask == TASK::postProcess) {
      if (isLastMerge) {
         // do nothing: the result areas are obsolete
      } else {
         JoinEdge::merge(leftSpan, 0, area2->left, 0, result->left);
         JoinEdge::merge(rightSpan, 0, area1->right, 0, result->right);
         JoinEdge::merge(area1->complete, area2->complete, leftRight,
               result->complete);
      }
      currentTask = TASK::done;
   }

   // assert (currentTask == TASK::done);
   return true;
}

bool SelfMerger::reportPairs(const JoinEdgeVec& span,
                         const JoinEdgeVec& left,
                         const JoinEdgeVec& complete) {
   static constexpr unsigned REPORT_SUB_TYPE_COUNT = 2;

   // inner loop of reportPairs() calls
   while (reportSubType < REPORT_SUB_TYPE_COUNT) {
      bool done;
      if (reportSubType == 0) {
         done = left.empty() || reportPairsSub(span, left);
      } else { // reportSubType == 1
         done = complete.empty() || reportPairsSub(span, complete);
      }

      if (!done)
         return false; // outTBlock full, continue at next merge() call

      // reportPairsSub() was completed for the given reportSubType,
      // proceed with next reportSubType
      ++reportSubType;
   }

#ifdef CDAC_SPATIAL_JOIN_METRICS
   ++MergerStats::onlyInstance->reportPairsCount;
#endif

   // report type completed, reset reportSubType to 0 for next reportType
   reportSubType = 0;
   return true;
}

bool SelfMerger::reportPairsSub(const JoinEdgeVec& edgesS,
                                const JoinEdgeVec& edgesT) {

   // get some values frequently used in the loop below; use specialized
   // functions if there is only one edge in one of the vectors
   const size_t sizeS = edgesS.size();
   const size_t sizeT = edgesT.size();

   // refer special cases to specialized functions. sizeS/T can be passed, too
   // since up to 4 parameters can be passed in registers
   if (sizeS == 1 && sizeT == 1)
      return reportPairsSub11(edgesS[0], edgesT[0]);
   if (sizeS == 1)
      return reportPairsSub1(edgesS[0], edgesT, sizeT);
   if (sizeT == 1)
      return reportPairsSub1(edgesT[0], edgesS, sizeS);
   // otherwise, both sets contain multiple edges which will be treated below

   // improve performance by using local variables for fields
   IOData* const ioData_ = ioData;
   size_t indexSBegin_ = indexSBegin;
   size_t indexTBegin_ = indexTBegin;
   size_t indexS_ = indexS;
   size_t indexT_ = indexT;

   while (indexSBegin_ < sizeS && indexTBegin_ < sizeT) {
      // determine the current edgeS/TBegin
      const JoinEdge& edgeSBegin = edgesS[indexSBegin_];
      const JoinEdge& edgeTBegin = edgesT[indexTBegin_];

      if (edgeSBegin.yMin < edgeTBegin.yMin) {
         const double yMaxS = edgeSBegin.yMax;
         // speed up the most frequent case (for many large joins, 99%)
         if (edgeTBegin.yMin > yMaxS) {
#ifdef CDAC_SPATIAL_JOIN_METRICS
            MergerStats::onlyInstance->add(0);
#endif
            ++indexSBegin_;
            continue;
         }

         // find intersections of the y range of edgeSBegin and edges in
         // edgesT[] (starting from indexTBegin)
         if (indexT_ < indexTBegin_)
            indexT_ = indexTBegin_;
         while (indexT_ < sizeT && edgesT[indexT_].yMin <= yMaxS) {
            // report pair of rectangles (represented by the two edges);
            // edges can be passed to the output function in any order
            const bool outTBlockFull =
                 !ioData_->selfJoinAppendToOutput(edgeSBegin, edgesT[indexT_]);

            // increase indexT before a possible "return false", so this pair
            // will not be reported again
            ++indexT_;

            if (outTBlockFull) {
               // outTBlock is full. Index positions remain stored in this
               // SelfMerger instance for the next call (with a new outTBlock)

               // write back local copies to fields
               indexSBegin = indexSBegin_;
               indexTBegin = indexTBegin_;
               indexS = indexS_;
               indexT = indexT_;
               return false;
            } // otherwise continue reporting
         }
#ifdef CDAC_SPATIAL_JOIN_METRICS
         MergerStats::onlyInstance->add(indexT_ - indexTBegin_);
#endif
         indexT_ = indexTBegin_;

         // proceed to next edge in edgesS
         ++indexSBegin_;

      } else { // edgeTBegin.yMin <= edgeSBegin.yMin
         const double yMaxT = edgeTBegin.yMax;
         // speed up the most frequent case (for many large joins, 99%)
         if (edgeSBegin.yMin > yMaxT) {
#ifdef CDAC_SPATIAL_JOIN_METRICS
            MergerStats::onlyInstance->add(0);
#endif
            ++indexTBegin_;
            continue;
         }

         // find intersections of the y range of edgeTBegin and edges in
         // edgesS[] (starting from indexSBegin)
         if (indexS_ < indexSBegin_)
            indexS_ = indexSBegin_;
         while (indexS_ < sizeS && edgesS[indexS_].yMin <= yMaxT) {
            // report pair of rectangles (represented by the two edges);
            // edges can be passed to the output function in any order
            const bool outTBlockFull =
                 !ioData_->selfJoinAppendToOutput(edgesS[indexS_], edgeTBegin);

            // increase indexS before a possible "return false", so this pair
            // will not be reported again
            ++indexS_;

            if (outTBlockFull) {
               // outTBlock is full. Index positions remain stored in this
               // SelfMerger instance for the next call (with a new outTBlock)

               // write back local copies to fields
               indexSBegin = indexSBegin_;
               indexTBegin = indexTBegin_;
               indexS = indexS_;
               indexT = indexT_;
               return false;
            } // otherwise continue reporting
         }
#ifdef CDAC_SPATIAL_JOIN_METRICS
         MergerStats::onlyInstance->add(indexS_ - indexSBegin_);
#endif
         indexS_ = indexSBegin_;

         // proceed to next edge in edgesT
         ++indexTBegin_;
      }

   }

#ifdef CDAC_SPATIAL_JOIN_METRICS
   ++MergerStats::onlyInstance->reportPairsSubCount;
#endif

   // reset indices for next call
   indexSBegin = 0;
   indexS = 0;
   indexTBegin = 0;
   indexT = 0;

   // report subtype completed
   return true;
}

bool SelfMerger::reportPairsSub1(const JoinEdge& edgeS,
                             const JoinEdgeVec& edgesT, const size_t sizeT) {

   // improve performance by using local variable for field "indexTBegin"
   IOData* const ioData_ = ioData;
   size_t indexTBegin_ = indexTBegin;

   // iterating over the edgesT[] entries,
   // first treat all cases in which edgeT.yMin <= edgeS.yMin
   const double yMinS = edgeS.yMin;
   while (indexTBegin_ < sizeT)  {
      const JoinEdge& edgeTBegin = edgesT[indexTBegin_];
      if (edgeTBegin.yMin > yMinS)
         break;
      ++indexTBegin_; // prepare next iteration in case of "return false"
      if (yMinS <= edgeTBegin.yMax) {
         if (!ioData_->selfJoinAppendToOutput(edgeS, edgeTBegin)) {
            // write back local copy to field
            indexTBegin = indexTBegin_;
            return false;
         }
      }
   }
   // now, for all remaining edgeT, yMinS < edgeT.yMin

   // continue iterating while edgeT.yMin is in the interval (yMinS, yMaxS]
   const double yMaxS = edgeS.yMax;
   if (indexTBegin_ < sizeT) {
      // improve performance by using local variable for field "indexT"
      size_t indexT_ = indexT;
      if (indexT_ < indexTBegin_)
         indexT_ = indexTBegin_;

      while (indexT_ < sizeT) {
         const JoinEdge& edgeT = edgesT[indexT_];
         if (edgeT.yMin > yMaxS)
            break;
         ++indexT_; // prepare next iteration in case of "return false"
         if (!ioData_->selfJoinAppendToOutput(edgeS, edgeT)) {
            // write back local copies to fields
            indexTBegin = indexTBegin_;
            indexT = indexT_;
            return false;
         }
      }
   }
   // now all possible intersections were reported

#ifdef CDAC_SPATIAL_JOIN_METRICS
   ++MergerStats::onlyInstance->reportPairsSub1Count;
#endif

   // reset indices for next call (indexSBegin and indexS were not used here)
   indexTBegin = 0;
   indexT = 0;

   // report subtype completed
   return true;
}

bool SelfMerger::reportPairsSub11(const JoinEdge& edgeS,
                              const JoinEdge& edgeT) {
   if (indexTBegin == 0) {
      if ((edgeS.yMax >= edgeT.yMin && edgeS.yMin <= edgeT.yMax)) {
         // report intersection
         if (!ioData->selfJoinAppendToOutput(edgeS, edgeT)) {
            indexTBegin = 1; // prevent multiple reporting
            return false;
         }
      }
   }

#ifdef CDAC_SPATIAL_JOIN_METRICS
   ++MergerStats::onlyInstance->reportPairsSub11Count;
#endif

   // reset indices for next call
   indexTBegin = 0;

   // report subtype completed
   return true;
}

size_t SelfMerger::getJoinEdgeCount(const bool includeAreas) const {
   size_t joinEdgeCount =
           leftSpan.size() + rightSpan.size() + leftRight.size();
   if (includeAreas) {
      joinEdgeCount += area1->getJoinEdgeCount() + area2->getJoinEdgeCount();
      joinEdgeCount += result->getJoinEdgeCount();
   }
   return joinEdgeCount;
}

size_t SelfMerger::getUsedMemory() const {
   size_t usedMemory = sizeof(SelfMerger)
                       + getJoinEdgeCount(false) * sizeof(JoinEdge)
                       + area1->getUsedMemory() + area2->getUsedMemory()
                       + result->getUsedMemory();
   return usedMemory;
}
