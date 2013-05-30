/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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
#include "NestedList.h"
#include "QueryProcessor.h"
#include "TileAlgebra.h"

/*
includes for TileAlgebra types

*/

#include "grid/grid2.h"
#include "grid/grid3.h"
#include "t/tintArray.h"
#include "t/tintFlob.h"
#include "t/tint.h"
#include "t/treal.h"
#include "t/tbool.h"
#include "t/tstring.h"

/*
includes for TileAlgebra operators

*/

#include "load.h"

/*
extern declarations

*/

extern NestedList* nl;
extern QueryProcessor* qp;

extern "C" Algebra* InitializeTileAlgebra(NestedList* pNestedList,
                                          QueryProcessor* pQueryProcessor)
{
  Algebra* pAlgebra = 0;

  nl = pNestedList;
  assert(nl != 0);

  qp = pQueryProcessor;
  assert(qp != 0);

  pAlgebra = new TileAlgebra::TileAlgebra();
  assert(pAlgebra != 0);

  return pAlgebra;
}

namespace TileAlgebra
{
  
TileAlgebra::TileAlgebra()
            :Algebra()
{
  /*
  Type Constructors

  */

  AddTypeConstructor(new TypeConstructor(grid2::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(grid3::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(tintArray::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(tintFlob::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(tint::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(treal::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(tbool::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(tstring::GetTypeConstructor()));

  /*
  Operators

  */

  AddOperator(loadInfo(), loadFunctions, loadSelectFunction, loadTypeMapping);
}

TileAlgebra::~TileAlgebra()
{
  
}

}
