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

  bool spacePartitionX(binaryTuple* bat,
                       const uint64_t numPartStripes,
                       const uint64_t numTuples,
                       const double xMin,
                       const double xMax)                {

    double stripeWidth = (xMax-xMin)/numPartStripes;
    uint64_t maxEntryPerBucket = 2*(numTuples/numPartStripes);

    // it must be at least two entry per bucket
    if(maxEntryPerBucket < 1) {
      maxEntryPerBucket = 2;
    }

    // partition array
    binaryTuple** stripes = new binaryTuple*[numPartStripes];
    // array saves current position in bucket[i]
    uint64_t* bucketIter = new uint64_t[numPartStripes];
    // Initialization
    for(uint64_t i = 0; i < numPartStripes; i++) {
      bucketIter[i] = 0;
    }

    // partition BAT
    for(uint64_t i = 0; i < numTuples; i++) {
      // compute left stripe number
      uint64_t bucketNumberLeft = (bat[i].xMin-xMin)/stripeWidth;
      // compute right stripe number
      uint64_t bucketNumberRight = (bat[i].xMax-xMin)/stripeWidth;
      // entry current tuple in all buckets (stripes), that it intersects
      for(uint64_t p = bucketNumberLeft;
          p <= bucketNumberRight; p++) {
        // compute free position in current bucket    
        uint64_t pos = bucketIter[p];  
        // if current bucket is not full
        if(pos < maxEntryPerBucket) {
          stripes[p][pos] = bat[i];
          ++bucketIter[p];
        }
        else {
          // bucket is full, number of partition stripes must be increased
          return false;
        }
      }     
    }

    // delete the old BAT and create a new BAT with a new number of member
    delete bat;
    bat = nullptr;
    bat = new binaryTuple[2*numTuples];
    for(uint64_t i = 0; i < numPartStripes; i++) {
        for(uint64_t pos = 0; pos < maxEntryPerBucket; pos++) {
          // if the member with postion number pos exist
          // (bucketIter[i] denote a next free position in bucket i)
          if(pos < bucketIter[i]) {
            bat[i*maxEntryPerBucket + pos] = stripes[numPartStripes][pos];
          }
          else {
            // create binary tuple with block number 0
            // block number 0 not exist 
            bat[i*maxEntryPerBucket + pos] = binaryTuple();
          } 
        }
    }

    // the end of partitions
    return true;
  }

  binaryTuple* createPartBAT(const vector<TBlock*> tBlockVector,
                             const uint64_t joinIndex,
                             const uint64_t numTuples,
                             const uint64_t numPartStripes,
                             size_t dim) {
                               
    binaryTuple* BAT = new binaryTuple[numTuples];
    uint64_t tupleCounter = 0;
    uint64_t tBlockNum = 1;

    double xMin = 0;
    double xMax = 0;
    double yMin = 0;
    double yMax = 0;
    double zMin = 0;
    double zMax = 0;

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

          // compute general bounding box for currently tuples
          if(BAT[tupleCounter].xMin < xMin)
            xMin = BAT[tupleCounter].xMin;
          if(BAT[tupleCounter].xMax > xMax)
            xMax = BAT[tupleCounter].xMax;
          if(BAT[tupleCounter].yMin < yMin)
            yMin = BAT[tupleCounter].yMin;
          if(BAT[tupleCounter].yMax > yMax)
            yMax = BAT[tupleCounter].yMax;
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

          // compute general bounding box for currently tuples
          if(BAT[tupleCounter].xMin < xMin)
            xMin = BAT[tupleCounter].xMin;
          if(BAT[tupleCounter].xMax > xMax)
            xMax = BAT[tupleCounter].xMax;
          if(BAT[tupleCounter].yMin < yMin)
            yMin = BAT[tupleCounter].yMin;
          if(BAT[tupleCounter].yMax > yMax)
            yMax = BAT[tupleCounter].yMax;
          if(BAT[tupleCounter].zMin < zMin)
            zMin = BAT[tupleCounter].zMin;
          if(BAT[tupleCounter].yMax > zMax)
            zMax = BAT[tupleCounter].zMax;
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
    for(uint64_t i=0; i<numTuples; i++)
      BinaryTupleToScreen(BAT[i], dim);

    MergeSortY(BAT, numTuples);

    // Test!!!
    for(uint64_t i=0; i<numTuples; i++)
      BinaryTupleToScreen(BAT[i], dim);

    // partitions phase
    uint64_t factor = 1;
    while(!spacePartitionX(BAT, numPartStripes*factor,
                           numTuples, xMin, xMax))     {
    // increase number of partitions
      factor *= 2;                         
    }

    return BAT;
  }

class SpatialJoinState {
  public: 
  // Constructor
  SpatialJoinState(const vector<TBlock*> fTBlockVector,
                   const vector<TBlock*> sTBlockVector,
                   uint64_t fIndex,
                   uint64_t sIndex,
                   uint64_t fNumTuples,
                   uint64_t sNumTuples,
                   uint64_t rTBlockSize) {}

  // Destructor                 
  ~SpatialJoinState() {}

  bool nextTBlock(TBlock* ntb) {
    return true;
  }

};
}
