/*
1 Merger

*/

#include <assert.h>
#include <iostream>
#include <iomanip>

#include "Merger.h"
#include "Utils.h"

using namespace cdacspatialjoin;
using namespace std;

#ifdef CDAC_SPATIAL_JOIN_METRICS
uint64_t Merger::reportPairsCount = 0;
uint64_t Merger::reportPairsSubCount = 0;
uint64_t Merger::reportPairsSub1Count = 0;
uint64_t Merger::reportPairsSub11Count = 0;
uint64_t Merger::loopStats[LOOP_STATS_COUNT];
#endif

Merger::Merger(MergedAreaPtr& area1_, MergedAreaPtr& area2_,
        const bool isLastMerge_) :
      area1(area1_),
      area2(area2_),
      isLastMerge(isLastMerge_),
      result(make_shared<MergedArea>(area1_, area2_)),
      currentTask(TASK::initialize) {

#ifdef CDAC_SPATIAL_JOIN_DETAILED_REPORT_TO_CONSOLE
   cout << endl;
   cout << "merge " << area1->toString() << " | " << area2->toString() << endl;
#endif

   // pre-process given areas by calculating from them the temporary vectors
   // leftA/BSpan, rightA/BSpan, and leftRightA/B
   removeCompleteRectangles(area1->leftA, area2->rightA,
                            leftASpan, rightASpan, leftRightA);
   removeCompleteRectangles(area1->leftB, area2->rightB,
                            leftBSpan, rightBSpan, leftRightB);

   // prepare reporting
   currentTask = TASK::report;
   reportType = 0;
   reportSubType = 0;
   indexSBegin = 0;
   indexS = 0;
   indexTBegin = 0;
   indexT = 0;
}


