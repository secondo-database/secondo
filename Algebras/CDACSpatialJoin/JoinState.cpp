/*
1 State

*/

#include <limits>
#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
#include <ctime>
#endif

#include "JoinState.h"
#include "Algebras/CRel/SpatialAttrArray.h"

using namespace cdacspatialjoin;
using namespace std;

JoinState::JoinState(
        std::shared_ptr<std::vector<CRelAlgebra::TBlock*>> tBlocks1,
        std::shared_ptr<std::vector<CRelAlgebra::TBlock*>> tBlocks2,
        unsigned attrIndexA,
        unsigned attrIndexB,
        uint64_t tupleCountA,
        uint64_t tupleCountB,
        unsigned dimA,
        unsigned dimB,
        uint64_t outTBlockSize_,
        unsigned joinStateId_) :

        tBlocks { std::move<>(tBlocks1), std::move<>(tBlocks2) },
        attrIndices { attrIndexA, attrIndexB },
        columnCounts { tBlocks[SET::A].get()->at(0)->GetColumnCount(),
                      tBlocks[SET::B].get()->at(0)->GetColumnCount() },
        tupleCounts { tupleCountA, tupleCountB },
        tupleSum { tupleCountA + tupleCountB },
        dims { dimA, dimB },
        minDim(std::min(dimA, dimB)),
        outTBlockSize(outTBlockSize_),
        joinStateId(joinStateId_),
        newTuple(new CRelAlgebra::AttrArrayEntry[
                columnCounts[SET::A] + columnCounts[SET::B]]) {

#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
   cout << endl << "JoinState " << joinStateId << " created: " << endl;
   for (SET set : SETS) {
      size_t size = tBlocks[set]->size();
      cout << "* set " << SET_NAMES[set] << ": "
           << formatInt(size) << (size == 1 ? " block " : " blocks ")
           << "with " << formatInt(tupleCounts[set]) << " tuples "
           << "from stream " << SET_NAMES[set] << "; ";
      auto iter = tBlocks[set]->at(0)->GetIterator();
      if (iter.IsValid()) {
         cout << "bbox of first entry is ";
         const CRelAlgebra::TBlockEntry &tuple = iter.Get();
         const unsigned attrIndex = attrIndices[set];
         if (dims[set] == 2) {
            CRelAlgebra::SpatialAttrArrayEntry<2> attr = tuple[attrIndex];
            attr.GetBoundingBox().Print(cout); // prints an endl
         } else {
            CRelAlgebra::SpatialAttrArrayEntry<3> attr = tuple[attrIndex];
            attr.GetBoundingBox().Print(cout); // prints an endl
         }
      } else {
         cout << "** block is empty **" << endl;
      }
   }
#endif

   // pre-calculate the bounding box of the second set (the parameter "false"
   // ensures the remaining parameters are not changed)
   vector<RectangleInfo> rectangleInfos;
   vector<SortEdge> sortEdges;
   Rectangle<3> bboxB = calculateBboxAndEdges(SET::B, false,
           Rectangle<3>(false), sortEdges, rectangleInfos);

   // calculate the bounding box of the first set, simultaneously adding
   // the first set to sortEdges and rectangleInfos
   rectangleInfos.reserve(tupleSum);
   sortEdges.reserve(2 * tupleSum); // "2 *" for left and right edges
   Rectangle<3> bboxA = calculateBboxAndEdges(SET::A, true, bboxB, sortEdges,
           rectangleInfos);
   if (bboxA.Intersects(bboxB)) {
      // add the second set to sortEdges and rectangleInfos
      bboxB = calculateBboxAndEdges(SET::B, true, bboxA, sortEdges,
              rectangleInfos);
   } else {
      // the bounding boxes of both sets do not intersect, so no intersection
      // between rectangles is possible
      sortEdges.clear();
      return; // TODO: testen
   }

   // report bounding boxes and sortEdges
#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
   for (SET set : SETS) {
      cout << "* bbox of set " << SET_NAMES[set] << ": ";
      Rectangle<3>& bbox = (set == SET::A) ? bboxA : bboxB;
      if (dims[set] == 2)
         bbox.Project2D(0, 1).Print(cout);
      else
         bbox.Print(cout);
      if (!bbox.IsDefined())
         cout << endl; // this is missing from Rectangle<dim>.Print
   }
   long ignored = tupleSum - sortEdges.size() / 2;
   if (ignored > 0) {
      double perc = round(ignored / static_cast<double>(tupleSum) * 100.0);
      cout << "* " << formatInt(ignored) << " rectangles (" << perc << "%) "
           << "were ignored for being outside the other set's bounding box"
           << endl;
   }
   cout << "* " << formatInt(sortEdges.size()) << " edges created from ";
   if (ignored > 0)
      cout << "the remaining ";
   cout << formatInt(sortEdges.size() / 2) << " rectangles" << endl;
   clock_t start = clock();
#endif

   std::sort(sortEdges.begin(), sortEdges.end()); // cp. Edge::operator<

   // report sorting
#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
   cout << "* " << formatInt(sortEdges.size()) << " SortEdges sorted in "
        << formatMillis(clock() - start) << endl;
   /*
   unsigned count = 0;
   for (const SortEdge& edge : sortEdges) {
      cout << edge.toString() << endl;
      ++count;
      if (count == 20)
         break;
   }
   */
   start = clock();
#endif

   // create JoinEdges
   EdgeIndex_t sortEdgeIndex = 0;
   for (SortEdge& sortEdge : sortEdges) {
      if (sortEdge.isLeft)
         rectangleInfos[sortEdge.rectInfoIndex].leftEdgeIndex = sortEdgeIndex;
      else
         rectangleInfos[sortEdge.rectInfoIndex].rightEdgeIndex = sortEdgeIndex;
      ++sortEdgeIndex;
   }
   joinEdges.reserve(2 * tupleSum);
   for (SortEdge& sortEdge : sortEdges) {
      const RectangleInfo& rectInfo = rectangleInfos[sortEdge.rectInfoIndex];
      joinEdges.emplace_back(sortEdge.yMin, rectInfo.yMax,
              rectInfo.getEdgeIndex(!sortEdge.isLeft), sortEdge.isLeft,
              rectInfo.set, rectInfo.block, rectInfo.row);
   }
   // sortEdges and rectangleInfos is now obsolete

   // report JoinEdges
#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
   cout << "* " << formatInt(joinEdges.size()) << " JoinEdges created in "
        << formatMillis(clock() - start) << endl;
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

   initializeCompleted = clock();
   tBlockCount = 0;
   outTupleCount = 0;
}

