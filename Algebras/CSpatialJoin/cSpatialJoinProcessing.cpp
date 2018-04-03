/*
----
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
----

\tableofcontents
 
1 Cache-conscious spatial join

The ~cspatialjoin~ operator is a cache-conscious spatial-join operator, 
which performs a partitioned spatial-join on two streams of tuple blocks.
As arguments it expects two streams of tuple blocks and the name of the 
join attribute for each argument relation.

1.1 Imports

*/
    
#include "NestedList.h"         
#include "QueryProcessor.h"    
#include "AlgebraManager.h"         
#include "StandardTypes.h"     
#include "Symbols.h"           
#include "ListUtils.h"          
#include <iostream>
#include <math.h> 
#include "CRel.h"
#include "CRelTC.h"
#include "OperatorUtils.h"
#include "RelationAlgebra.h"
#include "Stream.h"
#include "TBlock.h"
#include "TBlockTC.h"
#include "cSpatialJoinProcessing.h"
      

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace std;
using namespace CRelAlgebra;

namespace csj {

  SpatialJoinState::SpatialJoinState(const vector<TBlock*> fTBlockVector,
                           const vector<TBlock*> sTBlockVector,
                           uint64_t fIndex,
                           uint64_t sIndex,
                           uint64_t fNumTuples,
                           uint64_t sNumTuples,
                           uint64_t rTBlockSize) {}
  SpatialJoinState::~SpatialJoinState() {}

  bool SpatialJoinState::nextTBlock(TBlock* ntb) {
    return true;
}

}
