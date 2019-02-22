/*
1 State

*/

#include "JoinState.h"
#include "Algebras/CRel/SpatialAttrArray.h"

using namespace cdacspatialjoin;

JoinState::JoinState(
        std::shared_ptr<std::vector<CRelAlgebra::TBlock*>> tBlocks1,
          std::shared_ptr<std::vector<CRelAlgebra::TBlock*>> tBlocks2,
          unsigned attrIndex1,
          unsigned attrIndex2,
          uint64_t tupleCount1,
          uint64_t tupleCount2,
          unsigned dim1,
          unsigned dim2,
          uint64_t rTBlockSize,
          unsigned joinStateId_) :

        tBlocks { std::move<>(tBlocks1), std::move<>(tBlocks2) },
        attrIndex { attrIndex1, attrIndex2 },
        columnCount { tBlocks[0].get()->at(0)->GetColumnCount(),
                      tBlocks[1].get()->at(0)->GetColumnCount() },
        tupleCount { tupleCount1, tupleCount2 },
        dim { dim1, dim2 },
        rTBlockSize(rTBlockSize),
        joinStateId(joinStateId_),
        newTuple(new CRelAlgebra::AttrArrayEntry[
                columnCount[0] + columnCount[1]])

{
   // TODO: InputStream-Instanzen direkt übergeben?
#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
      cout << endl << "JoinState " << joinStateId << " created: " << endl;
      for (unsigned i = 0; i < 2; ++i) {
         cout << "* " << formatInt(tBlocks[i]->size()) << " blocks "
              << "with " << formatInt(tupleCount[i]) << " tuples "
              << "from stream " << std::to_string(i + 1) << "; ";
         auto iter = tBlocks[i]->at(0)->GetIterator();
         if (iter.IsValid()) {
            cout << "bbox of first entry is ";
            const CRelAlgebra::TBlockEntry& tuple = iter.Get();
            if (dim[i] == 2) {
               CRelAlgebra::SpatialAttrArrayEntry<2> attr = tuple[attrIndex[i]];
               attr.GetBoundingBox().Print(cout); // prints an endl
            } else {
               CRelAlgebra::SpatialAttrArrayEntry<3> attr = tuple[attrIndex[i]];
               attr.GetBoundingBox().Print(cout); // prints an endl
            }
         } else {
            cout << "** block is empty **" << endl;
         }
      }
#endif
}

JoinState::~JoinState() {
   delete[] newTuple;
}

bool JoinState::nextTBlock(CRelAlgebra::TBlock* outTBlock) {
   return false;
}
