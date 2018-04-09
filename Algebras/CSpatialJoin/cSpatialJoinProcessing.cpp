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

using namespace std;
using namespace CRelAlgebra;

namespace csj {

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
