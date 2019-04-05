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


1 JoinState class

*/

#include <limits>
#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
#include <ctime>
#endif

#include "JoinState.h"
#include "Utils.h"

#include "Algebras/CRel/SpatialAttrArray.h"

using namespace cdacspatialjoin;
using namespace std;


JoinState::JoinState(const bool countOnly_,
           InputStream* inputA, InputStream* inputB,
           const uint64_t outTBlockSize_, const unsigned joinStateId_,
           std::shared_ptr<Timer>& timer_) :
        ioData(countOnly_, inputA, inputB, outTBlockSize_),
        countOnly(countOnly_),
        tupleCounts { inputA->getCurrentTupleCount(),
                      inputB->getCurrentTupleCount() },
        timer(timer_) {

   // get the number of tuples stored in the current TBlocks of both streams
   const uint64_t tupleSum = tupleCounts[SET::A] + tupleCounts[SET::B];

#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
   // get the runtime of the previous(!) task, i.e. requestData
   // (not the current Task createJoinState, therefore no timer->stop() here)
   clock_t createJoinStateTime = timer->getLastTime();
   cout << endl << "JoinState " << joinStateId_ << " created: " << endl;

   // sum up both sets and give the time used for requesting the streams
   // timer->getListTime() will return the time for JoinTask::requestData
   const size_t sizeSum = inputA->getBlockCount() + inputB->getBlockCount();
   const size_t usedMemSum = inputA->getUsedMem() + inputB->getUsedMem();
   const string blockName = countOnly_ ?  "block " : "TBlock ";
   const string blocksName = countOnly_ ? "blocks" : "TBlocks";
   const string tuplesName = countOnly_ ? "rectangles" : "tuples";
   cout << "# input: " << setw(2) << formatInt(sizeSum) << " " << blocksName
        << " (" << setw(7) << formatInt(usedMemSum / 1024) << " KiB)"
        << " with " << setw(9) << formatInt(tupleSum) << " " << tuplesName
        << " requested in " << formatMillis(createJoinStateTime)
        << " (= " << formatMillis(createJoinStateTime / sizeSum)
        << " per block)" << endl;

   // details for each set
   static constexpr SET SETS[] { SET::A, SET::B };
   for (const SET set : SETS) {
      const InputStream* input = (set == SET::A) ? inputA : inputB;
      const size_t blockCount = input->getBlockCount();
      const size_t usedMem = input->getUsedMem();
      cout << "- set " << IOData::getSetName(set) << ": "
           << setw(2) << formatInt(blockCount)
           << " " << (blockCount == 1 ? blockName : blocksName)
           << " (" << setw(7) << formatInt(usedMem / 1024) << " KiB) with "
           << setw(9) << formatInt(input->getCurrentTupleCount())
           << " " << tuplesName << " (last entry id:"
           << setw(10) << formatInt(input->getPassTupleCount()) << ")"
           << " from stream " << IOData::getSetName(set) << " ";
      if (input->isFullyLoaded()) {
         cout << "(fully loaded); ";
      } else if (set == SET::A && !inputB->isFullyLoaded()) {
         // if neither stream fits completely into the main memory, only
         // inputA may be re-opened ("passed") several times
         cout << "(pass " << input->getOpenCount() << ", "
              << "chunk " << input->getChunkCount() << "); ";
      } else {
         cout << "(chunk " << input->getChunkCount() << "); ";
      }

      if (input->getCurrentTupleCount() > 0) {
         cout << "first entry ";
         if (input->dim == 2) {
            input->getRectangle2D(0, 0).Print(cout); // prints an endl
         } else {
            input->getRectangle3D(0, 0).Print(cout); // prints an endl
         }
      }
   }
#endif

   // pre-calculate the bounding box of the second set (the parameter "false"
   // ensures the remaining parameters are not changed)
   timer->start(JoinTask::createSortEdges);
   auto rectangleInfos = new vector<RectangleInfo>();
   auto sortEdges = new vector<SortEdge>();
   Rectangle<3> bboxB = ioData.calculateBboxAndEdges(SET::B, false,
           Rectangle<3>(false), sortEdges, rectangleInfos);

   // calculate the bounding box of the first set, simultaneously adding
   // the first set to sortEdges and rectangleInfos
   rectangleInfos->reserve(tupleSum);
   sortEdges->reserve(2 * tupleSum); // "2 *" for left and right edges
   const Rectangle<3> bboxA = ioData.calculateBboxAndEdges(SET::A, true, bboxB,
           sortEdges, rectangleInfos);

   // report bounding boxes
#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
   for (const SET set : SETS) {
      const InputStream* input = (set == SET::A) ? inputA : inputB;
      cout << "- bounding box of set " << IOData::getSetName(set) << ": ";
      const Rectangle<3>& bbox = (set == SET::A) ? bboxA : bboxB;
      if (input->dim == 2)
         bbox.Project2D(0, 1).Print(cout);
      else
         bbox.Print(cout);
      if (!bbox.IsDefined())
         cout << endl; // this is missing from Rectangle<dim>.Print
   }
#endif

   if (bboxA.Intersects(bboxB)) {
      // add the second set to sortEdges and rectangleInfos
      bboxB = ioData.calculateBboxAndEdges(SET::B, true, bboxA, sortEdges,
              rectangleInfos);
   } else {
      // the bounding boxes of both sets do not intersect, so no intersection
      // between any two rectangles from the two sets is possible
#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
      cout << "# bounding boxes do not intersect; JoinState is being discarded"
           << endl;
#endif
      timer->stop();
      delete sortEdges;
      delete rectangleInfos;
      return;
   }

   // report ignored rectangles and sortEdges
#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
   clock_t createSortEdgesTime = timer->stop();
   const long ignored = tupleSum - sortEdges->size() / 2;
   if (ignored > 0) {
      const double perc = ignored / static_cast<double>(tupleSum) * 100.0;
      cout << "- " << formatInt(ignored) << " rectangles (" << perc << "%) "
           << "were ignored for being outside the other set's bounding box"
           << endl;
   }
   cout << "# " << formatInt(sortEdges->size()) << " SortEdges and "
        << formatInt(rectangleInfos->size()) << " RectangleInfos created ";
   if (ignored > 0)
      cout << "from the remaining rectangles ";
   cout << "in " << formatMillis(createSortEdgesTime) << endl;
#endif

   timer->start(JoinTask::sortSortEdges);

   // std::qsort turns out to be 40-45% faster than std::sort although
   // L1 Data Cache misses are approx. 2.8 : 1.0 (qsort : sort)!
   std::qsort(&(*sortEdges)[0], sortEdges->size(), sizeof(SortEdge),
           SortEdge::compare);
   // previously, std::sort(sortEdges->begin(), sortEdges->end());

   // report sorting
#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
   clock_t sortSortEdgesTime = timer->stop();
   cout << "- " << formatInt(sortEdges->size()) << " SortEdges sorted in "
        << formatMillis(sortSortEdgesTime) << endl;
   /*
   unsigned count = 0;
   for (const SortEdge& edge : *sortEdges) {
      cout << edge.toString() << endl;
      ++count;
      if (count == 20)
         break;
   }
   */
#endif

   // create JoinEdges
   timer->start(JoinTask::createJoinEdges);
   EdgeIndex_t sortEdgeIndex = 0;
   for (SortEdge& sortEdge : *sortEdges) {
      RectangleInfo& rectInfo = (*rectangleInfos)[sortEdge.rectInfoIndex];
      if (sortEdge.isLeft)
         rectInfo.leftEdgeIndex = sortEdgeIndex;
      else
         rectInfo.rightEdgeIndex = sortEdgeIndex;
      ++sortEdgeIndex;
   }
   joinEdgesSize = sortEdges->size();
   joinEdges.reserve(joinEdgesSize);

   // count the number of MergedAreas to be created on the lowest level. Note
   // that an "atomic" MergedArea is not created from a single JoinEdge but
   // possibly from a sequence of JoinEdges that belong to the same set
   size_t atomsExpectedTotal_ = 0;
   bool isFirst = true;
   SET lastSet = SET::A; // any value
   // improve performance by using local variable for field "joinEdges"
   std::vector<JoinEdge>& joinEdges_ = joinEdges;
   for (const SortEdge& sortEdge : *sortEdges) {
      const RectangleInfo& rectInfo = (*rectangleInfos)[sortEdge.rectInfoIndex];
      joinEdges_.emplace_back(rectInfo.yMin, rectInfo.yMax, (sortEdge.isLeft ?
              rectInfo.rightEdgeIndex : rectInfo.leftEdgeIndex),
              sortEdge.isLeft, rectInfo.address);
      const SET set = IOData::getSet(rectInfo.address);
      if (isFirst || set != lastSet) {
         ++atomsExpectedTotal_;
         lastSet = set;
         isFirst = false;
      }
   }
   atomsExpectedTotal = atomsExpectedTotal_;

#ifdef CDAC_SPATIAL_JOIN_METRICS
   // the first three values are constant within this JoinState;
   // ioData and joinEdges will remain in memory during the lifetime of this
   // JoinState, while SortEdges and RectangleInfos will be deleted below
   usedInputDataMemory = ioData.getUsedMemory() + sizeof(JoinState);
   usedSortEdgeMemory = sortEdges->size() * sizeof(SortEdge)
                      + rectangleInfos->size() * sizeof(RectangleInfo);
   usedJoinEdgeMemory = joinEdges_.size() * sizeof(JoinEdge);

   // usedMergedAreaMemory and mergeJoinEdgeCount will change in the course of
   // action of this JoinState; we keep track of their maximum values
   usedMergedAreaMemory = 0;
   usedMergedAreaMemoryMax = 0;
   mergeJoinEdgeCount = 0;
   mergeJoinEdgeCountMax = 0;
#endif

   // sortEdges and rectangleInfos is now obsolete (but will anyway be
   // removed from memory when constructor terminates)
   delete sortEdges;
   delete rectangleInfos;

   // "divide" step of the divide and conquer algorithm:
   // to ensure the merge step is balanced (i.e. to avoid that a MergedArea
   // with 2^16 edges will be merged with the MergedArea containing the 1
   // remaining edge at the end), we determine the average number of atomic
   // MergedAreas that are expected at each "level" of the merge process
   // (e.g. if we have 20 atomic MergedAreas, we expect 10.0 at level 3,
   // 5.0 at level 2, and 2.5 at level 1 (rather than 8.0 / 4.0 / 2.0).
   // If the expected number is not yet met, MergedAreas are kept on a lower
   // level until they grow large enough. Actually, it is sufficient to
   // check level 1 which then automatically provides the required MergedAreas
   // sizes on higher levels.
   double expect = atomsExpectedTotal_;
   while (expect >= 4.0)
      expect /= 2.0;
   // thus, 2.0 <= expect < 4.0
   atomsExpectedStep = expect;
   // rather than comparing with ">= std::round(atomsExpectedNext)" we will
   // set atomsExpectedNext to be 0.5 less, so we can do without std::round
   atomsExpectedNext = expect - 0.5;

   // pre-assign enough elements to mergedAreas vector, so its size() need not
   // be checked later
   size_t shift = atomsExpectedTotal_;
   levelCount = 0;
   while (shift > 0) {
      mergedAreas[levelCount] = nullptr;
      ++levelCount;
      shift >>= 1U;
   }

   // report JoinEdges and expected merges
#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
   clock_t createJoinEdgesTime = timer->stop();
   cout << "- " << formatInt(joinEdges.size()) << " JoinEdges created in "
        << formatMillis(createJoinEdgesTime) << endl;
   cout << "# " << formatInt(atomsExpectedTotal_) << " 'atomic' "
        << "MergedAreas (with JoinEdges from the same set only) will be "
        << "created and merged on levels 0 to " << (levelCount - 1)
        << endl;


   /*
   unsigned count2 = 0;
   for (const JoinEdge& joinEdge : joinEdges) {
      cout << ioData.toString(joinEdge) << endl;
      ++count2;
      if (count2 == 20)
         break;
   }
   */
#endif

   initializeCompleted = clock();
   outTBlockCount = 0;
   joinCompleted = false;

   joinEdgeIndex = 0;
   atomsCreated = 0;
   mergeLevel = 0;
   merger = nullptr;

   timer->stop();
}

