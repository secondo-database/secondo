/*
----
This file is part of SECONDO.

Copyright (C) 2018,
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

\tableofcontents


1 CDACSpatialJoin operator

The ~cdacspatialjoin~ operator performs a cache-conscious spatial join on
two streams of tuples or tuple blocks, using a divide-and-conquer strategy.

As arguments, ~cdacspatialjoin~ expects two streams of tuples or tuple blocks.
Optionally, the name of the join attributes for each argument relation can be
specified. If these attribute names are omitted, the first attribute with a
suitable spatial kind is used. The operator returns a stream of tuple blocks.

The algorithm is based on Ralf Hartmut Gueting, Werner Schilling: A
practical divide-and-conquer algorithm for the rectangle intersection problem.
Inf. Sci. 42(2): 95-112 (1987). While this paper describes the self join case,
CDACSpatialJoin reports intersecting rectangles from two different rectangle
sets (streams) A and B.

*/

#pragma once

#include <memory>

#include "Operator.h"
#include "QueryProcessor.h"
#include "JoinState.h"
#include "InputStream.h"
#include "Timer.h"

#include "Algebras/Stream/Stream.h"
#include "Algebras/CRel/TBlock.h"
#include "Algebras/CRel/TypeConstructors/TBlockTC.h"

namespace cdacspatialjoin {

class CDACSpatialJoin {
private:
   /* the number of input streams (always 2, used for semantic clarity only) */
   static constexpr unsigned STREAM_COUNT = 2;

   /* the maximum number of args provided type mapping */
   static constexpr unsigned MAX_ARG_COUNT = 2 * STREAM_COUNT + 1;

   /* the TBlock size in MiB used to create TBlocks for InputTupleStreams
    * (i.e. when the input is tuples rather than TBlocks) */
   static uint64_t DEFAULT_INPUT_BLOCK_SIZE;

public:
   explicit CDACSpatialJoin() = default;

    ~CDACSpatialJoin() = default;

   std::shared_ptr<Operator> getOperator();

private:
   class Info;

   static ListExpr typeMapping(ListExpr args);

   static ListExpr typeMapping2(bool countOnly, ListExpr args);

   static CRelAlgebra::TBlockTI getTBlockTI(ListExpr attributeList,
           uint64_t desiredBlockSize, ListExpr& tBlockColumns);

   static int valueMapping(Word* args, Word& result, int message,
                           Word& local, Supplier s);

   static int valueMapping2(bool countOnly, Word* args, Word& result,
                            int message, Word& local, Supplier s);

   static InputStream* createInputStream(bool countOnly, Word* args,
           unsigned streamIndex);

   friend class CDACSpatialJoinCount;
};


/*
2 CDACSpatialJoinCount operator

The ~CDACSpatialJoinCount~ operator specializes the CDACSpatialJoin operator
for cases in which only the number of result tuples is required, but not the
result tuples themselves.

For ~CDACSpatialJoinCount~, only the bounding boxes of the respective join
attributes need to be kept in memory (for which purpose the RectangleBlock
class is being used), while all other tuple data can be discarded from memory,
allowing for JoinStates to treat rectangle sets of significantly higher
cardinality. Furthermore, the ~CDACSpatialJoinCount~ operator does not
implicitly convert input streams of tuples into tuple blocks (as the
~CDACSpatialJoin~ operator would) and, obviously, does not combine intersecting
tuples to create result tuples.

The four arguments of ~CDACSpatialJoinCount~ match the first four arguments of
the ~CDACSpatialJoin~ operator (i.e. two streams of tuples or tuple blocks and,
optionally, the names of the join attributes). The operator returns the number
of intersections, i.e. the number of tuples that a ~CDACSpatialJoin~ call with
the same arguments would return.

*/
class CDACSpatialJoinCount {
public:
   explicit CDACSpatialJoinCount() = default;

   ~CDACSpatialJoinCount() = default;

   std::shared_ptr<Operator> getOperator();

private:
   class Info;

   static ListExpr typeMapping(ListExpr args);

   static int valueMapping(Word* args, Word& result, int message,
                           Word& local, Supplier s);
};


/*
3 LocalInfo class

*/
class CDACLocalInfo {
   /* true if this instance is used for the CDACSpatialJoinCount operator
    * (which only returns the number of intersecting rectangles);
    * false if it is used for the CDACSpatialJoin operator (which returns
    * actual result tuples) */
   const bool countOnly;

   /* the first input stream. If neither stream can be kept in the main memory
    * at once, input1 is used as the inner loop (i.e. it is closed and
    * re-opened several times) */
   InputStream* const input1;

   /* the secondo input stream. If neither stream can be kept in the main memory
    * at once, input2 is used as the outer loop (i.e. it is traversed
    * only once) */
   InputStream* const input2;

   const Supplier s;

   /* true while no data has been requested from the input streams yet; false
    * thereafter */
   bool isFirstRequest;

   /* the memory limit for this operator */
   const uint64_t memLimit;

   /* information on the output TBlock type; unused if countOnly == true */
   const CRelAlgebra::TBlockTI outTypeInfo;
   const CRelAlgebra::PTBlockInfo outTBlockInfo;
   const uint64_t outTBlockSize;

   /* the current JoinState which operates on the data that could be read into
    * main memory */
   JoinState* joinState;

   /* the number of JoinState instances created so far */
   unsigned joinStateCount;

   /* the number of intersections found so far */
   size_t intersectionCount;

   std::shared_ptr<Timer> timer;

public:
   // constructor
   CDACLocalInfo(bool countOnly_, InputStream* input1_, InputStream* input2_,
           Supplier s);

   // destructor decreases the reference counters for all loaded tuple
   // blocks and closes both streams
   ~CDACLocalInfo();

   CRelAlgebra::TBlock* getNext();

   size_t getIntersectionCount() const { return intersectionCount; }

private:
   void requestInput();

   /* Computes the amount of memory in use, i.e. the size of the two block
    * vectors and of all binary tables */
   size_t getUsedMem() const;
}; // end class LocalInfo

} // end namespace cdacspatialjoin
