/*
1 JoinState class

At initialization, the JoinState class expects data from both input streams.
Its only public method, nextTBlock(), then fills the given outTBlock with
result tuples until either the outTBlock is full, or the operation is complete.

Result tuples are combined from one tuple of each input stream respectively,
where the bounding box of these tuples' GeoData intersect.

*/
#pragma once

#include <memory>
#include <vector>

#include "Utils.h"
#include "SortEdge.h"
#include "JoinEdge.h"
#include "MergedArea.h"
#include "Merger.h"
#include "RectangleInfo.h"
#include "InputStream.h"

#include "Algebras/CRel/TBlock.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"

namespace cdacspatialjoin {

typedef CRelAlgebra::TBlock* TBlockPtr;


class JoinState {
   // -----------------------------------------------------
   // data from the input streams, given to the constructor

   /* the TBlocks which were read from each of the two input streams */
   const std::shared_ptr<std::vector<CRelAlgebra::TBlock*>> tBlocks[SET_COUNT];

   /* the position of the join attribute in the tuple (for each stream) */
   const unsigned attrIndices[SET_COUNT];

   /* the number of columns (attributes) in the tuples (for each stream) */
   const uint64_t columnCounts[SET_COUNT];

   /* the number of tuples stored in the current TBlocks (for each stream) */
   const uint64_t tupleCounts[SET_COUNT];

   /* the number of tuples stored in the current TBlocks of both streams */
   const uint64_t tupleSum;

   /* the spatial dimension (2 or 3) of the GeoData (for each stream) */
   const unsigned dims[SET_COUNT];

   /* the number of spatial dimensions common to both streams */
   const unsigned minDim;

   /* the maximum size of the return TBlock in bytes */
   const uint64_t outTBlockSize;

   /* the consecutive number of this JoinState instance */
   const unsigned joinStateId;

   // -----------------------------------------------------
   // variables for computing SetRowBlock_t values (i.e. the "address" of
   // a rectangle in the input TBlocks).
   // Note that SET::A and SET::B have independent values for rowShift
   // (and consequently for rowMask and blockMask)

   /* the bit mask for the "set" information (SET::A / SET::B) which is
    * always stored in the highest bit of a SetRowBlock_t value */
   static constexpr SetRowBlock_t SET_MASK = 0x80000000;

   /* the number of bits by which the row information is left-shifted
    * inside a SetRowBlock_t. */
   const unsigned rowShift[SET_COUNT];

   /* the bit mask for the row information inside a SetRowBlock_t */
   const SetRowBlock_t rowMask[SET_COUNT];

   /* the bit mask for the block information in the lower bits of a
    * SetRowBlock_t value. */
   const SetRowBlock_t blockMask[SET_COUNT];

   // -----------------------------------------------------
   // variables storing the current state of the operation, allowing it to
   // be interrupted when the outTBlock is full, and to be resumed later

   /* stores the left and right edges of each rectangle in both sets, sorted
    * by their x position (and then their yMin value) */
   std::vector<JoinEdge> joinEdges;

   /* the index in joinEdges from which the next MergedArea will be created */
   EdgeIndex_t joinEdgeIndex = 0;

   /* stores one MergedArea for each level. A MergedArea is waiting for
    * the an adjacent MergedArea with which it can be merged to a bigger
    * MergedArea (to be stored on the next level). index 0 represents the
    * lowest level; here a MergedArea may only contain JoinEdges from one
    * set */
   std::vector<MergedAreaPtr> mergedAreas;

   /* the number of MergedAreas to be created on the lowest level. Note that an
    * "atomic" MergedArea is not created from a single JoinEdge but possibly
    * from a sequence of JoinEdges that belong to the same set */
   unsigned long level0AreaCountExpected = 0;

   /* the number of MergedAreas already created on the lowest level */
   unsigned long level0AreaCount = 0;

   /* the level (i.e. index in mergedAreas) at which the next merge operation
    * is performed (the result of which is stored one level higher, or
    * recursively merged with the entry at that level) */
   unsigned mergeLevel = 0;

   /* the Merger that performs the currently running merge operation;
    * nullptr, if no merge operation is currently running */
   std::unique_ptr<Merger> merger;

