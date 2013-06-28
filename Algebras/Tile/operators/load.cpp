/*
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, Department of Computer Science,
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

/*
definition of template loadFunction

*/

template <typename Type>
int loadFunction(Word* pArguments,
                 Word& rResult,
                 int message,
                 Word& rLocal,
                 Supplier supplier)
{
  Type* pImplementationType = static_cast<Type*>(pArguments[0].addr);

  if(pImplementationType != 0)
  {
    rResult = qp->ResultStorage(supplier);

    CcBool* pResult = static_cast<CcBool*>(rResult.addr);

    if(pResult != 0)
    {
      pResult->Set(true, pImplementationType->load());
    }
  }

  return 0;
}

/*
definition of load functions

*/

ValueMapping loadFunctions[] =
{
  loadFunction<tintArray>,
  loadFunction<tintFlob>,
  0
};

/*
definition of load select function

*/

int loadSelectFunction(ListExpr arguments)
{
  int nSelection = -1;

  NList type(arguments);

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

/*
definition of load type mapping function

*/

ListExpr loadTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator load expects "
                                   "a tintArray or a tintFlob.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(1))
  {
    std::string argument1 = argumentsList.first().str();

    if(argument1 == tintArray::BasicType() ||
       argument1 == tintFlob::BasicType())
    {
      type = NList(CcBool::BasicType()).listExpr();
    }
  }

  return type;
}

}
