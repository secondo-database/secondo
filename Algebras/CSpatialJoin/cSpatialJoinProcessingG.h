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
#include "EventList.h"
#include "Grid.h"
#include "RectangleAlgebra.h"
#include "SpatialAttrArray.h"

namespace csj {

  void firstPartitionX(std::vector<binaryTuple> &bat,
                       std::vector<std::vector<binaryTuple>> &partBAT,
                       std::vector<uint64_t> &bucketCounter,
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
      std::vector<binaryTuple> temp;
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

  uint64_t finalPartitionX(std::vector<std::vector<binaryTuple>> &pb1,
                       std::vector<std::vector<binaryTuple>> &pb2,
                       std::vector<binaryTuple> &b1,
                       std::vector<binaryTuple> &b2,
                       std::vector<uint64_t> &bc1,
                       std::vector<uint64_t> &bc2,
                       std::vector<double> &min,
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
        // Run completely bucket and read all the tuples
        for(uint64_t k=0; k<bucketPos1; k++) {
          b1.push_back(pb1[i][k]);
          // save bucket and partitions level
          b1.back().blockNum=b1.back().blockNum | ((i | (partLevel<<24))<<32);
          // save partition number
          b1.back().row = b1.back().row | (partNumber<<32);
        }
        // Run completely bucket and read all the tuples
        for(uint64_t k=0; k<bucketPos2; k++) {
          b2.push_back(pb2[i][k]);
          b2.back().blockNum=b2.back().blockNum | ((i | (partLevel<<24))<<32);
          // save partition number
          b2.back().row = b2.back().row | (partNumber<<32);
        }
        partNumber++;
      } // end of if none from both currently buckets is overload
      // if one of current buckets has more entries than maxEntryperBucket
      else {
        // Create temporary structures for additional partition
        // and partition currently buckets in both tables
        // both buckets don't must be empty
        if((bucketPos1 > 0) && (bucketPos2 > 0)) {
          std::vector<std::vector<binaryTuple>> tempPB1;
          std::vector<std::vector<binaryTuple>> tempPB2;
          std::vector<uint64_t> tempBC1;
          std::vector<uint64_t> tempBC2;

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

  uint64_t spacePartitionX(std::vector<binaryTuple> &bat1,
                           std::vector<binaryTuple> &bat2,
                           std::vector<double> &min,
                           uint64_t &numPartStripes,
                           uint64_t maxEntryPerBucket,
                           double &xMin,
                           double &xMax,
                           uint64_t &divideFactor) {

    uint64_t tempNumPartStripes = numPartStripes;
    // temporary partition table for bat1
    std::vector<std::vector<binaryTuple>> partBAT1;
    // temporary partition table for bat2
    std::vector<std::vector<binaryTuple>> partBAT2; 
    std::vector<uint64_t> bucketCounter1; //contains number of tuples in bucket
    std::vector<uint64_t> bucketCounter2; //contains number of tuples in bucket


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

  std::vector<binaryTuple> createBAT(
                          const std::vector<CRelAlgebra::TBlock*> &tBlockVector,
                             const uint64_t &joinIndex,
                             double &xMin,
                             double &xMax,
                             double &yMin,
                             double &yMax,
                             size_t dim) {
                               
    std::vector<binaryTuple> BAT;
    uint64_t tBlockNum = 1;
    binaryTuple temp;
    
    while(tBlockNum <= tBlockVector.size()) {

      CRelAlgebra::TBlockIterator tBlockIter =
                                       tBlockVector[tBlockNum-1]->GetIterator();
      uint64_t row = 0;

      while(tBlockIter.IsValid()) {

        const CRelAlgebra::TBlockEntry &tuple = tBlockIter.Get();
        temp.blockNum = tBlockNum;
        temp.row = row;

        if(dim == 2) {
          CRelAlgebra::SpatialAttrArrayEntry<2> attribute = tuple[joinIndex];
          Rectangle<2> rec = attribute.GetBoundingBox();
          temp.xMin = rec.MinD(0);
          temp.xMax = rec.MaxD(0);
          temp.yMin = rec.MinD(1);
          temp.yMax = rec.MaxD(1);
        }
        else {
          CRelAlgebra::SpatialAttrArrayEntry<3> attribute = tuple[joinIndex];
          Rectangle<3> rec = attribute.GetBoundingBox();
          temp.xMin = rec.MinD(0);
          temp.xMax = rec.MaxD(0);
          temp.yMin = rec.MinD(1);
          temp.yMax = rec.MaxD(1);
          temp.zMin = rec.MinD(2);
          temp.zMax = rec.MaxD(2);
        }

      // compute xMin, xMax, yMin and yMax for currently binary tables
      if(temp.xMin < xMin)
        xMin = temp.xMin;
      if(temp.xMax > xMax)
        xMax = temp.xMax;
      if(temp.yMin < yMin)
        yMin = temp.yMin;
      if(temp.yMax > yMax)
        yMax = temp.yMax;

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
  SpatialJoinState(const std::vector<CRelAlgebra::TBlock*> &fTBlockVector_,
                   const std::vector<CRelAlgebra::TBlock*> &sTBlockVector_,
                   uint64_t fIndex_,
                   uint64_t sIndex_,
                   uint64_t fNumTuples_,
                   uint64_t sNumTuples_,
                   uint64_t rTBlockSize_,
                   uint64_t numStripes_,
                   uint64_t maxTuple_,
                   size_t fDim_,
                   size_t sDim_) :

                   fTBlockVector(fTBlockVector_),
                   sTBlockVector(sTBlockVector_),
                   fIndex(fIndex_),
                   sIndex(sIndex_),
                   fNumTuples(fNumTuples_),
                   sNumTuples(sNumTuples_),
                   rTBlockSize(rTBlockSize_),
                   maxEntryPerBucket(8192),
                   structFactor(7),
                   fDim(fDim_),
                   sDim(sDim_),
                   xMin(0),
                   xMax(0),
                   yMin(0),
                   yMax(0),
                   fNumColumns(fTBlockVector[0]->GetColumnCount()),
                   sNumColumns(sTBlockVector[0]->GetColumnCount()),
                   pos1(0),
                   pos2(0),
                   resumePart(0),
                   resume(false),
                   stripeWidth(0),
                   eq_size(0),
                   partLevel(0),
                   bucketNumber(0),
                   newTuple(new CRelAlgebra::AttrArrayEntry[fNumColumns
                                                            +sNumColumns]) {

        // mask to decode bucket number
    bucketNumberMask = (1ULL << 24) - 1;
    // mask to decode block number
    // compute 0-16:1-48 bits
    blockMask = (1ULL << 32) - 1;
    // mask to decode row number
    rowMask = (1ULL << 32) -1;
                     
    fBAT = createBAT(fTBlockVector, fIndex, xMin, xMax, yMin, yMax, fDim);
    sBAT = createBAT(sTBlockVector, sIndex, xMin, xMax, yMin, yMax, sDim);

    beginNumStripes = (fBAT.size() > sBAT.size()) ?
                       fBAT.size()/maxEntryPerBucket :
                       sBAT.size()/maxEntryPerBucket;
    
    if(beginNumStripes == 0) {
      beginNumStripes = 1;
    }
    numStripes = beginNumStripes;

    stripeWidth = (xMax - xMin)/beginNumStripes;
    
    // factor used by computing of number of parts by additional partition
    divideFactor = sqrt(beginNumStripes);
    if(divideFactor < 2) {
      divideFactor = 2;
    }

    numStripes = spacePartitionX(fBAT, sBAT, min, numStripes,
                                 maxEntryPerBucket, xMin, xMax, divideFactor);
                                 
    sizeBAT1 = fBAT.size();
    sizeBAT2 = sBAT.size();
    
  }

  // Destructor                 
  ~SpatialJoinState() {
    
    delete[] newTuple;
    fBAT.clear();
    sBAT.clear();
    deleteGrid(fSweepStruct);
    deleteGrid(sSweepStruct);
    eq.clear();
    min.clear();
  }

  bool nextTBlock(CRelAlgebra::TBlock* ntb) {

    // plane-sweep over all parts
    for(uint64_t tempPart = resumePart; tempPart < numStripes; tempPart++) {

      // if not new tuple block
      if(!resume) {
        // read parts from partitioned binary tables
        // until boundary of part or of array is not arive
        // push tuples in event queue
        while((pos1 < sizeBAT1) && ((fBAT[pos1].row>>32) == tempPart)) {
          Event tempEvent_in;
          Event tempEvent_out;
          
          tempEvent_in.bt = fBAT[pos1];
          tempEvent_in.stream = 1;
          tempEvent_in.in_out = 1;
          tempEvent_in.y = fBAT[pos1].yMin;
          eq.push_back(tempEvent_in);

          tempEvent_out.bt = fBAT[pos1];
          tempEvent_out.stream = 1;
          tempEvent_out.in_out = 0;
          tempEvent_out.y = fBAT[pos1].yMax;
          eq.push_back(tempEvent_out);
          
          pos1++;
        }

        while((pos2 < sizeBAT2) && ((sBAT[pos2].row>>32) == tempPart)) {
          Event tempEvent_in;
          Event tempEvent_out;
          
          tempEvent_in.bt = sBAT[pos2];
          tempEvent_in.stream = 2;
          tempEvent_in.in_out = 1;
          tempEvent_in.y = sBAT[pos2].yMin;
          eq.push_back(tempEvent_in);

          tempEvent_out.bt = sBAT[pos2];
          tempEvent_out.stream = 2;
          tempEvent_out.in_out = 0;
          tempEvent_out.y = sBAT[pos2].yMax;
          eq.push_back(tempEvent_out);
          
          pos2++;
        }
        
        eq_size = eq.size();

        //if event queue is not empty
        if(eq_size != 0) {

          // sort actually event queue by y coordinate
          MergeSortY(eq);

          stripeWidth = (xMax - xMin)/beginNumStripes;
        
          partLevel = eq[0].bt.blockNum >> 56; // >>32 and >>24
          bucketNumber = (eq[0].bt.blockNum >> 32) & bucketNumberMask;

          // if original stripe was redistributed
          if(partLevel != 0) 
            for(uint64_t i=0; i<partLevel; i++) {
              stripeWidth = stripeWidth/divideFactor;
           }

          // create sweep struct
          newGrid(fSweepStruct, structFactor);
          newGrid(sSweepStruct, structFactor);
          
        } // end of if(eq_size != 0)
      } // end of if !resume

      double tempXMin = min[tempPart]+stripeWidth*bucketNumber;

      // make plane sweep over actually event queue
      while(!eq.empty()) {

        if(!resume) {
          tempEvent = eq.front();
        }

        // if input event
        if(tempEvent.in_out == 1) {
          // set rectangle as active
          // and save it in sweep struct
          if(tempEvent.stream == 1) {
            if(!resume) {
              sweepPush(fSweepStruct, tempEvent.bt, tempXMin, stripeWidth);
             
            }
            // search for intersections
            if(!sweepSearch(sSweepStruct, ntb, tempEvent.bt,
                            tempEvent.stream, tempPart)) {
              return false;              
            } 
          } // end of if stream == 1
          else {
            if(!resume) {
              sweepPush(sSweepStruct, tempEvent.bt, tempXMin, stripeWidth);
            }
            // search for intersections
            if(!sweepSearch(fSweepStruct, ntb, tempEvent.bt,
                            tempEvent.stream, tempPart)) {
              return false;
            }
          } // end of if stream == 2
        } // end of input event
        // output event
        else {
        // delete intervall from sweep struct
          if(tempEvent.stream == 1) {
            sweepRemove(fSweepStruct, tempEvent.bt, tempXMin, stripeWidth);
          }
          else {
            sweepRemove(sSweepStruct, tempEvent.bt, tempXMin, stripeWidth);
          }  
        } // end of if output event
        // move sweep line
        eq.pop_front();     
      } // end of while(!eq.empty) loop

      // free memory
      deleteGrid(fSweepStruct);
      deleteGrid(sSweepStruct);
      eq.clear();
      resumePart++;
    } // end for()
    
    return true;
  } // end of nextTBlock()

  private:

  bool sweepSearch(Grid &gr,
                   CRelAlgebra::TBlock* ntb,
                   binaryTuple searchTuple,
                   int stream,
                   uint64_t part) {

    binaryTuple tempTuple;
    bool intersection = false;
    double tempXMin = min[part]+stripeWidth*bucketNumber;

    uint64_t cellNumLeft;
    uint64_t cellNumRigth;

    if(tempXMin >= searchTuple.xMin) {
      cellNumLeft = 0;
    }
    else {
      cellNumLeft = (searchTuple.xMin - tempXMin)/(stripeWidth/gr.cellNum);
    }
    
    cellNumRigth = (searchTuple.xMax - tempXMin)/(stripeWidth/gr.cellNum);

    if(cellNumRigth >= gr.cellNum) {
      cellNumRigth = gr.cellNum-1;
    }

    if(!resume) {
      resumeIterGrid = cellNumLeft;
      resumeIterCell = 0;
    }
    else {
      resumeIterCell++;
      resume = false;
    }

    for(uint64_t i=resumeIterGrid; i<=cellNumRigth; i++) {
      for(uint64_t j=resumeIterCell; j<gr.cells[i].size(); j++) {
        
        tempTuple = gr.cells[i][j];
        
        // intersection test
        intersection = ((searchTuple.xMin <= tempTuple.xMax)
                     && (searchTuple.xMax >= tempTuple.xMin));

       // if both tuples are 3-dimensional
        if(fDim == 3 && sDim == 3) {
          intersection = intersection &&
                         ((searchTuple.zMin <= tempTuple.zMax)
                      && (searchTuple.zMax >= tempTuple.zMin));
        }

        // if both tuple was processed in last grid cell
        if((searchTuple.xMin < (tempXMin + (stripeWidth/gr.cellNum)*i))
        && (tempTuple.xMin < (tempXMin + (stripeWidth/gr.cellNum)*i))) {
            intersection = false;
        }

        if(intersection) {

          // save next result tuple in result tuple block
          
          if(stream == 1) {
            const CRelAlgebra::TBlockEntry &fTuple = CRelAlgebra::TBlockEntry(
                  fTBlockVector[(searchTuple.blockNum & blockMask) - 1],
                  searchTuple.row & rowMask);

            const CRelAlgebra::TBlockEntry &sTuple = CRelAlgebra::TBlockEntry(
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
            const CRelAlgebra::TBlockEntry &sTuple = CRelAlgebra::TBlockEntry(
                  sTBlockVector[(searchTuple.blockNum & blockMask) - 1],
                  searchTuple.row & rowMask);

            const CRelAlgebra::TBlockEntry &fTuple = CRelAlgebra::TBlockEntry(
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
            resumeIterCell = j;
            resumeIterGrid = i;
            resume = true;
            return false;
          }
          
        } // end of if intersection
        
      }
      resumeIterCell = 0;
    }

  return true;
  } // end of sweepSearch
  
  const std::vector<CRelAlgebra::TBlock*> &fTBlockVector;
  const std::vector<CRelAlgebra::TBlock*> &sTBlockVector;
  uint64_t fIndex;
  uint64_t sIndex;
  uint64_t fNumTuples;
  uint64_t sNumTuples;
  uint64_t rTBlockSize;
  std::vector<binaryTuple> fBAT;
  std::vector<binaryTuple> sBAT;
  std::vector<double> min;
  uint64_t maxEntryPerBucket;
  uint64_t structFactor;
  uint64_t beginNumStripes;
  uint64_t numStripes;
  size_t fDim;
  size_t sDim;
  
  double xMin;
  double xMax;
  double yMin;
  double yMax;

  const size_t fNumColumns;
  const size_t sNumColumns;

  uint64_t pos1;
  uint64_t pos2;
  uint64_t resumePart;
  uint64_t resumeIterCell;
  uint64_t resumeIterGrid;

  Grid fSweepStruct;
  Grid sSweepStruct;
  bool resume;

  uint64_t divideFactor;

  double stripeWidth;
  uint64_t eq_size;
    
  uint64_t bucketNumberMask;
  uint64_t partLevel;
  uint64_t bucketNumber;
  uint64_t blockMask;
  uint64_t rowMask;

  std::deque<Event> eq;
  Event tempEvent;
        
  uint64_t sizeBAT1;
  uint64_t sizeBAT2;


  CRelAlgebra::AttrArrayEntry* const newTuple; 
};
}
