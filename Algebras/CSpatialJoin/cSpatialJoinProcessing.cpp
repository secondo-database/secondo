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


  void firstPartitionX(vector<binaryTuple> &bat,
                       vector<vector<binaryTuple>> &partBAT,
                       vector<uint64_t> &bucketCounter,
                       uint64_t &numPartStripes,
                       double &xMin,
                       double &xMax) {

    double stripeWidth = 0;
    
    uint64_t numTuplesBAT = bat.size();
    uint64_t bucketNumberLeft; // first bucket where tuple is stored
    uint64_t bucketNumberRigth; // last bucket where tuple is stored

    // partition both binary tables 

    stripeWidth = (xMax - xMin)/numPartStripes;

    // Tables patition
    bucketNumberLeft = 0;
    bucketNumberRigth = 0;

    //initialization
    for(uint64_t i=0; i<numPartStripes; i++) {
      bucketCounter.push_back(0);
      vector<binaryTuple> temp;
      partBAT.push_back(temp);
    }
      
    for(uint64_t i=0; i<numTuplesBAT; i++) {
      // compute a number of bucket left and rigth
      // and next free positon in  buckets
      // paste the tuple in buckets
      if(bat[i].xMin < xMin) {
        bucketNumberLeft = 0; // used by further partitions
      }
      else {
        bucketNumberLeft = (bat[i].xMin-xMin)/stripeWidth;
      }
      if(bat[i].xMax >= xMax) {
        bucketNumberRigth = numPartStripes-1; // used by further partitions
      }
      else {
        bucketNumberRigth = (bat[i].xMax-xMin)/stripeWidth;
      }

      for(uint64_t j=bucketNumberLeft; j<=bucketNumberRigth; j++) {
        partBAT[j].push_back(bat[i]);
        // compute next free position in current bucket
        bucketCounter[j]++;
      }
    }
  
  }

  uint64_t finalPartitionX(vector<vector<binaryTuple>> &pb1,
                       vector<vector<binaryTuple>> &pb2,
                       vector<binaryTuple> &b1,
                       vector<binaryTuple> &b2,
                       vector<uint64_t> &bc1,
                       vector<uint64_t> &bc2,
                       vector<double> &min,
                       uint64_t &stripes,
                       uint64_t &maxEntry,
                       uint64_t outPartNumber,
                       double &xMin,
                       double &xMax,
                       uint64_t divideFactor,
                       uint64_t partLevel) {

    uint64_t bucketPos1 = 0; // contains next free position in bucket
    uint64_t bucketPos2 = 0; // contains next free position in bucket
    uint64_t partNumber = outPartNumber;
    double tempXMin;
    double tempXMax;
                       
    for(uint64_t i=0; i<stripes; i++) {

      bucketPos1 = bc1[i];
      bucketPos2 = bc2[i];
     
      // if none from both currently buckets is overload
      // and none from both is not empty
      if((bucketPos1 < maxEntry) && (bucketPos2 < maxEntry)
        && ((bucketPos1 > 0) && (bucketPos2 > 0))) {
        // save x-min for actually partition 
        min.push_back(xMin);
        // write bat1
        // if bucket in partitioned table 1 is not empty
        if(bucketPos1 > 0) {
          // Run completely bucket and read all the tuples
          for(uint64_t k=0; k<bucketPos1; k++) {
            b1.push_back(pb1[i][k]);
            // save bucket and partitions level
            b1.back().blockNum=b1.back().blockNum | ((i | (partLevel<<24))<<32);
            // save partition number
            b1.back().row = b1.back().row | (partNumber<<32);
          }
        }
        // if bucket in partitioned table 2 is not empty
        if(bucketPos2 > 0) {
          // Run completely bucket and read all the tuples
          for(uint64_t k=0; k<bucketPos2; k++) {
            b2.push_back(pb2[i][k]);
            b2.back().blockNum=b2.back().blockNum | ((i | (partLevel<<24))<<32);
            // save partition number
            b2.back().row = b2.back().row | (partNumber<<32);
          }
        }
        partNumber++;
      } // end of if none from both currently buckets is overload
      // if one of current buckets has more entries than maxEntryperBucket
      else {
        // Create temporary structures for additional partition
        // and partition currently buckets in both tables
        // both buckets don't must be empty
        if((bucketPos1 > 0) && (bucketPos2 > 0)) {
          vector<vector<binaryTuple>> tempPB1;
          vector<vector<binaryTuple>> tempPB2;
          vector<uint64_t> tempBC1;
          vector<uint64_t> tempBC2;

          // compute temporary X-Min and X-Max
          tempXMin = xMin + ((xMax - xMin)/stripes)*i;
          tempXMax = xMin + ((xMax - xMin)/stripes)*(i+1);

          // partition currently buckets
          firstPartitionX(pb1[i], tempPB1, tempBC1,
                            divideFactor, tempXMin, tempXMax);
          firstPartitionX(pb2[i], tempPB2, tempBC2,
                            divideFactor, tempXMin, tempXMax);
          pb1[i].clear();
          pb2[i].clear();

          // put distributed buckets in original table
          partNumber = finalPartitionX(tempPB1, tempPB2, b1, b2,
                          tempBC1, tempBC2,
                          min, divideFactor, maxEntry,
                          partNumber, tempXMin, tempXMax,
                          divideFactor, partLevel+1);

          // free memory
          for(uint64_t i=0; i<divideFactor; i++) {
            tempPB1[i].clear();
            tempPB2[i].clear();
          }
          
          
          tempPB1.clear();
          tempPB2.clear();
          tempBC1.clear();
          tempBC2.clear();
        }
         
      } // end of if one of current buckets
        //has more entries than maxEntryperBucket
    } // end of for-loop

    return partNumber;
  }

  uint64_t spacePartitionX(vector<binaryTuple> &bat1,
                           vector<binaryTuple> &bat2,
                           vector<double> &min,
                           uint64_t &numPartStripes,
                           uint64_t maxEntryPerBucket,
                           double &xMin,
                           double &xMax,
                           uint64_t &divideFactor) {

    uint64_t tempNumPartStripes = numPartStripes;
    vector<vector<binaryTuple>> partBAT1; // temporary partition table for bat1
    vector<vector<binaryTuple>> partBAT2; // temporary partition table for bat2
    vector<uint64_t> bucketCounter1; // contains number of tuples in bucket i
    vector<uint64_t> bucketCounter2; // contains number of tuples in bucket i


    // partition both binary tables for the first time
    firstPartitionX(bat1, partBAT1, bucketCounter1,
                    numPartStripes, xMin, xMax);
    firstPartitionX(bat2, partBAT2, bucketCounter2,
                    numPartStripes, xMin, xMax);    

    // BAT1 and BAT2
    bat1.clear();
    bat2.clear();

    tempNumPartStripes = finalPartitionX(partBAT1, partBAT2,
                                         bat1, bat2,
                                         bucketCounter1,
                                         bucketCounter2,
                                         min, numPartStripes,
                                         maxEntryPerBucket,
                                         0, xMin, xMax, divideFactor, 0);

    // free memory
    for(uint64_t i=0; i<numPartStripes; i++) {
      partBAT1[i].clear();
      partBAT2[i].clear();
    }
    
    partBAT1.clear();
    partBAT2.clear();
    bucketCounter1.clear();
    bucketCounter2.clear();

    return tempNumPartStripes;
  }

  vector<binaryTuple> createSortBAT(const vector<TBlock*> &tBlockVector,
                             const uint64_t &joinIndex,
                             double &xMin,
                             double &xMax,
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

      // compute xMin and xMax for currently binary tables
      if(temp.xMin < xMin)
        xMin = temp.xMin;
      if(temp.xMax > xMax)
        xMax = temp.xMax;

        BAT.push_back(temp);
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
                   uint64_t maxEntryPerBucket_,
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
                   maxEntryPerBucket(maxEntryPerBucket_),
                   beginNumStripes(numStripes_),
                   fDim(fDim_),
                   sDim(sDim_),
                   xMin(0),
                   xMax(0),
                   fNumColumns(fTBlockVector[0]->GetColumnCount()),
                   sNumColumns(sTBlockVector[0]->GetColumnCount()),
                   pos1(0),
                   pos2(0),
                   resumePart(0),
                   resumeIter(0),
                   resume(false),
                   partPos1(0),
                   partPos2(0),
                   partSize1(0),
                   partSize2(0),
                   newTuple(new AttrArrayEntry[fNumColumns+sNumColumns]) {
                     
    fBAT = createSortBAT(fTBlockVector, fIndex, xMin, xMax, fDim);
    sBAT = createSortBAT(sTBlockVector, sIndex, xMin, xMax, sDim);


    beginNumStripes = ((fBAT.size() + sBAT.size())*2)/maxEntryPerBucket;
    if(beginNumStripes == 0) {
      beginNumStripes = 1;
    }
    numStripes = beginNumStripes;
    // factor used by computing of number of parts by additional partition
    divideFactor = beginNumStripes/2;
    if(divideFactor < 2) {
      divideFactor = 2;
    }
    
    if(numStripes == 0) {
      numStripes = 1;
    }

    numStripes = spacePartitionX(fBAT, sBAT, min, numStripes,
                                 maxEntryPerBucket, xMin, xMax, divideFactor);
                                 
    sizeBAT1 = fBAT.size();
    sizeBAT2 = sBAT.size();
  }

  // Destructor                 
  ~SpatialJoinState() {
    
    delete newTuple;
    fBAT.clear();
    sBAT.clear();
    fSweepStruct.clear();
    sSweepStruct.clear();
    partBAT1.clear();
    partBAT2.clear();
  }

  bool nextTBlock(TBlock* ntb) {

    // plane-sweep over all parts
    for(uint64_t tempPart = resumePart; tempPart < numStripes; tempPart++) {
      // if not new tuple block
      if(!resume) {
        // read parts from partitioned binary tables
        // until boundary of part or of array is not arive
        while((pos1 < sizeBAT1) && ((fBAT[pos1].row>>32) == tempPart)) {
          partBAT1.push_back(fBAT[pos1]);
          pos1++;
        }

        while((pos2 < sizeBAT2) && ((sBAT[pos2].row>>32) == tempPart)) {
          partBAT2.push_back(sBAT[pos2]);
          pos2++;
        }
      } // end if(!resume)
      
      // make plane sweep over actually parts
      //if both parts are not empty
        if(!resume) {
          partPos1 = 0;
          partPos2 = 0;
          partSize1 = partBAT1.size();
          partSize2 = partBAT2.size();
        }
        
        while(!(partPos1 == partSize1) && !(partPos2 == partSize2)) {
          // move plane sweep line
          if(partBAT1[partPos1].yMin < partBAT2[partPos2].yMin) {
            if(!resume) {
              // insert actually object in the first plane sweep struct
              fSweepStruct.push_back(partBAT1[partPos1]);
              // remove 'carbage'-objects from the second plane sweep struct
              sSweepStruct = sweepRemove(sSweepStruct, partBAT1[partPos1]);
              // search for join in the second plane sweep struct
            }
            if(!sweepSearch(sSweepStruct, ntb,
                            partBAT1[partPos1], 1, tempPart)) {
              return false;
            }
            // move to next object
            partPos1++;
          }
          else {
            if(!resume) {
              // insert actually object in the second plane sweep struct
              sSweepStruct.push_back(partBAT2[partPos2]);
              // remove 'carbage'-objects from the first plane sweep struct
              fSweepStruct = sweepRemove(fSweepStruct, partBAT2[partPos2]);
              // search for join in the first plane sweep struct
            }
            if(!sweepSearch(fSweepStruct, ntb,
                            partBAT2[partPos2], 2, tempPart)) {
              return false;
            }
            // move to next object
            partPos2++;
          }
        } // end of while
        
        // additional test for including rectangle
        // move plane sweep line over the struct
        if(!(partPos1 == partSize1)) {
          while(!sSweepStruct.empty() && !(partPos1 == partSize1)) {
            if(!resume) {
              sSweepStruct = sweepRemove(sSweepStruct, partBAT1[partPos1]);
            }
            // search for join in the second plane sweep struct
            if(!sSweepStruct.empty()) {
              if(!sweepSearch(sSweepStruct, ntb,
                              partBAT1[partPos1], 1, tempPart)) {
                return false;
              }
            }
            // move to next object
            partPos1++;
          }
        }
        if(!(partPos2 == partSize2)) {
          while(!fSweepStruct.empty() && !(partPos2 == partSize2)) {
             if(!resume) {
               fSweepStruct = sweepRemove(fSweepStruct, partBAT2[partPos2]);
             }
            // search for join in the first plane sweep struct
            if(!fSweepStruct.empty()) {
              if(!sweepSearch(fSweepStruct, ntb,
                              partBAT2[partPos2], 2, tempPart)) {
                return false;
              }
            }
            // move to next object
            partPos2++;
          }
        }
      // free memory
      fSweepStruct.clear();
      sSweepStruct.clear();
      partBAT1.clear();
      partBAT2.clear();
      resumePart++;
    } // end for()
    
    return true;
  } // end of nextTBlock()

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

  bool sweepSearch(list<binaryTuple> &sweepStruct,
                   TBlock* ntb,
                   binaryTuple searchTuple,
                   int stream,
                   uint64_t part) {

    binaryTuple tempTuple;
    bool intersection = false;
    double stripeWidth = (xMax - xMin)/beginNumStripes;
    double tempXMin = xMin;
    uint64_t bucketNumberMask = (1ULL << 24) - 1;
    uint64_t partLevel = searchTuple.blockNum >> 56; // >>32 and >>24
    uint64_t bucketNumber = (searchTuple.blockNum >> 32) & bucketNumberMask;

    // mask for decode block number
    // compute 0-16:1-48 bits
    uint64_t blockMask = (1ULL << 32) - 1;
    // mask to decode row number
    uint64_t rowMask = (1ULL << 32) -1;

    // if original stripe was redistributed
    if(partLevel != 0) {
      for(uint64_t i=0; i<partLevel; i++) {
        stripeWidth = stripeWidth/divideFactor;
      }
      tempXMin = min[part];
    }

    if(!resume) {
      resumeIter = sweepStruct.begin();
    }
    else {
      resumeIter++;
      resume = false;
    }

    for(list<binaryTuple>::iterator iter = resumeIter;
        iter != sweepStruct.end(); iter++) {
          
      tempTuple = *iter;

        // if both tuple was processed in last part
        if((searchTuple.xMin < (tempXMin + stripeWidth*bucketNumber))
          && (tempTuple.xMin < (tempXMin + stripeWidth*bucketNumber))) {
            intersection = false;
        }
        else {
          // intersection test
          intersection = ((searchTuple.xMin <= tempTuple.xMax)
                      && (searchTuple.xMax >= tempTuple.xMin));
        }
        
      if(intersection) {

        // save next result tuple in result tuple block
        
        if(stream == 1) {
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
        }
        else {
          const TBlockEntry &sTuple = TBlockEntry(
                sTBlockVector[(searchTuple.blockNum & blockMask) - 1],
                searchTuple.row & rowMask);

          const TBlockEntry &fTuple = TBlockEntry(
                fTBlockVector[(tempTuple.blockNum & blockMask) - 1],
                tempTuple.row & rowMask);

          for(uint64_t i = 0; i < fNumColumns; ++i) {
                newTuple[i] = fTuple[i];
          }

          for(uint64_t i = 0; i < sNumColumns; ++i) {
                newTuple[fNumColumns + i] = sTuple[i];
          }

          ntb->Append(newTuple);
        }

        if(ntb->GetSize() > rTBlockSize) {
          resume = true;
          return false;
        }
        
      }
    resumeIter++;
    }

  return true;
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
  vector<double> min;
  uint64_t maxEntryPerBucket;
  uint64_t beginNumStripes;
  uint64_t numStripes;
  size_t fDim;
  size_t sDim;
  
  double xMin;
  double xMax;

  const size_t fNumColumns;
  const size_t sNumColumns;

  uint64_t pos1;
  uint64_t pos2;
  uint sizeBAT1;
  uint64_t sizeBAT2;
  uint64_t resumePart;
  list<binaryTuple>::iterator resumeIter;
  vector<binaryTuple> partBAT1;
  vector<binaryTuple> partBAT2;
  list<binaryTuple> fSweepStruct;
  list<binaryTuple> sSweepStruct;
  bool resume;

  uint64_t partPos1;
  uint64_t partPos2;
  uint64_t partSize1;
  uint64_t partSize2;
  uint64_t divideFactor;

  AttrArrayEntry* const newTuple; // AttrArrayEntry* const newTuple;

};
}
