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
#include "Algebras/Stream/Stream.h"
#include "Algebras/CRel/TBlock.h"
#include "Algebras/CRel/TypeConstructors/TBlockTC.h"

namespace cdacspatialjoin {

/*
1.1 CDACSpatialJoin

*/
class CDACSpatialJoin {
private:
   static uint64_t DEFAULT_BLOCK_SIZE; // in MB

public:
   explicit CDACSpatialJoin() = default;

    ~CDACSpatialJoin() = default;

   std::shared_ptr<Operator> getOperator();

private:
   class Info;

   static ListExpr typeMapping(ListExpr args);

   static CRelAlgebra::TBlockTI getTBlockTI(ListExpr attributeList,
           uint64_t desiredBlockSize, ListExpr& tBlockColumns);

   static int valueMapping(Word* args, Word& result, int message,
                           Word& local, Supplier s);
};


/*
1.2 InputStream classes

1.2.1 InputStream base class

*/
class InputStream {
public:
   const unsigned attrIndex; // index of join attribute

   const unsigned dim; // dimension of join attribute

   std::shared_ptr<std::vector<CRelAlgebra::TBlock*>> tBlocks;

private:
   uint64_t byteCount; // memory used by tBlocks

   uint64_t tupleCount; // number of tuples stored in tBlocks

protected:
   bool done; // true if all input has been read from the stream


public:
   InputStream(unsigned attrIndex_, unsigned dim_);

   virtual ~InputStream();

   // Deletes all tuple blocks of this input stream from the operator
   // and sets both the memory and tuple counters to zero
   void clearMem();

   // Requests tuple block from input stream and stores
   // them in tBlockVector
   bool request();

   bool hasTBlocks() { return !tBlocks->empty(); }

   size_t getUsedMem();

   bool isDone() { return done; }

   size_t getTupleCount() { return tupleCount; }

   virtual void restart() = 0;

private:
   virtual CRelAlgebra::TBlock* requestBlock() = 0;
};

/*
1.2.2 InputTBlockStream  class

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
1.2.3 InputTupleStream  class

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


/*
1.3 LocalInfo class

*/
class LocalInfo {
   InputStream* input1;
   InputStream* input2;

   Supplier s;

   bool isFirstRequest;
   bool isInput1FullyLoaded;

   uint64_t memLimit; // memory limit for operator

   const CRelAlgebra::TBlockTI outTypeInfo;
   const CRelAlgebra::PTBlockInfo outTBlockInfo;
   const uint64_t outTBlockSize;

   JoinState* joinState;
   unsigned joinStateCount;

public:
   // constructor
   LocalInfo(InputStream* input1_, InputStream* input2_, Supplier s);

   // destructor decreases the reference counters for all loaded tuple
   // blocks and closes both streams
   ~LocalInfo();

   CRelAlgebra::TBlock* getNext();

private:
   void requestInput();

   /* Computes the amount of memory in use, i.e. the size of the two block
    * vectors and of all binary tables */
   size_t getUsedMem();
}; // end class LocalInfo

} // end namespace cdacspatialjoin
