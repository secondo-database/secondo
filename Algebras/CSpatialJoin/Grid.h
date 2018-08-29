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
#ifndef GRID_H_
#define GRID_H_

#include "BinaryTuple.h"

//using namespace std;

namespace csj {

  struct Grid {

    Grid() : cellNum(0) {}
    
    vector<vector<binaryTuple>> cells;
    uint64_t cellNum; // number of grid cells
  };

  void newGrid(Grid &gr, uint64_t cellNum_) {

    gr.cellNum = cellNum_;

    for(uint64_t i=0; i<gr.cellNum; i++) {
      vector<binaryTuple> temp;
      gr.cells.push_back(temp);
    }
    
  }

  void toScreen(Grid &gr) {
    for(uint64_t i=0; i<gr.cells.size(); i++) {
      for(uint64_t k=0; k<gr.cells[i].size(); k++){
        BinaryTupleToScreen(gr.cells[i][k], 2);
      }
      cout<<endl;
    }
  }

  void MergeSortGridCell(vector<binaryTuple> &pss) {

    uint64_t sizeIter;
    uint64_t blockIter;
    uint64_t lBlockIter;
    uint64_t rBlockIter;
    uint64_t mergeIter;
    uint64_t lBorder;
    uint64_t mBorder;
    uint64_t rBorder;
    uint64_t numTuples = pss.size();

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
          if(pss[lBorder+lBlockIter].yMax > pss[mBorder+rBlockIter].yMax) {
            SortedBlock[lBlockIter+rBlockIter] = pss[lBorder + lBlockIter];
            lBlockIter += 1;
          }
          else {
            SortedBlock[lBlockIter+rBlockIter] = pss[mBorder+rBlockIter];
            rBlockIter += 1;
          }
        }
        
        // After that, we enter the remaining elements
        // from the left or right block
        while(lBorder+lBlockIter < mBorder) {
          SortedBlock[lBlockIter+rBlockIter] = pss[lBorder+lBlockIter];
          lBlockIter += 1;
        }
        while(mBorder + rBlockIter < rBorder) {
          SortedBlock[lBlockIter+rBlockIter] = pss[mBorder+rBlockIter];
          rBlockIter += 1;
        }

        for(mergeIter = 0; mergeIter < lBlockIter + rBlockIter; mergeIter++) {
          pss[lBorder+mergeIter] = SortedBlock[mergeIter];
        }
        SortedBlock.clear();
      }
    }
  } // end of MergeSortGridCell

  void sweepPush(Grid &gr, binaryTuple bt, double xMin, double stripeWidth) {

    uint64_t cellNumLeft;
    uint64_t cellNumRigth;

    if(xMin >= bt.xMin) {
      cellNumLeft = 0;
    }
    else {
      cellNumLeft = (bt.xMin - xMin)/(stripeWidth/gr.cellNum);
    }

    cellNumRigth = (bt.xMax - xMin)/(stripeWidth/gr.cellNum);

    if(cellNumRigth >= gr.cellNum) {
      cellNumRigth = gr.cellNum - 1;
    }

    for(uint64_t i=cellNumLeft; i<=cellNumRigth; i++) {
      gr.cells[i].push_back(bt);
    }

  } // end of sweepPush

  void sweepRemove(Grid &gr, binaryTuple bt, double xMin, double stripeWidth) {

    uint64_t cellNumLeft;
    uint64_t cellNumRigth;

    if(xMin >= bt.xMin) {
      cellNumLeft = 0;
    }
    else {
      cellNumLeft = (bt.xMin - xMin)/(stripeWidth/gr.cellNum);
    }
    
    cellNumRigth = (bt.xMax - xMin)/(stripeWidth/gr.cellNum);
    
    if(cellNumRigth >= gr.cellNum) {
      cellNumRigth = gr.cellNum - 1;
    }

    for(uint64_t i=cellNumLeft; i<=cellNumRigth; i++) {

      vector<binaryTuple> temp;

      for(uint64_t j = 0; j < gr.cells[i].size(); j++) {
        if(!tuplesAreEqual(bt, gr.cells[i][j])) {
          temp.push_back(gr.cells[i][j]);
        }
      }
      gr.cells[i].clear();
      gr.cells[i] = temp;
      temp.clear();
    }
     
  } // end of sweepRemove

  void deleteGrid(Grid &gr) {

    for(uint64_t i=0; i<gr.cells.size(); i++) {
      gr.cells[i].clear();  
    }
    gr.cells.clear();
  }
  
} // end of namespace csj

#endif // GRID_H_
