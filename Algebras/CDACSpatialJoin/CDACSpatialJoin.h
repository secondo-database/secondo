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


1 CDACSpatialJoin operator

The ~cdacspatialjoin~ operator performs a cache-oriented spatial join on
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

#include "JoinState.h" // -> Timer;
                       // -> ... -> InputStream
                       //   -> "Algebras/Stream/Stream.h",
                       //   -> "Algebras/CRel/TBlock.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "Algebras/CRel/TypeConstructors/TBlockTC.h"

namespace cdacspatialjoin {

class CDACSpatialJoin {
private:
   /* the number of input streams (always 2, used for semantic clarity only) */
   static constexpr unsigned STREAM_COUNT = 2;

   /* the maximum number of args provided type mapping */
   static constexpr unsigned MAX_ARG_COUNT = 2 * STREAM_COUNT + 1;

   /* the TBlock size in MiB used to create TBlocks in InputTupleStreams
    * (when the input is tuples but must be converted to TBlocks) */
   static uint64_t DEFAULT_INPUT_BLOCK_SIZE_MIB;

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

   static InputStream* createInputStream(OutputType outputType, Word* args,
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

   // for valueMapping, CDACSpatialJoin::valueMapping is used
};

/*
3 MemoryInfo struct

Encapsulates counters for analysing the memory usage of the JoinState instances
created by a CDACLocalInfo instance.

*/
struct MemoryInfo {
   unsigned joinStateCount = 0;

   /* the maximum number of bytes used by any JoinState for its input data
    * (i.e. the TBlocks / RBlocks stored in IOData)  */
   size_t maxMemInputData = 0;

   /* the maximum number of bytes used by any JoinState for its SortEdge
    * instances */
   size_t maxMemSortEdges = 0;

   /* the maximum number of bytes used by any JoinState for its RectangleInfo
    * instances */
   size_t maxMemRectInfos = 0;

   /* the maximum number of bytes used by any JoinState for its JoinEdge
    * vector */
   size_t maxMemJoinEdges = 0;

   /* the maximum number of bytes used by any JoinState for its Merger and
    * MergedArea instances */
   size_t maxMemMergedAreas = 0;

   /* the maximum size in bytes of one chunk of output data (i.e. one output
    * TBlock or one chunk of output tuples). The memory for Attribute instances
    * that are shared with input data does not count here. */
   size_t maxMemOutputDataAddSize = 0;

   /* the maximum size in bytes of one chunk of output data (i.e. one output
    * TBlock or one chunk of output tuples). The memory for Attribute instances
    * that are shared with input data counts here. */
   size_t maxMemOutputDataMemSize = 0;

   /* the total maximum number of bytes used by any JoinState. Note that this
    * is not necessarily the same as the sum of the other maxMem... values,
    * since those maximum values may have occurred at different times */
   size_t maxMemTotal = 0;

   /* the total number of bytes used by all JoinStates for their input data
    * (i.e. the TBlocks / RBlocks stored in IOData)  */
   size_t sumMemInputData = 0;

   /* the total number of bytes used by all JoinStates for their SortEdge
    * instances */
   size_t sumMemSortEdges = 0;

   /* the total number of bytes used by all JoinStates for their RectangleInfo
    * instances */
   size_t sumMemRectInfos  = 0;

   /* the total number of bytes used by all JoinStates for their JoinEdge
    * vector */
   size_t sumMemJoinEdges = 0;

   /* the total number of bytes used by all JoinStates for their Merger and
    * MergedArea instances */
   size_t sumMemMergedAreas = 0;

   /* the total number of bytes used by all JoinStates for their respective
    * largest chunk of output data (i.e. for the largest output TBlock or the
    * largest chunk of output tuples). The memory for Attribute instances
    * that are shared with input data does not count here. */
   size_t sumMemOutputDataAddSizeMax = 0;

   /* the total number of bytes used by all JoinStates for their respective
    * largest chunk of output data (i.e. for the largest output TBlock or the
    * largest chunk of output tuples). The memory for Attribute instances
    * that are shared with input data counts here. */
   size_t sumMemOutputDataMemSizeMax = 0;

   /* the total number of tuples provided by input stream A (in one pass) */
   size_t totalInputATupleCount = 0;

   /* the total number of bytes provided by input stream A (in one pass) */
   size_t totalInputADataSize = 0;

   /* the total number of tuples provided by input stream B (in one pass) */
   size_t totalInputBTupleCount = 0;

   /* the total number of bytes provided by input stream B (in one pass) */
   size_t totalInputBDataSize = 0;

   /* the total number of output tuples generated by all JoinStates */
   size_t totalOutputTupleCount = 0;

   /* the total number of bytes generated by all JoinStates for their output
    * data (i.e. the TBlocks passed down the stream). The memory for Attribute
    * instances that are shared with input data does not count here. */
   size_t totalOutputDataAddSize = 0;