Rectangle<3> JoinState::calculateBboxAndEdges(const SET set,
        const bool addToEdges, const Rectangle<3>& otherBbox,
        vector<SortEdge>& sortEdges, vector<RectangleInfo>& rectangleInfos) {

   // get some values that are used frequently in the loop below
   unsigned attrIndex = attrIndices[set];
   unsigned dim = dims[set];

   // a Rectangle<3> will be returned; however, for dim == 2,
   // it is easier to use a Rectangle<2> in the loop and convert it later
   Rectangle<2> bbox2 { false };
   Rectangle<3> bbox3 { false };

   // iterate over the TBlocks of this set
   uint16_t blockNum = 0;
   for (TBlockPtr block : *tBlocks[set]) {
      // iterate over the rows (tuples) of this tBlock
      auto iter = block->GetIterator();
      uint32_t row = 0;
      while (iter.IsValid()) {
         const CRelAlgebra::TBlockEntry& tuple = iter.Get();

         // get the 2-dimensional extent of this tuple's GeoData
         double xMin, xMax, yMin, yMax;
         bool skip = false;
         if (dim == 2) {
            CRelAlgebra::SpatialAttrArrayEntry<2> attr = tuple[attrIndex];
            Rectangle<2> rec = attr.GetBoundingBox();
            if (!rec.IsDefined()) {
               skip = true; // omit this rectangle
            } else {
               xMin = rec.MinD(0);
               xMax = rec.MaxD(0);
               yMin = rec.MinD(1);
               yMax = rec.MaxD(1);
               // since we cannot call Rectangle<3>.Intersects(Rectangle<2>),
               // we will simulate it for the x and y dimensions:
               if (addToEdges && otherBbox.IsDefined() &&
                   (xMax < otherBbox.MinD(0) || otherBbox.MaxD(0) < xMin ||
                    yMax < otherBbox.MinD(1) || otherBbox.MaxD(1) < yMin)) {
                  skip = true; // rec is outside the bbox of the other set
               } else {
                  if (bbox2.IsDefined())
                     bbox2.Extend(rec);
                  else // rec is the first rectangle
                     bbox2 = rec;
               }
            }

         } else { // dim == 3
            CRelAlgebra::SpatialAttrArrayEntry<3> attr = tuple[attrIndex];
            Rectangle<3> rec = attr.GetBoundingBox();
            if (!rec.IsDefined()) {
               skip = true; // omit this rectangle
            } else if (addToEdges && !otherBbox.Intersects(rec)) {
               skip = true; // rec is outside the bbox of the other set
            } else {
               xMin = rec.MinD(0);
               xMax = rec.MaxD(0);
               yMin = rec.MinD(1);
               yMax = rec.MaxD(1);
               if (bbox3.IsDefined())
                  bbox3.Extend(rec);
               else // rec is the first rectangle
                  bbox3 = rec;
            }
         } // end of "if (dim == ...)"

         // if addToEdges == true, add the rectangle information and its two
         // edges to the output vectors; otherwise, calculate the set's
         // bounding box only
         if (addToEdges && !skip) {
            auto rectInfoIndex =
                    static_cast<RectInfoIndex_t>(rectangleInfos.size());
            rectangleInfos.emplace_back(yMax, set, blockNum, row);
            sortEdges.emplace_back(xMin, yMin, rectInfoIndex, true);
            sortEdges.emplace_back(xMax, yMin, rectInfoIndex, false);
         }
         ++row;
         iter.MoveToNext();
      }
      ++blockNum;
   }

   // return this set's bounding box
   if (dim == 2) {
      // to keep things simple, a Rectangle<3> is returned in this case, too;
      // for the z-dimension, anything will be allowed (in case the other set
      // has three-dimensional information)
      double min[] { bbox2.MinD(0), bbox2.MinD(1),
                     -std::numeric_limits<double>::max() };
      double max[] { bbox2.MaxD(0), bbox2.MaxD(1),
                     std::numeric_limits<double>::max() };
      return Rectangle<3>(true, min, max);
   } else {
      assert (dim == 3);
      return bbox3;
   }
}

