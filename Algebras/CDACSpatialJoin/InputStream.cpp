/*
1 InputStream class

1.1 InputStream base class

*/
#include "InputStream.h"
#include "Algebras/CRel/TypeConstructors/TBlockTC.h"

using namespace cdacspatialjoin;

InputStream::InputStream(unsigned attrIndex_, unsigned dim_) :
        attrIndex(attrIndex_),
        dim(dim_),
        tBlocks(new std::vector<CRelAlgebra::TBlock*>()),
        byteCount(0),
        tupleCount(0),
        openCount(0),
        chunkCount(0),
        done(false),
        fullyLoaded(false) {
}

InputStream::~InputStream() {
   clearMem();
}

void InputStream::clearMem() {
   for (CRelAlgebra::TBlock* tBlock : *tBlocks) {
      if (tBlock)
         tBlock->DecRef();
   }
   tBlocks->clear();
   byteCount = 0;
   tupleCount = 0;
   ++chunkCount;
}

bool InputStream::request() {
   CRelAlgebra::TBlock* tupleBlock = nullptr;
   if (!(tupleBlock = this->requestBlock())) {
      done = true;
      if (chunkCount == 1)
         fullyLoaded = true;
      return false;
   }

   tBlocks->push_back(tupleBlock);
   byteCount += tupleBlock->GetSize();
   tupleCount += tupleBlock->GetRowCount();
   return true;
}

size_t InputStream::getUsedMem() const {
   return byteCount;
}

void InputStream::streamOpened() {
   ++openCount;
   chunkCount = 1;
   done = false;
}

/*
1.2 InputTBlockStream  class

*/
InputTBlockStream::InputTBlockStream(Word stream_,
        const unsigned attrIndex_, const unsigned dim_) :
        InputStream(attrIndex_, dim_),
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

void InputTBlockStream::restart() {
   tBlockStream.close();
   tBlockStream.open();
   streamOpened();
}


/*
1.3 InputTupleStream  class

*/
InputTupleStream::InputTupleStream(Word stream_,
        const unsigned attrIndex_, const unsigned dim_,
        const CRelAlgebra::PTBlockInfo& blockInfo_,
        const uint64_t desiredBlockSize_) :
        InputStream(attrIndex_, dim_),
        tupleStream(stream_),
        blockInfo(blockInfo_),
        blockSize(desiredBlockSize_ * CRelAlgebra::TBlockTI::blockSizeFactor) {
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
   } while (block->GetSize() < blockSize &&
            (tuple = tupleStream.request()) != nullptr);
   return block;
}

void InputTupleStream::restart() {
   tupleStream.close();
   tupleStream.open();
   streamOpened();
}
