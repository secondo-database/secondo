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
    
    std::vector<std::vector<binaryTuple>> cells;
    uint64_t cellNum; // number of grid cells
  };

  void newGrid(Grid &gr, uint64_t cellNum_) {

    gr.cellNum = cellNum_;

    for(uint64_t i=0; i<gr.cellNum; i++) {
      std::vector<binaryTuple> temp;
      gr.cells.push_back(temp);
    }
    
  }

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

      uint64_t deletePos = 0;
      bool hit = false;

      // search for tuple 
      while(!(deletePos == gr.cells[i].size()) && !hit) {
        if(tuplesAreEqual(bt, gr.cells[i][deletePos])) {
          hit = true;
        }
        else {
          deletePos++;
        }
      } // end of while
    
      if(hit) {
        gr.cells[i].erase(gr.cells[i].begin()+deletePos);
      }
    } // end of for
    
  } // end of sweepRemove

  void deleteGrid(Grid &gr) {

    for(uint64_t i=0; i<gr.cells.size(); i++) {
      gr.cells[i].clear();  
    }
    gr.cells.clear();
  }
  
} // end of namespace csj

#endif // GRID_H_
