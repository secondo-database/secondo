/*
1 JoinState class

*/

#include <limits>
#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
#include <ctime>
#endif

#include "JoinState.h"
#include "Algebras/CRel/SpatialAttrArray.h"

using namespace cdacspatialjoin;
using namespace std;


JoinState::JoinState(const bool countOnly_,
           InputStream* inputA, InputStream* inputB,
           const uint64_t outTBlockSize_, const unsigned joinStateId_,
           std::shared_ptr<Timer>& timer_) :

        countOnly { countOnly_ },
        tBlocks { inputA->tBlocks, inputB->tBlocks },
        rBlocks { inputA->rBlocks, inputB->rBlocks },
        attrIndices { inputA->attrIndex, inputB->attrIndex },
        columnCounts { inputA->attrCount, inputB->attrCount },
        tupleCounts {inputA->getCurrentTupleCount(),
                     inputB->getCurrentTupleCount() },
        tupleSum { tupleCounts[SET::A] + tupleCounts[SET::B] },
        dims { inputA->dim, inputB->dim },
        minDim(std::min(inputA->dim, inputB->dim)),
        outTBlockSize(outTBlockSize_),
        joinStateId(joinStateId_),
        timer(timer_),
        rowShift { getRowShift(inputA->getBlockCount()),
                   getRowShift(inputB->getBlockCount()) },
        rowMask { getRowMask(rowShift[0]), getRowMask(rowShift[1]) },
        blockMask { getBlockMask(rowShift[0]), getBlockMask(rowShift[1]) },
        newTuple(new CRelAlgebra::AttrArrayEntry[
                columnCounts[SET::A] + columnCounts[SET::B]]) {

#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
   // get the runtime of the previous(!) task, i.e. requestData
   // (not the current Task createJoinState, therefore no timer->stop() here)
   clock_t createJoinStateTime = timer->getLastTime();
   cout << endl << "JoinState " << joinStateId << " created: " << endl;

   // sum up both sets and give the time used for requesting the streams
   // timer->getListTime() will return the time for CDSjTask::requestData
   const size_t sizeSum = inputA->getBlockCount() + inputB->getBlockCount();
   const size_t usedMemSum = inputA->getUsedMem() + inputB->getUsedMem();
   const string blockName = countOnly ?  "block " : "TBlock ";
   const string blocksName = countOnly ? "blocks" : "TBlocks";
   const string tuplesName = countOnly ? "rectangles" : "tuples";
   cout << "# input: " << setw(2) << formatInt(sizeSum) << " " << blocksName
        << " (" << setw(7) << formatInt(usedMemSum / 1024) << " KiB)"
        << " with " << setw(9) << formatInt(tupleSum) << " " << tuplesName
        << " requested in " << formatMillis(createJoinStateTime)
        << " (= " << formatMillis(createJoinStateTime / sizeSum)
        << " per block)" << endl;

   // details for each set
   for (const SET set : SETS) {
      const InputStream* input = (set == SET::A) ? inputA : inputB;
      const size_t blockCount = input->getBlockCount();
      const size_t usedMem = input->getUsedMem();
      cout << "- set " << SET_NAMES[set] << ": "
           << setw(2) << formatInt(blockCount)
           << " " << (blockCount == 1 ? blockName : blocksName)
           << " (" << setw(7) << formatInt(usedMem / 1024) << " KiB) with "
           << setw(9) << formatInt(tupleCounts[set]) << " " << tuplesName
           << " (last entry id:"
           << setw(10) << formatInt(input->getPassTupleCount()) << ")"
           << " from stream " << SET_NAMES[set] << " ";
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
         if (dims[set] == 2) {
            input->getRectangle2D(0, 0).Print(cout); // prints an endl
         } else {
            input->getRectangle3D(0, 0).Print(cout); // prints an endl
         }
      }
   }