JoinState::~JoinState() {
   // assert (merger == nullptr);
   // for (MergedAreaPtr mergedArea : mergedAreas)
    //   assert (mergedArea == nullptr);
}

bool JoinState::nextTBlock(CRelAlgebra::TBlock* const outTBlock_) {
   if (joinEdges.empty()) // this may happen if the sets' bboxes
      return false;       // do not intersect
   if (joinCompleted)
      return false;

   timer->start(JoinTask::merge);

   ioData.setOutTBlock(outTBlock_);
   uint64_t outTupleCountAtStart = ioData.getOutTupleCount();

   // improve performance by using local variables for fields, allowing for
   // better compiler optimization
   const std::vector<JoinEdge>& joinEdges_ = joinEdges;
   const size_t joinEdgesSize_ = joinEdgesSize;
   const double atomsExpectedStep_ = atomsExpectedStep;
   EdgeIndex_t joinEdgeIndex_ = joinEdgeIndex;
   MergedAreaPtr* mergedAreas_ = mergedAreas;
   unsigned long atomsCreated_ = atomsCreated;
   double atomsExpectedNext_ = atomsExpectedNext;
   unsigned mergeLevel_ = mergeLevel;
   Merger* merger_ = merger;

   do {
      // if two MergesAreas are currently being merged, ...
      if (merger_) {
         // continue merging
         if (!merger_->merge())
            break; // outTBlock is full, merging will be continued later

         // merger has completed
#ifdef CDAC_SPATIAL_JOIN_METRICS
         usedMergedAreaMemoryMax = std::max(usedMergedAreaMemoryMax,
                 usedMergedAreaMemory + merger_->getUsedMemory());
         mergeJoinEdgeCountMax = std::max(mergeJoinEdgeCountMax,
                 mergeJoinEdgeCount + merger_->getJoinEdgeCount(true));
#endif
         MergedAreaPtr result = merger_->getResult();
         delete merger_;
         merger_ = nullptr;

         // increment mergeLevel - unless the result MergedArea has to wait
         // on level 0 in order to grow a bit larger before it can move up to
         // level 1 (this is done to keep MergedArea sizes balanced)
         if (mergeLevel_ == 0) {
            if (atomsCreated_ >= atomsExpectedNext_) {
               ++mergeLevel_;
               atomsExpectedNext_ += atomsExpectedStep_;
            } // otherwise, keep MergedArea on the same mergeLevel
         } else { // MergedAreas above mergeLevel 0 never have to wait
            ++mergeLevel_;
         }

         // mergedAreas vector was pre-assigned to the required size in the
         // constructor, so it can be used without checking its size
         if (mergedAreas_[mergeLevel_]) {
            // continue merging with the MergedArea at the next level
            merger_ = createMerger(mergeLevel_, result);
         } else {
            // enter result to a free entry to wait for a merge partner
#ifdef CDAC_SPATIAL_JOIN_METRICS
            usedMergedAreaMemory += result->getUsedMemory();
            mergeJoinEdgeCount += result->getJoinEdgeCount();
#endif
            mergedAreas_[mergeLevel_] = result;
            mergeLevel_ = 0;
         }

      } else if (joinEdgeIndex_ < joinEdgesSize_) {
         // no current merge action; create new MergedArea from next edge(s);
         // all edges must be from the same set
         mergeLevel_ = 0;
         const EdgeIndex_t indexStart = joinEdgeIndex_;
         const SetRowBlock_t set =
                 joinEdges_[joinEdgeIndex_].address & IOData::SET_MASK;
         do {
            ++joinEdgeIndex_;
         } while (joinEdgeIndex_ < joinEdgesSize_ && set ==
                 (joinEdges_[joinEdgeIndex_].address & IOData::SET_MASK));
         auto newArea = new MergedArea(joinEdges_, indexStart, joinEdgeIndex_,
                 IOData::getSet(set));
         ++atomsCreated_;
         if (mergedAreas_[0]) {
            // merge newArea with the MergedArea waiting at level 0
            merger_ = createMerger(0, newArea);
         } else  {
            // enter newArea to mergedAreas_[0] to merge it with the next atom
#ifdef CDAC_SPATIAL_JOIN_METRICS
            usedMergedAreaMemory += newArea->getUsedMemory();
            mergeJoinEdgeCount += newArea->getJoinEdgeCount();
#endif
            mergedAreas_[0] = newArea;
         }

      } else {
         // no current merge action, and no joinEdges left;
         // now merge lower level mergedAreas with higher level ones
         // assert (atomsCreated_ == atomsExpectedTotal);
         MergedAreaPtr mergedArea2 = nullptr;
         for (unsigned level = 0; level < levelCount; ++level) {
            if (!mergedAreas_[level])
               continue;
            if (!mergedArea2) {
               mergedArea2 = mergedAreas_[level];
#ifdef CDAC_SPATIAL_JOIN_METRICS
               // the memory used by mergedArea2 will be measured as part of
               // the Merger in merger_->getUsedMemory()
               usedMergedAreaMemory -= mergedArea2->getUsedMemory();
               mergeJoinEdgeCount -= mergedArea2->getJoinEdgeCount();
#endif
               mergedAreas_[level] = nullptr;
            } else {
               mergeLevel_ = level;
               merger_ = createMerger(level, mergedArea2);
               break;
            }
         }
         if (!merger_) {
            // join of the given data completed; outTBlock may contain
            // final result tuples
            delete mergedArea2; // the result of the last merge
            joinCompleted = true;
            break;
         } // otherwise, continue merging
      }
   } while (true);

   // write back local copies to fields
   joinEdgeIndex = joinEdgeIndex_;
   atomsCreated = atomsCreated_;
   atomsExpectedNext = atomsExpectedNext_;
   mergeLevel = mergeLevel_;
   merger = merger_;

   // update statistics
   size_t totalOutTupleCount = ioData.getOutTupleCount();
   uint64_t outTBlockTupleCount = totalOutTupleCount - outTupleCountAtStart;
   if (!countOnly && outTBlockTupleCount > 0) {
      // rowCount is ioData.outTBlock->GetRowCount();
      ++outTBlockCount;
   }

#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
   if (joinCompleted) {
      if (countOnly) {
         // count intersections only
         cout << "# " << formatInt(totalOutTupleCount)
              << " intersections counted ";
      } else {
         // actual result tuples were created
         cout << "# " << formatInt(outTBlockCount) << " "
              << (outTBlockCount == 1 ? "block" : "blocks") << " with "
              << formatInt(totalOutTupleCount) << " tuples returned ";
      }
      cout   << "in " << formatMillis(clock() - initializeCompleted)
         << " (intersection ratio "
         << totalOutTupleCount * 1000000.0 /
            (tupleCounts[SET::A] * tupleCounts[SET::B])
         << " per million)" << endl;
  }
#endif

   // reference to outTBlock is not needed any more
   ioData.setOutTBlock(nullptr);

   timer->stop();
   return (outTBlockTupleCount > 0);
}

