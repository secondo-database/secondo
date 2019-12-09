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


1 InputStream classes

InputStream encapsulates access to the underlying input stream which may
either be a stream of tuple blocks (InputTBlockStream) or a stream of tuples
(InputTupleStream).

If the InputStream is used for the CDACSpatialJoin operator, the requested data
is stored in a vector of Tuples or TBlocks, depending on the required output
type (for TBlock output, input from tuple streams is immediately being
converted into TBlocks); if the InputStream is used for the
CDACSpatialJoinCount operator, only the rectangles (bounding boxes) of the
spatial join attributes are extracted and stored in a vector of
RectangleBlocks, while all other tuple information is discarded from main
memory.

If neither InputStream fits into the main memory completely, the tuples or
bounding boxes from the "inner" stream A are saved into a temporary tuple file
and retrieved from there for all subsequent passes.

1.1 InputStream base class

*/
#pragma once

#include "RectangleBlock.h" // -> ... -> <memory>

#include "Stream.h"
#include "Algebras/CRel/TBlock.h"

namespace cdacspatialjoin {

enum OutputType {
   outputCount,
   outputTupleStream,
   outputTBlockStream
};

class InputStream {
public:
   /* the default size in RectangleBlock instances in MiB */
   static uint64_t DEFAULT_RECTANGLE_BLOCK_SIZE;

   /* the outputType determines the data that InputStream will accumulate:
    * - outputCount: rectangles in rBlocks (discarding the tuples / TBlocks)
    * - outputTupleStream: input tuples
    * - outputTBlockStream: input TBlocks (possibly created from tuples) */
   const OutputType outputType;

   /* the index of the join attribute */
   const unsigned attrIndex;

   /* the number of attributes */
   const unsigned attrCount;

   /* the dimension (2 or 3) of the join attribute */
   const unsigned dim;

   /* the size of the TBlocks in bytes */
   const uint64_t blockSizeInBytes;

   /* the TupleBlocks (TBlocks) received from this stream in the current
    * chunk (in case the output should be a TBlock stream) */
   std::vector<CRelAlgebra::TBlock*> tBlocks;

   /* the tuples received from this stream in the current chunk (in case the
    * output should be a tuple stream, too) */
   std::vector<Tuple*> tuples;

   /* the RectangleBlocks received from this stream in the current chunk
    * (in case bounding boxes are required only for the CDACSpatialJoinCount
    * operator) */
   std::vector<RectangleBlock*> rBlocks;

   /* the tuple type used in tupleFile */
   TupleType* tupleType;

   /* a tuple file to temporarily store tuples if the stream needs to be
    * passed more than once. tupleFile is also used to store the bounding
    * boxes in rBlocks */
   TupleFile* tupleFile;

   /* an iterator over tupleFile */
   TupleFileIterator* tupleFileIterator;

private:
   /* the memory currently used by tBlocks / tuples / rBlocks */
   uint64_t currentByteCount;

   /* the number of tuples currently stored in tBlocks / tuples / rBlocks */
   uint64_t currentTupleCount;

   /* the number of tuples received so far in this pass of the stream */
   uint64_t passTupleCount;

   /* the total number of tuples provided by this stream. This value is only
    * known after the stream was fully read once (i.e. when passCount > 1) */
   uint64_t totalTupleCount;

   /* the total memory used by all tBlocks / tuples / rBlocks read from this
    * stream. This value is only known after the stream was fully read once
    * (i.e. when passCount > 1) */
   uint64_t totalByteCount;

protected:
   /* the number of times the data of this stream was passed (1 during first
    * pass from the original stream; 2 etc. during subsequent passes, reading
    * from the temporary file) */
   unsigned passCount;

   /* 1 + the number of times clearMem was called after the stream was
    * originally opened or it was reopened from the temporary file */
   unsigned currentChunkCount;

   /* the total number of chunks needed for this stream. This value is only
    * known after the stream was fully read once (i.e. when passCount > 1) */
   unsigned chunksPerPass;

   /* true if all input has been read from the stream */
   bool done;

   /* true if all input could be read in the first chunk (i.e. without calling
    * clearMem() in between) */
   bool fullyLoaded;

public:
   /* creates a new InputStream to encapsulate reading the underlying stream.
    * outputType determines whether only the bbox of the join attribute is kept
    * in main memory, or full tuple information (in Tuples or TBlocks). The join
    * attribute must be found at index attrIndex_; dim_ must be 2 or 3. */
   InputStream(OutputType outputType_, unsigned attrIndex_, unsigned attrCount_,
           unsigned dim_, uint64_t blockSizeInMiB_);

   virtual ~InputStream();

   /* Deletes all TBlocks / tuples / bounding boxes of this input stream and
    * sets both the memory and tuple counters to zero */
   void clearMem();

   /* Requests data from the underlying stream and stores it in a TBlock /
    * Tuple vector or a RectangleBlock vector (bounding boxes only). Returns
    * true if any data was read. */
   bool request();

   /* returns the number of RectangleBlocks (if only bounding boxes are kept)
    * or TBlocks (if full tuple information is kept) */
   size_t getBlockCount() const;

   /* returns true if no information has been read to main memory since
    * construction or since the last clearMem() call */
   bool empty() const;

   /* returns the number of bytes currently used by the tBlocks / tuples /
    * rBlocks */
   size_t getUsedMem() const;

   /* returns true if the stream is completed */
   inline bool isDone() const { return done; }

   /* returns the number of tuples currently stored in the tBlocks / tuples /
    * rBlocks */
   inline size_t getCurrentTupleCount() const { return currentTupleCount; }