#endif

   // pre-calculate the bounding box of the second set (the parameter "false"
   // ensures the remaining parameters are not changed)
   timer->start(CDSjTask::createSortEdges);
   unique_ptr<vector<RectangleInfo>> rectangleInfos(
           new vector<RectangleInfo>());
   unique_ptr<vector<SortEdge>> sortEdges(new vector<SortEdge>());
   Rectangle<3> bboxB = calculateBboxAndEdges(SET::B, false,
           Rectangle<3>(false), sortEdges, rectangleInfos);

   // calculate the bounding box of the first set, simultaneously adding
   // the first set to sortEdges and rectangleInfos
   rectangleInfos->reserve(tupleSum);
   sortEdges->reserve(2 * tupleSum); // "2 *" for left and right edges
   const Rectangle<3> bboxA = calculateBboxAndEdges(SET::A, true, bboxB,
           sortEdges, rectangleInfos);

   // report bounding boxes
#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
   for (const SET set : SETS) {
      cout << "- bounding box of set " << SET_NAMES[set] << ": ";
      const Rectangle<3>& bbox = (set == SET::A) ? bboxA : bboxB;
      if (dims[set] == 2)
         bbox.Project2D(0, 1).Print(cout);
      else
         bbox.Print(cout);
      if (!bbox.IsDefined())
         cout << endl; // this is missing from Rectangle<dim>.Print
   }
#endif

   if (bboxA.Intersects(bboxB)) {
      // add the second set to sortEdges and rectangleInfos
      bboxB = calculateBboxAndEdges(SET::B, true, bboxA, sortEdges,
              rectangleInfos);
   } else {
      // the bounding boxes of both sets do not intersect, so no intersection
      // between any two rectangles from the two sets is possible
      sortEdges = nullptr;
#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
      cout << "# bounding boxes do not intersect; JoinState is being discarded"
           << endl;
#endif
      timer->stop();
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

   timer->start(CDSjTask::sortSortEdges);

   // std::qsort turns out to be 40-45% faster than std::sort although
   // L1 Data Cache misses are approx. 2.8 : 1.0 (qsort : sort)!
   std::qsort(&(*sortEdges)[0], sortEdges->size(), sizeof(SortEdge),
           compareSortEdges);
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
   timer->start(CDSjTask::createJoinEdges);
   EdgeIndex_t sortEdgeIndex = 0;
   for (SortEdge& sortEdge : *sortEdges) {
      RectangleInfo& rectInfo = (*rectangleInfos)[sortEdge.rectInfoIndex];
      if (sortEdge.isLeft)
         rectInfo.leftEdgeIndex = sortEdgeIndex;
      else
         rectInfo.rightEdgeIndex = sortEdgeIndex;
      ++sortEdgeIndex;
   }
   joinEdges.reserve(2 * tupleSum);
   SET lastSet = (getSet((*rectangleInfos)[(*sortEdges)[0].rectInfoIndex]
           .address) == SET::A) ? SET::B : SET::A; // not the set of the first
   level0AreaCountExpected = 0;
   for (const SortEdge& sortEdge : *sortEdges) {
      const RectangleInfo& rectInfo = (*rectangleInfos)[sortEdge.rectInfoIndex];
      joinEdges.emplace_back(rectInfo.yMin, rectInfo.yMax,
              rectInfo.getEdgeIndex(!sortEdge.isLeft), sortEdge.isLeft,
              rectInfo.address);
      const SET set = getSet(rectInfo.address);
      if (set != lastSet) {
         ++level0AreaCountExpected;
         lastSet = set;
      }
   }
   // sortEdges and rectangleInfos is now obsolete
   sortEdges.reset(nullptr);
   rectangleInfos.reset(nullptr);

   // report JoinEdges
#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
   clock_t createJoinEdgesTime = timer->stop();
   cout << "- " << formatInt(joinEdges.size()) << " JoinEdges created in "
        << formatMillis(createJoinEdgesTime) << endl;
   cout << "# " << formatInt(level0AreaCountExpected) << " 'atomic' "
        << "MergedAreas (with JoinEdges from the same set only) will be "
        << "created on the lowest level." << endl;

   /*
   unsigned count2 = 0;
   for (const JoinEdge& joinEdge : joinEdges) {
      cout << joinEdge.toString() << endl;
      ++count2;
      if (count2 == 20)
         break;
   }
   */
#endif

   // set lastBlockA/B to a value that ensures the first "newTuple" to
   // be fully assembled in the appendToOutput() function
   lastAddressA = std::numeric_limits<SetRowBlock_t >::max();
   lastAddressB = std::numeric_limits<SetRowBlock_t >::max();

   initializeCompleted = clock();
   outTBlockCount = 0;
   outTupleCount = 0;
   joinCompleted = false;

   timer->stop();
}

