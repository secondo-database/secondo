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


1 Merger

*/

#include <assert.h>
#include <iostream>
#include <iomanip>

#include "Merger.h"
#include "Utils.h"

using namespace cdacspatialjoin;
using namespace std;


/*
1 MergerStats struct

*/
MergerStats::MergerStats() :
   reportPairsCount(0),
   reportPairsSubCount(0),
   reportPairsSub1Count(0),
   reportPairsSub11Count(0),
   loopStats {0} {
}

void MergerStats::add(size_t cycleCount) {
   unsigned lbCycleCount = 0; // the binary logarithm of cycleCount
   while (cycleCount != 0) {
      ++lbCycleCount;
      cycleCount >>= 1U;
   }
   assert (lbCycleCount < LOOP_STATS_COUNT);
   ++loopStats[lbCycleCount];
}

void MergerStats::report(std::ostream& out) const {
   unsigned lastEntry = 0;
   uint64_t totalLoopCount = 0;
   for (unsigned i = 0; i < LOOP_STATS_COUNT; ++i) {
      if (loopStats[i] > 0) {
         lastEntry = i;
         totalLoopCount += loopStats[i];
      }
   }
   if (lastEntry > 0) {
      cout << endl << "Statistics for Merger::reportPairs(): "
           << formatInt(reportPairsCount) << " calls; " << endl;

      const uint64_t subSum = reportPairsSubCount + reportPairsSub1Count +
                              reportPairsSub11Count;
      cout << "Statistics for Merger::reportPairsSub(): "
           << formatInt(subSum) << " calls total: " << endl;
      cout << setw(14) << formatInt(reportPairsSubCount) << " calls "
           << "(" << reportPairsSubCount * 100.0 / subSum << " %) "
           << "with multiple edges in both sets + " << endl;
      cout << setw(14) << formatInt(reportPairsSub1Count) << " calls "
           << "(" << reportPairsSub1Count * 100.0 / subSum << " %) "
           << "with only one edge in either of the sets + " << endl;
      cout << setw(14) << formatInt(reportPairsSub11Count) << " calls "
           << "(" << reportPairsSub11Count * 100.0 / subSum << " %) "
           << "with only one edge in both of the sets" << endl;

      cout << "In a total of   " << formatInt(totalLoopCount) << " loops "
           << "in reportPairsSub() (multiple edges), the cycle count was ..."
           << endl;

      uint64_t cycleCount = 1;
      for (unsigned i = 0; i <= lastEntry; ++i) {
         if (i == 0 && loopStats[i] == 0)
            continue; // skip this if such while loops are prevented
         cout << "< " << setw(7) << formatInt(cycleCount) << ": "
              << setw(14) << formatInt(loopStats[i]) << " loops"
              << " (" << loopStats[i] * 100.0 / totalLoopCount << " %)" << endl;
         cycleCount <<= 1U;
      }
      cout << endl;
   }
}


/*
2 Merger class

*/
#ifdef CDAC_SPATIAL_JOIN_METRICS
unique_ptr<MergerStats> Merger::stats(new MergerStats());

void Merger::resetLoopStats() {
   stats.reset(new MergerStats());
}
#endif

Merger::Merger(MergedAreaPtr area1_, MergedAreaPtr area2_,
        const bool isLastMerge_, IOData* const ioData_) :
      area1(area1_),
      area2(area2_),
      isLastMerge(isLastMerge_),
      ioData(ioData_),
      result(new MergedArea(area1, area2)),
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
   // leftA/BSpan, rightA/BSpan, and leftRightA/B
   removeCompleteRectangles(area1_->leftA, area2_->rightA,
                            leftASpan, rightASpan, leftRightA);
   removeCompleteRectangles(area1_->leftB, area2_->rightB,
                            leftBSpan, rightBSpan, leftRightB);

   currentTask = TASK::report;
}

Merger::~Merger() {
   delete area1;
   delete area2;
}

