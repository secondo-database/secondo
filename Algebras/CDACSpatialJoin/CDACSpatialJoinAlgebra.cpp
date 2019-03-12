/*
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

#include "Algebra.h"
#include "AlgebraManager.h"

#include "CDACSpatialJoin.h"
#include "CacheTest.h"
#include "CreateRectangles.h"

namespace cdacspatialjoin {
   class CDACSpatialJoinAlgebra : public Algebra {
      // CDACSpatialJoin operator
      std::shared_ptr<Operator> opCDACSpatialJoin =
              CDACSpatialJoin().getOperator();

      // CacheTest operator
      std::shared_ptr<Operator> opCacheTest =
              CacheTest().getOperator();

      // createRectangles1D/2D/3D stream operators
      std::shared_ptr<Operator> opCreateRectangles1D =
              CreateRectangles<1>().getOperator();
      std::shared_ptr<Operator> opCreateRectangles2D =
              CreateRectangles<2>().getOperator();
      std::shared_ptr<Operator> opCreateRectangles3D =
              CreateRectangles<3>().getOperator();

   public:
      CDACSpatialJoinAlgebra() : Algebra() {

         // CDACSpatialJoin operator
         opCDACSpatialJoin.get()->SetUsesMemory();
         AddOperator(opCDACSpatialJoin.get());

         // cacheTest operator
         opCacheTest.get()->SetUsesMemory();
         AddOperator(opCacheTest.get());

         // createRectangles1D/2D/3D stream operators
         AddOperator(opCreateRectangles1D.get());
         AddOperator(opCreateRectangles2D.get());
         AddOperator(opCreateRectangles3D.get());
      }
   };
} // end namespace cdacspatialjoin


extern "C"
Algebra *InitializeCDACSpatialJoinAlgebra(
        NestedList *nlRef, QueryProcessor *qpRef) {
   return new cdacspatialjoin::CDACSpatialJoinAlgebra();
}