unsigned JoinState::getRowShift(const size_t blockCount) {
   unsigned shift = 0;
   size_t blockId = blockCount - 1;
   while (blockId != 0) {
      ++shift;
      blockId >>= 1;
   }
   return shift;
}

SetRowBlock_t JoinState::getRowMask(const size_t rowShift) {
   return ~getBlockMask(rowShift) & ~SET_MASK;
}

SetRowBlock_t JoinState::getBlockMask(const size_t rowShift) {
   return static_cast<SetRowBlock_t>((1U << rowShift) - 1);
}

std::string JoinState::toString(const JoinEdge& joinEdge) const {
   stringstream st;
   st << "y = [" << joinEdge.yMin << "; " << joinEdge.yMax << "]; ";
   st << (joinEdge.getIsLeft() ? "left" : "right") << " edge ";
   st << "from set " << SET_NAMES[getSet(joinEdge.address)] << ", ";
   st << "block " << getBlockIndex(joinEdge.address) << ", ";
   st << "row " << getRowIndex(joinEdge.address);
   return st.str();
}

Rectangle<3> JoinState::calculateBboxAndEdges(const SET set,
        const bool addToEdges, const Rectangle<3>& otherBbox,
        unique_ptr<vector<SortEdge>>& sortEdges,
        unique_ptr<vector<RectangleInfo>>& rectangleInfos)
        const {

   // to optimize runtime, two distinct versions are being used
   // for the 2D and 3D cases:
   if (countOnly) {
      return calculateBboxAndEdgesCount(set, addToEdges, otherBbox, sortEdges,
                                        rectangleInfos);
   } else if (dims[set] == 2) {
      return calculateBboxAndEdges2D(set, addToEdges, otherBbox, sortEdges,
                                     rectangleInfos);
   } else if (dims[set] == 3) {
      return calculateBboxAndEdges3D(set, addToEdges, otherBbox, sortEdges,
                                     rectangleInfos);
   } else {
      assert (false);
   }
}

Rectangle<3> JoinState::calculateBboxAndEdges2D(const SET set,
        const bool addToEdges, const Rectangle<3>& otherBbox,
        unique_ptr<vector<SortEdge>>& sortEdges,
        unique_ptr<vector<RectangleInfo>>& rectangleInfos)
   const {

   assert (dims[set] == 2);
   assert (!addToEdges || otherBbox.IsDefined());

   // get some values that are used frequently in the loop below
   const unsigned attrIndex = attrIndices[set];
   auto rectInfoIndex = static_cast<RectInfoIndex_t>(rectangleInfos->size());

   // the bbox of the other set serves as a filter to the rectangles in this set
   const double filterXMin = otherBbox.MinD(0);
   const double filterXMax = otherBbox.MaxD(0);
   const double filterYMin = otherBbox.MinD(1);
   const double filterYMax = otherBbox.MaxD(1);

   // at the same time, calculate the bbox of this set
   double bboxXMin = numeric_limits<double>::max();
   double bboxXMax = -numeric_limits<double>::max();
   double bboxYMin = numeric_limits<double>::max();
   double bboxYMax = -numeric_limits<double>::max();

   // iterate over the TBlocks of this set
   uint16_t blockNum = 0;
   for (TBlockPtr block : *tBlocks[set]) {
      // iterate over the join attribute array of this tBlock
      auto attrArray = (const CRelAlgebra::SpatialAttrArray<2>*)
              (&block->GetAt(attrIndex));

      const auto count = static_cast<RowIndex_t>(attrArray->GetCount());
      for (RowIndex_t row = 0; row < count; ++row) {
         // get the 2-dimensional extent of the current tuple's GeoData
         const Rectangle<2>& rec = attrArray->GetBoundingBox(row);
         if (!rec.IsDefined())
            continue;

         // get rectangle position
         const double xMin = rec.MinD(0);
         const double xMax = rec.MaxD(0);
         const double yMin = rec.MinD(1);
         const double yMax = rec.MaxD(1);

         // extend the set's bbox
         if (xMin < bboxXMin)
            bboxXMin = xMin;
         if (xMax > bboxXMax)
            bboxXMax = xMax;
         if (yMin < bboxYMin)
            bboxYMin = yMin;
         if (yMax > bboxYMax)
            bboxYMax = yMax;

         if (!addToEdges)
            continue;

         // simulate otherBbox.Intersects(rec) for the x and y dimensions
         if ((xMax < filterXMin || filterXMax < xMin ||
              yMax < filterYMin || filterYMax < yMin)) {
            // rec is outside the bbox of the other set
            continue;
         }

         // add the rectangle information and its two edges to the
         // output vectors; otherwise, calculate the set's bounding
         // box only
         rectangleInfos->emplace_back(yMin, yMax,
                                     getAddress(set, row, blockNum));
         sortEdges->emplace_back(xMin, rectInfoIndex, true);
         sortEdges->emplace_back(xMax, rectInfoIndex, false);
         ++rectInfoIndex;
      }
      ++blockNum;
   }

   // return this set's bounding box
   // to keep things simple, a Rectangle<3> is returned in this case, too;
   // for the z-dimension, anything will be allowed (in case the other set
   // has three-dimensional information)
   double bboxZMin = -numeric_limits<double>::max();
   double bboxZMax = numeric_limits<double>::max();
   const double min[] { bboxXMin, bboxYMin, bboxZMin };
   const double max[] { bboxXMax, bboxYMax, bboxZMax };
   const bool defined = (bboxXMin < numeric_limits<double>::max());
   return Rectangle<3>(defined, min, max);
}

