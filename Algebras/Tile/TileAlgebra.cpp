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
#include "Grid2.h"
#include "Grid3.h"
#include "tintArray.h"
#include "tintFlob.h"

/*
includes for TileAlgebra operators

*/



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
  AddTypeConstructor(new TypeConstructor(Grid2::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(Grid3::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(tintArray::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(tintFlob::GetTypeConstructor()));
}

TileAlgebra::~TileAlgebra()
{
  
}

}