/* creates a new Merger for the given areas, then deletes the areas */
Merger* JoinState::createMerger(unsigned levelOfArea1, MergedAreaPtr area2) {
   MergedAreaPtr area1 = mergedAreas[levelOfArea1];
#ifdef CDAC_SPATIAL_JOIN_METRICS
   // the memory used by area1 will be measured as part of the Merger in
   // merger_->getUsedMemory()
   usedMergedAreaMemory -= area1->getUsedMemory();
   mergeJoinEdgeCount -= area1->getJoinEdgeCount();
#endif
   mergedAreas[levelOfArea1] = nullptr;

   const bool isLastMerge = (area1->edgeIndexStart == 0 &&
                             area2->edgeIndexEnd == joinEdgesSize);

   // move ownership of source areas (area1 and area2) to new Merger;
   // source areas will be deleted in ~Merger()
   return new Merger(area1, area2, isLastMerge, &ioData);
}

#ifdef CDAC_SPATIAL_JOIN_METRICS
size_t JoinState::getTotalUsedMemoryMax() const {
   // two components are used during the whole lifetime of the JoinState:
   size_t usedWholeLifetime = usedInputDataMemory + usedJoinEdgeMemory;

   // usedSortEdgeMemory is only used during instantiation,
   // usedMergedAreaMemory[Max] is only used after instantiation, so the
   // maximum of those two must be used (usually, that is the latter):
   return usedWholeLifetime +
          std::max(usedSortEdgeMemory, usedMergedAreaMemoryMax);
}
#endif

