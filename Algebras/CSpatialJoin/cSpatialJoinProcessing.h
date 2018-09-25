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
#include "ITNode.h"
#include "RectangleAlgebra.h"
#include "SpatialAttrArray.h"

using namespace std;
using namespace CRelAlgebra;

namespace csj {

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
        // Create temporary structs for additional partition
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

  vector<binaryTuple> createBAT(const vector<TBlock*> &tBlockVector,
                             const uint64_t &joinIndex,
                             double &xMin,
                             double &xMax,
                             double &yMin,
                             double &yMax,
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
                   size_t sDim_,
                   fstream* file_) :

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
                   file(file_),
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
                   newTuple(new AttrArrayEntry[fNumColumns+sNumColumns]) {

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
                       (fBAT.size())/maxEntryPerBucket :
                       (sBAT.size())/maxEntryPerBucket;
                       
    stripeWidth = (xMax - xMin)/beginNumStripes;
                       
    
    if(beginNumStripes == 0) {
      beginNumStripes = 1;
    }
    numStripes = beginNumStripes;
    
    // factor used by computing of number of parts by additional partition
    divideFactor = sqrt(beginNumStripes);
    if(divideFactor < 2) {
      divideFactor = 2;
    }

    numStripes = spacePartitionX(fBAT, sBAT, min, numStripes,
                                 maxEntryPerBucket, xMin, xMax, divideFactor);

     //                          cout<<"  ens: "<<numStripes<<endl;
                                 
    sizeBAT1 = fBAT.size();
    sizeBAT2 = sBAT.size();

    fSweepStruct = nullptr;
    sSweepStruct = nullptr;
  }

  // Destructor                 
  ~SpatialJoinState() {
    
    delete[] newTuple;
    fBAT.clear();
    sBAT.clear();
    deleteTree(fSweepStruct);
    deleteTree(sSweepStruct);
    eq.clear();
    fSweepStruct = nullptr;
    sSweepStruct = nullptr;
    delete resumeIterTree;
    resumeIterTree = nullptr;
    min.clear();
  }

  bool nextTBlock(TBlock* ntb) {

    double tempXMin;
    double tempXMax;
    
    // plane-sweep over all parts
    for(uint64_t tempPart = resumePart; tempPart < numStripes; tempPart++) {

     // cout<<endl<<"PART: "<<tempPart<<endl;
      // if not new tuple block
      if(!resume) {
        // read parts from partitioned binary tables
        // until boundary of part or of array is not arive
        // push tuples in event queue
        while((pos1 < sizeBAT1) && ((fBAT[pos1].row>>32) == tempPart)) {
          Event tempEvent;
          
          tempEvent.bt = fBAT[pos1];
          tempEvent.stream = 1;
          tempEvent.y = fBAT[pos1].yMin;
          eq.push_back(tempEvent);
          
          pos1++;
        }

        while((pos2 < sizeBAT2) && ((sBAT[pos2].row>>32) == tempPart)) {
          Event tempEvent;
          
          tempEvent.bt = sBAT[pos2];
          tempEvent.stream = 2;
          tempEvent.y = sBAT[pos2].yMin;
          eq.push_back(tempEvent);
          
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

        tempXMin = min[tempPart]+stripeWidth*bucketNumber;
        tempXMax = tempXMin + stripeWidth;

        fSweepStruct = createITree(fSweepStruct, tempXMin, tempXMax, 3);
        sSweepStruct = createITree(sSweepStruct, tempXMin, tempXMax, 3);


       // display(fSweepStruct, 4);cout<<endl<<endl;
      //  display(sSweepStruct, 4);cout<<endl<<endl;
         
        } // end of if(eq_size != 0)
      } // end of if !resume

      // make plane sweep over actually event queue
      while(!eq.empty()) {

        if(!resume) {
          tempEvent = eq.front();
          son = false;
          resumeIterTree = nullptr;
        }

        // set rectangle as active
        // and save it in sweep struct
          if(tempEvent.stream == 1) {
            if(!resume) {
              resumeIterTree = sSweepStruct;
              sweepPush(fSweepStruct, tempEvent.bt);
              sweepRemove(sSweepStruct, tempEvent.bt);            
            }
            if(!sweepSearch(resumeIterTree, ntb, tempEvent.bt,
                                tempEvent.stream, tempPart)) {
              return false;              
            }
          }
          else {
            if(!resume) {
              resumeIterTree = fSweepStruct;
              sweepPush(sSweepStruct, tempEvent.bt);
              sweepRemove(fSweepStruct, tempEvent.bt);
            }
            if(!sweepSearch(resumeIterTree, ntb, tempEvent.bt,
                                tempEvent.stream, tempPart)) {
              return false;
            }
          }
        // move sweep line
        eq.pop_front();     
      } // end of while(!eq.empty) loop

      // free memory
      eq.clear();
      deleteTree(fSweepStruct);
      deleteTree(sSweepStruct);
      fSweepStruct = nullptr;
      sSweepStruct = nullptr;
      resumeIterTree = nullptr;
      
      resumePart++;
      son = false;
    } // end for()
    
    return true;
  } // end of nextTBlock()

