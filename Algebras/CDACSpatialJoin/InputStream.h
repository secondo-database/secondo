/*
1 InputStream classes

1.1 InputStream base class

*/
#pragma once

#include <memory>
#include "Algebras/Stream/Stream.h"
#include "Algebras/CRel/TBlock.h"

class InputStream {
public:
   /* the index of the join attribute */
   const unsigned attrIndex;

   /* the dimension of the join attribute */
   const unsigned dim;

   std::shared_ptr<std::vector<CRelAlgebra::TBlock*>> tBlocks;

private:
   /* the memory used by tBlocks */
   uint64_t byteCount;

   /* the number of tuples stored in tBlocks */
   uint64_t tupleCount;

protected:
   /* the number of times this stream was opened or re-opened */
   unsigned openCount;

   /* 1 + the number of times clearMem was called after the stream was last
    * opened or reopened */
   unsigned chunkCount;

   /* true if all input has been read from the stream */
   bool done;

   /* true if all input could be read in the first chunk (i.e. without calling
    * clearMem() in between */
   bool fullyLoaded;


public:
   InputStream(unsigned attrIndex_, unsigned dim_);

   virtual ~InputStream();

   /* Deletes all tuple blocks of this input stream and sets both the memory
    * and tuple counters to zero */
   void clearMem();

   /* Requests a tuple block from input stream and stores it in the TBlock
    * vector  */
   bool request();

   bool hasTBlocks() { return !tBlocks->empty(); }

   /* returns the number of bytes currently used by the TBlocks */
   size_t getUsedMem();

   /* returns true if the stream is completed */
   bool isDone() { return done; }

   /* returns the number of tuples currently stored in the TBlocks */
   size_t getTupleCount() { return tupleCount; }

   /* returns the number of times this stream was opened or re-opened */
   unsigned getOpenCount() { return openCount; }

   /* returns the number of chunks since the stream was opened or re-opened */
   unsigned getChunkCount() { return chunkCount; }

   /* returns true if all input could be read to main memory in the first
    * chunk (i.e. with no clearMem() call) */
   bool isFullyLoaded() { return fullyLoaded; }

   virtual void restart() = 0;

protected:
   void streamOpened();

private:
   virtual CRelAlgebra::TBlock* requestBlock() = 0;
};

/*
1.2 InputTBlockStream  class

*/
class InputTBlockStream : public InputStream {
   Stream<CRelAlgebra::TBlock> tBlockStream;

public:
   InputTBlockStream(Word stream_, unsigned attrIndex_, unsigned dim_);

   ~InputTBlockStream() override;

   void restart() override;

private:
   CRelAlgebra::TBlock* requestBlock() override;
};

/*
1.3 InputTupleStream  class

*/
class InputTupleStream : public InputStream {
private:
   Stream<Tuple> tupleStream;
   const CRelAlgebra::PTBlockInfo blockInfo;
   const uint64_t blockSize; // in bytes
   const SmiFileId fileId = 0; // block is not persistent

public:
   // desiredBlockSize in MB
   InputTupleStream(Word stream_, unsigned attrIndex_, unsigned dim_,
                    const CRelAlgebra::PTBlockInfo& blockInfo_,
                    uint64_t desiredBlockSize_);

   ~InputTupleStream() override;

   void restart() override;

private:
   CRelAlgebra::TBlock* requestBlock() override;
};


