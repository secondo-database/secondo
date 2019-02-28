/*
1 Merger

*/

#include <assert.h>
#include <iostream>

#include "Merger.h"

using namespace cdacspatialjoin;
using namespace std;

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
   resetReportIndices();
}


bool Merger::merge(AppendToOutput* output) {
   // outer loop of reportPairs() calls
   while (currentTask == TASK::report && reportType < REPORT_TYPE_COUNT) {
      bool done = false;
      if (reportType == 0)
         done = reportPairs(leftASpan, area2->leftB, area2->completeB, output);
      else if (reportType == 1)
         done = reportPairs(rightASpan, area1->leftB, area1->completeB, output);
      else if (reportType == 2)
         done = reportPairs(leftBSpan, area2->leftA, area2->completeA, output);
      else if (reportType == 3)
         done = reportPairs(rightBSpan, area1->leftA, area1->completeA, output);

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

void Merger::resetReportIndices() {
   indexR = 0;
   indexSBegin = 0;
   indexS = 0;
}

bool Merger::reportPairs(const std::vector<JoinEdge>& span,
                         const std::vector<JoinEdge>& left,
                         const std::vector<JoinEdge>& complete,
                         AppendToOutput* output) {
   // inner loop of reportPairs() calls
   while (reportSubType < REPORT_SUB_TYPE_COUNT) {
      bool done = false;
      // TODO: SubType 0 und 1 zusammen abarbeiten, ebenso SubType 2 und 3!
      if (reportSubType == 0)
         done = reportPairsSub(span, left, true, output);
      else if (reportSubType == 1)
         done = reportPairsSub(left, span, false, output);
      else if (reportSubType == 2)
         done = reportPairsSub(span, complete, true, output);
      else if (reportSubType == 3)
         done = reportPairsSub(complete, span, false, output);

      if (!done)
         return false; // outTBlock full, continue at next merge() call

      // reportPairsSub() was completed for the given reportSubType,
      // proceed with next reportSubType
      ++reportSubType;
      resetReportIndices();
   }
   // report type completed, reset reportSubType to 0 for next reportType
   reportSubType = 0;
   return true;
}

bool Merger::reportPairsSub(const std::vector<JoinEdge>& edgesR,
                         const std::vector<JoinEdge>& edgesS,
                         const bool reportEqualYMinValues,
                         AppendToOutput* output) {

   // get some values frequently used in the loop below
   size_t sizeR = edgesR.size();
   size_t sizeS = edgesS.size();

   while (indexR < sizeR && indexSBegin < sizeS) {
      // determine the current edgeR and its y range
      const JoinEdge& edgeR = edgesR[indexR];
      double yMinR = edgeR.yMin;
      double yMaxR = edgeR.yMax;

      // determine the corresponding indexSBegin position. We need to ensure
      // that edges with the same yMin value are not reported twice;
      // therefore, for a given pair of vectors edgesR and edgesS,
      // reportPairsSub() is once called with reportEqualYMinValues == true,
      // and then called with swapped vectors and reportEqualYMinValues false.
      if (reportEqualYMinValues) {
         // compare yMin value using "<" (twice)
         if (edgesS[indexSBegin].yMin < yMinR) {
            do {
               ++indexSBegin;
            } while (indexSBegin < sizeS && edgesS[indexSBegin].yMin < yMinR);
            indexS = indexSBegin;
         }
      } else {
         // compare yMin values using  "<=" (twice)
         if (edgesS[indexSBegin].yMin <= yMinR) {
            do {
               ++indexSBegin;
            } while (indexSBegin < sizeS && edgesS[indexSBegin].yMin <= yMinR);
            indexS = indexSBegin;
         }
      }
      // TODO: binäre Suche verwenden, wenn sizeS sehr groß gegen sizeR ist?

      // find intersections of edges in edgesS[] and the y range of edgeR
      while (indexS < sizeS && edgesS[indexS].yMin <= yMaxR) {
         // report pair of rectangles (represented by the two edges);
         // output expects the rectangle from set A first:
         bool outTBlockFull;
         if (edgeR.set == SET::A)
            outTBlockFull = !(*output)(edgeR, edgesS[indexS]);
         else
            outTBlockFull = !(*output)(edgesS[indexS], edgeR);

         // increase indexS before a possible "return false", so this pair
         // will not be reported again
         ++indexS;

         if (outTBlockFull) {
            // outTBlock is full. Index positions remain stored in
            // this Merger instance for the next call (with a new outTBlock)
            return false;
         } // otherwise continue reporting
      }
      indexS = indexSBegin;

      // proceed to next edge in edgesR
      ++indexR;
   }
   // report subtype completed
   return true;
}

void Merger::removeCompleteRectangles(
        const std::vector<JoinEdge>& left1,
        const std::vector<JoinEdge>& right2,
        std::vector<JoinEdge>& leftSpan,
        std::vector<JoinEdge>& rightSpan,
        std::vector<JoinEdge>& leftRight) {

   // get some values frequently used in the loops below
   size_t size1 = left1.size();
   size_t size2 = right2.size();

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
        const std::vector<JoinEdge>& source1, const size_t startIndex1,
        const std::vector<JoinEdge>& source2, const size_t startIndex2,
        std::vector<JoinEdge>& dest) {

   // get some values frequently used in the loops below
   size_t size1 = source1.size();
   size_t size2 = source2.size();

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

void Merger::merge(const std::vector<JoinEdge>& source1,
                          const std::vector<JoinEdge>& source2,
                          const std::vector<JoinEdge>& source3,
                          std::vector<JoinEdge>& dest) {

   // get some values frequently used in the loops below
   size_t size1 = source1.size();
   size_t size2 = source2.size();
   size_t size3 = source3.size();

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