Rectangle<3> JoinState::calculateBboxAndEdges3D(const SET set,
        const bool addToEdges, const Rectangle<3>& otherBbox,
        unique_ptr<vector<SortEdge>>& sortEdges,
        unique_ptr<vector<RectangleInfo>>& rectangleInfos)
        const {

   assert (dims[set] == 3);
   assert (!addToEdges || otherBbox.IsDefined());

   // get some values that are used frequently in the loop below
   const unsigned attrIndex = attrIndices[set];
   auto rectInfoIndex = static_cast<RectInfoIndex_t>(rectangleInfos->size());

   // the bbox of the other set serves as a filter to the rectangles in this set
   const double filterXMin = otherBbox.MinD(0);
   const double filterXMax = otherBbox.MaxD(0);
   const double filterYMin = otherBbox.MinD(1);
   const double filterYMax = otherBbox.MaxD(1);
   const double filterZMin = otherBbox.MinD(2);
   const double filterZMax = otherBbox.MaxD(2);

   // at the same time, calculate the bbox of this set
   double bboxXMin = numeric_limits<double>::max();
   double bboxXMax = -numeric_limits<double>::max();
   double bboxYMin = numeric_limits<double>::max();
   double bboxYMax = -numeric_limits<double>::max();
   double bboxZMin = numeric_limits<double>::max();
   double bboxZMax = -numeric_limits<double>::max();

   // iterate over the TBlocks of this set
   uint16_t blockNum = 0;
   for (TBlockPtr block : *tBlocks[set]) {

      // iterate over the join attribute array of this tBlock
      auto attrArray = (const CRelAlgebra::SpatialAttrArray<3>*)
              (&block->GetAt(attrIndex));

      const auto count = static_cast<RowIndex_t>(attrArray->GetCount());
      for (RowIndex_t row = 0; row < count; ++row) {
         // get the 2-dimensional extent of the current tuple's GeoData
         const Rectangle<3>& rec = attrArray->GetBoundingBox(row);
         if (!rec.IsDefined())
            continue;

         // get rectangle position
         const double xMin = rec.MinD(0);
         const double xMax = rec.MaxD(0);
         const double yMin = rec.MinD(1);
         const double yMax = rec.MaxD(1);
         const double zMin = rec.MinD(2);
         const double zMax = rec.MaxD(2);

         // extend the set's bbox
         if (xMin < bboxXMin)
            bboxXMin = xMin;
         if (xMax > bboxXMax)
            bboxXMax = xMax;
         if (yMin < bboxYMin)
            bboxYMin = yMin;
         if (yMax > bboxYMax)
            bboxYMax = yMax;
         if (zMin < bboxZMin)
            bboxZMin = zMin;
         if (zMax > bboxZMax)
            bboxZMax = zMax;

         if (!addToEdges)
            continue;

         // simulate otherBbox.Intersects(rec)
         if ((xMax < filterXMin || filterXMax < xMin ||
              yMax < filterYMin || filterYMax < yMin ||
              zMax < filterZMin || filterZMax < zMin)) {
            // rec is outside the bbox of the other set
            continue;
         }

         // add the rectangle information and its two edges to the
         // output vectors
         rectangleInfos->emplace_back(yMin, yMax,
                                     getAddress(set, row, blockNum));
         sortEdges->emplace_back(xMin, rectInfoIndex, true);
         sortEdges->emplace_back(xMax, rectInfoIndex, false);
         ++rectInfoIndex;
      }
      ++blockNum;
   }

   // return this set's bounding box
   const double min[] { bboxXMin, bboxYMin, bboxZMin };
   const double max[] { bboxXMax, bboxYMax, bboxZMax };
   const bool defined = (bboxXMin < numeric_limits<double>::max());
   return Rectangle<3>(defined, min, max);
}

