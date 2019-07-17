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


*/
#pragma once

#include "Merger.h" // -> ... -> <memory>, <vector>
                    // -> MergedArea, IOData
                    // -> InputStream, SortEdge, RectangleInfo, JoinEdge
                    // -> ... -> "Algebras/CRel/TBlock.h"
                    // -> ... -> "Algebras/Rectangle/RectangleAlgebra.h"
#include "SelfMerger.h"
#include "Timer.h"

namespace cdacspatialjoin {

/*
1 JoinTask enumeration

Lists the different tasks to be performed (possibly multiple times each)
during a CDACSpatialJoin or CDACSpatialJoinCount operation. This enum is used
to get distinct Timer evaluation for each task.

*/
enum JoinTask : unsigned {
   /* the task of requesting data from the InputStreams */
   requestData,
   /* the task of creating a JoinState instance */
   createJoinState,
   /* the task of creating a vector of SortEdge instances */
   createSortEdges,
   /* the task of sorting the vector of SortEdge instances */
   sortSortEdges,
   /* the task of creating a vector of JoinEdge instances */
   createJoinEdges,
   /* the task of merging the JoinEdges and reporting (or counting) the
    * intersections */
   merge,
   /* the task of destructing the CDACLocalInfo class */
   destructor
};

/*
2 JoinStateMemoryInfo struct

Encapsulates counters for the analysis of a JoinState's memory usage.

*/
struct JoinStateMemoryInfo {
   /* the size of the JoinState::joinEdges vector (a copy of
    * JoinState::joinEdgesSize) */
   size_t joinEdgesSize;

   /* the number of bytes used for the input data, i.e. the TBlocks / RBlocks
    * stored in IOData, during the whole lifetime of this JoinState */
   size_t usedInputDataMemory;

   /* the number of bytes used temporarily in the constructor for RectangleInfo
    * instances */
   size_t usedRectInfoMemory;

   /* the number of bytes used temporarily in the constructor for SortEdge
    * instances */
   size_t usedSortEdgeMemory;

   /* the number of bytes used for the JoinEdge vector during the whole
    * lifetime of this JoinState  */
   size_t usedJoinEdgeMemory;

   /* the number of bytes currently used for the Merger instance and all
    * MergedArea instances stored in mergedAreas[]. The variable is updated
    * whenever a mergedAreas[] entry is set */
   size_t usedMergedAreaMemory;

   /* the maximum number of bytes that occurred in usedMergedAreaMemory
    * during the lifetime of this JoinState instance */
   size_t usedMergedAreaMemoryMax;

   /* the number of of JoinEdge instances currently stored in the
    * Merger instance and all MergedArea instances */
   size_t mergeJoinEdgeCount;

   /* the maximum number of JoinEdge instances that were required in the
    * Merger and MergedArea instances at any given time */
   size_t mergeJoinEdgeCountMax;

   /* initializes the memory statistics with the given information */
   void initialize(size_t usedInputDataMemory_, size_t rectangleInfoCount,
           size_t sortEdgeCount, size_t joinEdgeCount);

   /* adds the memory used by the given MergedArea to the statistics of
    * currently used memory */
   inline void add(MergedAreaPtr mergedArea);

   /* subtracts the memory used by the given MergedArea from the statistics of
    * currently used memory */
   inline void subtract(MergedAreaPtr mergedArea);

   /* calculates the currently used memory (including MergedAreas and the given
    * Merger) and possibly increases the maximum values */
   inline void updateMaximum(Merger* merger);

   /* adds the memory used by the given SelfMergedArea to the statistics of
    * currently used memory */
   inline void add(SelfMergedAreaPtr mergedArea);

   /* subtracts the memory used by the given SelfMergedArea from the statistics
    * of currently used memory */
   inline void subtract(SelfMergedAreaPtr mergedArea);

   /* calculates the currently used memory (including SelfMergedAreas and the
    * given SelfMerger) and possibly increases the maximum values */
   inline void updateMaximum(SelfMerger* merger);

   /* the maximum number of main memory bytes used at any point during the
    * lifetime of this JoinState */
   size_t getTotalUsedMemoryMax() const;

