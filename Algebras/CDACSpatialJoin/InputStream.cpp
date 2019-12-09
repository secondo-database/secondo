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

InputStream::InputStream(const OutputType outputType_,
        const unsigned attrIndex_, const unsigned attrCount_,
        const unsigned dim_, const uint64_t blockSizeInMiB_) :
        outputType(outputType_),
        attrIndex(attrIndex_),
        attrCount(attrCount_),
        dim(dim_),
        blockSizeInBytes(blockSizeInMiB_ *
            CRelAlgebra::TBlockTI::blockSizeFactor),
        tBlocks {},
        rBlocks {},
        tupleType(nullptr),
        tupleFile(nullptr),
        tupleFileIterator(nullptr),
        currentByteCount(0),
        currentTupleCount(0),
        passTupleCount(0),
        totalTupleCount(0),
        totalByteCount(0),
        passCount(0),
        currentChunkCount(0),
        chunksPerPass(0),
        done(false),
        fullyLoaded(false) {
}

InputStream::~InputStream() {
   clearMem();
   delete tupleFileIterator;
   delete tupleFile;
   if (tupleType) {
      tupleType->DeleteIfAllowed();
      tupleType = nullptr;
   }
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

   for (Tuple* tuple : tuples) {
      if (tuple)
         tuple->DeleteIfAllowed();
   }
   tuples.clear();

   if (passCount == 1 && currentTupleCount > 0) {
      totalTupleCount += currentTupleCount;
      totalByteCount += currentByteCount;
      ++chunksPerPass;
   }
   currentTupleCount = 0;
   currentByteCount = 0;
   // passTupleCount remains unchanged
   ++currentChunkCount;
}

bool InputStream::request() {
   switch(outputType) {
      case outputCount: {
         return requestRectangles();
      }
      case outputTupleStream: {
         return requestTuples();
      }
      case outputTBlockStream: {
         CRelAlgebra::TBlock* tupleBlock = nullptr;
         if (!(tupleBlock = this->requestBlock())) {
            return finishRequest(0, 0, true);
         } else {
            tBlocks.push_back(tupleBlock);
            return finishRequest(tupleBlock->GetSize(),
                                 tupleBlock->GetRowCount(), false);
         }
      }
      default:
         return false;
   }
}

bool InputStream::finishRequest(uint64_t bytesAdded, uint64_t tuplesAdded,
        bool isStreamExhausted) {
   currentByteCount += bytesAdded;
   currentTupleCount += tuplesAdded;
   passTupleCount += tuplesAdded;
   if (isStreamExhausted) {
      done = true;
      if (currentChunkCount == 1)
         fullyLoaded = true;
   }
   return (tuplesAdded > 0);
}

size_t InputStream::getBlockCount() const {
   switch(outputType) {
      case outputCount:
         return rBlocks.size();
      case outputTupleStream:
         return 1;
      case outputTBlockStream:
         return tBlocks.size();
      default:
         return true;
   }
}

bool InputStream::empty() const {
   switch(outputType) {
      case outputCount:
         return rBlocks.empty();
      case outputTupleStream:
         return tuples.empty();
      case outputTBlockStream:
         return tBlocks.empty();
      default:
         return true;
   }
}

size_t InputStream::getUsedMem() const {
   return currentByteCount;
}

uint64_t InputStream::getTotalTupleCount() const {
   return totalTupleCount + ((passCount == 1) ? currentTupleCount : 0);
}

uint64_t InputStream::getTotalByteCount() const {
   return totalByteCount + ((passCount == 1) ? currentByteCount : 0);
}

bool InputStream::isAverageTupleCountExceeded() const {
   if (passCount <= 1)
      return false;
   return (currentTupleCount > totalTupleCount / chunksPerPass);
}

