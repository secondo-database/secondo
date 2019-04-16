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


1 InputStream class

1.1 InputStream base class

*/
#include "InputStream.h"

#include "Algebras/CRel/TypeConstructors/TBlockTC.h"
#include "Algebras/CRel/SpatialAttrArray.h"

using namespace cdacspatialjoin;
using namespace std;

uint64_t InputStream::DEFAULT_RECTANGLE_BLOCK_SIZE = 10;

InputStream::InputStream(const bool rectanglesOnly_, const unsigned attrIndex_,
                         const unsigned attrCount_, const unsigned dim_,
                         const uint64_t blockSizeInMiB_) :
        rectanglesOnly(rectanglesOnly_),
        attrIndex(attrIndex_),
        attrCount(attrCount_),
        dim(dim_),
        blockSizeInBytes(blockSizeInMiB_ *
            CRelAlgebra::TBlockTI::blockSizeFactor),
        tBlocks {},
        rBlocks {},
        byteCount(0),
        currentTupleCount(0),
        passTupleCount(0),
        totalTupleCount(0),
        openCount(0),
        currentChunkCount(0),
        chunksPerPass(0),
        done(false),
        fullyLoaded(false) {
}

InputStream::~InputStream() {
   clearMem();
}

void InputStream::clearMem() {
   for (CRelAlgebra::TBlock* tBlock : tBlocks) {
      if (tBlock)
         tBlock->DecRef();
   }
   tBlocks.clear();
   for (RectangleBlock* rBlock : rBlocks)
      delete rBlock;
   rBlocks.clear();

   if (openCount == 1 && currentTupleCount > 0) {
      totalTupleCount += currentTupleCount;
      ++chunksPerPass;
   }
   byteCount = 0;
   currentTupleCount = 0;
   // passTupleCount remains unchanged
   ++currentChunkCount;
}

bool InputStream::request() {
   if (rectanglesOnly) {
      return requestRectangles();
   } else {
      CRelAlgebra::TBlock* tupleBlock = nullptr;
      if (!(tupleBlock = this->requestBlock())) {
         return finishRequest(0, 0);
      } else {
         tBlocks.push_back(tupleBlock);
         return finishRequest(tupleBlock->GetSize(), tupleBlock->GetRowCount());
      }
   }
}

bool InputStream::finishRequest(uint64_t bytesAdded, uint64_t tuplesAdded) {
   if (tuplesAdded == 0) {
      done = true;
      if (currentChunkCount == 1)
         fullyLoaded = true;
      return false;
   } else {
      byteCount += bytesAdded;
      currentTupleCount += tuplesAdded;
      passTupleCount += tuplesAdded;
      return true;
   }
}

size_t InputStream::getUsedMem() const {
   return byteCount;
}

bool InputStream::isAverageTupleCountExceeded() const {
   if (openCount <= 1)
      return false;
   return (currentTupleCount > totalTupleCount / chunksPerPass);
}

Rectangle<2> InputStream::getRectangle2D(BlockIndex_t block, RowIndex_t row)
      const {
   assert (dim == 2);

   if (rectanglesOnly) {
      if (block < rBlocks.size()) {
         const RectangleBlock* rBlock = rBlocks[block];
         if (row < rBlock->getRectangleCount())
            return rBlock->getRectangle2D(row);
      }
   } else {
      if (block < tBlocks.size()) {
         const CRelAlgebra::TBlock* tBlock = tBlocks[block];
         if (row < tBlock->GetRowCount()) {
            return ((const CRelAlgebra::SpatialAttrArray<2>*)
                    &tBlock->GetAt(attrIndex))->GetBoundingBox(row);
         }
      }
   }
   // parameters invalid
   return Rectangle<2>(false);
}

Rectangle<3> InputStream::getRectangle3D(BlockIndex_t block, RowIndex_t row)
         const {
   assert (dim == 3);

   if (rectanglesOnly) {
      if (block < rBlocks.size()) {
         const RectangleBlock* rBlock = rBlocks[block];
         if (row < rBlock->getRectangleCount())
            return rBlock->getRectangle3D(row);
      }
   } else {
      if (block < tBlocks.size()) {
         const CRelAlgebra::TBlock* tBlock = tBlocks[block];
         if (row < tBlock->GetRowCount()) {
            return ((const CRelAlgebra::SpatialAttrArray<3>*)
                    &tBlock->GetAt(attrIndex))->GetBoundingBox(row);
         }
      }
   }
   // parameters invalid
   return Rectangle<3>(false);
}

RectangleBlock* InputStream::getFreeRectangleBlock() {
   if (rBlocks.empty() || rBlocks.back()->isFull()) {
      size_t sizeInBytes = DEFAULT_RECTANGLE_BLOCK_SIZE * 1024 * 1024;
      rBlocks.emplace_back(new RectangleBlock(dim, sizeInBytes));
   }
   return rBlocks.back();
}

void InputStream::streamOpened() {
   ++openCount;
   currentChunkCount = 1;
   passTupleCount = 0;
   done = false;
}