   /* returns a) the maximum number of JoinEdge instances that were required
    * in the Merger and MergedArea instances at any given time, divided by
    * b) the number of rectangles from both input streams (excluding those
    * that were outside the other input stream's bounding box).
    * This value is in [1.0, 2.0] and is useful to estimate the memory required
    * for join operations. Samples with wide rectangles will produce a higher
    * value than samples with narrow rectangles, since initially, left and
    * right edges are stored for a rectangle, but as soon as a rectangle is
    * "complete" inside a MergedArea, only one edge (the rectangle's
    * Y-interval) is stored */
   double getUsedJoinEdgeQuotaMax() const {
      return mergeJoinEdgeCountMax / (joinEdgesSize  / 2.0);
   }
};

/*
3 JoinState class

At initialization, the JoinState class expects data from both input streams.
Its public method nextTBlock() then fills the given outTBlock with result
tuples until either the outTBlock is full, or the operation is complete.
Result tuples are combined from one tuple of each input stream respectively,
where the bounding box of these tuples' GeoData intersect.

If "countOnly == true" is passed in the constructor, intersections are merely
counted, but no output tuples are written to the outTBlock. The result count
can then be retrieved using the getOutTupleCount() getter.

JoinState detects self joins (if the bounding boxes of both inputs are exactly
identical, including the same TBlock positions) and treats this special case
efficiently using specialized methods and classes (SelfMergedArea, SelfMerger).

*/
class JoinState {
   /* ioData holds all current data from input both streams and provides
    * methods to a) extract the SortEdges and RectangleInfos from it, and
    * b) fill the output tuple block with result tuples (for which purpose it
    * is passed to Merger instances) */
   IOData ioData;

   /* true if only the number of intersections should be counted;
    * false if actual result tuples should be returned */
   const bool countOnly;

   /* the number of tuples stored in the current TBlocks (for each stream) */
   const uint64_t tupleCounts[SET_COUNT];

   /* the number of the CDACSpatialJoin[Count] operator (or, more precisely,
    * the CDACLocalInfo instance) that created this JoinState instance. This
    * number can be used to distinguish several operators in console output */
   const unsigned operatorNum;

   const std::string outputPrefix;

   std::shared_ptr<Timer> timer;

   /* true if a self join was detected, i.e. the rectangles in input A and B
    * are identical, and the tuples appear in the same order and at the same
    * addresses in the TBlocks of input A and B */
   bool selfJoin;

   // -----------------------------------------------------
   // variables storing the current state of the operation, allowing it to
   // be interrupted when the outTBlock is full, and to be resumed later

   /* stores the left and right edges of each rectangle in both sets, sorted
    * by their x position (and then their yMin value) */
   std::vector<JoinEdge> joinEdges;

   /* equal to joinEdges.size() (but faster to access) */
   size_t joinEdgesSize;

   /* the index in joinEdges from which the next (Self)MergedArea will be
    * created */
   EdgeIndex_t joinEdgeIndex;

   /* in a self join, each rectangle appears in both input A and input B and
    * therefore an intersection must be reported for each rectangle. This is
    * done at the beginning of selfJoinNextTBlock(), where this index points
    * to the next edge in joinEdges which will be considered */
   EdgeIndex_t idJoinEdgeIndex;

   static constexpr unsigned MERGED_AREAS_SIZE = 32;

   /* stores one MergedArea for each level. A MergedArea is waiting for
    * the an adjacent MergedArea with which it can be merged to a bigger
    * MergedArea (to be stored on the next level). index 0 represents the
    * lowest level; here a MergedArea may only contain JoinEdges from one
    * set */
   MergedAreaPtr mergedAreas[MERGED_AREAS_SIZE];

   /* for a self join, stores one SelfMergedArea for each level. A
    * SelfMergedArea is waiting for the an adjacent SelfMergedArea with which
    * it can be merged to a bigger SelfMergedArea (to be stored on the next
    * level). index 0 represents the lowest level; here a SelfMergedArea may
    * only contain one single JoinEdges */
   SelfMergedAreaPtr selfMergedAreas[MERGED_AREAS_SIZE];