JoinState::~JoinState() {
   delete[] newTuple;
}

bool JoinState::nextTBlock(CRelAlgebra::TBlock* outTBlock) {
   if (joinEdges.empty()) // this may happen if the sets' bboxes
      return false;       // do not intersect

   do {
      // if two MergesAreas are currently being merged, ...
      if (merger) {
         // continue merging
         // the Merger needs to call a function of type AppendToOutput
         AppendToOutput appendToOutputFunc = [this, outTBlock]
                 (const JoinEdge& entryA, const JoinEdge& entryB) {
            return appendToOutput(entryA, entryB, outTBlock);
         };
         if (merger->merge(&appendToOutputFunc)) {
            // merger has completed
            MergedAreaPtr result = merger->getResult();
            ++mergeLevel;
            enqueueMergedAreaOrCreateMerger(result);
         } else {
            // outTBlock is full
            break;
         }

      } else if (joinEdgeIndex < joinEdges.size()) {
         // no current merge action; create new MergedArea from next edge(s);
         // all edges must be from the same set
         mergeLevel = 0;
         EdgeIndex_t indexStart = joinEdgeIndex;
         SET set = joinEdges[joinEdgeIndex].set;
         do {
            ++joinEdgeIndex;
         } while (joinEdgeIndex < joinEdges.size() &&
               joinEdges[joinEdgeIndex].set == set);
         MergedAreaPtr newArea = make_shared<MergedArea>(joinEdges, indexStart,
                 joinEdgeIndex, set);
         // enqueue the newArea to mergedAreas or create a merger to
         // merge it with the MergedArea in mergedAreas[0]
         enqueueMergedAreaOrCreateMerger(newArea);

      } else {
         // no current merge action, and no joinEdges left;
         // now merge lower level mergedAreas with higher level ones
         MergedAreaPtr mergedArea2;
         for (unsigned level = 0; level < mergedAreas.size(); ++level) {
            if (!mergedAreas[level])
               continue;
            if (!mergedArea2) {
               mergedArea2 = mergedAreas[level];
               mergedAreas[level] = nullptr;
            } else {
               merger.reset(new Merger(mergedAreas[level], mergedArea2));
               mergedAreas[level] = nullptr;
               mergeLevel = level;
               break;
            }
         }
         if (merger) {
            // continue merging
         } else {
            // join of the given data completed; outTBlock may contain
            // final result tuples
            break;
         }
      }
   } while (true);

   // update statistics
   uint64_t rowCount = outTBlock->GetRowCount();
   if (rowCount > 0) {
      ++tBlockCount;
      outTupleCount += rowCount;
   } else {
#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
      cout << "* " << formatInt(tBlockCount) << " "
         << (tBlockCount == 1 ? "block" : "blocks") << " with "
         << formatInt(outTupleCount) << " tuples returned in "
         << formatMillis(clock() - initializeCompleted)
         << " (intersection ratio "
         << outTupleCount * 100.0 / (tupleCounts[SET::A] * tupleCounts[SET::B])
         << "%)" << endl;
#endif
   }

   return (rowCount > 0);
}