  private:

  bool sweepSearch(ITNode *root,
                   TBlock *ntb,
                   binaryTuple searchTuple,
                   int stream,
                   uint64_t part) {



    if(root == nullptr) {
      return true;
    }

    if(searchTuple.xMax < root->xLeft) {
      if(!sweepSearch(root->left, ntb, searchTuple, stream, part)) {
        return false;
      }
      return true;
    }
  

    if(searchTuple.xMin > root->xRight) {
      if(!sweepSearch(root->right, ntb, searchTuple, stream, part)) {
        return false;
      }
      return true;
    }

/*cout<<endl<<root->xLeft<<":"<<root->xRight;
  cout<<endl<<"(son: "<<son<<")"<<endl;
 cout<<endl<<"search tuple: ";
    BinaryTupleToScreen(searchTuple, 2); */

    binaryTuple tempTuple;
    bool intersection = false;


/*
if(stream == 1) {
             cout<<"sSweepStruct Node vector: "<<endl;
}
else
 {
             cout<<"fSweepStruct Node vector: "<<endl;
 }

*/
    if(!resume) {
      resumeIterNode = 0;
    }
    else {
      resume = false;
    }

    if(son) {
      searchTuple.row = searchTuple.row | (1ULL << 63);
    }

    for(uint64_t j = resumeIterNode; j < root->vt.size(); j++) {
        
      tempTuple = root->vt[j];

  //cout<<"intersection with ";
  //BinaryTupleToScreen(tempTuple, 2);
      

     // BinaryTupleToScreen(tempTuple, 2);
     
      // if both tuple was processed in last part
      if((searchTuple.xMin < (min[part] + stripeWidth*bucketNumber))
        && (tempTuple.xMin < (min[part] + stripeWidth*bucketNumber))) {
          continue;
      }

    // if both tuple was processed in parent node
      if(((searchTuple.row >> 63) == 1)
        && ((tempTuple.row >> 63) == 1)) {
          continue;
        }


     // intersection test
     intersection = ((searchTuple.xMin <= tempTuple.xMax)
                   && (searchTuple.xMax >= tempTuple.xMin));
              //     cout<<"intersection before: "<<intersection<<"    ";



 //cout<<"intersection: "<<intersection<<endl;
  
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
          resumeIterNode = j++;
          resumeIterTree = root;
          resume = true;
          return false;
        }
          
      } // end of if intersection
    } // end of external for-loop

    if(searchTuple.xMin < resumeIterTree->xLeft) {
      son = true;
      if(!sweepSearch(root->left, ntb, searchTuple, stream, part)) {
        return false;
      }
    }

    if(searchTuple.xMax > root->xRight) {
      son = true;
      if(!sweepSearch(root->right, ntb, searchTuple, stream, part)) {
        return false;
      }
    }

  return true;
  } // end of sweepSearch
  
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

  fstream *file;
  
  double xMin;
  double xMax;
  double yMin;
  double yMax;

  const size_t fNumColumns;
  const size_t sNumColumns;

  uint64_t pos1;
  uint64_t pos2;
  uint64_t resumePart;
  uint64_t resumeIterNode;
  ITNode *resumeIterTree;
  bool resume;
  bool son;

  ITNode *fSweepStruct;
  ITNode *sSweepStruct;

  uint64_t divideFactor;

  double stripeWidth;
  uint64_t eq_size;
    
  uint64_t bucketNumberMask;
  uint64_t partLevel;
  uint64_t bucketNumber;
  uint64_t blockMask;
  uint64_t rowMask;

  deque<Event> eq;
  Event tempEvent;
        
  uint64_t sizeBAT1;
  uint64_t sizeBAT2;


  AttrArrayEntry* const newTuple; // AttrArrayEntry* const newTuple;

};
} // end of name space csj