   /* the total number of bytes generated by all JoinStates for their output
    * data (i.e. the TBlocks passed down the stream). The memory for Attribute
    * instances that are shared with input data counts here. */
   size_t totalOutputDataMemSize = 0;

   /* the total number of bytes used by all JoinStates. This equals the sum
    * of the other sumMem... values */
   size_t sumMemTotal = 0;

   double maxJoinEdgeQuota = 0.0;


   MemoryInfo() = default;

   ~MemoryInfo() = default;

   void add(const JoinStateMemoryInfo& joinStateInfo);

   void setInputSize(size_t totalInputATupleCount_, size_t totalInputADataSize_,
           size_t totalInputBTupleCount_, size_t totalInputBDataSize_);

   void print(std::ostream& out, OutputType outputType);

private:

   void printLineMem(std::ostream& out, const std::string& text,
           size_t sumValue, size_t maxValue, const std::string& note,
           unsigned cacheLineSize);

   static void printLineInOut(std::ostream& out, const std::string& text,
           uint64_t bytes, uint64_t tupleCount, const std::string& note);
};

/*
4 LocalInfo class

*/
class CDACLocalInfo {
   static unsigned activeInstanceCount;

   /* if the desired output type is a tuple stream, the join operation will
    * use a vector of tuples to temporarily store some output tuples. This
    * value determines how much main memory (in KiB) may be used by these
    * output tuples, before the temporary output tuple vector is flushed to the
    * stream */
   static uint64_t OUTPUT_TUPLE_VECTOR_MEM_SIZE_KIB;

   /* the desired output type: outputCount, if only the number of intersecting
    * rectangles should be returned (i.e. the CDACSpatialJoinCount operator was
    * called); outputTupleStream, if the result tuples should be returned as
    * a stream of tuples; outputTBlockStream, if the result tuples should be
    * returned as a stream of tuple blocks (both done by the CDACSpatialJoin
    * operator) */
   const OutputType outputType;

   /* the TupleType of the output tuples (if the desired outputType is a tuple
    * stream) */
   TupleType* outputTupleType;

   /* the first input stream. If neither stream can be kept in the main memory
    * at once, inputA is used as the inner loop (i.e. it is closed and
    * re-opened several times) */
   InputStream* const inputA;

   /* the secondo input stream. If neither stream can be kept in the main memory
    * at once, inputB is used as the outer loop (i.e. it is traversed
    * only once) */
   InputStream* const inputB;

   const Supplier s;

   /* true while no data has been requested from the input streams yet; false
    * thereafter */
   bool isFirstRequest;

   /* the memory limit for this operator */
   const uint64_t memLimit;

   /* information on the output TBlock type; unused if countOnly == true */
   const CRelAlgebra::TBlockTI outTypeInfo;
   const CRelAlgebra::PTBlockInfo outTBlockInfo;

   /* the size of the output buffer (i.e. the output TBlock or the outTuples
    * vector) in bytes */
   const uint64_t outBufferSize;

   /* a number with which different CDACLocalInfo instances can be
    * distinguished in console output (e.g. if several CDACSpatialJoin[Count]
    * operators are used within one query) */
   const unsigned instanceNum;

   /* the current JoinState which operates on the data that could be read into
    * main memory */
   JoinState* joinState;

   /* the number of JoinState instances created so far */
   unsigned joinStateCount;

   /* the number of intersections found so far */
   size_t intersectionCount;

   /* the current output tuple block (used only if the desired outputType is a
    * stream of TBlocks) */
   CRelAlgebra::TBlock* outTBlock;

   /* the current vector for output tuples (used only if the desired
    * outputType is a stream of tuples). This vector serves as a temporary
    * store and is flushed once the tuples exceed the memory limit given by
    * OUTPUT\_TUPLE\_VECTOR\_MEM\_SIZE\_MIB */
   std::vector<Tuple*>* outTuples;

#ifdef CDAC_SPATIAL_JOIN_METRICS
   MemoryInfo memoryInfo;
#endif

   std::shared_ptr<Timer> timer;

public:
   // constructor
   CDACLocalInfo(OutputType outputType_, ListExpr outputTupleTypeLE,
           InputStream* inputA_, InputStream* inputB_, Supplier s);

   // destructor decreases the reference counters for all loaded tuple
   // blocks and closes both streams
   ~CDACLocalInfo();

   size_t getIntersectionCount() const { return intersectionCount; }

   CRelAlgebra::TBlock* getNextTBlock();

   Tuple* getNextTuple();

   bool getNext();

private:
   void requestInput();

   /* Estimates the required amount of main memory, i.e. both the size of the
    * two input TBlock / RBlock vectors, and the expected JoinState memory
    * usage */
   size_t getRequiredMemory() const;

   std::string getOperatorName() const;
}; // end class LocalInfo

} // end namespace cdacspatialjoin