bool Merger::merge() {
   static constexpr unsigned REPORT_TYPE_COUNT = 4;

   // outer loop of reportPairs() calls
   while (currentTask == TASK::report && reportType < REPORT_TYPE_COUNT) {
      bool done;
      switch(reportType) {
         case 0:
            done = leftASpan.empty() ||
                reportPairs(leftASpan, area2->leftB, area2->completeB);
            break;
         case 1:
            done = rightASpan.empty() ||
                reportPairs(rightASpan, area1->leftB, area1->completeB);
            break;
         case 2:
            done = leftBSpan.empty() ||
                reportPairs(leftBSpan, area2->leftA, area2->completeA);
            break;
         case 3:
            done = rightBSpan.empty() ||
                reportPairs(rightBSpan, area1->leftA, area1->completeA);
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

   // post-processing: create the result MergedArea
   if (currentTask == TASK::postProcess) {
      if (isLastMerge) {
         // do nothing: the result areas are obsolete
      } else {
         merge(leftASpan, 0, area2->leftA, 0, result->leftA);
         merge(rightASpan, 0, area1->rightA, 0, result->rightA);
         merge(area1->completeA, area2->completeA, leftRightA,
               result->completeA);

         merge(leftBSpan, 0, area2->leftB, 0, result->leftB);
         merge(rightBSpan, 0, area1->rightB, 0, result->rightB);
         merge(area1->completeB, area2->completeB, leftRightB,
               result->completeB);
      }
      currentTask = TASK::done;
   }

   // assert (currentTask == TASK::done);
   return true;
}

bool Merger::reportPairs(const JoinEdgeVec& span,
                         const JoinEdgeVec& left,
                         const JoinEdgeVec& complete) {
   static constexpr unsigned REPORT_SUB_TYPE_COUNT = 2;

   // inner loop of reportPairs() calls
   while (reportSubType < REPORT_SUB_TYPE_COUNT) {
      bool done = false;
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
   ++stats->reportPairsCount;
#endif

   // report type completed, reset reportSubType to 0 for next reportType
   reportSubType = 0;
   return true;
}

bool Merger::reportPairsSub(const JoinEdgeVec& edgesS,
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
            stats->add(0);
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
                    !ioData_->appendToOutput(edgeSBegin, edgesT[indexT_]);

            // increase indexT before a possible "return false", so this pair
            // will not be reported again
            ++indexT_;

            if (outTBlockFull) {
               // outTBlock is full. Index positions remain stored in
               // this Merger instance for the next call (with a new outTBlock)

               // write back local copies to fields
               indexSBegin = indexSBegin_;
               indexTBegin = indexTBegin_;
               indexS = indexS_;
               indexT = indexT_;
               return false;
            } // otherwise continue reporting
         }
#ifdef CDAC_SPATIAL_JOIN_METRICS
         stats->add(indexT_ - indexTBegin_);
#endif
         indexT_ = indexTBegin_;

         // proceed to next edge in edgesS
         ++indexSBegin_;

      } else { // edgeTBegin.yMin <= edgeSBegin.yMin
         const double yMaxT = edgeTBegin.yMax;
         // speed up the most frequent case (for many large joins, 99%)
         if (edgeSBegin.yMin > yMaxT) {
#ifdef CDAC_SPATIAL_JOIN_METRICS
            stats->add(0);
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
                    !ioData_->appendToOutput(edgesS[indexS_], edgeTBegin);

            // increase indexS before a possible "return false", so this pair
            // will not be reported again
            ++indexS_;

            if (outTBlockFull) {
               // outTBlock is full. Index positions remain stored in
               // this Merger instance for the next call (with a new outTBlock)

               // write back local copies to fields
               indexSBegin = indexSBegin_;
               indexTBegin = indexTBegin_;
               indexS = indexS_;
               indexT = indexT_;
               return false;
            } // otherwise continue reporting
         }
#ifdef CDAC_SPATIAL_JOIN_METRICS
         stats->add(indexS_ - indexSBegin_);
#endif
         indexS_ = indexSBegin_;

         // proceed to next edge in edgesT
         ++indexTBegin_;
      }

   }

#ifdef CDAC_SPATIAL_JOIN_METRICS
   ++stats->reportPairsSubCount;
#endif

   // reset indices for next call
   indexSBegin = 0;
   indexS = 0;
   indexTBegin = 0;
   indexT = 0;

   // report subtype completed
   return true;
}

