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
  void BinaryTupleToScreen(binaryTuple bt, size_t dim) {
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
    cout<<'\n';
  }

  void MergeSortY(binaryTuple* bat, size_t numTuples) {

    size_t sizeIter;
    size_t blockIter;
    size_t lBlockIter;
    size_t rBlockIter;
    size_t mergeIter;
    size_t lBorder;
    size_t mBorder;
    size_t rBorder;
    
    for(sizeIter = 1; sizeIter < numTuples; sizeIter *= 2) {
      for(blockIter=0; blockIter<numTuples-sizeIter; blockIter += 2*sizeIter) {
        // We merge with the sorting of a pair of blocks starting
        // with the blockIter element
        // left has a size of sizeIter, right has a size of sizeIter or less
        lBlockIter = 0;
        rBlockIter = 0;
        lBorder = blockIter;
        mBorder = blockIter + sizeIter;
        rBorder = blockIter + 2*sizeIter;
        rBorder = (rBorder < numTuples) ? rBorder : numTuples;
        binaryTuple* SortedBlock = new binaryTuple[rBorder-lBorder];

        // While in both arrays there are elements we select the smaller
        // of them and put them in the sorted block
        while(lBorder+lBlockIter < mBorder && mBorder+rBlockIter < rBorder) {
          if(bat[lBorder+lBlockIter].yMin < bat[mBorder+rBlockIter].yMin) {
            SortedBlock[lBlockIter+rBlockIter] = bat[lBorder + lBlockIter];
            lBlockIter += 1;
          }
          else {
            SortedBlock[lBlockIter+rBlockIter] = bat[mBorder+rBlockIter];
            rBlockIter += 1;
          }
        }
        
        // After that, we enter the remaining elements
        // from the left or right block
        while(lBorder+lBlockIter < mBorder) {
          SortedBlock[lBlockIter+rBlockIter] = bat[lBorder+lBlockIter];
          lBlockIter += 1;
        }
        while(mBorder + rBlockIter < rBorder) {
          SortedBlock[lBlockIter+rBlockIter] = bat[mBorder+rBlockIter];
          rBlockIter += 1;
        }

        for(mergeIter = 0; mergeIter < lBlockIter + rBlockIter; mergeIter++) {
          bat[lBorder+mergeIter] = SortedBlock[mergeIter];
        }
        delete SortedBlock;
      }
    }
  }

  uint64_t spacePartitionX(binaryTuple* bat1,
                           binaryTuple* bat2,
                           const uint64_t numPartStripes,
                           double* xMin,
                           double* xMax) {

    uint64_t numTuplesBAT1 = sizeof(bat1)/sizeof(bat1[0]);
    uint64_t numTuplesBAT2 = sizeof(bat2)/sizeof(bat2[0]);
    uint64_t genTuplesCounter = numTuplesBAT1 > numTuplesBAT2 ?
                                numTuplesBAT1 : numTuplesBAT2;

    // compute xMin and xMax for currently binary tables
    for(uint64_t i=0; i < numTuplesBAT1; i++) {
      if(bat1[i].xMin < *xMin)
        *xMin = bat1[i].xMin;
      if(bat1[i].xMax > *xMax)
        *xMax = bat1[i].xMax;
    }
    
    for(uint64_t i=0; i < numTuplesBAT2; i++) {
      if(bat2[i].xMin < *xMin)
        *xMin = bat2[i].xMin;
      if(bat2[i].xMax > *xMax)
        *xMax = bat2[i].xMax;
    }

    // partition both binary tables 
    bool finished1 = false; // shows whether the bat1 has been completely edited
    bool finished2 = false; // shows whether the bat2 has been completely edited
    size_t factor = 1;
    uint64_t maxEntryPerBucket = 0;
    double stripeWidth = 0;
    uint64_t tempNumPartStripes = numPartStripes;
    binaryTuple** partBAT1; // temporary partition table for bat1
    binaryTuple** partBAT2; // temporary partition table for bat2
    uint64_t* bucketCounter1; // contains number of tuples in bucket i
    uint64_t* bucketCounter2; // contains number of tuples in bucket i
    uint64_t bucketNumberLeft; // first bucket where tuple is stored
    uint64_t bucketNumberRigth; // last bucket where tuple is stored
    uint64_t bucketPos; // contains next free position in bucket
    uint64_t numTuplesPart1; // number of all tuples in partBAT1
    uint64_t numTuplesPart2; // number of all tuples in partBAT2

    

    while(!finished1 && !finished2) {
      
      tempNumPartStripes = numPartStripes*factor;
      stripeWidth = (*xMax-*xMin)/tempNumPartStripes;
      maxEntryPerBucket = 2*(genTuplesCounter/tempNumPartStripes);

      // it must be at least two entry per bucket
      if(maxEntryPerBucket < 1) {
        maxEntryPerBucket = 2;
      }

      // Tables patition
      partBAT1 = new binaryTuple*[tempNumPartStripes];
      partBAT2 = new binaryTuple*[tempNumPartStripes];
      bucketCounter1 = new uint64_t[tempNumPartStripes];
      bucketCounter2= new uint64_t[tempNumPartStripes];
      bucketNumberLeft = 0;
      bucketNumberRigth = 0;
      bucketPos = 0;
      numTuplesPart1 = 0;
      numTuplesPart2 = 0;

      //initialization
      for(uint64_t i=0; i<tempNumPartStripes; i++) {
        bucketCounter1[i] = 0;
        bucketCounter2[i] = 0;
      }
      
      for(uint64_t i=0; i<genTuplesCounter; i++) {
        // if BAT1 is not empty
        if(i<numTuplesBAT1) {
          // if current bucket ist not full 
          if(bucketCounter1[i]<maxEntryPerBucket) {
            // compute a number of bucket left and rigth
            // and next free positon in  buckets
            // paste the tuple in buckets
            bucketNumberLeft = (bat1[i].xMin-*xMin)/stripeWidth;
            bucketNumberRigth = (bat1[i].xMax-*xMin)/stripeWidth;
            for(uint64_t j=bucketNumberLeft; j<=bucketNumberRigth; j++) {
              bucketPos = bucketCounter1[j];
              partBAT1[j][bucketPos] = bat1[i];
              // compute next free position in current bucket
              bucketCounter1[j]++;
              numTuplesPart1++;
            }
          }
          // current bucket is full
          // number of partitions must be increased
          else {
            delete[] partBAT1;
            delete[] partBAT2;
            delete[] bucketCounter1;
            delete[] bucketCounter2;
            factor *=2;
            break;
          }
        }
        // BAT1 is partitioned
        else {
          finished1 = true;
        }
        // if BAT2 is not empty
        if(i<numTuplesBAT2) {
          // if current bucket ist not full 
          if(bucketCounter2[i]<maxEntryPerBucket) {
            // compute a number of bucket left and rigth
            // and next free positon in  buckets
            // paste the tuple in buckets
            bucketNumberLeft = (bat2[i].xMin-*xMin)/stripeWidth;
            bucketNumberRigth = (bat2[i].xMax-*xMin)/stripeWidth;
            for(uint64_t j=bucketNumberLeft; j<=bucketNumberRigth; j++) {
              bucketPos = bucketCounter2[j];
              partBAT2[j][bucketPos] = bat2[i];
              // compute next free position in current bucket
              bucketCounter2[j]++;
              numTuplesPart2++;
            }
          }
          // current bucket is full
          // number of partitions must be increased
          else {
            delete[] partBAT1;
            delete[] partBAT2;
            delete[] bucketCounter1;
            delete[] bucketCounter2;
            factor *=2;
            break;
          }
        }
        // BAT2 is partitioned
        else {
          finished2 = true;
        }
      }
    }

  // save partBAT1 and partBAT2 in bat1 and bat2
  delete bat1;
  bat1 = nullptr;
  bat1 = new binaryTuple[numTuplesPart1];
  delete bat2;
  bat2 = nullptr;
  bat2 = new binaryTuple[numTuplesPart2];
  uint64_t posBAT1 = 0;
  uint64_t posBAT2 = 0;

  for(uint64_t i=0; i<tempNumPartStripes; i++) {
    // encode partition number
    uint64_t partNumber = (i+1)<<48;
    
    // BAT1
    uint64_t pos = bucketCounter1[i];
    // if bucket is not empty
    if(pos > 0) {
      // Run completely bucket and read all the tuples
      for(uint64_t k=0; k<pos; k++) {
        bat1[posBAT1] = partBAT1[i][k];
        // save partition number
        bat1[posBAT1].blockNum = bat1[posBAT1].blockNum | partNumber;
        // move to next free slot
        posBAT1++;
      }
    }
    // BAT2
    pos = bucketCounter2[i];
    // if bucket is not empty
    if(pos > 0) {
      // Run completely bucket and read all the tuples
      for(uint64_t k=0; k<pos; k++) {
        bat2[posBAT2] = partBAT2[i][k];
        // save partition number
        bat2[posBAT2].blockNum = bat2[posBAT2].blockNum | partNumber;
        // move to next free slot
        posBAT2++;
      }
    }
  }

  // end of partitions
  delete[] partBAT1;
  delete[] partBAT2;
  delete[] bucketCounter1;
  delete[] bucketCounter2;
  
  return tempNumPartStripes;
  }

  binaryTuple* createSortBAT(const vector<TBlock*> &tBlockVector,
                             const uint64_t joinIndex,
                             const uint64_t numTuples,
                             size_t dim) {
                               
    binaryTuple* BAT = new binaryTuple[numTuples];
    uint64_t tupleCounter = 0;
    uint64_t tBlockNum = 1;

    
    while(tBlockNum <= tBlockVector.size()) {

      TBlockIterator tBlockIter = tBlockVector[tBlockNum-1]->GetIterator();
      uint64_t row = 0;

      while(tBlockIter.IsValid()) {

        const TBlockEntry &tuple = tBlockIter.Get();
        BAT[tupleCounter].blockNum = tBlockNum;
        BAT[tupleCounter].row = row;

        if(dim == 2) {
          SpatialAttrArrayEntry<2> attribute = tuple[joinIndex];
          Rectangle<2> rec = attribute.GetBoundingBox();
          BAT[tupleCounter].xMin = rec.MinD(0);
          BAT[tupleCounter].xMax = rec.MaxD(0);
          BAT[tupleCounter].yMin = rec.MinD(1);
          BAT[tupleCounter].yMax = rec.MaxD(1);
        }
        else {
          SpatialAttrArrayEntry<3> attribute = tuple[joinIndex];
          Rectangle<3> rec = attribute.GetBoundingBox();
          BAT[tupleCounter].xMin = rec.MinD(0);
          BAT[tupleCounter].xMax = rec.MaxD(0);
          BAT[tupleCounter].yMin = rec.MinD(1);
          BAT[tupleCounter].yMax = rec.MaxD(1);
          BAT[tupleCounter].zMin = rec.MinD(2);
          BAT[tupleCounter].zMax = rec.MaxD(2);
        }

        ++tupleCounter;
        ++row;
        tBlockIter.MoveToNext();
      }
      ++tBlockNum;
    }
      
    // Test!!!!
    if(tupleCounter != numTuples)
           cout<<"tupleCounter != numTuples"<<'\n';
      
    // sort the binary table by Y-coordinate
    MergeSortY(BAT, numTuples);

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
                   numStripes(numStripes_),
                   fDim(fDim_),
                   sDim(sDim_),
                   fNumColumns(fTBlockVector[0]->GetColumnCount()),
                   sNumColumns(sTBlockVector[0]->GetColumnCount()),
                   newTuple(new AttrArrayEntry[fNumColumns+sNumColumns]) {

    fBAT = createSortBAT(fTBlockVector, fIndex, fNumTuples, fDim);
    sBAT = createSortBAT(sTBlockVector, sIndex, sNumTuples, sDim);
    numStripes = spacePartitionX(fBAT, sBAT, numStripes, xMin, xMax);
  }

  // Destructor                 
  ~SpatialJoinState() {
    delete[] fBAT;
    delete[] sBAT;
  }

  bool nextTBlock(TBlock* ntb) {
    
    list<binaryTuple> fSweepStruct;
    list<binaryTuple> sSweepStruct;

    uint64_t pos1 = 0;
    uint64_t pos2 = 0;

    // sweep-plane
    for(uint64_t tempPart = 0; tempPart < numStripes; tempPart++) {
      while(((fBAT[pos1].blockNum>>48)-1) == tempPart
            || ((sBAT[pos2].blockNum>>48)-1) == tempPart) {
          if(fBAT[pos1].yMin < sBAT[pos2].yMin) {
            fSweepStruct.push_back(fBAT[pos1]);
            sSweepStruct = sweepRemove(sSweepStruct, fBAT[pos1]);
            sweepSearch(sSweepStruct, ntb, fBAT[pos1], 1, tempPart);
            pos1++;
          }
          else {
            sSweepStruct.push_back(sBAT[pos2]);
            fSweepStruct = sweepRemove(fSweepStruct, sBAT[pos2]);
            sweepSearch(fSweepStruct, ntb, sBAT[pos2], 2, tempPart);
            pos2++;
          }      
        }
      // if actually part is not empty
      while(((fBAT[pos1].blockNum>>48)-1) == tempPart) {
        pos1++;
      }
      while(((sBAT[pos2].blockNum>>48)-1) == tempPart) {
        pos2++;
      }   
    }
    
    return true;
  }

  private:

  list<binaryTuple> sweepRemove(list<binaryTuple> sweepStruct, binaryTuple bt) {
    list<binaryTuple> tempList;
    binaryTuple tempTuple;
    
    for(list<binaryTuple>::iterator iter = sweepStruct.begin();
        iter != sweepStruct.end(); iter++) {
      tempTuple = *iter;
      if(tempTuple.yMax > bt.yMin) {
        tempList.push_back(tempTuple);
      }
    }
    sweepStruct.clear();
    return tempList;
  }

  void sweepSearch(list<binaryTuple> sweepStruct,
                   TBlock* ntb,
                   binaryTuple searchTuple,
                   int stream,
                   uint64_t part) {

    binaryTuple tempTuple;
    double x_overlap;
    double y_overlap;
    double stripeWidth = (*xMax-*xMin)/numStripes;

    // mask for decode block number
    // compute 0-16:1-48 bits
    uint64_t blockMask = (1ULL << 48) - 1;

    for(list<binaryTuple>::iterator iter = sweepStruct.begin();
        iter != sweepStruct.end(); iter++) {
          
      tempTuple = *iter;
      // intersection test
      x_overlap = (searchTuple.xMax < tempTuple.xMax ?
                   searchTuple.xMax : tempTuple.xMax) -
                  (searchTuple.xMin > tempTuple.xMin ?
                   searchTuple.xMin : tempTuple.xMin);
      x_overlap = 0 > x_overlap ? 0 : x_overlap;

      y_overlap = (searchTuple.yMax < tempTuple.yMax ?
                   searchTuple.yMax : tempTuple.yMax) -
                  (searchTuple.yMin > tempTuple.yMin ?
                   searchTuple.yMin : tempTuple.yMin);
      y_overlap = 0 > y_overlap ? 0 : y_overlap;
      
      // rectangle was processed in last part
      if((stripeWidth*part > searchTuple.xMin)
         && (stripeWidth*part > tempTuple.xMin)) {
           x_overlap = 0;
           y_overlap = 0;
      }
      
      //if intersection
      if(x_overlap*y_overlap != 0) {
        if(stream == 1) {
          const TBlockEntry &fTuple = TBlockEntry(
                fTBlockVector[(searchTuple.blockNum & blockMask) - 1],
                searchTuple.row);

          const TBlockEntry &sTuple = TBlockEntry(
                sTBlockVector[(tempTuple.blockNum & blockMask) - 1],
                tempTuple.row);

          for(uint64_t i = 0; i < fNumColumns; ++i) {
                newTuple[i] = fTuple[i];
          }

          for(uint64_t i = 0; i < sNumColumns; ++i) {
                newTuple[fNumColumns + i] = sTuple[i];
          }

          ntb->Append(newTuple);
        }
        else {
          const TBlockEntry &sTuple = TBlockEntry(
                sTBlockVector[(searchTuple.blockNum & blockMask) - 1],
                searchTuple.row);

          const TBlockEntry &fTuple = TBlockEntry(
                fTBlockVector[(tempTuple.blockNum & blockMask) - 1],
                tempTuple.row);

          for(uint64_t i = 0; i < fNumColumns; ++i) {
                newTuple[i] = fTuple[i];
          }

          for(uint64_t i = 0; i < sNumColumns; ++i) {
                newTuple[fNumColumns + i] = sTuple[i];
          }

          ntb->Append(newTuple);
        }
        
      }
    }
  }
  
  const vector<TBlock*> &fTBlockVector;
  const vector<TBlock*> &sTBlockVector;
  uint64_t fIndex;
  uint64_t sIndex;
  uint64_t fNumTuples;
  uint64_t sNumTuples;
  uint64_t rTBlockSize;
  binaryTuple* fBAT;
  binaryTuple* sBAT;
  uint64_t numStripes;
  size_t fDim;
  size_t sDim;
  
  double* xMin;
  double* xMax;

  const size_t fNumColumns;
  const size_t sNumColumns;

  AttrArrayEntry* const newTuple; // AttrArrayEntry* const newTuple;

};
}