Rectangle<3> JoinState::calculateBboxAndEdgesCount(const SET set,
        const bool addToEdges, const Rectangle<3>& otherBbox,
        unique_ptr<vector<SortEdge>>& sortEdges,
        unique_ptr<vector<RectangleInfo>>& rectangleInfos) const {

   assert (countOnly);
   assert (!addToEdges || otherBbox.IsDefined());

   // get some values that are used frequently in the loop below
   const unsigned dim = dims[set];
   auto rectInfoIndex = static_cast<RectInfoIndex_t>(rectangleInfos->size());

   // the bbox of the other set serves as a filter to the rectangles in this set
   const double filterXMin = otherBbox.MinD(0);
   const double filterXMax = otherBbox.MaxD(0);
   const double filterYMin = otherBbox.MinD(1);
   const double filterYMax = otherBbox.MaxD(1);
   const double filterZMin = otherBbox.MinD(2);
   const double filterZMax = otherBbox.MaxD(2);

   // at the same time, calculate the bbox of this set
   Rectangle<3> bbox { false };

   // iterate over the TBlocks of this set
   uint16_t blockNum = 0;
   for (RectangleBlock* rBlock : *rBlocks[set]) {
      // extend the set's bounding box by the bounding box of this block
      // which was already calculated by InputStream
      if (!bbox.IsDefined())
         bbox = rBlock->getBbox();
      else
         bbox.Extend(rBlock->getBbox());

      if (!addToEdges)
         continue;

      const vector<PlainRect2>& coordsXY = rBlock->getCoordsXY();
      const vector<PlainInterval>& coordsZ = rBlock->getCoordsZ();
      const auto count = rBlock->getRectangleCount();
      for (RowIndex_t row = 0; row < count; ++row) {
         // get the rectangle's x and y coordinates
         const PlainRect2& rect2 = coordsXY[row];
         // simulate otherBbox.Intersects(rec) for x and y coordinates
         if ((rect2.xMax < filterXMin || filterXMax < rect2.xMin ||
              rect2.yMax < filterYMin || filterYMax < rect2.yMin)) {
            // rec is outside the bbox of the other set
            continue;
         }

         if (dim == 3) {
            // get the rectangle's z coordinate
            const PlainInterval& intervalZ = coordsZ[row];
            // simulate otherBbox.Intersects(rec) for z coordinate
            if (intervalZ.zMax < filterZMin || filterZMax < intervalZ.zMin)
               continue;
         }

         // add the rectangle information and its two edges to the
         // output vectors
         rectangleInfos->emplace_back(rect2.yMin, rect2.yMax,
                                      getAddress(set, row, blockNum));
         sortEdges->emplace_back(rect2.xMin, rectInfoIndex, true);
         sortEdges->emplace_back(rect2.xMax, rectInfoIndex, false);
         ++rectInfoIndex;
      }
      ++blockNum;
   }

   // return this set's bounding box
   return bbox;
}

JoinState::~JoinState() {
   delete[] newTuple;
}