   /* an array of attributes of the result tuple type; this is used to
    * compile the next output tuple and pass it to the outTBlock */
   CRelAlgebra::AttrArrayEntry* const newTuple;

   // -----------------------------------------------------
   // variables making appendToOutput() more efficient

   /* the last source rectangle from set A that was used for output */
   SetRowBlock_t lastAddressA;

   /* the last source rectangle from set B that was used for output */
   SetRowBlock_t lastAddressB;

   // -----------------------------------------------------
   // statistical

   /* the clock() time at which the initialization of this JoinState instance
    * was completed */
   clock_t initializeCompleted;

   /* the number of (non-empty) outTBlocks returned by this JoinState */
   unsigned outTBlockCount;

   /* the total number of output tuples returned by this JoinState */
   uint64_t outTupleCount;

public:
   /* constructor taking all data required from the input streams:
    * tBlocksA/B containing as much tuples as the main memory can hold;
    * attrIndexA/B: the positions of the join attributes;
    * tupleCountA/B: the number of tuples stored in the given tBlocks;
    * dimA/B: the dimension (2 or 3) of the spatial information;
    * outTBlockSize: the maximum size of the return TBlock in bytes;
    * joinStateId: the consecutive number of this JoinState instance */
   JoinState(InputStream* inputA, InputStream* inputB,
         uint64_t outTBlockSize_, unsigned joinStateId_);

   ~JoinState();

   /* fills the given outTBlock with result tuples; returns true, if more
    * tuples were found, or false, if the operation is complete and no more
    * result tuples were found */
   bool nextTBlock(CRelAlgebra::TBlock* outTBlock);

private:
   static unsigned getRowShift(size_t blockCount);

   static SetRowBlock_t getRowMask(size_t rowShift);

   static SetRowBlock_t getBlockMask(size_t rowShift);

   inline SetRowBlock_t getAddress(const SET set, const RowIndex_t row,
                                   const BlockIndex_t block) const {
      return ((set == SET::A) ? 0 : SET_MASK)
            | static_cast<SetRowBlock_t>(row << rowShift[set]) | block;
      // SetRowBlock_t address = ...;
      // assert (set == getSet(address));
      // assert (row == getRowIndex(address));
      // assert (block == getBlockIndex(address));
      // return address;
   }

   inline SET getSet(const SetRowBlock_t address) const {
      return ((address & SET_MASK) == 0) ? SET::A : SET::B;
   }

   inline RowIndex_t getRowIndex(const SetRowBlock_t address) const {
      const SET set = getSet(address);
      return (address & rowMask[set]) >> rowShift[set];
   }

   inline BlockIndex_t getBlockIndex(const SetRowBlock_t address) const {
      const SET set = getSet(address);
      return static_cast<BlockIndex_t>(address & blockMask[set]);
   }

   std::string toString(const JoinEdge& joinEdge) const;

   /* calculates the bounding box of the given set; if addToEdges == true,
    * the edges of the rectangles in this set are added to sortEdges, and
    * information on these rectangles to rectangleInfos. otherBbox is only
    * required for addToEdges == true and used as a filter (since rectangles
    * that are completely outside the other set's bounding box need not be
    * considered any further) */
   Rectangle<3> calculateBboxAndEdges(SET set, bool addToEdges,
           const Rectangle<3>& otherBbox, std::vector<SortEdge>& sortEdges,
           std::vector<RectangleInfo>& rectangleInfos) const;

   /* enters the given, newly created MergedArea to the mergedAreas vector at
    * the current mergeLevel; if another MergedArea is already stored at this
    * level, a new Merger is created to merge the two areas */
   void enqueueMergedAreaOrCreateMerger(MergedAreaPtr& newArea,
                                        bool mayIncreaseMergeLevel);

   void createMerger(MergedAreaPtr& area1, MergedAreaPtr& area2);

   /* appends a new tuple to the outTBlock, creating it from the input tuples
    * represented by the two given JoinEdges */
   bool appendToOutput(const JoinEdge& entryS, const JoinEdge& entryT,
                       CRelAlgebra::TBlock* outTBlock);
};

} // end namespace
