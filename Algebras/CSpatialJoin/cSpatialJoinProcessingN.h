/*
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

*/
   
#include "StandardTypes.h"     
#include "CRel.h"
#include "TBlock.h"
#include "BinaryTuple.h"
#include "RectangleAlgebra.h"
#include "SpatialAttrArray.h"

using namespace std;
using namespace CRelAlgebra;

namespace csj {

  // test function
  void binaryTupleToScreen(binaryTuple bt, size_t dim) {
    cout<<"Tuple-Block number: "<<bt.blockNum
        <<" Row:"<<bt.row
        <<" xMin:"<<bt.xMin
        <<" xMax:"<<bt.xMax
        <<" yMin:"<<bt.yMin
        <<" yMax:"<<bt.yMax;
    if(dim == 3) {
      cout<<" zMin:"<<bt.zMin
          <<" zMax:"<<bt.xMax;
    }
    cout<<endl;
  }

  vector<binaryTuple> createSortBAT(const vector<TBlock*> &tBlockVector,
                             const uint64_t &joinIndex,
                             size_t dim) {
                               
    vector<binaryTuple> BAT;
    uint64_t tBlockNum = 1;
    binaryTuple temp;
    
    while(tBlockNum <= tBlockVector.size()) {

      TBlockIterator tBlockIter = tBlockVector[tBlockNum-1]->GetIterator();
      uint64_t row = 0;

      while(tBlockIter.IsValid()) {

        const TBlockEntry &tuple = tBlockIter.Get();
        temp.blockNum = tBlockNum;
        temp.row = row;

        if(dim == 2) {
          SpatialAttrArrayEntry<2> attribute = tuple[joinIndex];
          Rectangle<2> rec = attribute.GetBoundingBox();
          temp.xMin = rec.MinD(0);
          temp.xMax = rec.MaxD(0);
          temp.yMin = rec.MinD(1);
          temp.yMax = rec.MaxD(1);
        }
        else {
          SpatialAttrArrayEntry<3> attribute = tuple[joinIndex];
          Rectangle<3> rec = attribute.GetBoundingBox();
          temp.xMin = rec.MinD(0);
          temp.xMax = rec.MaxD(0);
          temp.yMin = rec.MinD(1);
          temp.yMax = rec.MaxD(1);
          temp.zMin = rec.MinD(2);
          temp.zMax = rec.MaxD(2);
        }

        BAT.push_back(temp);
        ++row;
        tBlockIter.MoveToNext();
      }
      ++tBlockNum;
    }

    return BAT;
  }

  

class SpatialJoinState {
  public: 
  // Constructor
  SpatialJoinState(const vector<TBlock*> &fTBlockVector_,
                   const vector<TBlock*> &sTBlockVector_,
                   uint64_t fIndex_,
                   uint64_t sIndex_,
                   uint64_t fNumTuples_,
                   uint64_t sNumTuples_,
                   uint64_t rTBlockSize_,
                   uint64_t numStripes_,
                   size_t fDim_,
                   size_t sDim_) :

                   fTBlockVector(fTBlockVector_),
                   sTBlockVector(sTBlockVector_),
                   fIndex(fIndex_),
                   sIndex(sIndex_),
                   fNumTuples(fNumTuples_),
                   sNumTuples(sNumTuples_),
                   rTBlockSize(rTBlockSize_),
                   fDim(fDim_),
                   sDim(sDim_),
                   fNumColumns(fTBlockVector[0]->GetColumnCount()),
                   sNumColumns(sTBlockVector[0]->GetColumnCount()),
                   fResumeIter(0),
                   sResumeIter(0),
                   resume(false),
                   newTuple(new AttrArrayEntry[fNumColumns+sNumColumns]) {
                     
    fBAT = createSortBAT(fTBlockVector, fIndex, fDim);
    sBAT = createSortBAT(sTBlockVector, sIndex, sDim);

    blockMask = (1ULL << 32) - 1;
    rowMask = (1ULL << 32) -1;
  }

  // Destructor                 
  ~SpatialJoinState() {
    
    delete newTuple;
    fBAT.clear();
    sBAT.clear();
  }

  bool nextTBlock(TBlock* ntb) {

    binaryTuple searchTuple;
    binaryTuple tempTuple;
    bool intersection;


    if(resume) {
      resume = false;
      sResumeIter++;
    }

    for(uint64_t f = fResumeIter; f < fBAT.size(); f++) {
      searchTuple = fBAT[f];
      for(uint64_t s = sResumeIter; s < sBAT.size(); s++) {
        tempTuple = sBAT[s];

        intersection = false;

        // intersection test
        intersection = ((searchTuple.xMin <= tempTuple.xMax)
                     && (searchTuple.xMax >= tempTuple.xMin));
        intersection = intersection &&
                       ((searchTuple.yMin <= tempTuple.yMax)
                       && (searchTuple.yMax >= tempTuple.yMin));
          
        if(intersection) {
          // save next result tuple in result tuple block
          const TBlockEntry &fTuple = TBlockEntry(
                fTBlockVector[(searchTuple.blockNum & blockMask) - 1],
                searchTuple.row & rowMask);

          const TBlockEntry &sTuple = TBlockEntry(
                sTBlockVector[(tempTuple.blockNum & blockMask) - 1],
                tempTuple.row & rowMask);

          for(uint64_t i = 0; i < fNumColumns; ++i) {
                newTuple[i] = fTuple[i];
          }

          for(uint64_t i = 0; i < sNumColumns; ++i) {
                 newTuple[fNumColumns + i] = sTuple[i];
          }

          ntb->Append(newTuple);

          if(ntb->GetSize() > rTBlockSize) {
            fResumeIter = f;
            sResumeIter = s;
            resume = true;
            return false;
          }          
        } // end of if intersection
      }
      sResumeIter = 0;
    }
    
    return true;
  } // end of nextTBlock()

  private:
  
  const vector<TBlock*> &fTBlockVector;
  const vector<TBlock*> &sTBlockVector;
  uint64_t fIndex;
  uint64_t sIndex;
  uint64_t fNumTuples;
  uint64_t sNumTuples;
  uint64_t rTBlockSize;
  vector<binaryTuple> fBAT;
  vector<binaryTuple> sBAT;
  size_t fDim;
  size_t sDim;
  const size_t fNumColumns;
  const size_t sNumColumns;


  uint64_t blockMask;
  uint64_t rowMask;
  uint64_t fResumeIter;
  uint64_t sResumeIter;
  bool resume;

  AttrArrayEntry* const newTuple; // AttrArrayEntry* const newTuple;

};
}