   unsigned levelCount;

   /* the level (i.e. index in mergedAreas) at which the next merge operation
    * is performed (the result of which is stored one level higher, or
    * recursively merged with the entry at that level) */
   unsigned mergeLevel;

   /* the number of MergedAreas to be created on the lowest level. Note that an
    * "atomic" MergedArea is not created from a single JoinEdge but possibly
    * from a sequence of JoinEdges that belong to the same set */
   unsigned long atomsExpectedTotal;

   /* the number of MergedAreas already created on the lowest level */
   unsigned long atomsCreated;

   /* the average number of atomic MergedAreas that need to be merged before
    * the resulting MergedArea can move up from lowest level 0 to level 1.
    * This value simulates the "divide" part of divide and conquer and ensures
    * that MergedAreas are of approximately equal sizes, avoiding inefficient
    * merge operations. If atomsExpectedTotal (the total number of atomic
    * MergedAreas) happens to be a power of two, atomsExpectedStep will be
    * 2.0 exactly. In any other case, it will be slightly higher (but always
    * less than 4.0). */
   double atomsExpectedStep;

   /* accumulates atomsExpectedStep. At any given moment, the atomsCreated
    * value must exceed atomsExpectedNext in order for a resulting MergedArea
    * to be moved up to level 1. */
   double atomsExpectedNext;

   /* the Merger that performs the currently running merge operation;
    * nullptr, if no merge operation is currently running */
   Merger* merger;

   /* in a self join, the SelfMerger that performs the currently running merge
    * operation; nullptr, if no merge operation is currently running */
   SelfMerger* selfMerger;

   // -----------------------------------------------------
   // statistical

   /* the clock() time at which the initialization of this JoinState instance
    * was completed */
   clock_t initializeCompleted;

   /* the number of (non-empty) outTBlocks returned by this JoinState */
   unsigned outTBlockCount;

   /* is set to true once the join has completed; the outTBlock may still
    * contain the last result tuples */
   bool joinCompleted;

#ifdef CDAC_SPATIAL_JOIN_METRICS
   JoinStateMemoryInfo memoryInfo;
#endif

public:
   /* constructor taking all data required from the input streams:
    * attrIndexA/B: the positions of the join attributes;
    * tupleCountA/B: the number of tuples stored in the given tBlocks;
    * dimA/B: the dimension (2 or 3) of the spatial information;
    * outTBlockSize: the maximum size of the return TBlock in bytes;
    * joinStateId: the consecutive number of this JoinState instance */
   JoinState(bool countOnly_, InputStream* inputA_, InputStream* inputB_,
         uint64_t outTBlockSize_, unsigned operatorNum_, unsigned joinStateId_,
         std::shared_ptr<Timer>& timer_);

   ~JoinState();

   /* fills the given outTBlock with result tuples; returns true, if more
    * tuples were found, or false, if the operation is complete and no more
    * result tuples were found */
   bool nextTBlock(CRelAlgebra::TBlock* outTBlock_);

   size_t getOutTupleCount() const { return ioData.getOutTupleCount(); }

#ifdef CDAC_SPATIAL_JOIN_METRICS
   const JoinStateMemoryInfo& getMemoryInfo() const { return memoryInfo; }
#endif

private:
   /* creates a new Merger for the given areas, then deletes the areas */
   inline Merger* createMerger(unsigned levelOfArea1, MergedAreaPtr area2);

#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
   void reportLastMerge(MergedAreaPtr area1, MergedAreaPtr area2) const;
#endif

   /* self join version of nextTBlock. Fills the outTBlock in ioData with
    * result tuples; returns true, if more tuples were found, or false, if the
    * operation is complete and no more result tuples were found */
   bool selfJoinNextTBlock();

   /* creates a new Merger for the given areas, then deletes the areas */
   inline SelfMerger* createSelfMerger(unsigned levelOfArea1,
           SelfMergedAreaPtr area2);

#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
   void reportLastMerge(SelfMergedAreaPtr area1, SelfMergedAreaPtr area2) const;
#endif
};

} // end namespace