Rectangle<2> InputStream::getRectangle2D(BlockIndex_t block, RowIndex_t row)
      const {
   assert (dim == 2);

   if (outputType == outputCount) {
      if (block < rBlocks.size()) {
         const RectangleBlock* rBlock = rBlocks[block];
         if (row < rBlock->getRectangleCount())
            return rBlock->getRectangle2D(row);
      }
   } else if (outputType == outputTupleStream) {
      return ((const StandardSpatialAttribute<2>*)
              tuples[row]->GetAttribute(attrIndex))->BoundingBox();
   } else { // outputType == outputTBlockStream
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

   if (outputType == outputCount) {
      if (block < rBlocks.size()) {
         const RectangleBlock* rBlock = rBlocks[block];
         if (row < rBlock->getRectangleCount())
            return rBlock->getRectangle3D(row);
      }
   } else if (outputType == outputTupleStream) {
      return ((const StandardSpatialAttribute<3>*)
              tuples[row]->GetAttribute(attrIndex))->BoundingBox();
   } else { // outputType == outputTBlockStream
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
   ++passCount;
   currentChunkCount = 1;
   passTupleCount = 0;
   done = false;
}

bool InputStream::saveToTempFile() {
   // this method is called only if the this InputStream is the "inner" stream,
   // and neither InputStream fits into the main memory. In this case, the data
   // which is currently loaded into the main memory (tuples, tuple blocks, or
   // bounding boxes, depending on the desired output type) must be saved to
   // disk in order to allow reading it a second time for the next chunk of
   // the "outer" stream.
   if (outputType == outputTupleStream) {
      if (tuples.empty())
         return false; // nothing to save

      // create a tupleFile for the tuples in memory, unless it already exists
      // (only the first chunk of the InputStream creates the tupleFile)
      if (!tupleFile) {
         tupleType = tuples[0]->GetTupleType();
         tupleType->IncReference();
         tupleFile = new TupleFile(tupleType, 0);
      }

      // save the tuples to the tupleFile
      for (Tuple* tuple : tuples) {
         // tuple->PinAttributes();
         // tupleFile->AppendTupleNoLOBs(tuple);
         tupleFile->Append(tuple);
      }
      return true;

   } else if (outputType == outputCount) {
      if (rBlocks.empty())
         return false; // nothing to save

      // create a tupleFile for the bounding boxes, unless it already exists
      // (only the first chunk of the InputStream creates the tupleFile)
      if (!tupleFile) {
         string rectType = (dim == 2) ? Rectangle<2>::BasicType()
                                      : Rectangle<3>::BasicType();
         ListExpr geoAttr = nl->TwoElemList(nl->SymbolAtom("BBox"),
                                            nl->SymbolAtom(rectType));
         ListExpr ttLE = nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                         nl->OneElemList(geoAttr));
         ListExpr ttNum = SecondoSystem::GetCatalog()->NumericType(ttLE);
         tupleType = new TupleType(ttNum);
         tupleFile = new TupleFile(tupleType, 0);
      }

      // save the rBlocks to the tupleFile, using its Rectangle<2/3> attribute
      for (RectangleBlock* rBlock : rBlocks) {
         size_t count = rBlock->getRectangleCount();
         for (size_t row = 0; row < count; ++row) {
            auto tuple = new Tuple(tupleType);
            Attribute* geoAttr = rBlock->getRectangleAttr(row);
            tuple->PutAttribute(0, geoAttr);
            tupleFile->Append(tuple);
            tuple->DeleteIfAllowed();
         }
      }
      return true;

   } else if (outputType == outputTBlockStream) {
      // TODO: save TBlocks from memory to TBlock file
   }
   return false;
}

Tuple* InputStream::requestTuple() {
   return tupleFileIterator->GetNextTuple();
}

bool InputStream::requestRectangles() {
   // request next tuple from tupleFile or (in the overridden InputTupleStream
   // implementation) from the underlying input tuple stream
   Tuple* tuple = requestTuple();
   if (!tuple)
      return finishRequest(0, 0, true);

   // if we are reading from the tupleFile, there is only one attribute with
   // the bounding box at index 0. Otherwise, use the attrIndex that depends
   // on the tuple type of the underlying input stream
   const unsigned int curAttrIndex = tupleFileIterator ? 0 : this->attrIndex;

   size_t tupleCount = 0;
   RectangleBlock* rBlock = getFreeRectangleBlock();
   if (dim == 2) {
      do {
         auto attr = dynamic_cast<StandardSpatialAttribute<2>*>(
                 tuple->GetAttribute(curAttrIndex));
         const Rectangle<2>& rec = attr->BoundingBox();
         if (rec.IsDefined()) {
            rBlock->add(rec);
            ++tupleCount;
         }
         tuple->DeleteIfAllowed();
      } while (!rBlock->isFull() && (tuple = requestTuple()) != nullptr);
   } else {
      do {
         auto attr = dynamic_cast<StandardSpatialAttribute<3>*>(
                 tuple->GetAttribute(curAttrIndex));
         const Rectangle<3>& rec = attr->BoundingBox();
         if (rec.IsDefined()) {
            rBlock->add(rec);
            ++tupleCount;
         }
         tuple->DeleteIfAllowed();
      } while (!rBlock->isFull() && (tuple = requestTuple()) != nullptr);
   }
   uint64_t bytesAdded = RectangleBlock::getRequiredMemory(dim, tupleCount);
   return finishRequest(bytesAdded, tupleCount, (tuple == nullptr));
}


/*
1.2 InputTBlockStream  class

*/
InputTBlockStream::InputTBlockStream(Word stream_, const OutputType outputType_,
        const unsigned attrIndex_, const unsigned attrCount_,
        const unsigned dim_, const uint64_t blockSizeInMiB_) :
        InputStream(outputType_, attrIndex_, attrCount_, dim_,
                blockSizeInMiB_),
        tBlockStream(stream_) {
   assert (outputType_ != outputTupleStream);
   tBlockStream.open();
   streamOpened();
}

InputTBlockStream::~InputTBlockStream() {
   if (outputType == outputCount) {
      if (passCount == 1) {
         tBlockStream.close();
      }
   } else {
      tBlockStream.close();
   }
}

bool InputTBlockStream::requestRectangles() {
   if (tupleFileIterator) {
      return InputStream::requestRectangles();
   }

   CRelAlgebra::TBlock* tupleBlock = nullptr;
   if (!(tupleBlock = this->requestBlock()))
      return finishRequest(0, 0, true);

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
   return finishRequest(bytesAdded, tupleCount, false);
}

CRelAlgebra::TBlock* InputTBlockStream::requestBlock() {
   // TODO: if (passCount > 1) read chunk from TBlock file; else:
   return tBlockStream.request();
}

void InputTBlockStream::restart() {
   switch(outputType) {
      case outputTupleStream:
         assert(false);
         break;

      case outputCount:
         if (passCount == 1) {
            tBlockStream.close();
         }
         assert(tupleFile);
         delete tupleFileIterator;
         tupleFileIterator = tupleFile->MakeScan();
         break;

      case outputTBlockStream:
         // TODO: after first pass, read data from file!
         tBlockStream.close();
         tBlockStream.open();
         break;
   }
   streamOpened();
}

/*
1.3 InputTupleStream  class

*/
InputTupleStream::InputTupleStream(Word stream_, const OutputType outputType_,
        const unsigned attrIndex_, const unsigned attrCount_,
        const unsigned dim_, const CRelAlgebra::PTBlockInfo& blockInfo_,
        const uint64_t desiredBlockSizeInMiB_) :
        InputStream(outputType_, attrIndex_, attrCount_, dim_,
                    desiredBlockSizeInMiB_),
        tupleStream(stream_),
        blockInfo(blockInfo_) {
   tupleStream.open();
   streamOpened();
}

InputTupleStream::~InputTupleStream() {
   if (outputType == outputTupleStream || outputType == outputCount) {
      if (passCount == 1) {
         tupleStream.close();
      }
   } else {
      tupleStream.close();
   }
}

CRelAlgebra::TBlock* InputTupleStream::requestBlock() {
   // TODO: if (passCount > 1) read chunk from TBlock file; else:

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

bool InputTupleStream::requestTuples() {
   Tuple* tuple = nullptr;
   uint64_t tupleCount = 0;
   uint64_t sizeSum = 0;
   while (sizeSum < blockSizeInBytes && (tuple = requestTuple())) {
      tuples.push_back(tuple);
      ++tupleCount;
      sizeSum += sizeof(Tuple*) + tuple->GetMemSize();
   }
   return finishRequest(sizeSum, tupleCount, (tuple == nullptr));
}

Tuple* InputTupleStream::requestTuple() {
   if (tupleFileIterator) {
      // after first pass: read tuples from the temporary tupleFile
      return tupleFileIterator->GetNextTuple();
   } else {
      // first pass: read tuples from the input tuple stream
      return tupleStream.request();
   }
}

void InputTupleStream::restart() {
   switch(outputType) {
      case outputTupleStream:
      case outputCount:
         if (passCount == 1) {
            tupleStream.close();
         }

         assert(tupleFile);
         delete tupleFileIterator;
         tupleFileIterator = tupleFile->MakeScan();
         break;

      case outputTBlockStream:
         // TODO: after first pass, read data from TBlock file!

         tupleStream.close();
         tupleStream.open();
         break;
   }

   streamOpened();
}