bool JoinState::nextTBlock(CRelAlgebra::TBlock* const outTBlock) {
   if (joinEdges.empty()) // this may happen if the sets' bboxes
      return false;       // do not intersect
   if (joinCompleted)
      return false;

   AppendToOutput appendToOutputFunc;
   if (countOnly) {
      appendToOutputFunc = [this](const JoinEdge& entryA,
                                  const JoinEdge& entryB) {
#ifndef CDAC_SPATIAL_JOIN_DETAILED_REPORT_TO_CONSOLE
         // for this very common case, save millions of calls to countOutput:
         if (minDim == 2) {
            ++outTupleCount;
            return true;
         }
#endif
         countOutput(entryA, entryB);
         return true;
      };
   } else {
      appendToOutputFunc = [this, outTBlock] (const JoinEdge& entryA,
                                              const JoinEdge& entryB) {
         return appendToOutput(entryA, entryB, outTBlock);
      };
   }

   timer->start(CDSjTask::merge);
   do {
      // if two MergesAreas are currently being merged, ...
      if (merger) {
         // continue merging
         // the Merger needs to call a function of type AppendToOutput
         if (merger->merge(&appendToOutputFunc)) {
            // merger has completed
            MergedAreaPtr result = merger->getResult();
            enqueueMergedAreaOrCreateMerger(result, true);
         } else {
            // outTBlock is full
            break;
         }

      } else if (joinEdgeIndex < joinEdges.size()) {
         // no current merge action; create new MergedArea from next edge(s);
         // all edges must be from the same set
         mergeLevel = 0;
         const EdgeIndex_t indexStart = joinEdgeIndex;
         const SET set = getSet(joinEdges[joinEdgeIndex].address);
         const auto joinEdgesSize = static_cast<EdgeIndex_t>(joinEdges.size());
         do {
            ++joinEdgeIndex;
         } while (joinEdgeIndex < joinEdgesSize &&
               getSet(joinEdges[joinEdgeIndex].address) == set);
         MergedAreaPtr newArea = make_shared<MergedArea>(joinEdges, indexStart,
                 joinEdgeIndex, set);
         ++level0AreaCount;
         // enqueue the newArea to mergedAreas or create a merger to
         // merge it with the MergedArea in mergedAreas[0]
         enqueueMergedAreaOrCreateMerger(newArea, false);

      } else {
         // no current merge action, and no joinEdges left;
         // now merge lower level mergedAreas with higher level ones
         assert (level0AreaCount == level0AreaCountExpected);
         MergedAreaPtr mergedArea2;
         for (unsigned level = 0; level < mergedAreas.size(); ++level) {
            if (!mergedAreas[level])
               continue;
            if (!mergedArea2) {
               mergedArea2 = mergedAreas[level];
               mergedAreas[level] = nullptr;
            } else {
               mergeLevel = level;
               createMerger(mergedAreas[level], mergedArea2);
               break;
            }
         }
         if (merger) {
            // continue merging
         } else {
            // join of the given data completed; outTBlock may contain
            // final result tuples
            joinCompleted = true;
            break;
         }
      }
   } while (true);

   // update statistics
   uint64_t rowCount = 0;
   if (countOnly) {
      // count only; outTupleCount has already been incremented
      rowCount = outTupleCount;
   } else {
      // actual result tuples were created
      rowCount = outTBlock->GetRowCount();
      if (rowCount > 0) {
         ++outTBlockCount;
         outTupleCount += rowCount;
      }
   }

#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
   if (joinCompleted) {
      if (countOnly) {
         // count intersections only
         cout << "# " << formatInt(outTupleCount) << " intersections counted ";
      } else {
         // actual result tuples were created
         cout << "# " << formatInt(outTBlockCount) << " "
              << (outTBlockCount == 1 ? "block" : "blocks") << " with "
              << formatInt(outTupleCount) << " tuples returned ";
      }
      cout   << "in " << formatMillis(clock() - initializeCompleted)
         << " (intersection ratio "
         << outTupleCount * 1000000.0 /
            (tupleCounts[SET::A] * tupleCounts[SET::B])
         << " per million)" << endl;
  }
#endif

   timer->stop();
   return (rowCount > 0);
}

