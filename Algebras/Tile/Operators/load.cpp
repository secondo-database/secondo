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

#include "load.h"
#include "../t/tintArray.h"
#include "../t/tintFlob.h"

namespace TileAlgebra
{

ValueMapping loadFunctions[] =
{
  loadFunction<tintArray>,
  loadFunction<tintFlob>,
  0
};

int loadSelectFunction(ListExpr args)
{
  int nSelection = -1;

  NList type(args);

  if(type.first().isSymbol(tintArray::BasicType()))
  {
    nSelection = 0;
  }

  else if (type.first().isSymbol(tintFlob::BasicType()))
  {
    nSelection = 1;
  }

  return nSelection;
}

ListExpr loadTypeMapping(ListExpr args)
{
  NList types(args);

  if(types.first() == NList(tintArray::BasicType()) ||
     types.first() == NList(tintFlob::BasicType()))
  {
    return NList().listExpr();
  }

  return NList::typeError("Expecting a tintArray or a tintFlob.");
}

}