/*
1.2 InputTBlockStream  class

*/
InputTBlockStream::InputTBlockStream(Word stream_, const bool rectanglesOnly_,
        const unsigned attrIndex_, const unsigned attrCount_,
        const unsigned dim_, const uint64_t blockSizeInMiB_) :
        InputStream(rectanglesOnly_, attrIndex_, attrCount_, dim_,
                blockSizeInMiB_),
        tBlockStream(stream_) {
   tBlockStream.open();
   streamOpened();
}

InputTBlockStream::~InputTBlockStream() {
   tBlockStream.close();
}

CRelAlgebra::TBlock* InputTBlockStream::requestBlock() {
   return tBlockStream.request();
}

bool InputTBlockStream::requestRectangles() {
   CRelAlgebra::TBlock* tupleBlock = nullptr;
   if (!(tupleBlock = this->requestBlock()))
      return finishRequest(0, 0);

   size_t tupleCount = 0;
   RectangleBlock* rBlock = getFreeRectangleBlock();

   if (dim == 2) {
      auto attrArray = (const CRelAlgebra::SpatialAttrArray<2>*)
              (&tupleBlock->GetAt(attrIndex));
      tupleCount = attrArray->GetCount();
      for (size_t row = 0; row < tupleCount; ++row) {
         const Rectangle<2>& rec = attrArray->GetBoundingBox(row);
         if (rec.IsDefined()) {
            rBlock->add(rec);
            if (rBlock->isFull())
               rBlock = getFreeRectangleBlock();
         }
      }
   } else {
      auto attrArray = (const CRelAlgebra::SpatialAttrArray<3>*)
              (&tupleBlock->GetAt(attrIndex));
      tupleCount = attrArray->GetCount();
      for (size_t row = 0; row < tupleCount; ++row) {
         const Rectangle<3>& rec = attrArray->GetBoundingBox(row);
         if (rec.IsDefined()) {
            rBlock->add(rec);
            if (rBlock->isFull())
               rBlock = getFreeRectangleBlock();
         }
      }
   }

   uint64_t bytesAdded = RectangleBlock::getRequiredMemory(dim, tupleCount);
   tupleBlock->DecRef();
   return finishRequest(bytesAdded, tupleCount);
}

void InputTBlockStream::restart() {
   tBlockStream.close();
   tBlockStream.open();
   streamOpened();
}

/*
1.3 InputTupleStream  class

*/
InputTupleStream::InputTupleStream(Word stream_, const bool rectanglesOnly_,
        const unsigned attrIndex_, const unsigned attrCount_,
        const unsigned dim_, const CRelAlgebra::PTBlockInfo& blockInfo_,
        const uint64_t desiredBlockSizeInMiB_) :
        InputStream(rectanglesOnly_, attrIndex_, attrCount_, dim_,
                    desiredBlockSizeInMiB_),
        tupleStream(stream_),
        blockInfo(blockInfo_) {
   tupleStream.open();
   streamOpened();
}

InputTupleStream::~InputTupleStream() {
   tupleStream.close();
}

CRelAlgebra::TBlock* InputTupleStream::requestBlock() {
   // cp. Algebras/CRel/Operators/ToBlocks.cpp
   Tuple *tuple = tupleStream.request();
   if (!tuple)
      return nullptr;

   auto block = new CRelAlgebra::TBlock(blockInfo, fileId, fileId, nullptr);
   do {
      block->Append(*tuple);
      tuple->DeleteIfAllowed();
   } while (block->GetSize() < blockSizeInBytes &&
            (tuple = tupleStream.request()) != nullptr);
   return block;
}

bool InputTupleStream::requestRectangles() {
   Tuple *tuple = tupleStream.request();
   if (!tuple)
      return finishRequest(0, 0);

   size_t tupleCount = 0;
   RectangleBlock* rBlock = getFreeRectangleBlock();
   if (dim == 2) {
      do {
         auto attr = dynamic_cast<StandardSpatialAttribute<2>*>(
                 tuple->GetAttribute(attrIndex));
         const Rectangle<2>& rec = attr->BoundingBox();
         if (rec.IsDefined()) {
            rBlock->add(rec);
            ++tupleCount;
         }
         tuple->DeleteIfAllowed();
      } while (!rBlock->isFull() && (tuple = tupleStream.request()) != nullptr);
   } else {
      do {
         auto attr = dynamic_cast<StandardSpatialAttribute<3>*>(
                 tuple->GetAttribute(attrIndex));
         const Rectangle<3>& rec = attr->BoundingBox();
         if (rec.IsDefined()) {
            rBlock->add(rec);
            ++tupleCount;
         }
         tuple->DeleteIfAllowed();
      } while (!rBlock->isFull() && (tuple = tupleStream.request()) != nullptr);
   }
   uint64_t bytesAdded = RectangleBlock::getRequiredMemory(dim, tupleCount);
   return finishRequest(bytesAdded, tupleCount);
}

void InputTupleStream::restart() {
   tupleStream.close();
   tupleStream.open();
   streamOpened();
}