void JoinState::enqueueMergedAreaOrCreateMerger(MergedAreaPtr& newArea) {
   if (mergeLevel < mergedAreas.size() && mergedAreas[mergeLevel]) {
      // continue merging with the MergedArea at the next level
      merger.reset(new Merger(mergedAreas[mergeLevel], newArea));
      mergedAreas[mergeLevel] = nullptr;
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

bool JoinState::appendToOutput(const JoinEdge& entryA, const JoinEdge& entryB,
                               CRelAlgebra::TBlock* outTBlock) {

   // get input tuples represented by the two given JoinEdges
   const CRelAlgebra::TBlock* tBlockA = tBlocks[SET::A]->at(entryA.block);
   const CRelAlgebra::TBlockEntry& tupleA =
           CRelAlgebra::TBlockEntry(tBlockA, entryA.row);
   const CRelAlgebra::TBlock* tBlockB = tBlocks[SET::B]->at(entryB.block);
   const CRelAlgebra::TBlockEntry& tupleB =
           CRelAlgebra::TBlockEntry(tBlockB, entryB.row);

   // if both tuples are 3-dimensional, only now the z dimension is
   // being tested. If the tuples' bounding boxes do not intersect in the
   // z dimension, the pair is rejected at this late stage
   if (minDim == 3) { // TODO: testen
      CRelAlgebra::SpatialAttrArrayEntry<3> attrA = tupleA[attrIndices[SET::A]];
      CRelAlgebra::SpatialAttrArrayEntry<3> attrB = tupleB[attrIndices[SET::B]];
      const Rectangle<3>& bboxA = attrA.GetBoundingBox();
      const Rectangle<3>& bboxB = attrB.GetBoundingBox();
      if (bboxA.MaxD(2) < bboxB.MinD(2) || bboxB.MaxD(2) < bboxA.MinD(2))
         return true; // nothing was added to outTBlock, so result must be true
   }

   unsigned newCol = 0;
   for (SET set : SETS) {
      const CRelAlgebra::TBlockEntry& tuple = (set == SET::A) ? tupleA : tupleB;

#ifdef CDAC_SPATIAL_JOIN_DETAILED_REPORT_TO_CONSOLE
      if (set == SET::A)
         cout << endl;
      bool isLeft = (set == SET::A) ? entryA.isLeft : entryB.isLeft;
      cout << (isLeft ? "L" : "R") << SET_NAMES[set] << ": ";
      const unsigned attrIndex = attrIndices[set];
      if (dims[set] == 2) {
         CRelAlgebra::SpatialAttrArrayEntry<2> attr = tuple[attrIndex];
         attr.GetBoundingBox().Print(cout); // prints an endl
      } else {
         CRelAlgebra::SpatialAttrArrayEntry<3> attr = tuple[attrIndex];
         attr.GetBoundingBox().Print(cout); // prints an endl
      }
#endif

      for (uint64_t sourceCol = 0; sourceCol < columnCounts[set]; ++sourceCol) {
         newTuple[newCol] = tuple[sourceCol];
         ++newCol;
      }
   }

   outTBlock->Append(newTuple);
   return (outTBlock->GetSize() < this->outTBlockSize);
}