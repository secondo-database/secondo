/*
1 State

*/
#pragma once

#include <memory>
#include <vector>

#include "Algebras/CRel/TBlock.h"
#include "Utils.h"

#define CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE

namespace cdacspatialjoin {

typedef CRelAlgebra::TBlock* TBlockPtr;

class JoinState {
   const std::shared_ptr<std::vector<CRelAlgebra::TBlock*>> tBlocks[2];
   unsigned attrIndex[2];
   uint64_t columnCount[2];
   uint64_t tupleCount[2];
   unsigned dim[2];
   uint64_t rTBlockSize;
   unsigned joinStateId;

   CRelAlgebra::AttrArrayEntry* const newTuple;

public:
   // Constructor
   JoinState(std::shared_ptr<std::vector<CRelAlgebra::TBlock*>> tBlocks1,
         std::shared_ptr<std::vector<CRelAlgebra::TBlock*>> tBlocks2,
         unsigned attrIndex1,
         unsigned attrIndex2,
         uint64_t tupleCount1,
         uint64_t tupleCount2,
         unsigned dim1,
         unsigned dim2,
         uint64_t rTBlockSize_,
         unsigned joinStateId_);

   ~JoinState();

   bool nextTBlock(CRelAlgebra::TBlock* outTBlock);
};

} // end namespace
