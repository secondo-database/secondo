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
   merge
};


/*
2 JoinState class

At initialization, the JoinState class expects data from both input streams.
Its public method nextTBlock() then fills the given outTBlock with result
tuples until either the outTBlock is full, or the operation is complete.
Result tuples are combined from one tuple of each input stream respectively,
where the bounding box of these tuples' GeoData intersect.

If "countOnly == true" is passed in the constructor, intersections are merely
counted, but no output tuples are written to the outTBlock. The result count
can then be retrieved using the getOutTupleCount() getter.

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

   std::shared_ptr<Timer> timer;

   // -----------------------------------------------------
   // variables storing the current state of the operation, allowing it to
   // be interrupted when the outTBlock is full, and to be resumed later

   /* stores the left and right edges of each rectangle in both sets, sorted
    * by their x position (and then their yMin value) */
   std::vector<JoinEdge> joinEdges;

   /* equal to joinEdges.size() (but faster to access) */
   size_t joinEdgesSize;

   /* the index in joinEdges from which the next MergedArea will be created */
   EdgeIndex_t joinEdgeIndex;

   static constexpr unsigned MERGED_AREAS_SIZE = 32;

   /* stores one MergedArea for each level. A MergedArea is waiting for
    * the an adjacent MergedArea with which it can be merged to a bigger
    * MergedArea (to be stored on the next level). index 0 represents the
    * lowest level; here a MergedArea may only contain JoinEdges from one
    * set */
   MergedAreaPtr mergedAreas[MERGED_AREAS_SIZE];

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

public:
   /* constructor taking all data required from the input streams:
    * attrIndexA/B: the positions of the join attributes;
    * tupleCountA/B: the number of tuples stored in the given tBlocks;
    * dimA/B: the dimension (2 or 3) of the spatial information;
    * outTBlockSize: the maximum size of the return TBlock in bytes;
    * joinStateId: the consecutive number of this JoinState instance */
   JoinState(bool countOnly_, InputStream* inputA_, InputStream* inputB_,
         uint64_t outTBlockSize_, unsigned joinStateId_,
         std::shared_ptr<Timer>& timer_);

   ~JoinState();

   /* fills the given outTBlock with result tuples; returns true, if more
    * tuples were found, or false, if the operation is complete and no more
    * result tuples were found */
   bool nextTBlock(CRelAlgebra::TBlock* outTBlock_);

   size_t getOutTupleCount() const { return ioData.getOutTupleCount(); }

private:
   /* creates a new Merger for the given areas, then deletes the areas */
   inline Merger* createMerger(unsigned levelOfArea1, MergedAreaPtr area2) {
      MergedAreaPtr area1 = mergedAreas[levelOfArea1];
      mergedAreas[levelOfArea1] = nullptr;

      const bool isLastMerge = (area1->edgeIndexStart == 0 &&
                                area2->edgeIndexEnd == joinEdgesSize);

      // move ownership of source areas (area1 and area2) to new Merger;
      // source areas will be deleted in ~Merger()
      return new Merger(area1, area2, isLastMerge, &ioData);
   }
};

} // end namespace
