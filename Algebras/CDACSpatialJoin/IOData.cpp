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


1 IOData class

*/

#include "IOData.h"

#include "Algebras/CRel/SpatialAttrArray.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"

using namespace cdacspatialjoin;
using namespace std;


std::string IOData::getSetName(SET set) {
   return (set == SET::A) ? "A" : "B";
}

unsigned IOData::getRowShift(const size_t blockCount) {
   unsigned shift = 0;
   size_t blockId = blockCount - 1;
   while (blockId != 0) {
      ++shift;
      blockId >>= 1U;
   }
   return shift;
}

SetRowBlock_t IOData::getRowMask(const size_t rowShift) {
   return ~getBlockMask(rowShift) & ~SET_MASK;
}

SetRowBlock_t IOData::getBlockMask(const size_t rowShift) {
   return static_cast<SetRowBlock_t>((1U << rowShift) - 1);
}

IOData::IOData(const OutputType outputType_, TupleType* outputTupleType_,
               InputStream* inputA, InputStream* inputB,
               const uint64_t outBufferSize_,
               const uint64_t outBufferTupleCountMax_):
        outputType { outputType_ },
        outputTupleType { outputTupleType_ },
        tBlocks { &inputA->tBlocks, &inputB->tBlocks },
        tuples { &inputA->tuples, &inputB->tuples},
        rBlocks { &inputA->rBlocks, &inputB->rBlocks },
        attrIndices { inputA->attrIndex, inputB->attrIndex },
        columnCounts { inputA->attrCount, inputB->attrCount },
        dims { inputA->dim, inputB->dim },
        minDim(std::min(inputA->dim, inputB->dim)),
        rowShift { getRowShift(inputA->getBlockCount()),
                   getRowShift(inputB->getBlockCount()) },
        rowMask { getRowMask(rowShift[0]), getRowMask(rowShift[1]) },
        blockMask { getBlockMask(rowShift[0]), getBlockMask(rowShift[1]) },
        usedMemory { inputA->getUsedMem(), inputB->getUsedMem() },
        newTuple(new CRelAlgebra::AttrArrayEntry[
                      columnCounts[SET::A] + columnCounts[SET::B]]),
        outBufferSize(outBufferSize_),
        outBufferTupleCountMax(outBufferTupleCountMax_) {

   // set lastBlockA/B to a value that ensures the first "newTuple" to
   // be fully assembled in the appendToOutput() function
   lastAddressA = std::numeric_limits<SetRowBlock_t >::max();
   lastAddressB = std::numeric_limits<SetRowBlock_t >::max();

   outTupleCount = 0;
#ifdef CDAC_SPATIAL_JOIN_METRICS
   outTuplesMemSize = 0;
#endif
}

IOData::~IOData() {
   delete[] newTuple;
}

#ifdef CDAC_SPATIAL_JOIN_DETAILED_REPORT_TO_CONSOLE
std::string IOData::toString(const JoinEdge& joinEdge) const {
   stringstream st;
   SetRowBlock_t address = joinEdge.address;
   SET set = getSet(address);
   st << "y = [" << joinEdge.yMin << "; " << joinEdge.yMax << "]; ";
   st << (joinEdge.getIsLeft() ? "left" : "right") << " edge ";
   st << "from set " << getSetName(set) << ", ";
   st << "block " << getBlockIndex(set, address) << ", ";
   st << "row " << getRowIndex(set, address);
   return st.str();
}
#endif

Rectangle<3> IOData::calculateBboxAndEdges(const SET set,
        const bool addToEdges, const Rectangle<3>& otherBbox,
        vector<SortEdge>* sortEdges,
        vector<RectangleInfo>* rectangleInfos) const {

   // to optimize runtime, two distinct versions are being used
   // for the 2D and 3D cases:
   if (outputType == outputCount) {
      return calculateBboxAndEdgesCount(set, addToEdges, otherBbox, sortEdges,
                                        rectangleInfos);
   } else if (outputType == outputTBlockStream) {
      if (dims[set] == 2) {
         return calculateBboxAndEdges2DFromTBlocks(set, addToEdges, otherBbox,
                                                   sortEdges, rectangleInfos);
      } else if (dims[set] == 3) {
         return calculateBboxAndEdges3DFromTBlocks(set, addToEdges, otherBbox,
                                                   sortEdges, rectangleInfos);
      }
   } else if (outputType == outputTupleStream) {
      if (dims[set] == 2) {
         return calculateBboxAndEdges2DFromTuples(set, addToEdges, otherBbox,
                                                   sortEdges, rectangleInfos);
      } else if (dims[set] == 3) {
         return calculateBboxAndEdges3DFromTuples(set, addToEdges, otherBbox,
                                                   sortEdges, rectangleInfos);
      }
   } else {
      assert (false);
   }
   return Rectangle<3>(false);
}

