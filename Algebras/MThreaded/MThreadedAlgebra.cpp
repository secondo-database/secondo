/*
----
This file is part of SECONDO.

Copyright (C) 2019,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



1 The MThreadedAlgebra class

Our MThreaded-Algebra has only a bunch of Operators.

*/
#include "AlgebraManager.h"
#include "ListUtils.h"

#include "basicOperators/opTypeOperators.h"
#include "basicOperators/opBasicOperators.h"
#include "sortOperators/opMergeSort.h"
#include "joinOperators/opHybridHashJoin.h"
#include "joinOperators/opSpatialJoin.h"
#include "joinOperators/opFilter.h"
#include "MThreadedAux.h"

extern NestedList* nl;
extern QueryProcessor* qp;

namespace mthreaded {

class MThreadedAlgebra : public Algebra {
/*
1.1 Operators for the algebra.
The shared-pointers are alive as long as the algebra-object lives in Secondo.
(C++11-feature)

1.1.1 Type Operators

*/
   std::shared_ptr<Operator> opTypeSpatial =
           op_typeop_spatial().getOperator();
   std::shared_ptr<Operator> opTypeSpatial2 =
           op_typeop_spatial2().getOperator();

/*
1.1.2 Basic Operators

*/
   std::shared_ptr<Operator> opMaxCore = op_maxcore().getOperator();
   std::shared_ptr<Operator> opSetCore = op_setcore().getOperator();
   std::shared_ptr<Operator> opGetCore = op_getcore().getOperator();

/*
1.1.3 Sort Operators

*/

   std::shared_ptr<Operator> opMergeSort = op_mergeSort().getOperator();

/*
1.1.4 Join Operators

*/

   std::shared_ptr<Operator> opHybridJoin = op_hybridHashJoin().getOperator();
   std::shared_ptr<Operator> opSpatialJoin = op_spatialJoin().getOperator();
   std::shared_ptr<Operator> oprefinement = op_refinement().getOperator();


   public:
   MThreadedAlgebra() : Algebra() {

      // type operators
      AddOperator(opTypeSpatial.get());
      AddOperator(opTypeSpatial2.get());

      // basic operators
      AddOperator(opMaxCore.get());
      AddOperator(opSetCore.get());
      AddOperator(opGetCore.get());

      // sort operators
      opMergeSort.get()->SetUsesArgsInTypeMapping();
      opMergeSort.get()->SetUsesMemory();
      AddOperator(opMergeSort.get());

      // join operators
      opHybridJoin.get()->SetUsesMemory();
      AddOperator(opHybridJoin.get());

      opSpatialJoin.get()->SetUsesMemory();
      AddOperator(opSpatialJoin.get());

      oprefinement.get()->SetUsesArgsInTypeMapping();
      AddOperator(oprefinement.get());
   }
};
}

extern "C"
Algebra* InitializeMThreadedAlgebra(NestedList* nlRef, QueryProcessor* qpRef) {
   return new mthreaded::MThreadedAlgebra;
}