bool Merger::merge(const AppendToOutput* output) {
   static constexpr unsigned REPORT_TYPE_COUNT = 4;

   // outer loop of reportPairs() calls
   while (currentTask == TASK::report && reportType < REPORT_TYPE_COUNT) {
      bool done = false;
      if (reportType == 0) {
         done = leftASpan.empty() ||
                reportPairs(leftASpan, area2->leftB, area2->completeB, output);
      } else if (reportType == 1) {
         done = rightASpan.empty() ||
                reportPairs(rightASpan, area1->leftB, area1->completeB, output);
      } else if (reportType == 2) {
         done = leftBSpan.empty() ||
                reportPairs(leftBSpan, area2->leftA, area2->completeA, output);
      } else if (reportType == 3) {
         done = rightBSpan.empty() ||
                reportPairs(rightBSpan, area1->leftA, area1->completeA, output);
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

   assert (currentTask == TASK::done);
   return true;
}

MergedAreaPtr Merger::getResult() const {
   assert (currentTask == TASK::done);
   return result;
}

bool Merger::reportPairs(const JoinEdgeVec& span,
                         const JoinEdgeVec& left,
                         const JoinEdgeVec& complete,
                         const AppendToOutput* output) {
   static constexpr unsigned REPORT_SUB_TYPE_COUNT = 2;

   // inner loop of reportPairs() calls
   while (reportSubType < REPORT_SUB_TYPE_COUNT) {
      bool done = false;
      if (reportSubType == 0) {
         done = left.empty() || reportPairsSub(span, left, output);
      } else if (reportSubType == 1) {
         done = complete.empty() || reportPairsSub(span, complete, output);
      }

      if (!done)
         return false; // outTBlock full, continue at next merge() call

      // reportPairsSub() was completed for the given reportSubType,
      // proceed with next reportSubType
      ++reportSubType;
   }

#ifdef CDAC_SPATIAL_JOIN_METRICS
   ++reportPairsCount;
#endif

   // report type completed, reset reportSubType to 0 for next reportType
   reportSubType = 0;
   return true;
}

bool Merger::reportPairsSub(const JoinEdgeVec& edgesS,
                            const JoinEdgeVec& edgesT,
                            const AppendToOutput* output) {

   // get some values frequently used in the loop below; use specialized
   // functions if there is only one edge in one of the vectors
   const size_t sizeS = edgesS.size();
   const size_t sizeT = edgesT.size();

   // refer special cases to specialized functions
   if (sizeS == 1 && sizeT == 1)
      return reportPairsSub11(edgesS[0], edgesT[0], output);
   if (sizeS == 1)
      return reportPairsSub1(edgesS[0], edgesT, output);
   if (sizeT == 1)
      return reportPairsSub1(edgesT[0], edgesS, output);
   // otherwise, both sets contain multiple edges which will be treated below

   while (indexSBegin < sizeS && indexTBegin < sizeT) {
      // determine the current edgeS/TBegin
      const JoinEdge& edgeSBegin = edgesS[indexSBegin];
      const JoinEdge& edgeTBegin = edgesT[indexTBegin];

      if (edgeSBegin.yMin < edgeTBegin.yMin) {
         const double yMaxS = edgeSBegin.yMax;
         // speed up the most frequent case (for many large joins, 99%)
         if (edgeTBegin.yMin > yMaxS) {
#ifdef CDAC_SPATIAL_JOIN_METRICS
            addToLoopStats(0);
#endif
            ++indexSBegin;
            continue;
         }

         // find intersections of the y range of edgeSBegin and edges in
         // edgesT[] (starting from indexTBegin)
         if (indexT < indexTBegin)
            indexT = indexTBegin;
         while (indexT < sizeT && edgesT[indexT].yMin <= yMaxS) {
            // report pair of rectangles (represented by the two edges);
            // edges can be passed to the output function in any order
            const bool outTBlockFull = !(*output)(edgeSBegin, edgesT[indexT]);

            // increase indexT before a possible "return false", so this pair
            // will not be reported again
            ++indexT;

            if (outTBlockFull) {
               // outTBlock is full. Index positions remain stored in
               // this Merger instance for the next call (with a new outTBlock)
               return false;
            } // otherwise continue reporting
         }
#ifdef CDAC_SPATIAL_JOIN_METRICS
         addToLoopStats(indexT - indexTBegin);
#endif
         indexT = indexTBegin;

         // proceed to next edge in edgesS
         ++indexSBegin;

      } else { // edgeTBegin.yMin <= edgeSBegin.yMin
         const double yMaxT = edgeTBegin.yMax;
         // speed up the most frequent case (for many large joins, 99%)
         if (edgeSBegin.yMin > yMaxT) {
#ifdef CDAC_SPATIAL_JOIN_METRICS
            addToLoopStats(0);
#endif
            ++indexTBegin;
            continue;
         }

         // find intersections of the y range of edgeTBegin and edges in
         // edgesS[] (starting from indexSBegin)
         if (indexS < indexSBegin)
            indexS = indexSBegin;
         while (indexS < sizeS && edgesS[indexS].yMin <= yMaxT) {
            // report pair of rectangles (represented by the two edges);
            // edges can be passed to the output function in any order
            const bool outTBlockFull = !(*output)(edgesS[indexS], edgeTBegin);

            // increase indexS before a possible "return false", so this pair
            // will not be reported again
            ++indexS;

            if (outTBlockFull) {
               // outTBlock is full. Index positions remain stored in
               // this Merger instance for the next call (with a new outTBlock)
               return false;
            } // otherwise continue reporting
         }
#ifdef CDAC_SPATIAL_JOIN_METRICS
         addToLoopStats(indexS - indexSBegin);
#endif
         indexS = indexSBegin;

         // proceed to next edge in edgesT
         ++indexTBegin;
      }

   }

#ifdef CDAC_SPATIAL_JOIN_METRICS
   ++reportPairsSubCount;
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
                            const JoinEdgeVec& edgesT,
                            const AppendToOutput* output) {

   // get some values frequently used in the loop below
   const size_t sizeT = edgesT.size();

   // iterating over the edgesT[] entries,
   // first treat all cases in which edgeT.yMin <= edgeS.yMin
   const double yMinS = edgeS.yMin;
   while (indexTBegin < sizeT)  {
      const JoinEdge& edgeTBegin = edgesT[indexTBegin];
      if (edgeTBegin.yMin > yMinS)
         break;
      ++indexTBegin; // prepare next iteration in case of "return false"
      if (yMinS <= edgeTBegin.yMax) {
         if (!(*output)(edgeS, edgeTBegin))
            return false;
      }
   }
   // now, for all remaining edgeT, yMinS < edgeT.yMin

   // continue iterating while edgeT.yMin is in the interval (yMinS, yMaxS]
   const double yMaxS = edgeS.yMax;
   if (indexTBegin < sizeT) {
      if (indexT < indexTBegin)
         indexT = indexTBegin;
      while (indexT < sizeT) {
         const JoinEdge& edgeT = edgesT[indexT];
         if (edgeT.yMin > yMaxS)
            break;
         ++indexT; // prepare next iteration in case of "return false"
         if (!(*output)(edgeS, edgeT))
            return false;
      }
   }
   // now all possible intersections were reported

#ifdef CDAC_SPATIAL_JOIN_METRICS
   ++reportPairsSub1Count;
#endif

   // reset indices for next call (indexSBegin and indexS were not used here)
   indexTBegin = 0;
   indexT = 0;

   // report subtype completed
   return true;
}

bool Merger::reportPairsSub11(const JoinEdge& edgeS,
                              const JoinEdge& edgeT,
                              const AppendToOutput* output) {
   if (indexTBegin == 0) {
      if ((edgeS.yMax >= edgeT.yMin && edgeS.yMin <= edgeT.yMax)) {
         // report intersection
         if (!(*output)(edgeS, edgeT)) {
            indexTBegin = 1; // prevent multiple reporting
            return false;
         }
      }
   }

#ifdef CDAC_SPATIAL_JOIN_METRICS
   ++reportPairsSub11Count;
#endif

   // reset indices for next call
   indexTBegin = 0;

   // report subtype completed
   return true;
}

#ifdef CDAC_SPATIAL_JOIN_METRICS
void Merger::addToLoopStats(size_t cycleCount) {
   unsigned lbCycleCount = 0; // the binary logarithm of cycleCount
   while (cycleCount != 0) {
      ++lbCycleCount;
      cycleCount >>= 1;
   }
   assert (lbCycleCount < LOOP_STATS_COUNT);
   ++loopStats[lbCycleCount];
}
#endif

#ifdef CDAC_SPATIAL_JOIN_METRICS
void Merger::resetLoopStats() {
   reportPairsCount = 0;
   reportPairsSubCount = 0;
   reportPairsSub1Count = 0;
   reportPairsSub11Count = 0;
   for (unsigned i = 0; i < LOOP_STATS_COUNT; ++i)
      loopStats[i] = 0;
}
#endif

#ifdef CDAC_SPATIAL_JOIN_METRICS
void Merger::reportLoopStats(std::ostream& out) {
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
         cycleCount <<= 1;
      }
      cout << endl;
   }
}
#endif