void JoinState::enqueueMergedAreaOrCreateMerger(MergedAreaPtr& newArea,
        const bool mayIncreaseMergeLevel) {

   if (mayIncreaseMergeLevel) {
      const unsigned long remainingLevel0Areas = level0AreaCountExpected
                                                 - level0AreaCount;
      // compare the remaining MergedAreas that are to be created on the
      // lowest level with the number of MergedAreas that are already
      // merged "inside" the "newArea" (which results from a merge)
      if (remainingLevel0Areas == 0 ||
          remainingLevel0Areas >= (1UL << (mergeLevel + 1))) {
         // normal case: increase mergeLevel to enter result at the next
         // level (or recursively merge it with the MergedArea stored
         // there)
         ++mergeLevel;
      } else {
         // special case at the end of the calculation: enter this
         // result at the *same* mergeLevel (rather than at the next)
         // to avoid unbalanced merges at the end (in which a very large
         // MergedArea would be merged with a very small one)
         assert (!mergedAreas[mergeLevel]);

#ifdef CDAC_SPATIAL_JOIN_DETAILED_REPORT_TO_CONSOLE
         cout << "- keeping MergedArea with "
              << formatInt(newArea->getEdgeCount()) << " edges "
              << "on level " << mergeLevel
              << " (" << formatInt(remainingLevel0Areas) << " MergedAreas "
              << "left on lowest level)" << endl;
#endif
      }
   }

   if (mergeLevel < mergedAreas.size() && mergedAreas[mergeLevel]) {
      // continue merging with the MergedArea at the next level
      createMerger(mergedAreas[mergeLevel], newArea);
   } else  {
      // enter result to new level / to free position in mergedAreas
      if (mergeLevel >= mergedAreas.size())
         mergedAreas.push_back(newArea);
      else // mergedAreas[mergeLevel] contains nullptr
         mergedAreas[mergeLevel] = newArea;
      merger.reset(nullptr);
      mergeLevel = 0;
   }
}

void JoinState::createMerger(MergedAreaPtr &area1,
        MergedAreaPtr &area2) {
   const bool isLastMerge = (joinEdgeIndex == joinEdges.size() &&
                             mergeLevel + 1 == mergedAreas.size());

#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
   if (isLastMerge) {
      cout << "- last merge: " << formatInt(area1->getEdgeCount()) << " + "
         << formatInt(area2->getEdgeCount()) << " edges "
         << "at level " << mergeLevel << endl;
   }
#endif

   merger.reset(new Merger(area1, area2, isLastMerge));
   mergedAreas[mergeLevel] = nullptr;
}

bool JoinState::appendToOutput(const JoinEdge& entryS, const JoinEdge& entryT,
                               CRelAlgebra::TBlock* const outTBlock) {

   const bool entrySIsSetA = (getSet(entryS.address) == SET::A);
   const JoinEdge& entryA = entrySIsSetA ? entryS : entryT;
   const JoinEdge& entryB = entrySIsSetA ? entryT : entryS;
   const SetRowBlock_t addressA = entryA.address;
   const SetRowBlock_t addressB = entryB.address;
   const BlockIndex_t blockA = getBlockIndex(addressA);
   const BlockIndex_t blockB = getBlockIndex(addressB);
   const RowIndex_t rowA = getRowIndex(addressA);
   const RowIndex_t rowB = getRowIndex(addressB);

   // get input tuples represented by the two given JoinEdges
   const CRelAlgebra::TBlock* tBlockA = (*tBlocks[SET::A])[blockA];
   const CRelAlgebra::TBlock* tBlockB = (*tBlocks[SET::B])[blockB];

   // if both tuples are 3-dimensional, only now the z dimension is
   // being tested. If the tuples' bounding boxes do not intersect in the
   // z dimension, the pair is rejected at this late stage
   if (minDim == 3) {
      const CRelAlgebra::SpatialAttrArrayEntry<3>& attrA =
              tBlockA->GetAt(attrIndices[SET::A]).GetAt(rowA);
      const CRelAlgebra::SpatialAttrArrayEntry<3>& attrB =
              tBlockB->GetAt(attrIndices[SET::B]).GetAt(rowB);
      const Rectangle<3>& bboxA = attrA.GetBoundingBox();
      const Rectangle<3>& bboxB = attrB.GetBoundingBox();
      if (bboxA.MaxD(2) < bboxB.MinD(2) || bboxB.MaxD(2) < bboxA.MinD(2))
         return true; // nothing was added to outTBlock, so result must be true
   }

#ifdef CDAC_SPATIAL_JOIN_DETAILED_REPORT_TO_CONSOLE
   cout << endl;
   for (const SET set : SETS) {
      const JoinEdge& entry = (set == SET::A) ? entryA : entryB;
      const RowIndex_t row = (set == SET::A) ? rowA : rowB;
      cout << (entry.getIsLeft() ? "L" : "R") << SET_NAMES[set] << ": ";

      const unsigned attrIndex = attrIndices[set];
      const CRelAlgebra::TBlock* tBlock = (set == SET::A) ? tBlockA
                                                          : tBlockB;
      if (dims[set] == 2) {
         const CRelAlgebra::SpatialAttrArrayEntry<2>& attr =
                 tBlock->GetAt(attrIndex).GetAt(row);
         attr.GetBoundingBox().Print(cout); // prints an endl
      } else {
         const CRelAlgebra::SpatialAttrArrayEntry<3>& attr =
                 tBlock->GetAt(attrIndex).GetAt(row);
         attr.GetBoundingBox().Print(cout); // prints an endl
      }
   }
#endif

   // copy attributes from tupleA and tupleB into the newTuple; if one of
   // these tuples was already used when this function was last called,
   // the corresponding newTuple[] entries can be reused
   const size_t columnCountA = columnCounts[SET::A];
   if (addressA != lastAddressA) {
      const CRelAlgebra::TBlockEntry& tupleA =
              CRelAlgebra::TBlockEntry(tBlockA, rowA);
      for (uint64_t col = 0; col < columnCountA; ++col) {
         newTuple[col] = tupleA[col];
      }
      lastAddressA = addressA;
   }
   if (addressB != lastAddressB) {
      const size_t columnCountB = columnCounts[SET::B];
      const CRelAlgebra::TBlockEntry& tupleB =
              CRelAlgebra::TBlockEntry(tBlockB, rowB);
      for (uint64_t col = 0; col < columnCountB; ++col) {
         newTuple[columnCountA + col] = tupleB[col];
      }
      lastAddressB = addressB;
   }

   outTBlock->Append(newTuple);

   return (outTBlock->GetSize() < this->outTBlockSize);
}

