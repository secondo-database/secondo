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

#ifndef EVENTLIST_H_
#define EVENTLIST_H_
   
#include "BinaryTuple.h"
#include <deque>


//using namespace std;

namespace csj {

  struct Event {

    Event () : in_out{0}, stream{0}, y{0} {}

    binaryTuple bt;
    size_t in_out; // =1, if input event (bottom border of rectangel)
                   // =0 if output event (top border of rectangle)
    size_t stream;
    double y; 
  };
  
  void MergeSortY(deque<Event> &eq) {

    uint64_t sizeIter;
    uint64_t blockIter;
    uint64_t lBlockIter;
    uint64_t rBlockIter;
    uint64_t mergeIter;
    uint64_t lBorder;
    uint64_t mBorder;
    uint64_t rBorder;
    uint64_t numTuples = eq.size();
    
    
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
        deque<Event> SortedBlock(rBorder-lBorder);

        // While in both arrays there are elements we select the smaller
        // of them and put them in the sorted block
        while(lBorder+lBlockIter < mBorder && mBorder+rBlockIter < rBorder) {
          if(eq[lBorder+lBlockIter].y < eq[mBorder+rBlockIter].y) {
            SortedBlock[lBlockIter+rBlockIter] = eq[lBorder + lBlockIter];
            lBlockIter += 1;
          }
          else {
            SortedBlock[lBlockIter+rBlockIter] = eq[mBorder+rBlockIter];
            rBlockIter += 1;
          }
        }
        
        // After that, we enter the remaining elements
        // from the left or right block
        while(lBorder+lBlockIter < mBorder) {
          SortedBlock[lBlockIter+rBlockIter] = eq[lBorder+lBlockIter];
          lBlockIter += 1;
        }
        while(mBorder + rBlockIter < rBorder) {
          SortedBlock[lBlockIter+rBlockIter] = eq[mBorder+rBlockIter];
          rBlockIter += 1;
        }

        for(mergeIter = 0; mergeIter < lBlockIter + rBlockIter; mergeIter++) {
          eq[lBorder+mergeIter] = SortedBlock[mergeIter];
        }
        SortedBlock.clear();
      }
    }
  }
}

#endif // EVENTLIST_H_
