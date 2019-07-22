/*
1 JoinEdge struct

*/

#include "JoinEdge.h"

using namespace cdacspatialjoin;
using namespace std;

void JoinEdge::removeCompleteRectangles(
        const JoinEdgeVec& left1, const JoinEdgeVec& right2,
        JoinEdgeVec& leftSpan, JoinEdgeVec& rightSpan,
        JoinEdgeVec& leftRight, const bool isNotLastMerge,
        const EdgeIndex_t resultEdgeStart, const EdgeIndex_t resultEdgeEnd) {

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

void JoinEdge::merge(
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

void JoinEdge::merge(const JoinEdgeVec& source1,
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