void JoinState::countOutput(const JoinEdge& entryS, const JoinEdge& entryT) {
#ifndef CDAC_SPATIAL_JOIN_DETAILED_REPORT_TO_CONSOLE
   if (minDim == 2) {
      ++outTupleCount;
      return;
   }
#endif

   const bool entrySIsSetA = (getSet(entryS.address) == SET::A);
   const JoinEdge& entryA = entrySIsSetA ? entryS : entryT;
   const JoinEdge& entryB = entrySIsSetA ? entryT : entryS;
   const SetRowBlock_t addressA = entryA.address;
   const SetRowBlock_t addressB = entryB.address;
   const BlockIndex_t blockA = getBlockIndex(addressA);
   const BlockIndex_t blockB = getBlockIndex(addressB);
   const RowIndex_t rowA = getRowIndex(addressA);
   const RowIndex_t rowB = getRowIndex(addressB);

   // get input tuples represented by the two given JoinEdges
   const RectangleBlock* rBlockA = (*rBlocks[SET::A])[blockA];
   const RectangleBlock* rBlockB = (*rBlocks[SET::B])[blockB];

   // if both tuples are 3-dimensional, only now the z dimension is
   // being tested. If the tuples' bounding boxes do not intersect in the
   // z dimension, the pair is rejected at this late stage
   if (minDim == 3) {
      const PlainInterval& intervalA = rBlockA->getCoordsZ()[rowA];
      const PlainInterval& intervalB = rBlockB->getCoordsZ()[rowB];
      if (intervalA.zMax < intervalB.zMin ||
          intervalB.zMax < intervalA.zMin)
         return;
   }

#ifdef CDAC_SPATIAL_JOIN_DETAILED_REPORT_TO_CONSOLE
   cout << endl;
   for (const SET set : SETS) {
      const JoinEdge& entry = (set == SET::A) ? entryA : entryB;
      const RowIndex_t row = (set == SET::A) ? rowA : rowB;
      cout << (entry.getIsLeft() ? "L" : "R") << SET_NAMES[set] << ": ";
      const RectangleBlock* rBlock = (set == SET::A) ? rBlockA : rBlockB;
      if (dims[set] == 2) {
         rBlock->getRectangle2D(row).Print(cout); // prints an endl
      } else {
         rBlock->getRectangle3D(row).Print(cout); // prints an endl
      }
   }
#endif

   ++outTupleCount;
}