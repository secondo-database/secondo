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

  void MergeSortY(vector<binaryTuple> &bat) {

    uint64_t sizeIter;
    uint64_t blockIter;
    uint64_t lBlockIter;
    uint64_t rBlockIter;
    uint64_t mergeIter;
    uint64_t lBorder;
    uint64_t mBorder;
    uint64_t rBorder;
    uint64_t numTuples = bat.size();
    
    
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
        vector<binaryTuple> SortedBlock(rBorder-lBorder);

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
        SortedBlock.clear();
      }
    }
  }

  uint64_t spacePartitionX(vector<binaryTuple> &bat1,
                           vector<binaryTuple> &bat2,
                           const uint64_t &numPartStripes,
                           double &xMin,
                           double &xMax) {

    
    
    size_t factor = 1;
    double stripeWidth = 0;
    uint64_t tempNumPartStripes = numPartStripes;
    uint64_t numTuplesBAT1 = bat1.size();
    uint64_t numTuplesBAT2 = bat2.size();
    binaryTuple** partBAT1; // temporary partition table for bat1
    binaryTuple** partBAT2; // temporary partition table for bat2
    uint64_t* bucketCounter1; // contains number of tuples in bucket i
    uint64_t* bucketCounter2; // contains number of tuples in bucket i
    uint64_t bucketNumberLeft; // first bucket where tuple is stored
    uint64_t bucketNumberRigth; // last bucket where tuple is stored
    uint64_t bucketPos; // contains next free position in bucket
    uint64_t numTuplesPart1; // number of all tuples in partBAT1
    uint64_t numTuplesPart2; // number of all tuples in partBAT2


    // compute xMin and xMax for currently binary tables
    for(uint64_t i=0; i < numTuplesBAT1; i++) {
      if(bat1[i].xMin < xMin)
        xMin = bat1[i].xMin;
      if(bat1[i].xMax > xMax)
        xMax = bat1[i].xMax;
    }
    
    for(uint64_t i=0; i < numTuplesBAT2; i++) {
      if(bat2[i].xMin < xMin)
        xMin = bat2[i].xMin;
      if(bat2[i].xMax > xMax)
        xMax = bat2[i].xMax;
    }

    // partition both binary tables 
      
      tempNumPartStripes = numPartStripes*factor;
      stripeWidth = (xMax - xMin)/tempNumPartStripes;

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
        partBAT1[i] = new binaryTuple[numTuplesBAT1];
        partBAT2[i] = new binaryTuple[numTuplesBAT2];
      }
      
      for(uint64_t i=0; i<numTuplesBAT1; i++) {
          // compute a number of bucket left and rigth
          // and next free positon in  buckets
          // paste the tuple in buckets
          bucketNumberLeft = (bat1[i].xMin-xMin)/stripeWidth;
          bucketNumberRigth = (bat1[i].xMax-xMin)/stripeWidth;
          // while xMax is equal Reck.xMax from BAT
          if(bucketNumberRigth == tempNumPartStripes) {
            bucketNumberRigth = tempNumPartStripes-1;
          }
          for(uint64_t j=bucketNumberLeft; j<=bucketNumberRigth; j++) {
            bucketPos = bucketCounter1[j];
            partBAT1[j][bucketPos] = bat1[i];
            // compute next free position in current bucket
            bucketCounter1[j]++;
            numTuplesPart1++;
          }
        }

        for(uint64_t i=0; i<numTuplesBAT2; i++) {
          // compute a number of bucket left and rigth
          // and next free positon in  buckets
          // paste the tuple in buckets
          bucketNumberLeft = (bat2[i].xMin-xMin)/stripeWidth;
          bucketNumberRigth = (bat2[i].xMax-xMin)/stripeWidth;
          // while xMax is equal Reck.xMax from BAT
          if(bucketNumberRigth == tempNumPartStripes) {
            bucketNumberRigth = tempNumPartStripes-1;
          }
          for(uint64_t j=bucketNumberLeft; j<=bucketNumberRigth; j++) {
            bucketPos = bucketCounter2[j];
            partBAT2[j][bucketPos] = bat2[i];
            // compute next free position in current bucket
            bucketCounter2[j]++;
            numTuplesPart2++;
          }
        }

  // save partBAT1 and partBAT2 in bat1 and bat2
  uint64_t posBAT1 = 0;
  uint64_t posBAT2 = 0;
  uint64_t partNumber = 0;

  // BAT1
  bat1.clear();
  for(uint64_t i=0; i<tempNumPartStripes; i++) {
    // encode partition number
    partNumber = i<<48;
  
    bucketPos = bucketCounter1[i];
    // if bucket is not empty
    if(bucketPos > 0) {
      // Run completely bucket and read all the tuples
      for(uint64_t k=0; k<bucketPos; k++) {
        bat1.push_back(partBAT1[i][k]);
        // save partition number
        bat1[posBAT1].blockNum = bat1[posBAT1].blockNum | partNumber;
        // move to next free slot
        
        posBAT1++;
      }
    }
  }

  // BAT2
  bat2.clear();
  for(uint64_t i=0; i<tempNumPartStripes; i++) {
    // encode partition number
    partNumber = i<<48;
  
    bucketPos = bucketCounter2[i];
    // if bucket is not empty
    if(bucketPos > 0) {
      // Run completely bucket and read all the tuples
      for(uint64_t k=0; k<bucketPos; k++) {
        bat2.push_back(partBAT2[i][k]);
        // save partition number
        bat2[posBAT2].blockNum = bat2[posBAT2].blockNum | partNumber;
        // move to next free slot

        posBAT2++;
      }
    }
  }

  // free memory
  for(uint64_t i=0; i<tempNumPartStripes; i++) {
    delete[] partBAT1[i];
    delete[] partBAT2[i];
  }
  delete[] partBAT1;
  delete[] partBAT2;
  delete[] bucketCounter1;
  delete[] bucketCounter2;
  
  return tempNumPartStripes;
  }

  vector<binaryTuple> createSortBAT(const vector<TBlock*> &tBlockVector,
                             const uint64_t &joinIndex,
                             size_t dim) {
                               
    vector<binaryTuple> BAT;
    uint64_t tupleCounter = 0;
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
        ++tupleCounter;
        ++row;
        tBlockIter.MoveToNext();
      }
      ++tBlockNum;
    }
      
    // sort the binary table by Y-coordinate
    MergeSortY(BAT);


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
                   xMin(0),
                   xMax(0),
                   fNumColumns(fTBlockVector[0]->GetColumnCount()),
                   sNumColumns(sTBlockVector[0]->GetColumnCount()),
                   newTuple(new AttrArrayEntry[fNumColumns+sNumColumns]) {
                     
    fBAT = createSortBAT(fTBlockVector, fIndex, fDim);
    sBAT = createSortBAT(sTBlockVector, sIndex, sDim);
    numStripes = spacePartitionX(fBAT, sBAT, numStripes,
                                 xMin, xMax);
  }

  // Destructor                 
  ~SpatialJoinState() {
    fBAT.clear();
    sBAT.clear();
  }

  bool nextTBlock(TBlock* ntb) {
    
    list<binaryTuple> fSweepStruct;
    list<binaryTuple> sSweepStruct;

    uint64_t pos1 = 0;
    uint64_t pos2 = 0;
    uint sizeBAT1 = fBAT.size();
    uint64_t sizeBAT2 = sBAT.size();
    uint64_t currentPart1 = 0;
    uint64_t currentPart2 = 0;
    vector<binaryTuple> partBAT1;
    vector<binaryTuple> partBAT2;
    
    // plane-sweep over all parts
    for(uint64_t tempPart = 0; tempPart < numStripes; tempPart++) {

    // if partitioned binary table are not empty
    if(pos1<sizeBAT1) {
      currentPart1 = fBAT[pos1].blockNum>>48;
    }
    else {
      return false;
    }
    if(pos2<sizeBAT2) {
      currentPart2 = sBAT[pos2].blockNum>>48;
    }
    else {
      return false;
    }

      // read parts from partitioned binary tables
      // until boundary of part or of array is not arive
      while(currentPart1 == tempPart) {
        partBAT1.push_back(fBAT[pos1]);
        pos1++;
        if(pos1<sizeBAT1) {
          currentPart1 = fBAT[pos1].blockNum>>48;
        }
        else {
          break;
        }
      }
      
      while(currentPart2 == tempPart) {
        partBAT2.push_back(sBAT[pos2]);
        pos2++;
        if(pos2<sizeBAT2) {
          currentPart2 = sBAT[pos2].blockNum>>48;
        }
        else {
          break;
        }
      }

      // prepade to go to next partition
      if(partBAT1.empty()) {
        pos1++;
      }
      if(partBAT2.empty()) {
        pos2++;
      }

      // make plane sweep over actually parts
      //if both parts are not empty
      if(!partBAT1.empty() && !partBAT2.empty()) {
        uint64_t partPos1 = 0;
        uint64_t partPos2 = 0;
        uint64_t partSize1 = partBAT1.size();
        uint64_t partSize2 = partBAT2.size();
        
        while(!(partPos1 == partSize1) && !(partPos2 == partSize2)) {
          // move plane sweep line
          if(partBAT1[partPos1].yMin < partBAT2[partPos2].yMin) {
            // insert actually object in the first plane sweep struct
            fSweepStruct.push_back(partBAT1[partPos1]);
            // remove 'carbage'-objects from the second plane sweep struct
            sSweepStruct = sweepRemove(sSweepStruct, partBAT1[partPos1]);
            // search for join in the second plane sweep struct
            sweepSearch(sSweepStruct, ntb, partBAT1[partPos1], 1, tempPart);
            // move to next object
            partPos1++;
          }
          else {
            // insert actually object in the second plane sweep struct
            sSweepStruct.push_back(partBAT2[partPos2]);
            // remove 'carbage'-objects from the first plane sweep struct
            fSweepStruct = sweepRemove(fSweepStruct, partBAT2[partPos2]);
            // search for join in the first plane sweep struct
            sweepSearch(fSweepStruct, ntb, partBAT2[partPos2], 2, tempPart);
            // move to next object
            partPos2++;
          }
        }
        // additional test for including rectangle
        // move plane sweep line over the empty struct
        if(!(partPos1 == partSize1)) {
          while(!sSweepStruct.empty() && !(partPos1 == partSize1)) {
            sSweepStruct = sweepRemove(sSweepStruct, partBAT1[partPos1]);
            // search for join in the second plane sweep struct
            if(!sSweepStruct.empty()) {
              sweepSearch(sSweepStruct, ntb, partBAT1[partPos1], 1, tempPart);
            }
            // move to next object
            partPos1++;
          }
        }
        if(!(partPos2 == partSize2) && !(partPos2 == partSize2)) {
          while(!fSweepStruct.empty()) {
            fSweepStruct = sweepRemove(fSweepStruct, partBAT2[partPos2]);
            // search for join in the first plane sweep struct
            if(!fSweepStruct.empty()) {
              sweepSearch(fSweepStruct, ntb, partBAT2[partPos2], 1, tempPart);
            }
            // move to next object
            partPos2++;
          }
        }
      }
      // free memory
      fSweepStruct.clear();
      sSweepStruct.clear();
      partBAT1.clear();
      partBAT2.clear();
      
    } // end for()
    
    return true;
  }

  private:

  list<binaryTuple> sweepRemove(list<binaryTuple> &sweepStruct,
                                binaryTuple bt) {
    list<binaryTuple> tempList;
    binaryTuple tempTuple;
    
    for(list<binaryTuple>::iterator iter = sweepStruct.begin();
        iter != sweepStruct.end(); iter++) {
      tempTuple = *iter;
      if(tempTuple.yMax >= bt.yMin) {
        tempList.push_back(tempTuple);
      }
    }
    sweepStruct.clear();
    return tempList;
  }

  void sweepSearch(list<binaryTuple> &sweepStruct,
                   TBlock* ntb,
                   binaryTuple searchTuple,
                   int stream,
                   uint64_t part) {

    binaryTuple tempTuple;
    bool intersection = false;
   //double stripeWidth = (xMax-xMin)/numStripes;

    // mask for decode block number
    // compute 0-16:1-48 bits
    uint64_t blockMask = (1ULL << 48) - 1;

    for(list<binaryTuple>::iterator iter = sweepStruct.begin();
        iter != sweepStruct.end(); iter++) {
          
      tempTuple = *iter;
      // intersection test
      intersection = ((searchTuple.xMin <= tempTuple.xMax)
                  && (searchTuple.xMax >= tempTuple.xMin));
      
      // rectangle was processed in last part
     // if((searchTuple.xMin > (xMin + stripeWidth*part))
      //   && (tempTuple.xMin > (xMin + stripeWidth*part))) {
       //    intersection = false;
     // }

      //if intersection

      if(intersection) {
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
  vector<binaryTuple> fBAT;
  vector<binaryTuple> sBAT;
  uint64_t numStripes;
  size_t fDim;
  size_t sDim;
  
  double xMin;
  double xMax;

  const size_t fNumColumns;
  const size_t sNumColumns;

  AttrArrayEntry* const newTuple; // AttrArrayEntry* const newTuple;

};
}