bool Merger::reportPairsSub1(const JoinEdge& edgeS,
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
         if (!ioData_->appendToOutput(edgeS, edgeTBegin)) {
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
         if (!ioData_->appendToOutput(edgeS, edgeT)) {
            // write back local copies to fields
            indexTBegin = indexTBegin_;
            indexT = indexT_;
            return false;
         }
      }
   }
   // now all possible intersections were reported

#ifdef CDAC_SPATIAL_JOIN_METRICS
   ++stats->reportPairsSub1Count;
#endif

   // reset indices for next call (indexSBegin and indexS were not used here)
   indexTBegin = 0;
   indexT = 0;

   // report subtype completed
   return true;
}

bool Merger::reportPairsSub11(const JoinEdge& edgeS,
                             const JoinEdge& edgeT) {
   if (indexTBegin == 0) {
      if ((edgeS.yMax >= edgeT.yMin && edgeS.yMin <= edgeT.yMax)) {
         // report intersection
         if (!ioData->appendToOutput(edgeS, edgeT)) {
            indexTBegin = 1; // prevent multiple reporting
            return false;
         }
      }
   }

#ifdef CDAC_SPATIAL_JOIN_METRICS
   ++stats->reportPairsSub11Count;
#endif

   // reset indices for next call
   indexTBegin = 0;

   // report subtype completed
   return true;
}

void Merger::removeCompleteRectangles(
        const JoinEdgeVec& left1,
        const JoinEdgeVec& right2,
        JoinEdgeVec& leftSpan,
        JoinEdgeVec& rightSpan,
        JoinEdgeVec& leftRight) {

   // get some values frequently used in the loops below
   const size_t size1 = left1.size();
   const size_t size2 = right2.size();
   const bool isNotLastMerge = !isLastMerge;
   const EdgeIndex_t resultEdgeStart = result->edgeIndexStart;
   const EdgeIndex_t resultEdgeEnd = result->edgeIndexEnd;

   // initialize indices
   size_t index1 = 0;
   size_t index2 = 0;

   // copy entries from both vectors in sort order as given by operator<
   while (index1 < size1 && index2 < size2) {
      const JoinEdge& edge1 = left1[index1];
      const JoinEdge& edge2 = right2[index2];
      if (edge1.yMin < edge2.yMin) {
         // inline version of "if (result->containsCounterPartOf(edge1))":
         const EdgeIndex_t counterPart = edge1.isLeftAndCounterPart &
                                         JoinEdge::COUNTER_PART_MASK;
         if (counterPart >= resultEdgeStart && counterPart < resultEdgeEnd) {
            if (isNotLastMerge) // in the last merge, leftRight is obsolete
               leftRight.push_back(edge1);
         } else {
            leftSpan.push_back(edge1);
         }
         ++index1;
      } else {
         // inline version of "if (result->containsCounterPartOf(edge2))":
         const EdgeIndex_t counterPart = edge2.isLeftAndCounterPart &
                                         JoinEdge::COUNTER_PART_MASK;
         if (counterPart >= resultEdgeStart && counterPart < resultEdgeEnd) {
            // do nothing: the left counterpart is in left1
            // and therefore was (or will be) added to leftRight
         } else {
            rightSpan.push_back(edge2);
         }
         ++index2;
      }
   }

   // copy the remaining entries from left1, if any
   while (index1 < size1) {
      const JoinEdge& edge1 = left1[index1];
      // inline version of "if (result->containsCounterPartOf(edge1))":
      const EdgeIndex_t counterPart = edge1.isLeftAndCounterPart &
                                      JoinEdge::COUNTER_PART_MASK;
      if (counterPart >= resultEdgeStart && counterPart < resultEdgeEnd) {
         if (isNotLastMerge) // in the last merge, leftRight is obsolete
            leftRight.push_back(edge1);
      } else {
         leftSpan.push_back(edge1);
      }
      ++index1;
   }

   // copy the remaining entries from right2, if any
   while (index2 < size2) {
      const JoinEdge& edge2 = right2[index2];
      // inline version of "if (result->containsCounterPartOf(edge2))":
      const EdgeIndex_t counterPart = edge2.isLeftAndCounterPart &
                                      JoinEdge::COUNTER_PART_MASK;
      if (counterPart >= resultEdgeStart && counterPart < resultEdgeEnd) {
         // do nothing: the left counterpart is in left1
         // and therefore was (or will be) added to leftRight
      } else {
         rightSpan.push_back(edge2);
      }
      ++index2;
   }
}