void Merger::removeCompleteRectangles(
        const JoinEdgeVec& left1,
        const JoinEdgeVec& right2,
        JoinEdgeVec& leftSpan,
        JoinEdgeVec& rightSpan,
        JoinEdgeVec& leftRight) {

   // get some values frequently used in the loops below
   const size_t size1 = left1.size();
   const size_t size2 = right2.size();

   // initialize indices
   size_t index1 = 0;
   size_t index2 = 0;

   // copy entries from both vectors in sort order as given by operator<
   while (index1 < size1 && index2 < size2) {
      const JoinEdge& edge1 = left1[index1];
      const JoinEdge& edge2 = right2[index2];
      if (edge1 < edge2) {
         if (result->containsCounterpartOf(edge1)) {
            if (!isLastMerge) // in the last merge, leftRight is obsolete
               leftRight.push_back(edge1);
         } else {
            leftSpan.push_back(edge1);
         }
         ++index1;
      } else {
         if (result->containsCounterpartOf(edge2)) {
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
      if (result->containsCounterpartOf(edge1)) {
         if (!isLastMerge) // in the last merge, leftRight is obsolete
            leftRight.push_back(edge1);
      } else {
         leftSpan.push_back(edge1);
      }
      ++index1;
   }

   // copy the remaining entries from right2, if any
   while (index2 < size2) {
      const JoinEdge& edge2 = right2[index2];
      if (!result->containsCounterpartOf(edge2)) { // see explanation above
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
   assert (index1 <= size1 && index2 <= size2);

   // reserve space in dest vector
   dest.reserve(dest.size() + size1 - index1 + size2 - index2);

   // copy entries from both vectors in sort order as given by operator<
   while (index1 < size1 && index2 < size2) {
      if (source1[index1] < source2[index2])
         dest.push_back(source1[index1++]);
      else
         dest.push_back(source2[index2++]);
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
   while (index1 < size1 && index2 < size2 && index3 < size3) {
      if (source1[index1] < source2[index2]) {
         if (source1[index1] < source3[index3])
            dest.push_back(source1[index1++]);
         else
            dest.push_back(source3[index3++]);
      } else { // edge2 <= edge1
         if (source2[index2] < source3[index3])
            dest.push_back(source2[index2++]);
         else
            dest.push_back(source3[index3++]);
      }
   }

   // copy entries from the remaining two vectors
   if (index1 == size1)
      merge(source2, index2, source3, index3, dest);
   else if (index2 == size2)
      merge(source1, index1, source3, index3, dest);
   else
      merge(source1, index1, source2, index2, dest);
}