Rectangle<3> IOData::calculateBboxAndEdges2DFromTBlocks(const SET set,
        const bool addToEdges, const Rectangle<3>& otherBbox,
        vector<SortEdge>* sortEdges,
        vector<RectangleInfo>* rectangleInfos) const {

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
   const std::vector<CRelAlgebra::TBlock*>* tBlocksOfSet = tBlocks[set];
   for (TBlockPtr block : *tBlocksOfSet) {
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

Rectangle<3> IOData::calculateBboxAndEdges3DFromTBlocks(const SET set,
        const bool addToEdges, const Rectangle<3>& otherBbox,
        vector<SortEdge>* sortEdges,
        vector<RectangleInfo>* rectangleInfos) const {

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
   const std::vector<CRelAlgebra::TBlock*>* tBlocksOfSet = tBlocks[set];
   for (TBlockPtr block : *tBlocksOfSet) {

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

Rectangle<3> IOData::calculateBboxAndEdges2DFromTuples(const SET set,
        const bool addToEdges, const Rectangle<3>& otherBbox,
        vector<SortEdge>* sortEdges,
        vector<RectangleInfo>* rectangleInfos) const {

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
   const std::vector<Tuple*>* tuplesOfSet = tuples[set];
   RowIndex_t row = 0;
   const BlockIndex_t blockNum = 0;
   for (Tuple* tuple : *tuplesOfSet) {
      ++row;
      // get the 2-dimensional extent of the current tuple's GeoData
      const Rectangle<2>& rec = ((StandardSpatialAttribute<2>*)
              tuple->GetAttribute(attrIndex))->BoundingBox();
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
                                   getAddress(set, row - 1, blockNum));
      sortEdges->emplace_back(xMin, rectInfoIndex, true);
      sortEdges->emplace_back(xMax, rectInfoIndex, false);
      ++rectInfoIndex;
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

Rectangle<3> IOData::calculateBboxAndEdges3DFromTuples(const SET set,
        const bool addToEdges, const Rectangle<3>& otherBbox,
        vector<SortEdge>* sortEdges,
        vector<RectangleInfo>* rectangleInfos) const {

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
   const std::vector<Tuple*>* tuplesOfSet = tuples[set];
   RowIndex_t row = 0;
   const BlockIndex_t blockNum = 0;
   for (Tuple* tuple : *tuplesOfSet) {
      ++row;
      // get the 2-dimensional extent of the current tuple's GeoData
      const Rectangle<3>& rec = ((StandardSpatialAttribute<3>*)
              tuple->GetAttribute(attrIndex))->BoundingBox();
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
                                   getAddress(set, row - 1, blockNum));
      sortEdges->emplace_back(xMin, rectInfoIndex, true);
      sortEdges->emplace_back(xMax, rectInfoIndex, false);
      ++rectInfoIndex;
   }

   // return this set's bounding box
   const double min[] { bboxXMin, bboxYMin, bboxZMin };
   const double max[] { bboxXMax, bboxYMax, bboxZMax };
   const bool defined = (bboxXMin < numeric_limits<double>::max());
   return Rectangle<3>(defined, min, max);
}

Rectangle<3> IOData::calculateBboxAndEdgesCount(const SET set,
        const bool addToEdges, const Rectangle<3>& otherBbox,
        vector<SortEdge>* sortEdges,
        vector<RectangleInfo>* rectangleInfos) const {

   assert (outputType == outputCount);
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
   const std::vector<RectangleBlock*>* rBlocksOfSet = rBlocks[set];
   for (RectangleBlock* rBlock : *rBlocksOfSet) {
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

size_t IOData::getOutputAddSize(const uint64_t tuplesAdded) const {
   switch(outputType) {
      case outputCount:
         return 0;
      case outputTBlockStream:
         return (outTBlock == nullptr) ? 0 : outTBlock->GetSize();
      case outputTupleStream:
         // since output tuples refer to the same Attribute instances as
         // input tuples, this value may be significantly smaller than
         // getOutputMemSize()
         return tuplesAdded * outputTupleType->GetCoreSize();
      default:
         return 0;
   }
}

#ifdef CDAC_SPATIAL_JOIN_METRICS
size_t IOData::getOutputMemSize() const {
   switch(outputType) {
      case outputCount:
         return 0;
      case outputTBlockStream:
         return (outTBlock == nullptr) ? 0 : outTBlock->GetSize();
      case outputTupleStream:
         return outTuplesMemSize;
      default:
         return 0;
   }
}
#endif

void IOData::addToOutTupleCount(const uint64_t count) {
   // assert (countOnly);
   outTupleCount += count;
}

bool IOData::selfJoinAppendToOutput(const JoinEdge& entryS,
        const JoinEdge& entryT) {
   bool result = appendToOutput(entryS, entryT, true);
   if ((entryS.address & ~SET_MASK) != (entryT.address & ~SET_MASK)) {
      result = appendToOutput(entryT, entryS, true);
   }
   return result;
}

bool IOData::appendToOutput(const JoinEdge& entryS, const JoinEdge& entryT,
          const bool overrideSet /* = false */) {
#ifndef CDAC_SPATIAL_JOIN_DETAILED_REPORT_TO_CONSOLE
   // for this very common case, shortcut the rest of the code
   if (outputType == outputCount && minDim == 2) {
      ++outTupleCount;
      return true;
   }
#endif

   const bool entrySIsSetA = overrideSet ? true :
           (getSet(entryS.address) == SET::A);
   const JoinEdge& entryA = entrySIsSetA ? entryS : entryT;
   const JoinEdge& entryB = entrySIsSetA ? entryT : entryS;
   const SetRowBlock_t addressA = entryA.address;
   const SetRowBlock_t addressB = entryB.address;
   const BlockIndex_t blockA = getBlockIndex(SET::A, addressA);
   const BlockIndex_t blockB = getBlockIndex(SET::B, addressB);
   const RowIndex_t rowA = getRowIndex(SET::A, addressA);
   const RowIndex_t rowB = getRowIndex(SET::B, addressB);
   if (outputType == outputCount) {
      // get input tuples represented by the two given JoinEdges
      const RectangleBlock* rBlockA = (*rBlocks[SET::A])[blockA];
      const RectangleBlock* rBlockB = (*rBlocks[SET::B])[blockB];

      // if both tuples are 3-dimensional, only now the z dimension is
      // being tested. If the tuples' bounding boxes do not intersect in the
      // z dimension, the pair is rejected at this late stage
      if (minDim == 3) {
         const PlainInterval& intervalA = rBlockA->getCoordsZ()[rowA];
         const PlainInterval& intervalB = rBlockB->getCoordsZ()[rowB];
         if (intervalA.zMax < intervalB.zMin || intervalB.zMax < intervalA.zMin)
            return true;
      }

#ifdef CDAC_SPATIAL_JOIN_DETAILED_REPORT_TO_CONSOLE
      cout << endl;
      static constexpr SET SETS[] { SET::A, SET::B };
      for (const SET set : SETS) {
         const JoinEdge& entry = (set == SET::A) ? entryA : entryB;
         const RowIndex_t row = (set == SET::A) ? rowA : rowB;
         cout << (entry.getIsLeft() ? "L" : "R") << getSetName(set) << ": ";
         const RectangleBlock* rBlock = (set == SET::A) ? rBlockA : rBlockB;
         if (dims[set] == 2) {
            rBlock->getRectangle2D(row).Print(cout); // prints an endl
         } else {
            rBlock->getRectangle3D(row).Print(cout); // prints an endl
         }
      }
#endif

      ++outTupleCount;
      return true;

   } else if (outputType == outputTBlockStream) {

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
            return true; // nothing was added to outTBlock
      }

#ifdef CDAC_SPATIAL_JOIN_DETAILED_REPORT_TO_CONSOLE
      cout << endl;
      static constexpr SET SETS[] { SET::A, SET::B };
      for (const SET set : SETS) {
         const JoinEdge& entry = (set == SET::A) ? entryA : entryB;
         const RowIndex_t row = (set == SET::A) ? rowA : rowB;
         cout << (entry.getIsLeft() ? "L" : "R") << getSetName(set) << ": ";

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
      // the corresponding newTuple[] entries can be reused.
      const size_t columnCountA = columnCounts[SET::A];
      // improve performance by using local variable for field "newTuple"
      CRelAlgebra::AttrArrayEntry* const newTuple_ = newTuple;
      if (addressA != lastAddressA) {
         const CRelAlgebra::TBlockEntry& tupleA =
               CRelAlgebra::TBlockEntry(tBlockA, rowA);
         for (uint64_t col = 0; col < columnCountA; ++col) {
            newTuple_[col] = tupleA[col];
         }
         lastAddressA = addressA;
      }
      if (addressB != lastAddressB) {
         const size_t columnCountB = columnCounts[SET::B];
         const CRelAlgebra::TBlockEntry& tupleB =
               CRelAlgebra::TBlockEntry(tBlockB, rowB);
         for (uint64_t col = 0; col < columnCountB; ++col) {
            newTuple_[columnCountA + col] = tupleB[col];
         }
         lastAddressB = addressB;
      }

      outTBlock->Append(newTuple_);
      ++outTupleCount;

      return (outTBlock->GetSize() < outBufferSize);

   } else if (outputType == outputTupleStream) {

      // get input tuples represented by the two given JoinEdges
      Tuple* tupleA = (*tuples[SET::A])[rowA];
      Tuple* tupleB = (*tuples[SET::B])[rowB];

      // if both tuples are 3-dimensional, only now the z dimension is
      // being tested. If the tuples' bounding boxes do not intersect in the
      // z dimension, the pair is rejected at this late stage
      if (minDim == 3) {
         const Rectangle<3>& bboxA = ((StandardSpatialAttribute<3>*)
                 tupleA->GetAttribute(attrIndices[SET::A]))->BoundingBox();
         const Rectangle<3>& bboxB = ((StandardSpatialAttribute<3>*)
                 tupleB->GetAttribute(attrIndices[SET::B]))->BoundingBox();
         if (bboxA.MaxD(2) < bboxB.MinD(2) || bboxB.MaxD(2) < bboxA.MinD(2))
            return true; // nothing was added to outTBlock
      }

#ifdef CDAC_SPATIAL_JOIN_DETAILED_REPORT_TO_CONSOLE
      cout << endl;
      static constexpr SET SETS[] { SET::A, SET::B };
      for (const SET set : SETS) {
         const JoinEdge& entry = (set == SET::A) ? entryA : entryB;
         cout << (entry.getIsLeft() ? "L" : "R") << getSetName(set) << ": ";
         const Tuple* tuple = (set == SET::A) ? tupleA : tupleB;
         if (dims[set] == 2) {
            const Rectangle<2>& bbox = ((StandardSpatialAttribute<2>*)
                    tuple->GetAttribute(attrIndices[set]))->BoundingBox();
            bbox.Print(cout); // prints an endl
         } else {
            const Rectangle<3>& bbox = ((StandardSpatialAttribute<3>*)
                    tuple->GetAttribute(attrIndices[set]))->BoundingBox();
            bbox.Print(cout); // prints an endl
         }
      }
#endif

      // copy attributes from tupleA and tupleB into the newTuple_
      auto newTuple_ = new Tuple(outputTupleType);
      Concat(tupleA, tupleB, newTuple_);
      outTuples->push_back(newTuple_);
      ++outTupleCount;
#ifdef CDAC_SPATIAL_JOIN_METRICS
      outTuplesMemSize += newTuple_->GetMemSize();
#endif

      // return true if more output tuples can be stored in outTuples.
      // Note that outTupleCount only counts the tuples added by the current
      // JoinState (and IOData) instance, and a half full outTuples vector
      // may be passed to a newly created IOData instance.
      return outTuples->size() < outBufferTupleCountMax;

   } else {
      assert (false);
      return false;
   }
}

#ifdef CDAC_SPATIAL_JOIN_METRICS
size_t IOData::getUsedMemory() const {
   size_t result = sizeof(IOData);
   // usedMemory[] already stores the memory used in TBlocks / RBlocks:
   result  += usedMemory[SET::A] + usedMemory[SET::B];
   if (newTuple) {
      result += (columnCounts[SET::A] + columnCounts[SET::B])
                * sizeof(CRelAlgebra::AttrArrayEntry);
   }
   return result;
}
#endif