   /* returns the number of tuples received so far in this pass of the stream */
   inline size_t getPassTupleCount() const { return passTupleCount; }

   /* returns the number of times this stream was originally opened or
    * re-opened from the temporary file */
   inline unsigned getOpenCount() const { return passCount; }

   /* returns the number of chunks since the stream was opened or re-opened
    * from the temporary file */
   inline unsigned getChunkCount() const { return currentChunkCount; }

   /* returns the total number of tuples provided by this stream. This value is
    * only known after the stream was fully read once (i.e. when
    * passCount > 1) */
   uint64_t getTotalTupleCount() const;

   /* the total memory used by all tBlocks / tuples / rBlocks read from this
    * stream. This value is only known after the stream was fully read once
    * (i.e. when passCount > 1) */
   uint64_t getTotalByteCount() const;

   /* returns true if all input could be read to main memory in the first
    * chunk (i.e. with no clearMem() call) */
   inline bool isFullyLoaded() const { return fullyLoaded; }

   /* returns true if the total tuple count of this stream is known (i.e. a
    * first pass of this stream has already been read) and enough tuples have
    * been requested for the current chunk. This ensures that, starting from
    * the second pass, tuples are more equally distributed between the chunks,
    * potentially enabling the other stream to contribute more tuples to a
    * chunk */
   bool isAverageTupleCountExceeded() const;

   /* returns true if the input data is currently being read from a temporary
    * file: If neither stream completely fits into the main memory, such a file
    * is created during the first pass of the "inner" input stream. The
    * "outer" stream is never passed more than once and will therefore
    * always return false */
   bool isReadingFromTempFile() const { return tupleFileIterator != nullptr; }

   /* returns the Rectangle<2> (i.e. the bounding box of a 2D spatial attribute)
    * for the entry at the given (block, row) position or an invalid Rectangle
    * if no such entry exists. Must only be called if the dimension of the join
    * attribute is 2. Note that this method should only be used for occasional
    * access but is not optimized for bulk access */
   Rectangle<2> getRectangle2D(BlockIndex_t block, RowIndex_t row) const;

   /* returns the Rectangle<3> (i.e. the bounding box of a 3D spatial attribute)
    * for the entry at the given (block, row) position or an invalid Rectangle
    * if no such entry exists. Must only be called if the dimension of the join
    * attribute is 3. Note that this method should only be used for occasional
    * access but is not optimized for bulk access */
   Rectangle<3> getRectangle3D(BlockIndex_t block, RowIndex_t row) const;

   /* If applicable, saves the current chunk to a temporary file from which it
    * can be read again later. Returns true if something was actually saved. */
   bool saveToTempFile();

   /* closes and reopens the stream from the temporary file */
   virtual void restart() = 0;

protected:
   /* returns a RectangleBlock to which at least one more rectangle can be
    * added; if necessary, a new RectangleBlock is created and returned */
   RectangleBlock* getFreeRectangleBlock();

   /* must be called after the underlying stream was first opened or
    * restarted from the temporary file */
   void streamOpened();

   /* must be called after requesting information from the underlying stream.
    * Call with (0, 0, true) when the underlying stream is completed. */
   bool finishRequest(uint64_t bytesAdded, uint64_t tuplesAdded,
           bool isStreamExhausted);

   /* creates a new RectangleBlock from the tuples or TBlocks which are either
    * requested from the underlying input stream, or from the temporary file.
    * Returns true if any data was read. */
   virtual bool requestRectangles();

private:
   /* requests a TBlock from the underlying stream (or creates a new TBlock
    * from tuples requested from the underlying stream). Returns true if any
    * data was read. */
   virtual CRelAlgebra::TBlock* requestBlock() = 0;

   /* requests tuples from the underlying tuple stream until either the
    * stream is exhausted, or blockSizeInBytes is exceeded. Returns true if any
    * data was read. */
   virtual bool requestTuples() = 0;

   /* requests the next tuple from the temporary tupleFile (or, in the
    * InputTupleStream subclass, from the input tuple stream, on first pass of
    * the stream) */
   virtual Tuple* requestTuple();
};


/*
1.2 InputTBlockStream  class

*/
class InputTBlockStream : public InputStream {
   Stream<CRelAlgebra::TBlock> tBlockStream;

public:
   InputTBlockStream(Word stream_, OutputType outputType_, unsigned attrIndex_,
                     unsigned attrCount_, unsigned dim_,
                     uint64_t blockSizeInMiB_);

   ~InputTBlockStream() override;

   void restart() override;

private:
   bool requestRectangles() override;

   CRelAlgebra::TBlock* requestBlock() override;

   bool requestTuples() override { assert(false); return false; }
};


/*
1.3 InputTupleStream  class

*/
class InputTupleStream : public InputStream {
private:
   /* the input stream of tuples */
   Stream<Tuple> tupleStream;

   /* the column configuration of the TBlocks that will be created from the
    * tuples */
   const CRelAlgebra::PTBlockInfo blockInfo;

   /* the SmiFileId used when creating TBlocks */
   const SmiFileId fileId = 0;

public:
   InputTupleStream(Word stream_, OutputType outputType_, unsigned attrIndex_,
           unsigned attrCount_, unsigned dim_,
           const CRelAlgebra::PTBlockInfo& blockInfo_,
           uint64_t desiredBlockSizeInMiB_);

   ~InputTupleStream() override;

   void restart() override;

private:
   // for requestRectangles(), the InputStream implementation can be used:
   // only requestTuple() needs to be overridden

   CRelAlgebra::TBlock* requestBlock() override;

   bool requestTuples() override;

   Tuple* requestTuple() override;
};

} // end of namespace cdacspatialjoin