void Merger::merge(
        const JoinEdgeVec& source1, const size_t startIndex1,
        const JoinEdgeVec& source2, const size_t startIndex2,
        JoinEdgeVec& dest) {

   // get some values frequently used in the loops below
   const size_t size1 = source1.size();
   const size_t size2 = source2.size();

   // initialize indices
   size_t index1 = startIndex1;
   size_t index2 = startIndex2;
   // assert (index1 <= size1 && index2 <= size2);

   // reserve space in dest vector
   dest.reserve(dest.size() + size1 - index1 + size2 - index2);

   // copy entries from both vectors in sort order as given by operator<
   if (index1 < size1 && index2 < size2) {
      double yMin1 = source1[index1].yMin;
      double yMin2 = source2[index2].yMin;
      do {
         if (yMin1 < yMin2) {
            dest.push_back(source1[index1]);
            if (++index1 == size1)
               break;
            yMin1 = source1[index1].yMin;
         } else {
            dest.push_back(source2[index2]);
            if (++index2 == size2)
               break;
            yMin2 = source2[index2].yMin;
         }
      } while(true);
   }

   // copy the remaining entries from one vector
   if (index1 < size1) {
      dest.insert(dest.end(), source1.begin() + index1, source1.end());
   } else if (index2 < size2) {
      dest.insert(dest.end(), source2.begin() + index2, source2.end());
   }
}

void Merger::merge(const JoinEdgeVec& source1,
                   const JoinEdgeVec& source2,
                   const JoinEdgeVec& source3,
                   JoinEdgeVec& dest) {

   // get some values frequently used in the loops below
   const size_t size1 = source1.size();
   const size_t size2 = source2.size();
   const size_t size3 = source3.size();

   // initialize indices
   size_t index1 = 0;
   size_t index2 = 0;
   size_t index3 = 0;

   // reserve space in dest vector
   dest.reserve(dest.size() + size1 + size2 + size3);

   // copy entries from all three vectors in sort order as given by operator<
   if (size1 != 0 && size2 != 0 && size3 != 0) {
      // rather than using array access for each comparison
      //   "if (source1[index1].yMin < source2[index2].yMin)",
      // yMin values are copied to local variables which are only updated
      // when necessary (i.e. when index1 etc. changes)
      double yMin1 = source1[index1].yMin;
      double yMin2 = source2[index2].yMin;
      double yMin3 = source3[index3].yMin;
      do {
         if (yMin1 < yMin2) {
            if (yMin1 < yMin3) {
               dest.push_back(source1[index1]);
               if (++index1 == size1)
                  break;
               yMin1 = source1[index1].yMin;
               continue;
            }
         } else { // yMin2 <= yMin1
            if (yMin2 < yMin3) {
               dest.push_back(source2[index2]);
               if (++index2 == size2)
                  break;
               yMin2 = source2[index2].yMin;
               continue;
            }
         }
         // yMin3 is smallest
         dest.push_back(source3[index3]);
         if (++index3 == size3)
            break;
         yMin3 = source3[index3].yMin;
      } while(true);
   }

   // copy entries from the remaining two vectors
   if (index1 == size1)
      merge(source2, index2, source3, index3, dest);
   else if (index2 == size2)
      merge(source1, index1, source3, index3, dest);
   else
      merge(source1, index1, source2, index2, dest);
}

size_t Merger::getJoinEdgeCount(const bool includeAreas) const {
   size_t joinEdgeCount =
             leftASpan.size() + rightASpan.size() + leftRightA.size()
           + leftBSpan.size() + rightBSpan.size() + leftRightB.size();
   if (includeAreas) {
      joinEdgeCount += area1->getJoinEdgeCount() + area2->getJoinEdgeCount();
      joinEdgeCount += result->getJoinEdgeCount();
   }
   return joinEdgeCount;
}

size_t Merger::getUsedMemory() const {
   size_t usedMemory = sizeof(Merger)
       + getJoinEdgeCount(false) * sizeof(JoinEdge)
       + area1->getUsedMemory() + area2->getUsedMemory()
       + result->getUsedMemory();
   return usedMemory;
}
